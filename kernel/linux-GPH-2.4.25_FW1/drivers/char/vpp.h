/*
 *  linux/kernel/drivers/char/vpp.h
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 *  Video Post Processor include file
 */
#ifndef _VPP_H_
#define _VPP_H_

/* FDC */
typedef struct _fdc_info {
    unsigned short VSync;
    unsigned short SrcField;//0:Frame, 1:Field
    unsigned short Chroma;
    unsigned short Rotate;
    unsigned short DstField;//0:Frame, 1:Field
    unsigned short ToScaler;//1:To Scaler
    unsigned short Width;
    unsigned short Height;
    unsigned short Y_Offset;
    unsigned short CB_Offset;
    unsigned short CR_Offset;
    unsigned long DstBaseAddr;
} FDC_INFO;

typedef struct _fdc_run_info {
    unsigned short Y_Offset;
    unsigned short CB_Offset;
    unsigned short CR_Offset;
} FDC_RUN_INFO;

/* SCALER */
typedef struct _sc_info {
    unsigned char Src;
    unsigned char Dst;
    unsigned char NumOfFrame;
    unsigned char WriteForm;
    unsigned char WriteField;
    unsigned char hSrcMirror;
    unsigned char vSrcMirror;
    unsigned char hDstMirror;
    unsigned char vDstMirror;
    unsigned short SrcWidth;
    unsigned short SrcHeight;
    unsigned short SrcStride;
    unsigned short DstWidth;
    unsigned short DstHeight;
    unsigned short DstStride;
    unsigned long SrcOddAddr;
    unsigned long SrcEvenAddr;
    unsigned long DstAddr;
    unsigned short Y_Offset;
    unsigned short CB_Offset;
    unsigned short CR_Offset;
} SC_INFO;

/* MLC */
#define MLC_YUV_PRIO_A  0x0
#define MLC_YUV_PRIO_B  0x1

#define MLC_YUV_MIRROR_ON   0x1
#define MLC_YUV_MIRROR_OFF  0x0

typedef struct _yuv_info {
    unsigned char Src;      //0: Mem, 1: Scaler(YUVA), FDC(YUVB)
    unsigned char Priority; //0: A, 1: B
    unsigned char vMirrorTop;
    unsigned char hMirrorTop;
    unsigned char vMirrorBot;
    unsigned char hMirrorBot;
    unsigned short SrcWidthTop;
    unsigned short SrcHeightTop;
    unsigned short DstWidthTop;
    unsigned short DstHeightTop;
    unsigned short StrideTop;
    unsigned short SrcWidthBot;
    unsigned short SrcHeightBot;
    unsigned short DstWidthBot;
    unsigned short DstHeightBot;
    unsigned short StrideBot;
    unsigned short StartX;
    unsigned short StartY;
    unsigned short EndX;
    unsigned short EndY;
    unsigned short BottomY;
    unsigned long TopAddr;
    unsigned long BotAddr;
} YUV_INFO;

/* MLC info get ?*/
typedef struct _window_info {
    unsigned short InputWidth;
    unsigned short InputHeight;
    unsigned short WindowWidth;
    unsigned short WindowHeight;
    unsigned short DisplayWidth;
    unsigned short DisplayHeight;
} WINDOW_INFO;

/* FDC IOCTL Commands */
#define IOCTL_MMSP2_SET_FDC     _IOW('v', 0x00, FDC_INFO)
#define IOCTL_MMSP2_START_FDC   _IOW('v', 0x01, FDC_RUN_INFO) // for gplayer speed up 
#define IOCTL_MMSP2_FDC_BUSY    _IO('v', 0x02) //1:Busy
#define IOCTL_MMSP2_STOP_FDC    _IO('v', 0x03)

/* SCALER IOCTL Commands */
#define IOCTL_MMSP2_SET_SC      _IOW('v', 0x08, SC_INFO)
#define IOCTL_MMSP2_ENABLE_SC   _IOW('v', 0x09, unsigned long)//1:Enable, 0:Disable
#if 0
#define IOCTL_MMSP2_START_SC    _IO('v', 0x0A)
#else
#define IOCTL_MMSP2_START_SC    _IOR('v', 0x0A, unsigned char)// hhsong modify 051121
#endif
#define IOCTL_MMSP2_SC_BUSY     _IO('v', 0x0B) //1:Busy
#define IOCTL_MMSP2_STOP_SC     _IO('v', 0x0C)

/* MLC IOCTL Commands */
#define IOCTL_MMSP2_SET_YUVA    _IOW('v', 0x10, YUV_INFO)
#define IOCTL_MMSP2_SET_YUVB    _IOW('v', 0x11, YUV_INFO)
#define IOCTL_MMSP2_YUVA_ON     _IO('v', 0x12)
#define IOCTL_MMSP2_YUVA_OFF    _IO('v', 0x13)
#define IOCTL_MMSP2_YUVB_ON     _IO('v', 0x14)
#define IOCTL_MMSP2_YUVB_OFF    _IO('v', 0x15)
#define IOCTL_MMSP2_CALC_WINDOW _IOR('v', 0x16, WINDOW_INFO)

#define IOCTL_MMSP2_JIFFIES     _IOR('v', 0x18, unsigned long)

#define IOCTL_MMSP2_RGB_ON      _IOR('v', 0x20, unsigned long)//0 ~ 4
#define IOCTL_MMSP2_RGB_OFF     _IOR('v', 0x21, unsigned long)//0 ~ 4

// hhsong add 051025
#define IOCTL_MMSP2_DPC_VSYNC_INTERRUPT_ENABLE  _IO('v', 0x30)
#define IOCTL_MMSP2_DPC_VSYNC_INTERRUPT_DISABLE _IO('v', 0x31)


#endif //#ifndef _VPP_H_
