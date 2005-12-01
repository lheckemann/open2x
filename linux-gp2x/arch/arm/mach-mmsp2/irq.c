/*
 *  linux/arch/arm/mach-mmsp2/irq.c
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
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
#include <asm/arch/mmsp20.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(x...) printk(x)
#else
#  define ds_printk(fmt, args...)
#endif

#define SDIIMSK1 __REG(0xC0001530)
#define SDIIMSK0 __REG(0xC0001532)
#define SDIDSTA __REGW(0xC0001528)
#define SDIDSTA_DFIN    (1 << 4)

/* Main IRQ */
static void mmsp2_mask_ack_irq_main(unsigned int irq)
{
	unsigned int ofs = (1 << irq);

	INTMASK |= ofs;		// interrupt mask
	SRCPEND = ofs;		// source pending flag clear
	INTPEND = ofs;		// interrupt pending flag clear
}

static void mmsp2_mask_irq_main(unsigned int irq)
{
	INTMASK |= (1<<irq);
}

static void mmsp2_unmask_irq_main(unsigned int irq)
{
	INTMASK &= ~(1<<irq);
}

/* DMA IRQ */
static void mmsp2_DMA_demux(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	int i;
	unsigned short event = DMAINT; //*(volatile unsigned short *)io_p2v(0xc0000000);
	
	for(i=0; i<16; i++) {
		if(event & (1<<i)) break;
	}
	
	if(i == 16) return;
	
	do_IRQ(IRQ_DMA_SUB(i), regs);
}

static struct irqaction DMA_irqaction = {
	name:		"DMA DEMUX",
	handler:	mmsp2_DMA_demux,
	flags:		SA_INTERRUPT
};

static void mmsp2_mask_ack_irq_dma(unsigned int irq)
{
	unsigned long control = io_p2v(0xc0000206+(0x10*(irq & 0xf)));	// DMACONSx (x:0~15)
	*(volatile unsigned short *)(control) &= ~0x20;					// dma interrupt disable	
	*(volatile unsigned short *)(control) &= ~0x2;					// REQPEND
}

static void mmsp2_mask_irq_dma(unsigned int irq)
{
	unsigned long control = io_p2v(0xc0000206+(0x10*(irq & 0xf)));	// DMACONSx (x:0~15)
	*(volatile unsigned short *)(control) &= ~0x20;					// dma interrupt disable	
}

static void mmsp2_unmask_irq_dma(unsigned int irq)
{
	unsigned long control = io_p2v(0xc0000206+(0x10*(irq & 0xf)));	// DMACONSx (x:0~15)
	*(volatile unsigned short *)(control) |= 0x20;					// dma interrupt enable	
}

/* UART IRQ */
static void mmsp2_UART_demux(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	int i;
	unsigned short event = *(volatile unsigned short *)io_p2v(0xc0001280);
	
	for(i=0; i<16; i++) {
		if(event & (1<<i)) break;
	}
	
	if(i == 16) return;
	
	do_IRQ(IRQ_UART_SUB(i), regs);
}

static struct irqaction UART_irqaction = {
	name:		"UART DEMUX",
	handler:	mmsp2_UART_demux,
	flags:		SA_INTERRUPT
};

static void mmsp2_mask_ack_irq_uart(unsigned int irq)
{
	int group, index;
	unsigned long mask_addr;
	
	group = irq>>2;
	index = irq&0x3;

	if(index == 3) {			// modem interrupt
		mask_addr = io_p2v(0xc0001206+(0x20*group));
		*(volatile unsigned short *)(mask_addr) &= ~0x8;	// modem interrupt disable
	} else {
		mask_addr = io_p2v(0xc0001202+(0x20*group));
		if(index == 0) {		// tx interrupt
			*(volatile unsigned short *)(mask_addr) &= ~0xC;
		} else if(index == 1) {	// rx interrupt
			*(volatile unsigned short *)(mask_addr) &= ~0x3;
		} else {				// error interrupt
			*(volatile unsigned short *)(mask_addr) &= ~0x40;
		}
	}

	*(volatile unsigned short *)io_p2v(0xc0001280) = (1<<(irq & 0xf));	// clear pending flag 
}

static void mmsp2_mask_irq_uart(unsigned int irq)
{
	int group, index;
	unsigned long mask_addr;
	
	group = irq>>2;
	index = irq&0x3;

	if(index == 3) {			// modem interrupt
		mask_addr = io_p2v(0xc0001206+(0x20*group));
		*(volatile unsigned short *)(mask_addr) &= ~0x8;	// modem interrupt disable
	} else {
		mask_addr = io_p2v(0xc0001202+(0x20*group));
		if(index == 0) {		// tx interrupt
			*(volatile unsigned short *)(mask_addr) &= ~0xC;
		} else if(index == 1) {	// rx interrupt
			*(volatile unsigned short *)(mask_addr) &= ~0x3;
		} else {				// error interrupt
			*(volatile unsigned short *)(mask_addr) &= ~0x40;
		}
	}
}

static void mmsp2_unmask_irq_uart(unsigned int irq)
{
	int group, index;
	unsigned long mask_addr;
	
	group = irq>>2;
	index = irq&0x3;

	if(index == 3) {			// modem interrupt
		mask_addr = io_p2v(0xc0001206+(0x20*group));
		*(volatile unsigned short *)(mask_addr) |= 0x8;	// modem interrupt disable
	} else {
		mask_addr = io_p2v(0xc0001202+(0x20*group));
		if(index == 0) {		// tx interrupt
			*(volatile unsigned short *)(mask_addr) &= ~0xC;
			*(volatile unsigned short *)(mask_addr) |= 0x4;
		} else if(index == 1) {	// rx interrupt
			*(volatile unsigned short *)(mask_addr) &= ~0x3;
			*(volatile unsigned short *)(mask_addr) |= 0x1;
		} else {				// error interrupt
			*(volatile unsigned short *)(mask_addr) |= 0x40;
		}
	}
}

/* Timer IRQ */
static void mmsp2_TIMER_demux(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	int i;
	unsigned short event = *(volatile unsigned short *)io_p2v(0xc0000a16);
	
	for(i=0; i<4; i++) {
		if(event & (1<<i)) break;
	}
	
	if(i == 4) return;
	
	do_IRQ(IRQ_TIMER_SUB(i), regs);
}

static struct irqaction TIMER_irqaction = {
	name:		"TIMER DEMUX",
	handler:	mmsp2_TIMER_demux,
	flags:		SA_INTERRUPT
};

static void mmsp2_mask_ack_irq_timer(unsigned int irq)
{
	*(volatile unsigned short *)io_p2v(0xc0000a18) &= ~(1<<(irq&0x3));	//mask
	*(volatile unsigned short *)io_p2v(0xc0000a16) = (1<<(irq&0x3));	//clear pending flag
}

static void mmsp2_mask_irq_timer(unsigned int irq)
{
	*(volatile unsigned short *)io_p2v(0xc0000a18) &= ~(1<<(irq&0x3));	//mask
}

static void mmsp2_unmask_irq_timer(unsigned int irq)
{
	*(volatile unsigned short *)io_p2v(0xc0000a18) |= (1<<(irq&0x3));	//mask
}

/* GPIO IRQ */
/*
 * Demux handler for GPIOA[15:0] ~ GPIOO[15:0] interrupts
 */
#define GPIO_EVT(i)		(*(volatile unsigned short *)io_p2v(0xc0001100 + 2*(i)))
#define GPIO_INTENB(i)	(*(volatile unsigned short *)io_p2v(0xc00010e0 + 2*(i)))

static void mmsp2_GPIO_demux(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	int i, j;
	unsigned int gpio_irq;
	volatile unsigned short gpio_event, group_event;
	
	// Read GPIO Group Event
	group_event = __REGW(0xc0001140)/*GPIOGREVTSTAT*/ & 0x7fff;
	if(group_event == 0) return;
	
	i=0;
	while(1){
		for(; i<15; i++)	// GPIOA ~ GPIOO
			if(group_event & (1<<i)) break;
		
		// if group search finished, return
		if(i == 15) return;
		
		// Read GPIO sub Event
		gpio_event = GPIO_EVT(i);
		if(gpio_event == 0) {	// if there is no event, 
			++i;		// search next group event.
			continue;
		}

		for(j=0; j<16; j++) {	// GPIOx[15:0]
			if(gpio_event & (1<<j)) {
				gpio_irq = GPIO_SUB_IRQ_BASE + (i << 4) + j;
				ds_printk("irq = 0x%03x, group_event = 0x%04x, gpio_event = 0x%04x\n", gpio_irq, group_event, gpio_event);
				do_IRQ(gpio_irq, regs);
			}		
		}
	}
}

static struct irqaction GPIO_irqaction = {
	name:		"GPIO DEMUX",
	handler:	mmsp2_GPIO_demux,
	flags:		SA_INTERRUPT
};

static void mmsp2_mask_ack_irq_gpio(unsigned int irq)
{
	unsigned short gpio_nr = IRQ_TO_GPIO(irq);	// gpio_nr : 0~239 == 0x00~0xEF
	unsigned short mask = 1 << (gpio_nr & 0xf);
	unsigned short index = gpio_nr >> 4;		// (GPIOA = 0) ~ (GPIOO = 14)

	GPIO_INTENB(index) &= ~mask;				// mask
	GPIO_EVT(index) = mask;						// pending irq
}

static void mmsp2_mask_irq_gpio(unsigned int irq)
{
	unsigned short gpio_nr = IRQ_TO_GPIO(irq);	// gpio_nr : 0~239 == 0x00~0xEF
	unsigned short mask = 1 << (gpio_nr & 0xf);
	unsigned short index = gpio_nr >> 4;		// (GPIOA = 0) ~ (GPIOO = 14)

	GPIO_INTENB(index) &= ~mask;				// mask
}

static void mmsp2_unmask_irq_gpio(unsigned int irq)
{
	unsigned short gpio_nr = IRQ_TO_GPIO(irq);	// gpio_nr : 0~239 == 0x00~0xEF
	unsigned short mask = 1 << (gpio_nr & 0xf);
	unsigned short index = gpio_nr >> 4;		// (GPIOA = 0) ~ (GPIOO = 14)

	GPIO_INTENB(index) |= mask;					// mask
}

void __init mmsp2_init_irq(void)
{
	int i;
	int irq;

	/* disable all interrupts */
	INTMASK = 0xffffffff;
	INTMOD = 0x00000000;	/* all interrupts is irq mode */

	IPRIORITY = 0x0;	/* disable rotation */
	
	/* clear all pending registers as far as I know */
	TSTATUS = 0xffff; /* TIMER */
	UINTSTAT = 0xffff; /* UART */
	
	for(i=0; i<15; i++)	
	{
		GPIO_INTENB(i) = 0;		/* disable all GPIO interrupts */
		GPIO_EVT(i)	= 0xffff;	/* clear all GPIO pending registers */
	}

	SRCPEND = 0xffffffff;
	INTPEND = 0xffffffff;

	/* main irq description init */
	for (irq = 0; irq < 32; irq++) {
		irq_desc[irq].valid = 1;
		irq_desc[irq].probe_ok = 1;
		irq_desc[irq].mask_ack = mmsp2_mask_ack_irq_main;
		irq_desc[irq].mask = mmsp2_mask_irq_main;
		irq_desc[irq].unmask = mmsp2_unmask_irq_main;
	}
	/* Those are reserved */
	irq_desc[3].valid = 0;
	irq_desc[4].valid = 0;
	irq_desc[21].valid = 0;
	irq_desc[22].valid = 0;

	/* dma irq description init */
	for (irq = DMA_SUB_IRQ_BASE; irq < DMA_SUB_IRQ_END; irq++) {
		irq_desc[irq].valid = 1;
		irq_desc[irq].probe_ok = 1;
		irq_desc[irq].mask_ack = mmsp2_mask_ack_irq_dma;
		irq_desc[irq].mask = mmsp2_mask_irq_dma;
		irq_desc[irq].unmask = mmsp2_unmask_irq_dma;
	}
	setup_arm_irq(IRQ_DMA, &DMA_irqaction);

	/* uart irq description init */
	for (irq = UART_SUB_IRQ_BASE; irq < UART_SUB_IRQ_END; irq++) {
		irq_desc[irq].valid = 1;
		irq_desc[irq].probe_ok = 1;
		irq_desc[irq].mask_ack = mmsp2_mask_ack_irq_uart;
		irq_desc[irq].mask = mmsp2_mask_irq_uart;
		irq_desc[irq].unmask = mmsp2_unmask_irq_uart;
	}
	setup_arm_irq(IRQ_UART, &UART_irqaction);

	/* gpio irq description init */
	for (irq = GPIO_SUB_IRQ_BASE; irq < GPIO_SUB_IRQ_END; irq++) {
		irq_desc[irq].valid = 1;
		irq_desc[irq].probe_ok = 1;
		irq_desc[irq].mask_ack = mmsp2_mask_ack_irq_gpio;
		irq_desc[irq].mask = mmsp2_mask_irq_gpio;
		irq_desc[irq].unmask = mmsp2_unmask_irq_gpio;
	}
	setup_arm_irq(IRQ_GPIO, &GPIO_irqaction);

	/* timer irq description init */
	for (irq = TIMER_SUB_IRQ_BASE; irq < TIMER_SUB_IRQ_END; irq++) {
		irq_desc[irq].valid = 1;
		irq_desc[irq].probe_ok = 1;
		irq_desc[irq].mask_ack = mmsp2_mask_ack_irq_timer;
		irq_desc[irq].mask = mmsp2_mask_irq_timer;
		irq_desc[irq].unmask = mmsp2_unmask_irq_timer;
	}
	setup_arm_irq(IRQ_TIMER, &TIMER_irqaction);
}
