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

/* 
 * This file contains all of the JTAG hardware calls.
 *
 */

#include <stdlib.h>
#include <stdio.h>

#ifndef DOS
# include <unistd.h>            /* for ioperm */
# include <sys/io.h>            /* and inb/outb */
#endif

#include "jtag.h"

int Port = 0x378;
unsigned long Data[10];

int jtag_restarted = 1;

#ifdef DOS
/* Macros for inb and outb */
static __inline unsigned char
inb (unsigned short int port)
{
  unsigned char _v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

static __inline void
outb (unsigned char value, unsigned short int port)
{
  __asm__ __volatile__ ("outb %b0,%w1": :"a" (value), "Nd" (port));
}
#endif

static int jtag_reg(unsigned long *data, unsigned int length)
{
	unsigned long * input_pointer;
	unsigned long * output_pointer;
	unsigned long input = 0, value;
	unsigned int count;
	unsigned long output = 0;

	input_pointer = data;
	output_pointer = data;

	/* select -> capture */
	outb(JTAG_PWR, Port);
	outb(JTAG_PWR | JTAG_CLK, Port);

	/* capture -> shift */
	outb(JTAG_PWR, Port);
	outb(JTAG_PWR | JTAG_CLK, Port);

	/* read/writing */
	for (count = 0; count < length; count ++) {
		if ((count & 0x1f) == 0) {
			/* fetch next word of data */
			input = *input_pointer;
			input_pointer ++;
		}
		
		value = JTAG_PWR;

		if (input & 0x01)
			value = value | JTAG_TDI;

		if (count == (length -1)) {
			/* shift ->  exit, as it's the last bit */
			value = value | JTAG_TMS;
		}

		outb(value, Port);
		outb(value | JTAG_CLK, Port);

		if ((inb(Port + 1) & JTAG_TDO) == 0)
			output = output | (1 << (count & 0x1f));

		input = input >> 1;
		
		if ((count & 0x1f) == 0x1f) {
			/* store recieved word of data */
			*output_pointer = output;
			output_pointer ++;
			output = 0;
		}
	}
	/* ensure that the output is captured if less than whole long word */
	*output_pointer = output;

	/* exit -> update */
	outb(JTAG_PWR | JTAG_TMS, Port);
	outb(JTAG_PWR | JTAG_TMS | JTAG_CLK, Port);

	if (jtag_restarted) {
		/* update -> Idle/Run */
		outb(JTAG_PWR, Port);
		outb(JTAG_PWR | JTAG_CLK, Port);
		outb(JTAG_PWR, Port);
	} else {
		/* update -> select DR */
		outb(JTAG_PWR | JTAG_TMS, Port);
		outb(JTAG_PWR | JTAG_TMS | JTAG_CLK, Port);
		outb(JTAG_PWR | JTAG_TMS, Port);
	}

	return(0);
}

int jtag_ireg(unsigned long *data, unsigned int length)
{
	if (jtag_restarted == 1) {
		/* this is only output if you have come from IDLE/RUN */
		jtag_restarted = 0;

		/* idle -> select DR */
		outb(JTAG_PWR | JTAG_TMS, Port);
		outb(JTAG_PWR | JTAG_TMS | JTAG_CLK, Port);
	}

	if (*data == JTAG_RESTART)
		jtag_restarted = 1;

	/* select DR -> select IR */
	outb(JTAG_PWR | JTAG_TMS, Port);
	outb(JTAG_PWR | JTAG_TMS | JTAG_CLK, Port);


	return(jtag_reg(data, length));
}

int jtag_dreg(unsigned long *data, unsigned int length)
{
	if (jtag_restarted == 1) {
		/* this is only output if you have come from IDLE/RUN */
		jtag_restarted = 0;

		/* idle -> select DR */
		outb(JTAG_PWR | JTAG_TMS, Port);
		outb(JTAG_PWR | JTAG_TMS | JTAG_CLK, Port);
	}
#if 1 
	/* we always go via Idle/Run for data register stuff */
	jtag_restarted = 1;
#endif
	return(jtag_reg(data, length));
}

int jtag_init(void)
{
	printf("Jtag - Using port 0x%x\n", Port);

	/* enable the use of the parallel port */
#ifndef DOS
	if (ioperm(Port, 3, 1)!=0) {
		printf ("ERROR: ioperm(0x%x) failed:\n", Port);
		return(-1);
	}
#endif
	return(0);
}

int jtag_reset(int argc, char **argv)
{
	int	loop_count;

	for(loop_count = 0; loop_count < 5; loop_count++) {
		/* goto Test Logic Reset and stay there */
		outb(JTAG_PWR | JTAG_TMS, Port);
		outb(JTAG_PWR | JTAG_TMS | JTAG_CLK, Port);
	}

	/* Test Logic Reset -> Idle */
	outb(JTAG_PWR, Port);
	outb(JTAG_PWR | JTAG_CLK, Port);
	outb(JTAG_PWR, Port);

	Data[0] = JTAG_RESTART;
	jtag_ireg(Data, JTAG_IRLENGTH);

	jtag_restarted = 1;
	printf("JTAG interface reset.\n");

	return(0);
}

int jtag_idcode(int argc, char **argv)
{
	printf("Device ID..");
	Data[0] = JTAG_IDCODE;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = 0x00000000;
	jtag_dreg(Data, 32);
	printf("0x%8.8lX\n", Data[0]);

	cpu_id = Data[0];

	Data[0] = JTAG_RESTART;
	jtag_ireg(Data, JTAG_IRLENGTH);

	return(0);
}

/* int jtag_devicecode(void)
{
	printf("Raw device referance - ");
	Data[0] = JTAG_IDCODE;
	jtag_ireg(Data, JTAG_IRLENGTH);

	Data[0] = 0x00000000;
	jtag_dreg(Data, 32);

	cpu_id = Data[0];

	printf("0x%8.8lX\n\n", Data[0]);

	Data[0] = JTAG_RESTART;
	jtag_ireg(Data, JTAG_IRLENGTH);

	return(0);
} */

