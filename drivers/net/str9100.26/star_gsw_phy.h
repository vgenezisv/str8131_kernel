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

#ifndef __STAR_GSW_PHY_H__
#define __STAR_GSW_PHY_H__
#include <linux/types.h>


//#define CONFIG_VIRGO
//#define CONFIG_DORADO
//#define CONFIG_DORADO2
//#define CONFIG_LEO
//#undef CONFIG_VIRGO
//#undef CONFIG_DORADO
//#undef CONFIG_DORADO2
//#undef CONFIG_LEO

#ifdef CONFIG_LIBRA
#include "libra.h"
#endif

#ifdef CONFIG_VELA
#include "vela.h"
#endif

#ifdef CONFIG_DORADO2
#include "dorado2.h"
#endif

#ifdef CONFIG_DORADO
#include "dorado.h"
#endif

#ifdef CONFIG_LEO
#include "leo.h"
#endif

#ifdef CONFIG_VIRGO
#include "virgo.h"
#endif


void star_gsw_config_ASIX(void);
void star_gsw_config_AGERE(void);
int AGERE_phy_power_down(int phy_addr, int y);
int disable_AN_VSC7385(int y);
void star_gsw_config_VSC7385(void);




void star_gsw_config_AGERE(void);

int str9100_gsw_config_mac_port0(void);
int str9100_gsw_config_mac_port1(void);



#define PORT0				(1 << 0)	/* bit map : bit 0 */
#define PORT1				(1 << 1)	/* bit map : bit 1 */
#define CPU_PORT			(1 << 2)	/* bit map : bit 2 */

#define VLAN0_GROUP_ID			(0)
#define VLAN1_GROUP_ID			(1)
#define VLAN2_GROUP_ID			(2)
#define VLAN3_GROUP_ID			(3)
#define VLAN4_GROUP_ID			(4)
#define VLAN5_GROUP_ID			(5)
#define VLAN6_GROUP_ID			(6)
#define VLAN7_GROUP_ID			(7)

#define PORT0_PVID			(VLAN1_GROUP_ID)
#define PORT1_PVID			(VLAN2_GROUP_ID)
#define CPU_PORT_PVID			(VLAN0_GROUP_ID)

#define INVALID_PORT_BASE_PMAP_PORT -1

#endif /* __STAR_GSW_PHY_H__ */
