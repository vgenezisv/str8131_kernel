/*
 *  linux/arch/arm/lib/strrchr.S
 *
 *  Copyright (C) 1995-2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  ASM optimised string functions
 */
#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/cache.h> /*new for L1_CACHE_SHIFT*/

		.text
		/*.align	5*/
		.align L1_CACHE_SHIFT
ENTRY(strrchr)
		mov	r3, #0
1:		ldrb	r2, [r0], #1
		teq	r2, r1
		subeq	r3, r0, #1
		teq	r2, #0
		bne	1b
		mov	r0, r3
		RETINSTR(mov,pc,lr)
