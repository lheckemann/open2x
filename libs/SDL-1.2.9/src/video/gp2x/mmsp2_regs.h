/*
 * MMSP2 (MP2520F) Registers
 *  Only listed needed regs for HW blit stuff
 *
 * 2005-12-20 Paeryn
 */
  

#ifndef _MMSP2_REG_H_
#define _MMSP2_REG_H_

/*
 * Blitter registers
 */

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

/*
 * Basic ROPs
 */
#define MESG_ROP_NULL 0xAA
#define MESG_ROP_COPY 0xCC
#define MESG_ROP_PAT  0xF0


/*
 * Control registers
 */
#define SYSCLKENREG   0x0904>>1
#define FASTIOCLK 1<<10

#define VCLKENREG     0x090a>>1
#define GRPCLK 1<<2

#define SC_STATUS     0x1802>>1
#define SC_DISP_FIELD 1<<7

#define GPIOB_PINLVL 0x1182>>1
#define GPIOB_VSYNC 1<<4

#define DPC_CNTL      0x2800>>1
#define DPC_INTERLACE 1<<5
#define DPC_X_MAX     0x2816>>1
#define DPC_Y_MAX     0x2818>>1
#define DPC_CLKCNTL   0x2848>>1
#define MLC_STL_CNTL  0x28da>>1
#define MLC_STL_BPP 9
#define MLC_STL_BPP_4 0x00aa
#define MLC_STL_BPP_8 0x02aa
#define MLC_STL_BPP_16 0x04aa
#define MLC_STL_BPP_24 0x06aa
#define MLC_STL5ACT 1<<8
#define MLC_STL4ACT 1<<6
#define MLC_STL3ACT 1<<6
#define MLC_STL2ACT 1<<4
#define MLC_STL1ACT 1<<0
#define MLC_STL_DEFAULT 0xaa

#define MLC_STL_MIXMUX    0x28dc>>1
#define MLC_STL5_MIXMUX 8
#define MLC_STL4_MIXMUX 6
#define MLC_STL3_MIXMUX 4
#define MLC_STL2_MIXMUX 2
#define MLC_STL1_MIXMUX 0

#define MLC_STL_ALPHAL 0x28de>>1
#define MLC_STL3_ALPHA 8
#define MLC_STL2_ALPHA 4
#define MLC_STL1_ALPHA 0

#define MLC_STL_ALPHAH 0x28e0>>1
#define MLC_STL5_ALPHA 4;
#define MLC_STL4_ALPHA 0;

#define MLC_STL1_STX  0x28e2>>1
#define MLC_STL1_ENDX 0x28e4>>1
#define MLC_STL1_STY  0x28e6>>1
#define MLC_STL1_ENDY 0x28e8>>1
#define MLC_STL2_STX  0x28ea>>1
#define MLC_STL2_ENDX 0x28ec>>1
#define MLC_STL2_STY  0x28ee>>1
#define MLC_STL2_ENDY 0x28f0>>1
#define MLC_STL3_STX  0x28f2>>1
#define MLC_STL3_ENDX 0x28f4>>1
#define MLC_STL3_STY  0x28f6>>1
#define MLC_STL3_ENDY 0x28f8>>1
#define MLC_STL4_STX  0x28fa>>1
#define MLC_STL4_ENDX 0x28fc>>1
#define MLC_STL4_STY  0x28fe>>1
#define MLC_STL4_ENDY 0x2900>>1
#define MLC_STL_CKEY_GB 0x2902>>1
#define MLC_STL_CKEYG 8
#define MLC_STL_CKEYB 0
#define MLC_STL_CKEY_R 0x2904>>1
#define MLC_STL_HSC   0x2906>>1
#define MLC_STL_VSCL  0x2908>>1
#define MLC_STL_VSCH  0x290a>>1
#define MLC_STL_HW    0x290c>>1
#define MLC_STL_OADRL 0x290e>>1
#define MLC_STL_OADRH 0x2910>>1
#define MLC_STL_EADRL 0x2912>>1
#define MLC_STL_EADRH 0x2914>>1
#define MLC_STL_PALLT_A 0x2958>>1
#define MLC_STL_PALLT_D 0x295a>>1


#endif
