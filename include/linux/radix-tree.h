/*
 * Copyright (C) 2001 Momchil Velikov
 * Portions Copyright (C) 2001 Christoph Hellwig
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef _LINUX_RADIX_TREE_H
#define _LINUX_RADIX_TREE_H

#include <linux/sched.h>
#include <linux/preempt.h>
#include <linux/types.h>

#define RADIX_TREE_MAP_SHIFT	6
#define RADIX_TREE_MAP_SIZE	(1UL << RADIX_TREE_MAP_SHIFT)
#define RADIX_TREE_MAP_MASK	(RADIX_TREE_MAP_SIZE-1)

struct radix_tree_root {
	unsigned int		height;
	gfp_t			gfp_mask;
	struct radix_tree_node	*rnode;
};

/*
 * Lookaside cache to support access patterns with strong locality.
 */
struct radix_tree_cache {
	unsigned long first_index;
	struct radix_tree_node *tree_node;
};

#define RADIX_TREE_INIT(mask)	{					\
	.height = 0,							\
	.gfp_mask = (mask),						\
	.rnode = NULL,							\
}

#define RADIX_TREE(name, mask) \
	struct radix_tree_root name = RADIX_TREE_INIT(mask)

#define INIT_RADIX_TREE(root, mask)					\
do {									\
	(root)->height = 0;						\
	(root)->gfp_mask = (mask);					\
	(root)->rnode = NULL;						\
} while (0)

int radix_tree_insert(struct radix_tree_root *, unsigned long, void *);
void *radix_tree_lookup_node(struct radix_tree_root *, unsigned long,
							unsigned int);
void **radix_tree_lookup_slot(struct radix_tree_root *root, unsigned long);
void *radix_tree_delete(struct radix_tree_root *, unsigned long);
unsigned int radix_tree_cache_count(struct radix_tree_cache *cache);
void *radix_tree_cache_lookup_node(struct radix_tree_root *root,
				struct radix_tree_cache *cache,
				unsigned long index, unsigned int level);
unsigned long radix_tree_scan_hole_backward(struct radix_tree_root *root,
				unsigned long index, unsigned long max_scan);
unsigned long radix_tree_scan_hole(struct radix_tree_root *root,
				unsigned long index, unsigned long max_scan);
unsigned int
radix_tree_gang_lookup(struct radix_tree_root *root, void **results,
			unsigned long first_index, unsigned int max_items);
int radix_tree_preload(gfp_t gfp_mask);
void radix_tree_init(void);
void *radix_tree_tag_set(struct radix_tree_root *root,
			unsigned long index, int tag);
void *radix_tree_tag_clear(struct radix_tree_root *root,
			unsigned long index, int tag);
int radix_tree_tag_get(struct radix_tree_root *root,
			unsigned long index, int tag);
unsigned int
radix_tree_gang_lookup_tag(struct radix_tree_root *root, void **results,
		unsigned long first_index, unsigned int max_items, int tag);
int radix_tree_tagged(struct radix_tree_root *root, int tag);

static inline void radix_tree_preload_end(void)
{
	preempt_enable();
}

/**
 *	radix_tree_lookup    -    perform lookup operation on a radix tree
 *	@root:		radix tree root
 *	@index:		index key
 *
 *	Lookup the item at the position @index in the radix tree @root.
 */
static inline void *radix_tree_lookup(struct radix_tree_root *root,
							unsigned long index)
{
	return radix_tree_lookup_node(root, index, 0);
}

/**
 *	radix_tree_cache_init    -    init a look-aside cache
 *	@cache:		look-aside cache
 *
 *	Init the radix tree look-aside cache @cache.
 */
static inline void radix_tree_cache_init(struct radix_tree_cache *cache)
{
	cache->first_index = RADIX_TREE_MAP_MASK;
	cache->tree_node = NULL;
}

/**
 *	radix_tree_cache_lookup    -    cached lookup on a radix tree
 *	@root:		radix tree root
 *	@cache:		look-aside cache
 *	@index:		index key
 *
 *	Lookup the item at the position @index in the radix tree @root,
 *	and make use of @cache to speedup the lookup process.
 */
static inline void *radix_tree_cache_lookup(struct radix_tree_root *root,
						struct radix_tree_cache *cache,
						unsigned long index)
{
	return radix_tree_cache_lookup_node(root, cache, index, 0);
}

static inline unsigned int radix_tree_cache_size(struct radix_tree_cache *cache)
{
	return RADIX_TREE_MAP_SIZE;
}

static inline int radix_tree_cache_full(struct radix_tree_cache *cache)
{
	return radix_tree_cache_count(cache) == radix_tree_cache_size(cache);
}

static inline unsigned long
radix_tree_cache_first_index(struct radix_tree_cache *cache)
{
	return cache->first_index;
}

#endif /* _LINUX_RADIX_TREE_H */
