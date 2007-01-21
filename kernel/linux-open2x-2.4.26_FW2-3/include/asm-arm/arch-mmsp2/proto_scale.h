#ifndef _SC_H_
#define _SC_H_

#include <asm/arch/wincetype.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SC_FRM_MEM	0
#define SC_FRM_MLC	1
#define SC_FRM_ISP	2
#define SC_FRM_FDC	3

#define SC_TO_MEM	0
#define SC_TO_MLC	1

//NumOfFrame
#define SC_FOREVER	0x0
#define SC_1_TIME   0x1

//WriteForm
#define SC_YCBYCR	0x0
#define SC_SEP_F420	0x1
#define SC_SEP_F422	0x2

//WriteField
#define SC_WR_FIELD	0x0
#define SC_WR_FRAME	0x1

//Enable Mode
//#define	SC_DISABLE	0x0
//#define	SC_ENABLE	0x1

//Status
//#define	SC_IDLE		0x0
//#define	SC_BUSY		0x1

//Field Status
//#define	SC_EVEN_FIELD		CFALSE
//#define	SC_ODD_FIELD		CTRUE

//Display or ISP
#define	SC_FID_NORM		0
#define	SC_FID_INV		1

// Interrupt
#define	SC_INT_NONE	0x0
#define	SC_INT_FDC	0x1
#define	SC_INT_SC	0x1<<1

#define SC_MIRROR_ON    1
#define SC_MIRROR_OFF   0

// hhsong add
#define SC_RUN          0x01
#define SC_STARTMEM     0x02
#define SC_STARTDIS     0x03

void SC_Initialize (void);

void SC_InitHardware (U8 LumaDelay, U8 ChromaDelay, U8 MemRdWordSz, U8 Occurance);

void SC_SelDataPath ( U8 SelSource, U8 SelDest);

void SC_SetOperMode (U8 NumOfFrame, U8 WriteForm, U8 WriteField );

void SC_EnableScale (CBOOL Enable);

void SC_SetFieldPolarity (U8 FieldPol);

//void SC_StartScale ( void );
void SC_StartScale ( unsigned char );   // hhsong modify

void SC_StopScale ( void );

void SC_StartDering ( void );

void SC_StopDering ( void );

CBOOL SC_IsBusy (void);

CBOOL SC_IsIspOddField (void);

CBOOL SC_IsDispOddField (void);

void SC_SetSource (U32 SrcEvenAddr, U32 SrcOddAddr, U16 Stride);

void SC_SetDest1D (U32 DstAddr, U16 Stride);

void SC_SetDest2D (U16 LumaOffset, U16 CbOffset, U16 CrOffset);

void SC_SetScale (U16 SrcWidth, U16 SrcHeight,U16 DestWidth, U16 DestHeight);

void SC_SetCoarseDownScale (U16 SrcWidth, U16 SrcHeight, U16 DestWidth, U16 DestHeight);

void SC_SetMirror (U8 hSrcMirror, U8 vSrcMirror, U8 hDestMirror, U8 vDestMirror);

void SC_SetInterruptEnable	(U8 InterruptEnable);

U8 SC_GetInterruptEnable (void);

U8 SC_GetInterruptPend (void);

void SC_ClrInterruptPend (U8 ClrPend);

void
SC_Util
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
);

#ifdef __cplusplus
}
#endif

#endif	// _SC_H
