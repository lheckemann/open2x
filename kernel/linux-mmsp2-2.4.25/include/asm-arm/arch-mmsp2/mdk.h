/*
 * include/asm-arm/arch-mmsp2/mdk.h
 *
 * Copyright (C) 2003-2004 MIZI Research, Inc.
 *
 * machine header for Magiceyes development board
 *
 * $Id$
 *
 * Revision History:
 *
 * 2004-04-08 Janghoon Lyu <nandy@mizi.com>
 * - initial version
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
 *
 */


#ifndef __MMSP2_MDK_H
#define __MMSP2_MDK_H

#ifndef __ASM_ARCH_HARDWARE_H
#error "include <asm/hardware.h> instead"
#endif
#include <linux/config.h>

/* GPIO out, USB host #2 power enable, negative */
#define GPIO_USB_PO_EN		GPIO_L11
#define GPIO_UH2_nPWR_EN	GPIO_USB_PO_EN

/* GPIO alt. PWM#1, LCD backlight control */
#define GPIO_LCD_BR_CON		GPIO_L12

/* CS8900 */
#define IRQ_ETHER		IRQ_GPIO_D12

/* IDE */
#define IRQ_HARD		IRQ_GPIO_D13
#define GPIO_IDE_nSWRST	GPIO_M4  /* GPIO out, negative */


/* board specific device IO address */
#define MDK_ETH_VIO_BASE	0xf3000000UL /* CS8900 */

#ifdef CONFIG_MMSP2_SHADOW_ENABLE
	#define MDK_ETH_PIO_BASE 0x84000000 /* static bank #1 */
#else
	#define MDK_ETH_PIO_BASE 0x04000000 /* static bank #1 */
#endif

#endif /* __MMSP2_MDK_H */
