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

int debug_poll(int argc, char **argv);
int debug_halt(int argc, char **argv);
int debug_restart(int argc, char **argv);
int debug_exec(unsigned long * data, int type);

#define DEBUG_SPEED		0
#define SYSTEM_SPEED		1
#define DEBUG_SPEED_SHORT	2	
