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

#ifndef _STAR_INTC_H_
#define _STAR_INTC_H_

#include <asm/arch/star_sys_memory_map.h>

#if defined(__UBOOT__)
#define INTC_MEM_MAP_VALUE(reg_offset)			(*((u32 volatile *)(SYSPA_VIC_BASE_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define INTC_MEM_MAP_VALUE(reg_offset)			(*((u32 volatile *)(SYSVA_VIC_BASE_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif

/*
 * define access macros
 */
#define INTC_INTERRUPT_RAW_STATUS_REG			INTC_MEM_MAP_VALUE(0x000)
#define INTC_EDGE_INTERRUPT_SOURCE_CLEAR_REG		INTC_MEM_MAP_VALUE(0x004)
#define INTC_INTERRUPT_MASK_REG				INTC_MEM_MAP_VALUE(0x008)
#define INTC_INTERRUPT_MASK_CLEAR_REG			INTC_MEM_MAP_VALUE(0x00C)
#define INTC_INTERRUPT_TRIGGER_MODE_REG			INTC_MEM_MAP_VALUE(0x010)
#define INTC_INTERRUPT_TRIGGER_LEVEL_REG		INTC_MEM_MAP_VALUE(0x014)
#define INTC_FIQ_SELECT_REG				INTC_MEM_MAP_VALUE(0x018)
#define INTC_IRQ_STATUS_REG				INTC_MEM_MAP_VALUE(0x01C)
#define INTC_FIQ_STATUS_REG				INTC_MEM_MAP_VALUE(0x020)
#define INTC_SOFTWARE_INTERRUPT_REG			INTC_MEM_MAP_VALUE(0x024)
#define INTC_SOFTWARE_INTERRUPT_CLEAR_REG		INTC_MEM_MAP_VALUE(0x028)
#define INTC_SOFTWARE_PRIORITY_MASK_REG			INTC_MEM_MAP_VALUE(0x02C)
#define INTC_INTERRUPT_SOURCE_BIT_INDEX_REG		INTC_MEM_MAP_VALUE(0x030)
#define INTC_POWER_MANAGEMENT_INTERRUPT_REG		INTC_MEM_MAP_VALUE(0x034)
#define INTC_EXTERNAL_INTERRUPT_SELECT_REG		INTC_MEM_MAP_VALUE(0x03C)

#define INTC_VECTOR_ADDRESS_0_REG			INTC_MEM_MAP_VALUE(0x040)
#define INTC_VECTOR_ADDRESS_1_REG			INTC_MEM_MAP_VALUE(0x044)
#define INTC_VECTOR_ADDRESS_2_REG			INTC_MEM_MAP_VALUE(0x048)
#define INTC_VECTOR_ADDRESS_3_REG			INTC_MEM_MAP_VALUE(0x04C)
#define INTC_VECTOR_ADDRESS_4_REG			INTC_MEM_MAP_VALUE(0x050)
#define INTC_VECTOR_ADDRESS_5_REG			INTC_MEM_MAP_VALUE(0x054)
#define INTC_VECTOR_ADDRESS_6_REG			INTC_MEM_MAP_VALUE(0x058)
#define INTC_VECTOR_ADDRESS_7_REG			INTC_MEM_MAP_VALUE(0x05C)
#define INTC_VECTOR_ADDRESS_8_REG			INTC_MEM_MAP_VALUE(0x060)
#define INTC_VECTOR_ADDRESS_9_REG			INTC_MEM_MAP_VALUE(0x064)
#define INTC_VECTOR_ADDRESS_10_REG			INTC_MEM_MAP_VALUE(0x068)
#define INTC_VECTOR_ADDRESS_11_REG			INTC_MEM_MAP_VALUE(0x06C)
#define INTC_VECTOR_ADDRESS_12_REG			INTC_MEM_MAP_VALUE(0x070)
#define INTC_VECTOR_ADDRESS_13_REG			INTC_MEM_MAP_VALUE(0x074)
#define INTC_VECTOR_ADDRESS_14_REG			INTC_MEM_MAP_VALUE(0x078)
#define INTC_VECTOR_ADDRESS_15_REG			INTC_MEM_MAP_VALUE(0x07C)
#define INTC_VECTOR_ADDRESS_16_REG			INTC_MEM_MAP_VALUE(0x080)
#define INTC_VECTOR_ADDRESS_17_REG			INTC_MEM_MAP_VALUE(0x084)
#define INTC_VECTOR_ADDRESS_18_REG			INTC_MEM_MAP_VALUE(0x088)
#define INTC_VECTOR_ADDRESS_19_REG			INTC_MEM_MAP_VALUE(0x08C)
#define INTC_VECTOR_ADDRESS_20_REG			INTC_MEM_MAP_VALUE(0x090)
#define INTC_VECTOR_ADDRESS_21_REG			INTC_MEM_MAP_VALUE(0x094)
#define INTC_VECTOR_ADDRESS_22_REG			INTC_MEM_MAP_VALUE(0x098)
#define INTC_VECTOR_ADDRESS_23_REG			INTC_MEM_MAP_VALUE(0x09C)
#define INTC_VECTOR_ADDRESS_24_REG			INTC_MEM_MAP_VALUE(0x0A0)
#define INTC_VECTOR_ADDRESS_25_REG			INTC_MEM_MAP_VALUE(0x0A4)
#define INTC_VECTOR_ADDRESS_26_REG			INTC_MEM_MAP_VALUE(0x0A8)
#define INTC_VECTOR_ADDRESS_27_REG			INTC_MEM_MAP_VALUE(0x0AC)
#define INTC_VECTOR_ADDRESS_28_REG			INTC_MEM_MAP_VALUE(0x0B0)
#define INTC_VECTOR_ADDRESS_29_REG			INTC_MEM_MAP_VALUE(0x0B4)
#define INTC_VECTOR_ADDRESS_30_REG			INTC_MEM_MAP_VALUE(0x0B8)
#define INTC_VECTOR_ADDRESS_31_REG			INTC_MEM_MAP_VALUE(0x0BC)

#define INTC_INTERRUPT_PRIORITY_0_REG			INTC_MEM_MAP_VALUE(0x0C0)
#define INTC_INTERRUPT_PRIORITY_1_REG			INTC_MEM_MAP_VALUE(0x0C4)
#define INTC_INTERRUPT_PRIORITY_2_REG			INTC_MEM_MAP_VALUE(0x0C8)
#define INTC_INTERRUPT_PRIORITY_3_REG			INTC_MEM_MAP_VALUE(0x0CC)
#define INTC_INTERRUPT_PRIORITY_4_REG			INTC_MEM_MAP_VALUE(0x0D0)
#define INTC_INTERRUPT_PRIORITY_5_REG			INTC_MEM_MAP_VALUE(0x0D4)
#define INTC_INTERRUPT_PRIORITY_6_REG			INTC_MEM_MAP_VALUE(0x0D8)
#define INTC_INTERRUPT_PRIORITY_7_REG			INTC_MEM_MAP_VALUE(0x0DC)
#define INTC_INTERRUPT_PRIORITY_8_REG			INTC_MEM_MAP_VALUE(0x0E0)
#define INTC_INTERRUPT_PRIORITY_9_REG			INTC_MEM_MAP_VALUE(0x0E4)
#define INTC_INTERRUPT_PRIORITY_10_REG			INTC_MEM_MAP_VALUE(0x0E8)
#define INTC_INTERRUPT_PRIORITY_11_REG			INTC_MEM_MAP_VALUE(0x0EC)
#define INTC_INTERRUPT_PRIORITY_12_REG			INTC_MEM_MAP_VALUE(0x0F0)
#define INTC_INTERRUPT_PRIORITY_13_REG			INTC_MEM_MAP_VALUE(0x0F4)
#define INTC_INTERRUPT_PRIORITY_14_REG			INTC_MEM_MAP_VALUE(0x0F8)
#define INTC_INTERRUPT_PRIORITY_15_REG			INTC_MEM_MAP_VALUE(0x0FC)
#define INTC_INTERRUPT_PRIORITY_16_REG			INTC_MEM_MAP_VALUE(0x100)
#define INTC_INTERRUPT_PRIORITY_17_REG			INTC_MEM_MAP_VALUE(0x104)
#define INTC_INTERRUPT_PRIORITY_18_REG			INTC_MEM_MAP_VALUE(0x108)
#define INTC_INTERRUPT_PRIORITY_19_REG			INTC_MEM_MAP_VALUE(0x10C)
#define INTC_INTERRUPT_PRIORITY_20_REG			INTC_MEM_MAP_VALUE(0x110)
#define INTC_INTERRUPT_PRIORITY_21_REG			INTC_MEM_MAP_VALUE(0x114)
#define INTC_INTERRUPT_PRIORITY_22_REG			INTC_MEM_MAP_VALUE(0x118)
#define INTC_INTERRUPT_PRIORITY_23_REG			INTC_MEM_MAP_VALUE(0x11C)
#define INTC_INTERRUPT_PRIORITY_24_REG			INTC_MEM_MAP_VALUE(0x120)
#define INTC_INTERRUPT_PRIORITY_25_REG			INTC_MEM_MAP_VALUE(0x124)
#define INTC_INTERRUPT_PRIORITY_26_REG			INTC_MEM_MAP_VALUE(0x128)
#define INTC_INTERRUPT_PRIORITY_27_REG			INTC_MEM_MAP_VALUE(0x12C)
#define INTC_INTERRUPT_PRIORITY_28_REG			INTC_MEM_MAP_VALUE(0x130)
#define INTC_INTERRUPT_PRIORITY_29_REG			INTC_MEM_MAP_VALUE(0x134)
#define INTC_INTERRUPT_PRIORITY_30_REG			INTC_MEM_MAP_VALUE(0x138)
#define INTC_INTERRUPT_PRIORITY_31_REG			INTC_MEM_MAP_VALUE(0x13C)

#define INTC_IRQ_VECTOR_ADDRESS_REG			INTC_MEM_MAP_VALUE(0x140)

#define INTC_VECTOR_INTERRUPT_ENABLE_REG		INTC_MEM_MAP_VALUE(0x144)



/*
 * define constants macros
 */
#define INTC_TIMER1_BIT_INDEX				(0)
#define INTC_TIMER2_BIT_INDEX				(1)

#define INTC_CLOCK_SCALE_BIT_INDEX			(2)

#define INTC_WATCHDOG_TIMER_BIT_INDEX			(3)

#define INTC_GPIO_EXTERNAL_INT_BIT_INDEX		(4)

#define INTC_PCI_INTA_BIT_INDEX				(5)
#define INTC_PCI_INTB_BIT_INDEX				(6)
#define INTC_PCI_BROKEN_BIT_INDEX			(7)
#define INTC_PCI_AHB2BRIDGE_BIT_INDEX			(8)

#define INTC_UART0_BIT_INDEX				(9)
#define INTC_UART1_BIT_INDEX				(10)

#define INTC_GDMAC_TC_BIT_INDEX				(11)
#define INTC_GDMAC_ERROR_BIT_INDEX			(12)

#define INTC_HSDMAC_BIT_INDEX				(13)

#define INTC_RTC_BIT_INDEX				(14)

#define INTC_PCM_BIT_INDEX				(15)

#define INTC_GSW_HNAT_BIT_INDEX				(16)

#define INTC_IDP_BIT_INDEX				(17)

#define INTC_GSW_STATUS_BIT_INDEX			(18)
#define INTC_GSW_TSTC_BIT_INDEX				(19)
#define INTC_GSW_FSRC_BIT_INDEX				(20)
#define INTC_GSW_TSQE_BIT_INDEX				(21)
#define INTC_GSW_FSQF_BIT_INDEX				(22)

#define INTC_VTSS_CAPTURE_QUEUE_BIT_INDEX		(23)	/* High level trigger */

#define INTC_USB20_BIT_INDEX				(24)

#define INTC_I2S_BIT_INDEX				(25)
#define INTC_SPI_BIT_INDEX				(26)
#define INTC_I2C_BIT_INDEX				(27)

#define INTC_EXT_INT28_BIT_INDEX			(28)	/* Shared with GPIO[0] */
#define INTC_EXT_INT29_BIT_INDEX			(29)	/* Shared with GPIO[1] */
#define INTC_EXT_INT30_BIT_INDEX			(30)	/* Shared with GPIO[2] */
#define INTC_VTSS_GPIO_BIT_INDEX			(31)	/* Programmable trigger */



/*
 * define interrupt types
 */
#define INTC_IRQ_INTERRUPT				(0)
#define INTC_FIQ_INTERRUPT				(1)

/*
 * define interrupt trigger mode
 */
#define INTC_LEVEL_TRIGGER				(0)
#define INTC_EDGE_TRIGGER				(1)

/*
 * define rising/falling edge for edge trigger mode
 */
#define INTC_RISING_EDGE				(0)
#define INTC_FALLING_EDGE				(1)

/*
 * define active High/Low for level trigger mode
 */
#define INTC_ACTIVE_HIGH				(0)
#define INTC_ACTIVE_LOW					(1)

/*
 * macro declarations
 */
#define HAL_INTC_READ_INTERRUPT_RAW_STATUS(int_raw_status) \
{ \
    (int_raw_status) = (INTC_INTERRUPT_RAW_STATUS_REG); \
}

#define HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(source_bit_index) \
{ \
    (INTC_EDGE_INTERRUPT_SOURCE_CLEAR_REG) = (1 << source_bit_index); \
}

#define HAL_INTC_READ_INTERRUPT_MASK(int_mask) \
{ \
    (int_mask) = (INTC_INTERRUPT_MASK_REG); \
}

#define HAL_INTC_WRITE_INTERRUPT_MASK(int_mask) \
{ \
    (INTC_INTERRUPT_MASK_REG) = (int_mask); \
}

#define HAL_INTC_DISABLE_INTERRUPT_SOURCE(source_bit_index) \
{ \
    (INTC_INTERRUPT_MASK_REG) = (1 << source_bit_index); \
}

#define HAL_INTC_ENABLE_INTERRUPT_SOURCE(source_bit_index) \
{ \
    (INTC_INTERRUPT_MASK_CLEAR_REG) = (1 << source_bit_index); \
}

#define HAL_INTC_SET_EDGE_TRIGGER_MODE(source_bit_index) \
{ \
    (INTC_INTERRUPT_TRIGGER_MODE_REG) |= (1 << source_bit_index);\
}

#define HAL_INTC_SET_LEVEL_TRIGGER_MODE(source_bit_index) \
{ \
    (INTC_INTERRUPT_TRIGGER_MODE_REG) &= (~(1 << source_bit_index)); \
}

#define HAL_INTC_SET_RISING_EDGE_TRIGGER_LEVEL(source_bit_index) \
{ \
    (INTC_INTERRUPT_TRIGGER_LEVEL_REG) &= (~(1 << source_bit_index)); \
}

#define HAL_INTC_SET_FALLING_EDGE_TRIGGER_LEVEL(source_bit_index) \
{ \
    (INTC_INTERRUPT_TRIGGER_LEVEL_REG) |= (1 << source_bit_index); \
}

#define HAL_INTC_SET_ACTIVE_HIGH_TRIGGER_LEVEL(source_bit_index) \
{ \
    (INTC_INTERRUPT_TRIGGER_LEVEL_REG) &= (~(1 << source_bit_index));\
}

#define HAL_INTC_SET_ACTIVE_LOW_TRIGGER_LEVEL(source_bit_index) \
{ \
    (INTC_INTERRUPT_TRIGGER_LEVEL_REG) |= ((1 << source_bit_index)); \
}

#define HAL_INTC_ASSIGN_INTERRUPT_TO_IRQ(source_bit_index) \
{ \
    (INTC_FIQ_SELECT_REG) &= (~(1 << source_bit_index)); \
}

#define HAL_INTC_ASSIGN_INTERRUPT_TO_FIQ(source_bit_index) \
{ \
    (INTC_FIQ_SELECT_REG) |= (1 << source_bit_index); \
}

#define HAL_INTC_READ_IRQ_STATUS(int_irq_status) \
{ \
    (int_irq_status) = (INTC_IRQ_STATUS_REG); \
}

#define HAL_INTC_READ_FIQ_STATUS(int_fiq_status) \
{ \
    (int_fiq_status) = (INTC_FIQ_STATUS_REG); \
}

#define HAL_INTC_READ_SOFTWARE_INTERRUPT(software_interrupt) \
{ \
    (software_interrupt) = (INTC_SOFTWARE_INTERRUPT_REG); \
}

#define HAL_INTC_ENABLE_SOFTWARE_INTERRUPT(source_bit_index) \
{ \
    (INTC_SOFTWARE_INTERRUPT_REG) = (1 << source_bit_index); \
}

#define HAL_INTC_CLEAR_SOFTWARE_INTERRUPT(source_bit_index) \
{ \
    (INTC_SOFTWARE_INTERRUPT_CLEAR_REG) = (1 << source_bit_index); \
}

#define HAL_INTC_SELECT_INTERRUPT_SOURCE_FOR_SLEEP_WAKEUP(source_bit_index) \
{ \
    (INTC_POWER_MANAGEMENT_INTERRUPT_REG) = (1 << source_bit_index); \
}

#define HAL_INTC_GET_CURRENT_INTERRUPT_SOURCE_BIT_INDEX_NUMBER(source_index_number) \
{ \
    (source_index_number) = (INTC_INTERRUPT_SOURCE_BIT_INDEX_REG & 0x1F); \
}

#define HAL_INTC_BYPASS_EXTERNAL_INTERRUPT_TRIGGER_MODE() \
{ \
    (INTC_EXTERNAL_INTERRUPT_SELECT_REG) |= (0x1); \
}

#define HAL_INTC_ENABLE_EXTERNAL_INTERRUPT_TRIGGER_MODE() \
{ \
    (INTC_EXTERNAL_INTERRUPT_SELECT_REG) &= ~(0x1); \
}

#define HAL_INTC_ENABLE_VIC() \
{ \
    (INTC_VECTOR_INTERRUPT_ENABLE_REG) = (0x1); \
}

#define HAL_INTC_DISABLE_VIC() \
{ \
    (INTC_VECTOR_INTERRUPT_ENABLE_REG) = (0x0); \
}

#endif  // end of #ifndef _STAR_INTC_H_

