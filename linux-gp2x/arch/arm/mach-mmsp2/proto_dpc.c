#include <linux/module.h>

#include <asm/arch/mmsp20.h>
#include <asm/arch/proto_dpc.h>

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(x...) printk(x)
#else
#  define ds_printk(fmt, args...)
#endif

MMSP20_DPC * DPC;

void DPC_GetDeviceInfo ( void )
{
}

void 
DPC_Initialize ( void )
{
	DPC = MMSP20_GetBase_DPC();
}

void
DPC_InitHardware
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
	ds_printk("Display Output Mode = %d\n", Mode);
	
	DPC->DPC_CNTL &= ~DOT;
	DPC->DPC_CNTL |= FInsrt((Mode>>2)&0x3, BF_DOT);	//(Mode<<4)&DOT;

	DPC->DPC_CNTL &= ~DOF;
	DPC->DPC_CNTL |= FInsrt(Mode&0x3, BF_DOF);	//(Mode<<1)&DOF;

	switch(Mode) {
	case DPC_RGB_444:
	case DPC_RGB_565:
	case DPC_RGB_666:
	case DPC_RGB_888:
		DPC->DPC_CLKCNTL &= ~CLK1SEL;
		DPC->DPC_CLKCNTL &= ~CLK2SEL;
		DPC->DPC_CNTL &= ~CSYNC;
		DPC->DPC_CNTL &= ~PAL;
		DPC->DPC_CNTL &= ~HDTV;
		DPC->DPC_CNTL &= ~ENC;
		DPC->DPC_CNTL &= ~TVMODE;
		DPC->DPC_CNTL &= ~SYNCCBCR;
		DPC->DPC_CNTL &= ~ESAVEN;
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

	if(IsInterlace)
		DPC->DPC_CNTL |= INTERLACE;
	else
		DPC->DPC_CNTL &= ~INTERLACE;

 	if((IntEn>>1)&1)
		DPC->DPC_INTR |= VSINTEN;
	else
		DPC->DPC_INTR &= ~VSINTEN;

	if(IntEn&0x1)
		DPC->DPC_INTR |= HSINTEN;
	else
		DPC->DPC_INTR &= ~HSINTEN;

	if(IntHSyncEdgePos)
		DPC->DPC_INTR |= HSINTEDGE;
	else
		DPC->DPC_INTR &= ~HSINTEDGE;

	if(IntVSyncEdgePos)
		DPC->DPC_INTR |= VSINTEDGE;
	else
		DPC->DPC_INTR &= ~VSINTEDGE;

	DPC->DPC_CLKCNTL &= ~CLKSRC;
	DPC->DPC_CLKCNTL |= FInsrt(ClockSource, BF_CLKSRC);

	DPC->DPC_CNTL &= ~SLAVE;	//Master Mode 

	ds_printk("<1> DPC_CNTL = 0x%02X\n", DPC->DPC_CNTL);
}

void
DPC_Run(void)
{
	ds_printk("dpc_run\n");
	DPC->DPC_FPIATV1 = 0xffff;
	DPC->DPC_FPIATV2 = 0xffff;
	DPC->DPC_FPIATV3 = 0xffff;
	DPC->DPC_CNTL |= ENB;
}

void
DPC_Stop(void)
{
	ds_printk("dpc_stop\n");
	DPC->DPC_FPIATV1 = 0x0000;
	DPC->DPC_FPIATV2 = 0x0000;
	DPC->DPC_FPIATV3 = 0x0000;
	DPC->DPC_CNTL &= ~ENB;
}

void
DPC_Close(void)
{
	
}

//#define DPC_SetInterlace(IsInterlace) (DPC->INTERLACE = IsInterlace)
//#define DPC_SetClockSource(ClockSource) (DPC->CLKSRC = ClockSource)	

void
DPC_SetFrameSize
(
	U16 Width, 
	U16 Height 
)
{
	DPC->DPC_X_MAX = Width - 1;
	DPC->DPC_Y_MAX = Height - 1;
}

void
DPC_SetHSyncParm
(
	U16 HorSyncWidth,
	U16 HorFrontPorch,
	U16 HorBackPorch,
	CBOOL   HorPolarity
)
{
	// Horizontal Sync WIdth
	DPC->DPC_HS_WIDTH &= ~HSWID;
	DPC->DPC_HS_WIDTH |= FInsrt(HorSyncWidth - 1,BF_HSWID);	// T2
	DPC->DPC_HS_END = (HorFrontPorch/2) - 1;	// T7
	DPC->DPC_HS_STR = (HorFrontPorch/2) + (HorFrontPorch%1) - 1; //T8
	// Horizontal Back Porch
	DPC->DPC_DE &= ~DESTR;
	DPC->DPC_DE |= FInsrt((HorBackPorch - 1),BF_DESTR);
	if(HorPolarity)
		DPC->DPC_FPIPOL1 |= HSPOL;
	else
		DPC->DPC_FPIPOL1 &= ~HSPOL; 
}

void
DPC_SetVSyncParm
(
	U16 	VerSyncWidth,
	U16 	VerFrontPorch,
	U16 	VerBackPorch,
	CBOOL   VerPolarity
)
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

void
DPC_SetDEParm
(
	U32	DEMode,
	U16	DEPulseWidth,
	U16 	DEShift,
	CBOOL	DEPolarity
)
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
	if(DEPolarity)
		DPC->DPC_FPIPOL1 |= DEPOL;
	else
		DPC->DPC_FPIPOL1 &= ~DEPOL;
}

void
DPC_SetPSParm
(
	CBOOL PSEn,
	U16	PSRisingPos,
	U16	PSFallingPos,
	CBOOL	PSPolarity
)
{
	if(PSEn==CTRUE)
	{
		DPC->DPC_FG |= PSEN;

		DPC->DPC_PS &= ~PSSTR;
		DPC->DPC_PS |= FInsrt(PSRisingPos-1, BF_PSSTR);

		DPC->DPC_PS &= ~PSEND;
		DPC->DPC_PS |= FInsrt(PSFallingPos-1, BF_PSEND);

		if(PSPolarity)
			DPC->DPC_FPIPOL1 |= PSPOL;
		else
			DPC->DPC_FPIPOL1 &= ~PSPOL;

	}
	else
	{
		DPC->DPC_FG &= ~PSEN;
	}
}

void
DPC_SetFGParm
(
	CBOOL	FGEn,
	U16	FGWidth,
	CBOOL	FGPolarity
)
{
	if(FGEn==CTRUE) 
	{
		DPC->DPC_FPIATV1 |= FGATV;
		DPC->DPC_FG &= ~FGWID;
		DPC->DPC_FG |= FInsrt(FGWidth-1,BF_FGWID);
		if(FGPolarity)
			DPC->DPC_FPIPOL1 |= FGPOL;
		else
			DPC->DPC_FPIPOL1 &= ~FGPOL;
	}
	else
	{
		DPC->DPC_FPIATV1 &= ~FGATV;
	}
}


void
DPC_SetLPParm
(
	CBOOL	LPEn,
	U16	LPWidth,
	U16	LPFrontPorch,
	U16	LPBackPorch
)
{
	if(LPEn == CTRUE)
	{
		DPC->DPC_LP |= LPEN;
		DPC->DPC_LP &= ~LPWID;
		DPC->DPC_LP |= FInsrt(LPWidth-1, BF_LPWID);
		DPC->DPC_LP &= ~LPSTR;
		DPC->DPC_LP |= FInsrt(LPFrontPorch-1, BF_LPSTR);
		DPC->DPC_HS_END = LPWidth + LPFrontPorch;
		DPC->DPC_HS_STR = 0;
		DPC->DPC_HS_WIDTH &= ~HSWID;
		DPC->DPC_HS_WIDTH |= FInsrt((LPBackPorch-2)/2-1, BF_HSWID);
		DPC->DPC_DE &= ~DESTR;
		DPC->DPC_DE |= FInsrt((LPBackPorch-2)/2-1+(LPBackPorch%2), BF_DESTR);
	}
	else
	{
		DPC->DPC_LP &= ~LPEN;
	}
}

void
DPC_SetPOLParm
(
	CBOOL	POLEn,
	CBOOL	POLVerticalToggleEn,
	U16	POLTogglePos,
	U16	POLToggleArea,
	U16	POLPeriod,
	CBOOL	POLPolarity
)
{
	if(POLEn == CTRUE)
	{
		DPC->DPC_POL |= POLEN;

		if(POLVerticalToggleEn)
			DPC->DPC_CLKVL |= POLTGLEN;
		else
			DPC->DPC_CLKVL &= ~POLTGLEN;

		DPC->DPC_POL &= ~POLTGLPOS;
		DPC->DPC_POL |= FInsrt(POLTogglePos, BF_POLTGLPOS);
		
		if(POLToggleArea)
			DPC->DPC_POL |= POLTGLAREA;
		else
			DPC->DPC_POL &= ~POLTGLAREA; 
	
		DPC->DPC_POL &= ~POLPERIOD;
		DPC->DPC_POL |= FInsrt(POLPeriod, BF_POLPERIOD);

		if(POLPolarity)
			DPC->DPC_FPIPOL1 |= POLPOL;
		else
			DPC->DPC_FPIPOL1 &= ~POLPOL;
	}
	else
	{
		DPC->DPC_POL &= ~POLEN;
	}
	
}

void
DPC_SetCLKVParm
(
	CBOOL	CLKVEn,
	U16	CLKVFallingPos,   // T21
	U16	CLKVRisingPos,    // T22
	U16	CLKVRisingArea,
	CBOOL	CLKVPolarity
)
{
	if(CLKVEn == CTRUE)
	{
		DPC->DPC_CLKVL |= CVEN;

		DPC->DPC_CLKVL &= ~CVFALPOS;
		DPC->DPC_CLKVL |= FInsrt(CLKVFallingPos, BF_CVFALPOS);
		
		DPC->DPC_CLKVH &= ~CVRISPOS;
		DPC->DPC_CLKVH |= FInsrt(CLKVRisingPos, BF_CVRISPOS);

		if(CLKVRisingArea)
			DPC->DPC_CLKVH |= CVATVAREA;
		else
			DPC->DPC_CLKVH &= ~CVATVAREA;

		if(CLKVPolarity)
			DPC->DPC_FPIPOL1 |= CVPOL;
		else
			DPC->DPC_FPIPOL1 &= ~CVPOL; 		
	}
	else
	{
		DPC->DPC_CLKVL &= ~CVEN;
	}
}


void
DPC_SetCLKHParm
(
	CBOOL	CLKHEn,
	U8 	CLKHPhase,
	CBOOL	CLKHPolarity
)
{
	if(CLKHEn == CTRUE) 
	{
		DPC->DPC_FPIATV1 |= CHATV;

		DPC->DPC_FPICNTL &= ~CHPHASE;
		DPC->DPC_FPICNTL |= FInsrt(CLKHPhase, BF_CHPHASE);

		if(CLKHPolarity)
			DPC->DPC_FPIPOL1 |= CHPOL;
		else
			DPC->DPC_FPIPOL1 &= ~CHPOL;
	}
	else
	{
		DPC->DPC_FPIATV1 &= ~CHATV;
	}
}



U16
DPC_GetInterruptPend(void)
{
	return (DPC->DPC_INTR)&0x3;	//DPC->REG[DPC_INTRC]&0x3;
}

void
DPC_ClrInterruptPend
(
	U16 IntFlag
)
{
	DPC->DPC_INTR |= IntFlag;	//DPC->REG[DPC_INTRC] = DPC->REG[DPC_INTRC] | IntFlag;
}

U16
DPC_GetInterruptEnable(void)
{
    return ((DPC->DPC_INTR)>>4)&0x3;	//(DPC->REG[DPC_INTRC]>>4)&0x3;
}

void
DPC_SetInterruptEnable
(
	U16 	IntEn,
	CBOOL 	IntHSyncEdgePos, // 0: First Edge, 1: Second Edge...
	CBOOL 	IntVSyncEdgePos  // 0: First Edge, 1: Second Edge...
)
{
	if(IntEn&1)
		DPC->DPC_INTR |= HSINTEN;
	else
		DPC->DPC_INTR &= ~HSINTEN;

	if((IntEn>>1)&1)
		DPC->DPC_INTR |= VSINTEN;
	else
		DPC->DPC_INTR &= ~VSINTEN;

	if(IntHSyncEdgePos)
		DPC->DPC_INTR |= HSINTEDGE;
	else
		DPC->DPC_INTR &= ~HSINTEDGE;

	if(IntVSyncEdgePos)
		DPC->DPC_INTR |= VSINTEDGE;
	else
		DPC->DPC_INTR &= ~VSINTEDGE; 
}

void
DPC_UTIL_HVSYNC
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
	CBOOL   VerPolarity
)
{

	DPC_SetFrameSize ( Width, Height );
	DPC_SetHSyncParm ( HorSyncWidth, HorFrontPorch, HorBackPorch, HorPolarity);
	DPC_SetVSyncParm ( VerSyncWidth, VerFrontPorch, VerBackPorch, VerPolarity);
	DPC_SetDEParm ( (U8)DPC_DE_DE, 0, 0, CFALSE );  
}


void
DPC_UTIL_HVSYNC_GPX320240
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

void 
DPC_UTIL_DATA_INV(CBOOL DataPolarity)
{
	if(DataPolarity){
		DPC->DPC_FPIPOL2 = DATA_FPIPOL2;
		DPC->DPC_FPIPOL3 = DATA_FPIPOL3; 
	}else{
		DPC->DPC_FPIPOL2 = ~DATA_FPIPOL2;
		DPC->DPC_FPIPOL3 = ~DATA_FPIPOL3; 
	}
}

void
DPC_UTIL_LTS350Q1(void)
{

}

EXPORT_SYMBOL(DPC_InitHardware);
EXPORT_SYMBOL(DPC_Run);
EXPORT_SYMBOL(DPC_Stop);
EXPORT_SYMBOL(DPC_SetFrameSize);
EXPORT_SYMBOL(DPC_SetHSyncParm);
EXPORT_SYMBOL(DPC_SetVSyncParm);
EXPORT_SYMBOL(DPC_SetDEParm);
EXPORT_SYMBOL(DPC_SetInterruptEnable); // hhsong add
