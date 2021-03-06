/*
 *  linux/include/asm-arm/bugs.h
 *
 *  Copyright (C) 1995  Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_BUGS_H
#define __ASM_BUGS_H

#include <linux/config.h>
#include <asm/proc-fns.h>

extern void check_writebuffer_bugs(void);

static inline void check_bugs(void)
{
#ifdef CONFIG_CPU_32
	check_writebuffer_bugs();
#endif
	cpu_check_bugs();
}

#endif
