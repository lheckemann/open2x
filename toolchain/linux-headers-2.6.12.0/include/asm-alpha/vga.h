/*
 *	Access to VGA videoram
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 */

#ifndef _LINUX_ASM_VGA_H_
#define _LINUX_ASM_VGA_H_

#include <asm/io.h>

#define VT_BUF_HAVE_RW
#define VT_BUF_HAVE_MEMSETW
#define VT_BUF_HAVE_MEMCPYW

extern inline void scr_writew(__u16 val, volatile __u16 *addr)
{
	if (__is_ioaddr(addr))
		__raw_writew(val, (volatile __u16 *) addr);
	else
		*addr = val;
}

extern inline __u16 scr_readw(volatile const __u16 *addr)
{
	if (__is_ioaddr(addr))
		return __raw_readw((volatile const __u16 *) addr);
	else
		return *addr;
}

extern inline void scr_memsetw(__u16 *s, __u16 c, unsigned int count)
{
	if (__is_ioaddr(s))
		memsetw_io((__u16 *) s, c, count);
	else
		memsetw(s, c, count);
}

/* Do not trust that the usage will be correct; analyze the arguments.  */
extern void scr_memcpyw(__u16 *d, const __u16 *s, unsigned int count);

/* ??? These are currently only used for downloading character sets.  As
   such, they don't need memory barriers.  Is this all they are intended
   to be used for?  */
#define vga_readb(a)	readb((__u8 *)(a))
#define vga_writeb(v,a)	writeb(v, (__u8 *)(a))

#define VGA_MAP_MEM(x)	((unsigned long) ioremap(x, 0))

#endif
