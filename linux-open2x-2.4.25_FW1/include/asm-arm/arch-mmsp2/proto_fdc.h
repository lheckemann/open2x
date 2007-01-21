#ifndef _PROTO_FDC_H_
#define _PROTO_FDC_H_

#include <asm/arch/wincetype.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FDC_FRAME               0
#define FDC_FIELD               1
#define FDC_TO_MEM      0
#define FDC_TO_SCMLC	1
#define FDC_SetWrMode(ToScaler)	(FDC_REG->FDC_CNTL = (ToScaler==TRUE) ? FDC_REG->FDC_CNTL | FDC_WRMOD : FDC_REG->FDC_CNTL & ~FDC_WRMOD)
// chroma = FDC_CHROMA_420 or FDC_CHROMA_422 or FDC_CHROMA_444
#define FDC_SetChroma(chroma)	(FDC_REG->FDC_CNTL = (FDC_REG->FDC_CNTL & ~FDC_CHROMA) | FInsrt(chroma,BF_FDC_CHROMA))
// angle = FDC_ROT_000 or FDC_ROT_090 or FDC_ROT_180 or FDC_ROT_270
#define FDC_SetRotation(angle)	(FDC_REG->FDC_CNTL = (FDC_REG->FDC_CNTL & ~FDC_ROT)|FInsrt(angle,BF_FDC_ROT))
#define FDC_SetFrameSize( MBX, MBY )  (FDC_REG->FDC_FRAME_SIZE = FInsrt(MBY,BF_FDC_MBY)|FInsrt(MBX,BF_FDC_MBX))
#define FDC_SetVSyncEnable()	(FDC_REG->FDC_CNTL |= FDC_VSE)
#define FDC_SetVSyncDisable()	(FDC_REG->FDC_CNTL &= ~FDC_VSE)
//#define FDC_Stop()  (FDC_REG->FDC_CNTL |= FDC_STOP)

void FDC_Initialize(void); 

void FDC_InitHardware(void);

void FDC_Close(void);

void FDC_CloseHardware(void);

void FDC_CfgSrcFrame(
	U16	IsFIELD,
	U16	LumaOffset,
	U16	CbOffset,
	U16	CrOffset
); 

void FDC_CfgDstFrame(
	U16	IsToScaler,
	U16	IsFIELD,
	U32	DestBaseAddr 
); 

void FDC_Run(void); 

void FDC_Util
(
	U16	IsVSync,		// CTRUE, CFALSE
	U16	IsSrcFIELD,		// FDC_FRAME, FDC_FIELD
	U16	Chroma,			// FDC_CHROMA_420, FDC_CHROMA_422, FDC_CHROMA_444
	U16 Rotate,			// FDC_ROT_000, FDC_ROT_090, FDC_ROT_180, FDC_ROT_270
	U16 IsDstFIELD, 	// FDC_FRAME, FDC_FIELD
	U16 IsToScaler,		// CTRUE, CFALSE
	U16 Width,			U16 Height,
	U16 LumaOffset,	U16 	CbOffset,	U16 	CrOffset,
	U32	DstBaseAddr
);

CBOOL FDC_IsBusy(void);
void FDC_Stop(void);

#ifdef __cplusplus
}
#endif

#endif // _PROTO_FDC_H_
