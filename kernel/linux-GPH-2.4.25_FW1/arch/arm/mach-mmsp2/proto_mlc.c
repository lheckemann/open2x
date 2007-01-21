#include <linux/module.h>

#include <asm/arch/mmsp20.h>
#include <asm/arch/proto_mlc.h>
#include <asm/arch/proto_dpc.h>

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(fmt, args...) printk("%s: " fmt, __FUNCTION__ , ## args)
#else
#  define ds_printk(fmt, args...)
#endif

MMSP20_MLC * MLC_REG;
extern MMSP20_DPC * DPC;	

void
MLC_Initialize(	void )
{
	MLC_REG = MMSP20_GetBase_MLC();
}

void
MLC_SetFieldPolarity
(
	U8 FieldPol
)
{
	if ( FieldPol == MLC_FID_NORM )			
		MLC_REG->OVLAY_CNTR &= ~DISP_FLD_POS;
	else	
		MLC_REG->OVLAY_CNTR |= DISP_FLD_POS;
}

void
MLC_SetOSDSPUPriority
(
	U8 OSD_SPU_Pri
)
{
	if ( OSD_SPU_Pri == MLC_ORD_OSD )		
		MLC_REG->OVLAY_CNTR &= ~DISP_SWAP;
	else	//( OSD_SPU_Pri == MLC_ORD_SPU )		
		MLC_REG->OVLAY_CNTR |= DISP_SWAP;
}

void
MLC_SetGammaPath
(
	U8 ByPathGamma
)
{
	if ( ByPathGamma == MLC_GAMM_USE )				// [12] <= 0
		MLC_REG->OVLAY_CNTR &= ~DISP_GAMM_BYPATH;
	else	//( ByPathGamma == MLC_GAMM_BYPASS )	// [12] <= 1
		MLC_REG->OVLAY_CNTR |= DISP_GAMM_BYPATH;
}

void MLC_YUVAOn ( void )
{
	MLC_REG->OVLAY_CNTR |= DISP_VLAON;
	ds_printk("OVLAY_CNTR = 0x%04x\n", MLC_REG->OVLAY_CNTR);	
	ds_printk("YUV_CNTL = 0x%04x\n", MLC_REG->YUV_CNTL);	
}

void MLC_YUVAOff ( void )
{
	MLC_REG->YUV_CNTL &= ~MLC_SC2DP_A;  // YUVA layer source = MEMORY 050630 hhsong add
	MLC_REG->OVLAY_CNTR &= ~DISP_VLAON;
	ds_printk("OVLAY_CNTR = 0x%04x\n", MLC_REG->OVLAY_CNTR);	
	ds_printk("YUV_CNTL = 0x%04x\n", MLC_REG->YUV_CNTL);	
}

void MLC_YUVBOn ( void )
{
	MLC_REG->OVLAY_CNTR |= DISP_VLBON;
	ds_printk("OVLAY_CNTR = 0x%04x\n", MLC_REG->OVLAY_CNTR);	
}

void MLC_YUVBOff ( void )
{
	MLC_REG->YUV_CNTL &= ~MLC_FDC2DP_B;  // YUVB layer source = MEMORY 050630 hhsong add
	MLC_REG->OVLAY_CNTR &= ~DISP_VLBON;
	ds_printk("OVLAY_CNTR = 0x%04x\n", MLC_REG->OVLAY_CNTR);	
}

void MLC_RGBOn (U8 RegionNr)
{
	MLC_REG->OVLAY_CNTR = (U16)(MLC_REG->OVLAY_CNTR | (1<<(RegionNr+2)));
	ds_printk("OVLAY_CNTR = 0x%04x\n", MLC_REG->OVLAY_CNTR);	
}

void MLC_RGBOff (U8 RegionNr)
{
	MLC_REG->OVLAY_CNTR = (U16)(MLC_REG->OVLAY_CNTR & ~(1<<(RegionNr+2)));
	ds_printk("OVLAY_CNTR = 0x%04x\n", MLC_REG->OVLAY_CNTR);	
}

void
MLC_FSCOn ( void )
{
	MLC_REG->OVLAY_CNTR |= DISP_OVLY2SCLE;
}

void
MLC_FSCOff ( void )
{
	MLC_REG->OVLAY_CNTR &= ~DISP_OVLY2SCLE;
}

void
MLC_HWCOn ( void )
{
	MLC_REG->OVLAY_CNTR |= DISP_CURSOR;
}

void
MLC_HWCOff ( void )
{
	MLC_REG->OVLAY_CNTR &= ~DISP_CURSOR;
}

void
MLC_SPUOn ( void )
{
	MLC_REG->SPU_START_DELAY = 100;	// Start Delay
	MLC_REG->SPU_CTRL |= MLC_SPU_ON;
	MLC_REG->OVLAY_CNTR |= DISP_SUBPICTURE;
}

void
MLC_SPUOff ( void )
{
	MLC_REG->SPU_CTRL &= ~MLC_SPU_ON;
	MLC_REG->OVLAY_CNTR &= ~DISP_SUBPICTURE;
}

void
MLC_OSDOn ( void )
{
	MLC_REG->OVLAY_CNTR &= ~DISP_OSD;
	MLC_REG->OVLAY_CNTR |= DISP_OSD;
	MLC_REG->OVLAY_CNTR &= ~DISP_OSD;
	MLC_REG->OVLAY_CNTR |= DISP_OSD;
}

void
MLC_OSDOff ( void )
{
	MLC_REG->OVLAY_CNTR &= ~DISP_OSD;
}

void
MLC_FSC_SetPath (U8 DataPath)
{
	if ( DataPath == MLC_FSC_SC_ONLY )			// [15] <= 0
	{
		MLC_REG->OVLAY_CNTR = (U16)(( MLC_REG->OVLAY_CNTR & ~(0x1<<15) ) | (0x0<<15));
		MLC_REG->YUV_CNTL	= (U16)(( MLC_REG->YUV_CNTL & ~(0x1<<13) ) | (0x1<<13));
	}
	else	//( DataPath == MLC_FSC_SC_DISP )	// [15] <= 1
	{
		MLC_REG->OVLAY_CNTR = (U16)(( MLC_REG->OVLAY_CNTR & ~(0x1<<15) ) | (0x1<<15));
		MLC_REG->YUV_CNTL	= (U16)(( MLC_REG->YUV_CNTL & ~(0x1<<13) ) | (0x0<<13));
	}
}

void
MLC_YUVA_EnableBotDisaply
(
	CBOOL enb
)
{
	// SelDiv
	if ( enb == CTRUE )		
		MLC_REG->YUV_EFECT |= MLC_VLA_BT;
	else	//( enb == CFALSE )		
		MLC_REG->YUV_EFECT &= ~MLC_VLA_BT;
}

void
MLC_YUVA_FromData
(
	U8 FromData
)
{
	// FromData
	if ( FromData == MLC_YUV_FROM_MEM )		
		MLC_REG->YUV_CNTL &= ~MLC_SC2DP_A;
	else	//( FromData == MLC_YUV_FROM_SC )	
		MLC_REG->YUV_CNTL |= MLC_SC2DP_A;
}

void
MLC_SetVidoeLayerPriority
(
	U8 Priority
)
{
	// Priority
	if ( Priority == MLC_YUV_PRIO_A )			
		MLC_REG->YUV_CNTL &= ~MLC_VL_PRIO;
	else	//( Priority == MLC_YUV_PRIO_B )		
		MLC_REG->YUV_CNTL |= MLC_VL_PRIO;
}

void
MLC_YUVA_SetMirrorTop
(
	U8 hTopMirror,
	U8 vTopMirror
)
{
	// hTopMirror
	if ( hTopMirror == MLC_YUV_MIRROR_OFF )		
		MLC_REG->YUV_EFECT &= ~MLC_VLA_TP_MR_H;
	else	//( hTopMirror == MLC_YUV_MIRROR )	
		MLC_REG->YUV_EFECT |= MLC_VLA_TP_MR_H;
		
	// vTopMirror
	if ( vTopMirror == MLC_YUV_MIRROR_OFF )			
		MLC_REG->YUV_EFECT &= ~MLC_VLA_TP_MR_V;
	else	//( vTopMirror == MLC_YUV_MIRROR )	
		MLC_REG->YUV_EFECT |= MLC_VLA_TP_MR_V;
		
}

void
MLC_YUVA_SetMirrorBot
(
	U8 hBottomMirror,
	U8 vBottomMirror
)
{
	// hBottomMirror
	if ( hBottomMirror == MLC_YUV_MIRROR_OFF )	
		MLC_REG->YUV_EFECT &= ~MLC_VLA_BT_MR_H;
	else	//( hBottomMirror == MLC_YUV_MIRROR )	// [1] <= 1
		MLC_REG->YUV_EFECT |= MLC_VLA_BT_MR_H;
		
	// vBottomMirror
	if ( vBottomMirror == MLC_YUV_MIRROR_OFF )	
		MLC_REG->YUV_EFECT &= ~MLC_VLA_BT_MR_H;
	else	//( vBottomMirror == MLC_YUV_MIRROR )	// [0] <= 1
		MLC_REG->YUV_EFECT |= MLC_VLA_BT_MR_H;
		
}

void
MLC_YUVA_SetAddressTop
(
	U32 TopOddAddress,
	U32 TopEvenAddress
)
{
	MLC_REG->YUVA_TP_OADR_L = (U16)(TopOddAddress & 0xffff);
	MLC_REG->YUVA_TP_OADR_H = (U16)(TopOddAddress >> 16);
	
	MLC_REG->YUVA_TP_EADR_L = (U16)(TopEvenAddress & 0xffff);
	MLC_REG->YUVA_TP_EADR_H = (U16)(TopEvenAddress >> 16);
}

void
MLC_YUVA_SetAddressBot
(
	U32 BotOddAddress,
	U32 BotEvenAddress
)
{
	MLC_REG->YUVA_BT_OADR_L = (U16)(BotOddAddress & 0xffff);
	MLC_REG->YUVA_BT_OADR_H = (U16)(BotOddAddress >> 16);
	
	MLC_REG->YUVA_BT_EADR_L = (U16)(BotEvenAddress & 0xffff);
	MLC_REG->YUVA_BT_EADR_H = (U16)(BotEvenAddress >> 16);
}

void
MLC_YUVA_SetScaleTop
(
	U16 SrcPixWidth, U16 SrcPixHeight, U16 SrcStride,
	U16 DestPixWidth, U16 DestPixHeight
)
{
	U32	hScale, vScale;
	if(DestPixWidth)
		hScale = (U32) ( ( SrcPixWidth * 1024 ) / DestPixWidth );
	else
		hScale = 0;
	if(DestPixHeight)
		vScale = (U32) ( ( SrcPixHeight * SrcStride >> 1 ) / DestPixHeight );
	else
		vScale = 0;
	
	MLC_REG->YUVA_TP_PXW = (U16)(SrcStride >> 1);
	MLC_REG->YUVA_TP_HSC = (U16)(hScale);
	MLC_REG->YUVA_TP_VSC_L = (U16)(vScale & 0xffff);
	MLC_REG->YUVA_TP_VSC_H = (U16)(vScale >> 16);
	
}

void
MLC_YUVA_SetScaleBot
(
	U16 SrcPixWidth, U16 SrcPixHeight, U16 SrcStride,
	U16 DestPixWidth, U16 DestPixHeight
)
{
	U32	hScale, vScale;
	if(DestPixWidth)
		hScale = (U32) ( ( SrcPixWidth * 1024 ) / DestPixWidth );
	else
		hScale = 0;
	if(DestPixHeight)
		vScale = (U32) ( ( SrcPixHeight * SrcStride >> 1 ) / DestPixHeight );
	else
		vScale = 0;
	
	MLC_REG->YUVA_BT_PXW = (U16)(SrcStride >> 1);
	MLC_REG->YUVA_BT_HSC = (U16)(hScale);
	MLC_REG->YUVA_BT_VSC_L = (U16)(vScale & 0xffff);
	MLC_REG->YUVA_BT_VSC_H = (U16)(vScale >> 16);
	
}

void
MLC_YUVA_SetCoordTop
(
	U16 TopStarX, U16 TopStartY,
	U16 TopEndX, U16 TopEndY
)
{
	MLC_REG->YUVA_STX = TopStarX;
	MLC_REG->YUVA_ENDX = TopEndX - 1;
	MLC_REG->YUVA_TP_STY = TopStartY;
	MLC_REG->YUVA_TP_ENDY = TopEndY - 1;
}

void
MLC_YUVA_SetCoordBot
(
	U16 BotEndY
)
{
	MLC_REG->YUVA_BT_ENDY = BotEndY - 1;
}

void
MLC_YUVB_EnableBotDisaply
(
	CBOOL enb
)
{
	// SelDiv
	if ( enb == CTRUE )			// [9] <= 1
		MLC_REG->YUV_EFECT |= MLC_VLB_BT;
	else	//( enb == CFALSE )		// [9] <= 0
		MLC_REG->YUV_EFECT &= ~MLC_VLB_BT;
}

void
MLC_YUVB_FromData
(
	U8 FromData
)
{
	// FromData
	if ( FromData == MLC_YUV_FROM_MEM )	
		MLC_REG->YUV_CNTL &= ~MLC_FDC2DP_B;
	else	//( FromData == MLC_YUV_FROM_FDC )	
		MLC_REG->YUV_CNTL |= MLC_FDC2DP_B;
}

void
MLC_YUVB_SetMirrorTop
(
	U8 hTopMirror,
	U8 vTopMirror
)
{
	// hTopMirror
	if ( hTopMirror == MLC_YUV_MIRROR_OFF )			// [3] <= 0
		MLC_REG->YUV_EFECT &= ~MLC_VLB_TP_MR_H;
	else	//( hTopMirror == MLC_YUV_MIRROR )	// [3] <= 1
		MLC_REG->YUV_EFECT |= MLC_VLB_TP_MR_H;
		
	// vTopMirror
	if ( vTopMirror == MLC_YUV_MIRROR_OFF )			// [2] <= 0
		MLC_REG->YUV_EFECT &= ~MLC_VLB_TP_MR_V;
	else	//( vTopMirror == MLC_YUV_MIRROR )	// [2] <= 1
		MLC_REG->YUV_EFECT |= MLC_VLB_TP_MR_V;	
}

void
MLC_YUVB_SetMirrorBot
(
	U8 hBottomMirror,
	U8 vBottomMirror
)
{
	// hBottomMirror
	if ( hBottomMirror == MLC_YUV_MIRROR_OFF )			// [1] <= 0
		MLC_REG->YUV_EFECT &= ~MLC_VLB_BT_MR_H;
	else	//( hBottomMirror == MLC_YUV_MIRROR )	// [1] <= 1
		MLC_REG->YUV_EFECT |= MLC_VLB_BT_MR_H;
		
	// vBottomMirror
	if ( vBottomMirror == MLC_YUV_MIRROR_OFF )			// [0] <= 0
		MLC_REG->YUV_EFECT &= ~MLC_VLB_BT_MR_V;
	else	//( vBottomMirror == MLC_YUV_MIRROR )	// [0] <= 1
		MLC_REG->YUV_EFECT |= MLC_VLB_BT_MR_V;
		
}

void
MLC_YUVB_SetAddressTop
(
	U32 TopOddAddress,
	U32 TopEvenAddress
)
{
	MLC_REG->YUVB_TP_OADR_L = (U16)(TopOddAddress & 0xffff);
	MLC_REG->YUVB_TP_OADR_H = (U16)(TopOddAddress >> 16);
	
	MLC_REG->YUVB_TP_EADR_L = (U16)(TopEvenAddress & 0xffff);
	MLC_REG->YUVB_TP_EADR_H = (U16)(TopEvenAddress >> 16);
}

void
MLC_YUVB_SetAddressBot
(
	U32 BotOddAddress,
	U32 BotEvenAddress
)
{
	MLC_REG->YUVB_BT_OADR_L = (U16)(BotOddAddress & 0xffff);
	MLC_REG->YUVB_BT_OADR_H = (U16)(BotOddAddress >> 16);
	
	MLC_REG->YUVB_BT_EADR_L = (U16)(BotEvenAddress & 0xffff);
	MLC_REG->YUVB_BT_EADR_H = (U16)(BotEvenAddress >> 16);
}

void
MLC_YUVB_SetScaleTop
(
	U16 SrcPixWidth, U16 SrcPixHeight, U16 SrcStride,
	U16 DestPixWidth, U16 DestPixHeight
)
{
	U32	hScale, vScale;

	if(DestPixWidth)
		hScale = (U32) ( ( SrcPixWidth * 1024 ) / DestPixWidth );
	else
		hScale = 0;
	if(DestPixHeight)
		vScale = (U32) ( ( SrcPixHeight * SrcStride >> 1 ) / DestPixHeight );
	else
		vScale = 0;
	
	MLC_REG->YUVB_TP_PXW = (U16)(SrcStride >> 1);
	MLC_REG->YUVB_TP_HSC = (U16)(hScale);
	MLC_REG->YUVB_TP_VSC_L = (U16)(vScale & 0xffff);
	MLC_REG->YUVB_TP_VSC_H = (U16)(vScale >> 16);
	
}

void
MLC_YUVB_SetScaleBot
(
	U16 SrcPixWidth, U16 SrcPixHeight, U16 SrcStride,
	U16 DestPixWidth, U16 DestPixHeight
)
{
	U32	hScale, vScale;
	if(DestPixWidth)
		hScale = (U32) ( ( SrcPixWidth * 1024 ) / DestPixWidth );
	else
		hScale = 0;
	if(DestPixHeight)
		vScale = (U32) ( ( SrcPixHeight * SrcStride >> 1 ) / DestPixHeight );
	else
		vScale = 0;
	
	MLC_REG->YUVB_BT_PXW = (U16)(SrcStride >> 1);
	MLC_REG->YUVB_BT_HSC = (U16)(hScale);
	MLC_REG->YUVB_BT_VSC_L = (U16)(vScale & 0xffff);
	MLC_REG->YUVB_BT_VSC_H = (U16)(vScale >> 16);
	
}

void
MLC_YUVB_SetCoordTop
(
	U16 TopStarX, U16 TopStartY,
	U16 TopEndX, U16 TopEndY
)
{
	MLC_REG->YUVB_STX = TopStarX;
	MLC_REG->YUVB_ENDX = TopEndX - 1;
	MLC_REG->YUVB_TP_STY = TopStartY;
	MLC_REG->YUVB_TP_ENDY = TopEndY - 1;
}

void
MLC_YUVB_SetCoordBot
(
	U16 BotEndY
)
{
	MLC_REG->YUVB_BT_ENDY = BotEndY - 1;
}

void
MLC_FDC_SetScale
(
	U16 SrcPixWidth, U16 SrcPixHeight,
	U16 DestPixWidth, U16 DestPixHeight
)
{
	U32	hScale, vScale, ratio, test;

	if(DestPixWidth)
		hScale = (U32) ( ( SrcPixWidth * 1024 ) / DestPixWidth );
	else
		hScale = 0;

	if(DestPixHeight)
		vScale = (U32) ( ( SrcPixHeight * 512 ) / DestPixHeight ); 
	else
		vScale = 0;
	
	MLC_REG->YUVB_TP_PXW = (U16)(SrcPixWidth);
	MLC_REG->YUVB_TP_HSC = (U16)(hScale);
	MLC_REG->YUVB_TP_VSC_L = (U16)(vScale & 0xffff);
	MLC_REG->YUVB_TP_VSC_H = (U16)(vScale >> 16);
}

void
MLC_RGB_SetBPP
(
	U8 BPP
)
{
	// BPP
	if ( BPP == MLC_RGB_4BPP )				// [10:9] <= 0
		MLC_REG->RGB_CNTL = (U16)(( MLC_REG->RGB_CNTL & ~MLC_STL_BPP ) | FInsrt(MLC_RGB_4BPP, BF_MLC_STL_BPP));
	else if ( BPP == MLC_RGB_8BPP )			// [10:9] <= 1
		MLC_REG->RGB_CNTL = (U16)(( MLC_REG->RGB_CNTL & ~MLC_STL_BPP ) | FInsrt(MLC_RGB_8BPP, BF_MLC_STL_BPP));
	else if ( BPP == MLC_RGB_16BPP )		// [10:9] <= 2
		MLC_REG->RGB_CNTL = (U16)(( MLC_REG->RGB_CNTL & ~MLC_STL_BPP ) | FInsrt(MLC_RGB_16BPP, BF_MLC_STL_BPP));
	else //if ( BPP == MLC_RGB_24BPP )		// [10:9] <= 3
		MLC_REG->RGB_CNTL = (U16)(( MLC_REG->RGB_CNTL & ~MLC_STL_BPP ) | FInsrt(MLC_RGB_24BPP, BF_MLC_STL_BPP));
}

void
MLC_RGB_SetColorKey
(
	U8 rColorKey, U8 gColorKey, U8 bColorKey
)
{
	MLC_REG->RGB_CKEY_R = (U16)(rColorKey);
	MLC_REG->RGB_CKEY_GB = (U16)FInsrt(gColorKey, BF_MLC_STL_CKEYG) | (U16)(bColorKey);
}

void
MLC_RGB_MixMux
(
	U8 rgnNum,	// 1 ~ 5 Region
	U8 MixMux,	// Mix or Mux
	U8 Alpha	// Alpha Value     
)
{
	if ( rgnNum == MLC_RGB_RGN_1 )
	{
		// MixMux
		if ( MixMux == MLC_RGB_MIXMUX_PRI )					// [1:0] <= 0
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL1_MIXMUX ) 
			                                        | FInsrt(MLC_RGB_MIXMUX_PRI, BF_MLC_STL1_MIXMUX));
		else if ( MixMux == MLC_RGB_MIXMUX_CKEY )			// [1:0] <= 1
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL1_MIXMUX ) 
			                                        | FInsrt(MLC_RGB_MIXMUX_CKEY, BF_MLC_STL1_MIXMUX));
		else //if ( MixMux == MLC_RGB_MIXMUX_ALPHA )		// [1:0] <= 2
			MLC_REG->RGB_MIXMUX = (U16)(( MLC_REG->RGB_MIXMUX & ~MLC_STL1_MIXMUX ) 
			                                        | FInsrt(MLC_RGB_MIXMUX_ALPHA, BF_MLC_STL1_MIXMUX));
		
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

void
MLC_RGB_SetScale
(                                                                       
	U16 SrcPixWidth, U16 SrcPixHeight, U16 SrcStride,                                                       
	U16 DestPixWidth, U16 DestPixHeight
)
{
	U32	hScale, vScale;

	if(DestPixWidth)
		hScale = (U32) ( ( SrcPixWidth * 1024 ) / DestPixWidth );
	else
		hScale = 0;
	if (DestPixHeight)
		vScale = (U32) ( ( SrcPixHeight * SrcStride ) / DestPixHeight );
	else
		vScale = 0;
		
	MLC_REG->RGB_HSC = (U16)(hScale & 0xffff);
	MLC_REG->RGB_VSC_L = (U16)(vScale & 0xffff);
	MLC_REG->RGB_VSC_H = (U16)(vScale >> 16);
	MLC_REG->RGB_HW = (U16)(SrcStride);
}

void
MLC_RGB_SetActivate
(
	U8 rgnNum, U8 Activate
)
{	
	U16	Act;
	
	// Activate
	if ( Activate == MLC_RGB_RGN_ACT )
		Act = 0x1;
	else	// if ( Activate == MLC_RGB_RGN_DISACT )
		Act = 0x0;
		
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

void
MLC_RGB_SetCoord
(
	U8 rgnNum,
	U16 StarX, U16 EndX, U16 StartY, U16 EndY
)
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
		ds_printk("rgb_region_number_error?\n");		
	}
}

void
MLC_RGB_SetAddress	(U32 OddAddr, U32 EvenAddr)
{
	MLC_REG->RGB_OADR_L = (U16)(OddAddr & 0xffff);
	MLC_REG->RGB_OADR_H = (U16)(OddAddr >> 16);
	MLC_REG->RGB_EADR_L = (U16)(EvenAddr & 0xffff);
	MLC_REG->RGB_EADR_H = (U16)(EvenAddr >> 16);
}

void
MLC_OSD_SetSrcAddress (U32 OddAddr, U32 EvenAddr)
{
	MLC_REG->OSD_OADR_L = (U16)(OddAddr & 0xffff);
	MLC_REG->OSD_OADR_H = (U16)(OddAddr >> 16);
	MLC_REG->OSD_EADR_L = (U16)(EvenAddr & 0xffff);
	MLC_REG->OSD_EADR_H = (U16)(EvenAddr >> 16);
}

void
MLC_SPU_SetSrcAddress (U32 Addr)
{
	
	MLC_REG->SPU_BASE_ADDRL = (U16)(Addr & 0xffff);
	MLC_REG->SPU_BASE_ADDRH = (U16)(Addr >> 16);
}

void
MLC_Cursor_Configure
(
	U8 Size,
	U8 ForeAlpha, U8 BackAlpha,
	U8 rForeColor, U8 gForeColor, U8 bForeColor,
	U8 rBackColor, U8 gBackColor, U8 bBackColor,
	U32 OddAddr, U32 EvenAddr
)
{
	MLC_REG->HWC_CNTR = (U16)(( MLC_REG->HWC_CNTR & ~MLC_HWC_SZ ) | FInsrt(Size, BF_MLC_HWC_SZ));
		
	// Foreground Alpha	[15:12] 
	MLC_REG->HWC_CNTR = (U16)(( MLC_REG->HWC_CNTR & ~MLC_HWC_FALPH ) | FInsrt(ForeAlpha,BF_MLC_HWC_FALPH));
	
	// Background Alpha	[11:8] 
	MLC_REG->HWC_CNTR = (U16)(( MLC_REG->HWC_CNTR & ~MLC_HWC_BALPH ) | FInsrt(BackAlpha, BF_MLC_HWC_BALPH));
	
	// Color
	MLC_REG->HWC_FGR = FInsrt(gForeColor, BF_MLC_HWC_FG)|FInsrt(rForeColor, BF_MLC_HWC_FR);
	MLC_REG->HWC_FB = (U16)(bForeColor & 0xff);
	
	MLC_REG->HWC_BGR = FInsrt(gBackColor, BF_MLC_HWC_BG)|FInsrt(rBackColor, BF_MLC_HWC_BR);
	MLC_REG->HWC_BB = (U16)(bBackColor & 0xff);
	
	// address
	MLC_REG->HWC_OADR_L = (U16)(OddAddr & 0xffff);
	MLC_REG->HWC_OADR_H = (U16)(OddAddr >> 16);
	MLC_REG->HWC_EADR_L = (U16)(EvenAddr & 0xffff);
	MLC_REG->HWC_EADR_H = (U16)(EvenAddr >> 16);
}

void
MLC_Cursor_SetCoord	(U16 hPosition, U16 vPosition)
{
	MLC_REG->HWC_STX = hPosition & 0xffff;
	MLC_REG->HWC_STY = vPosition & 0xffff;
}

void
MLC_LumaEnhance	( U8 Contrast, U8 Bright)
{
	U8	modContrast;
	
	if ( Contrast == MLC_CONTR_1_000 )
		modContrast = 0;
	else if ( Contrast == MLC_CONTR_1_125 )
		modContrast = 1;
	else if ( Contrast == MLC_CONTR_1_250 )
		modContrast = 2;
	else if ( Contrast == MLC_CONTR_1_375 )
		modContrast = 3;
	else if ( Contrast == MLC_CONTR_1_500 )
		modContrast = 4;
	else if ( Contrast == MLC_CONTR_1_625 )
		modContrast = 5;
	else if ( Contrast == MLC_CONTR_1_750 )
		modContrast = 6;
	else // if ( Contrast == MLC_CONTR_1_875 )
		modContrast = 7;
		
	MLC_REG->LUMA_ENH = FInsrt(modContrast, BF_MLC_CNTRST)|FInsrt(Bright-128, BF_MLC_BRIGHT);
}

void	
MLC_SetDither	(U8 DitherOn, U8 DitherType)
{
	if ( ( DitherOn == MLC_DITH_OFF ) && ( DitherType == MLC_DITH_TYPE_2X2 ) )
		MLC_REG->DITHER = 0x0;
	else if ( ( DitherOn == MLC_DITH_OFF ) && ( DitherType == MLC_DITH_TYPE_4x4 ) )
		MLC_REG->DITHER = 0x1;
	else if ( ( DitherOn == MLC_DITH_ON ) && ( DitherType == MLC_DITH_TYPE_2X2 ) )
		MLC_REG->DITHER = 0x2;
	else if ( ( DitherOn == MLC_DITH_ON ) && ( DitherType == MLC_DITH_TYPE_4x4 ) )
		MLC_REG->DITHER = 0x3;
}

void
MLC_OSD_SetPalette 
(
	U32 *pPalette, U16 firstEntry, U16 numEntries
)
{
	U16	Cnt;
	U32	TmpBuf;
	
	MLC_REG->OSD_PALLT_A = 0;
	for (Cnt=0;Cnt<firstEntry;Cnt++)
	{
		TmpBuf = *pPalette++;
	}
	
	for (Cnt=firstEntry;Cnt<firstEntry+numEntries;Cnt++)
	{
		TmpBuf = *pPalette++;
		MLC_REG->OSD_PALLT_D = (U16)(TmpBuf & 0xffff);
		MLC_REG->OSD_PALLT_D = (U16)(TmpBuf >> 16);
	}
}

void
MLC_SPU_SetPalette
(
	U32 *pPalette, U16 firstEntry, U16 numEntries
)
{
	U16	Cnt;
	U32	TmpBuf;

	for (Cnt=0;Cnt<firstEntry;Cnt++)
	{
		TmpBuf = *pPalette++;
	}
	
	for (Cnt=firstEntry;Cnt<firstEntry+numEntries;Cnt=Cnt+2)
	{
		TmpBuf = *pPalette++;  
		MLC_REG->SPU_PALLET[Cnt] = (U16)(TmpBuf & 0xffff);
		MLC_REG->SPU_PALLET[Cnt+1] = (U16)(TmpBuf >> 16);
	}
}

void
MLC_RGB_SetPalette
(
	U32 *pPalette, U16 firstEntry, U16 numEntries
)
{
	U16	Cnt;
	U32	TmpBuf;
	
	MLC_REG->RGB_PALLT_A = 0;
	for (Cnt=0;Cnt<firstEntry;Cnt++)
	{
		TmpBuf = *pPalette++;
	}
	
	for (Cnt=firstEntry;Cnt<firstEntry+numEntries;Cnt++)
	{
		TmpBuf = *pPalette++;  
		MLC_REG->RGB_PALLT_D = (U16)(TmpBuf & 0xffff);
		MLC_REG->RGB_PALLT_D = (U16)(TmpBuf >> 16);
	}
}

void
MLC_RGB_SetGamma
(
	U32 *pTable, U16 firstEntry, U16 numEntries
)
{
	U16	Cnt;
	U32	TmpBuf;
	
	MLC_REG->GAMMA_A = firstEntry;
	
	for (Cnt=0;Cnt<firstEntry;Cnt++)
	{
		TmpBuf = *pTable++;
	}
			
	for (Cnt=firstEntry;Cnt<firstEntry+numEntries;Cnt++)
	{
		TmpBuf = *pTable++;
		MLC_REG->GAMMA_D = (U16)(TmpBuf & 0xffff);
		MLC_REG->GAMMA_D = (U16)(TmpBuf >> 16);
	}
} 

void 
MLC_Util_YUVA
(
	U8  SrcMemScale,	// 0: Mem, 1 : Scale Processor
	U8	PriorityA,
	U8  vMirrorTop, U8  hMirrorTop, U8 vMirrorBot, U8  hMirrorBot,
	U16 SrcWidthTop, U16 SrcHeightTop, U16 DstWidthTop, U16 DstHeightTop, U16 StrideTop,
	U16 SrcWidthBot, U16 SrcHeightBot, U16 DstWidthBot, U16 DstHeightBot, U16 StrideBot,
	U16 StartX, U16 StartY, U16 EndX, U16 EndY, U16 BottomY,
	U32 Topaddr, U32 BotAddr
)
{
	U8 	Interlace;		//  0:Progressive, 1:Interlace
	U8		Source, Priority;
	CBOOL	Div2;

	DPC_Initialize();
	
	if(DPC->DPC_CNTL & INTERLACE)
		Interlace = 1;		// Interlace mode
	else
		Interlace = 0;		// Not Interlace mode
	
	ds_printk("Interlace = %d\n", Interlace);

	MLC_Initialize(  );

	// source
	if ( SrcMemScale == 0 )
		Source 	= MLC_YUV_FROM_MEM;
	else // if ( SrcMemScale == 1 )
		Source 	= MLC_YUV_FROM_SC;
	
	// Division
	if ( EndY == BottomY )
		Div2	= CFALSE;
	else //if ( EndY != BottomY )
		Div2	= CTRUE;
		
	// Priority A/B
	Priority	= ( PriorityA == 0x0 ) ? MLC_YUV_PRIO_A : MLC_YUV_PRIO_B;
			
	MLC_YUVA_EnableBotDisaply( Div2 );
	MLC_YUVA_FromData( Source );
	MLC_SetVidoeLayerPriority( Priority );
	MLC_YUVA_SetMirrorTop( hMirrorTop, vMirrorTop );
	MLC_YUVA_SetMirrorBot( hMirrorBot, vMirrorBot );
	

	MLC_YUVA_SetAddressTop
	(
		Topaddr, Topaddr
	);
	MLC_YUVA_SetAddressBot
	(
		BotAddr, BotAddr		// Bottom Region
	);

	if(Interlace == 0) {
		MLC_YUVA_SetScaleTop
		(	
			SrcWidthTop, SrcHeightTop, StrideTop,
			DstWidthTop, DstHeightTop
		);
		MLC_YUVA_SetScaleBot
		(	
			SrcWidthBot, SrcHeightBot, StrideBot,
			DstWidthBot, DstHeightBot
		);
	
		MLC_YUVA_SetCoordTop	
		(
			StartX, StartY,	// top sX, sY
			EndX, EndY// top eX, eY
		);
	
		MLC_YUVA_SetCoordBot	
		(
			BottomY	// U16 BotEndY);
		);
	}
	else {		// if(Interlace == 1)
		MLC_YUVA_SetScaleTop
		(	
			SrcWidthTop, SrcHeightTop/2, 2*StrideTop,
			DstWidthTop, DstHeightTop/2
		);
		MLC_YUVA_SetScaleBot
		(	
			SrcWidthBot, SrcHeightBot/2, 2*StrideBot,
			DstWidthBot, DstHeightBot/2
		);
	
		MLC_YUVA_SetCoordTop	
		(
			StartX, StartY/2,	// top sX, sY
			EndX, EndY/2		// top eX, eY
		);
	
		MLC_YUVA_SetCoordBot	
		(
			BottomY/2		// U16 BotEndY);
		);
	}	
	ds_printk("MLC_OVLAY_CNTR = 0x%04X,\n ", MLC_REG->OVLAY_CNTR);
	ds_printk("MLC_YUV_EFECT = 0x%04X,\n ", MLC_REG->YUV_EFECT);
	ds_printk("MLC_YUV_CNTL = 0x%04X,\n ", MLC_REG->YUV_CNTL);
	ds_printk("MLC_YUVA_TP_HSC = 0x%04X,\n ", MLC_REG->YUVA_TP_HSC);
	ds_printk("MLC_YUVA_BT_HSC = 0x%04X,\n ", MLC_REG->YUVA_BT_HSC);
	ds_printk("MLC_YUVA_TP_VSC_L = 0x%04X,\n ", MLC_REG->YUVA_TP_VSC_L);
	ds_printk("MLC_YUVA_TP_VSC_H = 0x%04X,\n ", MLC_REG->YUVA_TP_VSC_H);
	ds_printk("MLC_YUVA_BT_VSC_L = 0x%04X,\n ", MLC_REG->YUVA_BT_VSC_L);
	ds_printk("MLC_YUVA_BT_VSC_H = 0x%04X,\n ", MLC_REG->YUVA_BT_VSC_H);
	ds_printk("MLC_YUVA_TP_PXW = 0x%04X,\n ", MLC_REG->YUVA_TP_PXW);
	ds_printk("MLC_YUVA_BT_PXW = 0x%04X,\n ", MLC_REG->YUVA_BT_PXW);
	ds_printk("MLC_YUVA_STX = 0x%04X,\n ", MLC_REG->YUVA_STX);
	ds_printk("MLC_YUVA_ENDX = 0x%04X,\n ", MLC_REG->YUVA_ENDX);
	ds_printk("MLC_YUVA_TP_STY = 0x%04X,\n ", MLC_REG->YUVA_TP_STY);
	ds_printk("MLC_YUVA_TP_ENDY = 0x%04X,\n ", MLC_REG->YUVA_TP_ENDY);
	ds_printk("MLC_YUVA_BT_ENDY = 0x%04X,\n ", MLC_REG->YUVA_BT_ENDY);
	ds_printk("MLC_YUVA_TP_OADR_L = 0x%04X,\n ", MLC_REG->YUVA_TP_OADR_L);
	ds_printk("MLC_YUVA_TP_OADR_H = 0x%04X,\n ", MLC_REG->YUVA_TP_OADR_H);
	ds_printk("MLC_YUVA_TP_EADR_L = 0x%04X,\n ", MLC_REG->YUVA_TP_EADR_L);
	ds_printk("MLC_YUVA_TP_EADR_H = 0x%04X,\n ", MLC_REG->YUVA_TP_EADR_H);
	ds_printk("MLC_YUVA_BT_OADR_L = 0x%04X,\n ", MLC_REG->YUVA_BT_OADR_L);
	ds_printk("MLC_YUVA_BT_OADR_H = 0x%04X,\n ", MLC_REG->YUVA_BT_OADR_H);
	ds_printk("MLC_YUVA_BT_EADR_L = 0x%04X,\n ", MLC_REG->YUVA_BT_EADR_L);
	ds_printk("MLC_YUVA_BT_EADR_H = 0x%04X,\n ", MLC_REG->YUVA_BT_EADR_H);
}

void 
MLC_Util_YUVB
(
	U8  SrcMemFdc,	// 0: Mem, 1 : FDC
	U8	PriorityB,
	U8  vMirrorTop, U8  hMirrorTop, U8 vMirrorBot, U8  hMirrorBot,
	U16 SrcWidthTop, U16 SrcHeightTop, U16 DstWidthTop, U16 DstHeightTop, U16 StrideTop,
	U16 SrcWidthBot, U16 SrcHeightBot, U16 DstWidthBot, U16 DstHeightBot, U16 StrideBot,
	U16 StartX, U16 StartY, U16 EndX, U16 EndY, U16 BottomY,
	U32 Topaddr, U32 BotAddr
)
{
	U8 		Interlace;		// 0:Progressive, 1:Interlace
	U8		Source, Priority;
	CBOOL	Div2;
	
	DPC_Initialize();

	
	if(DPC->DPC_CNTL & INTERLACE)
		Interlace = 1;		// Interlace mode
	else
		Interlace = 0;		// Not Interlace mode

	ds_printk("Interlace = %d\n", Interlace);

	MLC_Initialize(  );

	// source
	if ( SrcMemFdc == 0 )
		Source 	= MLC_YUV_FROM_MEM;
	else // if ( SrcMemFdc == 1 )
		Source 	= MLC_YUV_FROM_FDC;
	
	// Division
	if ( EndY == BottomY )
		Div2	= CFALSE;
	else //if ( EndY != BottomY )
		Div2	= CTRUE;
		
	// Priority A/B
	Priority	= ( PriorityB == 0x1 ) ? MLC_YUV_PRIO_B : MLC_YUV_PRIO_A;
			
	MLC_YUVB_EnableBotDisaply( Div2 );
	MLC_YUVB_FromData( Source );
	MLC_SetVidoeLayerPriority( Priority );
	MLC_YUVB_SetMirrorTop( hMirrorTop, vMirrorTop );
	MLC_YUVB_SetMirrorBot( hMirrorBot, vMirrorBot );
	
	MLC_YUVB_SetAddressTop
	(
		Topaddr, Topaddr
	);
	MLC_YUVB_SetAddressBot
	(
		BotAddr, BotAddr		// Bottom Region
	);

	if(Interlace == 0) {	
		if ( SrcMemFdc )
		{
			MLC_FDC_SetScale
			(
				SrcWidthTop, SrcHeightTop,	//U16 SrcPixWidth, U16 SrcPixHeight,
				DstWidthTop, DstHeightTop	//U16 DestPixWidth, U16 DestPixHeight
			);
		}
		else
		{
			MLC_YUVB_SetScaleTop
			(	
				SrcWidthTop, SrcHeightTop, StrideTop,
				DstWidthTop, DstHeightTop
			);
		}
	
		MLC_YUVB_SetScaleBot
		(	
			SrcWidthBot, SrcHeightBot, StrideBot,
			DstWidthBot, DstHeightBot
		);
		
		MLC_YUVB_SetCoordTop	
		(
			StartX, StartY,	// top sX, sY
			EndX, EndY// top eX, eY
		);
	
		MLC_YUVB_SetCoordBot	
		(
			BottomY	// U16 BotEndY);
		);
	}
	else {	// if(Interlace == 1)
		if ( SrcMemFdc )
		{
			MLC_FDC_SetScale
			(
				SrcWidthTop, SrcHeightTop,		//U16 SrcPixWidth, U16 SrcPixHeight,
				DstWidthTop, DstHeightTop		//U16 DestPixWidth, U16 DestPixHeight
			);
		}
		else
		{
			MLC_YUVB_SetScaleTop
			(	
				SrcWidthTop, SrcHeightTop/2, 2*StrideTop,
				DstWidthTop, DstHeightTop/2
			);
		}
	
		MLC_YUVB_SetScaleBot
		(	
			SrcWidthBot, SrcHeightBot/2, 2*StrideBot,
			DstWidthBot, DstHeightBot/2
		);
		MLC_YUVB_SetCoordTop	
		(
			StartX, StartY/2,	// top sX, sY
			EndX, EndY/2	// top eX, eY
		);
	
		MLC_YUVB_SetCoordBot	
		(
			BottomY/2	// U16 BotEndY);
		);
	}
	ds_printk("MLC_OVLAY_CNTR = 0x%04X,\n ", MLC_REG->OVLAY_CNTR);
	ds_printk("MLC_YUV_EFECT = 0x%04X,\n ", MLC_REG->YUV_EFECT);
	ds_printk("MLC_YUV_CNTL = 0x%04X,\n ", MLC_REG->YUV_CNTL);
	ds_printk("MLC_YUVB_TP_HSC = 0x%04X,\n ", MLC_REG->YUVB_TP_HSC);
	ds_printk("MLC_YUVB_BT_HSC = 0x%04X,\n ", MLC_REG->YUVB_BT_HSC);
	ds_printk("MLC_YUVB_TP_VSC_L = 0x%04X,\n ", MLC_REG->YUVB_TP_VSC_L);
	ds_printk("MLC_YUVB_TP_VSC_H = 0x%04X,\n ", MLC_REG->YUVB_TP_VSC_H);
	ds_printk("MLC_YUVB_BT_VSC_L = 0x%04X,\n ", MLC_REG->YUVB_BT_VSC_L);
	ds_printk("MLC_YUVB_BT_VSC_H = 0x%04X,\n ", MLC_REG->YUVB_BT_VSC_H);
	ds_printk("MLC_YUVB_TP_PXW = 0x%04X,\n ", MLC_REG->YUVB_TP_PXW);
	ds_printk("MLC_YUVB_BT_PXW = 0x%04X,\n ", MLC_REG->YUVB_BT_PXW);
	ds_printk("MLC_YUVB_STX = 0x%04X,\n ", MLC_REG->YUVB_STX);
	ds_printk("MLC_YUVB_ENDX = 0x%04X,\n ", MLC_REG->YUVB_ENDX);
	ds_printk("MLC_YUVB_TP_STY = 0x%04X,\n ", MLC_REG->YUVB_TP_STY);
	ds_printk("MLC_YUVB_TP_ENDY = 0x%04X,\n ", MLC_REG->YUVB_TP_ENDY);
	ds_printk("MLC_YUVB_BT_ENDY = 0x%04X,\n ", MLC_REG->YUVB_BT_ENDY);
	ds_printk("MLC_YUVB_TP_OADR_L = 0x%04X,\n ", MLC_REG->YUVB_TP_OADR_L);
	ds_printk("MLC_YUVB_TP_OADR_H = 0x%04X,\n ", MLC_REG->YUVB_TP_OADR_H);
	ds_printk("MLC_YUVB_TP_EADR_L = 0x%04X,\n ", MLC_REG->YUVB_TP_EADR_L);
	ds_printk("MLC_YUVB_TP_EADR_H = 0x%04X,\n ", MLC_REG->YUVB_TP_EADR_H);
	ds_printk("MLC_YUVB_BT_OADR_L = 0x%04X,\n ", MLC_REG->YUVB_BT_OADR_L);
	ds_printk("MLC_YUVB_BT_OADR_H = 0x%04X,\n ", MLC_REG->YUVB_BT_OADR_H);
	ds_printk("MLC_YUVB_BT_EADR_L = 0x%04X,\n ", MLC_REG->YUVB_BT_EADR_L);
	ds_printk("MLC_YUVB_BT_EADR_H = 0x%04X,\n ", MLC_REG->YUVB_BT_EADR_H);
}

void 
MLC_Util_RGB
(
	U8	RGBRegionNO, //MLC_RGB_RGN_1 ~ MLC_RGB_RGN_5
	U8	bpp,		// MLC_RGB_4BPP, MLC_RGB_8BPP, MLC_RGB_16BPP, MLC_RGB_24BPP
	U8	AlphaCkey,	// MLC_RGB_MIXMUX_PRI, MLC_RGB_MIXMUX_CKEY, MLC_RGB_MIXMUX_ALPHA
	U8	AlphaValue,	// 0 ~ 15
	U8	CkeyR, U8 CkeyG, U8 CkeyB,
	U16	StartX, U16 StartY, U16 EndX, U16 EndY,
	U16 SrcWidth, U16 SrcHeight, U16 DstWidth, U16 DstHeight, U16 Stride,
	U32 addr
)
{
	U8	Interlace;		// 0:Progressive, 1:Interlace

	DPC_Initialize();
	
	if(DPC->DPC_CNTL & INTERLACE)
		Interlace = 1;		// Interlace mode
	else
		Interlace = 0;		// Not Interlace mode

	ds_printk("Interlace = %d\n", Interlace);

	MLC_Initialize();
	
	MLC_RGB_SetBPP( bpp );
	MLC_RGB_SetColorKey
	(
		CkeyR,	// R
		CkeyG,	// G
		CkeyB
	);


	MLC_RGB_MixMux
	(
		RGBRegionNO,
		AlphaCkey,	// Mix or Mux
		AlphaValue & 0xf		// Alpha Value
	);

	if(Interlace == 0) {
		MLC_RGB_SetScale
		(                                                                        
			SrcWidth, SrcHeight, Stride,	// source                                                 
			DstWidth, DstHeight					// destination
		);
	
		MLC_RGB_SetCoord
		(
			RGBRegionNO,
			StartX, EndX,
			StartY, EndY
		);
	}
	else {	// if(Interlace == 1) 
		MLC_RGB_SetScale
		(                                                                        
			SrcWidth, SrcHeight/2, 2*Stride,	// source                                                 
			DstWidth, DstHeight/2					// destination
		);
	
		MLC_RGB_SetCoord
		(
			RGBRegionNO,
			StartX, EndX,
			StartY/2, EndY/2
		);
	}
	
	MLC_RGB_SetActivate
	(
		RGBRegionNO, MLC_RGB_RGN_ACT
	);


	MLC_RGB_SetAddress	(addr, addr);
	
	MLC_RGBOn (RGBRegionNO);	
}

void 
MLC_Util_HWC
(
	U8 Size,	 // ~ 64;
	U16	StartX, U16 StartY,
	U8 ForeAlpha, U8 BackAlpha,	// 0 ~ 15
	U8 rForeColor, U8 gForeColor, U8 bForeColor,
	U8 rBackColor, U8 gBackColor, U8 bBackColor,
	U32 OddAddr, U32 EvenAddr
)
{	

	MLC_Initialize( );
	
	MLC_Cursor_Configure
	(
		Size,	//U8 Size,
		ForeAlpha, BackAlpha,	// 0 ~ 15
		rForeColor, gForeColor, bForeColor,
		rBackColor, gBackColor, bBackColor,
		OddAddr, EvenAddr
	);
	
	MLC_Cursor_SetCoord	(StartX, StartY);
	MLC_HWCOn();


}

void 
MLC_Util_OSD
(
	U32 OddAddr, U32 EvenAddr
)
{	
	MLC_Initialize( );
	
	MLC_OSD_SetSrcAddress (OddAddr,EvenAddr);   
	MLC_OSDOn();

}

void 
MLC_Util_SPU
(
	U32 *pPalette, U32 Addr
)
{	
	MLC_Initialize( );
	
	MLC_SPU_SetPalette ( pPalette, 0, 32);	
	MLC_SPU_SetSrcAddress (Addr);
	MLC_SPUOn();

}

U16 MLC_get_ovlay_ctrl(void)
{
	return MLC_REG->OVLAY_CNTR;
}

EXPORT_SYMBOL(MLC_Util_YUVA);
EXPORT_SYMBOL(MLC_Util_YUVB);
EXPORT_SYMBOL(MLC_Util_OSD);
EXPORT_SYMBOL(MLC_Util_RGB);
EXPORT_SYMBOL(MLC_YUVAOn);
EXPORT_SYMBOL(MLC_YUVAOff);
EXPORT_SYMBOL(MLC_YUVBOn);
EXPORT_SYMBOL(MLC_YUVBOff);
EXPORT_SYMBOL(MLC_RGBOn);
EXPORT_SYMBOL(MLC_RGBOff);
