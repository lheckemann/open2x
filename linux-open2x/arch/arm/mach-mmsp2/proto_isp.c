/*
 * linux/arch/arm/mach-mmsp2/proto_isp.c
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>

#include <asm/arch/mmsp20.h>
#include <asm/arch/proto_isp.h>

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(fmt, args...) printk("%s: " fmt, __FUNCTION__ , ## args)
#else
#  define ds_printk(fmt, args...)
#endif

MMSP20_ISP *ISP_REG;

void
ISP_Initialize (void)
{
	ISP_REG = MMSP20_GetBase_ISP();
	ds_printk("ISP_REG = 0x%08X\n", (unsigned long)ISP_REG);
}

void
ISP_SetInputDataFormat	(U8	InputDataFormat)
{
	ISP_REG->CON_1 &= ~(ISP_DT_SEL|ISP_DT_FMT);
	ISP_REG->CON_2 &= ~ISP_CLK_SRC;

	switch(InputDataFormat) {
	// ITU-R BT 601 (4:2:2)
		case ISP_F422_GPCH:
			ISP_REG->CON_1 |= (ISP_DT_SEL_0 | ISP_DT_422);		// Data  = GPIOC[15:0], Clock = GPIOH[4].
			ISP_REG->CON_2	|= ISP_CLK_SRC_0;
			break;
	//ITU-R BT 656 Interlace
		case ISP_F656I_GPIJK:
			ISP_REG->CON_1 |= (ISP_DT_SEL_1 | ISP_DT_CCIR656I);		//Data[7:0] = { GPIOI[15:14], GPIOI[3:1], GPIOJ[15:13] }, Clock = GPIOK[2]
			ISP_REG->CON_2	|= ISP_CLK_SRC_1;
			break;
	//ITU-R BT 656 Interlace
		case ISP_F656I_GPM:
			ISP_REG->CON_1 |= (ISP_DT_SEL_2 | ISP_DT_CCIR656I);		//Data[7:0] = GPIOM[8:1].Clock = GPIOM[0].
			ISP_REG->CON_2	|= ISP_CLK_SRC_1;
			break;
	//ITU-R BT 656 Interlace
		case ISP_F656I_GPCH:
			ISP_REG->CON_1 |= (ISP_DT_SEL_0 | ISP_DT_CCIR656I);		//Data[7:0] = GPIOC[7:0]Clock = GPIOH[4]
			ISP_REG->CON_2	|= ISP_CLK_SRC_0;
			break;
	//ITU-R BT 656 Progressive
		case ISP_F656P_GPIJK:
			ISP_REG->CON_1 |= (ISP_DT_SEL_1 | ISP_DT_CCIR656P);			//Data[7:0] = { GPIOI[15:14], GPIOI[3:1], GPIOJ[15:13] }.Clock = GPIOK[2]
			ISP_REG->CON_2	|= ISP_CLK_SRC_1;
			break;
	//ITU-R BT 656 Progressive
		case ISP_F656P_GPM:
			ISP_REG->CON_1 |= (ISP_DT_SEL_2 | ISP_DT_CCIR656P);			//Data[7:0] = GPIOM[8:1].Clock = GPIOM[0].
			ISP_REG->CON_2	|= ISP_CLK_SRC_1;
			break;
	//ITU-R BT 656 Progressive
		case ISP_F656P_GPCH:
			ISP_REG->CON_1 |= (ISP_DT_SEL_0 | ISP_DT_CCIR656P);			//Data[7:0] = GPIOC[7:0]Clock = GPIOH[4]
			ISP_REG->CON_2	|= ISP_CLK_SRC_0;
			break;
	//ITU-R BT 656 Like
		case ISP_F656L_GPIJK:
			ISP_REG->CON_1 |= (ISP_DT_SEL_1 | ISP_DT_CCIR656L);			//Data[7:0] = { GPIOI[15:14], GPIOI[3:1], GPIOJ[15:13] }.Clock = GPIOK[2].
			ISP_REG->CON_2	|= ISP_CLK_SRC_1;
			break;
	//ITU-R BT 656 Like
		case ISP_F656L_GPM:
			ISP_REG->CON_1 |= (ISP_DT_SEL_2 | ISP_DT_CCIR656L);			//Data[7:0] = GPIOM[8:1].Clock = GPIOM[0].
			ISP_REG->CON_2	|= ISP_CLK_SRC_1;
			break;
	//ITU-R BT 656 Like
		case ISP_F656L_GPCH:
			ISP_REG->CON_1 |= (ISP_DT_SEL_0 | ISP_DT_CCIR656L);			//Data[7:0] = GPIOC[7:0]Clock = GPIOH[4]
			ISP_REG->CON_2	|= ISP_CLK_SRC_0;
			break;
	//External Bayer RGB
		case ISP_BAYER_GPCH:
			ISP_REG->CON_1 |= (ISP_DT_SEL_0 | ISP_DT_BAYER_DEV);			//Data[9:0] = GPIOC[9:0].Clock = GPIOH[4]
			ISP_REG->CON_2	|= ISP_CLK_SRC_0;
			break;
	//External Parallel RGB (24bit )
	//R[7:0]=  { GPIOI[15:14], GPIOI[3:1], GPIOJ[15:13] }.
	//G[7:0] =  GPIOC[15:8]
	//B[7:0] =  GPIOC[7:0]Clock = GPIOH[4].
		case ISP_PRGB_GPIJCH:
			ISP_REG->CON_1 |= (ISP_DT_SEL_0 | ISP_DT_888);
		// Clock : GPIOH[4]
			ISP_REG->CON_2	|= ISP_CLK_SRC_0;
			break;
	//External Serial RGB (10bit )
		case ISP_SRGB_GPCH:
			ISP_REG->CON_1 |= (ISP_DT_SEL_0 | ISP_DT_SERIAL);			//Data[7:0] = GPIOC[9:0],Clock = GPIOH[4].
			ISP_REG->CON_2	|= ISP_CLK_SRC_0;
			break;
	}
}

void
ISP_SetMasterMode	(U8 MasterMode)
{
	if ( MasterMode == ISP_SLAVE )
		ISP_REG->CON_2	&= ~ISP_SYNC_DIR;		// input from external sync
	else	//if ( MasterMode == ISP_MASTER )
	{
		ISP_REG->CON_2	|= ISP_SYNC_DIR;		// output internal sync
		// Clock : Divided Clock from Internal PLL Clock
		ISP_REG->CON_2	= (U16) ( ( ISP_REG->CON_2 & ~ISP_CLK_SRC ) | ISP_CLK_SRC_2 );
	}
}

void
ISP_Clock	(U8 SrcClock, U8 DestClock )
{
	ISP_REG->CON_2 &= ~(ISP_CLK2_SYS | ISP_CLK1_SYS | ISP_CLK_POL);

	if ( SrcClock == ISP_CLK_DIV_NOR )
		ISP_REG->CON_2	&= ~( ISP_CLK2_SYS | ISP_CLK1_SYS | ISP_CLK_POL );
	else if ( SrcClock == ISP_CLK_DIV_INV )
		ISP_REG->CON_2	|= ISP_CLK_POL;
	else // if ( SrcClock == ISP_CLK_SYS )
		ISP_REG->CON_2	|= ( ISP_CLK2_SYS | ISP_CLK1_SYS );
}

void
ISP_SetSync
(
			U16 GenField,
			U16 VerSyncStPnt,
			U16 HorSyncPol, U16 VerSyncPol,
			U16 HorSyncPeriod, U16 HorSyncPulse, U16 HorEAVtoSyncStart,
			U16 VerSyncPeriod, U16 VerSyncPulse,U16 VerEAVtoSyncStart
)
{
	ISP_REG->CON_2 &= ~( ISP_VSYNC_ST_POL | ISP_FLD_GEN | ISP_VSYNC_POL | ISP_HSYNC_POL );

	// HorSyncPol
	if ( HorSyncPol == ISP_SYNC_POL_INV )	ISP_REG->CON_2 |= ISP_HSYNC_POL;

	// VerSyncPol
	if ( VerSyncPol == ISP_SYNC_POL_INV )	ISP_REG->CON_2 |= ISP_VSYNC_POL;

	// GenField
	if ( GenField == ISP_FIELD_HV )	ISP_REG->CON_2 |= ISP_FLD_GEN;

	// VerSyncStPnt
	if ( VerSyncStPnt == ISP_VSYNC_FAL ) ISP_REG->CON_2 |= ISP_VSYNC_ST_POL;

	// HorSyncPeriod
	ISP_REG->HPERIOD = HorSyncPeriod;

	// HorEAVtoSyncStart[15:9], HorSyncPulse[8:0]
	ISP_REG->HPW = FInsrt(HorEAVtoSyncStart, BF_ISP_EAV2HSYNC)|FInsrt(HorSyncPulse, BF_ISP_HPW);

	// VerSyncPeriod
	ISP_REG->VPERIOD = VerSyncPeriod;

	// VerEAVtoSyncStart [15:11], VerSyncPulse [9:0]
	ISP_REG->VPW = FInsrt(VerEAVtoSyncStart, BF_ISP_EAV2VSYNC)|FInsrt(VerSyncPulse, BF_ISP_VPW);
}

void ISP_ExtDataBitWidth	(U8 ExtBitWidth )
{
	if ( ExtBitWidth == ISP_EXT_BIT_8 )	// 8 bit width
		ISP_REG->CON_1	|= ISP_EDT_WID;
	else // if ( ExtBitWidth == ISP_EXT_BIT_10 )	// 10 bit width
		ISP_REG->CON_1	&= ~ISP_EDT_WID;
}

void ISP_MemDataBitWidth	(U8 MemBitWidth )
{
	if ( MemBitWidth == ISP_MEM_BIT_8 )	// 8 bit width
		ISP_REG->CON_1	|= ISP_MEM_WID;
	else if ( MemBitWidth == ISP_MEM_BIT_16 ) 	// 16 bit width
		ISP_REG->CON_1	&= ~ISP_MEM_WID;
}

void
ISP_SetMemRGBPixelWidth	(U8 SrcRGBBPP, U16 SrcRGBPixWid,
						U8 DestRGBBPP , U16 DestRGBPixWid)
{
	// source
	if ( SrcRGBBPP == ISP_RGB_8BPP )
		ISP_REG->SRC_HWIDTH = SrcRGBPixWid;
	else if ( SrcRGBBPP == ISP_RGB_16BPP )
		ISP_REG->SRC_HWIDTH = SrcRGBPixWid * 2;
	else if ( SrcRGBBPP == ISP_RGB_24BPP )
		ISP_REG->SRC_HWIDTH = SrcRGBPixWid * 3;
	else	//if ( SrcRGBBPP == ISP_RGB_48BPP )
		ISP_REG->SRC_HWIDTH = SrcRGBPixWid * 6;

	// destination
	if ( DestRGBBPP == ISP_RGB_8BPP )
		ISP_REG->DST_HWIDTH = DestRGBPixWid;
	else if ( DestRGBBPP == ISP_RGB_16BPP )
		ISP_REG->DST_HWIDTH = DestRGBPixWid * 2;
	else if ( DestRGBBPP == ISP_RGB_24BPP )
		ISP_REG->DST_HWIDTH = DestRGBPixWid * 3;
	else	//if ( DestRGBBPP == ISP_RGB_48BPP )
		ISP_REG->DST_HWIDTH = DestRGBPixWid * 6;
}

void ISP_SetBayerRGBArray	(U8 BayerRGBArray )
{
	ISP_REG->RGBREC &= ~ISP_BAYER_ODR;
	if ( BayerRGBArray == ISP_BAY_GB )
		ISP_REG->RGBREC	|= FIRST_GB;
	else if ( BayerRGBArray == ISP_BAY_BG )
		ISP_REG->RGBREC	|= FIRST_BG;
	else if ( BayerRGBArray == ISP_BAY_RG )
		ISP_REG->RGBREC	|= FIRST_RG;
	else //if ( BayerRGBArray == ISP_BAY_GR )
		ISP_REG->RGBREC	|= FIRST_GR;
}

void
ISP_SetRGBRecovType	(U8 rRecov, U8 gRecov, U8 bRecov )
{
	ISP_REG->RGBREC &= ~(ISP_R_REC_TYPE|ISP_G_REC_TYPE|ISP_B_REC_TYPE);

	// rRecov [8]
	if ( rRecov == ISP_RECV_AVALL )
		ISP_REG->RGBREC	|= ISP_R_REC_TYPE;

	// gRecov [7:5]
	if ( gRecov == ISP_RECV_AVLR )
		ISP_REG->RGBREC	|= FInsrt(0x0, BF_ISP_G_REC_TYPE);
	else if ( gRecov == ISP_RECV_AVUD )
		ISP_REG->RGBREC	|= FInsrt(0x1, BF_ISP_G_REC_TYPE);
	else if ( gRecov == ISP_RECV_MEDIAN )
		ISP_REG->RGBREC	|= FInsrt(0x2, BF_ISP_G_REC_TYPE);
	else // if ( bRecov == ISP_RECV_AVALL )
		ISP_REG->RGBREC	|= FInsrt(0x3, BF_ISP_G_REC_TYPE);

	// bRecov [4]
	if ( bRecov == ISP_RECV_AVALL )
		ISP_REG->RGBREC	|= ISP_B_REC_TYPE;
}

void
ISP_RGBOrder	(U8 RGBOrder )
{
	ISP_REG->CON_2 &= ~ISP_RGB_ORD;

	if ( RGBOrder == ISP_ORD_RGB )
		ISP_REG->CON_2	|= ORDER_RGB;
	else if ( RGBOrder == ISP_ORD_BRG )
		ISP_REG->CON_2	|= ORDER_BRG;
	else if ( RGBOrder == ISP_ORD_GBR )
		ISP_REG->CON_2	|= ORDER_GBR;
	else //if ( RGBOrder == ISP_ORD_BRG )
		ISP_REG->CON_2	|= ORDER_BGR;

}

void
ISP_SetCrCbDeciFtOn	( U8 CrCbDeciFtOnOff)
{
	if ( CrCbDeciFtOnOff == ISP_C_DECI_ON )
		ISP_REG->CSUPSLANT	&= ~ISP_DECI_OFF;
	else // if ( CrCbDeciFtOnOff == ISP_C_DECI_OFF )
		ISP_REG->CSUPSLANT	|= ISP_DECI_OFF;
}

void ISP_SetChromaDelay (U8 ChromaDelay)
{
	if ( ChromaDelay > 4 )
		ISP_REG->CDLY	= 4;
	else
		ISP_REG->CDLY =	ChromaDelay & 0x7;
}

void
ISP_SetCbCrSwap	(U8 CbCrSwap)
{
	if ( CbCrSwap == ISP_CRCBSWAP_OFF )
		ISP_REG->RGBREC	&= ~ISP_CBRSWAP;
	else // if ( CbCrSwap == ISP_CRCBSWAP_ON )
		ISP_REG->RGBREC	|= ISP_CBRSWAP;
}

void
ISP_SetVideoRange	(U8 VideoRange)
{
	if ( VideoRange == ISP_VID_220 )
		ISP_REG->RGBREC	&= ~ISP_VID_RANGE;
	else // if ( VideoRange == ISP_VID_255 )
		ISP_REG->RGBREC	|= ISP_VID_RANGE;
}

void
ISP_UseGamma	(U8 UseGamma)
{
	if ( UseGamma == ISP_GAMMA_USE )
		ISP_REG->CON_1	&= ~ISP_TB_BYPASS;
	else //if ( UseGamma == ISP_GAMMA_BYPASS )
		ISP_REG->CON_1	|= ISP_TB_BYPASS;
}

void
ISP_SetOperMode	(U8 PathMode )
{
	if ( PathMode  == ISP_EXT_ENH_YCBCR )
		ISP_REG->CON_1	= (U16) ( ( ISP_REG->CON_1	& ~ISP_PATH ) | ISP_TO_SC_YCE );
	else if ( PathMode  == ISP_EXT_BYP_YCBCR )
		ISP_REG->CON_1	= (U16) ( ( ISP_REG->CON_1	& ~ISP_PATH ) | ISP_TO_SC );
	else if ( PathMode  == ISP_EXT_BYP_BRGB )	// Bayer RGB
		ISP_REG->CON_1	= (U16) ( ( ISP_REG->CON_1	& ~( ISP_PATH| ISP_DT_FMT ))
						| ISP_TO_MEM | ISP_DT_BAYER_DEV );
	else if ( PathMode  == ISP_EXT_BYP_SRGB )	// Serial RGB
		ISP_REG->CON_1	= (U16) ( ( ISP_REG->CON_1	& ~( ISP_PATH| ISP_DT_FMT ))
						| ISP_TO_MEM | ISP_DT_SERIAL );
	else if ( PathMode  == ISP_EXT_BYP_PRGB )	// Parallel RGB
		ISP_REG->CON_1	= (U16) ( ( ISP_REG->CON_1	& ~( ISP_PATH| ISP_DT_FMT ))
						| ISP_TO_MEM | ISP_DT_888 );
	else if ( PathMode  == ISP_MEM_ENH_YCBCR )	//
		ISP_REG->CON_1	= (U16) ( ( ISP_REG->CON_1	& ~( ISP_PATH| ISP_DT_FMT ))
						| ISP_TO_SC_YCE | ISP_DT_RGB_MEM );
	else if ( PathMode  == ISP_MEM_BYP_YCBCR )	//
		ISP_REG->CON_1	= (U16) ( ( ISP_REG->CON_1	& ~( ISP_PATH| ISP_DT_FMT ))
						| ISP_TO_SC | ISP_DT_RGB_MEM );
	else if ( PathMode  == ISP_MEM_BYP_RGB )	// Memory Bayer RGB (mem ->mem)
		ISP_REG->CON_1	= (U16) ( ( ISP_REG->CON_1	& ~( ISP_PATH| ISP_DT_FMT ))
						| ISP_TO_MEM | ISP_DT_BAYER_MEM );
}

void
ISP_EnableISP( U8 Enable )
{
	if ( Enable == CTRUE )
	{
		// Enable
		ISP_REG->CON_1	|= ISP_ON;

		// ISP Active Signal
		ISP_REG->CON_1	|= ISP_ACT;
	}
	else // if ( Enable == CFALSE )
	{
		// ISP Active Signal
		ISP_REG->CON_1	&= ~ISP_ACT;

		// Enable
		ISP_REG->CON_1	&= ~ISP_ON;

	}

}

void
ISP_StartISP	( void )
{
	ISP_REG->CON_1	= ISP_ST;		// "0"->"1"->"0
}

void
ISP_SetPreWindow
(
	U16 RGBHorStart, U16 RGBHorWidth,
	U16 RGBVerStart, U16 RGBVerHeight
)
{
	ISP_REG->RGBSV	= RGBVerStart;
	ISP_REG->RGBEV	= RGBVerStart + RGBVerHeight;
	ISP_REG->RGBSH	= RGBHorStart;
	ISP_REG->RGBEH	= RGBHorStart + RGBHorWidth;

}

void
ISP_SetAvWindow
(
	U16 AVHorStart, U16 AVHorWidth,
	U16 AVVerStart, U16 AvVerHeight
)
{
	ISP_REG->VACT_ST	= AVVerStart;
	ISP_REG->V_HEIGHT	= AvVerHeight;
	ISP_REG->HACT_ST	= AVHorStart;
	ISP_REG->H_WIDTH	= AVHorWidth;
}

void ISP_SetRGBAddress	(U32 SrcAddr, U32 DestAddr, U8 OpMode )
{
	if 	(( OpMode == ISP_EXT_BYP_BRGB ) ||			// External Device --> Memory
		(  OpMode == ISP_EXT_BYP_SRGB ) ||			// Src memory address.
		(  OpMode == ISP_EXT_BYP_PRGB ))
	{
		// destination
		ISP_REG->MEMADDRL1	= (U16)(DestAddr & 0xffff);
		ISP_REG->MEMADDRH1	= (U16)((DestAddr>>16) & 0xffff);
	}
	else //if 	( OpMode == ISP_MEM_BYP_RGB )		// Memory --> Memory
	{
		// destination
		ISP_REG->MEMADDRL2	= (U16)(SrcAddr & 0xffff);
		ISP_REG->MEMADDRH2	= (U16)((SrcAddr>>16) & 0xffff);

		// source
		ISP_REG->MEMADDRL1	= (U16)(DestAddr & 0xffff);
		ISP_REG->MEMADDRH1	= (U16)((DestAddr>>16) & 0xffff);
	}

}

void
ISP_SetInterruptEnable	(U8 InterruptEnable)
{

	// Vsync and Hsync Interrupt Enable
	if ( InterruptEnable == (ISP_INT_VSYNC | ISP_INT_HSYNC) )
		ISP_REG->INT_CON	|= ( ISP_VINT_EN | ISP_HINT_EN );
	// Vsync Interrupt Enable & HSync Interrupt Disable
	else if ( InterruptEnable == ISP_INT_VSYNC )
		ISP_REG->INT_CON	|= ISP_VINT_EN;
	// Hsync Interrupt Enable and Vsync Interrupt Disable
	else if ( InterruptEnable == ISP_INT_HSYNC )
		ISP_REG->INT_CON	|= ISP_HINT_EN;
	// Interrupt Disable
	else
		ISP_REG->INT_CON	&= ~( ISP_VINT_EN | ISP_HINT_EN );
}

void
ISP_SetInterruptType	(U8 hInterruptType, U8 vInterruptType)
{
	// Vsync Interrupt Type
	if ( hInterruptType == ISP_INT_SYNC_FAL )
		ISP_REG->INT_CON	&= ~ISP_VINT_TYPE;
	else //if ( hInterruptType == ISP_INT_SYNC_RIS )
		ISP_REG->INT_CON	|= ISP_VINT_TYPE;

	// Hsync Interrupt Type
	if ( vInterruptType == ISP_INT_SYNC_FAL )
		ISP_REG->INT_CON	&= ~ISP_HINT_TYPE;
	else //if ( vInterruptType == ISP_INT_SYNC_RIS )
		ISP_REG->INT_CON	|= ISP_HINT_TYPE;
}

#ifdef DS_DEBUG
void ISP_Util(ISP_INFO *Parm)
{
	ISP_Initialize ();

	// External Device -> Scale Processor(YCbCr)
	ISP_SetOperMode	( Parm->PathMode );

	ISP_SetInputDataFormat	( Parm->InputDataFormat );
	ISP_SetMasterMode		( Parm->MasterMode );
	ISP_Clock				( Parm->SrcClock, Parm->DestClock );
	ISP_SetSync
	(
		Parm->GenField,
		Parm->VerSyncStPnt,
		Parm->HorSyncPol, Parm->VerSyncPol,
		Parm->HorSyncPeriod, Parm->HorSyncPulse, Parm->HorEAVtoSyncStart,
		Parm->VerSyncPeriod, Parm->VerSyncPulse, Parm->VerEAVtoSyncStart
	);

	ISP_SetPreWindow
	(
		Parm->PreStartX, Parm->PreWidth,
		Parm->PreStartY, Parm->PreHeight
	);
	ISP_SetAvWindow
	(
		Parm->AvStartX, Parm->AvWidth,
		Parm->AvStartY, Parm->AvHeight
	);

	ISP_ExtDataBitWidth		( Parm->ExtBitWidth );
	ISP_MemDataBitWidth		( Parm->MemBitWidth );
	ISP_SetMemRGBPixelWidth
	(
		Parm->SrcRGBBPP, Parm->SrcRGBPixWid,
		Parm->DestRGBBPP , Parm->DestRGBPixWid
	);
	ISP_SetBayerRGBArray	( Parm->BayerRGBArray );
	ISP_SetRGBRecovType		( Parm->rRecov, Parm->gRecov, Parm->bRecov );
	ISP_RGBOrder			( Parm->RGBOrder );
	ISP_SetCrCbDeciFtOn		( Parm->CrCbDeciFtOnOff );
	ISP_SetChromaDelay		( Parm->ChromaDelay );
	ISP_SetCbCrSwap			( Parm->CbCrSwap );
	ISP_SetVideoRange		( Parm->VideoRange );
	ISP_UseGamma			( Parm->UseGamma );
	ISP_SetInterruptType	( Parm->hInterruptType, Parm->vInterruptType );

	ISP_EnableISP(ISP_ENABLE);

	ds_printk("CON_1 = 0x%04X, ", ISP_REG->CON_1);
	ds_printk("CON_2 = 0x%04X, ", ISP_REG->CON_2);
	ds_printk("HPERIOD = 0x%04X, ", ISP_REG->HPERIOD);
	ds_printk("HPW = 0x%04X, ", ISP_REG->HPW);
	ds_printk("VPERIOD = 0x%04X, ", ISP_REG->VPERIOD);
	ds_printk("VPW = 0x%04X, ", ISP_REG->VPW);
	ds_printk("RGBSV = 0x%04X, ", ISP_REG->RGBSV);
	ds_printk("RGBEV = 0x%04X, ", ISP_REG->RGBEV);
	ds_printk("RGBSH = 0x%04X, ", ISP_REG->RGBSH);
	ds_printk("RGBEV = 0x%04X, ", ISP_REG->RGBEH);
	ds_printk("VACT_ST = 0x%04X, ", ISP_REG->VACT_ST);
	ds_printk("V_HEIGHT = 0x%04X, ", ISP_REG->V_HEIGHT);
	ds_printk("HACT_ST = 0x%04X, ", ISP_REG->HACT_ST);
	ds_printk("H_WIDTH = 0x%04X, ", ISP_REG->H_WIDTH);
	ds_printk("SRC_HWIDTH = 0x%04X, ", ISP_REG->SRC_HWIDTH);
	ds_printk("DST_HWIDTH = 0x%04X, ", ISP_REG->DST_HWIDTH);
	ds_printk("RGBREC = 0x%04X, ", ISP_REG->RGBREC);
	ds_printk("CSUPSLANT = 0x%04X, ", ISP_REG->CSUPSLANT);
	ds_printk("CDLY = 0x%04X, ", ISP_REG->CDLY);
	ds_printk("INT_CON = 0x%04X", ISP_REG->INT_CON);
}
#endif // DS_DEBUG

EXPORT_SYMBOL(ISP_SetInterruptEnable);
EXPORT_SYMBOL(ISP_SetInputDataFormat);
EXPORT_SYMBOL(ISP_SetOperMode);
EXPORT_SYMBOL(ISP_SetSync);
EXPORT_SYMBOL(ISP_SetPreWindow);
EXPORT_SYMBOL(ISP_SetAvWindow);
EXPORT_SYMBOL(ISP_EnableISP);
EXPORT_SYMBOL(ISP_StartISP);
EXPORT_SYMBOL(ISP_Initialize);
EXPORT_SYMBOL(ISP_SetMasterMode);
EXPORT_SYMBOL(ISP_Clock);
EXPORT_SYMBOL(ISP_ExtDataBitWidth);
EXPORT_SYMBOL(ISP_MemDataBitWidth);
EXPORT_SYMBOL(ISP_SetMemRGBPixelWidth);
EXPORT_SYMBOL(ISP_SetBayerRGBArray);
EXPORT_SYMBOL(ISP_SetRGBRecovType);
EXPORT_SYMBOL(ISP_RGBOrder);
EXPORT_SYMBOL(ISP_SetCrCbDeciFtOn);
EXPORT_SYMBOL(ISP_SetChromaDelay);
EXPORT_SYMBOL(ISP_SetCbCrSwap);
EXPORT_SYMBOL(ISP_SetVideoRange);
EXPORT_SYMBOL(ISP_UseGamma);
EXPORT_SYMBOL(ISP_SetInterruptType);
EXPORT_SYMBOL(ISP_SetRGBAddress);
