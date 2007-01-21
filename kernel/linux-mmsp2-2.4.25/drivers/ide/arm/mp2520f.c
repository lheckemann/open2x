/*
 * drivers/ide/arm/mmsp2.c
 *
 * IDE interface of MagicEyes MMSP2 CPU
 *
 * Copyright (C) 2004 MIZI Research, Inc.
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
 *
 * Author: SeonKon Choi <bushi@mizi.com>
 * $Id$
 *
 * 
 * physical info. :
 *
 * 	I/O base  : 0x3c000000 or 0xbc000000
 * 	IDE reg.s : 0x0 ~ 0x7,  0x8 ~ 0xf
 * 	IDE IRQ   : GPIO M[0]
 *
 * logical info. :
 *  
 *  IDE 
 *      ATA-2 final draft : http://file.mizi.com/DOC/bushi/Spec/0948dr3.pdf
 *  ATAPI CD-ROM
 *      SFF-8020i final draft : http://file.mizi.com/DOC/bushi/Spec/8020r26.pdf
 * 

   2004-04-13 SeonKon Choi <bushi@mizi.com>
   - initial release : PIO only
 
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/ide.h>

#include <asm/irq.h>
#include <asm/hardware.h>
#include <asm/io.h>

//#define DEBUG 1

#define MAX_MMIO_NUM_HWIFS 1  /* ide0 */

#define MM_IDE_REG(x)		((x) - 0x1f0) /* 0x1f0 ~ 0x1f7 -> 0x00 ~ 0x07 */
#define MM_IDE_ALT_REG(x)	((x) - 0x3f0 + 0x8) /* 0x3f0 ~ 0x3f7 -> 0x08 ~ 0x0f */

static hw_regs_t hw[MAX_MMIO_NUM_HWIFS];
static ide_hwif_t *hwif[MAX_MMIO_NUM_HWIFS] = {NULL, };

static int idereg_offsets[MAX_MMIO_NUM_HWIFS][IDE_NR_PORTS] __initdata = {
	[ IDE_DATA_OFFSET    ] = MM_IDE_REG(HD_DATA), 
	[ IDE_ERROR_OFFSET   ] = MM_IDE_REG(HD_ERROR), 
	[ IDE_NSECTOR_OFFSET ] = MM_IDE_REG(HD_NSECTOR), 
	[ IDE_SECTOR_OFFSET  ] = MM_IDE_REG(HD_SECTOR),
	[ IDE_LCYL_OFFSET    ] = MM_IDE_REG(HD_LCYL), 
	[ IDE_HCYL_OFFSET    ] = MM_IDE_REG(HD_HCYL), 
	[ IDE_SELECT_OFFSET  ] = MM_IDE_REG(HD_CURRENT), 
	[ IDE_STATUS_OFFSET  ] = MM_IDE_REG(HD_STATUS),
	[ IDE_CONTROL_OFFSET ] = MM_IDE_ALT_REG(HD_ALTSTATUS),
	[ IDE_IRQ_OFFSET     ] = 0
};

#if DEBUG
#define DPRINTK(x...) printk(x)
void inline __dump_ide_offsets(hw_regs_t * ide)
{
	int i;

	printk("-- ide --\n");
	for (i = 0; i < IDE_NR_PORTS; i++) {
		printk("0x%08lx\n", ide->io_ports[i]);
	}
	printk("IRQ : %d\n", ide->irq);
}
#else
#define DPRINTK(x...) (void)(0)
#define __dump_ide_offsets(x) (void)(0)
#endif /* DEBUG */

int __init ide_mmio_init(void)
{
	int i;

#ifdef CONFIG_MACH_MMSP2_MDK
	set_gpio_ctrl(GPIO_IDE_nSWRST, GPIOMD_OUT, GPIOPU_NOSET);
	write_gpio_bit(GPIO_IDE_nSWRST, 0);
#endif

	set_gpio_ctrl(GPIO_J10, GPIOMD_ALT1, GPIOPU_NOSET); // nIOR
	set_gpio_ctrl(GPIO_J9, GPIOMD_ALT1, GPIOPU_NOSET); // nIOW
	set_gpio_ctrl(GPIO_J6, GPIOMD_ALT1, GPIOPU_NOSET); // nIOIS16

	set_gpio_ctrl(GPIO_I5, GPIOMD_ALT1, GPIOPU_NOSET); // nICS1
	set_gpio_ctrl(GPIO_I4, GPIOMD_ALT1, GPIOPU_NOSET); // nICS0

	set_gpio_ctrl(GPIO_G0, GPIOMD_ALT1, GPIOPU_NOSET); // ZA0
	set_gpio_ctrl(GPIO_G1, GPIOMD_ALT1, GPIOPU_NOSET); // ZA1
	set_gpio_ctrl(GPIO_G2, GPIOMD_ALT1, GPIOPU_NOSET); // ZA2

	set_gpio_ctrl(GPIO_E0, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[0]
	set_gpio_ctrl(GPIO_E1, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[1]
	set_gpio_ctrl(GPIO_E2, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[2]
	set_gpio_ctrl(GPIO_E3, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[3]
	set_gpio_ctrl(GPIO_E4, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[4]
	set_gpio_ctrl(GPIO_E5, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[5]
	set_gpio_ctrl(GPIO_E6, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[6]
	set_gpio_ctrl(GPIO_E7, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[7]
	set_gpio_ctrl(GPIO_E8, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[8]
	set_gpio_ctrl(GPIO_E9, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[9]
	set_gpio_ctrl(GPIO_E10, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[10]
	set_gpio_ctrl(GPIO_E11, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[11]
	set_gpio_ctrl(GPIO_E12, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[12]
	set_gpio_ctrl(GPIO_E13, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[13]
	set_gpio_ctrl(GPIO_E14, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[14]
	set_gpio_ctrl(GPIO_E15, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[15]

#ifdef CONFIG_MACH_MMSP2_MDK
	write_gpio_bit(GPIO_IDE_nSWRST, 1);
#endif

	for (i=0; i < MAX_MMIO_NUM_HWIFS; i++) {
		hw_regs_t *hwreg = (hw_regs_t *)&hw[i];

		memset(hwreg, 0, sizeof(hw_regs_t));

		set_external_irq(IRQ_HARD, EINT_HIGH_LEVEL, GPIOPU_EN); /* yes */
//		set_external_irq(IRQ_HARD, EINT_RISING_EDGE, GPIOPU_EN);

		DPRINTK("ide%d uses 0x%08x, IRQ %d\n", i, IDE_PIO_BASE, IRQ_HARD);

		/* do not use IDE_IRQ_REG */
		idereg_offsets[i][IDE_IRQ_OFFSET] -= IDE_VIO_BASE;

		ide_setup_ports(
				hwreg, 
				(ide_ioreg_t)IDE_VIO_BASE,
				(int*)&idereg_offsets[i], 
				0,
				0,
				NULL,
				IRQ_HARD);
		ide_register_hw(hwreg, &hwif[i]);

		__dump_ide_offsets(hwreg);
	}

	return 0;
}

void __exit ide_mmio_exit(void)
{
	int i, unit;

	for (i = 0; i < MAX_MMIO_NUM_HWIFS; i++) {
		ide_hwif_t *my_hwif = hwif[i];
		if (my_hwif != NULL) {
			ide_unregister(i);
			my_hwif->present = 0;
			for (unit = 0; unit < MAX_DRIVES; ++unit) {
					ide_drive_t *drive = &my_hwif->drives[unit];
					drive->present = 0;
			}
		}
	}
}

module_init(ide_mmio_init);
module_exit(ide_mmio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SeonKon Choi <bushi@mizi.com>, MIZI Research Inc.");
