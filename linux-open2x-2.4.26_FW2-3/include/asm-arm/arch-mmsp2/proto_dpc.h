#ifndef _PROTO_DPC_H_
#define _PROTO_DPC_H_

#include <asm/arch/wincetype.h>

// Naming Rules...
// HV  --> HSYNC and VSYNC
// SLC --> SLAVE
// HS  --> HSYNC
// VS  --> VSYNC
// CH  --> CLKH
// CV  --> CLKV
// XDF --> XDOFF
// MSK --> MASK
// BNK --> BLANK
// PLS --> PULSE
// FLD --> FIELD
// POL --> Polarity
// ATV --> Active Enable
// WID --> WIDTH
// SFT --> SHIFT
// CFG --> CONFIGURATION
// LVL --> LEVEL
// SRC --> SOURCE

typedef enum {
	DPC_RGB_444       , // 0;
 	DPC_RGB_565       , // 1;
 	DPC_RGB_666       , // 2;
 	DPC_RGB_888       , // 3;
 	DPC_MRGB_555      , // 4;
 	DPC_MRGB_565      , // 5;
 	DPC_MRGB_888A     , // 6;
 	DPC_MRGB_888B     , // 7;
 	DPC_YCBCR_CCIR656 , // 8;   // EAV/SAV Sync
 	DPC_YCBCR_CCIR601 , // 9;   // EAV/SAV Sync
 	DPC_YCBCR_3x8     , // 10;  // EAV/SAV Sync
 	DPC_YCBCR_3x10      // 11;  // EAV/SAV Sync
} DPC_OPER_MODE;

/*
 Philip : 2003-10-31
 *********************************************************************************
 Warning : C2485W: ANSI C forbids bit field type 'unsigned short' ....
 *********************************************************************************

 In ANSI C, bit field type should be a int or unsigned int. But If we use unsigned int
 in MMSP20 Chip, I/O Master tries to reading integer.
 if I/O Device is assigned to 16 bit address. even if we want to bit field of one register,
 I/O Master always read or write two times without increasing the address.
 It could be problem in case to change the bit filed of each register.

 So We use unsigned short int in bit-field definition.
 In this case, I/O Master just read one 16 bit register and write one 16 bit register.
*/

U32
DPC_GetPhyAddress(void);

void
DPC_GetDeviceInfo(
	void
);

void
DPC_Initialize
(
	void
);

void
DPC_InitHardware
(
    	DPC_OPER_MODE  	Mode,
    	U16   	IsInterlace,          // Prograssive or Interlace Mode.
    	U16  	IsPALMode,            // It is only valid when DOT is YCbCr Mode
    	U16  	IntEn,                // Interrupt Enable
	U16   	IntHSyncEdgePos,	// Interrupt Edge Position in HSYNC.
	U16   	IntVSyncEdgePos,	// Interrupt Edge Position in HSYNC.
    	U16   	ClockSource           // Clock Configuration.
);

void
DPC_Run(void);

void
DPC_Stop(void);

void
DPC_Close(void);

void
DPC_CloseHardware(void);

#define DPC_SetVSyncFieldEnable() 	(DPC->DPC_V_SYNC |= VSFLDEN)
#define DPC_SetVSyncFieldDisable() 	(DPC->DPC_V_SYNC &= ~VSFLDEN)
#define DPC_SetEavSavEnable()		(DPC->DPC_CNTL |= ESAVEN)
#define DPC_SetEavSavDisable()		(DPC->DPC_CNTL &= ~ESAVEN)


void
DPC_SetFrameSize
(
	U16 	Width,
	U16 	Height
);

void
DPC_SetHSyncParm
(
	U16     HorSyncWidth,
	U16     HorFrontPorch,
	U16     HorBackPorch,
	CBOOL   HorPolarity
);

void
DPC_SetVSyncParm
(
	U16 	VerSyncWidth,
	U16 	VerFrontPorch,
	U16 	VerBackPorch,
	CBOOL   VerPolarity
);


void
DPC_SetDEParm
(
	U32		DEMode,
	U16		DEPulseWidth,
	U16		DEShift,
	CBOOL	DEPolarity
);

void
DPC_SetPSParm
(
    CBOOL 	PSEn,
    U16  	PSRisingPos,
    U16  	PSFallingPos,
    CBOOL   PSPolarity
);

void
DPC_SetFGParm
(
    CBOOL   FGEn,
    U16  	FGWidth,
    CBOOL   FGPolarity
);


void
DPC_SetLPParm
(
    CBOOL   LPEn,
    U16  	LPWidth,
    U16  	LPFrontPorch,
    U16  	LPBackPorch
);

void
DPC_SetPOLParm
(
    CBOOL   POLEn,
    CBOOL   POLVerticalToggleEn,
    U16  	POLTogglePos,
    U16  	POLToggleArea,
    U16  	POLPeriod,
    CBOOL   POLPolarity
);

void
DPC_SetCLKVParm
(
    CBOOL   CLKVEn,
    U16  	CLKVFallingPos,   // T21
    U16  	CLKVRisingPos,    // T22
    U16  	CLKVRisingArea,
    CBOOL   CLKVPolarity
);

void
DPC_SetCLKHParm
(
    CBOOL   CLKHEn,
    U8   	CLKHPhase,
    CBOOL   CLKHPolarity
);

U16
DPC_GetInterruptPend(void);

void
DPC_ClrInterruptPend
(
    U16 IntFlag
);

U16
DPC_GetInterruptEnable(void);

void
DPC_SetInterruptEnable
(
    U16 	IntEn,
    CBOOL   IntHSyncEdgePos,	// Interrupt Edge Position in HSYNC.
	CBOOL   IntVSyncEdgePos 	// Interrupt Edge Position in HSYNC.
);

void
DPC_UTIL_DATA_INV(CBOOL DataPolarity);


void
DPC_UTIL_HVSYNC
(
	U8      DataType,
	U16     Width,
	U16     Height,
	U16     HorSyncWidth,
	U16     HorFrontPorch,
	U16     HorBackPorch,
	CBOOL   HorPolarity,
	U16     VerSyncWidth,
	U16     VerFrontPorch,
	U16     VerBackPorch,
	CBOOL   VerPolarity
);

void
DPC_UTIL_HVSYNC_GPX320240
(
	U8      DataType,
	U16     Width,
	U16     Height,
	U16     HorSyncWidth,
	U16     HorFrontPorch,
	U16     HorBackPorch,
	CBOOL   HorPolarity,
	U16     VerSyncWidth,
	U16     VerFrontPorch,
	U16     VerBackPorch,
	CBOOL   VerPolarity,
	CBOOL	ClkPolarity,
	CBOOL	DePolarity,
	CBOOL	DataPolarity
);


void
DPC_UTIL_HVSYNC_YPbPr2x8Mode
(
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
	CBOOL	IsInterlaceMode  // VSync Filed Enable/Disable.
);

void DPC_UTIL_HVSYNC_SMPTE296M_601(void);

void
DPC_UTIL_CCIR656
(
    U8 NtscPalMode,
    U16 ExtClk
);

void
DPC_UTIL_CCIR601
(
    U8  NtscPalMode,
    U16 ExtClk
);

#define IS_PROGRESSIVE		0
#define IS_INTERLACE			1

#define DPC_SYNC_HV			0
#define DPC_SYNC_EAVSAV		1
#define DPC_SYNC_COMP		2

#define DPC_USE_EXTCLK		1
#define DPC_USE_PLLCLK		2

#define DPC_VSYNC_INT_EN		0x0002
#define DPC_HSYNC_INT_EN		0x0001
#define DPC_VSYNC_INT_FLG		0x0002
#define DPC_HSYNC_INT_FLG		0x0001

#define DPC_MASTER_MODE		0
#define DPC_SLAVE_MODE		1

#define DPC_PROGRESSIVE		0
#define DPC_INTERLACE		1

#define DPC_NTSC_MODE		0
#define DPC_PAL_MODE		1

// Data Enable
#define DPC_DE_DE			0
#define DPC_DE_STH			1
#define DPC_DE_PULSE		2

#define DPC_DE_SHIFT_ZERO		0
#define DPC_DE_SHIFT_ONE		1
#define DPC_DE_SHIFT_TWO		2
#define DPC_DE_SHIFT_THREE	3
#define DPC_DE_SHIFT_M_ONE	9
#define DPC_DE_SHIFT_M_TWO	10
#define DPC_DE_SHIFT_M_THREE	11

#endif // _PROTO_DPC_H_

