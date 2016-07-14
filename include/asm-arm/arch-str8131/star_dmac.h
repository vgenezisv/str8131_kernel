/*******************************************************************************
 *
 *  Copyright(c) 2006 Star Semiconductor Corporation, All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  this program; if not, write to the Free Software Foundation, Inc., 59
 *  Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *  The full GNU General Public License is included in this distribution in the
 *  file called LICENSE.
 *
 *  Contact Information:
 *  Technology Support <tech@starsemi.com>
 *  Star Semiconductor 4F, No.1, Chin-Shan 8th St, Hsin-Chu,300 Taiwan, R.O.C
 *
 ******************************************************************************/

#ifndef	_STAR_DMAC_H_
#define	_STAR_DMAC_H_


#include "star_sys_memory_map.h"


#if defined(__UBOOT__)
#define	DMAC_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSPA_GDMAC_BASE_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define	DMAC_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSVA_GDMAC_BASE_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif


/*
 * define access macros
 */
#define	DMAC_INT_STATUS_REG			DMAC_MEM_MAP_VALUE(0x000)

#define	DMAC_INT_TC_STATUS_REG			DMAC_MEM_MAP_VALUE(0x004)
#define	DMAC_INT_TC_STATUS_CLR_REG		DMAC_MEM_MAP_VALUE(0x008)

#define	DMAC_INT_ERR_STATUS_REG			DMAC_MEM_MAP_VALUE(0x00C)
#define	DMAC_INT_ERR_STATUS_CLR_REG		DMAC_MEM_MAP_VALUE(0x010)

#define	DMAC_TC_STATUS_REG			DMAC_MEM_MAP_VALUE(0x014)
#define	DMAC_ERR_STATUS_REG			DMAC_MEM_MAP_VALUE(0x018)

#define	DMAC_CH_ENABLE_STATUS_REG		DMAC_MEM_MAP_VALUE(0x01C)
#define	DMAC_CH_BUSY_STATUS_REG			DMAC_MEM_MAP_VALUE(0x020)

#define	DMAC_CSR_REG				DMAC_MEM_MAP_VALUE(0x024)
#define	DMAC_SYNC_REG				DMAC_MEM_MAP_VALUE(0x028)

#define	DMAC_CH0_CSR_REG			DMAC_MEM_MAP_VALUE(0x100)
#define	DMAC_CH0_CFG_REG			DMAC_MEM_MAP_VALUE(0x104)
#define	DMAC_CH0_SRC_ADDR_REG			DMAC_MEM_MAP_VALUE(0x108)
#define	DMAC_CH0_DST_ADDR_REG			DMAC_MEM_MAP_VALUE(0x10C)
#define	DMAC_CH0_LLP_REG			DMAC_MEM_MAP_VALUE(0x110)
#define	DMAC_CH0_SIZE_REG			DMAC_MEM_MAP_VALUE(0x114)

#define	DMAC_CH1_CSR_REG			DMAC_MEM_MAP_VALUE(0x120)
#define	DMAC_CH1_CFG_REG			DMAC_MEM_MAP_VALUE(0x124)
#define	DMAC_CH1_SRC_ADDR_REG			DMAC_MEM_MAP_VALUE(0x128)
#define	DMAC_CH1_DST_ADDR_REG			DMAC_MEM_MAP_VALUE(0x12C)
#define	DMAC_CH1_LLP_REG			DMAC_MEM_MAP_VALUE(0x130)
#define	DMAC_CH1_SIZE_REG			DMAC_MEM_MAP_VALUE(0x134)

#define	DMAC_CH2_CSR_REG			DMAC_MEM_MAP_VALUE(0x140)
#define	DMAC_CH2_CFG_REG			DMAC_MEM_MAP_VALUE(0x144)
#define	DMAC_CH2_SRC_ADDR_REG			DMAC_MEM_MAP_VALUE(0x148)
#define	DMAC_CH2_DST_ADDR_REG			DMAC_MEM_MAP_VALUE(0x14C)
#define	DMAC_CH2_LLP_REG			DMAC_MEM_MAP_VALUE(0x150)
#define	DMAC_CH2_SIZE_REG			DMAC_MEM_MAP_VALUE(0x154)

#define	DMAC_CH3_CSR_REG			DMAC_MEM_MAP_VALUE(0x160)
#define	DMAC_CH3_CFG_REG			DMAC_MEM_MAP_VALUE(0x164)
#define	DMAC_CH3_SRC_ADDR_REG			DMAC_MEM_MAP_VALUE(0x168)
#define	DMAC_CH3_DST_ADDR_REG			DMAC_MEM_MAP_VALUE(0x16C)
#define	DMAC_CH3_LLP_REG			DMAC_MEM_MAP_VALUE(0x170)
#define	DMAC_CH3_SIZE_REG			DMAC_MEM_MAP_VALUE(0x174)

#define	DMAC_CH4_CSR_REG			DMAC_MEM_MAP_VALUE(0x180)
#define	DMAC_CH4_CFG_REG			DMAC_MEM_MAP_VALUE(0x184)
#define	DMAC_CH4_SRC_ADDR_REG			DMAC_MEM_MAP_VALUE(0x188)
#define	DMAC_CH4_DST_ADDR_REG			DMAC_MEM_MAP_VALUE(0x18C)
#define	DMAC_CH4_LLP_REG			DMAC_MEM_MAP_VALUE(0x190)
#define	DMAC_CH4_SIZE_REG			DMAC_MEM_MAP_VALUE(0x194)

#define	DMAC_CH5_CSR_REG			DMAC_MEM_MAP_VALUE(0x1A0)
#define	DMAC_CH5_CFG_REG			DMAC_MEM_MAP_VALUE(0x1A4)
#define	DMAC_CH5_SRC_ADDR_REG			DMAC_MEM_MAP_VALUE(0x1A8)
#define	DMAC_CH5_DST_ADDR_REG			DMAC_MEM_MAP_VALUE(0x1AC)
#define	DMAC_CH5_LLP_REG			DMAC_MEM_MAP_VALUE(0x1B0)
#define	DMAC_CH5_SIZE_REG			DMAC_MEM_MAP_VALUE(0x1B4)

#define	DMAC_CH6_CSR_REG			DMAC_MEM_MAP_VALUE(0x1C0)
#define	DMAC_CH6_CFG_REG			DMAC_MEM_MAP_VALUE(0x1C4)
#define	DMAC_CH6_SRC_ADDR_REG			DMAC_MEM_MAP_VALUE(0x1C8)
#define	DMAC_CH6_DST_ADDR_REG			DMAC_MEM_MAP_VALUE(0x1CC)
#define	DMAC_CH6_LLP_REG			DMAC_MEM_MAP_VALUE(0x1D0)
#define	DMAC_CH6_SIZE_REG			DMAC_MEM_MAP_VALUE(0x1D4)

#define	DMAC_CH7_CSR_REG			DMAC_MEM_MAP_VALUE(0x1E0)
#define	DMAC_CH7_CFG_REG			DMAC_MEM_MAP_VALUE(0x1E4)
#define	DMAC_CH7_SRC_ADDR_REG			DMAC_MEM_MAP_VALUE(0x1E8)
#define	DMAC_CH7_DST_ADDR_REG			DMAC_MEM_MAP_VALUE(0x1EC)
#define	DMAC_CH7_LLP_REG			DMAC_MEM_MAP_VALUE(0x1F0)
#define	DMAC_CH7_SIZE_REG			DMAC_MEM_MAP_VALUE(0x1F4)


#define DMAC_CH0_INT_BIT_INDEX			0
#define DMAC_CH1_INT_BIT_INDEX			1
#define DMAC_CH2_INT_BIT_INDEX			2
#define DMAC_CH3_INT_BIT_INDEX			3
#define DMAC_CH4_INT_BIT_INDEX			4
#define DMAC_CH5_INT_BIT_INDEX			5
#define DMAC_CH6_INT_BIT_INDEX			6
#define DMAC_CH7_INT_BIT_INDEX			7

#define DMAC_CH0_INT_TC_BIT_INDEX		0
#define DMAC_CH1_INT_TC_BIT_INDEX		1
#define DMAC_CH2_INT_TC_BIT_INDEX		2
#define DMAC_CH3_INT_TC_BIT_INDEX		3
#define DMAC_CH4_INT_TC_BIT_INDEX		4
#define DMAC_CH5_INT_TC_BIT_INDEX		5
#define DMAC_CH6_INT_TC_BIT_INDEX		6
#define DMAC_CH7_INT_TC_BIT_INDEX		7

#define DMAC_CH0_INT_TC_CLR_BIT_INDEX		0
#define DMAC_CH1_INT_TC_CLR_BIT_INDEX		1
#define DMAC_CH2_INT_TC_CLR_BIT_INDEX		2
#define DMAC_CH3_INT_TC_CLR_BIT_INDEX		3
#define DMAC_CH4_INT_TC_CLR_BIT_INDEX		4
#define DMAC_CH5_INT_TC_CLR_BIT_INDEX		5
#define DMAC_CH6_INT_TC_CLR_BIT_INDEX		6
#define DMAC_CH7_INT_TC_CLR_BIT_INDEX		7

#define DMAC_CH0_INT_ERR_BIT_INDEX		0
#define DMAC_CH1_INT_ERR_BIT_INDEX		1
#define DMAC_CH2_INT_ERR_BIT_INDEX		2
#define DMAC_CH3_INT_ERR_BIT_INDEX		3
#define DMAC_CH4_INT_ERR_BIT_INDEX		4
#define DMAC_CH5_INT_ERR_BIT_INDEX		5
#define DMAC_CH6_INT_ERR_BIT_INDEX		6
#define DMAC_CH7_INT_ERR_BIT_INDEX		7

#define DMAC_CH0_INT_ERR_CLR_BIT_INDEX		0
#define DMAC_CH1_INT_ERR_CLR_BIT_INDEX		1
#define DMAC_CH2_INT_ERR_CLR_BIT_INDEX		2
#define DMAC_CH3_INT_ERR_CLR_BIT_INDEX		3
#define DMAC_CH4_INT_ERR_CLR_BIT_INDEX		4
#define DMAC_CH5_INT_ERR_CLR_BIT_INDEX		5
#define DMAC_CH6_INT_ERR_CLR_BIT_INDEX		6
#define DMAC_CH7_INT_ERR_CLR_BIT_INDEX		7

#define DMAC_CH0_TC_STATUS_BIT_INDEX		0
#define DMAC_CH1_TC_STATUS_BIT_INDEX		1
#define DMAC_CH2_TC_STATUS_BIT_INDEX		2
#define DMAC_CH3_TC_STATUS_BIT_INDEX		3
#define DMAC_CH4_TC_STATUS_BIT_INDEX		4
#define DMAC_CH5_TC_STATUS_BIT_INDEX		5
#define DMAC_CH6_TC_STATUS_BIT_INDEX		6
#define DMAC_CH7_TC_STATUS_BIT_INDEX		7

#define DMAC_CH0_ERR_STATUS_BIT_INDEX		0
#define DMAC_CH1_ERR_STATUS_BIT_INDEX		1
#define DMAC_CH2_ERR_STATUS_BIT_INDEX		2
#define DMAC_CH3_ERR_STATUS_BIT_INDEX		3
#define DMAC_CH4_ERR_STATUS_BIT_INDEX		4
#define DMAC_CH5_ERR_STATUS_BIT_INDEX		5
#define DMAC_CH6_ERR_STATUS_BIT_INDEX		6
#define DMAC_CH7_ERR_STATUS_BIT_INDEX		7

#define DMAC_CH0_ENABLE_STATUS_BIT_INDEX	0
#define DMAC_CH1_ENABLE_STATUS_BIT_INDEX	1
#define DMAC_CH2_ENABLE_STATUS_BIT_INDEX	2
#define DMAC_CH3_ENABLE_STATUS_BIT_INDEX	3
#define DMAC_CH4_ENABLE_STATUS_BIT_INDEX	4
#define DMAC_CH5_ENABLE_STATUS_BIT_INDEX	5
#define DMAC_CH6_ENABLE_STATUS_BIT_INDEX	6
#define DMAC_CH7_ENABLE_STATUS_BIT_INDEX	7

#define DMAC_CH0_BUSY_STATUS_BIT_INDEX		0
#define DMAC_CH1_BUSY_STATUS_BIT_INDEX		1
#define DMAC_CH2_BUSY_STATUS_BIT_INDEX		2
#define DMAC_CH3_BUSY_STATUS_BIT_INDEX		3
#define DMAC_CH4_BUSY_STATUS_BIT_INDEX		4
#define DMAC_CH5_BUSY_STATUS_BIT_INDEX		5
#define DMAC_CH6_BUSY_STATUS_BIT_INDEX		6
#define DMAC_CH7_BUSY_STATUS_BIT_INDEX		7

#define DMAC_ENABLE_BIT_INDEX			0
#define DMAC_MASTER0_ENDIAN_BIT_INDEX		1
#define DMAC_MASTER1_ENDIAN_BIT_INDEX		2

#define DMAC_CH0_SYNC_ENABLE_BIT_INDEX		0
#define DMAC_CH1_SYNC_ENABLE_BIT_INDEX		1
#define DMAC_CH2_SYNC_ENABLE_BIT_INDEX		2
#define DMAC_CH3_SYNC_ENABLE_BIT_INDEX		3
#define DMAC_CH4_SYNC_ENABLE_BIT_INDEX		4
#define DMAC_CH5_SYNC_ENABLE_BIT_INDEX		5
#define DMAC_CH6_SYNC_ENABLE_BIT_INDEX		6
#define DMAC_CH7_SYNC_ENABLE_BIT_INDEX		7

#define DMAC_CH_INT_TC_MASK_BIT_INDEX		0
#define DMAC_CH_INT_ERR_MASK_BIT_INDEX		1
#define DMAC_CH_INT_ABT_MASK_BIT_INDEX		2
#define DMAC_CH_BUSY_BIT_INDEX			8

#define DMAC_CH_ENABLE_BIT_INDEX		0
#define DMAC_CH_DST_SEL_BIT_INDEX		1
#define DMAC_CH_SRC_SEL_BIT_INDEX		2
#define DMAC_CH_DST_ADDR_CTL_BIT_INDEX		3
#define DMAC_CH_SRC_ADDR_CTL_BIT_INDEX		5
#define DMAC_CH_MODE_BIT_INDEX			7
#define DMAC_CH_DST_WIDTH_BIT_INDEX		8
#define DMAC_CH_SRC_WIDTH_BIT_INDEX		11
#define DMAC_CH_ABT_BIT_INDEX			15
#define DMAC_CH_SRC_BURST_SIZE_BIT_INDEX	16
#define DMAC_CH_PROTECT_MODE_BIT_INDEX		19
#define DMAC_CH_PROTECT_BUFFERABLE_BIT_INDEX	20
#define DMAC_CH_PROTECT_CACHEABLE_BIT_INDEX	21
#define DMAC_CH_PRIORITY_BIT_INDEX		22
#define DMAC_CH_HHST_SEL_BIT_INDEX		25

#define DMAC_CH_DST_ADDR_CTL_MASK		0x3
#define DMAC_CH_DST_ADDR_CTL_INC		0x0
#define DMAC_CH_DST_ADDR_CTL_DEC		0x1
#define DMAC_CH_DST_ADDR_CTL_FIXED		0x2

#define DMAC_CH_SRC_ADDR_CTL_MASK		0x3
#define DMAC_CH_SRC_ADDR_CTL_INC		0x0
#define DMAC_CH_SRC_ADDR_CTL_DEC		0x1
#define DMAC_CH_SRC_ADDR_CTL_FIXED		0x2

#define DMAC_CH_MODE_NORMAL			0x0
#define DMAC_CH_MODE_HANDSHAKE			0x1

#define DMAC_CH_DST_WIDTH_MASK			0x3
#define DMAC_CH_DST_WIDTH_8BIT			0x0
#define DMAC_CH_DST_WIDTH_16BIT			0x1
#define DMAC_CH_DST_WIDTH_32BIT			0x2

#define DMAC_CH_SRC_WIDTH_MASK			0x3
#define DMAC_CH_SRC_WIDTH_8BIT			0x0
#define DMAC_CH_SRC_WIDTH_16BIT			0x1
#define DMAC_CH_SRC_WIDTH_32BIT			0x2

#define DMAC_CH_SRC_BURST_SIZE_MASK		0x8
#define DMAC_CH_SRC_BURST_SIZE_1		0x0
#define DMAC_CH_SRC_BURST_SIZE_4		0x1
#define DMAC_CH_SRC_BURST_SIZE_8		0x2
#define DMAC_CH_SRC_BURST_SIZE_16		0x3
#define DMAC_CH_SRC_BURST_SIZE_32		0x4
#define DMAC_CH_SRC_BURST_SIZE_64		0x5
#define DMAC_CH_SRC_BURST_SIZE_128		0x6
#define DMAC_CH_SRC_BURST_SIZE_256		0x7

#define DMAC_CH_PRIORITY_MASK			0x4
#define DMAC_CH_PRIORITY_0			0x0	/* lowest priority */
#define DMAC_CH_PRIORITY_1			0x1
#define DMAC_CH_PRIORITY_2			0x2
#define DMAC_CH_PRIORITY_3			0x3	/* highest priority */


#define	DMAC_CH_CSR_REG(idx)			DMAC_MEM_MAP_VALUE(0x100+0x20*(idx))
#define	DMAC_CH_CFG_REG(idx)			DMAC_MEM_MAP_VALUE(0x104+0x20*(idx))
#define	DMAC_CH_SRC_ADDR_REG(idx)		DMAC_MEM_MAP_VALUE(0x108+0x20*(idx))
#define	DMAC_CH_DST_ADDR_REG(idx)		DMAC_MEM_MAP_VALUE(0x10C+0x20*(idx))
#define	DMAC_CH_LLP_REG(idx)			DMAC_MEM_MAP_VALUE(0x110+0x20*(idx))
#define	DMAC_CH_SIZE_REG(idx)			DMAC_MEM_MAP_VALUE(0x114+0x20*(idx))


#define	HAL_DMAC_ENABLE_CHANNEL(idx)		((DMAC_CH_CSR_REG(idx)) |= (0x1))

#define	HAL_DMAC_DISABLE_CHANNEL(idx)		((DMAC_CH_CSR_REG(idx)) &= ~(0x1))

#define	HAL_GET_DMAC_CHANNEL_LLP_COUNTER(ch)	((DMAC_CH_CFG_REG(ch) >> 16) & 0xF)


/*DMAC HW Hand-shake target ID*/
#define	DMAC_HW_HAND_SHAKE_PCM_TX0_ID		0x0
#define	DMAC_HW_HAND_SHAKE_PCM_RX0_ID		0x1

#define	DMAC_HW_HAND_SHAKE_SPI_TX_ID		0x2
#define	DMAC_HW_HAND_SHAKE_SPI_RX_ID		0x3

#define	DMAC_HW_HAND_SHAKE_I2S_TX_LEFT_ID	0x4
#define	DMAC_HW_HAND_SHAKE_I2S_TX_RIGHT_ID	0x5

#define	DMAC_HW_HAND_SHAKE_UART0_TX_ID		0x6
#define	DMAC_HW_HAND_SHAKE_UART0_RX_ID		0x7

#define	DMAC_HW_HAND_SHAKE_UART1_TX_ID		0x8
#define	DMAC_HW_HAND_SHAKE_UART1_RX_ID		0x9

#define	DMAC_HW_HAND_SHAKE_USBDEV_ID		0xA

#define	DMAC_HW_HAND_SHAKE_I2S_RX_LEFT_ID	0xB
#define	DMAC_HW_HAND_SHAKE_I2S_RX_RIGHT_ID	0xC

#define	DMAC_HW_HAND_SHAKE_PCM_TX1_ID		0xD
#define	DMAC_HW_HAND_SHAKE_PCM_RX1_ID		0xE


#define MAX_DMA_VEC				32

#define DMAC_DST_SEL_MASTER0			0
#define DMAC_DST_SEL_MASTER1			1
#define DMAC_SRC_SEL_MASTER0			0
#define DMAC_SRC_SEL_MASTER1			1

#define DMAC_RESPONSE_OK			0
#define DMAC_RESPONSE_ERR			-1

struct dma_xfer;
typedef struct dma_xfer dma_xfer_t;
typedef void (*dma_end_io_t)(dma_xfer_t *dma_xfer, int err);
typedef struct
{
	u32	src_addr;	// virtual addr
	u32	dst_addr;	// virtual addr
	u32	size;		// bytes
	u8	dst_sel;
	u8	src_sel;
	u8	dst_addr_ctl;
	u8	src_addr_ctl;
	u8	dst_width;
	u8	src_width;
} dma_vec_t;

struct dma_xfer
{
	u8		nr_vec;
	dma_vec_t	vec[MAX_DMA_VEC];
	dma_end_io_t	dma_end_io;
	void		*private;
};

/*
 * DMAC	LLP Descriptor
 */
typedef struct
{
	u32	src_addr;			// physical addr
	u32	dst_addr;			// physical addr
	u32	llp;
	u32	tot_size	: 12;
	u32	reserved0	: 4;
	u32	dst_sel		: 1;
	u32	src_sel		: 1;
	u32	dst_addr_ctl	: 2;
	u32	src_addr_ctl	: 2;
	u32	dst_width	: 3;
	u32	src_width	: 3;
	u32	tc_mask		: 1;
	u32	reserved1	: 3;
} __attribute__((packed)) dma_llp_descr_t;

#define HAL_DMAC_ENABLE() \
	    ((DMAC_CSR_REG) |= (1<<DMAC_ENABLE_BIT_INDEX))


#define HAL_DMAC_DISABLE() \
	    ((DMAC_CSR_REG) &= ~(1<<DMAC_ENABLE_BIT_INDEX))


#define HAL_DMAC_SET_MASTER0_BIG_ENDIAN() \
	    ((DMAC_CSR_REG) |= (1<<DMAC_MASTER0_ENDIAN_BIT_INDEX))


#define HAL_DMAC_SET_MASTER0_LITTLE_ENDIAN() \
	    ((DMAC_CSR_REG) &= ~(1<<DMAC_MASTER0_ENDIAN_BIT_INDEX))


#define HAL_DMAC_SET_MASTER1_BIG_ENDIAN() \
	    ((DMAC_CSR_REG) |= (1<<DMAC_MASTER1_ENDIAN_BIT_INDEX))


#define HAL_DMAC_SET_MASTER1_LITTLE_ENDIAN() \
	    ((DMAC_CSR_REG) &= ~(1<<DMAC_MASTER1_ENDIAN_BIT_INDEX))

#define HAL_DMAC_READ_ERROR_ABORT_INTERRUPT_STATUS(err_abt_status) \
	    ((err_abt_status) = (DMAC_INT_ERR_STATUS_REG))


#define HAL_DMAC_CLEAR_ERROR_ABORT_INTERRUPT_STATUS(err_abt_status) \
	    ((DMAC_INT_ERR_STATUS_CLR_REG) = (err_abt_status))


#define HAL_DMAC_READ_TERMINAL_COUNT_INTERRUPT_STATUS(tc_status) \
	    ((tc_status) = (DMAC_INT_TC_STATUS_REG) & 0xFF)


#define HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(tc_status) \
	    ((DMAC_INT_TC_STATUS_CLR_REG) = (tc_status) & 0xFF)






#endif	// end of #ifndef _STAR_DMAC_H_
