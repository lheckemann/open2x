/*
 *  linux/arch/arm/mach-mmsp2/irq.c
 *
 *  Copyright (c) 2004 MIZI Research, Inc. All rights reserved.
 *
 *  2004-04-07  Janghoon Lyu
 *              - created.
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>

#define CHAINED_GPIO	IRQ_GRP_GPIOA

#define GPIO_DEBUG 0

#if GPIO_DEBUG
#define GPRINTK(x...) printk(x)
#else
#define GPRINTK(x...) {do {} while(0);}
#endif


static const unsigned long p_regs[] = {
	[0] = IRQ_DUMMY,
	[1] = IRQ_DUMMY,
	[GRP_OFS(IRQ_GRP_TIMER)] = io_p2v(0xc0000a16),	/* timer status*/
	[GRP_OFS(IRQ_GRP_UART)]  = io_p2v(0xc0001280),	/* UART status */
	[GRP_OFS(IRQ_GRP_GPIOA)] = io_p2v(0xc0001100),
	[GRP_OFS(IRQ_GRP_GPIOB)] = io_p2v(0xc0001102),
	[GRP_OFS(IRQ_GRP_GPIOC)] = io_p2v(0xc0001104),
	[GRP_OFS(IRQ_GRP_GPIOD)] = io_p2v(0xc0001106),
	[GRP_OFS(IRQ_GRP_GPIOE)] = io_p2v(0xc0001108),
	[GRP_OFS(IRQ_GRP_GPIOF)] = io_p2v(0xc000110a),
	[GRP_OFS(IRQ_GRP_GPIOG)] = io_p2v(0xc000110c),
	[GRP_OFS(IRQ_GRP_GPIOH)] = io_p2v(0xc000110e),
	[GRP_OFS(IRQ_GRP_GPIOI)] = io_p2v(0xc0001110),
	[GRP_OFS(IRQ_GRP_GPIOJ)] = io_p2v(0xc0001112),
	[GRP_OFS(IRQ_GRP_GPIOK)] = io_p2v(0xc0001114),
	[GRP_OFS(IRQ_GRP_GPIOL)] = io_p2v(0xc0001116),
	[GRP_OFS(IRQ_GRP_GPIOM)] = io_p2v(0xc0001118),
	[GRP_OFS(IRQ_GRP_GPION)] = io_p2v(0xc000111a),
	[GRP_OFS(IRQ_GRP_GPIOO)] = io_p2v(0xc000111c),
	[GRP_OFS(IRQ_GRP_DMA)] = io_p2v(0xc0000000),
};


static const unsigned long m_regs[] = {
	[0] = IRQ_DUMMY,
	[1] = IRQ_DUMMY,
	[GRP_OFS(IRQ_GRP_TIMER)] = io_p2v(0xc0000a18),	/* timer interrupt */
	[GRP_OFS(IRQ_GRP_UART)]  = IRQ_DUMMY,		/* none for UART */
	[GRP_OFS(IRQ_GRP_GPIOA)] = io_p2v(0xc00010e0),
	[GRP_OFS(IRQ_GRP_GPIOB)] = io_p2v(0xc00010e2),
	[GRP_OFS(IRQ_GRP_GPIOC)] = io_p2v(0xc00010e4),
	[GRP_OFS(IRQ_GRP_GPIOD)] = io_p2v(0xc00010e6),
	[GRP_OFS(IRQ_GRP_GPIOE)] = io_p2v(0xc00010e8),
	[GRP_OFS(IRQ_GRP_GPIOF)] = io_p2v(0xc00010ea),
	[GRP_OFS(IRQ_GRP_GPIOG)] = io_p2v(0xc00010ec),
	[GRP_OFS(IRQ_GRP_GPIOH)] = io_p2v(0xc00010ee),
	[GRP_OFS(IRQ_GRP_GPIOI)] = io_p2v(0xc00010f0),
	[GRP_OFS(IRQ_GRP_GPIOJ)] = io_p2v(0xc00010f2),
	[GRP_OFS(IRQ_GRP_GPIOK)] = io_p2v(0xc00010f4),
	[GRP_OFS(IRQ_GRP_GPIOL)] = io_p2v(0xc00010f6),
	[GRP_OFS(IRQ_GRP_GPIOM)] = io_p2v(0xc00010f8),
	[GRP_OFS(IRQ_GRP_GPION)] = io_p2v(0xc00010fa),
	[GRP_OFS(IRQ_GRP_GPIOO)] = io_p2v(0xc00010fc),
	[GRP_OFS(IRQ_GRP_DMA)] = IRQ_DUMMY,
};

static const unsigned int r_irqs[32] = {
	[3]		= IRQ_DUMMY,
	[4]		= IRQ_DUMMY,
	[21]		= IRQ_DUMMY,			/* irq 21, dummy */
	[22]		= IRQ_DUMMY,			/* irq 22, dummy */
	[IRQ_DISP]	= IRQ_DISP,			/* irq 0, main */
	[IRQ_IMGH] 	= IRQ_IMGH,			/* irq 1, main */
	[IRQ_IMGV]	= IRQ_IMGV,			/* irq 2, main */
	[IRQ_TIMER]	= IRQ_GRP_TIMER | IRQ_TIMER,	/* irq 5, chained */
	[IRQ_MSTICK]	= IRQ_MSTICK,			/* irq 6, main */
	[IRQ_SSP]	= IRQ_SSP,			/* irq 7, main */
	[IRQ_PPM]	= IRQ_PPM,			/* irq 8, main */
	[IRQ_DMA]	= IRQ_GRP_DMA | IRQ_DMA,	/* irq 9, chanined */
	[IRQ_UART]	= IRQ_GRP_UART | IRQ_UART,	/* irq 10, chained */
	[IRQ_GRP2D]	= IRQ_GRP2D,			/* irq 11, main */
	[IRQ_SCALER]	= IRQ_SCALER,			/* irq 12, main */
	[IRQ_USBH]	= IRQ_USBH,			/* irq 13, main */
	[IRQ_SD]	= IRQ_SD,			/* irq 14, main */
	[IRQ_USBD]	= IRQ_USBD,			/* irq 15, main */
	[IRQ_RTC]	= IRQ_RTC,			/* irq 16, main */
	[IRQ_ADC]	= IRQ_ADC,			/* irq 17, main */
	[IRQ_IIC]	= IRQ_IIC,			/* irq 18, main */
	[IRQ_AC97]	= IRQ_AC97,			/* irq 19, main */
	[IRQ_IRDA]	= IRQ_IRDA,			/* irq 20, main */
	[IRQ_GPIO]	= CHAINED_GPIO | IRQ_GPIO,	/* irq 23, chained */
	[IRQ_CDROM]	= IRQ_CDROM,			/* irq 24, main */
	[IRQ_OWM]	= IRQ_OWM,			/* irq 25, main */
	[IRQ_DUALCPU]	= IRQ_DUALCPU,			/* irq 26, main */
	[IRQ_MCUC]	= IRQ_MCUC,			/* irq 27, main */
	[IRQ_VLD]	= IRQ_VLD,			/* irq 28, main */
	[IRQ_VIDEO]	= IRQ_VIDEO,			/* irq 29, main */
	[IRQ_MPEGIF]	= IRQ_MPEGIF,			/* irq 30, main */
	[IRQ_IIS]	= IRQ_IIS			/* irq 31, main */
};

unsigned int __inline__ fixup_gpio_irq(int irq)
{
#if 1
	int i;
	volatile unsigned short v = GPIOGREVTSTAT & 0xffff;
	unsigned int r;

	if (v == 0) return 0;

	for (i = 0; i < 16; i++)
		if (v & (1<<i)) 
			break;

	r = (i + GRP_OFS(CHAINED_GPIO));

	GPRINTK("r = %d\n", r);

	return r;
#else
	unsigned int ofs;
	unsigned int start = GRP_OFS(IRQ_GRP_GPIOA);
	unsigned int end = GRP_OFS(IRQ_GRP_GPIOO);
	volatile unsigned short v;
	for (ofs = start; ofs < end + 1; ofs++) {
		v = *(volatile unsigned short*)(p_regs[ofs]);
		if (v != 0)
			break;
	}
	if (ofs > end) return 0;
	return ofs;
#endif
}

int fixup_irq(int irq)
{
	unsigned int i, r, o;
	volatile unsigned short v;
	unsigned long addr;

	o = GRP_OFS(r_irqs[irq]);

//	if (GPIOGREVTSTAT != 0)
//		printk("0x%04x\n", GPIOGREVTSTAT);

	if (o == GRP_OFS(CHAINED_GPIO)) {
		o = fixup_gpio_irq(irq);
		GPRINTK("o = %d, pend_reg(0x%08lx) = 0x%04x\n", 
			o, p_regs[o], 
			*(volatile unsigned short *)(p_regs[o]));
		if (o == 0) BUG();
	}
	if (o == 0) return irq;	/* It's main irq ?? */

	/* It's chained irq */
	addr = p_regs[o];
	if (addr == IRQ_DUMMY) return IRQ_DUMMY;

	v = *(volatile unsigned short *)(p_regs[o]);
	for (i = 0; i < IRQ_GRP_MAX_IRQ; i++) {
		if (v & (1 << i))
			break;
	}

	if (i == IRQ_GRP_MAX_IRQ)
		return IRQ_DUMMY;

	r = i + o * IRQ_GRP_MAX_IRQ;

	if (irq_desc[r].valid == 0) {
		return IRQ_DUMMY;
	}

	return r;
}

/* 임시, 미완성 */
static const int sub_to_main[] = {
	[IRQ_TXD0] = IRQ_UART,
	[IRQ_RXD0] = IRQ_UART,
	[IRQ_TXD1] = IRQ_UART,
	[IRQ_RXD1] = IRQ_UART,
	[IRQ_TXD2] = IRQ_UART,
	[IRQ_RXD2] = IRQ_UART,
	[IRQ_TXD3] = IRQ_UART,
	[IRQ_RXD3] = IRQ_UART,
	[IRQ_TIMER0] = IRQ_TIMER,
	[IRQ_TIMER1] = IRQ_TIMER,
	[IRQ_TIMER2] = IRQ_TIMER,
	[IRQ_TIMER3] = IRQ_TIMER,
	[IRQ_DMA0] = IRQ_DMA,
	[IRQ_DMA1] = IRQ_DMA,
	[IRQ_DMA2] = IRQ_DMA,
	[IRQ_DMA3] = IRQ_DMA,
	[IRQ_DMA4] = IRQ_DMA,
	[IRQ_DMA5] = IRQ_DMA,
	[IRQ_DMA6] = IRQ_DMA,
	[IRQ_DMA7] = IRQ_DMA,
	[IRQ_DMA8] = IRQ_DMA,
	[IRQ_DMA9] = IRQ_DMA,
	[IRQ_DMA10] = IRQ_DMA,
	[IRQ_DMA11] = IRQ_DMA,
	[IRQ_DMA12] = IRQ_DMA,
	[IRQ_DMA13] = IRQ_DMA,
	[IRQ_DMA14] = IRQ_DMA,
	[IRQ_DMA15] = IRQ_DMA,
};

static void mmsp2_mask_ack_irq_main(unsigned int irq)
{
	unsigned int ofs = (1 << irq);
	INTMASK |= ofs;
	SRCPEND = ofs;
	INTPEND = ofs;
}

static void mmsp2_mask_ack_irq_sub(unsigned int irq)
{
	int main_irq = sub_to_main[irq];
	unsigned int main_offset = (1 << main_irq);
	unsigned int sub = GRP_NUM(irq);
	unsigned int sub_offset = (1<<(irq & 0xf));
	unsigned long mask_addr = m_regs[sub];
	unsigned long pend_addr = p_regs[sub];

#if 0
	/* 임시 : 테이블 미완성 */
	if (!main_irq)
		main_irq = irq;
#endif

	/* mask */
	if (mask_addr != IRQ_DUMMY) {
		/* 타이머만 이런지 다른 것들도 이런지 반드시 확인 */
		*(volatile unsigned short *)(mask_addr) &= ~(sub_offset);
	} 

	/* pend */
	if (pend_addr != IRQ_DUMMY) {
		/* 우쒸... */
		if (main_irq == IRQ_DMA) {
			unsigned long a = io_p2v(0xc0000200+(0x10*(irq & 0xf))+0x6);
			/* 이놈은 '0'을 쓰줘야 지워진다. */
			*(volatile unsigned short *)(a) &= ~(0x3);
		}
		/* 타이머, UART 만 이런지 다른 것들도 이런지 반드시 확인 */
		*(volatile unsigned short *)(pend_addr) = (sub_offset);
	}

	SRCPEND = main_offset;
	INTPEND = main_offset;
}

/* sub_to_main[] 테이블이 미완성이라 일단 분리 한다 */
static void mmsp2_mask_ack_irq_gpio(unsigned int irq)
{
	unsigned int sub = GRP_NUM(irq);
	unsigned int sub_offset = (1<<(irq & 0xf));
	unsigned long mask_addr = m_regs[sub];
	unsigned long pend_addr = p_regs[sub];

	/* pend */
	*(volatile unsigned short *)(pend_addr) = (sub_offset);

	/* mask */
	*(volatile unsigned short *)(mask_addr) &= ~(sub_offset);
	
	GPRINTK("mask_ack: irq = %d, sub_offset = %d\n", irq, sub_offset);
	GPRINTK("mask_ack: paddr = 0x%08lx, maddr = 0x%08lx, bit = 0x%04x\n", 
		pend_addr, mask_addr, sub_offset);
	GPRINTK("mask_ack: so 0x%04x, 0x%04x\n",
		*(volatile unsigned short *)(mask_addr), *(volatile unsigned short *)(pend_addr));

	SRCPEND = (1 << IRQ_GPIO);
	INTPEND = (1 << IRQ_GPIO);
//	INTMASK &= ~(1<< IRQ_GPIO);
}

static void mmsp2_mask_irq_main(unsigned int irq)
{
	INTMASK |= (1<<irq);
}

static void mmsp2_mask_irq_sub(unsigned int irq)
{
	unsigned int sub = GRP_NUM(irq);
	unsigned int sub_offset = (1<<(irq & 0xf));
	unsigned long mask_addr = m_regs[sub];

	if (mask_addr != IRQ_DUMMY) {
		/* 
		 * mask(1)/unmask(0) 가 아니라 enable(1)/disable(0) 개념
		 *
		 * 타이머만 이런지 다른 것들도 이런지 반드시 확인
		 */

		*(volatile unsigned short *)(mask_addr) &= ~(sub_offset);
	} 
}

static void mmsp2_mask_irq_gpio(unsigned int irq)
{
	unsigned int sub = GRP_NUM(irq);
	unsigned int sub_offset = (1<<(irq & 0xf));
	unsigned long mask_addr = m_regs[sub];

	*(volatile unsigned short *)(mask_addr) &= ~(sub_offset);

	GPRINTK("mask: sub_offset = %d\n", sub_offset);
	GPRINTK("mask: irq = %d, maddr = 0x%08lx, bit = 0x%04x, so 0x%04x\n",
		irq, mask_addr, sub_offset, *(volatile unsigned short *)(mask_addr));
}

static void mmsp2_unmask_irq_main(unsigned int irq)
{
	INTMASK &= ~(1<<irq);
}

static void mmsp2_unmask_irq_sub(unsigned int irq)
{
//	unsigned int main_offset = 1 << sub_to_main[irq];
	unsigned int sub_offset = (1<<(irq & 0xf));
	unsigned long mask_addr = m_regs[GRP_NUM(irq)];
	unsigned long pend_addr = p_regs[GRP_NUM(irq)];

	if (pend_addr != IRQ_DUMMY) {
		/* clear pending */
		if (*(volatile unsigned short *)(pend_addr) & sub_offset)
			 *(volatile unsigned short *)(pend_addr) = sub_offset;
	}

	if (mask_addr != IRQ_DUMMY) {
		/* 
		 * mask(1)/unmask(0) 가 아니라 enable(1)/disable(0) 개념
		 *
		 * 타이머만 이런지 다른 것들도 이런지 반드시 확인
		 */

		/* enable */
		*(volatile unsigned short *)(mask_addr) |= (sub_offset);
	}

#if 0
	/* 임시 : 테이블 미완성 */
	if (!main_offset)
		main_offset = 1 << irq;
	SRCPEND = main_offset; // needed ?
	INTPEND = main_offset; // needed ?
#endif
}

static void mmsp2_unmask_irq_gpio(unsigned int irq)
{
	unsigned int sub_offset = (1<<(irq & 0xf));
	unsigned long mask_addr = m_regs[GRP_NUM(irq)];
	unsigned long pend_addr = p_regs[GRP_NUM(irq)];

	/* clear pending */
	if (*(volatile unsigned short *)(pend_addr) & sub_offset)
		 *(volatile unsigned short *)(pend_addr) = sub_offset;

	/* enable */
	*(volatile unsigned short *)(mask_addr) |= (sub_offset);

	GPRINTK("unmask: sub_offset = %d\n", sub_offset);
	GPRINTK("unmask: irq = %d, maddr = 0x%08lx, bit = 0x%04x, so 0x%04x\n",
		irq, mask_addr, sub_offset, *(volatile unsigned short *)(mask_addr));
	GPRINTK("unmask: paddr = 0x%08lx, 0x%04x\n",
		pend_addr, *(volatile unsigned short *)(pend_addr));
	GPRINTK("INTMASK = 0x%08x, %d\n", INTMASK, INTMASK & (1<<IRQ_GPIO) ? 1:0);
	GPRINTK("INTMOD = 0x%08x, IPRIORITY = 0x%08x\n", INTMOD, IPRIORITY);
	GPRINTK("SRCPEND = 0x%08x, INTPEND = 0x%08x\n", SRCPEND, INTPEND);

	SRCPEND = (1<<IRQ_GPIO);
	if (INTPEND & (1<<IRQ_GPIO))
		INTPEND = (1<<IRQ_GPIO);
//	INTMASK &= ~(1<<IRQ_GPIO);
}


void __init mmsp2_init_irq(void)
{
	int irq;

	/* disable all interrupts */
	INTMASK = 0xffffffff;
	INTMOD = 0x00000000;	/* all interrupts is irq mode */

	IPRIORITY = 0x0;	/* disable rotation */
	
	/* clear all pending registers as far as I know */
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_TIMER)]) = 0xffff; /* TIMER */
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_UART)]) = 0xffff; /* UART */

	/* disable all GPIO interrupts */
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOA)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOB)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOC)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOD)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOE)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOF)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOG)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOH)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOI)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOJ)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOK)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOL)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOM)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPION)]) = 0;
	*(volatile unsigned short *)(m_regs[GRP_OFS(IRQ_GRP_GPIOO)]) = 0;

	/* clear all GPIO pending registers */
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOA)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOB)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOC)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOD)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOE)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOF)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOG)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOH)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOI)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOJ)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOK)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOL)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOM)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPION)]) = 0xffff;
	*(volatile unsigned short *)(p_regs[GRP_OFS(IRQ_GRP_GPIOO)]) = 0xffff;

	GPIODEVTTYPLOW = 0xffff; // hbahn

	SRCPEND = 0xffffffff;
	INTPEND = 0xffffffff;

	for (irq = 0; irq < NR_IRQS; irq++) {
		irq_desc[irq].mask_ack = mmsp2_mask_ack_irq_main;
		irq_desc[irq].mask = mmsp2_mask_irq_main;
		irq_desc[irq].unmask = mmsp2_unmask_irq_main;
	}

	/* 대표성이 있는 irq는 invalid로 */
	for (irq = 0; irq < 32; irq++) {
		if (r_irqs[irq] & IRQ_CHAIN_MASK)
			continue;
		irq_desc[irq].valid = 1;
		irq_desc[irq].probe_ok = 1;
	}

	/* 실제로 존재하는 IRQ만 골라서 풀어준다. */
	for (irq = IRQ_TIMER0; irq < (IRQ_TIMER0+4); irq++) {
		irq_desc[irq].valid = 1;
		irq_desc[irq].probe_ok = 1;
		irq_desc[irq].mask_ack = mmsp2_mask_ack_irq_sub;
		irq_desc[irq].mask = mmsp2_mask_irq_sub;
		irq_desc[irq].unmask = mmsp2_unmask_irq_sub;
	}

	for (irq = IRQ_TXD0; irq < (IRQ_TXD0+16); irq++) {
		irq_desc[irq].valid = 1;
		irq_desc[irq].probe_ok = 1;
		irq_desc[irq].mask_ack = mmsp2_mask_ack_irq_sub;
		irq_desc[irq].mask = mmsp2_mask_irq_sub;
		irq_desc[irq].unmask = mmsp2_unmask_irq_sub;
	}

	/* T.T */
	for (irq = IRQ_GPIO_A0; irq < (IRQ_GPIO_O15 + 1); irq++) {
		irq_desc[irq].valid = 1;
		irq_desc[irq].probe_ok = 1;
		irq_desc[irq].mask_ack = mmsp2_mask_ack_irq_gpio;
		irq_desc[irq].mask = mmsp2_mask_irq_gpio;
		irq_desc[irq].unmask = mmsp2_unmask_irq_gpio;

#if 0
		/* mask, pending */
		irq_desc[irq].mask_ack(irq);
#endif
	}

	for (irq = IRQ_DMA0; irq < (IRQ_DMA0+16); irq++) {
		irq_desc[irq].valid = 1;
		irq_desc[irq].probe_ok = 1;
		irq_desc[irq].mask_ack = mmsp2_mask_ack_irq_sub;
		irq_desc[irq].mask = mmsp2_mask_irq_sub;
		irq_desc[irq].unmask = mmsp2_unmask_irq_sub;
	}

	/* 대표성이 있는 IRQ는 풀어준다. */
	for (irq = 0; irq < 32; irq++) {
		if (r_irqs[irq] & IRQ_CHAIN_MASK) {
			INTMASK &= ~(1 << irq);
		}
	}
}
