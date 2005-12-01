/*
 * linux/include/asm-arm/arch-mmsp2/irq.h
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _ASM_ARCH_IRQS_H
#define _ASM_ARCH_IRQS_H

//-- Main irqs -------------------------------------------------------------------
#define MAIN_IRQ_BASE		0
#define IRQ_DISP			0		/* Display controller */
#define IRQ_IMGH			1		/* Image capture controller (HSYNC) */
#define IRQ_IMGV			2		/* Image capture controller (VSYNC) */
// IRQ No. 3,4 reserved
#define IRQ_TIMER			5		/* Timer -> Timer0,1,2,3*/
#define IRQ_MSTICK			6		/* Memory stick */
#define IRQ_SSP				7		/* SSP */
#define IRQ_PPM				8		/* PPM */
#define IRQ_DMA				9		/* DMA -> DMA ch0~15*/
#define IRQ_UART			10	/* UART -> UART 0,1,2,3 */
#define IRQ_GRP2D			11	/* 2D graphic controller */
#define IRQ_SCALER			12	/* Image scaler */
#define IRQ_USBH			13	/* USB Host */
#define IRQ_SD				14	/* MMC/SD */
#define IRQ_USBD			15	/* USB device */
#define IRQ_RTC				16	/* RTC */
#define IRQ_ADC				17	/* ADC */
#define IRQ_IIC				18	/* IIC */
#define IRQ_AC97			19	/* AC97 */
#define IRQ_IRDA			20	/* IrDA */
// IRQ No. 21,22 reserved
#define IRQ_GPIO			23	/* GPIO -> GPIOA[15:0]~GPIOO[15:0]*/
#define IRQ_CDROM			24	/* CDROM */
#define IRQ_OWM				25	/* One wire master */
#define IRQ_DUALCPU			26	/* Dual CPU */
#define IRQ_MCUC			27	/* Non DRAM Memory controller */
#define IRQ_VLD				28	/* VLD */
#define IRQ_VIDEO			29	/* Video processor */
#define IRQ_MPEGIF			30	/* MPEG Interface */
#define IRQ_IIS				31	/* IIS controller */
#define MAIN_IRQ_END		(MAIN_IRQ_BASE + 32)
//-- DMA sub irqs ------------------------------------------------------------------
#define DMA_SUB_IRQ_BASE	MAIN_IRQ_END
#define IRQ_DMA_SUB(x)		(DMA_SUB_IRQ_BASE + (x))	/* DMA channel 0~15 : 0x20 ~ 0x2f */
#define DMA_SUB_IRQ_END		IRQ_DMA_SUB(16)
//-- UART sub irqs -----------------------------------------------------------------
#define UART_SUB_IRQ_BASE	DMA_SUB_IRQ_END		/* UART 0~3 : 0x30 ~ 0x3f */
#define IRQ_UART_SUB(x)		(UART_SUB_IRQ_BASE + (x))
#define IRQ_TXD0			IRQ_UART_SUB(0)
#define IRQ_RXD0			IRQ_UART_SUB(1)
#define IRQ_ERROR0			IRQ_UART_SUB(2)
#define IRQ_MODEM0			IRQ_UART_SUB(3)
#define IRQ_TXD1			IRQ_UART_SUB(4)
#define IRQ_RXD1			IRQ_UART_SUB(5)
#define IRQ_ERROR1			IRQ_UART_SUB(6)
#define IRQ_MODEM1			IRQ_UART_SUB(7)
#define IRQ_TXD2			IRQ_UART_SUB(8)
#define IRQ_RXD2			IRQ_UART_SUB(9)
#define IRQ_ERROR2			IRQ_UART_SUB(10)
#define IRQ_MODEM2			IRQ_UART_SUB(11)
#define IRQ_TXD3			IRQ_UART_SUB(12)
#define IRQ_RXD3			IRQ_UART_SUB(13)
#define IRQ_ERROR3			IRQ_UART_SUB(14)
#define IRQ_MODEM3			IRQ_UART_SUB(15)
#define UART_SUB_IRQ_END	IRQ_UART_SUB(16)
//-- GPIO sub irqs ------------------------------------------------------------------
#define GPIO_SUB_IRQ_BASE 	UART_SUB_IRQ_END		/* GPIOA[15:0]~GPIOO[15:0] : 0x40 ~ 0x12f */
#define IRQ_GPIO_A(x)		(GPIO_SUB_IRQ_BASE + (x))			// 0x40~
#define IRQ_GPIO_B(x)		(GPIO_SUB_IRQ_BASE + 0x10 + (x))	// 0x50~
#define IRQ_GPIO_C(x)		(GPIO_SUB_IRQ_BASE + 0x20 + (x))	// 0x60~
#define IRQ_GPIO_D(x)		(GPIO_SUB_IRQ_BASE + 0x30 + (x))	// 0x70~
#define IRQ_GPIO_E(x)		(GPIO_SUB_IRQ_BASE + 0x40 + (x))	// 0x80~
#define IRQ_GPIO_F(x)		(GPIO_SUB_IRQ_BASE + 0x50 + (x))	// 0x90~
#define IRQ_GPIO_G(x)		(GPIO_SUB_IRQ_BASE + 0x60 + (x))	// 0xA0~
#define IRQ_GPIO_H(x)		(GPIO_SUB_IRQ_BASE + 0x70 + (x))	// 0xB0~
#define IRQ_GPIO_I(x)		(GPIO_SUB_IRQ_BASE + 0x80 + (x))	// 0xC0~
#define IRQ_GPIO_J(x)		(GPIO_SUB_IRQ_BASE + 0x90 + (x))	// 0xD0~
#define IRQ_GPIO_K(x)		(GPIO_SUB_IRQ_BASE + 0xA0 + (x))	// 0xE0~
#define IRQ_GPIO_L(x)		(GPIO_SUB_IRQ_BASE + 0xB0 + (x))	// 0xF0~
#define IRQ_GPIO_M(x)		(GPIO_SUB_IRQ_BASE + 0xC0 + (x))	// 0x100~
#define IRQ_GPIO_N(x)		(GPIO_SUB_IRQ_BASE + 0xD0 + (x))	// 0x110~
#define IRQ_GPIO_O(x)		(GPIO_SUB_IRQ_BASE + 0xE0 + (x))	// 0x120~0x12f
#define GPIO_SUB_IRQ_END 	(IRQ_GPIO_O(15)+1)
//-- TIMER sub irqs -----------------------------------------------------------------
#define TIMER_SUB_IRQ_BASE	GPIO_SUB_IRQ_END
#define IRQ_TIMER_SUB(x)	(TIMER_SUB_IRQ_BASE + (x))
#define IRQ_TIMER0			IRQ_TIMER_SUB(0)
#define IRQ_TIMER1			IRQ_TIMER_SUB(1)
#define IRQ_TIMER2			IRQ_TIMER_SUB(2)
#define IRQ_TIMER3			IRQ_TIMER_SUB(3)
#define TIMER_SUB_IRQ_END	IRQ_TIMER_SUB(4)
//-----------------------------------------------------------------------------------

#define NR_IRQS				TIMER_SUB_IRQ_END	

#define IRQ_TO_GPIO(x)		((x) - GPIO_SUB_IRQ_BASE)

#endif /* _ASM_ARCH_IRQS_H */
