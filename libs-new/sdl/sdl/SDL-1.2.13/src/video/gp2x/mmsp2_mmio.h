/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2004 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: mmsp2_mmio.h,v 1.5 2005/12/20 23:29:33 paeryn Exp $";
#endif

// MMSP2 register definitions

#include "mmsp2_regs.h"
#define NOFBACCEL_DEBUG 1

// Wait for fifo space
#define mmsp2_wait(space)						\
{									\
  do {} while (mmsp2_blit_regs[MESGFIFOSTATUS] & MESG_FULL);		\
}

// Wait for idle accelerator
#define mmsp2_waitidle()						\
{									\
  do {} while (mmsp2_blit_regs[MESGSTATUS] & MESG_BUSY);		\
}

// Convert virtual address to physical (fbmem only)
#define mmsp2_addr(addr) (((Uint32)addr)-((Uint32)mapped_mem)+0x3101000)
