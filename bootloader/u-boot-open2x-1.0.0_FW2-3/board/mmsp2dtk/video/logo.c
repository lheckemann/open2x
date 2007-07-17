//[*]----------------------------------------------------------------------------------------------------[*]
/*************************************************************************
  	(C) Copyright 2005

Project : GPX_FIRST_BOOT_LOGO
Version : 1.0
Date    : 2005-10-8(Lee) update:2005-10-11(hyun)
Author  : Lee chang whan & Hyun sog juk
Company : GamePark Holdings, Korea
Comments: ORIGNAL SOUCE => MAGIC EYES DISPLAY DRIVE
**************************************************************************/
//[*]----------------------------------------------------------------------------------------------------[*]
#include <common.h>
#include <mmsp20.h>

#include "../video/gp2xlcd.h"
#include "../video/proto_dpc.h"
#include "../video/proto_mlc.h"
#include "../video/proto_pwrman.h"
#include "../video/mmsp2-regs.h"

#ifdef CONFIG_GP2X_SPLASH
	#include "logo_open2x.h"
#endif

//[*]----------------------------------------------------------------------------------------------------[*]
#define LCD_WIDTH 	320
#define LCD_HEIGHT 	240
#define PLANE_X_OFFSET 0
#define PLANE_Y_OFFSET 0
#define PLANE_X_WIDTH  LCD_WIDTH
#define PLANE_Y_HEIGHT LCD_HEIGHT
//[*]----------------------------------------------------------------------------------------------------[*]
MMSP20_CLOCK_POWER 	*pPMR_REG;
MMSP20_DPC 			*DPC;
MMSP20_GPIO 		*pGPIO_REG;
MMSP20_MLC 			*MLC_REG;
//[*]----------------------------------------------------------------------------------------------------[*]
void PMR_Initialize()
{
	pPMR_REG = MMSP20_GetBase_CLOCK_POWER();
}
//[*]----------------------------------------------------------------------------------------------------[*]
void PMR_SetDispClk(tDispClkInfo *pDISPCLKINFO)
{
	PMR_Initialize();

	pPMR_REG->DISPCSETREG = (pDISPCLKINFO->DISPCLK_SOURCE << 14) | (pDISPCLKINFO->DISPCLK_DIVIDER << 8) | (pDISPCLKINFO->DISPCLK_POL << 7);
}
//[*]----------------------------------------------------------------------------------------------------[*]
void DPC_SetFrameSize(U16 Width, U16 Height)
{
	DPC->DPC_X_MAX = Width - 1;
	DPC->DPC_Y_MAX = Height - 1;
}
//[*]----------------------------------------------------------------------------------------------------[*]
void DPC_SetHSyncParm(U16 HorSyncWidth, U16 HorFrontPorch, U16 HorBackPorch, CBOOL HorPolarity)
{
	/* Horizontal Sync Width */
	DPC->DPC_HS_WIDTH 	&= ~HSWID;
	DPC->DPC_HS_WIDTH 	|= FInsrt(HorSyncWidth - 1,BF_HSWID);	// T2
	DPC->DPC_HS_END 	= (HorFrontPorch/2) - 1;	// T7
	DPC->DPC_HS_STR 	= (HorFrontPorch/2) + (HorFrontPorch%1) - 1; //T8
	/* Horizontal Back Porch */
	DPC->DPC_DE &= ~DESTR;
	DPC->DPC_DE |= FInsrt((HorBackPorch - 1),BF_DESTR);
	if(HorPolarity)	DPC->DPC_FPIPOL1 |= HSPOL;
	else			DPC->DPC_FPIPOL1 &= ~HSPOL;
}
//[*]----------------------------------------------------------------------------------------------------[*]
void DPC_SetVSyncParm(U16 VerSyncWidth, U16 VerFrontPorch, U16 VerBackPorch, CBOOL VerPolarity)
{
	DPC->DPC_V_SYNC &= ~VSWID;
	DPC->DPC_V_SYNC |= FInsrt((VerSyncWidth-1),BF_VSWID);
	DPC->DPC_V_SYNC &= ~VSSTR;
	DPC->DPC_V_SYNC |= FInsrt(VerFrontPorch,BF_VSSTR);
	DPC->DPC_V_END &= ~VSEND;
	DPC->DPC_V_END |= FInsrt(VerBackPorch+VerSyncWidth+VerFrontPorch-1,BF_VSEND);
	DPC->DPC_FPIPOL1 &= ~VSPOL;
	DPC->DPC_FPIPOL1 |= VerPolarity;
}
//[*]----------------------------------------------------------------------------------------------------[*]
void DPC_SetDEParm(U32 DEMode, U16 DEPulseWidth, U16 DEShift, CBOOL DEPolarity)
{
	if(DEMode == DPC_DE_DE)
	{
		DPC->DPC_DE &= ~DEPADCFG;
		DPC->DPC_DE |= DEPIN_DE;
		DPC->DPC_DE &= ~DEMOD;
		DPC->DPC_DE &= ~DEWID;
		DPC->DPC_DE |= FInsrt(DEShift, BF_DEWID);
	}
	else if(DEMode == DPC_DE_STH)
	{
		DPC->DPC_DE &= ~DEPADCFG;
		DPC->DPC_DE |= DEPIN_STH;
		DPC->DPC_DE &= ~DEMOD;
		DPC->DPC_DE &= ~DEWID;
	}
	else if(DEMode == DPC_DE_PULSE)
	{
		DPC->DPC_DE &= ~DEPADCFG;
		DPC->DPC_DE |= DEPIN_DE;
		DPC->DPC_DE |= DEMOD;
		DPC->DPC_DE &= ~DEWID;
		DPC->DPC_DE |= FInsrt(DEPulseWidth-1, BF_DEWID);
	}
	if(DEPolarity)	DPC->DPC_FPIPOL1 |= DEPOL;
	else			DPC->DPC_FPIPOL1 &= ~DEPOL;
}
//[*]----------------------------------------------------------------------------------------------------[*]
void DPC_UTIL_HVSYNC(U8 DataType, U16 Width, U16 Height, U16 HorSyncWidth, U16 HorFrontPorch, U16 HorBackPorch, CBOOL HorPolarity, U16 VerSyncWidth, U16 VerFrontPorch, U16 VerBackPorch, CBOOL VerPolarity)
{
	DPC_SetFrameSize(Width, Height);
	DPC_SetHSyncParm(HorSyncWidth, HorFrontPorch, HorBackPorch, HorPolarity);
	DPC_SetVSyncParm(VerSyncWidth, VerFrontPorch, VerBackPorch, VerPolarity);
	DPC_SetDEParm((U8)DPC_DE_DE, 0, 0, CFALSE );
}
//[*]----------------------------------------------------------------------------------------------------[*]
void DPC_UTIL_HVSYNC_GPX320240
(
	U8  	DataType,
	U16 	Width,
	U16 	Height,
	U16 	HorSyncWidth,
	U16 	HorFrontPorch,
	U16 	HorBackPorch,
	CBOOL   HorPolarity,
	U16 	VerSyncWidth,
	U16 	VerFrontPorch,
	U16 	VerBackPorch,
	CBOOL   VerPolarity,
	CBOOL	ClkPolarity,
	CBOOL	DePolarity,
	CBOOL	DataPolarity
)
{

	DPC_SetFrameSize ( Width, Height );
	DPC_SetHSyncParm ( HorSyncWidth, HorFrontPorch, HorBackPorch, HorPolarity);
	DPC_SetVSyncParm ( VerSyncWidth, VerFrontPorch, VerBackPorch, VerPolarity);
	DPC_SetDEParm ( (U8)DPC_DE_DE, 0, 0, DePolarity );

	if(ClkPolarity)
		DPC->DPC_FPIPOL1 |= CHPOL;
	else
		DPC->DPC_FPIPOL1 &= ~CHPOL;

	if(DataPolarity){
		DPC->DPC_FPIPOL2 = DATA_FPIPOL2;
		DPC->DPC_FPIPOL3 = DATA_FPIPOL3;
	}else{
		DPC->DPC_FPIPOL2 = ~DATA_FPIPOL2;
		DPC->DPC_FPIPOL3 = ~DATA_FPIPOL3;
	}
}
//[*]----------------------------------------------------------------------------------------------------[*]
void DPC_UTIL_DATA_INV(CBOOL DataPolarity)
{
	if(DataPolarity){
		DPC->DPC_FPIPOL2 = DATA_FPIPOL2;
		DPC->DPC_FPIPOL3 = DATA_FPIPOL3;
	}else{
		DPC->DPC_FPIPOL2 = ~DATA_FPIPOL2;
		DPC->DPC_FPIPOL3 = ~DATA_FPIPOL3;
	}
}



//[*]----------------------------------------------------------------------------------------------------[*]
void DPC_InitHardware
(
	DPC_OPER_MODE  	Mode,
	U16  	IsInterlace,		// Prograssive or Interlace Mode.
	U16  	IsPALMode,          // It is only valid when DOT is YCbCr Mode
	U16  	IntEn,				// Interrupt Enable
	U16   	IntHSyncEdgePos,	// Interrupt Edge Position in HSYNC.
	U16   	IntVSyncEdgePos,	// Interrupt Edge Position in VSYNC.
	U16  	ClockSource			// Clock Configuration.
)
{
//	ds_printk("Display Output Mode = %d\n", Mode);

	DPC->DPC_CNTL &= ~DOT;
	DPC->DPC_CNTL |= FInsrt((Mode>>2)&0x3, BF_DOT);	//(Mode<<4)&DOT;

	DPC->DPC_CNTL &= ~DOF;
	DPC->DPC_CNTL |= FInsrt(Mode&0x3, BF_DOF);	//(Mode<<1)&DOF;

	switch(Mode) {
	case DPC_RGB_444:
	case DPC_RGB_565:
	case DPC_RGB_666:
	case DPC_RGB_888:
		DPC->DPC_CLKCNTL 	&= ~CLK1SEL;
		DPC->DPC_CLKCNTL 	&= ~CLK2SEL;
		DPC->DPC_CNTL 		&= ~CSYNC;
		DPC->DPC_CNTL 		&= ~PAL;
		DPC->DPC_CNTL 		&= ~HDTV;
		DPC->DPC_CNTL 		&= ~ENC;
		DPC->DPC_CNTL 		&= ~TVMODE;
		DPC->DPC_CNTL 		&= ~SYNCCBCR;
		DPC->DPC_CNTL 		&= ~ESAVEN;
		break;
	case DPC_MRGB_555:
	case DPC_MRGB_565:
	case DPC_MRGB_888A:
	case DPC_MRGB_888B:
		DPC->DPC_CLKCNTL |= CLK1SEL;
		DPC->DPC_CLKCNTL &= ~CLK2SEL;
		DPC->DPC_CNTL &= ~CSYNC;
		DPC->DPC_CNTL &= ~PAL;
		DPC->DPC_CNTL &= ~HDTV;
		DPC->DPC_CNTL &= ~ENC;
		DPC->DPC_CNTL &= ~TVMODE;
		DPC->DPC_CNTL &= ~SYNCCBCR;
		DPC->DPC_CNTL &= ~ESAVEN;
		break;
	case DPC_YCBCR_CCIR656:
		DPC->DPC_CLKCNTL |= CLK1SEL;
		DPC->DPC_CLKCNTL &= ~CLK2SEL;
		DPC->DPC_CNTL &= ~CSYNC;
		DPC->DPC_CNTL &= ~PAL;
		DPC->DPC_CNTL &= ~HDTV;
		DPC->DPC_CNTL |= ENC;
		DPC->DPC_CNTL |= TVMODE;
		DPC->DPC_CNTL &= ~SYNCCBCR;
		DPC->DPC_CNTL |= ESAVEN;
		DPC->DPC_V_END |= NOSYMATVLINE;
		DPC->DPC_V_END |= NOSYMATVFLD;
		DPC->DPC_V_SYNC |= VSFLDEN;
		DPC->DPC_V_SYNC |= VSFLDPOL;
		break;
	case DPC_YCBCR_CCIR601:
	case DPC_YCBCR_3x8:
		DPC->DPC_CLKCNTL &= ~CLK1SEL;
		DPC->DPC_CLKCNTL &= ~CLK2SEL;
		DPC->DPC_CNTL &= ~CSYNC;
		if(IsPALMode)
			DPC->DPC_CNTL |= PAL;
		else
			DPC->DPC_CNTL &= ~PAL;
		DPC->DPC_CNTL &= ~HDTV;
		DPC->DPC_CNTL |= ENC;
		DPC->DPC_CNTL |= TVMODE;
		DPC->DPC_CNTL &= ~SYNCCBCR;
		DPC->DPC_CNTL &= ~ESAVEN;
		break;
	case DPC_YCBCR_3x10:
		DPC->DPC_CLKCNTL &= ~CLK1SEL;
		DPC->DPC_CLKCNTL &= ~CLK2SEL;
		DPC->DPC_CNTL &= ~CSYNC;
		if(IsPALMode)
			DPC->DPC_CNTL |= PAL;
		else
			DPC->DPC_CNTL &= ~PAL;
		DPC->DPC_CNTL &= ~HDTV;
		DPC->DPC_CNTL |= ENC;
		DPC->DPC_CNTL &= ~TVMODE;
		DPC->DPC_CNTL &= ~SYNCCBCR;
		DPC->DPC_CNTL |= ESAVEN;
		break;
	default:
		break;
	}

	if(IsInterlace)		DPC->DPC_CNTL |= INTERLACE;
	else				DPC->DPC_CNTL &= ~INTERLACE;
 	if((IntEn>>1)&1)	DPC->DPC_INTR |= VSINTEN;
	else				DPC->DPC_INTR &= ~VSINTEN;
	if(IntEn&0x1)		DPC->DPC_INTR |= HSINTEN;
	else				DPC->DPC_INTR &= ~HSINTEN;
	if(IntHSyncEdgePos)	DPC->DPC_INTR |= HSINTEDGE;
	else				DPC->DPC_INTR &= ~HSINTEDGE;
	if(IntVSyncEdgePos)	DPC->DPC_INTR |= VSINTEDGE;
	else				DPC->DPC_INTR &= ~VSINTEDGE;

	DPC->DPC_CLKCNTL &= ~CLKSRC;
	DPC->DPC_CLKCNTL |= FInsrt(ClockSource, BF_CLKSRC);
	DPC->DPC_CNTL &= ~SLAVE;	//Master Mode

	//ds_printk("<1> DPC_CNTL = 0x%02X\n", DPC->DPC_CNTL);
}
//[*]----------------------------------------------------------------------------------------------------[*]
//int do_bmpload_sub(unsigned char *, int, int, int);
//void DPC_InitHardware(DPC_OPER_MODE Mode, U16 IsInterlace, U16 IsPALMode, U16 IntEn, U16 IntHSyncEdgePos, U16 IntVSyncEdgePos, U16 ClockSource);
//[*]----------------------------------------------------------------------------------------------------[*]
const unsigned int _pins_of_gpio[] = {
	[GRP_NUM(GPIO_A0)] = 16,
	[GRP_NUM(GPIO_B0)] = 16,
	[GRP_NUM(GPIO_C0)] = 16,
	[GRP_NUM(GPIO_D0)] = 14,
	[GRP_NUM(GPIO_E0)] = 16,
	[GRP_NUM(GPIO_F0)] = 10,
	[GRP_NUM(GPIO_G0)] = 16,
	[GRP_NUM(GPIO_H0)] = 7,
	[GRP_NUM(GPIO_I0)] = 16,
	[GRP_NUM(GPIO_J0)] = 16,
	[GRP_NUM(GPIO_K0)] = 8,
	[GRP_NUM(GPIO_L0)] = 15,
	[GRP_NUM(GPIO_M0)] = 9,
	[GRP_NUM(GPIO_N0)] = 8,
	[GRP_NUM(GPIO_O0)] = 6,
};

void GPIO_Initialize(void)
{
	pGPIO_REG = MMSP20_GetBase_GPIO();
}
//[*]----------------------------------------------------------------------------------------------------[*]
void inline write_gpio_bit(unsigned int num, unsigned int val)
{
	unsigned short value;
	unsigned long bit_ofs = num & 0xf;

	GPIO_Initialize();

	value = pGPIO_REG->GPIOOUT[GRP_NUM(num)] & ~(1<<bit_ofs);
	value |= (val << bit_ofs);
	pGPIO_REG->GPIOOUT[GRP_NUM(num)] = value;
}
//[*]----------------------------------------------------------------------------------------------------[*]
void _set_mmsp2_gpio_func(unsigned int addr_l, unsigned int addr_h, unsigned int pins, unsigned int bit, unsigned int func)
{
	unsigned short 	value_l 	= readw(addr_l);
	unsigned short 	value_h 	= readw(addr_h);
	unsigned int 	*addr_t 	= &addr_l;
	unsigned short 	*value_t 	= &value_l;
	unsigned int 	bitofs_t 	= bit * 2;

	value_l |= (value_h << pins);
	value_h >>= (16-pins);

	if(bit > 7)
	{
		bitofs_t -= (8*2);
		addr_t = &addr_h;
		value_t = &value_h;
	}

	*value_t &= ~(0x3 << bitofs_t);
	*value_t |= ((func & 0x03) << bitofs_t);

	writew(*value_t, *addr_t);

	// verify
//	BUG_ON(func != _get_mmsp2_gpio_func(addr_l, addr_h, pins, bit));
}
//[*]----------------------------------------------------------------------------------------------------[*]
void set_gpio_ctrl(unsigned int num, unsigned int func, unsigned int pu)
{
	unsigned long reg_offset = GRP_NUM(num);		//((0xf0 & num) >> 3);
    unsigned long addr_l, addr_h;
	unsigned long bit_ofs = (0x7 & num) * 2;    // 0,1,2,.. --> 0,2,4,6,...
	unsigned short temp;

	GPIO_Initialize();

    addr_l = &(pGPIO_REG->GPIOALTFNLOW[reg_offset]);  //io_p2v(0xc0001020 + reg_offset);
	addr_h = &(pGPIO_REG->GPIOALTFNHI[reg_offset]);   //io_p2v(0xc0001040 + reg_offset);

	_set_mmsp2_gpio_func(addr_l, addr_h, _pins_of_gpio[GRP_NUM(num)], num & 0xf, func);

	// GPIO Pullup
	addr_l = &(pGPIO_REG->GPIOPUENB[reg_offset]);		//io_p2v(0xc00010c0 + reg_offset);
	bit_ofs = num & 0xf;
	temp = readw(addr_l);   // PUENB

	if(pu == GPIOPU_NOSET)	return;

	temp &= ~(1 << bit_ofs);
	temp |= (pu << bit_ofs);

	writew(temp, addr_l);
}
//[*]----------------------------------------------------------------------------------------------------[*]
void DPC_Initialize( void )
{
	DPC = MMSP20_GetBase_DPC();
}
//[*]----------------------------------------------------------------------------------------------------[*]
void MLC_Initialize(	void )
{
	MLC_REG = MMSP20_GetBase_MLC();
}
//[*]----------------------------------------------------------------------------------------------------[*]
void MLC_RGB_MixMux(U8 rgnNum, U8 MixMux, U8 Alpha)
{
	if ( rgnNum == MLC_RGB_RGN_1 )
	{
		// MixMux
		if ( MixMux == MLC_RGB_MIXMUX_PRI )			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL1_MIXMUX ) | FInsrt(MLC_RGB_MIXMUX_PRI, BF_MLC_STL1_MIXMUX));	// [1:0] <= 0
		else if ( MixMux == MLC_RGB_MIXMUX_CKEY )	MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL1_MIXMUX ) | FInsrt(MLC_RGB_MIXMUX_CKEY, BF_MLC_STL1_MIXMUX));	// [1:0] <= 1
		else //if ( MixMux == MLC_RGB_MIXMUX_ALPHA )
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL1_MIXMUX ) | FInsrt(MLC_RGB_MIXMUX_ALPHA, BF_MLC_STL1_MIXMUX));	// [1:0] <= 2

		// Alpha 1
		MLC_REG->RGB_ALPHA_L = (U16)(( MLC_REG->RGB_ALPHA_L & ~MLC_STL1_ALPHA ) | FInsrt(Alpha,BF_MLC_STL1_ALPHA));
	}
	else if ( rgnNum == MLC_RGB_RGN_2 )
	{
		// MixMux
		if ( MixMux == MLC_RGB_MIXMUX_PRI )				// [3:2] <= 0
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL2_MIXMUX )
			                                        | FInsrt(MLC_RGB_MIXMUX_PRI, BF_MLC_STL2_MIXMUX));
		else if ( MixMux == MLC_RGB_MIXMUX_CKEY )			// [3:2] <= 1
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL2_MIXMUX )
			                                        | FInsrt(MLC_RGB_MIXMUX_CKEY, BF_MLC_STL2_MIXMUX));
		else //if ( MixMux == MLC_RGB_MIXMUX_ALPHA )		// [3:2] <= 2
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL2_MIXMUX )
			                                        | FInsrt(MLC_RGB_MIXMUX_ALPHA, BF_MLC_STL2_MIXMUX));

		// Alpha 2
		MLC_REG->RGB_ALPHA_L = (U16)(( MLC_REG->RGB_ALPHA_L & ~MLC_STL2_ALPHA ) | FInsrt(Alpha,BF_MLC_STL2_ALPHA));
	}
	else if ( rgnNum == MLC_RGB_RGN_3 )
	{
		// MixMux
		if ( MixMux == MLC_RGB_MIXMUX_PRI )				// [5:4] <= 0
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL3_MIXMUX )
			                                        | FInsrt(MLC_RGB_MIXMUX_PRI, BF_MLC_STL3_MIXMUX));
		else if ( MixMux == MLC_RGB_MIXMUX_CKEY )			// [5:4] <= 1
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL3_MIXMUX )
			                                        | FInsrt(MLC_RGB_MIXMUX_CKEY, BF_MLC_STL3_MIXMUX));
		else //if ( MixMux == MLC_RGB_MIXMUX_ALPHA )		// [5:4] <= 2
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL3_MIXMUX )
			                                        | FInsrt(MLC_RGB_MIXMUX_ALPHA, BF_MLC_STL3_MIXMUX));

		// Alpha 3
		MLC_REG->RGB_ALPHA_L = (U16)(( MLC_REG->RGB_ALPHA_L & ~MLC_STL3_ALPHA ) | FInsrt(Alpha,BF_MLC_STL3_ALPHA));
	}
	else if ( rgnNum == MLC_RGB_RGN_4 )
	{
		// MixMux
		if ( MixMux == MLC_RGB_MIXMUX_PRI )				// [7:6] <= 0
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL4_MIXMUX )
			                                        | FInsrt(MLC_RGB_MIXMUX_PRI, BF_MLC_STL4_MIXMUX));
		else if ( MixMux == MLC_RGB_MIXMUX_CKEY )			// [7:6] <= 1
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL4_MIXMUX )
			                                        | FInsrt(MLC_RGB_MIXMUX_CKEY, BF_MLC_STL4_MIXMUX));
		else //if ( MixMux == MLC_RGB_MIXMUX_ALPHA )		// [7:6] <= 2
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL4_MIXMUX )
			                                        | FInsrt(MLC_RGB_MIXMUX_ALPHA, BF_MLC_STL4_MIXMUX));

		// Alpha 4
		MLC_REG->RGB_ALPHA_H = (U16)(( MLC_REG->RGB_ALPHA_H & ~MLC_STL4_ALPHA ) | FInsrt(Alpha,BF_MLC_STL4_ALPHA));
	}
	else //if ( rgnNum == MLC_RGB_RGN_5 )
	{
		// MixMux
		if ( MixMux == MLC_RGB_MIXMUX_PRI )				// [9:8] <= 0
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL5_MIXMUX )
			                                        | FInsrt(MLC_RGB_MIXMUX_PRI, BF_MLC_STL5_MIXMUX));
		else if ( MixMux == MLC_RGB_MIXMUX_CKEY )			// [9:8] <= 1
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL5_MIXMUX )
			                                        | FInsrt(MLC_RGB_MIXMUX_CKEY, BF_MLC_STL5_MIXMUX));
		else //if ( MixMux == MLC_RGB_MIXMUX_ALPHA )		// [9:8] <= 2
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL5_MIXMUX )
			                                        | FInsrt(MLC_RGB_MIXMUX_ALPHA, BF_MLC_STL5_MIXMUX));

		// Alpha 5
		MLC_REG->RGB_ALPHA_H = (U16)(( MLC_REG->RGB_ALPHA_H & ~MLC_STL5_ALPHA ) | FInsrt(Alpha,BF_MLC_STL5_ALPHA));
	}
}
//[*]----------------------------------------------------------------------------------------------------[*]
void MLC_RGB_SetScale(U16 SrcPixWidth, U16 SrcPixHeight, U16 SrcStride, U16 DestPixWidth, U16 DestPixHeight)
{
	U32	hScale, vScale;

	if(DestPixWidth)	hScale = (U32) ( ( SrcPixWidth * 1024 ) / DestPixWidth );
	else				hScale = 0;
	if (DestPixHeight)	vScale = (U32) ( ( SrcPixHeight * SrcStride ) / DestPixHeight );
	else				vScale = 0;

	MLC_REG->RGB_HSC = (U16)(hScale & 0xffff);
	MLC_REG->RGB_VSC_L = (U16)(vScale & 0xffff);
	MLC_REG->RGB_VSC_H = (U16)(vScale >> 16);
	MLC_REG->RGB_HW = (U16)(SrcStride);
}
//[*]----------------------------------------------------------------------------------------------------[*]
void MLC_RGB_SetActivate(U8 rgnNum, U8 Activate)
{
	U16	Act;

	// Activate
	if ( Activate == MLC_RGB_RGN_ACT )	Act = 0x1;
	else								Act = 0x0;	// if ( Activate == MLC_RGB_RGN_DISACT )

	if ( rgnNum == MLC_RGB_RGN_1 )
	{
		MLC_REG->RGB_CNTL = (U16)(( MLC_REG->RGB_CNTL & ~MLC_STL1ACT ) | (Act<<0) | 0xaa);
	}
	else if ( rgnNum == MLC_RGB_RGN_2 )
	{
		MLC_REG->RGB_CNTL = (U16)(( MLC_REG->RGB_CNTL & ~MLC_STL2ACT ) | (Act<<2) | 0xaa);
	}
	else if ( rgnNum == MLC_RGB_RGN_3 )
	{
		MLC_REG->RGB_CNTL = (U16)(( MLC_REG->RGB_CNTL & ~MLC_STL3ACT ) | (Act<<4) | 0xaa);
	}
	else if ( rgnNum == MLC_RGB_RGN_4 )
	{
		MLC_REG->RGB_CNTL = (U16)(( MLC_REG->RGB_CNTL & ~MLC_STL4ACT ) | (Act<<6) | 0xaa);
	}
	else	// if ( rgnNum == MLC_RGB_RGN_5 )
	{
		MLC_REG->RGB_CNTL = (U16)(( MLC_REG->RGB_CNTL & ~MLC_STL5ACT ) | (Act<<8) | 0xaa);
	}
}
//[*]----------------------------------------------------------------------------------------------------[*]
void MLC_RGB_SetCoord(U8 rgnNum,	U16 StarX, U16 EndX, U16 StartY, U16 EndY)
{
	if ( rgnNum == MLC_RGB_RGN_1 )
	{
		MLC_REG->RGB1_STX = StarX;
		MLC_REG->RGB1_ENDX = EndX - 1;
		MLC_REG->RGB1_STY = StartY;
		MLC_REG->RGB1_ENDY = EndY - 1;
	}
	else if ( rgnNum == MLC_RGB_RGN_2 )
	{
		MLC_REG->RGB2_STX = StarX;
		MLC_REG->RGB2_ENDX = EndX - 1;
		MLC_REG->RGB2_STY = StartY;
		MLC_REG->RGB2_ENDY = EndY - 1;
	}
	else if ( rgnNum == MLC_RGB_RGN_3 )
	{
		MLC_REG->RGB3_STX = StarX;
		MLC_REG->RGB3_ENDX = EndX - 1;
		MLC_REG->RGB3_STY = StartY;
		MLC_REG->RGB3_ENDY = EndY - 1;
	}
	else if ( rgnNum == MLC_RGB_RGN_4 )
	{
		MLC_REG->RGB4_STX = StarX;
		MLC_REG->RGB4_ENDX = EndX - 1;
		MLC_REG->RGB4_STY = StartY;
		MLC_REG->RGB4_ENDY = EndY - 1;
	}
	else	// if ( rgnNum == MLC_RGB_RGN_5 )
	{
		//ds_printk("rgb_region_number_error?\n");
	}
}
//[*]----------------------------------------------------------------------------------------------------[*]
void MLC_RGB_SetAddress	(U32 OddAddr, U32 EvenAddr)
{
	MLC_REG->RGB_OADR_L = (U16)(OddAddr & 0xffff);
	MLC_REG->RGB_OADR_H = (U16)(OddAddr >> 16);
	MLC_REG->RGB_EADR_L = (U16)(EvenAddr & 0xffff);
	MLC_REG->RGB_EADR_H = (U16)(EvenAddr >> 16);
}
//[*]----------------------------------------------------------------------------------------------------[*]
void MLC_RGB_SetColorKey(U8 rColorKey, U8 gColorKey, U8 bColorKey)
{
	MLC_REG->RGB_CKEY_R = (U16)(rColorKey);
	MLC_REG->RGB_CKEY_GB = (U16)FInsrt(gColorKey, BF_MLC_STL_CKEYG) | (U16)(bColorKey);
}
//[*]----------------------------------------------------------------------------------------------------[*]
void MLC_RGB_SetBPP(U8 BPP)
{
	// BPP

	if(BPP == MLC_RGB_4BPP)			MLC_REG->RGB_CNTL = (U16)(( MLC_REG->RGB_CNTL & ~MLC_STL_BPP ) | FInsrt(MLC_RGB_4BPP, BF_MLC_STL_BPP));	// [10:9] <= 0
	else if(BPP == MLC_RGB_8BPP)	MLC_REG->RGB_CNTL = (U16)(( MLC_REG->RGB_CNTL & ~MLC_STL_BPP ) | FInsrt(MLC_RGB_8BPP, BF_MLC_STL_BPP));	// [10:9] <= 1
	else if(BPP == MLC_RGB_16BPP)	MLC_REG->RGB_CNTL = (U16)(( MLC_REG->RGB_CNTL & ~MLC_STL_BPP ) | FInsrt(MLC_RGB_16BPP, BF_MLC_STL_BPP));	// [10:9] <= 2
	else 							MLC_REG->RGB_CNTL = (U16)(( MLC_REG->RGB_CNTL & ~MLC_STL_BPP ) | FInsrt(MLC_RGB_24BPP, BF_MLC_STL_BPP));	// [10:9] <= 3
}
//[*]----------------------------------------------------------------------------------------------------[*]
void MLC_RGBOn (U8 RegionNr)
{
	MLC_REG->OVLAY_CNTR = (U16)(MLC_REG->OVLAY_CNTR | (1<<(RegionNr+2)));
	//ds_printk("OVLAY_CNTR = 0x%04x\n", MLC_REG->OVLAY_CNTR);
}
//[*]----------------------------------------------------------------------------------------------------[*]
void MLC_Util_RGB
(
	U8	RGBRegionNO, 	//MLC_RGB_RGN_1 ~ MLC_RGB_RGN_5
	U8	bpp,			// MLC_RGB_4BPP, MLC_RGB_8BPP, MLC_RGB_16BPP, MLC_RGB_24BPP
	U8	AlphaCkey,		// MLC_RGB_MIXMUX_PRI, MLC_RGB_MIXMUX_CKEY, MLC_RGB_MIXMUX_ALPHA
	U8	AlphaValue,		// 0 ~ 15
	U8	CkeyR, U8 CkeyG, U8 CkeyB,
	U16	StartX, U16 StartY, U16 EndX, U16 EndY,
	U16 SrcWidth, U16 SrcHeight, U16 DstWidth, U16 DstHeight, U16 Stride,
	U32 addr
)
{
	U8	Interlace;		// 0:Progressive, 1:Interlace

	DPC_Initialize();

	if(DPC->DPC_CNTL & INTERLACE)		Interlace = 1;		// Interlace mode
	else								Interlace = 0;		// Not Interlace mode

	MLC_Initialize();

	MLC_RGB_SetBPP( bpp );
	MLC_RGB_SetColorKey(CkeyR, CkeyG, CkeyB);

	MLC_RGB_MixMux(RGBRegionNO, AlphaCkey, AlphaValue & 0xf);

	if(Interlace == 0)
	{
		MLC_RGB_SetScale(SrcWidth, SrcHeight, Stride, DstWidth, DstHeight);
		MLC_RGB_SetCoord(RGBRegionNO, StartX, EndX, StartY, EndY);
	}
	else	// if(Interlace == 1)
	{
		MLC_RGB_SetScale(SrcWidth, SrcHeight/2, 2*Stride, DstWidth, DstHeight/2);
		MLC_RGB_SetCoord(RGBRegionNO, StartX, EndX, StartY/2, EndY/2);
	}

	MLC_RGB_SetActivate(RGBRegionNO, MLC_RGB_RGN_ACT);
	MLC_RGB_SetAddress(addr, addr);
	MLC_RGBOn(RGBRegionNO);
}
//[*]----------------------------------------------------------------------------------------------------[*]
void init_lcd(void)
{
	tDispClkInfo ClkInfo;

    MLC_Initialize();
	DPC_Initialize();

    ClkInfo.DISPCLK_SOURCE  = 1;	// Use PLL Clock.
    ClkInfo.DISPCLK_DIVIDER = 32;	// 7;  //37;    //38;	//N=199.0656MHz / 5.23MHz = 38 @240x320 LCD	// 8;  // 25 Mhz @640x480 LCD
    ClkInfo.DISPCLK_POL = 0;     	// No Inversion..

    PMR_SetDispClk(&ClkInfo);

    DPC_InitHardware(DPC_RGB_666, CFALSE, CFALSE,CFALSE, 0, 0, DPC_USE_PLLCLK);

#if 0
    DPC_UTIL_HVSYNC_GPX320240(DPC_RGB_666, LCD_WIDTH, LCD_HEIGHT,30, 20, 38, CFALSE,4, 15, 4,CFALSE
								,CTRUE,CFALSE,CTRUE);
#endif

    DPC_UTIL_HVSYNC_GPX320240(DPC_RGB_666, LCD_WIDTH,LCD_HEIGHT,30, 20, 38, CFALSE,4, 4, 15,CFALSE
								,CTRUE,CFALSE,CTRUE);

    MLC_RGB_SetBPP(MLC_RGB_16BPP);
	MLC_RGB_SetColorKey(0xff, 0xff, 0xff);
	MLC_RGB_MixMux(MLC_RGB_RGN_1, MLC_RGB_MIXMUX_PRI, 0x8 & 0xf);
	MLC_RGB_SetScale(LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH*2, LCD_WIDTH, LCD_HEIGHT);
	MLC_RGB_SetActivate(MLC_RGB_RGN_1, MLC_RGB_RGN_ACT);
	MLC_RGB_SetCoord(MLC_RGB_RGN_1, PLANE_X_OFFSET, PLANE_X_OFFSET+PLANE_X_WIDTH, PLANE_Y_OFFSET, PLANE_Y_OFFSET+PLANE_Y_HEIGHT);
	MLC_RGB_SetAddress((unsigned long)PA_FB0_BASE, (unsigned long)PA_FB0_BASE);
	MLC_RGBOn(MLC_RGB_RGN_1);

	DPC->DPC_FPIATV1 = 0xffff;
	DPC->DPC_FPIATV2 = 0xffff;
	DPC->DPC_FPIATV3 = 0xffff;
	DPC->DPC_CNTL |= ENB;

	sdk2x_LcdSetPrintBase(PA_FB0_BASE);
	return;
}

/* Display startup logo */
void show_logo(void)
{
	int x = 0;
	int y = 0;
	int i=0;
	unsigned char r,g,b;
	unsigned char *fbaddr = (unsigned char*)PA_FB0_BASE;


	init_lcd();
#ifdef CONFIG_GP2X_SPLASH
	for(y=0; y<240; y++)
	{
		for(x=0; x<320; x++)
		{

			r = gp2xlogo[i++];
			g = gp2xlogo[i++];
			b = gp2xlogo[i++];

			fbaddr[0] = ((g & 0x1c) << 3) | ((b & 0xf8) >> 3);
			fbaddr[1] = (r & 0xf8) | (g & 0xe0) >> 5;
			fbaddr += 2;
		}
	}
#else
	/* Just blank the screen */
	for(y=0; y<240; y++)
	{
		for(x=0; x<320; x++)
		{
			*fbaddr++ = 0x00;
			*fbaddr++ = 0x00;
		}
	}

	sdk2x_LcdTextOut (100, 20, "Booting...", 0xFFFF);

	sdk2x_LcdTextOut (100, 20, "Press START for boot menu.", 0xFFFF);

#endif
}

/* Removed as we dont show a screen anymore, we give the user status info */
/*
void show_Firmware(void)
{
	int x = 0;
	int y = 0;
	int i = 0;

	// 233x98
	int img_x = (320-233)/2;
	int img_y = (240-98)/2;

	unsigned char r,g,b;
	unsigned char *fbaddr = (unsigned char*)PA_FB0_BASE;


	init_lcd();

	for(y=0; y<240; y++)
	{
		for(x=0; x<320; x++)
		{
			//imgFirmware
			if((img_x<x && x<=img_x+233) && (img_y<y && y<=img_y+98))
			{
				r = imgFirmware[i++];
				g = imgFirmware[i++];
				b = imgFirmware[i++];

				fbaddr[0] = ((g & 0x1c) << 3) | ((b & 0xf8) >> 3);
				fbaddr[1] = (r & 0xf8) | (g & 0xe0) >> 5;
				fbaddr += 2;
			}
			else
			{
				fbaddr[0] = fbaddr[1] = 0x00;
				fbaddr += 2;
			}
		}
	}
}
*/
