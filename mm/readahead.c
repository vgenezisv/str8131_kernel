/*
 * mm/readahead.c - address_space-level file readahead.
 *
 * Copyright (C) 2002, Linus Torvalds
 *
 * 09Apr2002	akpm@zip.com.au
 *		Initial version.
 */

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/backing-dev.h>
#include <linux/pagevec.h>
#include <linux/writeback.h>
#include <linux/nfsd/const.h>
#include <asm/div64.h>

/* The default max/min read-ahead pages. */
#define KB(size)	(((size)*1024 + PAGE_CACHE_SIZE-1) / PAGE_CACHE_SIZE)
#define MAX_RA_PAGES	KB(VM_MAX_READAHEAD)
#define MIN_RA_PAGES	KB(VM_MIN_READAHEAD)
#define MIN_NFSD_PAGES	KB(NFSSVC_MAXBLKSIZE/1024)

#define next_page(pg) (list_entry((pg)->lru.prev, struct page, lru))
#define prev_page(pg) (list_entry((pg)->lru.next, struct page, lru))

#ifdef CONFIG_ADAPTIVE_READAHEAD
/*
 * Adaptive read-ahead parameters.
 */

/* In laptop mode, poll delayed look-ahead on every ## pages read. */
#define LAPTOP_POLL_INTERVAL 16

/* Set look-ahead size to 1/# of the thrashing-threshold. */
#define LOOKAHEAD_RATIO 8

/* Set read-ahead size to ##% of the thrashing-threshold. */
// old: int readahead_ratio = 50;
int readahead_ratio = 35;
EXPORT_SYMBOL(readahead_ratio);

/* Readahead as long as cache hit ratio keeps above 1/##. */
int readahead_hit_rate = 2;
EXPORT_SYMBOL(readahead_hit_rate);

/*
 * Measures the aging process of cold pages.
 * Mainly increased on fresh page references to make it smooth.
 */
DEFINE_PER_CPU(unsigned long, readahead_aging);
EXPORT_PER_CPU_SYMBOL(readahead_aging);

/*
 * Detailed classification of read-ahead behaviors.
 */
#define RA_CLASS_SHIFT 4
#define RA_CLASS_MASK  ((1 << RA_CLASS_SHIFT) - 1)
enum ra_class {
	RA_CLASS_ALL,
	RA_CLASS_NEWFILE,
	RA_CLASS_STATE,
	RA_CLASS_CONTEXT,
	RA_CLASS_CONTEXT_AGGRESSIVE,
	RA_CLASS_BACKWARD,
	RA_CLASS_THRASHING,
	RA_CLASS_SEEK,
	RA_CLASS_END,
};
#endif /* CONFIG_ADAPTIVE_READAHEAD */

/*
 * Read-ahead events accounting.
 */
#ifdef CONFIG_DEBUG_READAHEAD
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>

#define DEBUG_READAHEAD_RADIXTREE

/* Read-ahead events to be accounted. */
enum ra_event {
	RA_EVENT_CACHE_MISS,		/* read cache misses */
	RA_EVENT_READRANDOM,		/* random reads */
	RA_EVENT_IO_CONGESTION,		/* io congestion */
	RA_EVENT_IO_CACHE_HIT,		/* canceled io due to cache hit */
	RA_EVENT_IO_BLOCK,		/* read on locked page */

	RA_EVENT_READAHEAD,		/* read-ahead issued */
	RA_EVENT_READAHEAD_HIT,		/* read-ahead page hit */
	RA_EVENT_LOOKAHEAD,		/* look-ahead issued */
	RA_EVENT_LOOKAHEAD_HIT,		/* look-ahead mark hit */
	RA_EVENT_LOOKAHEAD_NOACTION,	/* look-ahead mark ignored */
	RA_EVENT_READAHEAD_MMAP,	/* read-ahead for memory mapped file */
	RA_EVENT_READAHEAD_EOF,		/* read-ahead reaches EOF */
	RA_EVENT_READAHEAD_SHRINK,	/* ra_size under previous la_size */
	RA_EVENT_READAHEAD_THRASHING,	/* read-ahead thrashing happened */
	RA_EVENT_READAHEAD_MUTILATE,	/* read-ahead request mutilated */
	RA_EVENT_READAHEAD_RESCUE,	/* read-ahead rescued */

	RA_EVENT_END
};

static const char * const ra_event_name[] = {
	"cache_miss",
	"read_random",
	"io_congestion",
	"io_cache_hit",
	"io_block",
	"readahead",
	"readahead_hit",
	"lookahead",
	"lookahead_hit",
	"lookahead_ignore",
	"readahead_mmap",
	"readahead_eof",
	"readahead_shrink",
	"readahead_thrash",
	"readahead_mutilt",
	"readahead_rescue",
};

static const char * const ra_class_name[] = {
	"total",
	"newfile",
	"state",
	"context",
	"contexta",
	"backward",
	"onthrash",
	"onraseek",
	"none",
};

static unsigned long ra_events[RA_CLASS_END+1][RA_EVENT_END+1][2];

static inline void ra_account(struct file_ra_state *ra,
					enum ra_event e, int pages)
{
	enum ra_class c;

	if (e == RA_EVENT_READAHEAD_HIT && pages < 0) {
		c = (ra->flags >> RA_CLASS_SHIFT) & RA_CLASS_MASK;
		pages = -pages;
	} else if (ra)
		c = ra->flags & RA_CLASS_MASK;
	else
		c = RA_CLASS_END;

	if (!c)
		c = RA_CLASS_END;

	ra_events[c][e][0] += 1;
	ra_events[c][e][1] += pages;

	if (e == RA_EVENT_READAHEAD)
		ra_events[c][RA_EVENT_END][1] += pages * pages;
}

static int ra_events_show(struct seq_file *s, void *_)
{
	int i;
	int c;
	int e;
	static const char event_fmt[] = "%-16s";
	static const char class_fmt[] = "%10s";
	static const char item_fmt[] = "%10lu";
	static const char percent_format[] = "%9lu%%";
	static const char * const table_name[] = {
		"[table requests]",
		"[table pages]",
		"[table summary]"};

	for (i = 0; i <= 1; i++) {
		for (e = 0; e <= RA_EVENT_END; e++) {
			ra_events[0][e][i] = 0;
			for (c = 1; c < RA_CLASS_END; c++)
				ra_events[0][e][i] += ra_events[c][e][i];
		}

		seq_printf(s, event_fmt, table_name[i]);
		for (c = 0; c <= RA_CLASS_END; c++)
			seq_printf(s, class_fmt, ra_class_name[c]);
		seq_puts(s, "\n");

		for (e = 0; e < RA_EVENT_END; e++) {
			if (e == RA_EVENT_READAHEAD_HIT && i == 0)
				continue;
			if (e == RA_EVENT_IO_BLOCK && i == 1)
				continue;

			seq_printf(s, event_fmt, ra_event_name[e]);
			for (c = 0; c <= RA_CLASS_END; c++)
				seq_printf(s, item_fmt, ra_events[c][e][i]);
			seq_puts(s, "\n");
		}
		seq_puts(s, "\n");
	}

	seq_printf(s, event_fmt, table_name[2]);
	for (c = 0; c <= RA_CLASS_END; c++)
		seq_printf(s, class_fmt, ra_class_name[c]);
	seq_puts(s, "\n");

	seq_printf(s, event_fmt, "random_rate");
	for (c = 0; c <= RA_CLASS_END; c++)
		seq_printf(s, percent_format,
			(ra_events[c][RA_EVENT_READRANDOM][0] * 100) /
			((ra_events[c][RA_EVENT_READRANDOM][0] +
			  ra_events[c][RA_EVENT_READAHEAD][0]) | 1));
	seq_puts(s, "\n");

	seq_printf(s, event_fmt, "ra_hit_rate");
	for (c = 0; c <= RA_CLASS_END; c++)
		seq_printf(s, percent_format,
			(ra_events[c][RA_EVENT_READAHEAD_HIT][1] * 100) /
			(ra_events[c][RA_EVENT_READAHEAD][1] | 1));
	seq_puts(s, "\n");

	seq_printf(s, event_fmt, "la_hit_rate");
	for (c = 0; c <= RA_CLASS_END; c++)
		seq_printf(s, percent_format,
			(ra_events[c][RA_EVENT_LOOKAHEAD_HIT][0] * 100) /
			(ra_events[c][RA_EVENT_LOOKAHEAD][0] | 1));
	seq_puts(s, "\n");

	seq_printf(s, event_fmt, "var_ra_size");
	for (c = 0; c <= RA_CLASS_END; c++)
		seq_printf(s, item_fmt,
			(ra_events[c][RA_EVENT_END][1] -
			 ra_events[c][RA_EVENT_READAHEAD][1] *
			(ra_events[c][RA_EVENT_READAHEAD][1] /
			(ra_events[c][RA_EVENT_READAHEAD][0] | 1))) /
			(ra_events[c][RA_EVENT_READAHEAD][0] | 1));
	seq_puts(s, "\n");

	seq_printf(s, event_fmt, "avg_ra_size");
	for (c = 0; c <= RA_CLASS_END; c++)
		seq_printf(s, item_fmt,
			(ra_events[c][RA_EVENT_READAHEAD][1] +
			 ra_events[c][RA_EVENT_READAHEAD][0] / 2) /
			(ra_events[c][RA_EVENT_READAHEAD][0] | 1));
	seq_puts(s, "\n");

	seq_printf(s, event_fmt, "avg_la_size");
	for (c = 0; c <= RA_CLASS_END; c++)
		seq_printf(s, item_fmt,
			(ra_events[c][RA_EVENT_LOOKAHEAD][1] +
			 ra_events[c][RA_EVENT_LOOKAHEAD][0] / 2) /
			(ra_events[c][RA_EVENT_LOOKAHEAD][0] | 1));
	seq_puts(s, "\n");

	return 0;
}

static int ra_events_open(struct inode *inode, struct file *file)
{
	return single_open(file, ra_events_show, NULL);
}

static ssize_t ra_events_write(struct file *file, const char __user *buf,
						size_t size, loff_t *offset)
{
	memset(ra_events, 0, sizeof(ra_events));
	return 1;
}

struct file_operations ra_events_fops = {
	.owner		= THIS_MODULE,
	.open		= ra_events_open,
	.write		= ra_events_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

u32 readahead_debug_level = 0;
u32 disable_stateful_method = 0;

static int __init readahead_init(void)
{
	struct dentry *root;

	root = debugfs_create_dir("readahead", NULL);

	debugfs_create_file("events", 0644, root, NULL, &ra_events_fops);

	debugfs_create_u32("debug_level", 0644, root, &readahead_debug_level);
	debugfs_create_bool("disable_stateful_method", 0644, root,
			    &disable_stateful_method);

	return 0;
}

module_init(readahead_init)
#else
#define ra_account(ra, e, pages)	do { } while (0)
#define readahead_debug_level 		(0)
#define disable_stateful_method		(0)
#endif /* CONFIG_DEBUG_READAHEAD */

#define dprintk(args...) \
	do { if (readahead_debug_level >= 1) printk(KERN_DEBUG args); } while(0)
#define ddprintk(args...) \
	do { if (readahead_debug_level >= 2) printk(KERN_DEBUG args); } while(0)


void default_unplug_io_fn(struct backing_dev_info *bdi, struct page *page)
{
}
EXPORT_SYMBOL(default_unplug_io_fn);

struct backing_dev_info default_backing_dev_info = {
	.ra_pages	= MAX_RA_PAGES,
	.state		= 0,
	.capabilities	= BDI_CAP_MAP_COPY,
	.unplug_io_fn	= default_unplug_io_fn,
};
EXPORT_SYMBOL_GPL(default_backing_dev_info);

/*
 * Initialise a struct file's readahead state.  Assumes that the caller has
 * memset *ra to zero.
 */
void
file_ra_state_init(struct file_ra_state *ra, struct address_space *mapping)
{
	ra->ra_pages = mapping->backing_dev_info->ra_pages;
	ra->prev_page = -1;
}

/*
 * Return max readahead size for this inode in number-of-pages.
 */
static inline unsigned long get_max_readahead(struct file_ra_state *ra)
{
	return ra->ra_pages;
}

static inline unsigned long get_min_readahead(struct file_ra_state *ra)
{
	return MIN_RA_PAGES;
}

static inline void ra_off(struct file_ra_state *ra)
{
	ra->start = 0;
	ra->flags = 0;
	ra->size = 0;
	ra->ahead_start = 0;
	ra->ahead_size = 0;
	return;
}

/*
 * Set the initial window size, round to next power of 2 and square
 * for small size, x 4 for medium, and x 2 for large
 * for 128k (32 page) max ra
 * 1-8 page = 32k initial, > 8 page = 128k initial
 */
static unsigned long get_init_ra_size(unsigned long size, unsigned long max)
{
	unsigned long newsize = roundup_pow_of_two(size);

	if (newsize <= max / 64)
		newsize = newsize * newsize;
	else if (newsize <= max / 4)
		newsize = max / 4;
	else
		newsize = max;
	return newsize;
}

/*
 * Set the new window size, this is called only when I/O is to be submitted,
 * not for each call to readahead.  If a cache miss occured, reduce next I/O
 * size, else increase depending on how close to max we are.
 */
static inline unsigned long get_next_ra_size(struct file_ra_state *ra)
{
	unsigned long max = get_max_readahead(ra);
	unsigned long min = get_min_readahead(ra);
	unsigned long cur = ra->size;
	unsigned long newsize;

	if (ra->flags & RA_FLAG_MISS) {
		ra->flags &= ~RA_FLAG_MISS;
		newsize = max((cur - 2), min);
	} else if (cur < max / 16) {
		newsize = 4 * cur;
	} else {
		newsize = 2 * cur;
	}
	return min(newsize, max);
}

#define list_to_page(head) (list_entry((head)->prev, struct page, lru))

/**
 * read_cache_pages - populate an address space with some pages, and
 * 			start reads against them.
 * @mapping: the address_space
 * @pages: The address of a list_head which contains the target pages.  These
 *   pages have their ->index populated and are otherwise uninitialised.
 * @filler: callback routine for filling a single page.
 * @data: private data for the callback routine.
 *
 * Hides the details of the LRU cache etc from the filesystems.
 */
int read_cache_pages(struct address_space *mapping, struct list_head *pages,
			int (*filler)(void *, struct page *), void *data)
{
	struct page *page;
	struct pagevec lru_pvec;
	int ret = 0;

	pagevec_init(&lru_pvec, 0);

	while (!list_empty(pages)) {
		page = list_to_page(pages);
		list_del(&page->lru);
		if (add_to_page_cache(page, mapping, page->index, GFP_KERNEL)) {
			page_cache_release(page);
			continue;
		}
		ret = filler(data, page);
		if (!pagevec_add(&lru_pvec, page)) {
			cond_resched();
			__pagevec_lru_add(&lru_pvec);
		}
		if (ret) {
			while (!list_empty(pages)) {
				struct page *victim;

				victim = list_to_page(pages);
				list_del(&victim->lru);
				page_cache_release(victim);
			}
			break;
		}
	}
	pagevec_lru_add(&lru_pvec);
	return ret;
}

EXPORT_SYMBOL(read_cache_pages);

static int read_pages(struct address_space *mapping, struct file *filp,
		struct list_head *pages, unsigned nr_pages)
{
	unsigned page_idx;
	struct pagevec lru_pvec;
	int ret;

	if (mapping->a_ops->readpages) {
		ret = mapping->a_ops->readpages(filp, mapping, pages, nr_pages);
		goto out;
	}

	pagevec_init(&lru_pvec, 0);
	for (page_idx = 0; page_idx < nr_pages; page_idx++) {
		struct page *page = list_to_page(pages);
		list_del(&page->lru);
		if (!add_to_page_cache(page, mapping,
					page->index, GFP_KERNEL)) {
			ret = mapping->a_ops->readpage(filp, page);
			if (ret != AOP_TRUNCATED_PAGE) {
				if (!pagevec_add(&lru_pvec, page)) {
					cond_resched();
					__pagevec_lru_add(&lru_pvec);
				}
				continue;
			} /* else fall through to release */
		}
		page_cache_release(page);
	}
	pagevec_lru_add(&lru_pvec);
	ret = 0;
out:
	return ret;
}

/*
 * Readahead design.
 *
 * The fields in struct file_ra_state represent the most-recently-executed
 * readahead attempt:
 *
 * start:	Page index at which we started the readahead
 * size:	Number of pages in that read
 *              Together, these form the "current window".
 *              Together, start and size represent the `readahead window'.
 * prev_page:   The page which the readahead algorithm most-recently inspected.
 *              It is mainly used to detect sequential file reading.
 *              If page_cache_readahead sees that it is again being called for
 *              a page which it just looked at, it can return immediately without
 *              making any state changes.
 * ahead_start,
 * ahead_size:  Together, these form the "ahead window".
 * ra_pages:	The externally controlled max readahead for this fd.
 *
 * When readahead is in the off state (size == 0), readahead is disabled.
 * In this state, prev_page is used to detect the resumption of sequential I/O.
 *
 * The readahead code manages two windows - the "current" and the "ahead"
 * windows.  The intent is that while the application is walking the pages
 * in the current window, I/O is underway on the ahead window.  When the
 * current window is fully traversed, it is replaced by the ahead window
 * and the ahead window is invalidated.  When this copying happens, the
 * new current window's pages are probably still locked.  So
 * we submit a new batch of I/O immediately, creating a new ahead window.
 *
 * So:
 *
 *   ----|----------------|----------------|-----
 *       ^start           ^start+size
 *                        ^ahead_start     ^ahead_start+ahead_size
 *
 *         ^ When this page is read, we submit I/O for the
 *           ahead window.
 *
 * A `readahead hit' occurs when a read request is made against a page which is
 * the next sequential page. Ahead window calculations are done only when it
 * is time to submit a new IO.  The code ramps up the size agressively at first,
 * but slow down as it approaches max_readhead.
 *
 * Any seek/ramdom IO will result in readahead being turned off.  It will resume
 * at the first sequential access.
 *
 * There is a special-case: if the first page which the application tries to
 * read happens to be the first page of the file, it is assumed that a linear
 * read is about to happen and the window is immediately set to the initial size
 * based on I/O request size and the max_readahead.
 *
 * This function is to be called for every read request, rather than when
 * it is time to perform readahead.  It is called only once for the entire I/O
 * regardless of size unless readahead is unable to start enough I/O to satisfy
 * the request (I/O request > max_readahead).
 */

/*
 * do_page_cache_readahead actually reads a chunk of disk.  It allocates all
 * the pages first, then submits them all for I/O. This avoids the very bad
 * behaviour which would occur if page allocations are causing VM writeback.
 * We really don't want to intermingle reads and writes like that.
 *
 * Returns the number of pages requested, or the maximum amount of I/O allowed.
 *
 * do_page_cache_readahead() returns -1 if it encountered request queue
 * congestion.
 */
static int
__do_page_cache_readahead(struct address_space *mapping, struct file *filp,
			pgoff_t offset, unsigned long nr_to_read,
			unsigned long lookahead_size)
{
	struct inode *inode = mapping->host;
	struct page *page;
	unsigned long end_index;	/* The last page we want to read */
	LIST_HEAD(page_pool);
	int page_idx;
	int ret = 0;
	loff_t isize = i_size_read(inode);

	if (isize == 0)
		goto out;

	end_index = ((isize - 1) >> PAGE_CACHE_SHIFT);

	/*
	 * Preallocate as many pages as we will need.
	 */
	read_lock_irq(&mapping->tree_lock);
	for (page_idx = 0; page_idx < nr_to_read; page_idx++) {
		pgoff_t page_offset = offset + page_idx;
		
		if (page_offset > end_index)
			break;

		page = radix_tree_lookup(&mapping->page_tree, page_offset);
		if (page)
			continue;

		read_unlock_irq(&mapping->tree_lock);
		cond_resched();
		page = page_cache_alloc_cold(mapping);
		read_lock_irq(&mapping->tree_lock);
		if (!page)
			break;
		page->index = page_offset;
		list_add(&page->lru, &page_pool);
		if (page_idx == nr_to_read - lookahead_size)
			__SetPageReadahead(page);
		ret++;
	}
	read_unlock_irq(&mapping->tree_lock);

	/*
	 * Now start the IO.  We ignore I/O errors - if the page is not
	 * uptodate then the caller will launch readpage again, and
	 * will then handle the error.
	 */
	if (ret)
		read_pages(mapping, filp, &page_pool, ret);
	BUG_ON(!list_empty(&page_pool));
out:
	return ret;
}

/*
 * Chunk the readahead into 2 megabyte units, so that we don't pin too much
 * memory at once.
 */
int force_page_cache_readahead(struct address_space *mapping, struct file *filp,
		pgoff_t offset, unsigned long nr_to_read)
{
	int ret = 0;

	if (unlikely(!mapping->a_ops->readpage && !mapping->a_ops->readpages))
		return -EINVAL;

	while (nr_to_read) {
		int err;

		unsigned long this_chunk = (2 * 1024 * 1024) / PAGE_CACHE_SIZE;

		if (this_chunk > nr_to_read)
			this_chunk = nr_to_read;
		err = __do_page_cache_readahead(mapping, filp,
						offset, this_chunk, 0);
		if (err < 0) {
			ret = err;
			break;
		}
		ret += err;
		offset += this_chunk;
		nr_to_read -= this_chunk;
	}

	ra_account(NULL, RA_EVENT_READAHEAD, ret);

	return ret;
}

/*
 * Check how effective readahead is being.  If the amount of started IO is
 * less than expected then the file is partly or fully in pagecache and
 * readahead isn't helping.
 *
 */
static inline int check_ra_success(struct file_ra_state *ra,
			unsigned long nr_to_read, unsigned long actual)
{
	if (actual == 0) {
		ra->cache_hit += nr_to_read;
		if (ra->cache_hit >= VM_MAX_CACHE_HIT) {
			ra_off(ra);
			ra->flags |= RA_FLAG_INCACHE;
			return 0;
		}
	} else {
		ra->cache_hit=0;
	}
	return 1;
}

/*
 * This version skips the IO if the queue is read-congested, and will tell the
 * block layer to abandon the readahead if request allocation would block.
 *
 * force_page_cache_readahead() will ignore queue congestion and will block on
 * request queues.
 */
int do_page_cache_readahead(struct address_space *mapping, struct file *filp,
			pgoff_t offset, unsigned long nr_to_read)
{
	unsigned long ret;

	if (bdi_read_congested(mapping->backing_dev_info))
		return -1;

	ret = __do_page_cache_readahead(mapping, filp, offset, nr_to_read, 0);

	ra_account(NULL, RA_EVENT_READAHEAD, ret);

	return ret;
}

/*
 * Read 'nr_to_read' pages starting at page 'offset'. If the flag 'block'
 * is set wait till the read completes.  Otherwise attempt to read without
 * blocking.
 * Returns 1 meaning 'success' if read is succesfull without switching off
 * readhaead mode. Otherwise return failure.
 */
static int
blockable_page_cache_readahead(struct address_space *mapping, struct file *filp,
			pgoff_t offset, unsigned long nr_to_read,
			struct file_ra_state *ra, int block)
{
	int actual;

	if (!block && bdi_read_congested(mapping->backing_dev_info))
		return 0;

	actual = __do_page_cache_readahead(mapping, filp, offset, nr_to_read, 0);

	ra_account(NULL, RA_EVENT_READAHEAD, actual);
	dprintk("blockable-readahead(ino=%lu, ra=%lu+%lu) = %d\n",
			mapping->host->i_ino, offset, nr_to_read, actual);

	return check_ra_success(ra, nr_to_read, actual);
}

static int make_ahead_window(struct address_space *mapping, struct file *filp,
				struct file_ra_state *ra, int force)
{
	int block, ret;

	ra->ahead_size = get_next_ra_size(ra);
	ra->ahead_start = ra->start + ra->size;

	block = force || (ra->prev_page >= ra->ahead_start);
	ret = blockable_page_cache_readahead(mapping, filp,
			ra->ahead_start, ra->ahead_size, ra, block);

	if (!ret && !force) {
		/* A read failure in blocking mode, implies pages are
		 * all cached. So we can safely assume we have taken
		 * care of all the pages requested in this call.
		 * A read failure in non-blocking mode, implies we are
		 * reading more pages than requested in this call.  So
		 * we safely assume we have taken care of all the pages
		 * requested in this call.
		 *
		 * Just reset the ahead window in case we failed due to
		 * congestion.  The ahead window will any way be closed
		 * in case we failed due to excessive page cache hits.
		 */
		ra->ahead_start = 0;
		ra->ahead_size = 0;
	}

	return ret;
}

/**
 * page_cache_readahead - generic adaptive readahead
 * @mapping: address_space which holds the pagecache and I/O vectors
 * @ra: file_ra_state which holds the readahead state
 * @filp: passed on to ->readpage() and ->readpages()
 * @offset: start offset into @mapping, in PAGE_CACHE_SIZE units
 * @req_size: hint: total size of the read which the caller is performing in
 *            PAGE_CACHE_SIZE units
 *
 * page_cache_readahead() is the main function.  It performs the adaptive
 * readahead window size management and submits the readahead I/O.
 *
 * Note that @filp is purely used for passing on to the ->readpage[s]()
 * handler: it may refer to a different file from @mapping (so we may not use
 * @filp->f_mapping or @filp->f_dentry->d_inode here).
 * Also, @ra may not be equal to &@filp->f_ra.
 *
 */
unsigned long
page_cache_readahead(struct address_space *mapping, struct file_ra_state *ra,
		     struct file *filp, pgoff_t offset, unsigned long req_size)
{
	unsigned long max, newsize;
	int sequential;

	/*
	 * We avoid doing extra work and bogusly perturbing the readahead
	 * window expansion logic.
	 */
	if (offset == ra->prev_page && --req_size)
		++offset;

	/* Note that prev_page == -1 if it is a first read */
	sequential = (offset == ra->prev_page + 1);
	ra->prev_page = offset;

	max = get_max_readahead(ra);
	newsize = min(req_size, max);

	/* No readahead or sub-page sized read or file already in cache */
	if (newsize == 0 || (ra->flags & RA_FLAG_INCACHE))
		goto out;

	ra->prev_page += newsize - 1;

	/*
	 * Special case - first read at start of file. We'll assume it's
	 * a whole-file read and grow the window fast.  Or detect first
	 * sequential access
	 */
	if (sequential && ra->size == 0) {
		ra->size = get_init_ra_size(newsize, max);
		ra->start = offset;
		if (!blockable_page_cache_readahead(mapping, filp, offset,
							 ra->size, ra, 1))
			goto out;

		/*
		 * If the request size is larger than our max readahead, we
		 * at least want to be sure that we get 2 IOs in flight and
		 * we know that we will definitly need the new I/O.
		 * once we do this, subsequent calls should be able to overlap
		 * IOs,* thus preventing stalls. so issue the ahead window
		 * immediately.
		 */
		if (req_size >= max)
			make_ahead_window(mapping, filp, ra, 1);

		goto out;
	}

	/*
	 * Now handle the random case:
	 * partial page reads and first access were handled above,
	 * so this must be the next page otherwise it is random
	 */
	if (!sequential) {
		ra_off(ra);
		blockable_page_cache_readahead(mapping, filp, offset,
				 newsize, ra, 1);
		goto out;
	}

	/*
	 * If we get here we are doing sequential IO and this was not the first
	 * occurence (ie we have an existing window)
	 */

	if (ra->ahead_start == 0) {	 /* no ahead window yet */
		if (!make_ahead_window(mapping, filp, ra, 0))
			goto out;
	}
	/*
	 * Already have an ahead window, check if we crossed into it.
	 * If so, shift windows and issue a new ahead window.
	 * Only return the #pages that are in the current window, so that
	 * we get called back on the first page of the ahead window which
	 * will allow us to submit more IO.
	 */
	if (ra->prev_page >= ra->ahead_start) {
		ra->start = ra->ahead_start;
		ra->size = ra->ahead_size;
		make_ahead_window(mapping, filp, ra, 0);
	}

out:
	return ra->prev_page + 1;
}

/*
 * handle_ra_miss() is called when it is known that a page which should have
 * been present in the pagecache (we just did some readahead there) was in fact
 * not found.  This will happen if it was evicted by the VM (readahead
 * thrashing)
 *
 * Turn on the cache miss flag in the RA struct, this will cause the RA code
 * to reduce the RA size on the next read.
 */
void handle_ra_miss(struct address_space *mapping,
		struct file_ra_state *ra, pgoff_t offset)
{
	ra->flags |= RA_FLAG_MISS;
	ra->flags &= ~RA_FLAG_INCACHE;
	ra->cache_hit = 0;
}

/*
 * Given a desired number of PAGE_CACHE_SIZE readahead pages, return a
 * sensible upper limit.
 */
unsigned long max_sane_readahead(unsigned long nr)
{
	unsigned long active;
	unsigned long inactive;
	unsigned long free;

	__get_zone_counts(&active, &inactive, &free, NODE_DATA(numa_node_id()));
	return min(nr, (inactive + free) / 2);
}

/*
 * Adaptive read-ahead.
 *
 * Good read patterns are compact both in space and time. The read-ahead logic
 * tries to grant larger read-ahead size to better readers under the constraint
 * of system memory and load pressure.
 *
 * It employs two methods to estimate the max thrashing safe read-ahead size:
 *   1. state based   - the default one
 *   2. context based - the failsafe one
 * The integration of the dual methods has the merit of being agile and robust.
 * It makes the overall design clean: special cases are handled in general by
 * the stateless method, leaving the stateful one simple and fast.
 *
 * To improve throughput and decrease read delay, the logic 'looks ahead'.
 * In most read-ahead chunks, one page will be selected and tagged with
 * PG_readahead. Later when the page with PG_readahead is read, the logic
 * will be notified to submit the next read-ahead chunk in advance.
 *
 *                 a read-ahead chunk
 *    +-----------------------------------------+
 *    |       # PG_readahead                    |
 *    +-----------------------------------------+
 *            ^ When this page is read, notify me for the next read-ahead.
 *
 *
 * Here are some variable names used frequently:
 *
 *                                   |<------- la_size ------>|
 *                  +-----------------------------------------+
 *                  |                #                        |
 *                  +-----------------------------------------+
 *      ra_index -->|<---------------- ra_size -------------->|
 *
 */

#ifdef CONFIG_ADAPTIVE_READAHEAD

/*
 * The nature of read-ahead allows false tests to occur occasionally.
 * Here we just do not bother to call get_page(), it's meaningless anyway.
 */
static inline struct page *__find_page(struct address_space *mapping,
							pgoff_t offset)
{
	return radix_tree_lookup(&mapping->page_tree, offset);
}

static inline struct page *find_page(struct address_space *mapping,
							pgoff_t offset)
{
	struct page *page;

	read_lock_irq(&mapping->tree_lock);
	page = __find_page(mapping, offset);
	read_unlock_irq(&mapping->tree_lock);
	return page;
}

/*
 * Move pages in danger (of thrashing) to the head of inactive_list.
 * Not expected to happen frequently.
 */
static unsigned long rescue_pages(struct page *page, unsigned long nr_pages)
{
	int pgrescue;
	pgoff_t index;
	struct zone *zone;
	struct address_space *mapping;

	BUG_ON(!nr_pages || !page);
	pgrescue = 0;
	index = page_index(page);
	mapping = page_mapping(page);

	dprintk("rescue_pages(ino=%lu, index=%lu nr=%lu)\n",
			mapping->host->i_ino, index, nr_pages);

	for(;;) {
		zone = page_zone(page);
		spin_lock_irq(&zone->lru_lock);

		if (!PageLRU(page))
			goto out_unlock;

		while (page_mapping(page) == mapping &&
				page_index(page) == index) {
			struct page *the_page = page;
			page = next_page(page);
			if (!PageActive(the_page) &&
					!PageLocked(the_page) &&
					page_count(the_page) == 1) {
				list_move(&the_page->lru, &zone->inactive_list);
				pgrescue++;
			}
			index++;
			if (!--nr_pages)
				goto out_unlock;
		}

		spin_unlock_irq(&zone->lru_lock);

		cond_resched();
		page = find_page(mapping, index);
		if (!page)
			goto out;
	}
out_unlock:
	spin_unlock_irq(&zone->lru_lock);
out:
	ra_account(NULL, RA_EVENT_READAHEAD_RESCUE, pgrescue);
	return nr_pages;
}

/*
 * Set a new look-ahead mark at @new_index.
 * Return 0 if the new mark is successfully set.
 */
static inline int renew_lookahead(struct address_space *mapping,
				struct file_ra_state *ra,
				pgoff_t index, pgoff_t new_index)
{
	struct page *page;

	if (index == ra->lookahead_index &&
			new_index >= ra->readahead_index)
		return 1;

	page = find_page(mapping, new_index);
	if (!page)
		return 1;

	__SetPageReadahead(page);
	if (ra->lookahead_index == index)
		ra->lookahead_index = new_index;

	return 0;
}

/*
 * State based calculation of read-ahead request.
 *
 * This figure shows the meaning of file_ra_state members:
 *
 *             chunk A                            chunk B
 *  +---------------------------+-------------------------------------------+
 *  |             #             |                   #                       |
 *  +---------------------------+-------------------------------------------+
 *                ^             ^                   ^                       ^
 *              la_index      ra_index     lookahead_index         readahead_index
 */

/*
 * The node's effective length of inactive_list(s).
 */
static unsigned long node_free_and_cold_pages(void)
{
	unsigned int i;
	unsigned long sum = 0;
	struct zone *zones = NODE_DATA(numa_node_id())->node_zones;

	for (i = 0; i < MAX_NR_ZONES; i++)
		sum += zones[i].nr_inactive +
			zones[i].free_pages - zones[i].pages_low;

	return sum;
}

/*
 * The node's accumulated aging activities.
 */
static unsigned long node_readahead_aging(void)
{
	unsigned long cpu;
	unsigned long sum = 0;
	cpumask_t mask = node_to_cpumask(numa_node_id());

	for_each_cpu_mask(cpu, mask)
		sum += per_cpu(readahead_aging, cpu);

	return sum;
}

/*
 * The 64bit cache_hits stores three accumulated values and a counter value.
 * MSB                                                                   LSB
 * 3333333333333333 : 2222222222222222 : 1111111111111111 : 0000000000000000
 */
static inline int ra_cache_hit(struct file_ra_state *ra, int nr)
{
	return (ra->cache_hits >> (nr * 16)) & 0xFFFF;
}

/*
 * Conceptual code:
 * ra_cache_hit(ra, 1) += ra_cache_hit(ra, 0);
 * ra_cache_hit(ra, 0) = 0;
 */
static inline void ra_addup_cache_hit(struct file_ra_state *ra)
{
	int n;

	n = ra_cache_hit(ra, 0);
	ra->cache_hits -= n;
	n <<= 16;
	ra->cache_hits += n;
}

/*
 * The read-ahead is deemed success if cache-hit-rate >= 1/readahead_hit_rate.
 */
static inline int ra_cache_hit_ok(struct file_ra_state *ra)
{
	return ra_cache_hit(ra, 0) * readahead_hit_rate >=
					(ra->lookahead_index - ra->la_index);
}

/*
 * Check if @index falls in the @ra request.
 */
static inline int ra_has_index(struct file_ra_state *ra, pgoff_t index)
{
	if (index < ra->la_index || index >= ra->readahead_index)
		return 0;

	if (index >= ra->ra_index)
		return 1;
	else
		return -1;
}

/*
 * Which method is issuing this read-ahead?
 */
static inline void ra_set_class(struct file_ra_state *ra,
				enum ra_class ra_class)
{
	unsigned long flags_mask;
	unsigned long flags;
	unsigned long old_ra_class;

	flags_mask = ~(RA_CLASS_MASK | (RA_CLASS_MASK << RA_CLASS_SHIFT));
	flags = ra->flags & flags_mask;

	old_ra_class = (ra->flags & RA_CLASS_MASK) << RA_CLASS_SHIFT;

	ra->flags = flags | old_ra_class | ra_class;

	ra_addup_cache_hit(ra);
	if (ra_class != RA_CLASS_STATE)
		ra->cache_hits <<= 16;

	ra->age = node_readahead_aging();
}

/*
 * Where is the old read-ahead and look-ahead?
 */
static inline void ra_set_index(struct file_ra_state *ra,
				pgoff_t la_index, pgoff_t ra_index)
{
	ra->la_index = la_index;
	ra->ra_index = ra_index;
}

/*
 * Where is the new read-ahead and look-ahead?
 */
static inline void ra_set_size(struct file_ra_state *ra,
				unsigned long ra_size, unsigned long la_size)
{
	/* Disable look-ahead for loopback file. */
	if (unlikely(ra->flags & RA_FLAG_NO_LOOKAHEAD))
		la_size = 0;

	ra->readahead_index = ra->ra_index + ra_size;
	ra->lookahead_index = ra->readahead_index - la_size;
}

/*
 * Submit IO for the read-ahead request in file_ra_state.
 */
static int ra_dispatch(struct file_ra_state *ra,
			struct address_space *mapping, struct file *filp)
{
	pgoff_t eof_index;
	unsigned long ra_size;
	unsigned long la_size;
	int actual;
	enum ra_class ra_class;

	ra_class = (ra->flags & RA_CLASS_MASK);
	BUG_ON(ra_class == 0 || ra_class > RA_CLASS_END);

	eof_index = ((i_size_read(mapping->host) - 1) >> PAGE_CACHE_SHIFT) + 1;
	ra_size = ra->readahead_index - ra->ra_index;
	la_size = ra->readahead_index - ra->lookahead_index;

	/* Snap to EOF. */
	if (unlikely(ra->ra_index >= eof_index))
		return 0;
	if (ra->readahead_index + ra_size / 2 > eof_index) {
		if (ra_class == RA_CLASS_CONTEXT_AGGRESSIVE &&
				eof_index > ra->lookahead_index + 1)
			la_size = eof_index - ra->lookahead_index;
		else
			la_size = 0;
		ra_size = eof_index - ra->ra_index;
		ra_set_size(ra, ra_size, la_size);
	}

	actual = __do_page_cache_readahead(mapping, filp,
					ra->ra_index, ra_size, la_size);

#ifdef CONFIG_DEBUG_READAHEAD
	if (ra->flags & RA_FLAG_MMAP)
		ra_account(ra, RA_EVENT_READAHEAD_MMAP, actual);
	if (ra->readahead_index == eof_index)
		ra_account(ra, RA_EVENT_READAHEAD_EOF, actual);
	if (la_size)
		ra_account(ra, RA_EVENT_LOOKAHEAD, la_size);
	if (ra_size > actual)
		ra_account(ra, RA_EVENT_IO_CACHE_HIT, ra_size - actual);
	ra_account(ra, RA_EVENT_READAHEAD, actual);

	if (!ra->ra_index && filp->f_dentry->d_inode) {
		char *fn;
		static char path[1024];
		unsigned long size;

		size = (i_size_read(filp->f_dentry->d_inode)+1023)/1024;
		fn = d_path(filp->f_dentry, filp->f_vfsmnt, path, 1000);
		if (!IS_ERR(fn))
			ddprintk("ino %lu is %s size %luK by %s(%d)\n",
					filp->f_dentry->d_inode->i_ino,
					fn, size,
					current->comm, current->pid);
	}

	dprintk("readahead-%s(ino=%lu, index=%lu, ra=%lu+%lu-%lu) = %d\n",
			ra_class_name[ra_class],
			mapping->host->i_ino, ra->la_index,
			ra->ra_index, ra_size, la_size, actual);
#endif /* CONFIG_DEBUG_READAHEAD */

	return actual;
}

/*
 * Determine the ra request from primitive values.
 *
 * It applies the following rules:
 *   - Substract ra_size by the old look-ahead to get real safe read-ahead;
 *   - Set new la_size according to the (still large) ra_size;
 *   - Apply upper limits;
 *   - Make sure stream_shift is not too small.
 *     (So that the next global_shift will not be too small.)
 *
 * Input:
 * ra_size stores the estimated thrashing-threshold.
 * la_size stores the look-ahead size of previous request.
 */
static inline int adjust_rala(unsigned long ra_max,
				unsigned long *ra_size, unsigned long *la_size)
{
	unsigned long stream_shift = *la_size;

	if (*ra_size > *la_size)
		*ra_size -= *la_size;
	else {
		ra_account(NULL, RA_EVENT_READAHEAD_SHRINK, *ra_size);
		return 0;
	}

	*la_size = *ra_size / LOOKAHEAD_RATIO;

	if (*ra_size > ra_max)
		*ra_size = ra_max;
	if (*la_size > *ra_size)
		*la_size = *ra_size;

	stream_shift += (*ra_size - *la_size);
	if (stream_shift < *ra_size / 4)
		*la_size -= (*ra_size / 4 - stream_shift);

	return 1;
}

/*
 * The function estimates two values:
 * 1. thrashing-threshold for the current stream
 *    It is returned to make the next read-ahead request.
 * 2. the remained safe space for the current chunk
 *    It will be checked to ensure that the current chunk is safe.
 *
 * The computation will be pretty accurate under heavy load, and will vibrate
 * more on light load(with small global_shift), so the grow speed of ra_size
 * must be limited, and a moderate large stream_shift must be insured.
 *
 * This figure illustrates the formula used in the function:
 * While the stream reads stream_shift pages inside the chunks,
 * the chunks are shifted global_shift pages inside inactive_list.
 *
 *      chunk A                    chunk B
 *                          |<=============== global_shift ================|
 *  +-------------+         +-------------------+                          |
 *  |       #     |         |           #       |            inactive_list |
 *  +-------------+         +-------------------+                     head |
 *          |---->|         |---------->|
 *             |                  |
 *             +-- stream_shift --+
 */
static inline unsigned long compute_thrashing_threshold(
						struct file_ra_state *ra,
						unsigned long *remain)
{
	unsigned long global_size;
	unsigned long global_shift;
	unsigned long stream_shift;
	unsigned long ra_size;
	uint64_t ll;

	global_size = node_free_and_cold_pages();
	global_shift = node_readahead_aging() - ra->age;
	global_shift |= 1UL;
	stream_shift = ra_cache_hit(ra, 0);

	ll = (uint64_t) stream_shift * (global_size >> 9) * readahead_ratio * 5;
	do_div(ll, global_shift);
	ra_size = ll;

	if (global_size > global_shift) {
		ll = (uint64_t) stream_shift * (global_size - global_shift);
		do_div(ll, global_shift);
		*remain = ll;
	} else
		*remain = 0;

	ddprintk("compute_thrashing_threshold: "
			"at %lu ra %lu=%lu*%lu/%lu, remain %lu for %lu\n",
			ra->readahead_index, ra_size,
			stream_shift, global_size, global_shift,
			*remain, ra->readahead_index - ra->lookahead_index);

	return ra_size;
}

/*
 * Main function for file_ra_state based read-ahead.
 */
static inline unsigned long
state_based_readahead(struct address_space *mapping, struct file *filp,
			struct file_ra_state *ra,
			struct page *page, pgoff_t index,
			unsigned long ra_size, unsigned long ra_max)
{
	unsigned long ra_old;
	unsigned long la_size;
	unsigned long remain_space;
	unsigned long growth_limit;

	la_size = ra->readahead_index - index;
	ra_old = ra->readahead_index - ra->ra_index;
	growth_limit = ra_size + ra_max / 16 +
				(2 + readahead_ratio / 64) * ra_old;
	ra_size = compute_thrashing_threshold(ra, &remain_space);

	if (page && remain_space <= la_size && la_size > 1) {
		rescue_pages(page, la_size);
		return 0;
	}

	if (!adjust_rala(min(ra_max, growth_limit), &ra_size, &la_size))
		return 0;

	ra_set_class(ra, RA_CLASS_STATE);
	ra_set_index(ra, index, ra->readahead_index);
	ra_set_size(ra, ra_size, la_size);

	return ra_dispatch(ra, mapping, filp);
}

/*
 * Page cache context based estimation of read-ahead/look-ahead size/index.
 *
 * The logic first looks around to find the start point of next read-ahead,
 * and then, if necessary, looks backward in the inactive_list to get an
 * estimation of the thrashing-threshold.
 *
 * The estimation theory can be illustrated with figure:
 *
 *   chunk A           chunk B                      chunk C                 head
 *
 *   l01 l11           l12   l21                    l22
 *| |-->|-->|       |------>|-->|                |------>|
 *| +-------+       +-----------+                +-------------+               |
 *| |   #   |       |       #   |                |       #     |               |
 *| +-------+       +-----------+                +-------------+               |
 *| |<==============|<===========================|<============================|
 *        L0                     L1                            L2
 *
 * Let f(l) = L be a map from
 * 	l: the number of pages read by the stream
 * to
 * 	L: the number of pages pushed into inactive_list in the mean time
 * then
 * 	f(l01) <= L0
 * 	f(l11 + l12) = L1
 * 	f(l21 + l22) = L2
 * 	...
 * 	f(l01 + l11 + ...) <= Sum(L0 + L1 + ...)
 *			   <= Length(inactive_list) = f(thrashing-threshold)
 *
 * So the count of countinuous history pages left in the inactive_list is always
 * a lower estimation of the true thrashing-threshold.
 */

#define PAGE_REFCNT_0           0
#define PAGE_REFCNT_1           (1 << PG_referenced)
#define PAGE_REFCNT_2           (1 << PG_active)
#define PAGE_REFCNT_3           ((1 << PG_active) | (1 << PG_referenced))
#define PAGE_REFCNT_MASK        PAGE_REFCNT_3

/*
 * STATUS   REFERENCE COUNT
 *  __                   0
 *  _R       PAGE_REFCNT_1
 *  A_       PAGE_REFCNT_2
 *  AR       PAGE_REFCNT_3
 *
 *  A/R: Active / Referenced
 */
static inline unsigned long page_refcnt(struct page *page)
{
        return page->flags & PAGE_REFCNT_MASK;
}

/*
 * STATUS   REFERENCE COUNT      TYPE
 *  __                   0      fresh
 *  _R       PAGE_REFCNT_1      stale
 *  A_       PAGE_REFCNT_2      disturbed once
 *  AR       PAGE_REFCNT_3      disturbed twice
 *
 *  A/R: Active / Referenced
 */
static inline unsigned long cold_page_refcnt(struct page *page)
{
	if (!page || PageActive(page))
		return 0;

	return page_refcnt(page);
}

static inline char page_refcnt_symbol(struct page *page)
{
	if (!page)
		return 'X';

	switch (page_refcnt(page)) {
		case 0:
			return '_';
		case PAGE_REFCNT_1:
			return '-';
		case PAGE_REFCNT_2:
			return '=';
		case PAGE_REFCNT_3:
			return '#';
		default:
			return '?';
	}
}

/*
 * Count/estimate cache hits in range [first_index, last_index].
 * The estimation is simple and optimistic.
 */
static int count_cache_hit(struct address_space *mapping,
				pgoff_t first_index, pgoff_t last_index)
{
	struct page *page;
	int size = last_index - first_index + 1;
	int count = 0;
	int i;

	cond_resched();
	read_lock_irq(&mapping->tree_lock);

	/*
	 * The first page may well is chunk head and has been accessed,
	 * so it is index 0 that makes the estimation optimistic. This
	 * behavior guarantees a readahead when (size < ra_max) and
	 * (readahead_hit_rate >= 16).
	 */
	for (i = 0; i < 16;) {
		page = __find_page(mapping, first_index +
						size * ((i++ * 29) & 15) / 16);
		if (cold_page_refcnt(page) >= PAGE_REFCNT_1 && ++count >= 2)
			break;
	}

	read_unlock_irq(&mapping->tree_lock);

	return size * count / i;
}

/*
 * Look back and check history pages to estimate thrashing-threshold.
 */
static unsigned long query_page_cache_segment(struct address_space *mapping,
				struct file_ra_state *ra,
				unsigned long *remain, pgoff_t offset,
				unsigned long ra_min, unsigned long ra_max)
{
	pgoff_t index;
	unsigned long count;
	unsigned long nr_lookback;
	struct radix_tree_cache cache;

	/*
	 * Scan backward and check the near @ra_max pages.
	 * The count here determines ra_size.
	 */
	cond_resched();
	read_lock_irq(&mapping->tree_lock);
	index = radix_tree_scan_hole_backward(&mapping->page_tree,
							offset, ra_max);
#ifdef DEBUG_READAHEAD_RADIXTREE
	WARN_ON(index > offset);
	if (index != offset)
		WARN_ON(!__find_page(mapping, index + 1));
	if (index && offset - index < ra_max)
		WARN_ON(__find_page(mapping, index));
#endif
	read_unlock_irq(&mapping->tree_lock);

	*remain = offset - index;

	if (offset == ra->readahead_index && ra_cache_hit_ok(ra))
		count = *remain;
	else if (count_cache_hit(mapping, index + 1, offset) *
						readahead_hit_rate >= *remain)
		count = *remain;
	else
		count = ra_min;

	/*
	 * Unnecessary to count more?
	 */
	if (count < ra_max)
		goto out;

	if (unlikely(ra->flags & RA_FLAG_NO_LOOKAHEAD))
		goto out;

	/*
	 * Check the far pages coarsely.
	 * The big count here helps increase la_size.
	 */
	nr_lookback = ra_max * (LOOKAHEAD_RATIO + 1) *
						100 / (readahead_ratio + 1);

	cond_resched();
	radix_tree_cache_init(&cache);
	read_lock_irq(&mapping->tree_lock);
	for (count += ra_max; count < nr_lookback; count += ra_max) {
		struct radix_tree_node *node;
		node = radix_tree_cache_lookup_node(&mapping->page_tree,
						&cache, offset - count, 1);
#ifdef DEBUG_READAHEAD_RADIXTREE
		if (node != radix_tree_lookup_node(&mapping->page_tree,
							offset - count, 1))
			BUG();
#endif
		if (!node)
			break;
	}
	read_unlock_irq(&mapping->tree_lock);

out:
	/*
	 *  For sequential read that extends from index 0, the counted value
	 *  may well be far under the true threshold, so return it unmodified
	 *  for further process in adjust_rala_aggressive().
	 */
	if (count >= offset)
		count = offset;
	else
		count = max(ra_min, count * readahead_ratio / 100);

	ddprintk("query_page_cache_segment: "
			"ino=%lu, idx=%lu, count=%lu, remain=%lu\n",
			mapping->host->i_ino, offset, count, *remain);

	return count;
}

/*
 * Find past-the-end index of the segment before @index.
 */
static inline pgoff_t find_segtail_backward(struct address_space *mapping,
					pgoff_t index, unsigned long max_scan)
{
	struct radix_tree_cache cache;
	struct page *page;
	pgoff_t origin;

	origin = index;
	if (max_scan > index)
		max_scan = index;

	cond_resched();
	radix_tree_cache_init(&cache);
	read_lock_irq(&mapping->tree_lock);
	for (; origin - index < max_scan;) {
		page = radix_tree_cache_lookup(&mapping->page_tree,
							&cache, --index);
		if (page) {
			read_unlock_irq(&mapping->tree_lock);
			return index + 1;
		}
	}
	read_unlock_irq(&mapping->tree_lock);

	return 0;
}

/*
 * Find past-the-end index of the segment at @index.
 */
static inline pgoff_t find_segtail(struct address_space *mapping,
					pgoff_t index, unsigned long max_scan)
{
	pgoff_t ra_index;

	cond_resched();
	read_lock_irq(&mapping->tree_lock);
	ra_index = radix_tree_scan_hole(&mapping->page_tree, index, max_scan);
#ifdef DEBUG_READAHEAD_RADIXTREE
	BUG_ON(!__find_page(mapping, index));
	WARN_ON(ra_index < index);
	if (ra_index != index && !__find_page(mapping, ra_index - 1))
		printk(KERN_ERR "radix_tree_scan_hole(index=%lu ra_index=%lu "
				"max_scan=%lu nrpages=%lu) fooled!\n",
				index, ra_index, max_scan, mapping->nrpages);
	if (ra_index != ~0UL && ra_index - index < max_scan)
		WARN_ON(__find_page(mapping, ra_index));
#endif
	read_unlock_irq(&mapping->tree_lock);

	if (ra_index <= index + max_scan)
		return ra_index;
	else
		return 0;
}

/*
 * Determine the request parameters for context based read-ahead that extends
 * from start of file.
 *
 * The major weakness of stateless method is perhaps the slow grow up speed of
 * ra_size. The logic tries to make up for this in the important case of
 * sequential reads that extend from start of file. In this case, the ra_size
 * is not chosen to make the whole next chunk safe (as in normal ones). Only
 * half of which is safe. The added 'unsafe' half is the look-ahead part. It
 * is expected to be safeguarded by rescue_pages() when the previous chunks are
 * lost.
 */
static inline int adjust_rala_aggressive(unsigned long ra_max,
				unsigned long *ra_size, unsigned long *la_size)
{
	pgoff_t index = *ra_size;

	*ra_size -= min(*ra_size, *la_size);
	*ra_size = *ra_size * readahead_ratio / 100;
	*la_size = index * readahead_ratio / 100;
	*ra_size += *la_size;

	if (*ra_size > ra_max)
		*ra_size = ra_max;
	if (*la_size > *ra_size)
		*la_size = *ra_size;

	return 1;
}

/*
 * Main function for page context based read-ahead.
 */
static inline int
try_context_based_readahead(struct address_space *mapping,
			struct file_ra_state *ra, struct page *prev_page,
			struct page *page, pgoff_t index,
			unsigned long ra_min, unsigned long ra_max)
{
	pgoff_t ra_index;
	unsigned long ra_size;
	unsigned long la_size;
	unsigned long remain_pages;

	/* Where to start read-ahead?
	 * NFSv3 daemons may process adjacent requests in parallel,
	 * leading to many locally disordered, globally sequential reads.
	 * So do not require nearby history pages to be present or accessed.
	 */
	if (page) {
		ra_index = find_segtail(mapping, index, ra_max * 5 / 4);
		if (!ra_index)
			return -1;
	} else if (prev_page || find_page(mapping, index - 1)) {
		ra_index = index;
	} else if (readahead_hit_rate > 1) {
		ra_index = find_segtail_backward(mapping, index,
						readahead_hit_rate + ra_min);
		if (!ra_index)
			return 0;
		ra_min += 2 * (index - ra_index);
		index = ra_index;	/* pretend the request starts here */
	} else
		return 0;

	ra_size = query_page_cache_segment(mapping, ra, &remain_pages,
							index, ra_min, ra_max);

	la_size = ra_index - index;
	if (page && remain_pages <= la_size &&
			remain_pages < index && la_size > 1) {
		rescue_pages(page, la_size);
		return -1;
	}

	if (ra_size == index) {
		if (!adjust_rala_aggressive(ra_max, &ra_size, &la_size))
			return -1;
		ra_set_class(ra, RA_CLASS_CONTEXT_AGGRESSIVE);
	} else {
		if (!adjust_rala(ra_max, &ra_size, &la_size))
			return -1;
		ra_set_class(ra, RA_CLASS_CONTEXT);
	}

	ra_set_index(ra, index, ra_index);
	ra_set_size(ra, ra_size, la_size);

	return 1;
}

/*
 * Read-ahead on start of file.
 *
 * The strategies here are most important for small files.
 * 1. Set a moderately large read-ahead size;
 * 2. Issue the next read-ahead request as soon as possible.
 *
 * But be careful, there are some applications that dip into only the very head
 * of a file. The most important thing is to prevent them from triggering the
 * next (much larger) read-ahead request, which leads to lots of cache misses.
 * Two pages should be enough for them, correct me if I'm wrong.
 */
static inline unsigned long
newfile_readahead(struct address_space *mapping,
		struct file *filp, struct file_ra_state *ra,
		unsigned long req_size, unsigned long ra_min)
{
	unsigned long ra_size;
	unsigned long la_size;

	if (req_size > ra_min) /* larger value risks thrashing */
		req_size = ra_min;

	if (unlikely(ra->flags & RA_FLAG_NFSD)) {
		ra_size = MIN_NFSD_PAGES;
		la_size = 0;
	} else {
		ra_size = 4 * req_size;
		la_size = 2 * req_size;
	}

	ra_set_class(ra, RA_CLASS_NEWFILE);
	ra_set_index(ra, 0, 0);
	ra_set_size(ra, ra_size, la_size);

	return ra_dispatch(ra, mapping, filp);
}

/*
 * Backward prefetching.
 * No look ahead and thrashing threshold estimation for stepping backward
 * pattern: should be unnecessary.
 */
static inline int
try_read_backward(struct file_ra_state *ra, pgoff_t begin_index,
			unsigned long ra_size, unsigned long ra_max)
{
	pgoff_t end_index;

	/* Are we reading backward? */
	if (begin_index > ra->prev_page)
		return 0;

	if ((ra->flags & RA_CLASS_MASK) == RA_CLASS_BACKWARD &&
					ra_has_index(ra, ra->prev_page)) {
		ra_size += 2 * ra_cache_hit(ra, 0);
		end_index = ra->la_index;
	} else {
		ra_size += ra_size + ra_size * (readahead_hit_rate - 1) / 2;
		end_index = ra->prev_page;
	}

	if (ra_size > ra_max)
		ra_size = ra_max;

	/* Read traces close enough to be covered by the prefetching? */
	if (end_index > begin_index + ra_size)
		return 0;

	begin_index = end_index - ra_size;

	ra_set_class(ra, RA_CLASS_BACKWARD);
	ra_set_index(ra, begin_index, begin_index);
	ra_set_size(ra, ra_size, 0);

	return 1;
}

/*
 * Readahead thrashing recovery.
 */
static inline unsigned long
thrashing_recovery_readahead(struct address_space *mapping,
				struct file *filp, struct file_ra_state *ra,
				pgoff_t index, unsigned long ra_max)
{
	unsigned long ra_size;

	if (readahead_debug_level && find_page(mapping, index - 1))
		ra_account(ra, RA_EVENT_READAHEAD_MUTILATE,
						ra->readahead_index - index);
	ra_account(ra, RA_EVENT_READAHEAD_THRASHING,
						ra->readahead_index - index);

	/*
	 * Some thrashing occur in (ra_index, la_index], in which case the
	 * old read-ahead chunk is lost soon after the new one is allocated.
	 * Ensure that we recover all needed pages in the old chunk.
	 */
	if (index < ra->ra_index)
		ra_size = ra->ra_index - index;
	else {
		/* After thrashing, we know the exact thrashing-threshold. */
		ra_size = ra_cache_hit(ra, 0);

		/* And we'd better be a bit conservative. */
		ra_size = ra_size * 3 / 4;
	}

	if (ra_size > ra_max)
		ra_size = ra_max;

	ra_set_class(ra, RA_CLASS_THRASHING);
	ra_set_index(ra, index, index);
	ra_set_size(ra, ra_size, ra_size / LOOKAHEAD_RATIO);

	return ra_dispatch(ra, mapping, filp);
}

/*
 * If there is a previous sequential read, it is likely to be another
 * sequential read at the new position.
 * Databases are known to have this seek-and-read-one-block pattern.
 */
static inline int
try_readahead_on_seek(struct file_ra_state *ra, pgoff_t index,
			unsigned long ra_size, unsigned long ra_max)
{
	unsigned long hit0 = ra_cache_hit(ra, 0);
	unsigned long hit1 = ra_cache_hit(ra, 1) + hit0;
	unsigned long hit2 = ra_cache_hit(ra, 2);
	unsigned long hit3 = ra_cache_hit(ra, 3);

	/* There's a previous read-ahead request? */
	if (!ra_has_index(ra, ra->prev_page))
		return 0;

	/* The previous read-ahead sequences have similiar sizes? */
	if (!(ra_size < hit1 && hit1 > hit2 / 2 &&
				hit2 > hit3 / 2 &&
				hit3 > hit1 / 2))
		return 0;

	hit1 = max(hit1, hit2);

	/* Follow the same prefetching direction. */
	if ((ra->flags & RA_CLASS_MASK) == RA_CLASS_BACKWARD)
		index = ((index > hit1 - ra_size) ? index - hit1 + ra_size : 0);

	ra_size = min(hit1, ra_max);

	ra_set_class(ra, RA_CLASS_SEEK);
	ra_set_index(ra, index, index);
	ra_set_size(ra, ra_size, 0);

	return 1;
}

/*
 * ra_min is mainly determined by the size of cache memory.
 * Table of concrete numbers for 4KB page size:
 *   inactive + free (MB):    4   8   16   32   64  128  256  512 1024
 *            ra_min (KB):   16  16   16   16   20   24   32   48   64
 */
static inline void get_readahead_bounds(struct file_ra_state *ra,
					unsigned long *ra_min,
					unsigned long *ra_max)
{
	unsigned long pages;

	pages = max_sane_readahead(KB(1024*1024));
	*ra_max = min(min(pages, 0xFFFFUL), ra->ra_pages);
	*ra_min = min(min(MIN_RA_PAGES + (pages>>13), KB(128)), *ra_max/2);
}

/**
 * page_cache_readahead_adaptive - adaptive read-ahead main function
 * @mapping, @ra, @filp: the same as page_cache_readahead()
 * @prev_page: the page at @index-1, may be NULL to let the function find it
 * @page: the page at @index, or NULL if non-present
 * @begin_index, @index, @end_index: offsets into @mapping
 * 		[@begin_index, @end_index) is the read the caller is performing
 *	 	@index indicates the page to be read now
 *
 * page_cache_readahead_adaptive() is the entry point of the adaptive
 * read-ahead logic. It tries a set of methods in turn to determine the
 * appropriate readahead action and submits the readahead I/O.
 *
 * The caller is expected to point ra->prev_page to the previously accessed
 * page, and to call it on two conditions:
 * 1. @page == NULL
 *    A cache miss happened, some pages have to be read in
 * 2. @page != NULL && PageReadahead(@page)
 *    A look-ahead mark encountered, this is set by a previous read-ahead
 *    invocation to instruct the caller to give the function a chance to
 *    check up and do next read-ahead in advance.
 */
unsigned long
page_cache_readahead_adaptive(struct address_space *mapping,
			struct file_ra_state *ra, struct file *filp,
			struct page *prev_page, struct page *page,
			pgoff_t begin_index, pgoff_t index, pgoff_t end_index)
{
	unsigned long size;
	unsigned long ra_min;
	unsigned long ra_max;
	int ret;

	might_sleep();

	if (page) {
		if(!TestClearPageReadahead(page))
			return 0;
		if (bdi_read_congested(mapping->backing_dev_info)) {
			ra_account(ra, RA_EVENT_IO_CONGESTION,
							end_index - index);
			return 0;
		}
		if (laptop_mode && laptop_spinned_down()) {
			if (!renew_lookahead(mapping, ra, index,
						index + LAPTOP_POLL_INTERVAL))
				return 0;
		}
	}

	if (page)
		ra_account(ra, RA_EVENT_LOOKAHEAD_HIT,
				ra->readahead_index - ra->lookahead_index);
	else if (index)
		ra_account(ra, RA_EVENT_CACHE_MISS, end_index - begin_index);

	size = end_index - index;
	get_readahead_bounds(ra, &ra_min, &ra_max);

	/* readahead disabled? */
	if (unlikely(!ra_max || !readahead_ratio)) {
		size = max_sane_readahead(size);
		goto readit;
	}

	/*
	 * Start of file.
	 */
	if (index == 0)
		return newfile_readahead(mapping, filp, ra, end_index, ra_min);

	/*
	 * State based sequential read-ahead.
	 */
	if (!disable_stateful_method &&
			index == ra->lookahead_index && ra_cache_hit_ok(ra))
		return state_based_readahead(mapping, filp, ra, page,
							index, size, ra_max);

	/*
	 * Recover from possible thrashing.
	 */
	if (!page && index == ra->prev_page + 1 && ra_has_index(ra, index))
		return thrashing_recovery_readahead(mapping, filp, ra,
								index, ra_max);

	/*
	 * Backward read-ahead.
	 */
	if (!page && begin_index == index &&
				try_read_backward(ra, index, size, ra_max))
		return ra_dispatch(ra, mapping, filp);

	/*
	 * Context based sequential read-ahead.
	 */
	ret = try_context_based_readahead(mapping, ra, prev_page, page,
							index, ra_min, ra_max);
	if (ret > 0)
		return ra_dispatch(ra, mapping, filp);
	if (ret < 0)
		return 0;

	/* No action on look ahead time? */
	if (page) {
		ra_account(ra, RA_EVENT_LOOKAHEAD_NOACTION,
						ra->readahead_index - index);
		return 0;
	}

	/*
	 * Random read that follows a sequential one.
	 */
	if (try_readahead_on_seek(ra, index, size, ra_max))
		return ra_dispatch(ra, mapping, filp);

	/*
	 * Random read.
	 */
	if (size > ra_max)
		size = ra_max;

readit:
	size = __do_page_cache_readahead(mapping, filp, index, size, 0);

	ra_account(ra, RA_EVENT_READRANDOM, size);
	dprintk("readrandom(ino=%lu, pages=%lu, index=%lu-%lu-%lu) = %lu\n",
			mapping->host->i_ino, mapping->nrpages,
			begin_index, index, end_index, size);

	return size;
}

/**
 * readahead_cache_hit - adaptive read-ahead feedback function
 * @ra: file_ra_state which holds the readahead state
 * @page: the page just accessed
 *
 * readahead_cache_hit() is the feedback route of the adaptive read-ahead
 * logic. It must be called on every access on the read-ahead pages.
 */
void fastcall readahead_cache_hit(struct file_ra_state *ra, struct page *page)
{
	if (PageActive(page) || PageReferenced(page))
		return;

	if (!PageUptodate(page))
		ra_account(ra, RA_EVENT_IO_BLOCK, 1);

	if (!ra_has_index(ra, page->index))
		return;

	ra->cache_hits++;

	if (page->index >= ra->ra_index)
		ra_account(ra, RA_EVENT_READAHEAD_HIT, 1);
	else
		ra_account(ra, RA_EVENT_READAHEAD_HIT, -1);
}

#endif /* CONFIG_ADAPTIVE_READAHEAD */
