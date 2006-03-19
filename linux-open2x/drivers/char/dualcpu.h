//------------------------------------------------------------------------------
//
// MPEG4 Decoder Filter
// Copyright (C) MagicEyes Digital Co. 2003
// Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
// Kane Ahn < hbahn@dignsys.com >
// hhsong < hhsong@dignsys.com >
//
//------------------------------------------------------------------------------

#ifndef _MMSP2_DUALCPU_H_
#define	_MMSP2_DUALCPU_H_

//------------------------------------------------------------------------------
//	CODEC COMMAND
//------------------------------------------------------------------------------
#define	MP_CMD_NULL			0

//------------------------------------------------------------------------------
//	CODEC TYPE
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
//	CODEC COMMAND
//------------------------------------------------------------------------------

/* ARM940 command */
#define	MP_CMD_INIT					1
#define MP_CMD_INIT_BUF     		16
#define	MP_CMD_RUN					2
#define	MP_CMD_RELEASE				4
#define	MP_CMD_GETINFO				6
#define	MP_CMD_940IDLE				5
// init / relase for seek
#define	MP_CMD_INIT_FOR_SEEK		7
#define	MP_CMD_RELEASE_FOR_SEEK		8

/* ARM920 command */
#define MP_CMD_DISPLAY      		17

/* general */
#define	MP_CMD_ACK					5
#define	MP_CMD_FAIL					15


//------------------------------------------------------------------------------
//	CODEC VERSION
//  Length : 16bits
//------------------------------------------------------------------------------

#define	CODEC_VER_DIVX_311	311
#define	CODEC_VER_DIVX_500	500
#define	CODEC_VER_XVID		900


//------------------------------------------------------------------------------
//	CODEC INIT. PARAMETER
//------------------------------------------------------------------------------

// SYSTEM 
typedef struct _SYSTEM_INIT_PARAM_ {
	U16	Arm940BaseOffset;
} SYSTEM_INIT_PARAM;

// MPEG4 DECODER
typedef struct _MP4D_INIT_PARAM_ {

    U16 Command;            // commands ARM940T Firmware. (codec.h)
	U16 CodecType;          // defines what CODEC do you want to use. (codec.h)
	U16 Width;              // Video image width
	U16 Height;             // Video image height
	U16 CodecVersion;       // defined in codec.h
	U16 RL_L;               // Low 16-bit physical address of RL buffer
	U16 RL_H;               // High 16-bit physical address of RL buffer
	U16 RL_Length;          // 512 (Unit is Kbytes, The Size of RL buffer should be 512KB)
	U16 mbp_a_addr_offset;  // high 16bits addresss for macroblock param  A
	U16 mbp_b_addr_offset;  // high 16bits addresss for macroblock param  B
	U16 qmat_addr_h; 		// high 16bits address for quantizer matrix data save
	U16 qmat_addr_l; 		// low 16bits address for quantizer matrix data save
	U16 pred_q_lb_offset; 	// 16bit offset address for mpeg4 h/w block's line buffer 
	U16 arm940_offset;      // arm940t's base address
    U16	non_bframe_max;
    U16 b_frame_max;    

} MP4D_INIT_PARAM;

// MPEG4 ENCODER
typedef struct _MP4E_INIT_PARAM_ {
	U16 Command;
	U16 CodecType;
	U16 Width;
	U16 Height;
	U16	CodecVersion;
	U16 Framerate;
	U16 RcPeriod;
	U16 RcReactionPeriod;
	U16 RcReactionRation;
	U16 MaxKeyInterval;
	U16 MaxQuantizer;
	U16 MinQuantizer;
	U16 SearchRrange;
} MP4E_INIT_PARAM;

// MJPEG DECODER
typedef struct _MJPD_INIT_PARAM_ {
	U16 Command;
	U16 CodecType;
	U16 mbp_a_addr_offset;  // high 16bits addresss for macroblock param  A
	U16 mbp_b_addr_offset;  // high 16bits addresss for macroblock param  B
	U16 qmat_addr_h;        // high 16bits address for quantizer matrix data save
	U16 qmat_addr_l;        // low 16bits address for quantizer matrix data save
	U16 pred_q_lb_offset;   // 16bit offset address for mpeg4 h/w block's line buffer 

} MJPD_INIT_PARAM;

//------------------------------------------------------------------------------
//	CODEC INIT BUF
//------------------------------------------------------------------------------
// MPEG4 DECODER
typedef struct _MP4D_INIT_BUF_PARAM_ {

	U16 Command;            // commands ARM940T Firmware. (codec.h)
    U16 buffer_type;        // non bframe buffer of bframe buffer
    U16 buffer_idx;         // buffer idx
    U16 luma_offset_addr;   // luma 16bits offset address
    U16 cb_offset_addr;     // cb 16bits offset address
    U16 cr_offset_addr;     // cr 16bits offset address
        
} MP4D_INIT_BUF_PARAM;

//------------------------------------------------------------------------------
//	CODEC RUN PARAMETER
//------------------------------------------------------------------------------

// MPEG4 ENCODER
typedef struct _MP4E_RUN_PARAM_ {
	U16 Command;
	U16 CodecType;
	U16 StreamBufferPaddrL;
	U16 StreamBufferPaddrH;
	U32 StreamBufferSize;
} MP4E_RUN_PARAM;

// MPEG4 DECODER
typedef struct _MP4D_RUN_PARAM_ {

    U16 Command;                // commands ARM940T Firmware. (codec.h)
    U16 CodecType;              // defines what CODEC do you want to use. (codec.h)
    U16 frames;                 // frame number in current received stream  /* not implemented */
    U16 stream_length;          // stream length                            /* not implemented */
    U16 stream_last_marker_h;   // high 16bits data of stream last marker   /* not implemented */
    U16 stream_last_marker_l;   // low 16bits data of stream last marker    /* not implemented */
    U16 StreamBufferPaddrL;     // 16bits low address of stream address 
	U16 StreamBufferPaddrH;     // 16bits high address of stream address

} MP4D_RUN_PARAM;


// MJPEG DECODER
typedef struct _MJPD_RUN_PARAM_ {
	U16 Command;
	U16 CodecType;
	U16 ImageLuOffset;
	U16 ImageCbOffset;
	U16 ImageCrOffset;
	U16 dump;
	U32 RL_Paddr;
	U32 StreamBufferSize;
	U32 StreamBufferPaddr;	
	U16 Reset_Offset;			//		Register Reset Offset Address
	U16 Dec_OK;

} MJPD_RUN_PARAM;

// MJPEG ENCODER
typedef struct _MJPE_RUN_PARAM_ {
	U16 Command;
	U16 CodecType;

	U16 REC_Offset[3];     	//		MPEG_CUR_OFFSET_LU , CB , CR	-> 

	U16 MB_Offset;       	//		MPEG_MBP_ADDR		->         
	U32 QWM_Addr;        	//		MPEG_QWMAT_ADDR      	->         
	U32 RL_Addr;         	//		MPEG_RL_ADDR		->         
	U32 QLB_Addr;       	//		MPEG_QLB_ADDR		->         
	U16 Reset_Offset;			//		Register Reset Offset Address

} MJPE_RUN_PARAM;

//------------------------------------------------------------------------------
//	CODEC SET OFFSET ADDR PARAMETER
//------------------------------------------------------------------------------
// MJPEG DECODER
typedef struct _MJPD_OFFSET_PARAM_ {
	U16 Command;
	U16 CodecType;
	U16 LU_Offset;
	U16 CB_Offset;
	U16 CR_Offset;
} MJPD_OFFSET_PARAM;

//------------------------------------------------------------------------------
//	CODEC GETINFO PARAMETER
//------------------------------------------------------------------------------
// MJPEG DECODER
typedef struct _MJPD_GETINFO_PARAM_ {
	U16 Command;
	U16 CodecType;

	U32 SourcePAddr;
	U32 TargetPAddr;

	U16 x_dim;
	U16 y_dim;
} MJPD_GETINFO_PARAM;

// MJPEG ENCODER
typedef struct _MJPE_GETINFO_PARAM_ {
	U16 Command;
	U16 CodecType;

	U16 YUV_Offset[3];     	//		MPEG_CUR_OFFSET_LU , CB , CR	-> 

	U32 OutStream_Addr;  	//		ENC_STREAM_ADDR		->         
	U16 x_dim;
	U16 y_dim;
} MJPE_GETINFO_PARAM;

typedef struct _MJPE_ENC_RESULT_ 
{
	U16 Command;
	U16 CodecType;
	
	U16 REC_Offset[3];
	U16 dump;
	U32	StreamLength;
	U32	TotalStreamLength;
	U32 error_check;

} MJPE_ENC_RESULT;

//------------------------------------------------------------------------------
//	CODEC RELEASE  PARAMETER
//------------------------------------------------------------------------------

// MPEG4 ENCODER
typedef struct _MP4E_REL_PARAM_ {
	U16 Command;
	U16 CodecType;
} MP4E_REL_PARAM;

// MPEG4 DECODER
typedef struct _MP4D_REL_PARAM_ {
	U16 Command;
	U16 CodecType;
} MP4D_REL_PARAM;

// MJPEG DECODER
typedef struct _MJPD_REL_PARAM_ {
	U16 Command;
	U16 CodecType;
} MJPD_REL_PARAM;

//------------------------------------------------------------------------------
//	CODEC DISPLAY PARAMETER
//------------------------------------------------------------------------------
// MPEG4 DECODER  or comman codec 
typedef struct _MP4D_DISPLAY_PARAM_ {

    U16 Command;                        // commands ARM940T Firmware. (codec.h)  0
    U16 has_bframe;                     // b frame flag in p frame               1
    U16 display_frames;                 // number of frame to display            2
    
    U16 time;                           // #0 frame's time                 3 /* not implemented */
    U16 luma_offset_addr;               // #0 16bits luma offset address 4
    U16 cb_offset_addr;                 // #0 16bits cb offset address 5
    U16 cr_offset_addr;                 // #0 16bits cr offset address 6
    
    U16 remain_frame_time;              // #1 frame's time              7    /* not implemented */
    U16 remain_frame_luma_offset_addr;  // #1 16bits luma offset address 8
    U16 remain_frame_cb_offset_addr;    // #1 16bits cb offset address 9
    U16 remain_frame_cr_offset_addr;    // #1 16bits cr offset address 10
    
    
    U16 remain_frame_time1;              // #1 frame's time             11     /* not implemented */
    U16 remain_frame_luma_offset_addr1;  // #1 16bits luma offset address 12
    U16 remain_frame_cb_offset_addr1;    // #1 16bits cb offset address 13
    U16 remain_frame_cr_offset_addr1;    // #1 16bits cr offset address 14
    U16 read_length_344;					// 15

} MP4D_DISPLAY_PARAM;

/* MPEG 4 Decode IOCTL Commands */
#define IOCTL_DUALCPU_MP4D_INIT     		_IOW('d', 0x01, MP4D_INIT_PARAM)
#define IOCTL_DUALCPU_MP4D_INIT_BUF 		_IOW('d', 0x02, MP4D_INIT_BUF_PARAM)
#define IOCTL_DUALCPU_MP4D_RUN      		_IOW('d', 0x03, MP4D_RUN_PARAM)
#define IOCTL_DUALCPU_MP4D_RELEASE  		_IOW('d', 0x04, MP4D_REL_PARAM)
#define IOCTL_DUALCPU_MP4D_STATUS   		_IOR('d', 0x05, MP4D_DISPLAY_PARAM)

/* MPEG 4 Encode IOCTL Commands */
#	define IOCTL_DUALCPU_MP4E_OPENHANDLE	_IOW('d', 0x11, OPENHANDLEPARAMS)
#	define IOCTL_DUALCPU_MP4E_REFBUF		_IOW('d', 0x12, REFBUFPARAMS)
#	define IOCTL_DUALCPU_MP4E_ETCBUF		_IOW('d', 0x13, ETCBUFPARAMS)
#	define IOCTL_DUALCPU_MP4E_RCPARM		_IOW('d', 0x14, RCPARAMS)
#	define IOCTL_DUALCPU_MP4E_INITDEVICE	_IOW('d', 0x15, CMDPARAMS)
#	define IOCTL_DUALCPU_MP4E_RUN			_IOW('d', 0x16, ENC_FRAME)
#	define IOCTL_DUALCPU_MP4E_RELEASE		_IOW('d', 0x17, CMDPARAMS)
#	define IOCTL_DUALCPU_MP4E_STATUS		_IOR('d', 0x18, ENC_RESULT)

/* JPEG Decode IOCTL Commands */
#	define IOCTL_DUALCPU_MJPD_INIT   		_IOW('d', 0x21, MJPD_INIT_PARAM)
#	define IOCTL_DUALCPU_MJPD_RUN      		_IOW('d', 0x22, MJPD_RUN_PARAM)
#	define IOCTL_DUALCPU_MJPD_GETINFO  		_IOW('d', 0x23, MJPD_GETINFO_PARAM)
#	define IOCTL_DUALCPU_MJPD_RELEASE  		_IOW('d', 0x24, MJPD_REL_PARAM)

/* JPEG Encode IOCTL Commands */
#	define IOCTL_DUALCPU_MJPE_RUN			_IOW('d', 0x32, MJPE_RUN_PARAM)
#	define IOCTL_DUALCPU_MJPE_GETINFO		_IOW('d', 0x33, MJPE_GETINFO_PARAM)
#	define IOCTL_DUALCPU_MJPE_STATUS		_IOW('d', 0x34, MJPE_ENC_RESULT)

#endif	// _MMSP2_DUALCPU_H_
