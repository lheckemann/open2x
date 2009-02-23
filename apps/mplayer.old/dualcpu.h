/*              
 *  - godori <ghcstop>, www.aesop-embedded.org
 *             
 *    => Modified from Magiceyes code. Oct. 2004
 *
 *    => DIGNSYS Inc. < www.dignsys.com > developing from April 2005
 *
 */

#ifndef _MMSP2_DUALCPU_H_
#    define _MMSP2_DUALCPU_H_

#    include "wincetype.h"
#    define VIDEO_FW_041126		1
#define	MP_CMD_NULL			0
#define	MP_CODEC_NULL		-1
#define	MP_CODEC_MP4D		0
#define	MP_CODEC_MP4D_DIVX3	1
#define	MP_CODEC_H263D		2
#define	MP_CODEC_MP2D		3
#define	MP_CODEC_MJPD		4
#define	MP_CODEC_MP4E		5
#define	MP_CODEC_WMVD		6
#define MP_CODEC_MP3D       7
#define	MP_CODEC_MP1D		9
#define	MP_CODEC_MJPE		10
#define	MP_SYSTEM_INIT		8
#define	MP_CMD_INIT					1
#define MP_CMD_INIT_BUF     		16
#define	MP_CMD_RUN					2
#define	MP_CMD_RELEASE				4
#define	MP_CMD_GETINFO				6
#define	MP_CMD_940IDLE				5
#define	MP_CMD_INIT_FOR_SEEK		7
#define	MP_CMD_RELEASE_FOR_SEEK		8
#define MP_CMD_DISPLAY      		17
#define	MP_CMD_ACK					5
#define	MP_CMD_FAIL					15
#define	CODEC_VER_DIVX_311	311
#define	CODEC_VER_DIVX_500	500
#define	CODEC_VER_XVID		900


typedef struct _MP2D_INIT_PARAM_
{
    U16             Command;
    U16             CodecType;
    U16             Width;
    U16             Height;
    U16             CodecVersion;
    U16             RL_L;
    U16             RL_H;
    U16             RL_Length;  
    U16             StreamBufferPaddrL;
    U16             StreamBufferPaddrH;
} MP2D_INIT_PARAM;


typedef struct _MP4E_INIT_PARAM_
{
    U16             Command;
    U16             CodecType;
    U16             Width;
    U16             Height;
    U16             CodecVersion;
    U16             Framerate;
    U16             RcPeriod;
    U16             RcReactionPeriod;
    U16             RcReactionRation;
    U16             MaxKeyInterval;
    U16             MaxQuantizer;
    U16             MinQuantizer;
    U16             SearchRrange;
} MP4E_INIT_PARAM;


typedef struct _MJPD_INIT_PARAM_
{
    U16             Command;
    U16             CodecType;
} MJPD_INIT_PARAM;


typedef struct _MP2D_RUN_PARAM_
{
    U16             Command;
    U16             CodecType;
    U16             StreamBufferPaddrL;
    U16             StreamBufferPaddrH;
} MP2D_RUN_PARAM;


typedef struct _MP4E_RUN_PARAM_
{
    U16             Command;
    U16             CodecType;
    U16             StreamBufferPaddrL;
    U16             StreamBufferPaddrH;
    U32             StreamBufferSize;
} MP4E_RUN_PARAM;


typedef struct _MJPD_RUN_PARAM_
{
    U16             Command;
    U16             CodecType;
} MJPD_RUN_PARAM;






typedef struct _MJPD_OFFSET_PARAM_
{
    U16             Command;
    U16             CodecType;
    U16             LU_Offset;
    U16             CB_Offset;
    U16             CR_Offset;
} MJPD_OFFSET_PARAM;


typedef struct _MP2D_REL_PARAM_
{
    U16             Command;
    U16             CodecType;
} MP2D_REL_PARAM;


typedef struct _MP4D_REL_PARAM_
{
    U16             Command;
    U16             CodecType;
} MP4D_REL_PARAM;


typedef struct _MP4E_REL_PARAM_
{
    U16             Command;
    U16             CodecType;
} MP4E_REL_PARAM;


typedef struct _MJPD_REL_PARAM_
{
    U16             Command;
    U16             CodecType;
} MJPD_REL_PARAM;

typedef struct _MP4D_INIT_PARAM_
{
    U16             Command;    
    U16             CodecType;  
    U16             Width;      
    U16             Height;     
    U16             CodecVersion;   
    U16             RL_L;       
    U16             RL_H;       
    U16             RL_Length;  
    U16             mbp_a_addr_offset;  
    U16             mbp_b_addr_offset;  
    U16             qmat_addr_h;    
    U16             qmat_addr_l;    
    U16             pred_q_lb_offset;   
    U16             arm940_offset;  
#    if VIDEO_FW_041126         
    U16             non_bframe_max;
    U16             b_frame_max;
#    endif

} MP4D_INIT_PARAM;





#    if 0                       
typedef struct _MP4D_INIT_BUF_PARAM_
{

    U16             Command;    
    U16             buffer_type;    
    U16             buffer_idx; 
    U16             luma_offset_addr;   
    U16             cb_offset_addr; 
    U16             cr_offset_addr; 

} MP4D_INIT_BUF_PARAM;
#    else
typedef struct _MP4D_INIT_BUF_PARAM_
{
    unsigned short  Command;    
    unsigned short  BufferType; 
    unsigned short  BufferIdx;  
    unsigned short  Y_Offset;   
    unsigned short  CB_Offset;  
    unsigned short  CR_Offset;  
} MP4D_INIT_BUF_PARAM;
#    endif
typedef struct _MP4D_RUN_PARAM_
{

    U16             Command;    
    U16             CodecType;  
    U16             frames;     
    U16             stream_length;  
    U16             stream_last_marker_h;   
    U16             stream_last_marker_l;   
    U16             StreamBufferPaddrL; 
    U16             StreamBufferPaddrH; 

} MP4D_RUN_PARAM;
#    if 0                       
typedef struct _MP4D_DISPLAY_PARAM_
{
    unsigned short  Command;    
    unsigned short  BFrame;     
    unsigned short  DisplayFrames;  

    unsigned short  Number1;    
    unsigned short  Time1;      
    unsigned short  TimeWidth1; 
    unsigned short  Y_Offset1;  
    unsigned short  CB_Offset1; 
    unsigned short  CR_Offset1; 

    unsigned short  Number2;    
    unsigned short  Time2;      
    unsigned short  TimeWidth2; 
    unsigned short  Y_Offset2;  
    unsigned short  CB_Offset2; 
    unsigned short  CR_Offset2; 
} MP4D_DISPLAY_PARAM;
#    else
typedef struct _MP4D_DISPLAY_PARAM_
{
    U16             Command;    
    U16             has_bframe; 
    U16             display_frames; 

    U16             time;       
    U16             luma_offset_addr;   
    U16             cb_offset_addr; 
    U16             cr_offset_addr; 

    U16             remain_frame_time;  
    U16             remain_frame_luma_offset_addr;  
    U16             remain_frame_cb_offset_addr;    
    U16             remain_frame_cr_offset_addr;    

    U16             remain_frame_time1; 
    U16             remain_frame_luma_offset_addr1; 
    U16             remain_frame_cb_offset_addr1;   
    U16             remain_frame_cr_offset_addr1;   
    U16             dummy;

} MP4D_DISPLAY_PARAM;
#    endif

#    define IOCTL_DUALCPU_MP4D_INIT     _IOW('d', 0x01, MP4D_INIT_PARAM)
#    define IOCTL_DUALCPU_MP4D_INIT_BUF _IOW('d', 0x02, MP4D_INIT_BUF_PARAM)
#    define IOCTL_DUALCPU_MP4D_RUN      _IOW('d', 0x03, MP4D_RUN_PARAM)
#    define IOCTL_DUALCPU_MP4D_RELEASE  _IOW('d', 0x04, MP4D_REL_PARAM)
#    define IOCTL_DUALCPU_MP4D_STATUS   _IOR('d', 0x05, MP4D_DISPLAY_PARAM)

#endif                          
