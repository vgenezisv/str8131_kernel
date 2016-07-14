/*******************************************************************************
 *
 *
 *   Copyright(c) 2003 - 2006 Star Semiconductor Corporation, All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *   more details.
 *
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59
 *   Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *   The full GNU General Public License is included in this distribution in the
 *   file called LICENSE.
 *
 *   Contact Information:
 *   Technology Support <tech@starsemi.com>
 *   Star Semiconductor 4F, No.1, Chin-Shan 8th St, Hsin-Chu,300 Taiwan, R.O.C
 *
 ********************************************************************************/

#include "star_gsw.h"

#ifdef LINUX24
#include <asm/arch/str9100/star_tool.h>
#include <asm/arch/str9100/star_misc.h>
#endif

#ifdef LINUX26
#include <asm/arch/star_misc.h>
#endif

#if defined(LINUX24)
#if defined(CONFIG_STAR9100_SHNAT_PCI_FASTPATH) 
#include <linux/star9100/star9100_shnat.h>
#include <linux/star9100/str9100_shnat_hook.h>
#endif /* defined(CONFIG_STAR9100_SHNAT_PCI_FASTPATH) */
#elif defined(LINUX26) /* defined(LINUX24) */
#if defined(CONFIG_STR9100_SHNAT) 
#include <linux/str9100/star9100_shnat.h>
#include <linux/str9100/str9100_shnat_hook.h>
#endif /* defined(CONFIG_STAR9100_SHNAT_PCI_FASTPATH) */
#endif /* defined(LINUX24) */

#if defined(LINUX24)
#define IRQ_RETURN void
#define IRQ_HANDLED 
static const char star_gsw_driver_version[] =
	"Star GSW Driver(for Linux Kernel 2.4) - Star Semiconductor\n";
#elif defined(LINUX26)
#define IRQ_RETURN irqreturn_t
static const char star_gsw_driver_version[] =
	"Star GSW Driver(for Linux Kernel 2.6) - Star Semiconductor\n";
#endif

//struct proc_dir_entry *str9100_gsw_procdir=0;
static u32 max_pend_int_cnt=MAX_PEND_INT_CNT, max_pend_time=MAX_PEND_TIME;

#define MIN_PACKET_LEN 60


static struct net_device *STAR_GSW_LAN_DEV;
static struct net_device *STAR_GSW_WAN_DEV;
static struct net_device *STAR_GSW_EWC_DEV;

static struct net_device STAR_NAPI_DEV;


static int install_isr_account = 0;
static int rc_port = 0; // rc mean reference counting, determine port open/close.
static int fsrc_count = 0;
static volatile unsigned long is_qf = 0; // determine queue full state

gsw_info_t star_gsw_info;
static spinlock_t star_gsw_send_lock;

static TXRING_INFO txring;
static RXRING_INFO rxring;

static struct proc_dir_entry *star_gsw_proc_entry;

#ifdef CONFIG_STAR_GSW_NAPI
static void star_gsw_receive_packet(int mode, int *work_done, int work_to_do);
#else
static void star_gsw_receive_packet(int mode);
#endif

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
void gsw_vlan_rx_kill_vid(struct net_device *dev, unsigned short vid);
void gsw_vlan_rx_register(struct net_device *dev, struct vlan_group *grp);
#endif

static int star_gsw_notify_reboot(struct notifier_block *nb, unsigned long event, void *ptr);

static struct notifier_block star_gsw_notifier_reboot = {
	.notifier_call	= star_gsw_notify_reboot,
	.next		= NULL,
	.priority	= 0
};




#ifdef STAR_GSW_TIMER
static struct timer_list star_gsw_timer;
static void star_gsw_timer_func(unsigned long data)
{
	int i;
	int tssd_index;
	int tssd_current;
	int skb_free_count = 0;
	STAR_GSW_TXDESC volatile *txdesc_ptr;
	unsigned long flags;

	local_irq_save(flags);
	HAL_GSW_READ_TSSD(tssd_current);
	tssd_index = (tssd_current - (u32)txring.phy_addr) >> 4;
	if (tssd_index > txring.to_free_index) {
		skb_free_count = tssd_index - txring.to_free_index;
	} else if (tssd_index < txring.to_free_index) {
		skb_free_count = STAR_GSW_MAX_TFD_NUM + tssd_index - txring.to_free_index;
	}
	for (i = 0; i < skb_free_count; i++) {
		txdesc_ptr = txring.vir_addr + txring.to_free_index;
		if (txdesc_ptr->cown == 0) {
			break;
		}
		if (txring.skb_ptr[txring.to_free_index]) {
			dev_kfree_skb_any(txring.skb_ptr[txring.to_free_index]);
			txring.skb_ptr[txring.to_free_index] = NULL;
		}
		txring.to_free_index++;
		if (txring.to_free_index == STAR_GSW_MAX_TFD_NUM) {
			txring.to_free_index = 0;
		}
	}
	local_irq_restore(flags);
}
#endif


#define between(x, start, end) ((x)>=(start) && (x)<=(end))
void print_packet(unsigned char *data, int len) 
{
    int i,j;

    printk("packet length: %d%s:\n", len, len>128?"(only show the first 128 bytes)":"");
    if(len > 128) {
        len = 128;
    }
    for(i=0;len;) {
        if(len >=16 ) {
            for(j=0;j<16;j++) {
                printk("%02x ", data[i++]);
            }
            printk("| ");

            i -= 16;
            for(j=0;j<16;j++) {
                if( between(data[i], 0x21, 0x7e) ) {
                    printk("%c", data[i++]);
                }
                else {
                    printk(".");
                    i++;
                }
            }
            printk("\n");

            len -= 16;
        }
        else {
            /* last line */

            for(j=0; j<len; j++) {
                printk("%02x ", data[i++]);
            }
            for(;j<16;j++) {
                printk("   ");
            }
            printk("| ");

            i -= len;
            for(j=0;j<len;j++) {
                if( between(data[i], 0x21, 0x7e) ) {
                    printk("%c", data[i++]);
                }
                else {
                    printk(".");
                    i++;
                }
            }
            for(;j<16;j++) {
                printk(" ");
            }
            printk("\n");

            len = 0;
        }
    }
    return;

}

// add by descent 2006/07/07
void init_switch()
{
        u32 sw_config;

        /*
         * Configure GSW configuration
         */
        sw_config = GSW_SWITCH_CONFIG;

#if 0
        // orignal virgon configuration
        // enable fast aging
        sw_config |= (0xF);

        // CRC stripping
        sw_config |= (0x1 << 21);

        // IVL learning
        sw_config |= (0x1 << 22);
        // HNAT disable
        sw_config &= ~(0x1 << 23);

        GSW_SWITCH_CONFIG = sw_config;

        sw_config = GSW_SWITCH_CONFIG;
#endif

//#if 0
        /* configure switch */
        sw_config = GSW_SWITCH_CONFIG;

        sw_config &= ~0xF;      /* disable aging */
        sw_config |= 0x1;       /* disable aging */

#ifdef JUMBO_ENABLE

        // CRC stripping and GSW_CFG_MAX_LEN_JMBO
        //sw_config |= (GSW_CFG_CRC_STRP | GSW_CFG_MAX_LEN_JMBO);
        // CRC stripping and GSW_CFG_MAX_LEN_JMBO
        sw_config |= ((0x1 << 21) | (0x3 << 4));
	       
#else
        // CRC stripping and 1536 bytes
        //sw_config |= (GSW_CFG_CRC_STRP | GSW_CFG_MAX_LEN_1536);
	sw_config |= ((0x1 << 21) | (0x2 << 4));
#endif

        /* IVL */
        //sw_config |= GSW_CFG_IVL;
        sw_config |= (0x1 << 22);


        /* disable HNAT */
        //sw_config &= ~GSW_CFG_HNAT_EN;
        sw_config &= ~(0x1 << 23);


#ifdef CONFIG_STAR9100_SHNAT_PCI_FASTPATH
	// PCI FASTPATH must enable firewall mode
	sw_config |= (0x1 << 24);
#endif

        GSW_SWITCH_CONFIG = sw_config;
//#endif
}



static int star_gsw_write_arl_table_entry(gsw_arl_table_entry_t *arl_table_entry)
{
	int i;

	GSW_ARL_TABLE_ACCESS_CONTROL_0 = 0x0;
	GSW_ARL_TABLE_ACCESS_CONTROL_1 = 0x0;
	GSW_ARL_TABLE_ACCESS_CONTROL_2 = 0x0;

	GSW_ARL_TABLE_ACCESS_CONTROL_1 = (((arl_table_entry->filter & 0x1) << 3) |
		((arl_table_entry->vlan_mac & 0x1) << 4) |
		((arl_table_entry->vlan_gid & 0x7) << 5) |
		((arl_table_entry->age_field & 0x7) << 8) |
		((arl_table_entry->port_map & 0x7) << 11) |
		((arl_table_entry->mac_addr[0] & 0xFF) << 16) |
		((arl_table_entry->mac_addr[1] & 0xFF) << 24));

	GSW_ARL_TABLE_ACCESS_CONTROL_2 = (((arl_table_entry->mac_addr[2] & 0xFF) << 0) |
		((arl_table_entry->mac_addr[3] & 0xFF) << 8) |
		((arl_table_entry->mac_addr[4] & 0xFF) << 16) |
		((arl_table_entry->mac_addr[5] & 0xFF) << 24));

	// issue the write command
	GSW_ARL_TABLE_ACCESS_CONTROL_0 = (0x1 << 3);

	for (i = 0; i < 0x1000; i++) {
		if (GSW_ARL_TABLE_ACCESS_CONTROL_1 & (0x1)) {
			return (1);  // write OK
		} else {
			udelay(10);
		}
	}

	return (0);  // write failed
}

static int star_gsw_config_cpu_port(void)
{
	gsw_arl_table_entry_t arl_table_entry;
	u32 cpu_port_config;

	/*
	 * Write some default ARL table entries
	 */
	// default ARL entry for VLAN0
	arl_table_entry.filter		= 0;
	arl_table_entry.vlan_mac	= 1;	// the MAC in this table entry is MY VLAN MAC
	arl_table_entry.vlan_gid	= star_gsw_info.vlan[0].vlan_gid;
	arl_table_entry.age_field	= 0x7;	// static entry
	arl_table_entry.port_map	= star_gsw_info.vlan[0].vlan_group;
	memcpy(arl_table_entry.mac_addr, star_gsw_info.vlan[0].vlan_mac, 6);
	if (!star_gsw_write_arl_table_entry(&arl_table_entry)) {
		return 1;
	}

	// default ARL entry for VLAN1
	arl_table_entry.filter		= 0;
	arl_table_entry.vlan_mac	= 1;
	arl_table_entry.vlan_gid	= star_gsw_info.vlan[1].vlan_gid;
	arl_table_entry.age_field	= 0x7;
	arl_table_entry.port_map	= star_gsw_info.vlan[1].vlan_group;
	memcpy(arl_table_entry.mac_addr, star_gsw_info.vlan[1].vlan_mac, 6);
	if (!star_gsw_write_arl_table_entry(&arl_table_entry)) {
		return 1;
	}

	// default ARL entry for VLAN2
	arl_table_entry.filter		= 0;
	arl_table_entry.vlan_mac	= 1;
	arl_table_entry.vlan_gid	= star_gsw_info.vlan[2].vlan_gid;
	arl_table_entry.age_field	= 0x7;
	arl_table_entry.port_map	= star_gsw_info.vlan[2].vlan_group;
	memcpy(arl_table_entry.mac_addr, star_gsw_info.vlan[2].vlan_mac, 6);
	if (!star_gsw_write_arl_table_entry(&arl_table_entry)) {
		return 1;
	}

	// default ARL entry for VLAN3
	arl_table_entry.filter		= 0;
	arl_table_entry.vlan_mac	= 1;
	arl_table_entry.vlan_gid	= star_gsw_info.vlan[3].vlan_gid;
	arl_table_entry.age_field	= 0x7;
	arl_table_entry.port_map	= star_gsw_info.vlan[3].vlan_group;
	memcpy(arl_table_entry.mac_addr, star_gsw_info.vlan[3].vlan_mac, 6);
	if (!star_gsw_write_arl_table_entry(&arl_table_entry)) {
		return 1;
	}

	GSW_SET_PORT0_PVID(star_gsw_info.port[0].pvid);
	GSW_SET_PORT1_PVID(star_gsw_info.port[1].pvid);
	GSW_SET_CPU_PORT_PVID(star_gsw_info.port[2].pvid);

	GSW_SET_VLAN_0_VID(star_gsw_info.vlan[0].vlan_vid);
	GSW_SET_VLAN_1_VID(star_gsw_info.vlan[1].vlan_vid);
	GSW_SET_VLAN_2_VID(star_gsw_info.vlan[2].vlan_vid);
	GSW_SET_VLAN_3_VID(star_gsw_info.vlan[3].vlan_vid);
	GSW_SET_VLAN_4_VID(star_gsw_info.vlan[4].vlan_vid);
	GSW_SET_VLAN_5_VID(star_gsw_info.vlan[5].vlan_vid);
	GSW_SET_VLAN_6_VID(star_gsw_info.vlan[6].vlan_vid);
	GSW_SET_VLAN_7_VID(star_gsw_info.vlan[7].vlan_vid);

	GSW_SET_VLAN_0_MEMBER(star_gsw_info.vlan[0].vlan_group);
	GSW_SET_VLAN_1_MEMBER(star_gsw_info.vlan[1].vlan_group);
	GSW_SET_VLAN_2_MEMBER(star_gsw_info.vlan[2].vlan_group);
	GSW_SET_VLAN_3_MEMBER(star_gsw_info.vlan[3].vlan_group);
	GSW_SET_VLAN_4_MEMBER(star_gsw_info.vlan[4].vlan_group);
	GSW_SET_VLAN_5_MEMBER(star_gsw_info.vlan[5].vlan_group);
	GSW_SET_VLAN_6_MEMBER(star_gsw_info.vlan[6].vlan_group);
	GSW_SET_VLAN_7_MEMBER(star_gsw_info.vlan[7].vlan_group);

	GSW_SET_VLAN_0_TAG(star_gsw_info.vlan[0].vlan_tag_flag);
	GSW_SET_VLAN_1_TAG(star_gsw_info.vlan[1].vlan_tag_flag);
	GSW_SET_VLAN_2_TAG(star_gsw_info.vlan[2].vlan_tag_flag);
	GSW_SET_VLAN_3_TAG(star_gsw_info.vlan[3].vlan_tag_flag);
	GSW_SET_VLAN_4_TAG(star_gsw_info.vlan[4].vlan_tag_flag);
	GSW_SET_VLAN_5_TAG(star_gsw_info.vlan[5].vlan_tag_flag);
	GSW_SET_VLAN_6_TAG(star_gsw_info.vlan[6].vlan_tag_flag);
	GSW_SET_VLAN_7_TAG(star_gsw_info.vlan[7].vlan_tag_flag);

	// disable all interrupt status sources
	GSW_DISABLE_ALL_INTERRUPT_STATUS_SOURCES();

	// clear previous interrupt sources
	GSW_CLEAR_ALL_INTERRUPT_STATUS_SOURCES();

	// disable all DMA-related interrupt sources
	INTC_DISABLE_INTERRUPT_SOURCE(INTC_GSW_TSTC_BIT_INDEX);
	INTC_DISABLE_INTERRUPT_SOURCE(INTC_GSW_FSRC_BIT_INDEX);
	INTC_DISABLE_INTERRUPT_SOURCE(INTC_GSW_TSQE_BIT_INDEX);
	INTC_DISABLE_INTERRUPT_SOURCE(INTC_GSW_FSQF_BIT_INDEX);

	// clear previous interrupt sources
	INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GSW_TSTC_BIT_INDEX);
	INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GSW_FSRC_BIT_INDEX);
	INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GSW_TSQE_BIT_INDEX);
	INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GSW_FSQF_BIT_INDEX);

	GSW_TS_DMA_STOP();
	GSW_FS_DMA_STOP();

	GSW_WRITE_TSSD(txring.phy_addr);
	GSW_WRITE_TS_BASE(txring.phy_addr);
	GSW_WRITE_FSSD(rxring.phy_addr);
	GSW_WRITE_FS_BASE(rxring.phy_addr);

	/*
	 * Configure CPU port
	 */
	cpu_port_config = GSW_CPU_PORT_CONFIG;

	//SA learning Disable 
	cpu_port_config |= (0x1 << 19);

	//offset 4N +2 
	cpu_port_config &= ~(1 << 31);
	//cpu_port_config |= (1 << 31);

	/* enable the CPU port */
	cpu_port_config &= ~(1 << 18);

	GSW_CPU_PORT_CONFIG = cpu_port_config;

	return 0;
}

static void star_gsw_interrupt_disable(void)
{
	INTC_DISABLE_INTERRUPT_SOURCE(INTC_GSW_STATUS_BIT_INDEX);
	INTC_DISABLE_INTERRUPT_SOURCE(INTC_GSW_TSTC_BIT_INDEX);
	INTC_DISABLE_INTERRUPT_SOURCE(INTC_GSW_FSRC_BIT_INDEX);
}

static void star_gsw_interrupt_enable(void)
{
	INTC_ENABLE_INTERRUPT_SOURCE(INTC_GSW_STATUS_BIT_INDEX);
	// 20070321 
	//INTC_ENABLE_INTERRUPT_SOURCE(INTC_GSW_TSTC_BIT_INDEX);
	INTC_ENABLE_INTERRUPT_SOURCE(INTC_GSW_FSRC_BIT_INDEX);
}

static int star_gsw_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
#if 0
	int num = 0;
	int ad;
	u32 port;
	u32 fssd_current;
	int fssd_index, rxcount;
	STAR_GSW_RXDESC volatile *rxdesc_ptr = (rxring.vir_addr + rxring.cur_index);

	GSW_READ_FSSD(fssd_current);
	fssd_index = (fssd_current - (u32)rxring.phy_addr) >> 4;

	if (fssd_index > rxring.cur_index) {
		rxcount = fssd_index - rxring.cur_index;
	} else if (fssd_index < rxring.cur_index) {
		rxcount = (STAR_GSW_MAX_RFD_NUM - rxring.cur_index) + fssd_index;
	} else {
		if (rxdesc_ptr->cown == 0) {
			//goto receive_packet_exit;
			rxcount = -1;
		} else {
			// Queue Full
			rxcount = STAR_GSW_MAX_RFD_NUM;
		}
	}

	port = GSW_MAC_PORT_0_CONFIG;
	num = sprintf(page, "\nStar Giga Bit Switch\n");
#ifdef CONFIG_STAR_GSW_NAPI
	num += sprintf(page + num, "Receive Method : NAPI\n");
#else
	num += sprintf(page + num, "Receive Method : General\n");
#endif

#ifdef CONFIG_STAR9100_SHNAT_PCI_FASTPATH
	num += sprintf(page + num, "Support SHNAT PCI Fast PATH\n");
#endif

	HAL_MISC_ORION_ECO_AD(ad);
	num += sprintf(page + num, "Orion Version: %s\n", ad==1?"AD":"AC");

	num += sprintf(page + num, "GSW_DELAYED_INTERRUPT_CONFIG: %x\n", GSW_DELAYED_INTERRUPT_CONFIG);

	num += sprintf(page + num, "GSW_VLAN_VID_0_1: %x\n", GSW_VLAN_VID_0_1);
	num += sprintf(page + num, "GSW_VLAN_VID_2_3: %x\n", GSW_VLAN_VID_2_3);
	num += sprintf(page + num, "GSW_VLAN_VID_4_5: %x\n", GSW_VLAN_VID_4_5);
	num += sprintf(page + num, "GSW_VLAN_VID_6_7: %x\n", GSW_VLAN_VID_6_7);

	num += sprintf(page + num, "STAR_GSW_LAN_DEV: %x\n", STAR_GSW_LAN_DEV);
	num += sprintf(page + num, "STAR_GSW_WAN_DEV: %x\n", STAR_GSW_WAN_DEV);
	num += sprintf(page + num, "GSW_VLAN_TAG_PORT_MAP: %x\n", GSW_VLAN_TAG_PORT_MAP);
	num += sprintf(page + num, "GSW_SWITCH_CONFIG: %x \n", GSW_SWITCH_CONFIG);
	//num += sprintf(page + num, "is_qf: %d \n", is_qf);
	num += sprintf(page + num, "GSW_VLAN_VID_0_1: %08X \n", GSW_VLAN_VID_0_1);
	num += sprintf(page + num, "VLAN0_VID: %d \n", VLAN0_VID);
	num += sprintf(page + num, "VLAN1_VID: %d \n", VLAN1_VID);

	num += sprintf(page + num, "GSW_QUEUE_STATUS_TEST_1  : %x \n", GSW_QUEUE_STATUS_TEST_1);
	num += sprintf(page + num, "GW_GSW_MAX_RFD_NUM  : %d \n", STAR_GSW_MAX_RFD_NUM);
	num += sprintf(page + num, "GW_GSW_MAX_TFD_NUM  : %d \n", STAR_GSW_MAX_TFD_NUM);
	num += sprintf(page + num, "GSW_INTERRUPT_STATUS  : %x \n", GSW_INTERRUPT_STATUS);

	num += sprintf(page + num, "MAC PORT 0   : %x \n", GSW_MAC_PORT_0_CONFIG);
	if (port & (0x1 << 22))
		num += sprintf(page + num, "  IVL: IVL\n");
	else
		num += sprintf(page + num, "  IVL: SVL\n");

	port = GSW_MAC_PORT_1_CONFIG;
	num += sprintf(page + num, "MAC PORT 1   : %x \n", GSW_MAC_PORT_1_CONFIG);
	if (port & (0x1 << 22))
		num += sprintf(page + num, "  IVL: IVL\n");
	else
		num += sprintf(page + num, "  IVL: SVL\n");

	num += sprintf(page + num, " CPU PORT 1   : %x \n", GSW_CPU_PORT_CONFIG);

	num += sprintf(page + num, "MODEL: %s\n", MODEL);
#ifdef STAR_GSW_TX_HW_CHECKSUM
	num += sprintf(page + num, "use TX hardware checksum\n");
#endif
#ifdef STAR_GSW_RX_HW_CHECKSUM
	num += sprintf(page + num, "use RX hardware checksum\n");
#endif

#ifdef CONFIG_STR9100_VLAN_BASE
	num += sprintf(page + num, "VLAN BASE\n");
  #ifdef CONFIG_HAVE_VLAN_TAG
	num += sprintf(page + num, "HAVE VLAN TAG\n");
  #else
	num += sprintf(page + num, "HAVE NO VLAN TAG\n");
  #endif
#endif

#ifdef CONFIG_STR9100_PORT_BASE
	num += sprintf(page + num, "PORT BASE\n");
#endif

// 20060922 descent
#ifdef CONFIG_NIC_MODE
	num += sprintf(page + num, "NIC MODE ON\n");
#else
	num += sprintf(page + num, "NIC MODE OFF\n");
#endif
// 20060922 descent end

#ifdef STAR_GSW_SG
	num += sprintf(page + num, "scatter gather on\n");
#else
	num += sprintf(page + num, "scatter gather off\n");
#endif

#ifdef FREE_TX_SKB_MULTI
	num += sprintf(page + num, "FREE_TX_SKB_MULTI on\n");
#else
	num += sprintf(page + num, "FREE_TX_SKB_MULTI off\n");
#endif

#ifdef STAR_GSW_TIMER
	num += sprintf(page + num, "STAR_GSW_TIMER on\n");
#else
	num += sprintf(page + num, "STAR_GSW_TIMER off\n");
#endif


#if 0
	num += sprintf(page + num, "  lan (eth0) mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
			star_gsw_info.vlan[1].vlan_mac[0],
			star_gsw_info.vlan[1].vlan_mac[1],
			star_gsw_info.vlan[1].vlan_mac[2],
			star_gsw_info.vlan[1].vlan_mac[3],
			star_gsw_info.vlan[1].vlan_mac[4],
			star_gsw_info.vlan[1].vlan_mac[5]);
	num += sprintf(page + num, "  wan (eth1) mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
			star_gsw_info.vlan[0].vlan_mac[0],
			star_gsw_info.vlan[0].vlan_mac[1],
			star_gsw_info.vlan[0].vlan_mac[2],
			star_gsw_info.vlan[0].vlan_mac[3],
			star_gsw_info.vlan[0].vlan_mac[4],
			star_gsw_info.vlan[0].vlan_mac[5]);
#endif

	return num;

#endif


	int num = 0;
	u32 port=0;
	const char *STR_ENABLE="Enable";
	const char *STR_DISABLE="Disable";


	num  = sprintf(page, "Star STR9100 Gigabit Switch Driver Information \n");

	num += sprintf(page + num, "%s\n", star_gsw_driver_version);
	num += sprintf(page + num, "Demo Board Name: %s\n", MODEL);
#ifdef CONFIG_STAR_GSW_NAPI
	num += sprintf(page + num, "NAPI Function : %s\n", STR_ENABLE);
#else
	num += sprintf(page + num, "NAPI Function : %s\n", STR_DISABLE);
#endif

	port = GSW_SWITCH_CONFIG;
	if (port & (0x1 << 22))
		num += sprintf(page + num, "Independent VLAN Learning(IVL) Enable\n");
	else
		num += sprintf(page + num, "Share VLAN Learning (SVL) Enable\n");

#ifdef CONFIG_STR9100_VLAN_BASE
	num += sprintf(page + num, "Support Tag Base VLAN , Receive packet ");
  #ifdef CONFIG_HAVE_VLAN_TAG
	num += sprintf(page + num, "with vlan tag\n");
  #else
	num += sprintf(page + num, "without vlan tag\n");
  #endif
#endif

#ifdef CONFIG_STR9100_PORT_BASE
	num += sprintf(page + num, "Support Port Base VLAN\n");
#endif


	num += sprintf(page + num, "Max Receive Ring Buffer:  %02d \n", STAR_GSW_MAX_RFD_NUM );
	num += sprintf(page + num, "Max Send Ring Buffer:     %02d\n",  STAR_GSW_MAX_TFD_NUM );
#ifdef STAR_GSW_TX_HW_CHECKSUM
	num += sprintf(page + num, "TX Hardware checksum:     %s \n", STR_ENABLE);
#else
	num += sprintf(page + num, "TX Hardware checksum:     %s \n", STR_DISABLE);
#endif
#ifdef STAR_GSW_RX_HW_CHECKSUM
	num += sprintf(page + num, "Rx Hardware checksum:     %s\n", STR_ENABLE );
#else
	num += sprintf(page + num, "Rx Hardware checksum:     %s\n", STR_DISABLE );
#endif

#ifndef STAR_GSW_DELAYED_INTERRUPT
	// Disable Delayed Interrupt
	num += sprintf(page + num, "Delay Interrupt %s\n",STR_DISABLE);
#else
	num += sprintf(page + num, "Delay Interrupt %s , Max Pending Interrupt Count: %d , Max Pending Timer : %d \n",
		       STR_ENABLE, MAX_PEND_INT_CNT, MAX_PEND_TIME);
#endif
	num += sprintf(page + num, "Group VID Info: GVID0_VID:%02X   GVID1_VID: %02X   GVID2_VID:%02X   GVID3_VID: %02X \n", 
				GSW_VLAN_VID_0_1&0xFFF, (GSW_VLAN_VID_0_1>>12)&0xFFF,
				GSW_VLAN_VID_2_3&0xFFF, (GSW_VLAN_VID_2_3>>12)&0xFFF);

	
	num += sprintf(page + num, "                GVID4_VID:%02X   GVID5_VID: %02X   GVID6_VID:%02X   GVID7_VID: %02X \n", 
				GSW_VLAN_VID_4_5&0xFFF, (GSW_VLAN_VID_4_5>>12)&0xFFF,
				GSW_VLAN_VID_6_7&0xFFF, (GSW_VLAN_VID_6_7>>12)&0xFFF);


	num += sprintf(page + num, "Int. Buffer free pages count : %x(%d)\n", 
				   GSW_QUEUE_STATUS_TEST_1&0xFF,GSW_QUEUE_STATUS_TEST_1&0xFF);
	num += sprintf(page + num, "Interrupt Status    : %x (Clean After Read)\n", GSW_INTERRUPT_STATUS);
	GSW_INTERRUPT_STATUS= GSW_INTERRUPT_STATUS;

	num += sprintf(page + num, "Switch Register: %x \n", GSW_SWITCH_CONFIG);
	num += sprintf(page + num, "MAC0 REG: %x (%s:%s,%s,%s,%s)\n", GSW_MAC_PORT_0_CONFIG,
			(GSW_MAC_PORT_0_CONFIG&(0x1<<18))==0?"Port Enable":"Port Disable",
			(GSW_MAC_PORT_0_CONFIG&(0x1<<7))!=0?"AN Enable":"AN Disable",
			(GSW_MAC_PORT_0_CONFIG&(0x11<<2))!=0x10?"1000Mbps":"10/100Mbps",
			(GSW_MAC_PORT_0_CONFIG&(0x1<<4))==0x0?"Half Duplex":"Full Duplex",
			(GSW_MAC_PORT_0_CONFIG&(0x1))!=0?"Link Up":"Link Down"
			);
	num += sprintf(page + num, "MAC1 REG: %x (%s:%s,%s,%s,%s)\n", GSW_MAC_PORT_1_CONFIG,
			(GSW_MAC_PORT_0_CONFIG&(0x1<<18))==0?"Port Enable":"Port Disable",
			(GSW_MAC_PORT_0_CONFIG&(0x1<<7))!=0?"AN Enable":"AN Disable",
			(GSW_MAC_PORT_0_CONFIG&(0x11<<2))!=0x10?"1000Mbps":"10/100Mbps",
			(GSW_MAC_PORT_0_CONFIG&(0x1<<4))==0x0?"Half Duplex":"Full Duplex",
			(GSW_MAC_PORT_0_CONFIG&(0x1))!=0?"Link Up":"Link Down"
			);

	num += sprintf(page + num, "CPU  REG: %x \n", GSW_CPU_PORT_CONFIG);
	num += sprintf(page + num, "GSW_BIST_RESULT_TEST_0: %x\n", GSW_BIST_RESULT_TEST_0);
#ifdef CONFIG_STR9100_VLAN_BASE
	num += sprintf(page + num, "VLAN BASE\n");
  #ifdef CONFIG_HAVE_VLAN_TAG
	num += sprintf(page + num, "HAVE VLAN TAG\n");
  #else
	num += sprintf(page + num, "HAVE NO VLAN TAG\n");
  #endif
#endif

#ifdef CONFIG_STR9100_PORT_BASE
	num += sprintf(page + num, "PORT BASE\n");
#endif

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
	num += sprintf(page + num, "8021Q support\n");
#else
	num += sprintf(page + num, "no 8021Q support\n");
#endif

#ifdef STAR_GSW_SG
	num += sprintf(page + num, "Scatter Gather on\n");
#else
	num += sprintf(page + num, "Scatter Gather off\n");
#endif

#ifdef FREE_TX_SKB_MULTI
	num += sprintf(page + num, "FREE_TX_SKB_MULTI on\n");
#else
	num += sprintf(page + num, "FREE_TX_SKB_MULTI off\n");
#endif

#ifdef STAR_GSW_TIMER
	num += sprintf(page + num, "STAR_GSW_TIMER on\n");
#else
	num += sprintf(page + num, "STAR_GSW_TIMER off\n");
#endif


	return num;
}

int star_gsw_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{

// 20061103 descent
#ifdef CONFIG_CONF_VID
	char *str, *pos;
	u16 gid, vid;

	str=buffer;

	if (count)
	{
		//simple_strtol();
		//printk("input str: %s\n", buffer);

		// skip blank
		while (*str==' ')
		{
			++str;
		}
		pos = strstr(str, " ");
		if (pos)
		{
			*pos='\0';
			//printk("str : %s\n", str);
			gid=simple_strtol(str, NULL, 10);
			//printk("gid : %d\n", gid);
		}

		str=(++pos);

		// skip blank
		while (*str==' ')
		{
			++str;
		}

		//pos = strstr(str, " ");
		//if (pos)
		{
			//*pos='\0';
			//printk("str : %s\n", str);
			vid=simple_strtol(str, NULL, 10);
			//printk("vid : %d\n", vid);
		}
		star_gsw_info.vlan[gid].vlan_vid=vid;
		switch (gid)
		{
			case 0:
			{
				GSW_SET_VLAN_0_VID(vid);
				break;
			}
			case 1:
			{
				GSW_SET_VLAN_1_VID(vid);
				break;
			}
			case 2:
			{
				GSW_SET_VLAN_2_VID(vid);
				break;
			}
			case 3:
			{
				GSW_SET_VLAN_3_VID(vid);
				break;
			}
			case 4:
			{
				GSW_SET_VLAN_4_VID(vid);
				break;
			}
			case 5:
			{
				GSW_SET_VLAN_5_VID(vid);
				break;
			}
			case 6:
			{
				GSW_SET_VLAN_6_VID(vid);
				break;
			}
			case 7:
			{
				GSW_SET_VLAN_7_VID(vid);
				break;
			}
		}



		printk("GSW_VLAN_VID_0_1: %x\n", GSW_VLAN_VID_0_1);
		printk("GSW_VLAN_VID_2_3: %x\n", GSW_VLAN_VID_2_3);
		printk("GSW_VLAN_VID_4_5: %x\n", GSW_VLAN_VID_4_5);
		printk("GSW_VLAN_VID_6_7: %x\n", GSW_VLAN_VID_6_7);
	}

#endif
// 20061103 descent end

// 20060922 descent
#ifdef CONFIG_NIC_MODE
       	u32 sw_config = GSW_SWITCH_CONFIG;

	// NIC mode on
	if (count && buffer[0]=='1') {
		sw_config |= (1 << 30);

		star_gsw_info.vlan[0].vlan_tag_flag	= 0;
		star_gsw_info.vlan[1].vlan_tag_flag	= 0;

		printk("NIC mode on\n");
	}

	// NIC mode off
	if (count && buffer[0]=='0') {
		sw_config &= ~(1 << 30);

		star_gsw_info.vlan[0].vlan_tag_flag	= VLAN0_VLAN_TAG;
		star_gsw_info.vlan[1].vlan_tag_flag	= VLAN1_VLAN_TAG;

		printk("NIC mode off\n");
	}
	GSW_SET_VLAN_0_TAG(star_gsw_info.vlan[0].vlan_tag_flag);
	GSW_SET_VLAN_1_TAG(star_gsw_info.vlan[1].vlan_tag_flag);

       	GSW_SWITCH_CONFIG = sw_config;
#endif
// 20060922 descent end

#ifdef CHANGE_DELAY_INT
	int i=0, j=0;
	int c=count;
	char *str=buffer;
	char str_num[5];
	unsigned long n[2];
	int index=0;
	const char cmd_on[]="delay_int_on";
	const char cmd_off[]="delay_int_off";

	while(*str==' ') {
		++str;
		--c;
	}
	PDEBUG("count: %d\n", count);
	PDEBUG("c: %d\n", c);
	if (strncmp(cmd_on, str, strlen(cmd_on))==0) {
		PDEBUG("delay int on\n");
		GSW_DELAYED_INTERRUPT_CONFIG |= (1 << 16) ;
		return count;
	}
	if (strncmp(cmd_off, str, strlen(cmd_off))==0) {
		PDEBUG("delay int off \n");
		GSW_DELAYED_INTERRUPT_CONFIG &= (~(0x1 << 16));
		return count;
	}
	for (i=0, j=0 ; i < c; ++i){
		if ( ('0' <= str[i] && str[i] <= '9') || ('a' <= str[i] && str[i] <= 'f') || ('A' <= str[i] && str[i] <= 'F'))
			str[j++]=str[i];
		else
		{
			str[j++]=0;
			n[index]=simple_strtoul(str, NULL, 16);
			PDEBUG("n: %x\n", n[index]);
			++index;
			PDEBUG("str: %s\n", str);
			j=0;
		}
	}
	//if (count && buffer[0]=='0')
	max_pend_int_cnt=n[0];
	max_pend_time=n[1];
#ifdef STAR_GSW_DELAYED_INTERRUPT
	GSW_DELAYED_INTERRUPT_CONFIG = (1 << 16) | (max_pend_int_cnt << 8) | (max_pend_time);
#endif

#endif

// add by descent, 2006/07/04
// ADJUSTMENT TX RX SKEW
#ifdef ADJUSTMENT_TX_RX_SKEW
	// adjust MAC port 0/1 RX/TX clock skew
	if (count && buffer[0]=='0')
	{
		printk("port 1 tx skew 0 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 30);
		GSW_BIST_RESULT_TEST_0 |= (0x0 << 30);

	}
	if (count && buffer[0]=='1')
	{
		printk("port 1 tx skew 1.5 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 30);
		GSW_BIST_RESULT_TEST_0 |= (0x1 << 30);
	}
	if (count && buffer[0]=='2')
	{
		printk("port 1 tx skew 2.0 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 30);
		GSW_BIST_RESULT_TEST_0 |= (0x2 << 30);
	}
	if (count && buffer[0]=='3')
	{
		printk("port 1 tx skew 2.5 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 30);
		GSW_BIST_RESULT_TEST_0 |= (0x3 << 30);
	}



	if (count && buffer[0]=='4')
	{
		printk("port 1 rx skew 0 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 28);
		GSW_BIST_RESULT_TEST_0 |= (0x0 << 28);
	}
	if (count && buffer[0]=='5')
	{
		printk("port 1 rx skew 1.5 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 28);
		GSW_BIST_RESULT_TEST_0 |= (0x1 << 28);
	}
	if (count && buffer[0]=='6')
	{
		printk("port 1 rx skew 2.0 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 28);
		GSW_BIST_RESULT_TEST_0 |= (0x2 << 28);
	}
	if (count && buffer[0]=='7')
	{
		printk("port 1 rx skew 2.5 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 28);
		GSW_BIST_RESULT_TEST_0 |= (0x3 << 28);
	}


	if (count && buffer[0]=='8')
	{
		printk("port 0 tx skew 0 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 26);
		GSW_BIST_RESULT_TEST_0 |= (0x0 << 26);
	}
	if (count && buffer[0]=='9')
	{
		printk("port 0 tx skew 1.5 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 26);
		GSW_BIST_RESULT_TEST_0 |= (0x1 << 26);
	}
	if (count && buffer[0]=='a')
	{
		printk("port 0 tx skew 2.0 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 26);
		GSW_BIST_RESULT_TEST_0 |= (0x2 << 26);
	}
	if (count && buffer[0]=='b')
	{
		printk("port 0 tx skew 2.5 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 26);
		GSW_BIST_RESULT_TEST_0 |= (0x3 << 26);
	}


	if (count && buffer[0]=='c')
	{
		printk("port 0 rx skew 0 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 24);
		GSW_BIST_RESULT_TEST_0 |= (0x0 << 24);
	}
	if (count && buffer[0]=='d')
	{
		printk("port 0 rx skew 1.5 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 24);
		GSW_BIST_RESULT_TEST_0 |= (0x1 << 24);
	}
	if (count && buffer[0]=='e')
	{
		printk("port 0 rx skew 2.0 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 24);
		GSW_BIST_RESULT_TEST_0 |= (0x2 << 24);
	}
	if (count && buffer[0]=='f')
	{
		printk("port 0 rx skew 2.5 ns\n");
                GSW_BIST_RESULT_TEST_0 &= ~(0x3 << 24);
		GSW_BIST_RESULT_TEST_0 |= (0x3 << 24);
	}

	printk("GSW_BIST_RESULT_TEST_0: %x\n", GSW_BIST_RESULT_TEST_0);
#endif

#ifdef STR9100_GSW_FAST_AGE_OUT_
	{
	// 00:02:A5:BE:59:AA
	u8 src_mac[6] = {0x00, 0x02, 0xa5, 0xbe, 0x59, 0xaa};
	int vlan_gid=1; // lan

	printk("src mac = %x:%x:%x:%x:%x:%x\n", *src_mac,*(src_mac+1), *(src_mac+2), *(src_mac+3), *(src_mac+4), *(src_mac+5));
	printk("vlan_gid : %d\n", vlan_gid);
	if (star_gsw_search_arl_table(src_mac, vlan_gid))
	{
		printk("find it\n");
	}
	else
	{
		printk("not found\n");
	}

	}

	{
	// 00:02:A5:BE:59:AA
	u8 src_mac[6] = {0x00, 0x02, 0xa5, 0xbe, 0x59, 0x99};
	int vlan_gid=1; // lan

	printk("src mac = %x:%x:%x:%x:%x:%x\n", *src_mac,*(src_mac+1), *(src_mac+2), *(src_mac+3), *(src_mac+4), *(src_mac+5));
	printk("vlan_gid : %d\n", vlan_gid);
	if (star_gsw_search_arl_table(src_mac, vlan_gid))
	{
		printk("find it\n");
	}
	else
	{
		printk("not found\n");
	}
	}
#endif
	return count;
}


static void star_gsw_enable(struct net_device *dev)
{
	GSW_FS_DMA_START();
	star_gsw_interrupt_enable();
}


static void star_gsw_shutdown(struct net_device *dev)
{
}

inline int star_gsw_search_arl_table(u8 *mac, u32 vlan_gid)
{
	volatile u32 lookup_result;

	GSW_ARL_TABLE_ACCESS_CONTROL_0 = 0x0;
	GSW_ARL_TABLE_ACCESS_CONTROL_1 = 0x0;
	GSW_ARL_TABLE_ACCESS_CONTROL_2 = 0x0;

	GSW_ARL_TABLE_ACCESS_CONTROL_2 =
		(((mac[2] & 0xFF) << 0)	|
		((mac[3] & 0xFF) << 8)	|
		((mac[4] & 0xFF) << 16)	|
		((mac[5] & 0xFF) << 24));

	GSW_ARL_TABLE_ACCESS_CONTROL_1 =
		((vlan_gid << 5)	|
		((mac[0] & 0xFF) << 16)	|
		((mac[1] & 0xFF) << 24) );

	GSW_ARL_TABLE_ACCESS_CONTROL_0 = (0x1 << 2);

	do {
		lookup_result = GSW_ARL_TABLE_ACCESS_CONTROL_1;
		// still search, bit2 and bit0
	} while ((lookup_result & 0x5) == 0); 

	if (lookup_result & (0x1 << 2)) {
		return 1;
	} else {
		return 0; // not found
	}
}

// add by descent 2006/07/03
// del arl entry
int star_gsw_del_arl_table(u8 *mac, u32 vlan_gid)
{
	volatile u32 age_field=0; // invalid mean erase this entry
	volatile u32 port_map=star_gsw_info.vlan[0].vlan_group; // invalid mean erase this entry
	volatile u32 result;
	
	GSW_ARL_TABLE_ACCESS_CONTROL_1 = 0x0;
	GSW_ARL_TABLE_ACCESS_CONTROL_2 = 0x0;

	GSW_ARL_TABLE_ACCESS_CONTROL_1 = ( ((vlan_gid & 0x7) << 5) |
                                           ((age_field & 0x7) << 8 ) | 
                                           ((port_map & 0x7) << 11 ) | 
                                           ((mac[0] & 0xFF) << 16) |
                                           ((mac[1] & 0xFF) << 24) );

	GSW_ARL_TABLE_ACCESS_CONTROL_2 = ( ((mac[2] & 0xFF) << 0)  |
                                           ((mac[3] & 0xFF) << 8)  |
                                           ((mac[4] & 0xFF) << 16) |
                                           ((mac[5] & 0xFF) << 24));


	GSW_ARL_TABLE_ACCESS_CONTROL_0 = 0x8; // write command
	do
	{
		result=GSW_ARL_TABLE_ACCESS_CONTROL_1;
	}while((result & 0x1)==0); 

	return 0;
}

void star_gsw_hnat_write_vlan_src_mac(u8 index, u8 *vlan_src_mac)
{
	switch (index) {
	case 0:
		GSW_HNAT_SOURCE_MAC_0_HIGH = (vlan_src_mac[0] << 8) |
			(vlan_src_mac[1] << 0);

		GSW_HNAT_SOURCE_MAC_0_LOW = (vlan_src_mac[2] << 24) |
			(vlan_src_mac[3] << 16) |
			(vlan_src_mac[4] << 8) |
			(vlan_src_mac[5] << 0);
		break;

	case 1:
		GSW_HNAT_SOURCE_MAC_1_HIGH = (vlan_src_mac[0] << 8) |
			(vlan_src_mac[1] << 0);

		GSW_HNAT_SOURCE_MAC_1_LOW = (vlan_src_mac[2] << 24) |
			(vlan_src_mac[3] << 16) |
			(vlan_src_mac[4] << 8) |
			(vlan_src_mac[5] << 0);
		break;

	case 2:
		GSW_HNAT_SOURCE_MAC_2_HIGH = (vlan_src_mac[0] << 8) |
			(vlan_src_mac[1] << 0);

		GSW_HNAT_SOURCE_MAC_2_LOW = (vlan_src_mac[2] << 24) |
			(vlan_src_mac[3] << 16) |
			(vlan_src_mac[4] << 8) |
			(vlan_src_mac[5] << 0);
		break;

	case 3:
		GSW_HNAT_SOURCE_MAC_3_HIGH = (vlan_src_mac[0] << 8) |
			(vlan_src_mac[1] << 0);

		GSW_HNAT_SOURCE_MAC_3_LOW = (vlan_src_mac[2] << 24) |
			(vlan_src_mac[3] << 16) |
			(vlan_src_mac[4] << 8) |
			(vlan_src_mac[5] << 0);
		break;

	case 4:
		GSW_HNAT_SOURCE_MAC_4_HIGH = (vlan_src_mac[0] << 8) |
			(vlan_src_mac[1] << 0);

		GSW_HNAT_SOURCE_MAC_4_LOW = (vlan_src_mac[2] << 24) |
			(vlan_src_mac[3] << 16) |
			(vlan_src_mac[4] << 8) |
			(vlan_src_mac[5] << 0);
		break;

	case 5:
		GSW_HNAT_SOURCE_MAC_5_HIGH = (vlan_src_mac[0] << 8) |
			(vlan_src_mac[1] << 0);

		GSW_HNAT_SOURCE_MAC_5_LOW = (vlan_src_mac[2] << 24) |
			(vlan_src_mac[3] << 16) |
			(vlan_src_mac[4] << 8) |
			(vlan_src_mac[5] << 0);
		break;

	case 6:
		GSW_HNAT_SOURCE_MAC_6_HIGH = (vlan_src_mac[0] << 8) |
			(vlan_src_mac[1] << 0);

		GSW_HNAT_SOURCE_MAC_6_LOW = (vlan_src_mac[2] << 24) |
			(vlan_src_mac[3] << 16) |
			(vlan_src_mac[4] << 8) |
			(vlan_src_mac[5] << 0);
		break;

	case 7:
		GSW_HNAT_SOURCE_MAC_7_HIGH = (vlan_src_mac[0] << 8) |
			(vlan_src_mac[1] << 0);

		GSW_HNAT_SOURCE_MAC_7_LOW = (vlan_src_mac[2] << 24) |
			(vlan_src_mac[3] << 16) |
			(vlan_src_mac[4] << 8) |
			(vlan_src_mac[5] << 0);
		break;

	default:
		break;
	}
}

static int star_gsw_hnat_setup_vlan_src_mac(void)
{
	star_gsw_hnat_write_vlan_src_mac(0, star_gsw_info.vlan[0].vlan_mac);
	star_gsw_hnat_write_vlan_src_mac(1, star_gsw_info.vlan[1].vlan_mac);
	star_gsw_hnat_write_vlan_src_mac(2, star_gsw_info.vlan[2].vlan_mac);
	star_gsw_hnat_write_vlan_src_mac(3, star_gsw_info.vlan[3].vlan_mac);

	return 0;
}

static void star_gsw_vlan_init(void)
{
	star_gsw_info.vlan[0].vlan_gid		= VLAN0_GROUP_ID;
	star_gsw_info.vlan[0].vlan_vid		= VLAN0_VID;
	star_gsw_info.vlan[0].vlan_group	= VLAN0_GROUP;
	star_gsw_info.vlan[0].vlan_tag_flag	= VLAN0_VLAN_TAG;
	printk("VLAN0_VLAN_TAG: %x\n", VLAN0_VLAN_TAG);

	// store My VLAN0 MAC
	memcpy(star_gsw_info.vlan[0].vlan_mac, my_vlan0_mac, 6);

	star_gsw_info.vlan[1].vlan_gid		= VLAN1_GROUP_ID;
	star_gsw_info.vlan[1].vlan_vid		= VLAN1_VID;
	star_gsw_info.vlan[1].vlan_group	= VLAN1_GROUP;
	star_gsw_info.vlan[1].vlan_tag_flag	= VLAN1_VLAN_TAG;

	// store My VLAN1 MAC
	memcpy(star_gsw_info.vlan[1].vlan_mac, my_vlan1_mac, 6);

	star_gsw_info.vlan[2].vlan_gid		= VLAN2_GROUP_ID;
	star_gsw_info.vlan[2].vlan_vid		= VLAN2_VID;
	star_gsw_info.vlan[2].vlan_group	= VLAN2_GROUP;
	star_gsw_info.vlan[2].vlan_tag_flag	= VLAN2_VLAN_TAG;

	// store My VLAN2 MAC
	memcpy(star_gsw_info.vlan[2].vlan_mac, my_vlan2_mac, 6);

	star_gsw_info.vlan[3].vlan_gid		= VLAN3_GROUP_ID;
	star_gsw_info.vlan[3].vlan_vid		= VLAN3_VID;
	star_gsw_info.vlan[3].vlan_group	= VLAN3_GROUP;
	star_gsw_info.vlan[3].vlan_tag_flag	= VLAN3_VLAN_TAG;

	// store My VLAN3 MAC
	memcpy(star_gsw_info.vlan[3].vlan_mac, my_vlan3_mac, 6);

	star_gsw_info.vlan[4].vlan_gid		= VLAN4_GROUP_ID;
	star_gsw_info.vlan[4].vlan_vid		= VLAN4_VID;
	star_gsw_info.vlan[4].vlan_group	= VLAN4_GROUP;
	star_gsw_info.vlan[4].vlan_tag_flag	= VLAN4_VLAN_TAG;

	star_gsw_info.vlan[5].vlan_gid		= VLAN5_GROUP_ID;
	star_gsw_info.vlan[5].vlan_vid		= VLAN5_VID;
	star_gsw_info.vlan[5].vlan_group	= VLAN5_GROUP;
	star_gsw_info.vlan[5].vlan_tag_flag	= VLAN5_VLAN_TAG;

	star_gsw_info.vlan[6].vlan_gid		= VLAN6_GROUP_ID;
	star_gsw_info.vlan[6].vlan_vid		= VLAN6_VID;
	star_gsw_info.vlan[6].vlan_group	= VLAN6_GROUP;
	star_gsw_info.vlan[6].vlan_tag_flag	= VLAN6_VLAN_TAG; 

	star_gsw_info.vlan[7].vlan_gid		= VLAN7_GROUP_ID;
	star_gsw_info.vlan[7].vlan_vid		= VLAN7_VID;
	star_gsw_info.vlan[7].vlan_group	= VLAN7_GROUP;
	star_gsw_info.vlan[7].vlan_tag_flag	= VLAN7_VLAN_TAG;

	star_gsw_info.port[0].pvid		= PORT0_PVID;
	star_gsw_info.port[0].config_flag	= 0;
	star_gsw_info.port[0].status_flag	= 0;

	star_gsw_info.port[1].pvid		= PORT1_PVID;
	star_gsw_info.port[1].config_flag	= 0;
	star_gsw_info.port[1].status_flag	= 0;

	star_gsw_info.port[2].pvid		= CPU_PORT_PVID;
	star_gsw_info.port[2].config_flag	= 0;
	star_gsw_info.port[2].status_flag	= 0;   
}

IRQ_RETURN star_gsw_receive_isr(int irq, void *dev_id, struct pt_regs *regs)
{
#ifdef CONFIG_STAR_GSW_NAPI
	INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GSW_FSRC_BIT_INDEX);
	disable_irq(INTC_GSW_FSRC_BIT_INDEX);


        if (likely(netif_rx_schedule_prep(&STAR_NAPI_DEV))) {
                __netif_rx_schedule(&STAR_NAPI_DEV);
	} else {
                enable_irq(INTC_GSW_FSRC_BIT_INDEX);
        }

#if 0
	CUR_NAPI_DEV=STAR_GSW_WAN_DEV;

	if (CUR_NAPI_DEV && netif_running(CUR_NAPI_DEV)) {
		if (likely(netif_rx_schedule_prep(CUR_NAPI_DEV))) {
			__netif_rx_schedule(CUR_NAPI_DEV);
		} else {
			PDEBUG("lan driver bug! interrupt while in poll\n");
		}
	}
	else
	{
		CUR_NAPI_DEV=STAR_GSW_LAN_DEV;
		if (CUR_NAPI_DEV && netif_running(CUR_NAPI_DEV)) {
			if (likely(netif_rx_schedule_prep(CUR_NAPI_DEV))) {
				__netif_rx_schedule(CUR_NAPI_DEV);
			} else {
				PDEBUG("lan driver bug! interrupt while in poll\n");
			}
		}
		else
		{
			CUR_NAPI_DEV=STAR_GSW_EWC_DEV;
			if (CUR_NAPI_DEV && netif_running(CUR_NAPI_DEV)) {
				if (likely(netif_rx_schedule_prep(CUR_NAPI_DEV))) {
					__netif_rx_schedule(CUR_NAPI_DEV);
				} else {
					PDEBUG("lan driver bug! interrupt while in poll\n");
				}
			}

		}
	}
#endif
#else
	// TODO: mask interrupt
	INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GSW_FSRC_BIT_INDEX);
	// MASK Interrupt
	INTC_INTERRUPT_MASK |= (0x1 << INTC_GSW_FSRC_BIT_INDEX);
	INTC_INTERRUPT_MASK |= (0x1 << INTC_GSW_FSQF_BIT_INDEX);
	++fsrc_count;
	star_gsw_receive_packet(0); // Receive Once
	// TODO: unmask interrupt
	INTC_INTERRUPT_MASK &= ~(0x1 << INTC_GSW_FSRC_BIT_INDEX);
	INTC_INTERRUPT_MASK &= ~(0x1 << INTC_GSW_FSQF_BIT_INDEX);
#endif

	return IRQ_HANDLED;
}


#ifdef STAR_GSW_FSQF_ISR
IRQ_RETURN star_gsw_fsqf_isr(int irq, void *dev_id, struct pt_regs *regs)
{
	INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GSW_FSQF_BIT_INDEX);
#ifdef CONFIG_STAR_GSW_NAPI
	// because in normal state, fsql only invoke once and set_bit is atomic function.
	// so I don't mask it.
	set_bit(0, &is_qf);
#else
	INTC_INTERRUPT_MASK |= (0x1 << INTC_GSW_FSRC_BIT_INDEX);
	INTC_INTERRUPT_MASK |= (0x1 << INTC_GSW_FSQF_BIT_INDEX);

	star_gsw_receive_packet(1); // Receive at Queue Full Mode

	// TODO: unmask interrupt
	INTC_INTERRUPT_MASK &= (0x0 << INTC_GSW_FSRC_BIT_INDEX);
	INTC_INTERRUPT_MASK &= (0x0 << INTC_GSW_FSQF_BIT_INDEX);
#endif

	return IRQ_HANDLED;
}
#endif

#ifdef STAR_GSW_STATUS_ISR
static char *star_gsw_status_tbl[] = {
	"\nGlobal threshold reached and Port 0 queue threshold reached.\n",
	"\nGlobal threshold reached and Port 1 queue threshold reached.\n",
	"\nGlobal threshold reached and CPU port queue threshold reached.\n",
	"\nGlobal threshold reached and HNAT queue threshold reached.\n",
	"\nGlobal threshold reached.\n",
	"\nAll pages of packet buffer are used.\n",
	"\nPort change link state.\n",
	"\nPort 0 received intruder packets.\n",
	"\nPort 1 received intruder packets.\n",
	"\n",
	"\nPort 0 received packets with unknown VLAN.\n",
	"\nPort 1 received packets with unknown VLAN.\n",
	"\nPort CPU received packets with unknown VLAN.\n",
	"\n",
	"\n",
	"\n",
	"\nDrop by no free links(Port 0).\n",
	"\nDrop by broadcast storm(Port 0).\n",
	"\nDrop by rx packet error(Port 0).\n",
	"\nDrop by backpressure(Port 0).\n",
	"\nDrop by no destination(Port 0).\n",
	"\nDrop by reserved MC packets(Port 0).\n",
	"\nDrop by local traffic(Port 0).\n",
	"\nDrop by ingress check(Port 0).\n",
	"\nDrop by no free links(Port 1).\n",
	"\nDrop by broadcast storm(Port 1).\n",
	"\nDrop by rx packet error(Port 1).\n",
	"\nDrop by backpressure(Port 1).\n",
	"\nDrop by no destination(Port 1).\n",
	"\nDrop by reserved MC packets(Port 1).\n",
	"\nDrop by local traffic(Port 1).\n",
	"\nDrop by ingress checki(Port 1).\n",
};

IRQ_RETURN star_gsw_status_isr(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 int_status;
	u32 i;

	INTC_DISABLE_INTERRUPT_SOURCE(INTC_GSW_STATUS_BIT_INDEX);

	GSW_READ_INTERRUPT_STATUS(int_status);

	PDEBUG("\n status:%08X \n",int_status);
	PDEBUG("\n GSW_MAC_PORT_0_CONFIG:%08X\n",GSW_MAC_PORT_0_CONFIG);
	PDEBUG("\n GSW_MAC_PORT_1_CONFIG:%08X\n",GSW_MAC_PORT_1_CONFIG);

	for (i = 0; i < 32; i++) {
		if (int_status & (1 << i)) {
			PRINT_INFO(star_gsw_status_tbl[i]);
		}
	}

	GSW_CLEAR_INTERRUPT_STATUS_SOURCES(int_status);

	INTC_ENABLE_INTERRUPT_SOURCE(INTC_GSW_STATUS_BIT_INDEX);

	return IRQ_HANDLED;

}
#endif // STAR_GSW_STATUS_ISR

static int star_gsw_uninstall_isr(struct net_device *dev)
{
	--install_isr_account;
	if (install_isr_account == 0) {
		PDEBUG("star gsw uninstall isr\n");
		free_irq(INTC_GSW_FSRC_BIT_INDEX, STAR_GSW_LAN_DEV);

#ifdef STAR_GSW_FSQF_ISR
		free_irq(INTC_GSW_FSQF_BIT_INDEX, STAR_GSW_LAN_DEV);
#endif

#ifdef STAR_GSW_STATUS_ISR
		free_irq(INTC_GSW_STATUS_BIT_INDEX, STAR_GSW_LAN_DEV);
#endif


#ifdef CONFIG_STAR_GSW_NAPI
                netif_poll_disable(&STAR_NAPI_DEV);
#endif

	}

	return 0;
}

static int star_gsw_install_isr(struct net_device *dev)
{
	int retval;

	
	if (install_isr_account == 0) {
#ifdef STAR_GSW_DELAYED_INTERRUPT
		GSW_DELAYED_INTERRUPT_CONFIG = (1 << 16) | (max_pend_int_cnt << 8) | (max_pend_time);
#endif
#ifdef STAR_GSW_STATUS_ISR
		str9100_set_interrupt_trigger(INTC_GSW_STATUS_BIT_INDEX, INTC_LEVEL_TRIGGER, INTC_ACTIVE_HIGH);
#endif
		str9100_set_interrupt_trigger((u32)INTC_GSW_FSRC_BIT_INDEX, (u32)INTC_EDGE_TRIGGER, (u32)INTC_RISING_EDGE);
#ifdef STAR_GSW_FSQF_ISR
		str9100_set_interrupt_trigger(INTC_GSW_FSQF_BIT_INDEX, INTC_EDGE_TRIGGER, INTC_RISING_EDGE);
#endif

		retval = request_irq(INTC_GSW_FSRC_BIT_INDEX, &star_gsw_receive_isr, SA_SHIRQ, "GSW FSRC INT", STAR_GSW_LAN_DEV);

		if (retval) {
			PRINT_INFO("%s: unable to get IRQ %d (irqval=%d).\n", "GSW FSRC INT", INTC_GSW_FSRC_BIT_INDEX, retval);
			return 1;
		}

#ifdef STAR_GSW_FSQF_ISR
		/*  QUEUE full interrupt handler */
		retval = request_irq(INTC_GSW_FSQF_BIT_INDEX, &star_gsw_fsqf_isr, SA_INTERRUPT, "GSW FSQF INT", STAR_GSW_LAN_DEV);

		if (retval) {
			PRINT_INFO("%s: unable to get IRQ %d (irqval=%d).\n", "GSW FSQF INT", INTC_GSW_FSQF_BIT_INDEX, retval);
			return 2;
		}
#endif	

#ifdef STAR_GSW_STATUS_ISR
		/*  GSW Status interrupt handler */
		retval = request_irq(INTC_GSW_STATUS_BIT_INDEX, &star_gsw_status_isr, SA_INTERRUPT, "GSW STATUS", STAR_GSW_LAN_DEV);

		if (retval) {
			PRINT_INFO("%s: unable to get IRQ %d (irqval=%d).\n", "GSW STATUS INT", INTC_GSW_STATUS_BIT_INDEX, retval);
			return 3;
		}
		GSW_ENABLE_ALL_INTERRUPT_STATUS_SOURCES();
#endif
	} // end if(install_isr_account == 0)

	++install_isr_account;

	return 0;
}

// add by descent 2006/07/12
void enable_cpu_port(int y)
{
	u32 cpu_port_config;
	cpu_port_config = GSW_CPU_PORT_CONFIG;		
	if (y==1) // enable CPU
		cpu_port_config &= ~(0x1 << 18);
	if (y==0) // disable CPU
		cpu_port_config |= (0x1 << 18);
	GSW_CPU_PORT_CONFIG = cpu_port_config;
}

static int star_gsw_open(struct net_device *dev)
{
	OPEN_PORT(dev)
	enable_cpu_port(1);
	//memcpy(dev->dev_addr, star_gsw_info.vlan[1].vlan_mac, 6);

	star_gsw_hnat_setup_vlan_src_mac();

#ifdef MODULE
	MOD_INC_USE_COUNT;
#endif

#ifdef CONFIG_STAR_GSW_NAPI
        netif_poll_enable(&STAR_NAPI_DEV);
#endif


	//CUR_NAPI_DEV = dev;

	star_gsw_enable(dev);

	netif_start_queue(dev);

	GSW_ENABLE_ALL_INTERRUPT_STATUS_SOURCES();

	star_gsw_install_isr(dev);

	return 0;
}



static struct net_device_stats *star_gsw_get_stats(struct net_device *dev)
{
	struct star_gsw_private *priv = netdev_priv(dev);

	return &priv->stats;
}

static void star_gsw_timeout(struct net_device *dev)
{
	PRINT_INFO("%s:star_gsw_timeout\n", dev->name);
	star_gsw_enable(dev);
	netif_wake_queue(dev);
	dev->trans_start = jiffies;
}



static int star_gsw_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	star_gsw_uninstall_isr(dev);
	//star_gsw_shutdown(dev);

	//CLOSE_PORT0
	//CLOSE_PORT1
	CLOSE_PORT(dev)

#ifdef MODULE
	MOD_DEC_USE_COUNT;
#endif

#if 0
	if (dev == STAR_GSW_WAN_DEV) {
		CUR_NAPI_DEV = STAR_GSW_LAN_DEV;
	} else if (dev == STAR_GSW_LAN_DEV) {
		CUR_NAPI_DEV = STAR_GSW_WAN_DEV;
	} else if (dev == STAR_GSW_EWC_DEV) {
		CUR_NAPI_DEV = STAR_GSW_LAN_DEV;
	}
	//} // if (dev == STAR_GSW_LAN_DEV)
#endif

	//phy_power_down_ptr(1,1);
	return 0;
}

static inline struct sk_buff *star_gsw_alloc_skb(void)
{
	struct sk_buff *skb;

	skb = dev_alloc_skb(MAX_PACKET_LEN + 2);

	if (unlikely(!skb)) {
		PDEBUG("\n dev_alloc_skb fail!! while allocate RFD ring !!\n");
		return NULL;
	}


	/* Make buffer alignment 2 beyond a 16 byte boundary
	 * this will result in a 16 byte aligned IP header after
	 * the 14 byte MAC header is removed
	 */
	
	skb_reserve(skb, 2);	/* 16 bit alignment, 4N + 2 mode */

	return skb;
}

static void star_gsw_buffer_free(void)
{
	int i;

	if (rxring.vir_addr) {
		for (i = 0; i < STAR_GSW_MAX_RFD_NUM; i++) {
			if (rxring.skb_ptr[i]) {
				dev_kfree_skb(rxring.skb_ptr[i]);
			}
		}
		pci_free_consistent(NULL, STAR_GSW_MAX_RFD_NUM * sizeof(STAR_GSW_RXDESC), rxring.vir_addr, rxring.phy_addr);
	}

	if (txring.vir_addr) {
		pci_free_consistent(NULL, STAR_GSW_MAX_TFD_NUM * sizeof(STAR_GSW_TXDESC), txring.vir_addr, txring.phy_addr);
	}
}

static int __init star_gsw_buffer_alloc(void)
{
	STAR_GSW_RXDESC	volatile *rxdesc_ptr;
	STAR_GSW_TXDESC	volatile *txdesc_ptr;
	struct sk_buff	*skb_ptr;
	int err;
	int i;

	rxring.vir_addr = pci_alloc_consistent(NULL, STAR_GSW_MAX_RFD_NUM * sizeof(STAR_GSW_RXDESC), &rxring.phy_addr);
	if (!rxring.vir_addr) {
		PDEBUG("\n ERROR: Allocate RFD Failed\n");
		err = -ENOMEM;
		goto err_out;
	}

	txring.vir_addr = pci_alloc_consistent(NULL, STAR_GSW_MAX_TFD_NUM * sizeof(STAR_GSW_TXDESC), &txring.phy_addr);
	if (!txring.vir_addr) {
		PDEBUG("\n ERROR: Allocate TFD Failed\n");
		err = -ENOMEM;
		goto err_out;
	}

	// Clean RX Memory
	memset((void *)rxring.vir_addr, 0, STAR_GSW_MAX_RFD_NUM * sizeof(STAR_GSW_RXDESC));
	PDEBUG("    rxring.vir_addr=0x%08X rxring.phy_addr=0x%08X\n", (u32)rxring.vir_addr, (u32)rxring.phy_addr);
	rxring.cur_index = 0;	// Set cur_index Point to Zero
	rxdesc_ptr = rxring.vir_addr;
	for (i = 0; i < STAR_GSW_MAX_RFD_NUM; i++, rxdesc_ptr++) {
		if (i == (STAR_GSW_MAX_RFD_NUM - 1)) { 
			rxdesc_ptr->eor = 1;	// End bit == 0;
		}
		skb_ptr = star_gsw_alloc_skb();
		if (!skb_ptr) {
			PDEBUG("ERROR: Allocate skb Failed!\n");
			err = -ENOMEM;
			goto err_out;
		}
		// Trans Packet from Virtual Memory to Physical Memory
		rxring.skb_ptr[i]	= skb_ptr;
		rxdesc_ptr->data_ptr	= (u32)virt_to_phys(skb_ptr->data);
		rxdesc_ptr->length	= MAX_PACKET_LEN;
	}

	// Clean TX Memory
	memset((void *)txring.vir_addr, 0, STAR_GSW_MAX_TFD_NUM * sizeof(STAR_GSW_TXDESC));
	PDEBUG("    txring.vir_addr=0x%08X txring.phy_addr=0x%08X\n", (u32)txring.vir_addr, (u32)txring.phy_addr);
	txring.cur_index = 0;	// Set cur_index Point to Zero
	txdesc_ptr = txring.vir_addr;
	for (i = 0; i < STAR_GSW_MAX_TFD_NUM; i++, txdesc_ptr++) {
		if (i == (STAR_GSW_MAX_TFD_NUM - 1)) { 
			txdesc_ptr->eor = 1;	// End of Ring ==1
		}
		txdesc_ptr->cown = 1;	// TX Ring , Cown == 1

#ifdef STAR_GSW_TX_HW_CHECKSUM
		// Enable Checksum
		txdesc_ptr->ico		= 0;
		txdesc_ptr->uco		= 1;
		txdesc_ptr->tco		= 1;
#else
		txdesc_ptr->ico		= 0;
		txdesc_ptr->uco		= 0;
		txdesc_ptr->tco		= 0;
#endif
		txring.skb_ptr[i] 	= NULL;	// clear txring.skb_ptr
	}

	return 0;

err_out:
	star_gsw_buffer_free();
	return err;
}

#ifdef CONFIG_STAR_GSW_NAPI

//#ifdef LINUX24
static int star_gsw_poll(struct net_device *netdev, int *budget)
{
	int work_done = 0;
	int work_to_do = min(*budget, netdev->quota); // where is min define

	star_gsw_receive_packet(0, &work_done, work_to_do);

	*budget -= work_done;
	netdev->quota -= work_done;

#if 0
	/* if no Tx and not enough Rx work done, exit the polling mode */
	if (work_done < work_to_do) {
		if (test_bit(0, &is_qf) == 1) { // queue full
			clear_bit(0, &is_qf);
			GSW_FS_DMA_START();
			return 1;
		} else {
			netif_rx_complete(CUR_NAPI_DEV);
			enable_irq(INTC_GSW_FSRC_BIT_INDEX);
			return 0;
		}
	}
#else
        if (work_done) {
                if (is_qf) {
                        is_qf = 0;
                        HAL_GSW_FS_DMA_START();
                        return 1;
                }
        }
        else {
                netif_rx_complete(&STAR_NAPI_DEV);
                enable_irq(INTC_GSW_FSRC_BIT_INDEX);
                return 0;
        }

#endif

	return 1;
}
//#endif // LINUX24


#ifdef LINUX26_
#ifdef CONFIG_CPU_ISPAD_ENABLE
__attribute__((section(".ispad")))
#endif
static int star_gsw_poll(struct net_device *netdev, int *budget)
{
	int work_done = 0;
	int work_to_do = min(*budget, netdev->quota); // where is min define

	star_gsw_receive_packet(0, &work_done, work_to_do);

	*budget -= work_done;
	netdev->quota -= work_done;

#if 0
	/* if no Tx and not enough Rx work done, exit the polling mode */
#if 1
	if (work_done == 0) {
#else
	if (work_done <= work_to_do) {
#endif
		if (is_qf) {
			is_qf = 0;
			HAL_GSW_FS_DMA_START();
			return 1;
		}
		else {

			//if (work_done == 0) {
				netif_rx_complete(&STAR_NAPI_DEV);
				enable_irq(INTC_GSW_FSRC_BIT_INDEX);
				return 0;
			//}
		}
	}
#else
	if (work_done) {
		if (is_qf) {
			is_qf = 0;
			HAL_GSW_FS_DMA_START();
			return 1;
		}				
	}
        else {
		netif_rx_complete(&STAR_NAPI_DEV);
		enable_irq(INTC_GSW_FSRC_BIT_INDEX);
		return 0;
	}

#endif
	return 1;
}

#endif // LINUX26





#endif // CONFIG_STAR_GSW_NAPI

static int star_gsw_get_rfd_buff(int index)
{
	struct star_gsw_private *priv;
	STAR_GSW_RXDESC volatile *rxdesc_ptr;
	struct sk_buff *skb_ptr;
	unsigned char *data;
	int len;
#ifdef CONFIG_STR9100_VLAN_BASE
	u32 vlan_gid = 0;
	u8 *src_mac;
#endif

	rxdesc_ptr = rxring.vir_addr + index;
	skb_ptr = rxring.skb_ptr[index];
	len = rxdesc_ptr->length;

	consistent_sync(skb_ptr->data, len, PCI_DMA_FROMDEVICE);

#ifdef CONFIG_STR9100_PORT_BASE
	if (rxdesc_ptr->sp == 0) {
		/*
		 * Note this packet is from GSW Port 0, and the device index of GSW Port 0 is 1
		 * Note the device index = 0 is for internal loopback device
		 */
		//skb_ptr->dev = STAR_GSW_LAN_DEV;
		skb_ptr->dev = NET_DEV0;
		if (skb_ptr->dev)
			goto determine_dev_ok;
	} else {
		// Note this packet is from GSW Port 1, and the device index of GSW Port 1 is 2
		//skb_ptr->dev = STAR_GSW_WAN_DEV;
		skb_ptr->dev = NET_DEV1;
		if (skb_ptr->dev)
			goto determine_dev_ok;
	}

#endif /* CONFIG_STR9100_PORT_BASE */

#ifdef CONFIG_STAR9100_SHNAT_PCI_FASTPATH
	if (rxdesc_ptr->hr == 0x27 && star9100_shnat_hook_ready) {
		if(star9100_shnat_pci_fp_getdev_hook(skb_ptr)){
			skb_put(skb_ptr, len);

#if CONFIG_HAVE_VLAN_TAG
	#define PPPOE_ID_1_LOC 16
	#define PPPOE_ID_2_LOC 17
#else
	#define PPPOE_ID_1_LOC 12
	#define PPPOE_ID_2_LOC 13
#endif

			if (skb_ptr->data[PPPOE_ID_1_LOC] == 0x88 && skb_ptr->data[PPPOE_ID_2_LOC]==0x64) { // pppoe session 
				/* Remove PPPoE Header */
                memmove(skb_ptr->data+8, skb_ptr->data, 12); 
                skb_ptr->data+=8; 
                skb_ptr->len-=8; 
                skb_ptr->data[PPPOE_ID_1_LOC]=0x08; 
                skb_ptr->data[PPPOE_ID_2_LOC]=0x0; 
       		 } else { 
				/* Remove VLAN Tag */
#ifdef CONFIG_HAVE_VLAN_TAG
				memmove(skb_ptr->data + 4, skb_ptr->data, 12);
				skb_ptr->len-=4;
				skb_ptr->data+=4;
#endif
			} 
			skb_ptr->dev->hard_start_xmit(skb_ptr, skb_ptr->dev);
			return 0;
			}
	}
#endif /* CONFIG_STAR9100_SHNAT_PCI_FASTPATH */

RECV_PACKET:

#ifdef CONFIG_STR9100_VLAN_BASE

#ifdef CONFIG_HAVE_VLAN_TAG


// 20060922 descent
	{ // NIC MODE off

		const char lan_tag[]={0x81, 0x00, 0x00, 0x01};
		const char wan_tag[]={0x81, 0x00, 0x00, 0x02};



		if (memcmp(skb_ptr->data+12, lan_tag,4)==0) {
			//printk("lan dev\n");
			skb_ptr->dev = STAR_GSW_LAN_DEV;
		} else if (memcmp(skb_ptr->data+12, wan_tag,4)==0) {
			//printk("wan dev\n");
			skb_ptr->dev = STAR_GSW_WAN_DEV;
		} else {
			PDEBUG("no vlan tag\n");
			//print_packet(skb_ptr->data, 32);
			goto freepacket;

		}
// 20070503 descent
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
		// let 8021Q to determine vlan tag
#else  /* defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE) */
		memmove(skb_ptr->data + 4, skb_ptr->data, 12);
		//skb_ptr->data += 4; 
		skb_reserve(skb_ptr, 4);
		len -= 4; // minus 4 byte vlan tag
#endif /* defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE) */

#else  /* CONFIG_HAVE_VLAN_TAG */
	
// 20060922 descent
#ifdef CONFIG_NIC_MODE
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
// do nothing
#else // defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
	// if NIC MODE on
        if ( ((GSW_SWITCH_CONFIG >> 30) & 0x1) ==1){
		const char lan_tag[]={0x81, 0x00, 0x00, 0x01};
		const char wan_tag[]={0x81, 0x00, 0x00, 0x02};

		//printk("NIC mode on\n");
		if (memcmp(skb_ptr->data+12, lan_tag,4)==0) {
			//printk("lan dev\n");
			skb_ptr->dev = STAR_GSW_LAN_DEV;
		} else if (memcmp(skb_ptr->data+12, wan_tag,4)==0) {
			//printk("wan dev\n");
			skb_ptr->dev = STAR_GSW_WAN_DEV;
		} else {
			//printk("no vlan tag\n");
			//print_packet(skb_ptr->data, 32);
			goto freepacket;

		}

		memmove(skb_ptr->data + 4, skb_ptr->data, 12);
		skb_ptr->data += 4; 
		goto determine_dev_ok;

	}
#endif // defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#endif
// 20060922 descent end

	src_mac = skb_ptr->data + 6; // get source mac address

	// use gid and source mac to serarch arl table
	vlan_gid = 1; // lan
	if (star_gsw_search_arl_table(src_mac, vlan_gid)) {
		//printk("STAR_GSW_LAN_DEV\n");
		skb_ptr->dev = STAR_GSW_LAN_DEV;
		#ifdef STR9100_GSW_FAST_AGE_OUT
		star_gsw_del_arl_table(src_mac, vlan_gid);
		#endif
		goto determine_dev_ok;
	} else {
		vlan_gid = 0; // wan
		if (star_gsw_search_arl_table(src_mac, vlan_gid)) {
			//printk("STAR_GSW_WAN_DEV\n");
			skb_ptr->dev = STAR_GSW_WAN_DEV;
			#ifdef STR9100_GSW_FAST_AGE_OUT
			star_gsw_del_arl_table(src_mac, vlan_gid);
			#endif
			goto determine_dev_ok;
		} else {
			PDEBUG("not determine come from lan or wan\n"); // should not go here
			PDEBUG("not determine come from lan or wan\n"); // should not go here
			goto freepacket;
		}
	}

#endif // CONFIG_HAVE_VLAN_TAG

#endif /* CONFIG_STR9100_VLAN_BASE */


determine_dev_ok:
	skb_put(skb_ptr, len);



	if (skb_ptr->dev!=NULL) {
		priv = netdev_priv(skb_ptr->dev);
	}
	else{
		PDEBUG("skb_ptr->dev==NULL\n");
		//goto freepacket;
	}

#ifdef STAR_GSW_RX_HW_CHECKSUM
	if (rxdesc_ptr->ipf == 1 || rxdesc_ptr->l4f == 1) {
		if (rxdesc_ptr->prot != 0x11) {
			skb_ptr->ip_summed = CHECKSUM_NONE;
		} else {
			// CheckSum Fail
			priv->stats.rx_errors++;
			goto freepacket;
		}
	} else {
			skb_ptr->ip_summed = CHECKSUM_UNNECESSARY;
	}
#else
	skb_ptr->ip_summed = CHECKSUM_NONE;
#endif

	// this line must, if no, packet will not send to network layer
	skb_ptr->protocol = eth_type_trans(skb_ptr, skb_ptr->dev);
	//skb_ptr->protocol = htons(ETH_P_8021Q);

#ifndef CONFIG_STAR_GSW_BRIDGE
	// send any packet in bridge mode
	/*
	 * This is illegality packet so drop it.
	*/
	if (skb_ptr->protocol == htons(ETH_P_802_2)) {
		PDEBUG("ETH_P_802_2\n");
		goto freepacket;
	}
#endif

	priv->stats.rx_packets++;
	priv->stats.rx_bytes += len;
	skb_ptr->dev->last_rx = jiffies;


	//PRINT_PACKET(skb_ptr->data, 32, "RX")

	// if netif_rx any package, will let this driver core dump.
#ifdef CONFIG_STAR_GSW_NAPI
	netif_receive_skb(skb_ptr);
#else
	netif_rx(skb_ptr);
#endif

	return 0;

freepacket:
	dev_kfree_skb_any(skb_ptr);
	return 0;
}

#ifdef CONFIG_STAR_GSW_NAPI
void star_gsw_receive_packet(int mode, int *work_done, int work_to_do)
#else
void star_gsw_receive_packet(int mode)
#endif
{
	int fssd_index;
	int fssd_current;
	STAR_GSW_RXDESC volatile *rxdesc_ptr = rxring.vir_addr + rxring.cur_index;
	struct sk_buff *skb_ptr;
#ifndef CONFIG_STAR_GSW_NAPI
	int fsqf = 0; // Queue Full Mode =0
#endif
	int i, rxcount = 0;
	GSW_READ_FSSD(fssd_current);
	fssd_index = (fssd_current - (u32)rxring.phy_addr) >> 4;

	if (fssd_index > rxring.cur_index) {
		rxcount = fssd_index - rxring.cur_index;
	} else if (fssd_index < rxring.cur_index) {
		rxcount = (STAR_GSW_MAX_RFD_NUM - rxring.cur_index) + fssd_index;
	} else {
		if (rxdesc_ptr->cown == 0) {
			goto receive_packet_exit;
		} else {
			// Queue Full
#ifndef CONFIG_STAR_GSW_NAPI
			fsqf = 1;
#endif
			rxcount = STAR_GSW_MAX_RFD_NUM;
			//set_bit(0, &is_qf);
		}
	}

#ifndef CONFIG_STAR_GSW_NAPI
	if (mode == 1) {
		fsqf = 1;
		rxcount = STAR_GSW_MAX_RFD_NUM;
	}
#endif

	for (i = 0; i < rxcount; i++) {
#ifdef CONFIG_STAR_GSW_NAPI
		if (*work_done >= work_to_do)
			break;
		++(*work_done);
#endif
		if (rxdesc_ptr->cown != 0) {
			// Alloc New skb_buff 
			skb_ptr = star_gsw_alloc_skb();
			// Check skb_buff
			if (skb_ptr != NULL) {
				star_gsw_get_rfd_buff(rxring.cur_index);
				rxring.skb_ptr[rxring.cur_index] = skb_ptr;
				rxdesc_ptr->data_ptr	= (u32)virt_to_phys(skb_ptr->data);
				rxdesc_ptr->length	= MAX_PACKET_LEN;	
				rxdesc_ptr->cown	= 0; // set cbit to 0 for CPU Transfer	
			} else {
				// TODO:
				// I will add dev->lp.stats->rx_dropped, it will effect the performance
				PDEBUG("%s: Alloc sk_buff fail, reuse the buffer\n", __FUNCTION__);
				rxdesc_ptr->cown	= 0; // set cbit to 0 for CPU Transfer	
				return;
			}
		}

		if (rxring.cur_index == (STAR_GSW_MAX_RFD_NUM - 1)) {
			rxring.cur_index	= 0;
			rxdesc_ptr		= rxring.vir_addr;
		} else {
			rxring.cur_index++;
			rxdesc_ptr++;
		}
	}

#ifndef CONFIG_STAR_GSW_NAPI
	if (fsqf) {
		rxring.cur_index = fssd_index;
		mb();
		GSW_FS_DMA_START();
	}
#endif

receive_packet_exit:
	return;
}

#ifdef FREE_TX_SKB_MULTI
#define MAX_TX_SKB_FREE_NUM     16
#endif


static int star_gsw_send_packet(struct sk_buff *skb, struct net_device *dev)
{
	struct star_gsw_private *priv = netdev_priv(dev);
	STAR_GSW_TXDESC volatile *txdesc_ptr;
	unsigned long flags;
	u16 vlan_tag;

#ifdef FREE_TX_SKB_MULTI
	int i;
	int tssd_index;
	int tssd_current;
	int skb_free_count = 0;
	struct sk_buff *skb_free[MAX_TX_SKB_FREE_NUM];
#endif


#if defined(MAX_SKB_FRAGS) && defined(STAR_GSW_SG)
	int org_index;
	int cur_index;

	unsigned int f;
	unsigned int nr_frags = skb_shinfo(skb)->nr_frags;
	unsigned int len = skb->len - skb->data_len;
	unsigned int offset;

#ifndef FREE_TX_SKB_MULTI
	int skb_free_count = 0;
	struct sk_buff *skb_free[MAX_SKB_FRAGS];
#endif
#else /* defined(MAX_SKB_FRAGS) && defined(STAR_GSW_SG) */
#ifndef FREE_TX_SKB_MULTI
	struct sk_buff *skb_free = NULL;
#endif
#endif /* defined(MAX_SKB_FRAGS) && defined(STAR_GSW_SG) */

	HAL_GSW_TS_DMA_STOP();
	spin_lock_irqsave(&star_gsw_send_lock, flags);

#ifdef FREE_TX_SKB_MULTI
	HAL_GSW_READ_TSSD(tssd_current);
	tssd_index = (tssd_current - (u32)txring.phy_addr) >> 4;

	if (tssd_index > txring.to_free_index) {
		skb_free_count = tssd_index - txring.to_free_index;
	} else if (tssd_index < txring.to_free_index) {
		skb_free_count = STAR_GSW_MAX_TFD_NUM + tssd_index - txring.to_free_index;
	}

	if (skb_free_count >= MAX_TX_SKB_FREE_NUM) {
		int count = 0;
		for (i = 0; i < skb_free_count; i++) {
			txdesc_ptr = txring.vir_addr + txring.to_free_index;
			if (txdesc_ptr->cown == 0) {
				break;
			}
			if (txring.skb_ptr[txring.to_free_index]) {
				skb_free[count++] = txring.skb_ptr[txring.to_free_index];
				txring.skb_ptr[txring.to_free_index] = NULL;
			}
			txring.to_free_index++;
			if (txring.to_free_index == STAR_GSW_MAX_TFD_NUM) {
				txring.to_free_index = 0;
			}
			if (count == MAX_TX_SKB_FREE_NUM) {
				break;
			}
		}
		skb_free_count = count;
	} else {
		skb_free_count = 0;
	}
#endif



#if defined(MAX_SKB_FRAGS) && defined(STAR_GSW_SG)
	org_index = txring.cur_index;
	cur_index = txring.cur_index;
	//printk("nr_frags: %d\n", nr_frags);
	for (f = 0; f < (nr_frags + 1); f++) {
		txdesc_ptr = txring.vir_addr + cur_index;

		if (txdesc_ptr->cown == 0) {
			spin_unlock_irqrestore(&star_gsw_send_lock, flags);
			// re-queue the skb
			return 1;
		}

#ifndef FREE_TX_SKB_MULTI
		if (txring.skb_ptr[cur_index]) {
			skb_free[skb_free_count++] = txring.skb_ptr[cur_index];
#ifdef STAR_GSW_TIMER
			txring.to_free_index = cur_index + 1;
			if (txring.to_free_index == STAR_GSW_MAX_TFD_NUM) {
				txring.to_free_index = 0;
			}
#endif
		}
#endif

		if (f == 0) {
			txdesc_ptr->fs		= 1;
		} else {
			txdesc_ptr->fs		= 0;
		}
		if (f == nr_frags) {
			txdesc_ptr->ls		= 1;
		} else {
			txdesc_ptr->ls		= 0;
		}

		if (skb->protocol == __constant_htons(ETH_P_IP)) {
			txdesc_ptr->ico = 1;
			if (skb->nh.iph->protocol == IPPROTO_UDP) {
				txdesc_ptr->uco = 1;
				txdesc_ptr->tco = 0;
			} else if (skb->nh.iph->protocol == IPPROTO_TCP) {
				txdesc_ptr->uco = 0;
				txdesc_ptr->tco = 1;
			} else {
				txdesc_ptr->uco = 0;
				txdesc_ptr->tco = 0;
			}
		} else {
			txdesc_ptr->ico = 0;
			txdesc_ptr->uco = 0;
			txdesc_ptr->tco = 0;
		}

		txdesc_ptr->interrupt = 0;
		txdesc_ptr->fr = 1;



#ifdef CONFIG_STR9100_VLAN_BASE
		if (priv->pmap==-1) {
			txdesc_ptr->insv	= 1;
			txdesc_ptr->pmap	= 1; // MAC0
			if (dev == STAR_GSW_WAN_DEV) {
				txdesc_ptr->vid	= VLAN0_GROUP_ID; 
			} else {
				txdesc_ptr->vid	= VLAN1_GROUP_ID; 
			}
		}
#endif // CONFIG_STR9100_VLAN_BASE

#ifdef CONFIG_STR9100_PORT_BASE
		if (priv->pmap != -1) {
			txdesc_ptr->insv	= 0;
			txdesc_ptr->pmap	= priv->pmap;
		}
#endif

		cur_index++;
		if (cur_index == STAR_GSW_MAX_TFD_NUM) {
			cur_index = 0;
		}
	} // end for (f = 0; f < (nr_frags + 1); f++) 

	txdesc_ptr = (txring.vir_addr + txring.cur_index);
	txdesc_ptr->data_ptr			= virt_to_phys(skb->data);
	if ((nr_frags == 0) && (len < MIN_PACKET_LEN)) {
		txdesc_ptr->length		= MIN_PACKET_LEN;
		memset(skb->data + len, 0x00, MIN_PACKET_LEN - len);
	} else {
		txdesc_ptr->length		= len;
	}
	if (nr_frags) {
		txring.skb_ptr[txring.cur_index]	= NULL;
	} else {
		txring.skb_ptr[txring.cur_index]	= skb;
	}
	consistent_sync(skb->data, txdesc_ptr->length, PCI_DMA_TODEVICE);

#if 0
	printk("txdesc_ptr : %x\n", txdesc_ptr);
	printk("txdesc_ptr->length: %d\n", txdesc_ptr->length);
	printk("txdesc_ptr->insv : %d\n", txdesc_ptr->insv);
	printk("txdesc_ptr->pmap : %d\n", txdesc_ptr->pmap);
	printk("txdesc_ptr->vid : %d\n", txdesc_ptr->vid);
#endif

	txring.cur_index++;
	if (txring.cur_index == STAR_GSW_MAX_TFD_NUM) {
		txring.cur_index = 0;
	}

	for (f = 0; f < nr_frags; f++) {
		struct skb_frag_struct *frag; 
		txdesc_ptr = txring.vir_addr + txring.cur_index;
		frag = &skb_shinfo(skb)->frags[f]; 
		len = frag->size; 
		offset = frag->page_offset; 

		txdesc_ptr->data_ptr		= virt_to_phys(page_address(frag->page) + offset);
		txdesc_ptr->length		= len;
		if (f == (nr_frags - 1)) {
			txring.skb_ptr[txring.cur_index] = skb;
		} else {
			txring.skb_ptr[txring.cur_index] = NULL;
		}
		consistent_sync(page_address(frag->page) + offset, txdesc_ptr->length, PCI_DMA_TODEVICE);

		txring.cur_index++;
		if (txring.cur_index == STAR_GSW_MAX_TFD_NUM) {
			txring.cur_index = 0;
		}
	}

	for (f = 0; f < (nr_frags + 1); f++) {
		txdesc_ptr = txring.vir_addr + org_index;

		txdesc_ptr->cown = 0;

		org_index++;
		if (org_index == STAR_GSW_MAX_TFD_NUM) {
			org_index = 0;
		}
	}

#else // defined(MAX_SKB_FRAGS) && defined(STAR_GSW_SG)
	txdesc_ptr = txring.vir_addr + txring.cur_index;


	if (txdesc_ptr->cown == 0) { // This TFD is busy
		spin_unlock_irqrestore(&star_gsw_send_lock, flags);
		// re-queue the skb
		return 1;
	}

	if (txdesc_ptr->data_ptr != 0) {
		// MUST TODO: Free skbuff
		dev_kfree_skb_any(txring.skb_ptr[txring.cur_index]);
	}

	/* clean dcache range  in order that data synchronization*/
	//consistent_sync(skb->data, skb->len, PCI_DMA_TODEVICE);
	txring.skb_ptr[txring.cur_index]	= skb;
	txdesc_ptr->data_ptr			= virt_to_phys(skb->data);

        if (skb->len < MIN_PACKET_LEN) {
                txdesc_ptr->length              = MIN_PACKET_LEN;
                memset(skb->data + skb->len, 0x00, MIN_PACKET_LEN - skb->len);
        } else {
                txdesc_ptr->length              = skb->len;
        }

 

        /* clean dcache range  in order that data synchronization*/
        consistent_sync(skb->data, txdesc_ptr->length, PCI_DMA_TODEVICE);


	// 20060922 descent
	// if NIC MODE on
	#ifdef CONFIG_NIC_MODE
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
	// do nothing
#else
        if ( ((GSW_SWITCH_CONFIG >> 30) & 0x1) ==1){
		const char lan_tag[]={0x81, 0x00, 0x00, 0x01};
		const char wan_tag[]={0x81, 0x00, 0x00, 0x02};
		unsigned char	data; /* Data head pointer */

		// insert vlan tag and move other byte to back
		memmove(skb->data+16, skb->data + 12, skb->len-12);
		skb->len+=4;

                txdesc_ptr->length = skb->len;

		if (dev == STAR_GSW_LAN_DEV) {
			memcpy(skb->data+12, lan_tag, 4);
		}
		if (dev == STAR_GSW_WAN_DEV) {
			memcpy(skb->data+12, wan_tag, 4);
		}
        	consistent_sync(skb->data, txdesc_ptr->length, PCI_DMA_TODEVICE);
	}
#endif //defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
	#endif // ifdef CONFIG_NIC_MODE
	// 20060922 descent end

	/*
	* Basically, according to the result of the search of the destination 
	* address in the routing table, the specific network interface will be 
	* correctly selected
	* According to the device entry index value, we can know the packet will 
	* be destined for LAN port (port 0) or WAN port (port 1)
	*
	* Note:
	* device entry index = 0 means local loopback network interface
	* device entry index = 1 means GSW port 0 for LAN port network interface
	* device entry index = 2 means GSW port 1 for WAN port network interface
	* and also note:
	* Force Route Port Map = 1 : GSW port 0
	*                      = 2 : GSW port 1
	*                      = 4 : GSW CPU port 
	*/     


	PDEBUG("\n00 priv->pmap: %d\n", priv->pmap);


#ifdef CONFIG_STR9100_VLAN_BASE
	PDEBUG("CONFIG_STR9100_VLAN_BASE\n");



	if (priv->pmap==-1)
	{
		// 20060922 descent
		// if NIC MODE on
		#ifdef CONFIG_NIC_MODE
       		if ( ((GSW_SWITCH_CONFIG >> 30) & 0x1) ==1)
		{
			txdesc_ptr->insv	= 0;
		}
		else
		#endif
		// 20060922 descent end
		{
			txdesc_ptr->insv	= 1;
		}


		PDEBUG("txdesc_ptr->insv	= 1;\n");




		txdesc_ptr->pmap	= 1; // MAC0


#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
	// let 8021Q insert vlan tag
	// so insv set to 0
	txdesc_ptr->insv	= 0;
	#if 0
        if (priv->vlgrp && vlan_tx_tag_present(skb)) {
                //vlan_tag = cpu_to_be16(vlan_tx_tag_get(skb));
                //vlan_tag = ntohl(vlan_tx_tag_get(skb));
                //vlan_tag = (vlan_tx_tag_get(skb));
                vlan_tag = cpu_to_le16(vlan_tx_tag_get(skb));
		//printk("vlan_tag : %x\n", vlan_tag);
		if (vlan_tag == 1) {
			txdesc_ptr->vid	= VLAN1_GROUP_ID; // lan
		}
		if (vlan_tag == 2) {
			txdesc_ptr->vid	= VLAN0_GROUP_ID; // wan
		}
	}
	#endif
#else // #if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)



		PDEBUG("txdesc_ptr->pmap = 1\n");
		if (dev == STAR_GSW_WAN_DEV) {
			txdesc_ptr->vid	= VLAN0_GROUP_ID; 
			PDEBUG("VLAN0_GROUP_ID: %d\n", VLAN0_GROUP_ID);
		} else {
			txdesc_ptr->vid	= VLAN1_GROUP_ID; 
			PDEBUG("VLAN1_GROUP_ID: %d\n", VLAN1_GROUP_ID);
		}

#endif // end #if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)

	}

#endif // CONFIG_STR9100_VLAN_BASE

#ifdef CONFIG_STR9100_PORT_BASE
	if (priv->pmap != -1)
	{
		txdesc_ptr->insv	= 0;
		PDEBUG("txdesc_ptr->insv        = 0;\n");
		txdesc_ptr->pmap	= priv->pmap;
		PDEBUG("txdesc_ptr->pmap	= priv->pmap;\n");
	}
	PDEBUG("CONFIG_STR9100_PORT_BASE\n");
#endif
	PDEBUG("txdesc_ptr->pmap: %d\n", txdesc_ptr->pmap);

	txdesc_ptr->fr		= 1;

#ifdef CONFIG_STAR9100_SHNAT_PCI_FASTPATH
	if (star9100_shnat_pci_fp_forward_skb_ptr == 0) 
		goto SEND_PACKET;
	if(priv->pmap == PORT_BASE_PMAP_TUN_PORT){

		struct iphdr *iph = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
		star9100_arp_table volatile *arp_table;
		u32			fp_gvid = 0;

		arp_table = star9100_shnat_getarptable_hook(iph->saddr);
		if(arp_table != NULL){
			fp_gvid = arp_table->unused &= 0x7; 
		}
#if 0
	   printk("[SEND PACKET] FP Path send_packet\n");
#endif
	   txdesc_ptr->fr = 0;
	   txdesc_ptr->insv  = 1;
	   txdesc_ptr->vid = fp_gvid;
#if 0
	   print_packet(skb->data,128);
#endif
	}
#endif /*  CONFIG_STAR9100_SHNAT_PCI_FASTPATH */

SEND_PACKET:

	txdesc_ptr->fs		= 1;
	txdesc_ptr->ls		= 1;
	// Wake interrupt
	txdesc_ptr->interrupt	= 0;
	txdesc_ptr->cown	= 0;


	if (txring.cur_index == (STAR_GSW_MAX_TFD_NUM - 1)) {
		txring.cur_index = 0;
	} else {
		txring.cur_index++;
	}
	

#endif // defined(MAX_SKB_FRAGS) && defined(STAR_GSW_SG)
	mb();
	GSW_TS_DMA_START();


	priv->stats.tx_packets++;
	priv->stats.tx_bytes += skb->len;
	dev->trans_start = jiffies;

sendpacket_exit:
	spin_unlock_irqrestore(&star_gsw_send_lock, flags);

#ifdef FREE_TX_SKB_MULTI
	for (i = 0; i < skb_free_count; i++) {
		dev_kfree_skb(skb_free[i]);
	}
#else
#if defined(MAX_SKB_FRAGS) && defined(STAR_GSW_SG)
	for (f = 0; f < skb_free_count; f++) {
		dev_kfree_skb(skb_free[f]);
	}
#else
	if (skb_free) {
		dev_kfree_skb(skb_free);
	}
#endif
#endif

#ifdef STAR_GSW_TIMER
	mod_timer(&star_gsw_timer, jiffies + 10);
#endif




	return 0;
}

static void star_gsw_set_mac_addr(int index, const char *mac, int mac_len)
{
	gsw_arl_table_entry_t arl_table_entry;

	// erase old mac
	arl_table_entry.filter		= 0;
	arl_table_entry.vlan_mac	= 1;
	arl_table_entry.vlan_gid	= star_gsw_info.vlan[index].vlan_gid;
	arl_table_entry.age_field	= 0x0; // invalid mean erase this entry
	arl_table_entry.port_map	= star_gsw_info.vlan[index].vlan_group;
	memcpy(arl_table_entry.mac_addr, star_gsw_info.vlan[index].vlan_mac, 6);
	star_gsw_write_arl_table_entry(&arl_table_entry);

	// copy new mac to star_gsw_info
	memcpy(star_gsw_info.vlan[index].vlan_mac, mac, mac_len);
	arl_table_entry.filter		= 0;
	arl_table_entry.vlan_mac	= 1;
	arl_table_entry.vlan_gid	= star_gsw_info.vlan[index].vlan_gid;
	arl_table_entry.age_field	= 0x7;
	arl_table_entry.port_map	= star_gsw_info.vlan[index].vlan_group;
	memcpy(arl_table_entry.mac_addr, star_gsw_info.vlan[index].vlan_mac, 6);
	star_gsw_write_arl_table_entry(&arl_table_entry);
}


static int star_gsw_set_lan_mac_addr(struct net_device *dev, void *addr)
{
	struct sockaddr *sock_addr = addr;
	struct star_gsw_private *priv = netdev_priv(dev);

	spin_lock_irq(&priv->lock);
	star_gsw_set_mac_addr(LAN_GID, sock_addr->sa_data, dev->addr_len);
	//star_gsw_set_mac_addr(0, sock_addr->sa_data, dev->addr_len);
	spin_unlock_irq(&priv->lock);

	return 0;
}

static int star_gsw_set_wan_mac_addr(struct net_device *dev, void *addr)
{
	struct sockaddr *sock_addr = addr;
	struct star_gsw_private *priv = netdev_priv(dev);

	spin_lock_irq(&priv->lock);
	//star_gsw_set_mac_addr(1, sock_addr->sa_data, dev->addr_len);
	star_gsw_set_mac_addr(WAN_GID, sock_addr->sa_data, dev->addr_len);
	spin_unlock_irq(&priv->lock);

	return 0;
}

// current dorado2 use this function
static int star_gsw_set_ewc_mac_addr(struct net_device *dev, void *addr)
{
	struct sockaddr *sock_addr = addr;
	struct star_gsw_private *priv = netdev_priv(dev);

	spin_lock_irq(&priv->lock);
	star_gsw_set_mac_addr(2, sock_addr->sa_data, dev->addr_len);
	spin_unlock_irq(&priv->lock);

	return 0;
}

static void __init star_gsw_hw_init(void)
{
	u32 mac_port_config;
	int i;
	u32 cfg_reg = 0;

	cfg_reg = PWRMGT_SOFTWARE_RESET_CONTROL;
	// set reset bit to HIGH active;
	cfg_reg |=0x10;
	PWRMGT_SOFTWARE_RESET_CONTROL = cfg_reg;

	//pulse delay
	udelay(100);

	// set reset bit to LOW active;
	cfg_reg &=~0x10;
	PWRMGT_SOFTWARE_RESET_CONTROL = cfg_reg;

	//pulse delay
	udelay(100);

	// set reset bit to HIGH active;
	cfg_reg |= 0x10;
	PWRMGT_SOFTWARE_RESET_CONTROL = cfg_reg; 

	for (i = 0; i < 1000; i++) {
		cfg_reg = GSW_BIST_RESULT_TEST_0;
		if ((cfg_reg & BIT(17))) {
			break;
		} else {
			udelay(10);
		}
	}
	// Set to defaule value
	GSW_SWITCH_CONFIG = 0x007AA7A1;

	// Set Mac port 0 to default value
	GSW_MAC_PORT_0_CONFIG = 0x00423D80;

	// Set Mac port 1 to default value
	GSW_MAC_PORT_1_CONFIG = 0x00423D80;

	// Set CPU port to default Value
	GSW_CPU_PORT_CONFIG = 0x004C0000;

	// Disable Port 0
	mac_port_config = GSW_MAC_PORT_0_CONFIG;
	mac_port_config |= ((0x1 << 18)); 
	GSW_MAC_PORT_0_CONFIG = mac_port_config; 

	// Disable Port 1
	mac_port_config = GSW_MAC_PORT_1_CONFIG;
	mac_port_config |= ((0x1 << 18)); 
	GSW_MAC_PORT_1_CONFIG = mac_port_config; 
}

#ifdef CONFIG_STAR9100_SHNAT_PCI_FASTPATH
static int tundev_close(struct net_device *dev){
	netif_stop_queue(dev);
	printk("Close Orion Fast Path Tunnel Device \n");
	return 0;
}
static int tundev_open(struct net_device *dev){
	netif_start_queue(dev);
	printk("Open Orion Fast Path Tunnel Device \n");
	return 0;
}
static void tundev_init(struct net_device *dev){
	return;
}
static int __init star_gsw_probe_tun(void){
	struct net_device *netdev;
	struct star_gsw_private *priv;
	int err=0;

	//netdev = alloc_netdev(sizeof(struct star_gsw_private),"fp",tundev_init);
	netdev = alloc_etherdev(sizeof(struct star_gsw_private));
	if (!netdev) {
		err = -ENOMEM;
		goto progend;
	}

	sprintf(netdev->name,"fp"); // force name

	priv = netdev_priv(netdev);
	memset(priv, 0, sizeof(struct star_gsw_private));
	spin_lock_init(&priv->lock);

	//netdev->base_addr			= IO_ADDRESS(GSW_BASE_ADDR);
	netdev->base_addr			= 0;
	netdev->stop				= tundev_close;
	netdev->hard_start_xmit		= star_gsw_send_packet;
	netdev->open				= tundev_open;
	//netdev->set_mac_address		= star_gsw_set_lan_mac_addr;

	netdev->features			= NETIF_F_NO_CSUM;
	netdev->hard_header			= NULL;
	netdev->rebuild_header 		= NULL;
	netdev->hard_header_cache	= NULL;
	netdev->header_cache_update = NULL;
	netdev->hard_header_parse   = NULL;
	netdev->flags				= 0; // Don't need any flags
	priv->pmap			= PORT_BASE_PMAP_TUN_PORT;



	err = register_netdev(netdev);
	if (err) {
		free_netdev(netdev);
		err = -ENOMEM;
	}

progend:
	return err;
}
#endif


static int __init star_gsw_probe(int port_type)
{
	struct net_device *netdev;
	struct star_gsw_private *priv;
	int err;

	netdev = alloc_etherdev(sizeof(struct star_gsw_private));
	if (!netdev) {
		err = -ENOMEM;
		goto err_alloc_etherdev;
	}

	priv = netdev_priv(netdev);
	memset(priv, 0, sizeof(struct star_gsw_private));
	spin_lock_init(&priv->lock);

	//netdev->base_addr		= IO_ADDRESS(GSW_BASE_ADDR);
	netdev->base_addr		= 0;
	netdev->stop			= star_gsw_close;
	netdev->hard_start_xmit		= star_gsw_send_packet;
	netdev->tx_timeout		= star_gsw_timeout;
	netdev->get_stats		= star_gsw_get_stats;
#if defined(MAX_SKB_FRAGS) && defined(STAR_GSW_SG)
	netdev->features		= NETIF_F_IP_CSUM | NETIF_F_SG;
#elif defined(STAR_GSW_TX_HW_CHECKSUM)
	netdev->features		= NETIF_F_IP_CSUM;
#endif

#ifdef CONFIG_STAR_GSW_NAPI
	netdev->poll			= star_gsw_poll;
	netdev->weight			= 64;
#endif
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
	// do not let 8021Q module insert vlan tag
	// can use the snippet code to get vlan tage
	// if (priv->vlgrp && vlan_tx_tag_present(skb)) {
	//   vlan_tag = cpu_to_be16(vlan_tx_tag_get(skb));
        //netdev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
        netdev->features |= NETIF_F_HW_VLAN_RX; // remove NETIF_F_HW_VLAN_TX flag that 8021Q module to insert vlan tag.

        netdev->vlan_rx_register = gsw_vlan_rx_register;
        netdev->vlan_rx_kill_vid = gsw_vlan_rx_kill_vid;
#endif


	netdev->open		= star_gsw_open;
	switch (port_type) {
	case LAN_PORT:
		netdev->set_mac_address	= star_gsw_set_lan_mac_addr;
		//priv->pmap		= PMAP_PORT0;
		//priv->pmap		= PORT_BASE_PORT0;
		priv->pmap		= PORT_BASE_PMAP_LAN_PORT;
		break;

	case WAN_PORT:
		//netdev->open		= star_gsw_wan_open;
		netdev->set_mac_address	= star_gsw_set_wan_mac_addr;
		//priv->pmap		= PMAP_PORT1;
		priv->pmap		= PORT_BASE_PMAP_WAN_PORT;
		break;

	case EWC_PORT:
		//netdev->open		= star_gsw_ewc_open;
		netdev->set_mac_address	= star_gsw_set_ewc_mac_addr;
		//priv->pmap		= PMAP_PORT1;
		priv->pmap		= PORT_BASE_PMAP_EWC_PORT;
		break;

	default:
		break;
	}

	err = register_netdev(netdev);
	if (err) {
		goto err_register_netdev;
	}

	SET_MODULE_OWNER(netdev);

	switch (port_type) {
	case LAN_PORT:
		STAR_GSW_LAN_DEV = netdev;
		break;

	case WAN_PORT:
		STAR_GSW_WAN_DEV = netdev;


		break;

	case EWC_PORT:
		PDEBUG("create ewc port\n");
		STAR_GSW_EWC_DEV = netdev;
		break;

	default:
		break;
	}

	return 0;

err_register_netdev:
	free_netdev(netdev);

err_alloc_etherdev:
	return err;
}

#ifdef LINUX26
extern struct proc_dir_entry *str9100_proc_dir;
static int __init star_gsw_proc_init(void)
{
        star_gsw_proc_entry = create_proc_entry("gsw", S_IFREG | S_IRUGO, str9100_proc_dir);
        if (star_gsw_proc_entry) {
                star_gsw_proc_entry->read_proc = star_gsw_read_proc;
                star_gsw_proc_entry->write_proc = star_gsw_write_proc;
        }
        return 1;
}
#endif


#ifdef LINUX24
static int __init star_gsw_proc_init(void)
{
	struct proc_dir_entry *procdir=0;

	const char proc_str[]="str9100";

	//str9100_gsw_procdir=proc_mkdir(proc_str, NULL);
	
        procdir=create_proc_str9100(PROC_STR);

        if (procdir)
        {
		star_gsw_proc_entry = create_proc_entry("gsw", S_IFREG | S_IRUGO, procdir);
		if (star_gsw_proc_entry) {
			star_gsw_proc_entry->read_proc = star_gsw_read_proc;
			star_gsw_proc_entry->write_proc = star_gsw_write_proc;
		}
		return 1;
        }
	else
		return -1;
	


}
#endif

static int star_gsw_notify_reboot(struct notifier_block *nb, unsigned long event, void *ptr)
{
	u32 mac_port_config;

	/* stop the DMA engine */
	GSW_TS_DMA_STOP();
	GSW_FS_DMA_STOP();

	// disable Port 0
	mac_port_config = GSW_MAC_PORT_0_CONFIG;
	mac_port_config |= ((0x1 << 18)); 
	GSW_MAC_PORT_0_CONFIG = mac_port_config; 

	// disable Port 1
	mac_port_config = GSW_MAC_PORT_1_CONFIG;
	mac_port_config |= ((0x1 << 18)); 
	GSW_MAC_PORT_1_CONFIG = mac_port_config; 

	// disable all interrupt status sources
	GSW_DISABLE_ALL_INTERRUPT_STATUS_SOURCES();

	// clear previous interrupt sources
	GSW_CLEAR_ALL_INTERRUPT_STATUS_SOURCES();

	// disable all DMA-related interrupt sources
	INTC_DISABLE_INTERRUPT_SOURCE(INTC_GSW_TSTC_BIT_INDEX);
	INTC_DISABLE_INTERRUPT_SOURCE(INTC_GSW_FSRC_BIT_INDEX);
	INTC_DISABLE_INTERRUPT_SOURCE(INTC_GSW_TSQE_BIT_INDEX);
	INTC_DISABLE_INTERRUPT_SOURCE(INTC_GSW_FSQF_BIT_INDEX);

	// clear previous interrupt sources
	INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GSW_TSTC_BIT_INDEX);
	INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GSW_FSRC_BIT_INDEX);
	INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GSW_TSQE_BIT_INDEX);
	INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GSW_FSQF_BIT_INDEX);

	return NOTIFY_DONE;
}





static int __init star_gsw_init_module(void)
{
	int err = 0;
	u8 mac_num[6]; // get from flash

	spin_lock_init(&star_gsw_send_lock);


//#define CONFIG_GET_FLASH_MAC
#ifdef CONFIG_GET_FLASH_MAC
	char *mac_addr;

	PRINT_INFO(KERN_INFO "%s", star_gsw_driver_version);
	mac_addr=get_flash_env("ethaddr");
	if (mac_addr)
	{
		printk("mac addr: %s\n", mac_addr);
		printk("mac len: %d\n", strlen(mac_addr) );

	}
	sscanf(mac_addr ,"%x:%x:%x:%x:%x:%x", (unsigned int *)&mac_num[0], (unsigned int *)&mac_num[1], (unsigned int *)&mac_num[2], (unsigned int *)&mac_num[3], (unsigned int *)&mac_num[4], (unsigned int *)&mac_num[5]);
	printk("flash mac : %x:%x:%x:%x:%x:%x\n", *mac_num,*(mac_num+1),*(mac_num+2), *(mac_num+3), *(mac_num+4), *(mac_num+5) );


	memcpy(my_vlan0_mac, mac_num, 6);
	++mac_num[5];
	memcpy(my_vlan1_mac, mac_num, 6);
	++mac_num[5];
	memcpy(my_vlan2_mac, mac_num, 6);
	++mac_num[5];
	memcpy(my_vlan3_mac, mac_num, 6);



#endif



	star_gsw_hw_init();
	err = star_gsw_buffer_alloc();
	if (err != 0) {
		return err;
	}
	star_gsw_vlan_init();
	star_gsw_config_cpu_port();
	init_switch();

//#if 0
	CREATE_NET_DEV0 
	CREATE_NET_DEV1
	CREATE_NET_DEV2
#ifdef CONFIG_STAR9100_SHNAT_PCI_FASTPATH
	CREATE_NET_DEV_AD
#endif

	str9100_gsw_config_mac_port0();
	str9100_gsw_config_mac_port1();

#ifdef CONFIG_STAR_GSW_NAPI
        memset(&STAR_NAPI_DEV, 0x0, sizeof(struct net_device));
        STAR_NAPI_DEV.priv = NULL;
        STAR_NAPI_DEV.poll = star_gsw_poll;
        STAR_NAPI_DEV.weight = 64;
        dev_hold(&STAR_NAPI_DEV);
        set_bit(__LINK_STATE_START, &STAR_NAPI_DEV.state);
#endif


//#endif

#if 0
	star_gsw_lan_init();

#ifndef CONFIG_STAR_GSW_TYPE_9109
	star_gsw_wan_init();
#endif
#ifdef CONFIG_STAR_GSW_TYPE_EWC
	star_gsw_ewc_init();
#endif
#endif

	star_gsw_proc_init();

	register_reboot_notifier(&star_gsw_notifier_reboot);
#ifdef STAR_GSW_TIMER
        init_timer(&star_gsw_timer);
        star_gsw_timer.function = &star_gsw_timer_func;
        star_gsw_timer.data = (unsigned long)NULL;
#endif


	return 0;
}

static void __exit star_gsw_exit_module(void)
{
	unregister_reboot_notifier(&star_gsw_notifier_reboot);
	star_gsw_buffer_free(); 
}


// this snippet code ref 8139cp.c
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
void gsw_vlan_rx_register(struct net_device *dev, struct vlan_group *grp)
{
        struct star_gsw_private *priv = netdev_priv(dev);
        unsigned long flags;

        spin_lock_irqsave(&priv->lock, flags);
	printk("gsw_vlan_rx_register\n");
        priv->vlgrp = grp;
        spin_unlock_irqrestore(&priv->lock, flags);
}

void gsw_vlan_rx_kill_vid(struct net_device *dev, unsigned short vid)
{
        struct star_gsw_private *priv = netdev_priv(dev);
        unsigned long flags;

        spin_lock_irqsave(&priv->lock, flags);
        if (priv->vlgrp)
                priv->vlgrp->vlan_devices[vid] = NULL;
        spin_unlock_irqrestore(&priv->lock, flags);
}

#endif

//#define CONFIG_SWITCH_IOCTL
#ifdef CONFIG_SWITCH_IOCTL

/* ADD MAC into ARL */
/***
 * add_mac_into_arl  -  add extra(without hnat support) my mac in ARL table.
 * 
 * INPUTS:
 *        gid    -   vlan group (0-7)
 *        mac    -   mac address
 * 
 * OUTPUTS:
 *        None
 * 
 * RETURNS:
 *        void
 ***/
int add_mac_into_arl(u16 gid, u8 *mac)
{
        gsw_arl_table_entry_t arl_table_entry;

        arl_table_entry.filter          = 0;
        arl_table_entry.vlan_mac        = 1;    // the MAC in this table entry is MY VLAN MAC 
        arl_table_entry.vlan_gid        = gid;
        arl_table_entry.age_field       = 0x7;  // static entry
        arl_table_entry.port_map        = star_gsw_info.vlan[gid].vlan_group;
        memcpy(arl_table_entry.mac_addr, mac, 6); 

        if (!star_gsw_write_arl_table_entry(&arl_table_entry)) {
                //DEBUG_MSG(WARNING_MSG, "star_gsw_write_arl_table_entry fail\n");
                return 1;
        }

        return 0; 
}

/* DEL MAC from ARL*/
//void del_my_vlan_mac_2argu(u8 gid, u8 *mac)
/***
 * star_gsw_del_arl_table - delete my mac from ARL table
 * 
 * INPUTS:
 *        mac      - mac address
 *        vlan_gid - gid value 
 * 
 * OUTPUTS:
 *        None
 * 
 * RETURNS:
 *        0		-   successful
 *
 ***/
void del_mac_from_arl(u8 gid, u8 *mac)
{
    gsw_arl_table_entry_t arl_table_entry;

    // erase old mac
    arl_table_entry.filter        = 0;
    arl_table_entry.vlan_mac    = 1;
    //arl_table_entry.vlan_gid    = star_gsw_info.vlan[gid].vlan_gid;
    arl_table_entry.vlan_gid    = gid;
    arl_table_entry.age_field    = 0x0; // invalid mean erase this entry
    arl_table_entry.port_map    = star_gsw_info.vlan[gid].vlan_group;
    memcpy(arl_table_entry.mac_addr, mac, 6);
    star_gsw_write_arl_table_entry(&arl_table_entry);
}


/***
 * del_my_vlan_mac  -  delete my mac of default setting from ARL table.
 * 
 * INPUTS:
 *        gid    -   vlan group (0-7)
 * 
 * OUTPUTS:
 *        None
 * 
 * RETURNS:
 *        void
 ***/
void del_my_vlan_mac(u8 gid)
{
    gsw_arl_table_entry_t arl_table_entry; 

    // erase old mac
    arl_table_entry.filter        = 0; 
    arl_table_entry.vlan_mac    = 1;
    arl_table_entry.vlan_gid    = gid;
    arl_table_entry.age_field    = 0x0; // invalidate this entry 
    arl_table_entry.port_map    = star_gsw_info.vlan[gid].vlan_group;
    memcpy(arl_table_entry.mac_addr, star_gsw_info.vlan[gid].vlan_mac, 6);
    star_gsw_write_arl_table_entry(&arl_table_entry);
}

/***
 * config_my_vlan_mac  -  change my mac default setting, according to 
 * gid and vid value.
 * 
 * INPUTS:
 *        gid    -   vlan group (0-7)
 *        vid    -   vlan id (12 bits)
 *        mac    -   mac address
 * 
 * OUTPUTS:
 *        None
 * 
 * RETURNS:
 *        0  -  successful
 *        1  -  fail
 ***/
int config_my_vlan_mac(u16 gid, u16 vid, u8 *mac)
{
	gsw_arl_table_entry_t arl_table_entry; 

        star_gsw_info.vlan[gid].vlan_gid          = gid;
        star_gsw_info.vlan[gid].vlan_vid          = vid;
        star_gsw_info.vlan[gid].vlan_group        = (PORT0 | CPU_PORT); // this case always (PORT0 | CPU_PORT)
        star_gsw_info.vlan[gid].vlan_tag_flag     = 0;
        memcpy(star_gsw_info.vlan[gid].vlan_mac, mac, 6);



	arl_table_entry.filter          = 0;
        arl_table_entry.vlan_mac        = 1;    // the MAC in this table entry is MY VLAN MAC
        arl_table_entry.vlan_gid        = star_gsw_info.vlan[gid].vlan_gid;
        arl_table_entry.age_field       = 0x7;  // static entry
        arl_table_entry.port_map        = star_gsw_info.vlan[gid].vlan_group;
        memcpy(arl_table_entry.mac_addr, star_gsw_info.vlan[gid].vlan_mac, 6);

        if (!star_gsw_write_arl_table_entry(&arl_table_entry)) {
		//DEBUG_MSG(WARNING_MSG, "star_gsw_write_arl_table_entry fail\n");
        	return 1;
        }

	return 0;
}

#endif // end CONFIG_SWITCH_IOCTL


module_init(star_gsw_init_module);
module_exit(star_gsw_exit_module);

