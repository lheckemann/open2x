/*
 * linux/arch/arm/mach-mmsp2/mdk.c
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
#include <asm/arch/proto_fdc.h>

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(fmt, args...) printk("%s: " fmt, __FUNCTION__, ## args)
#else
#  define ds_printk(fmt, args...)
#endif

//Register Declaration..
MMSP20_FDC *FDC_REG;

void FDC_Initialize(void)
{
	FDC_REG = MMSP20_GetBase_FDC();
}

void FDC_InitHardware(void)
{
	FDC_REG->FDC_CNTL = 0;
}

void FDC_Close(void)
{
	FDC_SetVSyncDisable();
}

void FDC_CloseHardware(void)
{
	FDC_SetVSyncDisable();
}

void FDC_CfgSrcFrame(
	U16	IsFIELD,
	U16 	LumaOffset,
	U16 	CbOffset,
	U16 	CrOffset
)
{
	FDC_REG->FDC_CNTL = (IsFIELD==FDC_FIELD) ? FDC_REG->FDC_CNTL | FDC_SFF : FDC_REG->FDC_CNTL & ~FDC_SFF;
	FDC_REG->FDC_LUMA_OFFSET = LumaOffset;
	FDC_REG->FDC_CB_OFFSET   = CbOffset;
	FDC_REG->FDC_CR_OFFSET   = CrOffset;
}

void FDC_CfgDstFrame(
	U16 IsToScaler,
	U16	IsFIELD,
	U32	DestBaseAddr
)
{
	if(IsToScaler)
	{
		FDC_REG->FDC_CNTL |= FDC_WRMOD;
		FDC_REG->FDC_CNTL = (IsFIELD==FDC_FIELD) ? FDC_REG->FDC_CNTL |FDC_DFF : FDC_REG->FDC_CNTL & ~FDC_DFF;
	}
	else
	{
		FDC_REG->FDC_CNTL &= ~FDC_WRMOD;
		FDC_REG->FDC_CNTL = (IsFIELD==FDC_FIELD) ? FDC_REG->FDC_CNTL |FDC_DFF : FDC_REG->FDC_CNTL & ~FDC_DFF;
		FDC_REG->FDC_DST_BASE_L = (U16)(DestBaseAddr&0xffff);
		FDC_REG->FDC_DST_BASE_H = (U16)((DestBaseAddr>>16)&0xffff);
	}
}

void FDC_Run(void)
{
	FDC_REG->FDC_CNTL = (FDC_REG->FDC_CNTL&0xefff) | FDC_START;
	FDC_REG->FDC_CNTL = FDC_REG->FDC_CNTL & ~FDC_START;
}

void FDC_Util
(
	U16	IsVSync,		// CTRUE, CFALSE
	U16	IsSrcFIELD,		// FDC_FRAME, FDC_FIELD
	U16	Chroma,		// FDC_CHROMA_420, FDC_CHROMA_422, FDC_CHROMA_444
	U16 	Rotate,			// FDC_ROT_000, FDC_ROT_090, FDC_ROT_180, FDC_ROT_270
	U16   IsDstFIELD, 	// FDC_FRAME, FDC_FIELD
	U16   IsToScaler,		// CTRUE, CFALSE
	U16 	Width,			U16 Height,
	U16 	LumaOffset, U16 	CbOffset, U16 	CrOffset,
	U32	DstBaseAddr
)
{
	U16	MBX, MBY;

	FDC_Initialize();

	// don't use : FDC_InitHardware();

	FDC_REG->FDC_CNTL &= ~FDC_IsBIG; // little endian.. test by hhsong 050729

	FDC_CfgSrcFrame(
		IsSrcFIELD,
		LumaOffset, CbOffset, CrOffset
	);
	FDC_CfgDstFrame(IsToScaler, IsDstFIELD, DstBaseAddr);
	//FDC_SetWrMode(IsToScaler);
	FDC_SetRotation(Rotate);

	MBX	= ( ( Width >> 4 ) - 1);
	MBY	= ( ( Height >> 4 ) -1 );
	FDC_SetFrameSize( MBX, MBY );
	FDC_SetChroma(Chroma);

	if ( IsVSync == 1 )
		FDC_SetVSyncEnable();
	else	// CFALSE
		FDC_SetVSyncDisable();

	ds_printk("FDC_REG = 0x%08x\n", FDC_REG);
	ds_printk("FDC_CNTL = 0x%04x\n", FDC_REG->FDC_CNTL);
	ds_printk("FDC_LUMA_OFFSET = 0x%04x, FDC_CB_OFFSET = 0x%04x, FDC_CR_OFFSET = 0x%04x\n",
			FDC_REG->FDC_LUMA_OFFSET, FDC_REG->FDC_CB_OFFSET, FDC_REG->FDC_CR_OFFSET);
	ds_printk("FDC_DST_BASE_L = 0x%04x, FDC_DST_BASE_H = 0x%04x\n", FDC_REG->FDC_DST_BASE_L, FDC_REG->FDC_DST_BASE_H);
	ds_printk("FDC_FAME_SIZE = 0x%04x\n",FDC_REG->FDC_FRAME_SIZE);
}

CBOOL FDC_IsBusy(void)
{
	return(FDC_REG->FDC_CNTL & FDC_START);
}

void FDC_Stop(void)
{
	FDC_REG->FDC_CNTL |= FDC_STOP;
}

EXPORT_SYMBOL(FDC_Util);
EXPORT_SYMBOL(FDC_Run);
EXPORT_SYMBOL(FDC_IsBusy);
EXPORT_SYMBOL(FDC_Stop);

