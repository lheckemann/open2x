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

#include <stdio.h>

#include "jtag.h"
#include "debug.h"
#include "regs.h"

regs_type arm_regs;

int regs_read(void)
{
	int temp;
	/* read in the contents of all the arm_regs. except PC and R0 */
	/* select Scan Chain 1 */
	Data[0] = JTAG_SCAN_N;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = 0x00000001;
	jtag_dreg(Data, 5);

	/* select Intest */
	Data[0] = JTAG_INTEST;
	jtag_ireg(Data, JTAG_IRLENGTH);

	/* STMIA r0, {r1-r15} */
	Data[0] = 0xe880fffe;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP */
	Data[0] = 0xe1a00000;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP */
	Data[0] = 0xe1a00000;
	debug_exec(Data, DEBUG_SPEED);

	for (temp = 1; temp < 16; temp ++) {
		Data[0] = 0xe1a00000;
		Data[1] = 0;
		debug_exec(Data, DEBUG_SPEED);
		arm_regs.r[temp] = Data[1];
	}

	printf("registers read\n");
	arm_regs.status = ARM_HALTED_WITH_REGS;
	return(0);
}

int regs_write(void)
{
	int temp;

	/* select Scan Chain 1 */
	Data[0] = JTAG_SCAN_N;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = 0x00000001;
	jtag_dreg(Data, 5);

	/* select Intest */
	Data[0] = JTAG_INTEST;
	jtag_ireg(Data, JTAG_IRLENGTH);

	/* LDMIA r0, {r1-r15} */
	Data[0] = 0xe890fffe;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP */
	Data[0] = 0xe1a00000;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP */
	Data[0] = 0xe1a00000;
	debug_exec(Data, DEBUG_SPEED);

	for (temp = 1; temp < 16; temp ++) {
		Data[0] = 0xe1a00000;
		Data[1] = arm_regs.r[temp];
		debug_exec(Data, DEBUG_SPEED);
	}

	printf("registers written\n");
	arm_regs.status = ARM_HALTED_WITH_REGS_WRITTEN;

        return(0);
}

int regs_print(void)
{
	/* print out the registers */
	int temp;
	
	if (arm_regs.status == ARM_RUNNING) {
		printf("Error: Target not halted.\n");
		return(1);
	}

	if (arm_regs.status == ARM_HALTED) 
		regs_read();

	printf("Registers:\n");
	for (temp = 0; temp < 8; temp++)
		printf("  R%2.2d - 0x%8.8lx   R%2.2d - 0x%8.8lx\n",
				temp, arm_regs.r[temp],
				temp+8, arm_regs.r[temp+8]);

	printf("\n  PC  - 0x%8.8lx\n", arm_regs.pc);

	return(0);
}

int regs_set(int reg, unsigned long value)
{
	if (arm_regs.status == ARM_RUNNING) {
		printf("System not halted..\n");
		return(1);
	}

	if (arm_regs.status == ARM_HALTED) {
		/* need to read the registers */
		regs_read();
	}

	if (reg < 0 || reg > 15) {
		printf("register must be between 0 and 15\n");
		return(1);
	}

	arm_regs.r[reg] = value;
	arm_regs.status = ARM_HALTED_WITH_REGS;
	return (0);
}
