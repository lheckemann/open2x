/*
 * linux/drivers/ide/rs-ide.c
 *
 * Copyright (c) 2002 Ben Dooks
 * Copyright (c) 2002 Simtec Electronics
 *
 * Simple RiscStation IDE support
*/

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/errno.h>
#include <linux/ide.h>
#include <linux/init.h>

#include <asm/hardware/iomd.h>
#include <asm/mach-types.h>
#include <asm/io.h>

#ifndef CONFIG_ARCH_RISCSTATION
#error "compiling this code for non-riscstation hardware is dangerous!"
#endif

#define DRV_PREFIX "ide-rs"

#define IRQ_PRI  (40+3)
#define IRQ_SEC  (40+4)

#define PORT_BASE ((0x2b800 - 0x10000) >> 2)
#define SEC_OFF (0x400 >> 2)

int __init rside_reg(unsigned long base, unsigned int irq);

int __init rside_init(void)
{
    int iotcr;

    if (!machine_is_riscstation()) {
	printk(DRV_PREFIX ": hardware is not a RiscStation!\n");
	return 0;
    }

    /* select correct area cycle time */

    iotcr = inb(IOMD_IOTCR);
    outb((iotcr & ~3) | 1, IOMD_IOTCR);

    /* register h/w */

    rside_reg(PORT_BASE, IRQ_PRI);
    rside_reg(PORT_BASE + SEC_OFF, IRQ_SEC);

    return 0;
}


int __init rside_reg(unsigned long port, unsigned int irq)
{
    unsigned long addr, i;
    hw_regs_t hw;

    hw.irq = irq;

    addr = port;

    for (i = IDE_DATA_OFFSET; i  <= IDE_STATUS_OFFSET; i++) {
	hw.io_ports[i] = (ide_ioreg_t)addr;
	addr += 0x40 >> 2;
    }

    hw.io_ports[IDE_CONTROL_OFFSET] = port + ((0xb80 - 0x800) >> 2);

    printk(DRV_PREFIX ": registering channel at %08lx, %08lx, irq %d\n",
	   port, hw.io_ports[IDE_CONTROL_OFFSET], irq);

    return ide_register_hw(&hw, NULL);
}
