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

#include <stdlib.h>
#include <stdio.h>

#include "jtag.h"
#include "memory.h"
#include "debug.h"
#include "regs.h"

int memory_poke(unsigned long address, unsigned long data, unsigned long length)
{ 
	unsigned long temp;

	if (arm_regs.status == ARM_RUNNING) {
		printf("Error: Target not halted.\n");
		return(1);
	}

	if (arm_regs.status == ARM_HALTED) 
		regs_read();

	printf("Poke 0x%8.8lX, 0x%8.8lX", address, data);
	if (length != 1) printf(", 0x%8.8lX", length);
	printf("\n");

	/* select Scan Chain 1 */
	Data[0] = JTAG_SCAN_N;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = 0x00000001;
	jtag_dreg(Data, 5);

	/* select Intest */
	Data[0] = JTAG_INTEST;
	jtag_ireg(Data, JTAG_IRLENGTH);

	/* send 32bit sequence.... */

	/* LDMIA r0, [r0, r1]	- put address and data */
	Data[0] = 0xe8900003;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP */
	Data[0] = 0xe1a00000;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP */
	Data[0] = 0xe1a00000;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP 			- scan in address */
	Data[0] = 0xe1a00000;
	Data[1] = address;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP 			- scan in data */
	Data[0] = 0xe1a00000;
	Data[1] = data;
	debug_exec(Data, DEBUG_SPEED);

	for (temp = 0; temp < length; temp ++) {
		/* STR r1, [r0], #4	- store r1 in location of r0 */
		Data[0] = 0xe4801004;
		debug_exec(Data, DEBUG_SPEED);

		/* Run at system speed */
		Data[0] = 0xe1a00000;
		debug_exec(Data, SYSTEM_SPEED);

		Data[0] = JTAG_RESTART;
		jtag_ireg(Data, JTAG_IRLENGTH);

		Data[0] = JTAG_INTEST;
		jtag_ireg(Data, JTAG_IRLENGTH);
	}

	return(0);
}

int memory_peek(unsigned long address, unsigned long * data)
{
	if (arm_regs.status == ARM_RUNNING) {
		printf("Error: Target not halted.\n");
		return(1);
	}

	if (arm_regs.status == ARM_HALTED) 
		regs_read();

	/* select Scan Chain 1 */
	Data[0] = JTAG_SCAN_N;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = 0x00000001;
	jtag_dreg(Data, 5);

	/* select Intest */
	Data[0] = JTAG_INTEST;
	jtag_ireg(Data, JTAG_IRLENGTH);

	/* send 32bit sequence.... */

	/* LDR r0, [r0]		- put address in r0 */
	Data[0] = 0xe5900000;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP */
	Data[0] = 0xe1a00000;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP */
	Data[0] = 0xe1a00000;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP 			- scan in address */
	Data[0] = 0xe1a00000;
	Data[1] = address;
	debug_exec(Data, DEBUG_SPEED);

	/* LDR r1, [r0], #4	- load r1 from location of r0 */
	Data[0] = 0xe4901004;
	debug_exec(Data, DEBUG_SPEED);

	/* Run at system speed */
	Data[0] = 0xe1a00000;
	debug_exec(Data, SYSTEM_SPEED);

	Data[0] = JTAG_RESTART;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = JTAG_INTEST;
	jtag_ireg(Data, JTAG_IRLENGTH);

	/* STR r1, [r0], #4	- output the data value */
	Data[0] = 0xe4801004;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP */
	Data[0] = 0xe1a00000;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP */
	Data[0] = 0xe1a00000;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP */
	Data[0] = 0xe1a00000;
	debug_exec(Data, DEBUG_SPEED);
	*data = Data[1];

	printf("Peek 0x%8.8lX is 0x%8.8lX\n", address, *data);
	return(0);
}

int memory_write(unsigned long address, unsigned long * data, unsigned long length)
{ 
	unsigned long temp;

	if (arm_regs.status == ARM_RUNNING) {
		printf("Error: Target not halted.\n");
		return(1);
	}

	if (arm_regs.status == ARM_HALTED) 
		regs_read();

	printf("Write Block 0x%8.8lX, length 0x%8.8lX\n", address, length);

	/* select Scan Chain 1 */
	Data[0] = JTAG_SCAN_N;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = 0x00000001;
	jtag_dreg(Data, 5);

	/* select Intest */
	Data[0] = JTAG_INTEST;
	jtag_ireg(Data, JTAG_IRLENGTH);

	/* send 32bit sequence.... */

	/* LDR r0, [r0]		- put address in r0 */
	Data[0] = 0xe5900000;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP */
	Data[0] = 0xe1a00000;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP */
	Data[0] = 0xe1a00000;
	debug_exec(Data, DEBUG_SPEED);

	/* NOP 			- scan in address */
	Data[0] = 0xe1a00000;
	Data[1] = address;
	debug_exec(Data, DEBUG_SPEED);

	for (temp = 0; temp < length; temp = temp + 8) {
		/* LDMIA r0, [r1-r8]	- put data */
		Data[0] = 0xe89001fe;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP 			- scan in data 1 */
		Data[0] = 0xe1a00000;
		Data[1] = *(data++);
		debug_exec(Data, DEBUG_SPEED);

		/* NOP 			- scan in data 2 */
		Data[0] = 0xe1a00000;
		Data[1] = *(data++);
		debug_exec(Data, DEBUG_SPEED);

		/* NOP 			- scan in data 3 */
		Data[0] = 0xe1a00000;
		Data[1] = *(data++);
		debug_exec(Data, DEBUG_SPEED);

		/* NOP 			- scan in data 4 */
		Data[0] = 0xe1a00000;
		Data[1] = *(data++);
		debug_exec(Data, DEBUG_SPEED);

		/* NOP 			- scan in data 5 */
		Data[0] = 0xe1a00000;
		Data[1] = *(data++);
		debug_exec(Data, DEBUG_SPEED);

		/* NOP 			- scan in data 6 */
		Data[0] = 0xe1a00000;
		Data[1] = *(data++);
		debug_exec(Data, DEBUG_SPEED);

		/* NOP 			- scan in data 7 */
		Data[0] = 0xe1a00000;
		Data[1] = *(data++);
		debug_exec(Data, DEBUG_SPEED);

		/* NOP 			- scan in data 8 */
		Data[0] = 0xe1a00000;
		Data[1] = *(data++);
		debug_exec(Data, DEBUG_SPEED);

		/* STMIA r0!, {r1-r8}	- store r1 in location of r0 */
		Data[0] = 0xe8a001fe;
		debug_exec(Data, DEBUG_SPEED);

		/* Run at system speed */
		Data[0] = 0xe1a00000;
		debug_exec(Data, SYSTEM_SPEED);

		Data[0] = JTAG_RESTART;
		jtag_ireg(Data, JTAG_IRLENGTH);

		Data[0] = JTAG_INTEST;
		jtag_ireg(Data, JTAG_IRLENGTH);
	}

	return(0);
}

int memory_read(unsigned long  address, unsigned long * data, unsigned long length)
{
	int temp;

	if (arm_regs.status == ARM_RUNNING) {
		printf("Error: Target not halted.\n");
		return(1);
	}

	if (arm_regs.status == ARM_HALTED) 
		regs_read();

	/* select Scan Chain 1 */
	Data[0] = JTAG_SCAN_N;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = 0x00000001;
	jtag_dreg(Data, 5);

	/* select Intest */
	Data[0] = JTAG_INTEST;
	jtag_ireg(Data, JTAG_IRLENGTH);

	for (temp = 0; temp < length; temp++) {
		/* LDR r0, [r0]		- put address in r0 */
		Data[0] = 0xe5900000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP 			- scan in address */
		Data[0] = 0xe1a00000;
		Data[1] = address;
		debug_exec(Data, DEBUG_SPEED);

		/* LDR r1, [r0], #4	- load r1 from location of r0 */
		Data[0] = 0xe4901004;
		debug_exec(Data, DEBUG_SPEED);

		/* Run at system speed */
		Data[0] = 0xe1a00000;
		debug_exec(Data, SYSTEM_SPEED);

		Data[0] = JTAG_RESTART;
		jtag_ireg(Data, JTAG_IRLENGTH);

		Data[0] = JTAG_INTEST;
		jtag_ireg(Data, JTAG_IRLENGTH);

		/* STR r1, [r0], #4	- output the data value */
		Data[0] = 0xe4801004;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);
		*data = Data[1];

		printf("Peek 0x%8.8lX is 0x%8.8lX\n", address, *data);

		/* increase the pointers */
		address++;
		data++;
	}

	return(0);
}
