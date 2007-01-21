/*
 * include/asm-arm/arch-mmsp2/ide.h
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 */

#ifndef _ASM_ARCH_IDE_H_

#undef MAX_HWIFS
#define MAX_HWIFS 1 // 2 ?

/* HDIO_SCAN_HWIF ioctl, linux_command_line */
static __inline__ void ide_init_hwif_ports(hw_regs_t *hw, int data_port, int ctrl_port, int *irq)
{
	/* FIXME */
}

static __inline__ void ide_init_default_hwifs(void)
{
	/* do nothing */
}

#endif /* _ASM_ARCH_IDE_H_ */
