/*
 * linux/include/asm-arm/arch-mmsp2/uncompress.h
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define UART0		((volatile unsigned short *)0xc0001200)
#define UART1		((volatile unsigned short *)0xc0001220)
#define UART2		((volatile unsigned short *)0xc0001240)
#define UART3		((volatile unsigned short *)0xc0001260)

#define UART	UART0

static __inline__ void putc(char c)
{
	while (!(UART[4] & 0x4));
	UART[8] = c;

}

static void puts(const char *s)
{
	while (*s) {
		putc(*s);
		if (*s == '\n')
			putc('\r');
		s++;
	}
}

/*
 * nothing to do
 */
#define arch_decomp_setup()
#define arch_decomp_wdog()
