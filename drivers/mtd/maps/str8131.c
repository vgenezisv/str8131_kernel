#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#ifdef CONFIG_MTD_PARTITIONS
#include <linux/mtd/partitions.h>
#endif

#define WINDOW_ADDR	0x10000000
#define WINDOW_SIZE	0x00800000
#define BUSWIDTH	1

static struct map_info str8131_map = {
	.name = "STR8131 NOR Flash",
	.size = WINDOW_SIZE,
	.bankwidth = BUSWIDTH,
	.phys = WINDOW_ADDR
};

#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition str8131_partitions[] = {
	{
		.name =		"ARMBOOT",
		.offset =	CONFIG_ARMBOOT_OFFSET,
		.size =		CONFIG_KERNEL_OFFSET-CONFIG_ARMBOOT_OFFSET,
	},{
		.name =		"Linux Kernel",
		.offset =	CONFIG_KERNEL_OFFSET,
		.size =		CONFIG_ROOTFS_OFFSET-CONFIG_KERNEL_OFFSET,
	},{
		.name =		"MTD Disk1",
		.offset =	CONFIG_ROOTFS_OFFSET,
		.size =		CONFIG_CFG_OFFSET-CONFIG_ROOTFS_OFFSET,
	},{
		.name =		"MTD Disk2",
		.offset =	CONFIG_CFG_OFFSET,
		.size =		0x800000-CONFIG_CFG_OFFSET,
	}
};
#endif

static struct mtd_info *mymtd;

static int __init init_str8131_mtd(void)
{
	struct mtd_partition *parts;
	int nb_parts = 0;

	str8131_map.virt = ioremap(WINDOW_ADDR, WINDOW_SIZE);
	if (!str8131_map.virt) {
		printk("Failed to ioremap\n");
		return -EIO;
	}
	simple_map_init(&str8131_map);

	mymtd = do_map_probe("cfi_probe", &str8131_map);
	if (!mymtd) {
		iounmap((void *)str8131_map.virt);
		return -ENXIO;
	}

	mymtd->owner = THIS_MODULE;
	add_mtd_device(mymtd);

#ifdef CONFIG_MTD_PARTITIONS
	parts = str8131_partitions;
	nb_parts = ARRAY_SIZE(str8131_partitions);
	add_mtd_partitions(mymtd, parts, nb_parts);
#endif

	return 0;
}

static void __exit cleanup_str8131_mtd(void)
{
	if (mymtd) {
#ifdef CONFIG_MTD_PARTITIONS
		del_mtd_partitions(mymtd);
#endif
		map_destroy(mymtd);
	}
	if (str8131_map.virt)
		iounmap((void *)str8131_map.virt);
}

module_init(init_str8131_mtd);
module_exit(cleanup_str8131_mtd);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("STAR Semiconductor Corp");
MODULE_DESCRIPTION("MTD map driver for Star Semi STR8131 SOC");

