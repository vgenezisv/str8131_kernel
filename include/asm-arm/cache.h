/*
 *  linux/include/asm-arm/cache.h
 */
#ifndef __ASMARM_CACHE_H
#define __ASMARM_CACHE_H

#ifdef CONFIG_ARCH_STR8131
	#define L1_CACHE_SHIFT		4
#else
	#define L1_CACHE_SHIFT		5
#endif

#define L1_CACHE_BYTES		(1 << L1_CACHE_SHIFT)

#endif
