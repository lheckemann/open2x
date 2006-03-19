/*
 * linux/arch/arm/mach-mmsp2/proto_scale.c
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
#include <asm/arch/proto_scale.h>

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(fmt, args...) printk("%s: " fmt, __FUNCTION__ , ## args)
#else
#  define ds_printk(fmt, args...)
#endif

MMSP20_SC *SC_REG;

void SC_Initialize (void)
{
        SC_REG = MMSP20_GetBase_SC();
        ds_printk("SC_REG = 0x%08X\n", (unsigned long)SC_REG);
}

void SC_InitHardware
(
	U8 LumaDelay, U8 ChromaDelay, U8 MemRdWordSz, U8 Occurance
)
{
	SC_REG->SC_DELAY = FInsrt(LumaDelay, BF_LUMA_DELAY)|FInsrt(ChromaDelay, BF_CHROMA_DELAY);
	SC_REG->SC_MEM_CNTR = ( SC_REG->SC_MEM_CNTR & ( ~(SC_OCCURANCE | SC_MEM_RD_SZ) ) )
				| FInsrt(Occurance, BF_SC_OCCURANCE)
				| FInsrt(MemRdWordSz, BF_SC_MEM_RD_SZ);
	SC_REG->SC_STATUS &= ~SC_FIELD_POL;
	SC_REG->SC_STATUS = ( SC_REG->SC_STATUS & ( ~(0x3<<11) ) ) |  0x2<<11;
}

void SC_SetFieldPolarity
(
	U8 FieldPol
)
{
	if ( FieldPol == SC_FID_NORM )
		SC_REG->SC_STATUS &= ~SC_FIELD_POL;
	else	//( FieldPol == SC_FID_INV )	// [9] <= 1
		SC_REG->SC_STATUS |= SC_FIELD_POL;
}

void SC_SelDataPath ( U8 SelSource, U8 SelDest)
{
	SC_REG->SC_CMD &= ~(SC_SRC | SC_DEST);
	// source
	if ( SelSource == SC_FRM_MEM )
		SC_REG->SC_CMD	|= SC_SRC_MEM;
	else if ( SelSource == SC_FRM_MLC )
		SC_REG->SC_CMD |= SC_SRC_DISP;
	else if ( SelSource == SC_FRM_ISP )
		SC_REG->SC_CMD |= SC_SRC_ISP;
	else	// if ( SelSource == SC_FRM_FDC )
		SC_REG->SC_CMD	|= SC_SRC_FMT;

	// Destination
	if ( SelDest == SC_TO_MEM )
		SC_REG->SC_CMD	|= SC_DEST_MEM;
	else	// if ( SelDest == SC_TO_MLC )
		SC_REG->SC_CMD	|= SC_DEST_DISP;
}

void SC_SetOperMode (U8 NumOfFrame, U8 WriteForm, U8 WriteField )
{
	SC_REG->SC_CMD &= ~(SC_ONE_TIME | SC_F422 | SC_WR_TYPE);

	// NumOfFrame Setting
	if ( NumOfFrame == SC_1_TIME ) // [8] <= 1
		SC_REG->SC_CMD |= SC_ONE_TIME;

	// WriteForm Setting
	if ( WriteForm != SC_YCBYCR )
		SC_REG->SC_CMD |= SC_WR_TYPE;

	if ( WriteForm == SC_SEP_F422 )
		SC_REG->SC_CMD |= SC_F422;

	//WriteField Setting
	if ( WriteField == SC_WR_FIELD )				// [10] <= 1
		SC_REG->SC_STATUS |= SC_FIELD;
	else	// ( WriteField == SC_WR_FRAME )		// [10] <= 0
		SC_REG->SC_STATUS &= ~SC_FIELD;
}

#include <linux/delay.h>

void SC_EnableScale (CBOOL Enable)
{
	ds_printk("SC_REG->SC_CMD = %x \n",SC_REG->SC_CMD);
	ds_printk("SC_REG->SC_STATUS = %x \n",SC_REG->SC_STATUS);

#if 1		// 050928 hhsong modify later 0 mmsp2_test application change
	if ( Enable == CFALSE )
	{
		if( ( (SC_REG->SC_CMD & SC_SRC) == SC_SRC_MEM ||		// If source is memory
		  (SC_REG->SC_CMD & SC_DEST) == SC_DEST_MEM ) &&		// If dest is memory
		  (SC_REG->SC_CMD & SC_ENABLE))		            		// If Scaler is run
//		  (SC_REG->SC_CMD & SC_ENABLE) &&
//		  (!(SC_REG->SC_CMD & SC_ONE_TIME)))
		{
			ds_printk("SC_REG->SC_CMD = %x \n",SC_REG->SC_CMD);
			// One Time Write
			SC_REG->SC_CMD |= SC_ONE_TIME;
			//Interrupt Disable == SC_INT_SC
			SC_REG->SC_IRQ	&= ~SC_IRQ_EN;
			SC_REG->SC_CMD &= ~SC_ENABLE;
//			printk("CFALSE , SC_FOREVER\n");
		}
		else
		{
			SC_REG->SC_CMD &= ~SC_ENABLE;
//			SC_REG->SC_CMD = (unsigned short)0x0100;
//			printk("CFALSE , SC_ONE_TIME\n");
		}
	}
	else // if ( Enable == SC_ENABLE )
	{
		SC_REG->SC_CMD |= SC_ENABLE;
	}
#else
	if( Enable )
		SC_REG->SC_CMD |= SC_ENABLE;
	else
	{
//		unsigned short cmd;
		int errorcount = 0;

//		cmd = SC_REG->SC_CMD;

		if(SC_REG->SC_CMD & SC_ENABLE)
		{
			// Restore CMD register as old value except a ENABLE bit
			if(!(SC_REG->SC_CMD & SC_ONE_TIME))	// hhsong add.
				SC_REG->SC_CMD |= SC_ONE_TIME;

//			cmd &= 0x9fff;
//			SC_REG->SC_CMD = cmd;

			while(SC_REG->SC_STATUS & SC_BUSY)
			{
				if(errorcount++ > 330) // 33ms Oooops.. for 30fps mpeg decoding.. hhsong 050929
				{
					printk("Can't stop SCALER!!\n");
					break;
				}
				udelay(100);
//				SC_REG->SC_CMD = cmd;
			}
			SC_REG->SC_CMD &= ~SC_ENABLE;
//				printk("SC_REG->SC_CMD = 0x%x\n",SC_REG->SC_CMD);
//				printk("errorcount = %d!!\n",errorcount);
		}
	}
#endif
}

void SC_StartScale ( unsigned char flipmode )
{
	if( flipmode == SC_RUN )
		SC_REG->SC_CMD |= SC_START;
	else if ( flipmode == SC_STARTMEM)
		SC_REG->SC_CMD |= SC_START | SC_ENABLE | SC_DEST_MEM;
	else if (flipmode == SC_STARTDIS)
		SC_REG->SC_CMD |= SC_START | SC_ENABLE | SC_DEST_DISP;
}

void SC_StopScale ( void )
{
        SC_EnableScale(CFALSE);
}

void SC_StartDering ( void )
{
	SC_REG->SC_CMD |= SC_DERING;
}

void SC_StopDering ( void )
{
	SC_REG->SC_CMD &= ~SC_DERING;
}

CBOOL SC_IsBusy (void)
{
	ds_printk("SC_REG->SC_STATUS = %x \n",SC_REG->SC_STATUS);
	ds_printk("SC_REG->SC_CMD = %x \n",SC_REG->SC_CMD);
	if ( SC_REG->SC_STATUS & SC_BUSY )
	{
	ds_printk("SC_REG->SC_STATUS = %x \n",SC_REG->SC_STATUS);
		return	CTRUE;
	}
	else
	{
	ds_printk("SC_REG->SC_STATUS = %x \n",SC_REG->SC_STATUS);
		return	CFALSE;
	}
}

CBOOL SC_IsIspOddField (void)
{

	if ( SC_REG->SC_STATUS & SC_ISP_FIELD )
		return CTRUE;
	else
		return CFALSE;

}

CBOOL SC_IsDispOddField (void)
{

	if ( SC_REG->SC_STATUS & SC_DISP_FIELD )
		return CTRUE;
	else
		return CFALSE;
}

void SC_SetSource (U32 SrcOddAddr, U32 SrcEvenAddr, U16 Stride)
{
	SC_REG->SC_SRC_EVEN_ADDRL = (U16)(SrcEvenAddr & 0xffff);
	SC_REG->SC_SRC_EVEN_ADDRH = (U16)(SrcEvenAddr >> 16);

	SC_REG->SC_SRC_ODD_ADDRL = (U16)(SrcOddAddr & 0xffff);
	SC_REG->SC_SRC_ODD_ADDRH = (U16)(SrcOddAddr >> 16);

	SC_REG->SC_SRC_PXL_WIDTH = (U16)((Stride>> 1) - 1);
}

void SC_SetDest1D (U32 DstAddr, U16 Stride)
{
	SC_REG->SC_DST_ADDRL = (U16)(DstAddr & 0xffff);
	SC_REG->SC_DST_ADDRH = (U16)(DstAddr >> 16);

	SC_REG->SC_DST_WPXL_WIDTH = (U16)((Stride >> 1) );
}

void SC_SetDest2D	(U16 LumaOffset, U16 CbOffset, U16 CrOffset)
{
	SC_REG->SC_LUMA_OFFSET = LumaOffset;
	SC_REG->SC_CB_OFFSET = CbOffset;
	SC_REG->SC_CR_OFFSET = CrOffset;
}

void SC_SetScale	(U16 SrcWidth, U16 SrcHeight,
			U16 DestWidth, U16 DestHeight)
{
	U16	SetSrcWidth;
	U16	SetDestWidth;
	U32	vScale;
	U32	hScale;

	// Make Even Data
	if ( SrcWidth & 0x1 )			// odd
		SetSrcWidth	= SrcWidth - 1;
	else							// even
		SetSrcWidth	= SrcWidth;

	if ( DestWidth & 0x1 )			// odd
		SetDestWidth	= DestWidth - 1;
	else							// even
		SetDestWidth	= DestWidth;

	SC_REG->SC_SRC_PXL_HEIGHT	= SrcHeight;
	SC_REG->SC_DST_PXL_WIDTH	= SetDestWidth - 1;
	SC_REG->SC_DST_PXL_HEIGHT	= DestHeight;//- 1;

	//----------------------------------------------------------------------------
	// Horizontal Scale
	//----------------------------------------------------------------------------
	if ( ( SrcWidth == DestWidth ) && ( SrcHeight ==  DestHeight ) )	// Simple Buffer Mode
	{
		SC_REG->SC_CMD &= ~SC_PRESCLE;	// PreScale : off
		SC_REG->SC_CMD &= ~SC_POSTSCLE;         // PostScale : off
		SC_REG->SC_SRC_PXL_REQCNT 	= SetSrcWidth - 1;
		SC_REG->SC_PRE_HRATIO	= 0x1000;	// Not PreScale
		hScale	= 0x4000;				// Not PostScale
	}
	else if ( SrcWidth > DestWidth )								// Horizontal Scale Down
	{
		if ( SrcWidth > 1024 )
		{
			SC_REG->SC_CMD |= SC_PRESCLE;	// PreScale : On
			SC_REG->SC_CMD |= SC_POSTSCLE;	// PostScale : on
			SC_REG->SC_PRE_HRATIO = 0x400 * 2;	// Scale Down by 2

			if ( (SrcWidth>>1) > DestWidth )						// Horizontal Scale Down
			{
				SC_REG->SC_CMD &= ~SC_POSTHSCLE;	// Post: Horizontal Scale Down
				SC_REG->SC_SRC_PXL_REQCNT 	= (SrcWidth>>1) - 1;
				hScale	= ( 0x4000 * (SrcWidth>>1) ) / DestWidth;
			}
			else													// Horizontal Scale Up
			{
				SC_REG->SC_CMD |= SC_POSTHSCLE;	// Post: Horizontal Scale UP
				SC_REG->SC_SRC_PXL_REQCNT 	= (SrcWidth>>1) - 1;
				hScale	= ( 0x4000 * (SrcWidth>>1) ) / DestWidth;
			}
		}
		else														// Horizontal Scale Down
		{
			SC_REG->SC_CMD &= ~SC_PRESCLE;	// PreScale : off
			SC_REG->SC_CMD |= SC_POSTSCLE;	// PostScale : on
			SC_REG->SC_CMD &= ~SC_POSTHSCLE;	// Post: Horizontal Scale Down
			SC_REG->SC_SRC_PXL_REQCNT 	= SetSrcWidth - 1;
			SC_REG->SC_PRE_HRATIO	= 0x400;	// Not PreScale
			hScale	= ( 0x4000 * SrcWidth ) / DestWidth;
		}
	}
	else	// ( SrcWidth < DestWidth ) 							// HorizontalScale Up
	{
		SC_REG->SC_CMD &= ~SC_PRESCLE;	// PreScale : off
		SC_REG->SC_CMD |= SC_POSTSCLE;	// PostScale : on
		SC_REG->SC_CMD |= SC_POSTHSCLE;	// Post: Horizontal Scale UP
		SC_REG->SC_SRC_PXL_REQCNT 	= SetSrcWidth - 1;
		SC_REG->SC_PRE_HRATIO	= 0x400;	// Not PreScale
		hScale	= ( 0x4000 * SrcWidth ) / DestWidth;
	}
	SC_REG->SC_POST_HRATIOL	= (U16)(hScale & 0xffff);
	SC_REG->SC_POST_HRATIOH	= (U16)(hScale >> 16);

	//----------------------------------------------------------------------------
	// Vertical Scale
	//----------------------------------------------------------------------------
	if ( ( SrcWidth == DestWidth ) && ( SrcHeight ==  DestHeight ) )	// Simple Buffer Mode
	{
	}
	else if ( SrcHeight > DestHeight )								// Vertical Scale Down
	{
		SC_REG->SC_CMD &= ~SC_POSTVSCLE;	// Post Vertical Down
	}
	else	// ( SrcWidth < DestWidth ) 							// Vertical Scale Up
	{
		SC_REG->SC_CMD |= SC_POSTVSCLE;	// Post Vertical UP
	}

	vScale	= ( 0x4000 * SrcHeight ) / (DestHeight + 1 );

	SC_REG->SC_PRE_VRATIO	= 0x400;	// Not PreScale
	SC_REG->SC_POST_VRATIOL = (U16)(vScale & 0xffff);
	SC_REG->SC_POST_VRATIOH = (U16)(vScale >> 16);
}

void SC_SetCoarseDownScale
(
	U16 SrcWidth, U16 SrcHeight,
	U16 DestWidth, U16 DestHeight
)
{
	U16	SetSrcWidth;
	U16	SetDestWidth;
	U32	vScale;
	U32	hScale;

	// Make Even Data
	if ( SrcWidth & 0x1 )			// odd
		SetSrcWidth	= SrcWidth - 1;
	else							// even
		SetSrcWidth	= SrcWidth;

	if ( DestWidth & 0x1 )			// odd
		SetDestWidth	= DestWidth - 1;
	else							// even
		SetDestWidth	= DestWidth;


	SC_REG->SC_SRC_PXL_HEIGHT	= SrcHeight;
	SC_REG->SC_SRC_PXL_REQCNT 	= SetDestWidth - 1;
	SC_REG->SC_DST_PXL_WIDTH	= SetDestWidth - 1;
	SC_REG->SC_DST_PXL_HEIGHT	= DestHeight;//- 1;

	SC_REG->SC_CMD &= ~SC_POSTSCLE;	// PostScale : off
	SC_REG->SC_CMD |= SC_PRESCLE;	        // PreScale : On

	//----------------------------------------------------------------------------
	// Horizontal Scale
	//----------------------------------------------------------------------------
	hScale	= ( 0x400 * SrcWidth ) / DestWidth;
	SC_REG->SC_PRE_HRATIO	= (U16)hScale;

	//----------------------------------------------------------------------------
	// Vertical Scale
	//----------------------------------------------------------------------------
	vScale	= ( 0x400 * SrcHeight ) / (DestHeight);
	SC_REG->SC_PRE_VRATIO	= (U16)vScale;	// Not PreScale
}

void SC_SetMirror	(U8 hSrcMirror, U8 vSrcMirror,
				U8 hDestMirror, U8 vDestMirror)
{
	// Horizontal Source Mirror
	if ( hSrcMirror == SC_MIRROR_OFF )
		SC_REG->SC_MIRROR &= ~SC_SRC_HMRR;
	else	// if ( hSrcMirror == SC_MIRROR_ON )
		SC_REG->SC_MIRROR |= SC_SRC_HMRR;

	// Vertical Source Mirror
	if ( vSrcMirror == SC_MIRROR_OFF )
		SC_REG->SC_MIRROR &= ~SC_SRC_VMRR;
	else	// if ( vSrcMirror == SC_MIRROR_ON )
		SC_REG->SC_MIRROR |= SC_SRC_VMRR;

	// Horizontal Destination Mirror
	if ( hDestMirror == SC_MIRROR_OFF )
		SC_REG->SC_MIRROR &= ~SC_DST_HMRR;
	else	// if ( hDestMirror == SC_MIRROR_ON )
		SC_REG->SC_MIRROR |= SC_DST_HMRR;

	// Vertical Destination Mirror
	if ( vDestMirror == SC_MIRROR_OFF )
		SC_REG->SC_MIRROR &= ~SC_DST_VMRR;
	else	// if ( vDestMirror == SC_MIRROR_ON )
		SC_REG->SC_MIRROR |= SC_DST_VMRR;
}

void SC_SetInterruptEnable	(U8 InterruptEnable)
{

	// Scale and FDC Interrupt Enable
	if ( InterruptEnable == (SC_INT_SC | SC_INT_FDC) )
		SC_REG->SC_IRQ	|= (SC_IRQ_EN|FC_IRQ_EN);
	// Scale Interrupt Enable & HSync Interrupt Disable
	else if ( InterruptEnable == SC_INT_SC )
		SC_REG->SC_IRQ	= ( SC_REG->SC_IRQ & ~FC_IRQ_EN ) | SC_IRQ_EN;
	// FDC Interrupt Enable and Scale Interrupt Disable
	else if ( InterruptEnable == SC_INT_FDC )
		SC_REG->SC_IRQ	= ( SC_REG->SC_IRQ & ~SC_IRQ_EN ) | FC_IRQ_EN;
	// Interrupt Disable
	else
		SC_REG->SC_IRQ	= SC_REG->SC_IRQ & ~( SC_IRQ_EN | FC_IRQ_EN );
}

U8 SC_GetInterruptEnable	(void)
{
	U16	TmpReg_3_1;

	TmpReg_3_1	= SC_REG->SC_IRQ & ( SC_IRQ_EN | FC_IRQ_EN );
	TmpReg_3_1	= ( (TmpReg_3_1 >> 2) & (0x1<<1) ) | ( (TmpReg_3_1 >> 1)& (0x1<<0) );

	// Vsync and Hsync Interrupt Enable
	if ( TmpReg_3_1 == (SC_INT_SC | SC_INT_FDC) )
		return	(SC_INT_SC | SC_INT_FDC);
	// Vsync Interrupt Enable & HSync Interrupt Disable
	else if ( TmpReg_3_1 == SC_INT_SC )
		return	SC_INT_SC;
	// Hsync Interrupt Enable and Vsync Interrupt Disable
	else if ( TmpReg_3_1 == SC_INT_FDC )
		return	SC_INT_FDC;
	// Interrupt Disable
	else
		return	SC_INT_NONE;
}

U8 SC_GetInterruptPend	(void)
{
	U16	TmpReg_2_0;

	TmpReg_2_0	= SC_REG->SC_IRQ & ( SC_IRQ_CLEAR | FC_IRQ_CLEAR );
	TmpReg_2_0	= ( (TmpReg_2_0 >> 1) & (0x1<<1) ) | ( TmpReg_2_0 & 0x1 );

	// Vsync and Hsync Interrupt pending
	if ( TmpReg_2_0 == (SC_INT_SC | SC_INT_FDC) )
		return	(SC_INT_SC | SC_INT_FDC);
	// Vsync Interrupt pending
	else if ( TmpReg_2_0 == SC_INT_SC )
		return	SC_INT_SC;
	// Hsync Interrupt pending
	else if ( TmpReg_2_0 == SC_INT_FDC )
		return	SC_INT_FDC;
	//
	else
		return	SC_INT_NONE;
}

void SC_ClrInterruptPend	(U8 ClrPend)
{

	// Scale and FDC Interrupt Enable
	if ( ClrPend == (SC_INT_SC | SC_INT_FDC) )
		SC_REG->SC_IRQ	 |= ( SC_IRQ_CLEAR | FC_IRQ_CLEAR );
	// Scale Interrupt Enable & HSync Interrupt Disable
	else if ( ClrPend == SC_INT_SC )
		SC_REG->SC_IRQ	|= SC_IRQ_CLEAR;
	// FDC Interrupt Enable and Scale Interrupt Disable
	else if ( ClrPend == SC_INT_FDC )
		SC_REG->SC_IRQ	|= FC_IRQ_CLEAR;
	// Interrupt Disable
}

void SC_Util
(
	U8 SelSource,	// SC_FRM_MEM, SC_FRM_MLC, SC_FRM_ISP
	U8 SelDest,		// SC_TO_MEM, SC_TO_MLC

	U8 NumOfFrame,	// SC_ONE_TIME, SC_FOREVER
	U8 WriteForm,	// SC_YCBYCR,  SC_SEP_F422, SC_SEP_F420
	U8 WriteField,	// SC_WR_FIELD, SC_WR_FRAME

	U8 hSrcMirror, U8 vSrcMirror, U8 hDestMirror, U8 vDestMirror,

	U16 SrcWidth, U16 SrcHeight, U16 SrcStride,
	U16 DestWidth, U16 DestHeight, U16 DstStride,

	U32 SrcOddAddr, U32 SrcEvenAddr,
	U32 DstAddr,
	U16 LumaOffset, U16 CbOffset, U16 CrOffset
)
{
	U16	DstWidhtMod, DstHeightMod;

	SC_Initialize();
	SC_InitHardware	(3, 1, 16, 0);

	SC_SelDataPath ( SelSource, SelDest);

	SC_SetOperMode (NumOfFrame, WriteForm, WriteField );

	if ( WriteForm != SC_YCBYCR )	// Write 2 Dimension
	{
		DstWidhtMod		= ( DestWidth + 15 ) & 0xfff0;
		DstHeightMod	= ( DestHeight + 1 ) & 0xfffe;
	}
	else	// Write 1 Dimension
	{
		DstWidhtMod		= DestWidth;
		DstHeightMod	= DestHeight;
	}

	SC_SetScale
	(
		SrcWidth, SrcHeight,	// source
		DstWidhtMod, DstHeightMod	// destination
	);

	SC_SetMirror
	(
		hSrcMirror, vSrcMirror, hDestMirror, vDestMirror
	);

	SC_SetSource (SrcOddAddr, SrcEvenAddr, SrcStride);

	SC_SetDest1D (DstAddr, DstStride);

	SC_SetDest2D (LumaOffset, CbOffset, CrOffset);

//	SC_EnableScale(CTRUE);

	ds_printk("SC_CMD = 0x%04X,\n ", SC_REG->SC_CMD);
	ds_printk("SC_STATUS = 0x%04X,\n ", SC_REG->SC_STATUS);
	ds_printk("SC_MIRROR = 0x%04X,\n ", SC_REG->SC_MIRROR);
	ds_printk("SC_PRE_VRATIO = 0x%04X,\n ", SC_REG->SC_PRE_VRATIO);
	ds_printk("SC_PRE_HRATIO = 0x%04X,\n ", SC_REG->SC_PRE_HRATIO);
	ds_printk("SC_POST_VRATIOL = 0x%04X,\n ", SC_REG->SC_POST_VRATIOL);
	ds_printk("SC_POST_VRATIOH = 0x%04X,\n ", SC_REG->SC_POST_VRATIOH);
	ds_printk("SC_POST_HRATIOL = 0x%04X,\n ", SC_REG->SC_POST_HRATIOL);
	ds_printk("SC_POST_HRATIOH = 0x%04X,\n ", SC_REG->SC_POST_HRATIOH);
	ds_printk("SC_SRC_ODD_ADDRL = 0x%04X,\n ", SC_REG->SC_SRC_ODD_ADDRL);
	ds_printk("SC_SRC_ODD_ADDRH = 0x%04X,\n ", SC_REG->SC_SRC_ODD_ADDRH);
	ds_printk("SC_SRC_EVEN_ADDRL = 0x%04X,\n ", SC_REG->SC_SRC_EVEN_ADDRL);
	ds_printk("SC_SRC_EVEN_ADDRH = 0x%04X,\n ", SC_REG->SC_SRC_EVEN_ADDRH);
	ds_printk("SC_DST_ADDRL = 0x%04X,\n ", SC_REG->SC_DST_ADDRL);
	ds_printk("SC_DST_ADDRH = 0x%04X,\n ", SC_REG->SC_DST_ADDRH);

	ds_printk("SC_SRC_PXL_HEIGHT = 0x%04X,\n ", SC_REG->SC_SRC_PXL_HEIGHT);
	ds_printk("SC_SRC_PXL_WIDTH = 0x%04X,\n ", SC_REG->SC_SRC_PXL_WIDTH);
	ds_printk("SC_SRC_PXL_REQCNT = 0x%04X,\n ", SC_REG->SC_SRC_PXL_REQCNT);
	ds_printk("SC_DST_PXL_HEIGHT = 0x%04X,\n ", SC_REG->SC_DST_PXL_HEIGHT);
	ds_printk("SC_DST_PXL_WIDTH = 0x%04X,\n ", SC_REG->SC_DST_PXL_WIDTH);
	ds_printk("SC_DST_WPXL_WIDTH = 0x%04X,\n ", SC_REG->SC_DST_WPXL_WIDTH);
	ds_printk("SC_LUMA_OFFSET = 0x%04X,\n ", SC_REG->SC_LUMA_OFFSET);
	ds_printk("SC_CB_OFFSET = 0x%04X,\n ", SC_REG->SC_CB_OFFSET);
	ds_printk("SC_CR_OFFSET = 0x%04X,\n ", SC_REG->SC_CR_OFFSET);
	ds_printk("SC_DELAY = 0x%04X,\n ", SC_REG->SC_DELAY);
	ds_printk("SC_MEM_CNTR = 0x%04X\n", SC_REG->SC_MEM_CNTR);

}

EXPORT_SYMBOL(SC_Util);
EXPORT_SYMBOL(SC_EnableScale);
EXPORT_SYMBOL(SC_StartScale);
EXPORT_SYMBOL(SC_StopScale);
EXPORT_SYMBOL(SC_StartDering);
EXPORT_SYMBOL(SC_StopDering);
EXPORT_SYMBOL(SC_IsBusy);
EXPORT_SYMBOL(SC_SetFieldPolarity);
