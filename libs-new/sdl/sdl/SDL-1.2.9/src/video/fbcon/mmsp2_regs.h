/*
 * MMSP2 (MP2520F) Registers
 *  Only listed needed registers for HW blit stuff
 *
 * 2005-12-20 Paeryn
 */
static char rcsid=
  "@(#) $Id: mmsp2_regs.h,v 1.0 2006/01/04 18:57:00 paeryn Exp $";
  

#ifndef _MMSP2_REG_H_
#define _MMSP2_REG_H_

// Debugging defines
#define FBCON_DEBUG 1
#define FBACCEL_DEBUG 1

////
// Blitter registers

#define MESGDSTCTRL      0x0000>>2
#define MESG_DSTENB    1<<6
#define MESG_BSTBPP    0x60
#define MESG_DSTBPP_8  0<<5
#define MESG_DSTBPP_16 1<<5

#define MESGDSTADDR      0x0004>>2

#define MESGDSTSTRIDE    0x0008>>2

#define MESGSRCCTRL      0x000c>>2
#define MESG_INVIDEO   1<<8
#define MESG_SRCENB    1<<7
#define MESG_SRCBPP    0x60
#define MESG_SRCBPP_8  0<<5
#define MESG_SRCBPP_16 1<<5
#define MESG_SRCBPP_1  1<<6

#define MESGSRCADDR      0x0010>>2

#define MESGSRCSTRIDE    0x0014>>2

#define MESGSRCFORCOLOR  0x0018>>2

#define MESGSRCBACKCOLOR 0x001c>>2

#define MESGPATCTRL      0x0020>>2
#define MESG_PATMONO   1<<6
#define MESG_PATENB    1<<5
#define MESG_PATBPP    0x18
#define MESG_PATBPP_8  0<<3
#define MESG_PATBPP_16 1<<3
#define MESG_PATBPP_1  1<<4
#define MESG_YOFFSET   0x07

#define MESGFORCOLOR     0x0024>>2

#define MESGBACKCOLOR    0x0028>>2

#define MESGSIZE         0x002c>>2
#define MESG_HEIGHT 16
#define MESG_WIDTH  0

#define MESGCTRL         0x0030>>2
#define MESG_TRANSPCOLOR 16
#define MESG_TRANSPEN    1<<11
#define MESG_FFCLR       1<<10
#define MESG_YDIR        1<<9
#define MESG_YDIR_NEG    0<<9
#define MESG_YDIR_POS    1<<9
#define MESG_XDIR        1<<8
#define MESG_XDIR_NEG    0<<8
#define MESG_XDIR_POS    1<<8
#define MESG_ROP         0xff

#define MESGSTATUS       0x0034>>2
#define MESG_BUSY 1<<0

#define MESGFIFOSTATUS   0x0038>>2
#define MESG_FULL    1<<31
#define MESG_REMAIN  0x1f

#define MESGFIFO         0x003c>>2
#define MESGPAT          0x0080>>2

////
// Basic ROPs
#define MMSP2_ROP_SRC 0xCC
#define MMSP2_ROP_PAT 0xF0

////
// Control registers
#define SYSCLKENREG   0x0904>>1
#define FASTIOCLK 1<<10

#define VCLKENREG     0x090a>>1
#define GRPCLK 1<<2

#define MLC_STL_OADRL 0x290e>>1
#define MLC_STL_OADRH 0x2910>>1
#define MLC_STL_EADRL 0x2912>>1
#define MLC_STL_EADRH 0x2914>>1

// GPIOB[4] is vsync
#define GPIOB_PINLVL 0x1182>>1
#define GPIOB_VSYNC 1<<4

////
// A few macros,

// mmsp2_wait()
//   Waits until the blitter is ready to accept next command
void mmsp2_blit_wait(void)
{
  do {} while (mmsp2_blit_regs[MESGSTATUS] & MESG_BUSY);
}

// Uint32 mmsp2_virt2phys(void *addr)
//   Convert virtual address to physical
Uint32 mmsp2_virt2phys(void *)
{
  return (Uint32)addr - (Uint32)mapped_mem + 0x3101000;
}

#endif
