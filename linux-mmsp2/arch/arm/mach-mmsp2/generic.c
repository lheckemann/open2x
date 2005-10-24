/*
 *  linux/arch/arm/mach-mmsp2/generic.c
 *
 *  Copyright (c) 2004 MIZI Research, Inc. All rights reserved.
 *
 *  2004-04-06  Janghoon Lyu
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
#include <linux/kernel.h>
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/system.h>
#include <asm/pgtable.h>
#include <asm/mach/map.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/timex.h>	/* for CLOCK_TICK_RATE */

#include "generic.h"

#define _DEBUG 0

#if _DEBUG
#define DPRINTK(x...) printk(x)
#else
#define DPRINTK(x...) {do {} while(0);}
#endif

#define CONFIG_CPU_GPIO_HAS_DUMB_BUG 1

#if CONFIG_CPU_GPIO_HAS_DUMB_BUG == 0
void set_gpio_ctrl(unsigned int num, unsigned int mode, unsigned int pu)
{
	unsigned long base = (0x8 & num) ? 0xc0001040 : 0xc0001020;
	unsigned long addr = io_p2v(base + (0x2 * ((0xf0 & num) >> 4)));
	unsigned long bit_ofs = (0x7 & num) * 2;
	unsigned short temp;

	DPRINTK("%s(): gpio = %04x(GPIO %c[%d])\n",
		 	__FUNCTION__, num, GRP_NUM(num)+'A', num & 0xf);

	temp = readw(addr);
	temp &= ~(0x3 << bit_ofs);
	temp |= (mode << bit_ofs);
	writew(temp, addr);

	if (pu == GPIOPU_NOSET)
		return;

	addr = io_p2v(0xc00010c0 + (0x2 * ((0xf0 & num) >> 4)));
	bit_ofs = num & 0xf;
	temp = readw(addr);
	temp &= ~(1 << bit_ofs);
	temp |= (pu << bit_ofs);
	writew(temp, addr);
}
#else

/* buggy CPU */

const unsigned int _pins_of_gpio[] = {
	[GRP_NUM(GPIO_A0)] = 16,
	[GRP_NUM(GPIO_B0)] = 16,
	[GRP_NUM(GPIO_C0)] = 16,
	[GRP_NUM(GPIO_D0)] = 14,
	[GRP_NUM(GPIO_E0)] = 16,
	[GRP_NUM(GPIO_F0)] = 10,
	[GRP_NUM(GPIO_G0)] = 16,
	[GRP_NUM(GPIO_H0)] = 7,
	[GRP_NUM(GPIO_I0)] = 16,
	[GRP_NUM(GPIO_J0)] = 16,
	[GRP_NUM(GPIO_K0)] = 8,
	[GRP_NUM(GPIO_L0)] = 15,
	[GRP_NUM(GPIO_M0)] = 9,
	[GRP_NUM(GPIO_N0)] = 8,
	[GRP_NUM(GPIO_O0)] = 6,
};

/* for ALT and EVT_TYPE */
unsigned int _get_mmsp2_gpio_func(unsigned int addr_l, unsigned int addr_h, unsigned int pins, unsigned int bit)
{
	unsigned short value_l = readw(addr_l);
	unsigned short value_h = readw(addr_h);
	unsigned int *addr_t = &addr_l;
	unsigned short *value_t = &value_l;
	unsigned int bitofs_t = bit * 2;

	value_l |= (value_h << pins);
	value_h >>= (16-pins);

	if (bit > 7) {
		bitofs_t -= (8*2);
		addr_t = &addr_h;
		value_t = &value_h;
	}

	DPRINTK("%s(): read from 0x%08x is 0x%04x\n", 
					__FUNCTION__, *addr_t, *value_t);

	return ((*value_t >> bitofs_t) & 0x03);
}

void _set_mmsp2_gpio_func(unsigned int addr_l, unsigned int addr_h, unsigned int pins, unsigned int bit, unsigned int func)
{
	unsigned short value_l = readw(addr_l);
	unsigned short value_h = readw(addr_h);
	unsigned int *addr_t = &addr_l;
	unsigned short *value_t = &value_l;
	unsigned int bitofs_t = bit * 2;

	value_l |= (value_h << pins);
	value_h >>= (16-pins);

	if (bit > 7) {
		bitofs_t -= (8*2);
		addr_t = &addr_h;
		value_t = &value_h;
	}

	*value_t &= ~(0x3 << bitofs_t);
	*value_t |= ((func & 0x03) << bitofs_t);

	DPRINTK("%s(): try to set 0x%08x as 0x%04x\n", 
					__FUNCTION__, *addr_t, *value_t);

	writew(*value_t, *addr_t);

	/* verify */
	BUG_ON(func != _get_mmsp2_gpio_func(addr_l, addr_h, pins, bit));
}

void set_gpio_ctrl(unsigned int num, unsigned int mode, unsigned int pu)
{
	unsigned long reg_offset = ((0xf0 & num) >> 3);
	unsigned long addr_l = io_p2v(0xc0001020 + reg_offset);
	unsigned long addr_h = io_p2v(0xc0001040 + reg_offset);
	unsigned long bit_ofs = (0x7 & num) * 2;
	unsigned short temp;

	DPRINTK("%s(): gpio = %04x(GPIO %c[%d])\n",
		 	__FUNCTION__, num, GRP_NUM(num)+'A', num & 0xf);

	_set_mmsp2_gpio_func(addr_l, addr_h, 
			_pins_of_gpio[GRP_NUM(num)], num & 0xf, mode);

	addr_l = io_p2v(0xc00010c0 + reg_offset);
	bit_ofs = num & 0xf;
	temp = readw(addr_l);

	DPRINTK("%s(): PU: read from 0x%08x is 0x%04x\n", 
					__FUNCTION__, addr_l, temp);

	if (pu == GPIOPU_NOSET)
		return;

	temp &= ~(1 << bit_ofs);
	temp |= (pu << bit_ofs);

	DPRINTK("%s(): PU: try to set 0x%08x as 0x%04x\n", 
					__FUNCTION__, addr_l, temp);

	writew(temp, addr_l);

	DPRINTK("%s(): PU: read from 0x%08x is 0x%04x\n",
					__FUNCTION__, addr_l, readw(addr_l));
}
#endif /* CONFIG_CPU_GPIO_HAS_DUMB_BUG */

/*
 * FIXME
 */
#define IRQ_TO_GPIO(irq) \
	( ((GRP_NUM(irq) - GRP_OFS(IRQ_GRP_GPIOA)) << 4) + ((irq) & 0xf) )

#if CONFIG_CPU_GPIO_HAS_DUMB_BUG == 0
void set_external_irq(int irq, unsigned int edge, unsigned int pullup)
{
	unsigned int gpio = IRQ_TO_GPIO(irq);
	unsigned long base = (0x8 & gpio) ? 0xc00010a0 : 0xc0001080;
	unsigned long addr = io_p2v(base + (0x2 * ((0xf0 & gpio) >> 4)));
	unsigned long bit_ofs = (0x7 & gpio) * 2;
	unsigned short temp;

	DPRINTK("%s(): irq = %d(%04x), gpio = %04x(GPIO %c[%d])\n",
		 	__FUNCTION__, irq, irq, gpio, GRP_NUM(gpio)+'A', gpio & 0xf);

	/* as input */
	set_gpio_ctrl(gpio, GPIOMD_IN, pullup);

	temp = readw(addr);
	temp &= ~(0x3 << bit_ofs);
	temp |= (edge << bit_ofs);
	writew(temp, addr);
}
#else

/* buggy CPU */

void _set_gpio_event_type(unsigned int num, unsigned int mode)
{
	unsigned long reg_offset = ((0xf0 & num) >> 3);
	unsigned long addr_l = io_p2v(0xc0001080 + reg_offset);
	unsigned long addr_h = io_p2v(0xc00010a0 + reg_offset);

	DPRINTK("%s(): gpio = %04x(GPIO %c[%d])\n",
		 	__FUNCTION__, num, GRP_NUM(num)+'A', num & 0xf);

	_set_mmsp2_gpio_func(addr_l, addr_h, 
			_pins_of_gpio[GRP_NUM(num)], num & 0xf, mode);
}

void set_external_irq(int irq, unsigned int edge, unsigned int pullup)
{
	unsigned int gpio = IRQ_TO_GPIO(irq);

	DPRINTK("%s(): irq = %d(%04x), gpio = %04x(GPIO %c[%d])\n",
		 	__FUNCTION__, irq, irq, gpio, GRP_NUM(gpio)+'A', gpio & 0xf);

	/* as input */
	set_gpio_ctrl(gpio, GPIOMD_IN, pullup);

	_set_gpio_event_type(gpio, edge);
}

#endif  /* CONFIG_CPU_GPIO_HAS_DUMB_BUG */

EXPORT_SYMBOL(set_gpio_ctrl);
EXPORT_SYMBOL(set_external_irq);

#if 0
unsigned int inline read_gpio_bit(unsigned int num)
{
	unsigned long addr = io_p2v(0xc0001180 + (0x2 * ((0xf0 & num) >> 4)));
	unsigned long bit_ofs = num & 0xf;

	return ((readw(addr) & (1 << bit_ofs)) >> bit_ofs);
}

void inline write_gpio_bit(unsigned int num, unsigned int val)
{
	unsigned long addr = io_p2v(0xc0001060 + (0x2 * ((0xf0 & num) >> 4)));
	unsigned long bit_ofs = num & 0xf;
	unsigned short temp;

	temp = readw(addr);
	temp &= ~(1 << bit_ofs);
	temp |= (val << bit_ofs);
	writew(temp, addr);
}
#endif

/* clock = (m * Fin) / (p * s)
 * m = M + 8
 * p = P + 2
 * s = 2 ^ S
 *
 * Fin = Frequency Input (7372800 Hz)
 * S : Output frequency scaler
 * M : VCO frequency scaler
 * P : Input frequency scaler
 */
static inline unsigned long calc_clk(unsigned long v)
{
	unsigned long m, p, s;
	m = GET_MDIV(v); p = GET_PDIV(v); s = GET_SDIV(v);
	return ( ((m + 8) * CLOCK_TICK_RATE) / ((p + 2) * (1 << s)) );
}

unsigned long mmsp2_get_fclk(void)
{
	//return 199065600;
	return calc_clk(FPLLVSET);
}
EXPORT_SYMBOL(mmsp2_get_fclk);

unsigned long mmsp2_get_uclk(void)
{
	//return 95000000;
	return calc_clk(UPLLVSET);
}
EXPORT_SYMBOL(mmsp2_get_uclk);

unsigned long mmsp2_get_aclk(void)
{
	//return 147456000;
	return calc_clk(APLLVSET);
}
EXPORT_SYMBOL(mmsp2_get_aclk);

unsigned long mmsp2_get_pclk(void)
{
	unsigned long c, d, b, p;
	c = calc_clk(FPLLVSET);
	d = c / (GET_DDIV(SYSCSET) + 1); // DCLK
	b = c / 2; // BCLK
	p = b / 2; // PCLK
	return p;	
}
EXPORT_SYMBOL(mmsp2_get_pclk);

static void __init mmsp2_show_clk(void)
{
	unsigned long v, c, m, p, s;

	v = FPLLVSET;
	m = GET_MDIV(v); p = GET_PDIV(v); s = GET_SDIV(v);
	c = ( ((m + 8) * CLOCK_TICK_RATE) / ((p + 2) * (1 << s)) );
	printk(KERN_INFO "MP2520F FCLK: %9lu Hz, M = 0x%lx P = %lu S = %lu\n", c, m, p, s);

	v = UPLLVSET;
	m = GET_MDIV(v); p = GET_PDIV(v); s = GET_SDIV(v);
	c = ( ((m + 8) * CLOCK_TICK_RATE) / ((p + 2) * (1 << s)) );
	printk(KERN_INFO "MP2520F UCLK: %9lu Hz, M = 0x%lx P = %lu S = %lu\n", c, m, p, s);

	v = APLLVSET;
	m = GET_MDIV(v); p = GET_PDIV(v); s = GET_SDIV(v);
	c = ( ((m + 8) * CLOCK_TICK_RATE) / ((p + 2) * (1 << s)) );
	printk(KERN_INFO "MP2520F ACLK: %9lu Hz, M = 0x%lx P = %lu S = %lu\n", c, m, p, s);

	c = mmsp2_get_pclk();
	printk(KERN_INFO "MP2520F PCLK: %9lu Hz\n", c);
}

/*
 * Note that 0xfffe0000-0xffffffff is reserved for the vector table and
 * cache flush area.
 */
static struct map_desc standard_io_desc[] __initdata = {
	/* virtual	physical	length	 domain	    r  w  c  b */
	{ VIO_BASE,	PIO_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ VFIO_BASE, PFIO_BASE, 0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ NF_VIO_BASE,	NF_PIO_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ IDE_VIO_BASE,	IDE_PIO_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ CF0_VIO_BASE,	CF0_PIO_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ CF0_VATTR_BASE,CF0_PATTR_BASE,0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ CF0_VMEM_BASE,CF0_PMEM_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ CF1_VIO_BASE,	CF1_PIO_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ CF1_VATTR_BASE,CF1_PATTR_BASE,0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ CF1_VMEM_BASE,CF1_PMEM_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	LAST_DESC
};

void __init mmsp2_map_io(void)
{
	iotable_init(standard_io_desc);
	mmsp2_show_clk();
}
