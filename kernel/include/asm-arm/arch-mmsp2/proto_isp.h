#ifndef _PROTO_ISP_H_
#define _PROTO_ISP_H_

#include <asm/arch/wincetype.h>

#ifdef __cplusplus
extern "C" {
#endif

//InputDataFormat : Input Data Format.
#define	ISP_F422_GPCH			0x0	//ITU-R BT 601 (4:2:2)Data  = GPIOC[15:0]Clock = GPIOH[4].
#define	ISP_F656I_GPIJK		0x1	//ITU-R BT 656 InterlaceData[7:0] = { GPIOI[15:14], GPIOI[3:1], GPIOJ[15:13] }.Clock = GPIOK[2].
#define	ISP_F656I_GPM			0x2	//ITU-R BT 656 InterlaceData[7:0] = GPIOM[8:1].Clock = GPIOM[0].
#define	ISP_F656I_GPCH		0x3	//ITU-R BT 656 InterlaceData[7:0]  = GPIOC[7:0]Clock = GPIOH[4].
#define	ISP_F656P_GPIJK		0x4	//ITU-R BT 656 ProgressiveData[7:0] = { GPIOI[15:14], GPIOI[3:1], GPIOJ[15:13] }.Clock = GPIOK[2].
#define	ISP_F656P_GPM			0x5	//ITU-R BT 656 ProgressiveData[7:0] = GPIOM[8:1].Clock = GPIOM[0].
#define	ISP_F656P_GPCH		0x6	//ITU-R BT 656 ProgressiveData[7:0]  = GPIOC[7:0]Clock = GPIOH[4].
#define	ISP_F656L_GPIJK		0x7	//ITU-R BT 656 LikeData[7:0] = { GPIOI[15:14], GPIOI[3:1], GPIOJ[15:13] }.Clock = GPIOK[2].
#define	ISP_F656L_GPM			0x8	//ITU-R BT 656 ProgressiveData[7:0] = GPIOM[8:1].Clock = GPIOM[0].
#define	ISP_F656L_GPCH		0x9	//ITU-R BT 656 ProgressiveData[7:0]  = GPIOC[7:0]Clock = GPIOH[4].
#define	ISP_BAYER_GPCH		0xa	//External Bayer RGBData[9:0] = GPIOC[9:0].Clock = GPIOH[4].
#define	ISP_PRGB_GPIJCH		0xb	//External Parallel RGB (24bit )R[7:0]=  { GPIOI[15:14], GPIOI[3:1], GPIOJ[15:13] }.G[7:0] =  GPIOC[15:8]B[7:0] =  GPIOC[7:0]Clock = GPIOH[4].
#define	ISP_SRGB_GPCH			0xc	//External Serial RGB (10bit )Data[7:0] = GPIOC[9:0],Clock = GPIOH[4].

// Enable Mode
#define	ISP_ENABLE				0x1
#define	ISP_DISABLE				0x0

// UseGamma
#define	ISP_GAMMA_USE			0x0
#define	ISP_GAMMA_BYPASS	0x1

// MasterMode : Master/Slave
#define	ISP_MASTER				0x0
#define	ISP_SLAVE					0x1

//SrcClock, DestClock : Source/Destination Clock
#define	ISP_CLK_DIV_NOR		0x0
#define	ISP_CLK_DIV_INV		0x1
#define	ISP_CLK_SYS				0x2

//HorSyncPol, VerSyncPol : Polarity
#define	ISP_SYNC_POL_NOR	0x0
#define	ISP_SYNC_POL_INV	0x1

//VerSyncStPnt : Vertical Sync Starting Point  Type
#define	ISP_VSYNC_RIS			0x0	//Rising Edge Internal Vertical Sync
#define	ISP_VSYNC_FAL			0x1	//Falling Edge Internal Vertical Sync

//GenField : Internal Field Signal
#define	ISP_FIELD_F656		0x0	//ITU-R BT 656 Field
#define	ISP_FIELD_HV			0x1	//External H/V Sync Field

//ExtBitWidth : External Data Bit Width
#define	ISP_EXT_BIT_8			0x0	//8 bits per data.
#define	ISP_EXT_BIT_10		0x1	//10 bits per data.

//MemBitWidth : Memory Interface Bit Width
#define	ISP_MEM_BIT_8	0x0	//8 bits per data.
#define	ISP_MEM_BIT_16	0x1	//16 bits per data.

//SrcRGBBPP, DestRGBBPP : Source/ Destination Data¿« bits per pixel
#define	ISP_RGB_8BPP	0x0	//8bits Bayer RGB.
#define	ISP_RGB_16BPP	0x1	//16bits Bayer RGB.
#define	ISP_RGB_24BPP	0x2	//24bits RGB.
#define	ISP_RGB_48BPP	0x3	//48bits RGB.

//BayerRGBArray
#define	ISP_BAY_GB	0x0
#define	ISP_BAY_BG	0x1
#define	ISP_BAY_RG	0x2
#define	ISP_BAY_GR	0x3


//rRecov, gRecov, bRecov
#define	ISP_RECV_MEDIAN	0x0
#define	ISP_RECV_AVLR	0x1
#define	ISP_RECV_AVUD	0x2
#define	ISP_RECV_AVALL	0x3

//RGBOrder
#define	ISP_ORD_RGB	0x0	//R,G,B
#define	ISP_ORD_BRG	0x1	//B,R,G
#define	ISP_ORD_GBR	0x2	//G,B,R
#define	ISP_ORD_BGR	0x3	//B,G,R


//ISP_SetVideoRange
#define	ISP_VID_255	0x0	//Y ( 0 ~ 255 ), Cb/Cr ( 0 ~ 255 ).
#define	ISP_VID_220	0x1	//Y ( 16 ~ 235 ), Cb/Cr ( 16 ~ 240 ).

//PathMode
#define	ISP_EXT_ENH_YCBCR	0x0	//External Data Y/C Enhancement Y/Cb/Cr format
#define	ISP_EXT_BYP_YCBCR	0x1	//External Data Y/C Enhancement Y/Cb/Cr format
#define	ISP_EXT_BYP_BRGB	0x2	//External Bayer RGB Data Format Memory
#define	ISP_EXT_BYP_SRGB	0x3	//External Serial RGB Data Format Memory
#define	ISP_EXT_BYP_PRGB	0x4	//External Parallel RGB Data Format Memory
#define	ISP_MEM_ENH_YCBCR	0x5	//Memory RGB Y/C Enhancement Y/Cb/Cr format
#define	ISP_MEM_BYP_YCBCR	0x6	//Memory RGB Y/C Enhancement Y/Cb/Cr format
#define	ISP_MEM_BYP_RGB		0x7	//Memory Bayer RGB Memory

//CrCbDeciFtOnOff
#define	ISP_C_DECI_OFF	0x0	//Cr/Cb Decimation Filter Off
#define	ISP_C_DECI_ON	0x1	//Cr/Cb Decimation Filter On

//CbCrSwap
#define	ISP_CRCBSWAP_ON		0x1	//Cb Cr Swap on
#define	ISP_CRCBSWAP_OFF	0x0	//Cb Cr Swap off

//ContrastGain
#define	ISP_CNTRAST_0_25	0x0	//Gain = 0.25
#define	ISP_CNTRAST_0_50	0x1	//Gain = 0.50
#define	ISP_CNTRAST_0_75	0x2	//Gain = 0.75
#define	ISP_CNTRAST_1_00	0x3	//Gain = 1.00
#define	ISP_CNTRAST_1_25	0x4	//Gain = 1.25
#define	ISP_CNTRAST_1_50	0x5	//Gain = 1.50
#define	ISP_CNTRAST_1_75	0x6	//Gain = 1.75
#define	ISP_CNTRAST_2_00	0x7	//Gain = 2.00

//EdgeGain
#define	ISP_EDGE_0_0	0x0	//Gain = 0.0
#define	ISP_EDGE_1_0	0x1	//Gain = 1.0
#define	ISP_EDGE_1_5	0x2	//Gain = 1.5
#define	ISP_EDGE_2_0	0x3	//Gain = 2.0
#define	ISP_EDGE_2_5	0x4	//Gain = 2.5
#define	ISP_EDGE_3_0	0x5	//Gain = 3.0
#define	ISP_EDGE_3_5	0x6	//Gain = 3.5
#define	ISP_EDGE_4_0	0x7	//Gain = 4.0

//MainGain
#define	ISP_MAIN_0_00	0x0	//Gain = 0.00
#define	ISP_MAIN_0_50	0x1	//Gain = 0.50
#define	ISP_MAIN_0_75	0x2	//Gain = 0.75
#define	ISP_MAIN_1_00	0x3	//Gain = 1.00
#define	ISP_MAIN_1_25	0x4	//Gain = 1.25
#define	ISP_MAIN_1_50	0x5	//Gain = 1.50
#define	ISP_MAIN_1_75	0x6	//Gain = 1.75
#define	ISP_MAIN_2_00	0x7	//Gain = 2.00

// ClipOn
#define	ISP_CLIP_OFF	0x0	// Clip OFF
#define	ISP_CLIP_ON		0x1	// Clip ON

//ClipGain
#define	ISP_CLIP_0_00	0x0	//Gain = 0.00
#define	ISP_CLIP_0_125	0x1	//Gain = 0.125
#define	ISP_CLIP_0_250	0x2	//Gain = 0.250
#define	ISP_CLIP_0_375	0x3	//Gain = 0.375
#define	ISP_CLIP_0_500	0x4	//Gain = 0.500
#define	ISP_CLIP_0_625	0x5	//Gain = 0.625
#define	ISP_CLIP_0_750	0x6	//Gain = 0.750
#define	ISP_CLIP_0_875	0x7	//Gain = 0.875

//Coordinate
#define	ISP_COORD_1234	0x0
#define	ISP_COORD_1		0x1
#define	ISP_COORD_2		0x2
#define	ISP_COORD_3		0x3
#define	ISP_COORD_4		0x4

//SuppressOn
#define	ISP_SUPPRES_ON	0x1	//Color Suppress on.
#define	ISP_SUPPRES_OFF	0x0	//Color Suppress off.

//SuppressGain
#define	ISP_SUPPRES_0_00	0x0	//Gain = 0.00
#define	ISP_SUPPRES_0_125	0x1	//Gain = 0.125
#define	ISP_SUPPRES_0_250	0x2	//Gain = 0.250
#define	ISP_SUPPRES_0_375	0x3	//Gain = 0.375
#define	ISP_SUPPRES_0_500	0x4	//Gain = 0.500
#define	ISP_SUPPRES_0_625	0x5	//Gain = 0.625
#define	ISP_SUPPRES_0_750	0x6	//Gain = 0.750
#define	ISP_SUPPRES_0_875	0x7	//Gain = 0.875

//InterruptEnable
#define	ISP_INT_NONE	0x0	//Interrupt Disable
#define	ISP_INT_HSYNC	0x1	//Horizontal Sync Interrupt Enable
#define	ISP_INT_VSYNC	0x2	//Vertical Sync Interrupt Enable

//ISP Block Interrupt Type
//hInterruptType, vInterruptType
#define	ISP_INT_SYNC_RIS	0x0	//Vertical /Horizontal Sync Rising Interrupt
#define	ISP_INT_SYNC_FAL	0x1	//Vertical /Horizontal Sync Falling Interrupt

typedef struct {
	U8	PathMode;

	U8	InputDataFormat;
	U8	MasterMode;
	U8	SrcClock;
	U8	DestClock;
	U8	GenField;
	U8	VerSyncStPnt;
	U8	HorSyncPol;
	U8	VerSyncPol;
	U16	HorSyncPeriod;
	U8	HorSyncPulse;
	U8	HorEAVtoSyncStart;
	U16	VerSyncPeriod;
	U16	VerSyncPulse;
	U8	VerEAVtoSyncStart;

	U16	PreStartX;
	U16 PreWidth;
	U16 PreStartY;
	U16 PreHeight;
	U16 AvStartX;
	U16 AvWidth;
	U16 AvStartY;
	U16 AvHeight;

	U8	ExtBitWidth;
	U8	MemBitWidth;
	U8	SrcRGBBPP;
	U16	SrcRGBPixWid;
	U8	DestRGBBPP;
	U16	DestRGBPixWid;
	U8	BayerRGBArray;
	U8	rRecov;
	U8	gRecov;
	U8	bRecov;
	U8	RGBOrder;
	U8	CrCbDeciFtOnOff;
	U8	ChromaDelay;
	U8	CbCrSwap;
	U8	VideoRange;
	U8	UseGamma;
	U8	vInterruptType;
	U8	hInterruptType;
}  ISP_INFO;

void
ISP_Initialize (void);

void
ISP_CloseHardware(void);

void
ISP_SetInputDataFormat	(U8	InputDataFormat);

void
ISP_SetMasterMode	(U8 MasterMode);

void
ISP_Clock	(U8 SrcClock, U8 DestClock );

void
ISP_SetSync
(
			U16 GenField,
			U16 VerSyncStPnt,
			U16 HorSyncPol, U16 VerSyncPol,
			U16 HorSyncPeriod, U16 HorSyncPulse, U16 VerEAVtoSyncStart,
			U16 VerSyncPeriod, U16 VerSyncPulse,U16 HorEAVtoSyncStart
);

void ISP_ExtDataBitWidth	(U8 ExtBitWidth );

void ISP_MemDataBitWidth	(U8 MemBitWidth );

void
ISP_SetMemRGBPixelWidth	(U8 SrcRGBBPP, U16 SrcRGBPixWid,
						U8 DestRGBBPP , U16 DestRGBPixWid);

void ISP_SetBayerRGBArray	(U8 BayerRGBArray );

void
ISP_SetRGBRecovType	(U8 rRecov, U8 gRecov, U8 bRecov );

void
ISP_RGBOrder	(U8 RGBOrder );

void
ISP_SetCrCbDeciFtOn	( U8 CrCbDeciFtOnOff);

void ISP_SetChromaDelay (U8 ChromaDelay);

void
ISP_SetCbCrSwap	(U8 CbCrSwap);

void
ISP_SetVideoRange	(U8 VideoRange);

void
ISP_UseGamma	(U8 UseGamma);

void
ISP_SetGammaAddress	( U16 Addr );

void
ISP_WriteGammaData	( U16 Data );

U16
ISP_ReadGammaData	( U16 Data );

void
ISP_SetGamma ( U16 *pGamma );

void
ISP_SetOperMode	(U8 PathMode );

void
ISP_EnableISP( U8 Enable );

void
ISP_StartISP	( void );

void
ISP_SetPreWindow
(
	U16 RGBHorStart, U16 RGBHorWidth,
	U16 RGBVerStart, U16 RGBVerHeight
);

void
ISP_SetAvWindow
(
	U16 AVHorStart, U16 AVHorWidth,
	U16 AVVerStart, U16 AvVerHeight
);

void ISP_SetRGBAddress	(U32 SrcAddr, U32 DestAddr, U8 OpMode );

CBOOL
ISP_FrameIsDone ( void );

void
ISP_AE_Setup	( U16 HorStart, U16 HorWidth,
				U16 VerStart, U16 VerHeight,
				U8 LowerLim , U8 UpperLim);

void
ISP_AE_On ( void );

void
ISP_AE_Off ( void );

void
ISP_AE_GetInfo	( U32 *pAEInfo );

void
ISP_WB_Setup	(U8 LowerLimLuma, U8 UpperLimLuma,
				U16 *pCbCrBlockValid);

void
ISP_WB_On ( void );

void
ISP_WB_Off ( void );

void
ISP_WB_GetInfo	( U32 *pNoCbCr, U32 *pACCCb, U32 *pACCCr);

void
ISP_AF_Setup	( U16 HorStart, U16 HorWidth,
				U16 VerStart, U16 VerHeight);

void
ISP_AF_On ( void );

void
ISP_AF_Off ( void );

void
ISP_AF_GetInfo	( U32 *pAccData);

void
ISP_ContrastSetup 	(U8 LowerLim, U8 UpperLim);

void
ISP_ContrastOn ( void );

void
ISP_ContrastOff ( void );

void
ISP_ContrastGetInfo	( U32 *pNoLowLuma, U32 *pDataLowLuma,
					U32 *pNoUppLuma, U32 *pDataUppLuma);

void
ISP_SetLumaEnhance	(U8 ContrastGain,
					U8 EdgeGain, U8 MainGain,
					U8 Bright,
					U8 SliceLim );

void
ISP_SetClipSuppress
(
	U8 ClipOn, U8 ClipKnee, U8 ClipLim, U8 ClipGain,
	U8 SuppressOn, U8 SuppressGain
);

CBOOL
ISP_IsOddField (void);

void
ISP_SetInterruptEnable	(U8 InterruptEnable);

U8
ISP_GetInterruptEnable	(void);

U8
ISP_GetInterruptPend	(void);

void
ISP_ClrInterruptPend	(U8 ClrPend);

void
ISP_SetInterruptType	(U8 hInterruptType, U8 vInterruptType);

void ISP_Util(ISP_INFO* isp_param);

#ifdef __cplusplus
}
#endif

#endif	// _PROTO_ISP_H
