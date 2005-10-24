/*
 *  linux/include/asm-arm/arch-mmsp2/irq.h
 *
 *  Copyright (c) 2004 MIZI Research, Inc. All rights reserved.
 *
 *  2004-04-02  Janghoon Lyu
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

extern unsigned int fixup_irq(unsigned int i);
extern void do_IRQ(int irq, struct pt_regs *regs);
