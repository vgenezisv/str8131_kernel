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

#include <linux/signal.h>	/* SA_INTERRUPT */
#include <linux/jiffies.h>
#include <linux/platform_device.h>

#include <asm/arch/hardware.h>
#include <asm/mach-types.h>

void str9100_usb_hcd_remove(struct usb_hcd *, struct platform_device *);

static void __init str9100_usb11_config_reg_init(void)
{
#if 0
	__asm__ volatile (
		"mov	r1, #0			\n\t"
		"mcr	p15, 0, r1, c7, c5, 0	\n\t"
		"mov	r1, #0			\n\t"
		"mcr	p15, 0, r1, c7, c14, 0	\n\t"
        );
#endif
	__raw_writel(0x146, SYSVA_USB11_CONFIG_BASE_ADDR + 0x04);
	__raw_writel(0x200, SYSVA_USB11_CONFIG_BASE_ADDR + 0x44);
	mdelay(100);
}

int str9100_usb_hcd_probe(const struct hc_driver *driver, struct platform_device *pdev)
{
	int retval;
	struct usb_hcd *hcd = 0;
	struct ohci_hcd *ohci;

	str9100_usb11_config_reg_init();
	hcd = usb_create_hcd(driver, &pdev->dev, pdev->dev.bus_id);
	if (!hcd) {
		retval = -ENOMEM;
		return retval;
	}
	hcd->regs = (unsigned int *)SYSVA_USB11_OPERATION_BASE_ADDR;
	hcd->rsrc_start = SYSPA_USB11_OPERATION_BASE_ADDR;
	hcd->rsrc_len = 4096;
	ohci = hcd_to_ohci(hcd);
	ohci_hcd_init(ohci);
	retval = usb_add_hcd(hcd, INTC_USB11_BIT_INDEX, SA_INTERRUPT);
	if (retval == 0) {
		return retval;
	}
	printk("str9100 ohci init fail, %d\n", retval);
	usb_put_hcd(hcd);
	return retval;
}

void str9100_usb_hcd_remove(struct usb_hcd *hcd, struct platform_device *pdev)
{
	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);
}

static int __devinit str9100_ohci_start(struct usb_hcd *hcd)
{
	struct ohci_hcd	*ohci = hcd_to_ohci(hcd);
	int ret;

	//ohci->regs = hcd->regs;
	//ohci->fminterval = 0x27782edf;
	if ((ret = ohci_init(ohci)) < 0)
		return ret;

	if ((ret = ohci_run(ohci)) < 0) {
		err("can't start %s", ohci_to_hcd(ohci)->self.bus_name);
		ohci_stop(hcd);
		return ret;
	}

	return 0;
}

/*-------------------------------------------------------------------------*/

static const struct hc_driver str9100_ohci_hc_driver = {
	.description =		hcd_name,
	.product_desc =		"str9100-ohci",
	.hcd_priv_size =	sizeof(struct ohci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq =			ohci_irq,
	.flags =		HCD_USB11 | HCD_MEMORY,

	/*
	 * basic lifecycle operations
	 */
	.start =		str9100_ohci_start,
	.stop =			ohci_stop,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		ohci_urb_enqueue,
	.urb_dequeue =		ohci_urb_dequeue,
	.endpoint_disable =	ohci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number =	ohci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data =	ohci_hub_status_data,
	.hub_control =		ohci_hub_control,
#ifdef	CONFIG_PM
	.bus_suspend =		ohci_bus_suspend,
	.bus_resume =		ohci_bus_resume,
#endif
	.start_port_reset =	ohci_start_port_reset,
};

/*-------------------------------------------------------------------------*/

static int str9100_ohci_hcd_drv_probe(struct platform_device *pdev)
{
	if (usb_disabled())
		return -ENODEV;

	return str9100_usb_hcd_probe(&str9100_ohci_hc_driver, pdev);
}

static int str9100_ohci_hcd_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);

	str9100_usb_hcd_remove(hcd, pdev);
	if (ohci->transceiver) {
		(void) otg_set_host(ohci->transceiver, 0);
		put_device(ohci->transceiver->dev);
	}
	platform_set_drvdata(pdev, NULL);

	return 0;
}

/*-------------------------------------------------------------------------*/

#ifdef	CONFIG_PM

//mkl070226: the functionality of suspend/resume is not complete
static int str9100_ohci_suspend(struct platform_device *pdev, pm_message_t message)
{
	printk("%s: not implemented, just pass it\n",__FUNCTION__);
	return 0;
}
static int str9100_ohci_resume(struct platform_device *pdev)
{
	printk("%s: not implemented, just pass it\n",__FUNCTION__);
	return 0;
}
#if 0
static int str9100_ohci_suspend(struct platform_device *pdev, pm_message_t message)
{
	struct ohci_hcd	*ohci = hcd_to_ohci(platform_get_drvdata(pdev));

	if (time_before(jiffies, ohci->next_statechange))
		msleep(5);
	ohci->next_statechange = jiffies;

	str9100_ohci_clock_power(0);
	ohci_to_hcd(ohci)->state = HC_STATE_SUSPENDED;
	pdev->power.power_state = PMSG_SUSPEND;
	return 0;
}

static int str9100_ohci_resume(struct platform_device *pdev)
{
	struct ohci_hcd	*ohci = hcd_to_ohci(platform_get_drvdata(pdev));

	if (time_before(jiffies, ohci->next_statechange))
		msleep(5);
	ohci->next_statechange = jiffies;
	pdev->power.power_state = PMSG_ON;
	usb_hcd_resume_root_hub(dev_get_drvdata(pdev));
	return 0;
}

#endif
#endif

/*-------------------------------------------------------------------------*/

/*
 * Driver definition to register with the OMAP bus
 */
static struct platform_driver str9100_ohci_hcd_driver = {
	.probe		= str9100_ohci_hcd_drv_probe,
	.remove		= str9100_ohci_hcd_drv_remove,
#ifdef	CONFIG_PM
	.suspend	= str9100_ohci_suspend,
	.resume		= str9100_ohci_resume,
#endif
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "str9100-ohci",
	},
};

static int __init str9100_ohci_hcd_init(void)
{
	printk("%s: " DRIVER_INFO " (STR9100)\n", hcd_name);
	pr_debug("%s: block sizes: ed %Zd td %Zd\n", hcd_name, sizeof(struct ed), sizeof(struct td));
	return platform_driver_register(&str9100_ohci_hcd_driver);
}
module_init(str9100_ohci_hcd_init);

static void __exit str9100_ohci_hcd_cleanup(void)
{
	platform_driver_unregister(&str9100_ohci_hcd_driver);
}
module_exit(str9100_ohci_hcd_cleanup);

