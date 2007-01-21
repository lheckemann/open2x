/*
 *  linux/include/asm-i386/ide.h
 *
 *  Copyright (C) 1994-1996  Linus Torvalds & authors
 */

/*
 *  This file contains the i386 architecture specific IDE code.
 */

#ifndef __ASMi386_IDE_H
#define __ASMi386_IDE_H

#ifdef __KERNEL__

#include <linux/config.h>

#ifndef MAX_HWIFS
# ifdef CONFIG_BLK_DEV_IDEPCI
#define MAX_HWIFS	10
# else
#define MAX_HWIFS	6
# endif
#endif

#define ide_default_io_base(i)	((ide_ioreg_t)0)
#define ide_default_irq(b)	(0)

static __inline__ void ide_init_hwif_ports(hw_regs_t *hw, ide_ioreg_t data_port, ide_ioreg_t ctrl_port, int *irq)
{
	ide_ioreg_t reg = data_port;
	int i;

	memset(hw, 0, sizeof(*hw));
	for (i = IDE_DATA_OFFSET; i <= IDE_STATUS_OFFSET; i++) {
		hw->io_ports[i] = reg;
		reg += 1;
	}
	if (ctrl_port) {
		hw->io_ports[IDE_CONTROL_OFFSET] = ctrl_port;
	} else {
		hw->io_ports[IDE_CONTROL_OFFSET] = data_port + 0x206;
	}
	if (irq != NULL)
		*irq = 0;
	hw->io_ports[IDE_IRQ_OFFSET] = 0;
}

static __inline__ void ide_init_default_hwifs(void)
{
#ifndef CONFIG_BLK_DEV_IDEPCI
	hw_regs_t hw;

	ide_init_hwif_ports(&hw, 0x1f0, 0x3f6, NULL);
	hw.irq = 14;
	ide_register_hw(&hw, NULL);
	ide_init_hwif_ports(&hw, 0x170, 0x376, NULL);
	hw.irq = 15;
	ide_register_hw(&hw, NULL);
	ide_init_hwif_ports(&hw, 0x1e8, 0x3ee, NULL);
	hw.irq = 11;
	ide_register_hw(&hw, NULL);
	ide_init_hwif_ports(&hw, 0x168, 0x36e, NULL);
	hw.irq = 10;
	ide_register_hw(&hw, NULL);
	ide_init_hwif_ports(&hw, 0x1e0, 0x3e6, NULL);
	hw.irq = 8;
	ide_register_hw(&hw, NULL);
	ide_init_hwif_ports(&hw, 0x160, 0x366, NULL);
	hw.irq = 12;
	ide_register_hw(&hw, NULL);
#endif /* CONFIG_BLK_DEV_IDEPCI */
}

#include <asm-generic/ide_iops.h>

#endif /* __KERNEL__ */

#endif /* __ASMi386_IDE_H */
