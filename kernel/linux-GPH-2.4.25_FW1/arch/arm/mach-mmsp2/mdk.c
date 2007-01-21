/*
 *  linux/arch/arm/mach-mmsp2/mdk.c
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/config.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/bitops.h>

#include <asm/types.h>
#include <asm/setup.h>
#include <asm/memory.h>
#include <asm/mach-types.h>
#include <asm/hardware.h>
#include <asm/irq.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/arch/irq.h>

#include "generic.h"

static struct map_desc mdk_io_desc[] __initdata = {
	/* virtual			physical		 length			  domain	 r  w  c  b */
	{MDK_ETH_VIO_BASE, MDK_ETH_PIO_BASE, 0x10000, 		  DOMAIN_IO, 0, 1, 0, 0 },
	{VA_ARM940_BASE,   PA_ARM940_BASE,   ARM940_MEM_SIZE, DOMAIN_IO, 0, 1, 0, 0 },
	{VA_BUFFER_BASE,   PA_BUFFER_BASE,   BUFFER_MEM_SIZE, DOMAIN_IO, 0, 1, 0, 0 },
	LAST_DESC
};

static void __init
fixup_mmsp2(struct machine_desc *desc, struct param_struct *params,
	     char **cmdline, struct meminfo *mi)
{
	mi->bank[0].start = PA_SDRAM_BASE;
	mi->bank[0].size =  MP2520F_MEM_SIZE;
	mi->bank[0].node =  0;
	mi->nr_banks = 1;
}

static void __init mdk_map_io(void)
{
	mmsp2_map_io();
	
	iotable_init(mdk_io_desc);

	mmsp2_register_uart(0, 0);
	mmsp2_register_uart(1, 1);
	mmsp2_register_uart(2, 2);

	MEMCFG &= ~(0xCL);
	MEMCFG |= 0xC; // 16bit

	set_gpio_ctrl(GPIO_I9, GPIOMD_ALT1, GPIOPU_EN); // nSCS[1]        
}

MACHINE_START(MMSP2_MDK, "MagicEye-MDK")
	BOOT_MEM(0x00000000, 0xc0000000, 0xf0000000)
	BOOT_PARAMS(0x0000100)
	FIXUP(fixup_mmsp2)
	MAPIO(mdk_map_io)
	INITIRQ(mmsp2_init_irq)
MACHINE_END
