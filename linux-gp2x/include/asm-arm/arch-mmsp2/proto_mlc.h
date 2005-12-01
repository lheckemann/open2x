#ifndef _MLC_H_
#define _MLC_H_

#include <asm/arch/wincetype.h>

//FieldPol 
#define MLC_FID_NORM    0x0
#define MLC_FID_INV     0x1 

//ByPassGamma 
#define MLC_GAMM_BYPASS 0x0 
#define MLC_GAMM_USE    0x1 

//OSD_SPU_Pri
#define MLC_ORD_OSD     0x0
#define MLC_ORD_SPU     0x1

//Overlay Data
#define MLC_FSC_SC_ONLY     0x0
#define MLC_FSC_SC_DISP     0x1 

//SelDiv 
#define MLC_YUV_DIV_ONE     0x0 
#define MLC_YUV_DIV_TWO 0x1 

//FromData
#define MLC_YUV_FROM_MEM    0x0 
#define MLC_YUV_FROM_SC     0x1 
#define MLC_YUV_FROM_FDC    0x2 

//Priority
#define MLC_YUV_PRIO_A  0x0 
#define MLC_YUV_PRIO_B  0x1 

//HTopMirror, vTopMirror, hBottomMirror, vBottomMirror : Mirror
#define MLC_YUV_MIRROR_ON   0x1 
#define MLC_YUV_MIRROR_OFF  0x0 

//IsTop 
#define MLC_YUV_TOP     0x0 
#define MLC_YUV_BOT     0x1 

//IsRegion 
#define MLC_RGB_RGN_1   0x0 
#define MLC_RGB_RGN_2   0x1 
#define MLC_RGB_RGN_3   0x2 
#define MLC_RGB_RGN_4   0x3 
#define MLC_RGB_RGN_5   0x4 

//BPP 
#define MLC_RGB_4BPP    0x0 //4 bpp.    
#define MLC_RGB_8BPP    0x1 //8 bpp.    
#define MLC_RGB_16BPP   0x2 //16 bpp.   
#define MLC_RGB_24BPP   0x3 //24 bpp.   

//MixMux1, U8 MixMux2, U8 MixMux3, U8 MixMux4, U8 MixMux5
#define MLC_RGB_MIXMUX_PRI      0x0 
#define MLC_RGB_MIXMUX_CKEY     0x1 
#define MLC_RGB_MIXMUX_ALPHA    0x2 


//Activate 
#define MLC_RGB_RGN_ACT         0x0 
#define MLC_RGB_RGN_DISACT      0x1 

//Size 
#define MLC_HWC_SZ_32   0x20    //32x32 
#define MLC_HWC_SZ_64   0x40    //64x64 

//Coordinate 
#define MLC_COORD_1234  0x0 
#define MLC_COORD_1     0x1 
#define MLC_COORD_2     0x2 
#define MLC_COORD_3     0x3 
#define MLC_COORD_4     0x4 

//DitherOn
#define MLC_DITH_ON     0x0 
#define MLC_DITH_OFF    0x1 

//DitherType
#define MLC_DITH_TYPE_2X2   0x0 
#define MLC_DITH_TYPE_4x4   0x1 

//Contrast
#define MLC_CONTR_1_000 0x0 //Gain = 1.000  
#define MLC_CONTR_1_125 0x1 //Gain = 1.125  
#define MLC_CONTR_1_250 0x2 //Gain = 1.250  
#define MLC_CONTR_1_375 0x3 //Gain = 1.375  
#define MLC_CONTR_1_500 0x4 //Gain = 1.500  
#define MLC_CONTR_1_625 0x5 //Gain = 1.625  
#define MLC_CONTR_1_750 0x6 //Gain = 1.750  
#define MLC_CONTR_1_875 0x7 //Gain = 1.875  

void 
MLC_GetDeviceInfo
(
    void
);

void
MLC_Initialize 
(
    void
);

void
MLC_Close
(
    void
);

void
MLC_InitHardware
(
    void
);

void
MLC_CloseHardware
(
    void
);

void
MLC_SetFieldPolarity
(
    U8 FieldPol
);

void
MLC_SetOSDSPUPriority
(
    U8 OSD_SPU_Pri
);

void
MLC_SetGammaPath
(
    U8 ByPathGamma
);

void MLC_YUVAOn ( void );
void MLC_YUVAOff ( void );

void MLC_YUVBOn ( void );
void MLC_YUVBOff ( void );

void MLC_RGBOn (U8 RegionNr);
void MLC_RGBOff (U8 RegionNr);

void MLC_FSCOn ( void );
void MLC_FSCOff ( void );

void MLC_HWCOn ( void );
void MLC_HWCOff ( void );

void MLC_SPUOn ( void );
void MLC_SPUOff ( void );

void MLC_OSDOn ( void );
void MLC_OSDOff ( void );

void
MLC_FSC_SetPath (U8 DataPath);

void
MLC_YUVA_EnableBotDisaply
(
    CBOOL enb
);

void
MLC_YUVA_FromData
(
    U8 FromData
);

void
MLC_SetVidoeLayerPriority
(
    U8 Priority
);

void
MLC_YUVA_SetMirrorTop
(
    U8 hTopMirror,
    U8 vTopMirror
);

void
MLC_YUVA_SetMirrorBot
(
    U8 hBottomMirror,
    U8 vBottomMirror
);

void
MLC_YUVA_SetAddressTop
(
    U32 TopOddAddress,
    U32 TopEvenAddress
);

void
MLC_YUVA_SetAddressBot
(
    U32 BotOddAddress,
    U32 BotEvenAddress
);

void
MLC_YUVA_SetScaleTop
(
    U16 SrcPixWidth, U16 SrcPixHeight, U16 SrcStride,
    U16 DestPixWidth, U16 DestPixHeight
);

void
MLC_YUVA_SetScaleBot
(
    U16 SrcPixWidth, U16 SrcPixHeight, U16 SrcStride,
    U16 DestPixWidth, U16 DestPixHeight
);

void
MLC_YUVA_SetCoordTop
(
    U16 TopStarX, U16 TopStartY,
    U16 TopEndX, U16 TopEndY
);

void
MLC_YUVA_SetCoordBot
(
    U16 BotEndY
);

void
MLC_YUVB_EnableBotDisaply
(
    CBOOL enb
);

void
MLC_YUVB_FromData
(
    U8 FromData
);

void
MLC_YUVB_SetMirrorTop
(
    U8 hTopMirror,
    U8 vTopMirror
);

void
MLC_YUVB_SetMirrorBot
(
    U8 hBottomMirror,
    U8 vBottomMirror
);

void
MLC_YUVB_SetAddressTop
(
    U32 TopOddAddress,
    U32 TopEvenAddress
);

void
MLC_YUVB_SetAddressBot
(
    U32 BotOddAddress,
    U32 BotEvenAddress
);

void
MLC_YUVB_SetScaleTop
(
    U16 SrcPixWidth, U16 SrcPixHeight, U16 SrcStride,
    U16 DestPixWidth, U16 DestPixHeight
);

void
MLC_YUVB_SetScaleBot
(
    U16 SrcPixWidth, U16 SrcPixHeight, U16 SrcStride,
    U16 DestPixWidth, U16 DestPixHeight
);

void
MLC_YUVB_SetCoordTop
(
    U16 TopStarX, U16 TopStartY,
    U16 TopEndX, U16 TopEndY
);

void
MLC_YUVB_SetCoordBot
(
    U16 BotEndY
);

void
MLC_FDC_SetScale
(
    U16 SrcPixWidth, U16 SrcPixHeight,
    U16 DestPixWidth, U16 DestPixHeight
);

void
MLC_RGB_SetBPP
(
    U8 BPP
);

void
MLC_RGB_SetColorKey
(
    U8 rColorKey, U8 gColorKey, U8 bColorKey
);

void
MLC_RGB_MixMux  (U8 rgnNum, // 1 ~ 5 Region
                U8 MixMux,  // Mix or Mux
                U8 Alpha);  // Alpha Value

void
MLC_RGB_SetScale
(                                                     
    U16 SrcPixWidth, U16 SrcPixHeight, U16 SrcStride,                                                       
    U16 DestPixWidth, U16 DestPixHeight
);

void
MLC_RGB_SetActivate
(
    U8 rgnNum, U8 Activate
);

void
MLC_RGB_SetCoord
(
    U8 rgnNum,
    U16 StarX, U16 EndX, U16 StartY, U16 EndY
);

void
MLC_RGB_SetAddress  (U32 OddAddr, U32 EvenAddr);

void
MLC_OSD_SetSrcAddress (U32 OddAddr, U32 EvenAddr);

void    
MLC_SPU_SetSrcAddress (U32 Addr);

void
MLC_Cursor_Configure    (U8 Size,
                        U8 ForeAlpha, U8 BackAlpha,
                        U8 rForeColor, U8 gForeColor, U8 bForeColor,
                        U8 rBackColor, U8 gBackColor, U8 bBackColor,
                        U32 OddAddr, U32 EvenAddr);

void
MLC_Cursor_SetCoord (U16 hPosition, U16 vPosition);

void
MLC_LumaEnhance ( U8 Contrast, U8 Bright);

void
MLC_SetChromaEnhance    (U8 Coordinate, 
                        U16 Angle, float Gain,
                        float CbGain, float CrGain);

void
MLC_SetDither   (U8 DitherOn, U8 DitherType);

void
MLC_OSD_SetPalette 
(
    U32 *pPalette, U16 firstEntry, U16 numEntries
);

void
MLC_SPU_SetPalette
(
    U32 *pPalette, U16 firstEntry, U16 numEntries
);

void
MLC_RGB_SetPalette
(
    U32 *pPalette, U16 firstEntry, U16 numEntries
);

void    
MLC_RGB_SetGamma
(
    U32 *pTable, U16 firstEntry, U16 numEntries
);

void 
MLC_Util_YUVA
(
    U8  SrcMemScale,    // 0: Mem, 1 : Scale Processor
    U8  PriorityA,
    U8  vMirrorTop, U8  hMirrorTop, U8 vMirrorBot, U8  hMirrorBot,
    U16 SrcWidthTop, U16 SrcHeightTop, U16 DstWidthTop, U16 DstHeightTop, U16 StrideTop,
    U16 SrcWidthBot, U16 SrcHeightBot, U16 DstWidthBot, U16 DstHeightBot, U16 StrideBot,
    U16 StartX, U16 StartY, U16 EndX, U16 EndY, U16 BottomY,
    U32 Topaddr, U32 BotAddr
);

void 
MLC_Util_YUVB
(
    U8  SrcMemFdc,  // 0: Mem, 1 : FDC
    U8  PriorityB,
    U8  vMirrorTop, U8  hMirrorTop, U8 vMirrorBot, U8  hMirrorBot,
    U16 SrcWidthTop, U16 SrcHeightTop, U16 DstWidthTop, U16 DstHeightTop, U16 StrideTop,
    U16 SrcWidthBot, U16 SrcHeightBot, U16 DstWidthBot, U16 DstHeightBot, U16 StrideBot,
    U16 StartX, U16 StartY, U16 EndX, U16 EndY, U16 BottomY,
    U32 Topaddr, U32 BotAddr
);

void 
MLC_Util_RGB
(
    U8  RGBRegionNO, //MLC_RGB_RGN_1 ~ MLC_RGB_RGN_5
    U8  bpp,        // MLC_RGB_4BPP, MLC_RGB_8BPP, MLC_RGB_16BPP, MLC_RGB_24BPP
    U8  AlphaCkey,  // MLC_RGB_MIXMUX_PRI, MLC_RGB_MIXMUX_CKEY, MLC_RGB_MIXMUX_ALPHA
    U8  AlphaValue, // 0 ~ 15
    U8  CkeyR, U8 CkeyG, U8 CkeyB,
    U16 StartX, U16 StartY, U16 EndX, U16 EndY,
    U16 SrcWidth, U16 SrcHeight, U16 DstWidth, U16 DstHeight, U16 Stride,
    U32 addr
);

void 
MLC_Util_HWC
(
    U8 Size,     // ~ 64;
    U16 StartX, U16 StartY,
    U8 ForeAlpha, U8 BackAlpha, // 0 ~ 15
    U8 rForeColor, U8 gForeColor, U8 bForeColor,
    U8 rBackColor, U8 gBackColor, U8 bBackColor,
    U32 OddAddr, U32 EvenAddr
);

void 
MLC_Util_OSD
(
    U32 OddAddr, U32 EvenAddr
);

void 
MLC_Util_SPU
(
    U32 *pPalette, U32 Addr
);

U16 MLC_get_ovlay_ctrl(void);

#endif  // _MLC_H_
