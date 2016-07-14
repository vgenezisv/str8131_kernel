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
#ifndef LEO_H
#define LEO_H

// add by descent 2006/07/10
#define LEO
#ifdef LEO
// init phy or switch chip
#define INIT_PORT0_PHY star_gsw_config_VSC8201(0,0);
#define INIT_PORT1_PHY 

// configure mac0/mac1 register
#define INIT_PORT0_MAC init_packet_forward(0);
#define INIT_PORT1_MAC disable_AN(1, 1);
// if no disable_AN port 1, maybe get link up/down (change link state)

#define PORT0_LINK_DOWN std_phy_power_down(0,1);
#define PORT0_LINK_UP std_phy_power_down(0,0);

#define PORT1_LINK_DOWN 
#define PORT1_LINK_UP 

#define CREATE_NET_DEV0 star_gsw_probe(LAN_PORT);
#define CREATE_NET_DEV1 
#define CREATE_NET_DEV2


#define CONFIG_STR9100_PORT_BASE
#undef CONFIG_STR9100_VLAN_BASE
#undef CONFIG_HAVE_VLAN_TAG

// for port base, port base max is 2 port.
// NET_DEV0 : rx->sp 0 (port 0)
#define NET_DEV0 STAR_GSW_LAN_DEV
// NET_DEV1 : rx->sp 1 (port 1)
#define NET_DEV1 0

// for star_gsw_send_packet
// port base and vlan base packet flow
#define PORT_BASE_PMAP_LAN_PORT PORT0
#define PORT_BASE_PMAP_WAN_PORT INVALID_PORT_BASE_PMAP_PORT
#define PORT_BASE_PMAP_EWC_PORT INVALID_PORT_BASE_PMAP_PORT

#define MODEL "LEO"

// OPEN_PORT0 include 2 actions
// 1. enable mac port
// 2. link up port
#define OPEN_PORT(dev) \
{ \
        u32 mac_port_config; \
	if (dev == STAR_GSW_LAN_DEV) { \
		memcpy(dev->dev_addr, star_gsw_info.vlan[1].vlan_mac, 6); \
       		PRINT_INFO("open mac port 0\n"); \
	        mac_port_config = GSW_MAC_PORT_0_CONFIG_REG; \
       		/* enable port 0 */ \
	        mac_port_config &= (~(0x1 << 18)); \
       		GSW_MAC_PORT_0_CONFIG_REG = mac_port_config; \
		PORT0_LINK_UP \
	} \
}

// CLOSE_PORT include 2 actions
// 1. disable mac port
// 2. link down port
#define CLOSE_PORT(dev) \
{ \
        u32 mac_port_config; \
	if (dev == STAR_GSW_LAN_DEV) { \
       		PRINT_INFO("close mac port 0\n"); \
		PORT0_LINK_DOWN \
	        mac_port_config = GSW_MAC_PORT_0_CONFIG_REG; \
       		/* disable port 0 */ \
	        mac_port_config |= ((0x1 << 18)); \
       		GSW_MAC_PORT_0_CONFIG_REG = mac_port_config; \
	} \
}


#define VLAN0_VID			(0x111)
#define VLAN1_VID			(0x222)
#define VLAN2_VID			(0x333)
#define VLAN3_VID			(0x444)
#define VLAN4_VID			(0x555)
#define VLAN5_VID			(0x666)
#define VLAN6_VID			(0x777)
#define VLAN7_VID			(0x888)


#define VLAN0_GROUP			(CPU_PORT | PORT0)
#define VLAN1_GROUP			(PORT0 | CPU_PORT)
//#define VLAN0_GROUP                     (PORT0 | PORT1 | CPU_PORT)
//#define VLAN1_GROUP                     (PORT0 | CPU_PORT)
//#define VLAN2_GROUP                     (PORT1 | CPU_PORT)
#define VLAN2_GROUP			(0)
#define VLAN3_GROUP			(0)
#define VLAN4_GROUP			(0)
#define VLAN5_GROUP			(0)
#define VLAN6_GROUP			(0)
#define VLAN7_GROUP			(0)

#define VLAN0_VLAN_TAG			(0)
#define VLAN1_VLAN_TAG			(0)
#define VLAN2_VLAN_TAG			(0)
#define VLAN3_VLAN_TAG			(0)
#define VLAN4_VLAN_TAG			(0)
#define VLAN5_VLAN_TAG			(0)
#define VLAN6_VLAN_TAG			(0)
#define VLAN7_VLAN_TAG			(0)

//#define PORT0_PVID			(VLAN0_GROUP_ID)
//#define PORT1_PVID			(VLAN2_GROUP_ID)
//#define CPU_PORT_PVID			(VLAN1_GROUP_ID)

static u8 my_vlan0_mac[6] = {0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0x12};
static u8 my_vlan1_mac[6] = {0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0x22};
static u8 my_vlan2_mac[6] = {0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0x32};
static u8 my_vlan3_mac[6] = {0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0x42};

// this value is for hnat
// GID is vlan group id
#define LAN_GID 0
#define WAN_GID 2


#endif // LEO

#endif
