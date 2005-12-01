/*
 *  linux/arch/arm/mach-mmsp2/generic.h
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

extern void __init mmsp2_map_io(void);
extern void __init mmsp2_init_irq(void);
extern void __init mmsp2_register_uart(int idx, int port);
