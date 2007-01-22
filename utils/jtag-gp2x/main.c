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

#include "jtag.h"
#include "parse.h"
/* #include "debug.h" */

char input_line[128];
unsigned long cpu_id=0;

int main(void)
{
	printf("Open2x JTAG tool for the GP2X - (c) Open2x Project 2006\n");
	printf("Largely based on Jtag-Arm9 - (c) Simon Wood 2001\n\n");
	printf("For commands type 'help', to exit type 'quit'\n\n");
	printf("To unbrick a GP2X type 'script gp2x_unbrick.sh' at the prompt\n\n");

	if (jtag_init())
		return(-1);

	jtag_reset(0, 0);
	jtag_idcode(0, 0);

	if(cpu_id == GP2X) {
		printf("MagicEyes MP2520F CPU (GP2X) found.\n\n");
	} else {
		printf("Sorry, MagicEyes MP2520F CPU (GP2X) not found.\n\n");
		printf("Is everything connected correctly and do you have\n");
		printf("root permissions to the parallel port?\n\n");
		exit(0);
	}

	/* debug_halt(0,0); */

	/* parse_main("halt"); */

	while (1) {
		printf("JTAG:# ");
		fgets(input_line, sizeof(input_line), stdin);

		parse_main(input_line);
	}

	exit(0);
}
