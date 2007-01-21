/*
 * linux/include/asm-arm/arch-mmsp2/system.h
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "hardware.h"

static inline void arch_idle(void)
{
	cpu_do_idle();
}

static inline void arch_reset(char mode)
{
	if (mode == 's') {
		/* Jump into ROM at address 0 */
		cpu_reset(0);
	} else {
		/* Initialize the watchdog and let it fire */
		TMATCH3 = TCOUNT + 3703703;	/* ... in 100 ms */
		TCONTROL |= (1 << 1);		/* Enable watch-dog timer */
		TINTEN |= TCNT3;
	}
}

