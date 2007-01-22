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

/* Include file for JTAG functions */

#define JTAG_TMS	0x02
#define JTAG_CLK	0x04
#define JTAG_TDI	0x08
#define JTAG_PWR	0x80

#define JTAG_TDO	0x80	/* remember this signal is inverted */

#define JTAG_IRLENGTH	4

#define JTAG_EXTEST	0x00
#define JTAG_SCAN_N	0x02
#define JTAG_INTEST	0x0C
#define JTAG_IDCODE	0x0E
#define JTAG_BYPASS	0x0F
#define JTAG_CLAMP	0x05
#define JTAG_HIGHZ	0x07
#define JTAG_CLAMPZ	0x09
#define JTAG_SAMPLE	0x03
#define JTAG_RESTART	0x04

/* CPU type defines */
#define ARM7TDMI	0x1F0F0F0F
#define ARM920		0x00920F0F
#define ARM922		0x00922F0F
#define ARM940		0x00940F0F
#define S3C2410		0x0032409D
#define S3C2510		0x1094009D
#define GP2X		0x10920F0F

extern unsigned long Data[10];
extern unsigned long cpu_id;

int jtag_ireg(unsigned long *data, unsigned int length);
int jtag_dreg(unsigned long *data, unsigned int length);

int jtag_init(void);
/* int jtag_devicecode(void); */
int jtag_reset(int argc, char **argv);
int jtag_idcode(int argc, char **argv);
