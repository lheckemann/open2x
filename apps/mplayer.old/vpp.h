/*              
 *  - godori <ghcstop>, www.aesop-embedded.org
 *             
 *    => Modified from Magiceyes Code. October, 2004
 *
 *    => DIGNSYS Inc. < www.dignsys.com > developing from April 2005
 *
 */

#ifndef _VPP_H_
#    define _VPP_H_


#    define FDC_CHROMA_420  0
#    define FDC_CHROMA_422  1
#    define FDC_CHROMA_444  2

#    define FDC_ROT_000 0
#    define FDC_ROT_090 1
#    define FDC_ROT_180 2
#    define FDC_ROT_270 3

typedef struct _fdc_info
{
    unsigned short  VSync;
    unsigned short  SrcField;   
    unsigned short  Chroma;
    unsigned short  Rotate;
    unsigned short  DstField;   
    unsigned short  ToScaler;   
    unsigned short  Width;
    unsigned short  Height;
    unsigned short  Y_Offset;
    unsigned short  CB_Offset;
    unsigned short  CR_Offset;
    unsigned long   DstBaseAddr;
} FDC_INFO;

typedef struct _fdc_run_info
{
    unsigned short  Y_Offset;
    unsigned short  CB_Offset;
    unsigned short  CR_Offset;
} FDC_RUN_INFO;



#    define SC_FRM_MEM  (0x0<<0)
#    define SC_FRM_MLC  (0x1<<0)
#    define SC_FRM_ISP  (0x1<<1)
#    define SC_FRM_FDC  (0x1<<2)

#    define SC_TO_MEM   0
#    define SC_TO_MLC   1

#    define SC_FOREVER  0x0
#    define SC_ONE_TIME 0x1
#    define SC_1_TIME 0x1

#    define SC_YCBYCR   0x0
#    define SC_SEP_F420 0x1
#    define SC_SEP_F422 0x2

#    define SC_WR_FIELD 0x0
#    define SC_WR_FRAME 0x1

#    define SC_DISABLE  0x0
#    define SC_ENABLE   0x1

#    define SC_EVEN_FIELD   0x0
#    define SC_ODD_FIELD    0x1

#    define SC_FID_NORM     0
#    define SC_FID_INV      1

#    define SC_MIRROR_OFF   0x0
#    define SC_MIRROR_ON    0x1

#    define SC_INT_NONE 0x0
#    define SC_INT_FDC  0x1
#    define SC_INT_SC   (0x1<<1)


#   define SC_RUN          0x01
#   define SC_STARTMEM     0x02
#   define SC_STARTDIS     0x03

typedef struct _sc_info
{
    unsigned char   Src;
    unsigned char   Dst;
    unsigned char   NumOfFrame;
    unsigned char   WriteForm;
    unsigned char   WriteField;
    unsigned char   hSrcMirror;
    unsigned char   vSrcMirror;
    unsigned char   hDstMirror;
    unsigned char   vDstMirror;
    unsigned short  SrcWidth;
    unsigned short  SrcHeight;
    unsigned short  SrcStride;
    unsigned short  DstWidth;
    unsigned short  DstHeight;
    unsigned short  DstStride;
    unsigned long   SrcOddAddr;
    unsigned long   SrcEvenAddr;
    unsigned long   DstAddr;
    unsigned short  Y_Offset;
    unsigned short  CB_Offset;
    unsigned short  CR_Offset;
} SC_INFO;





#    define MLC_YUV_PRIO_A  0x0
#    define MLC_YUV_PRIO_B  0x1

#    define MLC_YUV_MIRROR_ON   0x1
#    define MLC_YUV_MIRROR_OFF  0x0

typedef struct _yuv_info
{
    unsigned char   Src;        
    unsigned char   Priority;   
    unsigned char   vMirrorTop;
    unsigned char   hMirrorTop;
    unsigned char   vMirrorBot;
    unsigned char   hMirrorBot;
    unsigned short  SrcWidthTop;
    unsigned short  SrcHeightTop;
    unsigned short  DstWidthTop;
    unsigned short  DstHeightTop;
    unsigned short  StrideTop;
    unsigned short  SrcWidthBot;
    unsigned short  SrcHeightBot;
    unsigned short  DstWidthBot;
    unsigned short  DstHeightBot;
    unsigned short  StrideBot;
    unsigned short  StartX;
    unsigned short  StartY;
    unsigned short  EndX;
    unsigned short  EndY;
    unsigned short  BottomY;
    unsigned long   TopAddr;
    unsigned long   BotAddr;
} YUV_INFO;




typedef struct _window_info
{
    unsigned short  InputWidth;
    unsigned short  InputHeight;
    unsigned short  WindowWidth;
    unsigned short  WindowHeight;
    unsigned short  DisplayWidth;
    unsigned short  DisplayHeight;
} WINDOW_INFO;


#    define IOCTL_MMSP2_SET_FDC     _IOW('v', 0x00, FDC_INFO)
#    define IOCTL_MMSP2_START_FDC   _IOW('v', 0x01, FDC_RUN_INFO)
#    define IOCTL_MMSP2_FDC_BUSY    _IO('v', 0x02)
#    define IOCTL_MMSP2_STOP_FDC    _IO('v', 0x03)

#    define IOCTL_MMSP2_SET_SC      _IOW('v', 0x08, SC_INFO)
#    define IOCTL_MMSP2_ENABLE_SC   _IOW('v', 0x09, unsigned long)
#    define IOCTL_MMSP2_START_SC    _IOR('v', 0x0A, unsigned char) 
#    define IOCTL_MMSP2_SC_BUSY     _IO('v', 0x0B)
#    define IOCTL_MMSP2_STOP_SC     _IO('v', 0x0C)

#    define IOCTL_MMSP2_SET_YUVA    _IOW('v', 0x10, YUV_INFO)
#    define IOCTL_MMSP2_SET_YUVB    _IOW('v', 0x11, YUV_INFO)
#    define IOCTL_MMSP2_YUVA_ON     _IO('v', 0x12)
#    define IOCTL_MMSP2_YUVA_OFF    _IO('v', 0x13)
#    define IOCTL_MMSP2_YUVB_ON     _IO('v', 0x14)
#    define IOCTL_MMSP2_YUVB_OFF    _IO('v', 0x15)
#    define IOCTL_MMSP2_CALC_WINDOW _IOR('v', 0x16, WINDOW_INFO)
#    define IOCTL_MMSP2_JIFFIES     _IOR('v', 0x18, unsigned long)
#    define IOCTL_MMSP2_RGB_ON      _IOR('v', 0x20, unsigned long)
#    define IOCTL_MMSP2_RGB_OFF     _IOR('v', 0x21, unsigned long)
                                                                
#define IOCTL_MMSP2_DPC_VSYNC_INTERRUPT_ENABLE  _IO('v', 0x30)
#define IOCTL_MMSP2_DPC_VSYNC_INTERRUPT_DISABLE _IO('v', 0x31)


#endif                          
