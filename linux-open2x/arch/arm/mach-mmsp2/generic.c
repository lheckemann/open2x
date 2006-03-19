/*
 * linux/arch/arm/mach-mmsp2/generic.c
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
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
#include <asm/arch/mmsp20.h>
#include <asm/arch/proto_pwrman.h>

#include "generic.h"

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

static inline unsigned long calc_clk(unsigned short v)
{
	unsigned long m, p, s;
	m = GET_MDIV(v); p = GET_PDIV(v); s = GET_SDIV(v);

	return ( ((m + 8) * CLOCK_TICK_RATE) / ((p + 2) * (1 << s)) );
}

unsigned long mmsp2_get_fclk(void)
{
	return calc_clk(FPLLVSET);	//return 199065600;
}
EXPORT_SYMBOL(mmsp2_get_fclk);

unsigned long mmsp2_get_uclk(void)
{
	return calc_clk(UPLLVSET);	//return 95000000;
}
EXPORT_SYMBOL(mmsp2_get_uclk);

unsigned long mmsp2_get_aclk(void)
{
	return calc_clk(APLLVSET);	//return 147456000;
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
	/* virtual			physical		length	 domain	    r  w  c  b */
	{ VIO_BASE,			PIO_BASE,		0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ VFIO_BASE,    	PFIO_BASE,      0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ NF_VIO_BASE,		NF_PIO_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ IDE_VIO_BASE,		IDE_PIO_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ CF0_VIO_BASE,		CF0_PIO_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ CF0_VATTR_BASE,	CF0_PATTR_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ CF0_VMEM_BASE,	CF0_PMEM_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ CF1_VIO_BASE,		CF1_PIO_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ CF1_VATTR_BASE,	CF1_PATTR_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ CF1_VMEM_BASE,	CF1_PMEM_BASE,	0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ PERI_VREG_BASE,	PERI_PREG_BASE, 0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	{ INTR_VREG_BASE,	INTR_PREG_BASE, 0x10000, DOMAIN_IO, 0, 1, 0, 0 },
	LAST_DESC
};

void __init mmsp2_map_io(void)
{
	iotable_init(standard_io_desc);
	mmsp2_show_clk();
}
