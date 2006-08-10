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
#include <stdlib.h>
#include <unistd.h>

#include "jtag.h"
#include "debug.h"
#include "regs.h"

extern unsigned long Data[10];

int debug_exec(unsigned long * data, int type)
{
	/* allows for an automatic swapping around command/data */

	unsigned long my_data[3];
	int temp;

	if (type == DEBUG_SPEED_SHORT) {
		printf("short access not fully tested\n");
		/* do a short (only 33 bit) access */
		my_data[0] = 0;
		my_data[1] = 0;

		/* need to reverse the bit order for the command */
		for (temp = 1; temp < 32; temp ++) {
			if (data[0] & (1 << temp)) {
				/* bit set */
				my_data[0] = my_data[0] | (1 << (33 - temp));
			}
		}
	
		if (data[0] & 1 ) {
			/* bit set */
			my_data[2] = 1;
		}

		jtag_dreg(my_data, 33);
	} else {
		/* do a long (67 bit) access */
		my_data[0] = data[1];
		my_data[1] = 0; 
		my_data[2] = 0;
		
		if (type == SYSTEM_SPEED)
			my_data[1] = 1 << 2;

		/* need to reverse the bit order for the command */
		for (temp = 3; temp < 32; temp ++) {
			if (data[0] & (1 << temp)) {
				/* bit set */
				my_data[1] = my_data[1] | (1 << (34 - temp));
			}
		}
	
		for (temp = 0; temp < 3; temp ++) {
			if (data[0] & (1 << temp)) {
				/* bit set */
				my_data[2] = my_data[2] | (1 << (2 - temp));
			}
		}
	
		jtag_dreg(my_data, 67);
	}

	/* return the data value */
	data[1] = my_data[0];

	return(1);
}

static int debug_halted(void)
{
	printf("System HALTED in ");
	
	/* disable interrupts */
	Data[0]=0x00000004;
	Data[1]=0x20;
	jtag_dreg(Data, 38);

	/* Check current DEBUG state */
	Data[0] = 0x000000000;
	Data[1] = 0x01;
	jtag_dreg(Data, 38);

	if ((Data[0] & 0x10) == 0x10) {
		printf("Thumb State\n");
		arm_regs.mode = ARM_THUMB;
	} else {
		printf("32bit State\n");
		arm_regs.mode = ARM_32BIT;
	}

	/* select Scan Chain 1 */
	Data[0] = JTAG_SCAN_N;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = 0x00000001;
	jtag_dreg(Data, 5);

	/* select Intest and write command sequence to get PC */
	Data[0] = JTAG_INTEST;
	jtag_ireg(Data, JTAG_IRLENGTH);

	if (arm_regs.mode == ARM_THUMB) {
		/* send Thumb sequence */
		/* STR r0, [r0]	- Save R0 before use */
		/* MOV r0, PC 	- Copy PC to R0 */
		/* STR r0, [r0] - Save PC into R0 */
		/* BX PC	- jump into ARM state */
		printf("Thumb sequence not yet coded....\n");
	} else {
		/* send 32bit sequence */
		/* STR r0, [r0]	- Save R0 before use */
		Data[0] = 0xe5800000;
		debug_exec(Data, DEBUG_SPEED);

		/* MOV r0, PC 	- Copy PC to R0 */
		Data[0] = 0xe1a0000f;
		debug_exec(Data, DEBUG_SPEED);

		/* STR r0, [r0] - Save PC into R0 */
		Data[0] = 0xe5800000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP		- Read out R0 */
		Data[0] = 0xe1a00000;
		Data[1] = 0x00;
		debug_exec(Data, DEBUG_SPEED);
		arm_regs.r[0] = Data[1];
		printf("R0 is 0x%8.8lX\n", arm_regs.r[0]);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP		- Read out PC */
		Data[0] = 0xe1a00000;
		Data[1] = 0x00;
		debug_exec(Data, DEBUG_SPEED);

		/* Correct for delay */
		arm_regs.pc = Data[1] - 0x18;
		printf("PC is 0x%8.8lX\n", arm_regs.pc);

		Data[0] = JTAG_RESTART;
		jtag_ireg(Data, JTAG_IRLENGTH);
	}

	arm_regs.status = ARM_HALTED;

	return(0);
}

/**** Public functions ****/

int debug_poll(int argc, char **argv)
{
#if 0
	if (arm_regs.status == ARM_RUNNING) {
		printf("Already detected, don't waste my time\n");
		return(1);
	}
#endif
	printf("Polling debug state...");

	/* select Scan Chain 2 */
	Data[0] = JTAG_SCAN_N;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = 0x00000002;
	jtag_dreg(Data, 5);

	/* select Intest */
	Data[0] = JTAG_INTEST;
	jtag_ireg(Data, JTAG_IRLENGTH);

	/* Check current DEBUG state */
	Data[0] = 0x000000000;
	Data[1] = 0x01;
	jtag_dreg(Data, 38);

	if ((Data[0] & 0x09) == 0x09) {
		return(debug_halted());
	}

	Data[0] = JTAG_RESTART;
	jtag_ireg(Data, JTAG_IRLENGTH);

	printf("System running\n");
	return(0);
}

int debug_halt(int argc, char **argv)
{
	int temp;
#if 0
	if (arm_regs.status != ARM_RUNNING) {
		printf("Already halted, don't waste my time\n");
		return(1);
	}

	if (debug_poll()) {
		/* system is halted already, for what ever reason */
		return(1);
	}
#endif

	printf("Requesting HALT..\n");

	/* select Scan Chain 2 */
	Data[0] = JTAG_SCAN_N;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = 0x00000002;
	jtag_dreg(Data, 5);

	/* select Intest */
	Data[0] = JTAG_INTEST;
	jtag_ireg(Data, JTAG_IRLENGTH);

	/* set debug request bit */
	Data[0] = 0x00000002;
	Data[1] = 0x20;
	jtag_dreg(Data, 38);

	Data[0] = JTAG_RESTART;
	jtag_ireg(Data, JTAG_IRLENGTH);

	temp = 0;

	while (temp < 10) {
		/* select Intest */
		Data[0] = JTAG_INTEST;
		jtag_ireg(Data, JTAG_IRLENGTH);

		/* read debug status */
		Data[0] = 0x000000000;
		Data[1] = 0x01;
		jtag_dreg(Data, 38);
		
		if ((Data[0] & 0x09) != 0x09) {
			/* success, clear request */
			Data[0] = 0x00000000;
			Data[1] = 0x20;
			jtag_dreg(Data, 38);
	
			return(debug_halted());
		}

		Data[0] = JTAG_RESTART;
		jtag_ireg(Data, JTAG_IRLENGTH);

		printf(".");
		usleep(100);
	}

	/* Failed, cancel request flag */
	Data[0] = 0x00000000;
	Data[1] = 0x20;
	jtag_dreg(Data, 38);
	
	Data[0] = JTAG_RESTART;
	jtag_ireg(Data, JTAG_IRLENGTH);

	printf("HALT Failed\n");
	return(1);
}

int debug_restart(int argc, char **argv)
{
	if (arm_regs.status == ARM_RUNNING) {
		printf("System not halted, don't waste my time\n");
		return(1);
	}

	if (arm_regs.status == ARM_HALTED_WITH_REGS)
		regs_write();

	printf("Restarting at 0x%8.8lX\n", arm_regs.pc);

	/* select Scan Chain 2 */
	Data[0] = JTAG_SCAN_N;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = 0x00000002;
	jtag_dreg(Data, 5);

	/* select Intest */
	Data[0] = JTAG_INTEST;
	jtag_ireg(Data, JTAG_IRLENGTH);

	/* enable interrupts */
	Data[0]=0x00000000;
	Data[1]=0x20;
	jtag_dreg(Data, 38);

	/* select Scan Chain 1 */
	Data[0] = JTAG_SCAN_N;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = 0x00000001;
	jtag_dreg(Data, 5);

	/* select Intest and write command sequence to set PC */
	Data[0] = JTAG_INTEST;
	jtag_ireg(Data, JTAG_IRLENGTH);

	if (arm_regs.mode == ARM_THUMB) {
		printf("No thumb sequence yet...\n");
		/* send Thumb sequence */
		/* ORR r0, r0, #1	- add 1 to PC to force thumb */
		/* BX r1		- enter thumb state */
		/* LDR r0, [pc]		- restore R0 */
		/* <r0> nop		- scan in calue for r0 */
		/* B <pc> - 8		- */
	} else {
		/* send 32bit sequence */

		/* LDR r0, [r0]		- restore PC */
		Data[0] = 0xe5900000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP 			- scan in value for PC */
		Data[0] = 0xe1a00000;
		Data[1] = arm_regs.pc;
		debug_exec(Data, DEBUG_SPEED);

		/* MOV PC, r0		- put r0 into PC */
		Data[0] = 0xe1a0f000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);

		/* LDR r0, [r0]		- restore R0 */
		Data[0] = 0xe5900000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP */
		Data[0] = 0xe1a00000;
		debug_exec(Data, DEBUG_SPEED);

		/* NOP			- scan in value for r0 */
		Data[0] = 0xe1a00000;
		Data[1] = arm_regs.r[0];
		debug_exec(Data, DEBUG_SPEED);

		/* SUB PC, PC, #0x14	- jump back PC */
		Data[0] = 0xe24ff014;
		debug_exec(Data, DEBUG_SPEED);

		/* Return to system speed */
		Data[0] = 0xe1a00000;
		debug_exec(Data, SYSTEM_SPEED);

		Data[0] = JTAG_RESTART;
		jtag_ireg(&Data[0], JTAG_IRLENGTH);
	}

	printf("Done\n");
	arm_regs.status = ARM_RUNNING;

	return(0);
}
