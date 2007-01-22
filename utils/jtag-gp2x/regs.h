/* Open2x Project - GP2X JTAG Software
 * Copyright (C) 2006 The Open2x project
 *
 * Largely based on Jtag-Arm9 (http://jtag-arm9.sourceforge.net/)
 * Copyright (C) 2001 Simon Wood
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* status */
#define	ARM_RUNNING	0
#define ARM_HALTED	1
#define ARM_HALTED_WITH_REGS	2
#define ARM_HALTED_WITH_REGS_WRITTEN	3

/* mode */
#define ARM_THUMB	0
#define ARM_32BIT	1

struct regs_struct {
	int status;
	int mode;
	unsigned long r[16];
	unsigned long pc;
};

typedef struct regs_struct regs_type;

extern regs_type arm_regs;

int regs_print(void);
int regs_set(int reg, unsigned long value);
int regs_read(void);
int regs_write(void);
