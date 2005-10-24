/*
 * linux/kernel/char/dualcpu.h
 *
 * MMSP2 DUAL CPU Interface include file
 *
 */

#ifndef _MMSP2_DUALCPU_H_
#define _MMSP2_DUALCPU_H_

//------------------------------------------------------------------------------
//	CODEC COMMAND
//------------------------------------------------------------------------------
//#define	MP_CMD_NULL			0
#define	MP_CMD_INIT			1
#define	MP_CMD_RUN			2
//#define	MP_CMD_SETREFOFFSET	3
#define	MP_CMD_RELEASE		4
#define	MP_CMD_ACK			5
//#define	MP_CMD_GETINFO		6
#define	MP_CMD_FAIL			15
#define MP_CMD_INIT_BUF     16
#define MP_CMD_DISPLAY      17

//------------------------------------------------------------------------------
//	CODEC TYPE
//------------------------------------------------------------------------------
#define	MP_CODEC_NULL		-1
#define	MP_CODEC_MP4D		0
#define MP_CODEC_MP4D_DIVX3 1
//#define	MP_CODEC_MP2D		1
//#define	MP_CODEC_MJPD		2
//#define	MP_CODEC_MP4E		3
//#define MP_CODEC_MP3D       4


//------------------------------------------------------------------------------
//	CODEC VERSION
//  Length : 16bits
//------------------------------------------------------------------------------
// MPEG2 DECODER

// MPEG4 DECODER
#define	CODEC_VER_DIVX_311	311
#define	CODEC_VER_DIVX_500	500
#define	CODEC_VER_XVID		900

// MPEG4 ENCODER


//------------------------------------------------------------------------------
//	CODEC INIT. PARAMETER
//------------------------------------------------------------------------------
// MPEG2 DECODER
typedef struct _MP2D_INIT_PARAM_ {
	unsigned short Command;
	unsigned short CodecType;
	unsigned short Width;
	unsigned short Height;
	unsigned short CodecVersion;
	unsigned short RL_L;
	unsigned short RL_H;
	unsigned short RL_Length;	// in kbytes
	unsigned short StreamBufferPaddrL;
	unsigned short StreamBufferPaddrH;
} MP2D_INIT_PARAM;

// MPEG4 DECODER
typedef struct _MP4D_INIT_PARAM_ {
	unsigned short Command; 						
	unsigned short CodecType;					// command type of param , i.e. INIT, RUN, RELEASE
	unsigned short Width;						// Video image width
	unsigned short Height; 						// Video image height
	unsigned short CodecVersion;				// build number
	unsigned short RL_L;						// Low 16-bit physical address of RL buffer
	unsigned short RL_H;						// High 16-bit physical address of RL buffer
	unsigned short RL_Length;					// 512 (Unit is Kbytes, The Size of RL buffer should be 512KB)
	unsigned short mbp_a_addr_offset;			// mbp a offset address
	unsigned short mbp_b_addr_offset;			// mbp b offset address
	unsigned short qmat_addr_h;					// qmat address high 16 bits
	unsigned short qmat_addr_l;					// qmat address low 16 bits
	unsigned short pred_q_lb_offset;			// pred/q h/w's line buffer offset
	unsigned short arm940_offset;				// arm940 base address high 16 bits
	unsigned short non_bframe_max;				// arm940 internal non bframe buffer max index
    unsigned short b_frame_max;					// arm940 internal bframe buffer max index

} MP4D_INIT_PARAM;

// MPEG4 ENCODER
typedef struct _MP4E_INIT_PARAM_ {
	unsigned short Command;
	unsigned short CodecType;
	unsigned short Width;
	unsigned short Height;
	unsigned short CodecVersion;
	unsigned short Framerate;
	unsigned short RcPeriod;
	unsigned short RcReactionPeriod;
	unsigned short RcReactionRation;
	unsigned short MaxKeyInterval;
	unsigned short MaxQuantizer;
	unsigned short MinQuantizer;
	unsigned short SearchRrange;
} MP4E_INIT_PARAM;

// MJPEG DECODER
typedef struct _MJPD_INIT_PARAM_ {
	unsigned short Command;
	unsigned short CodecType;
} MJPD_INIT_PARAM;


//------------------------------------------------------------------------------
//	CODEC INIT BUF
//------------------------------------------------------------------------------
// MPEG4 DECODER
typedef struct _MP4D_INIT_BUF_PARAM_ {
	unsigned short Command;           // commands ARM940T Firmware. (codec.h)
    unsigned short BufferType;        // non bframe buffer of bframe buffer
    unsigned short BufferIdx;         // buffer idx
    unsigned short Y_Offset;          // luma 16bits offset address
    unsigned short CB_Offset;         // cb 16bits offset address
    unsigned short CR_Offset;         // cr 16bits offset address
} MP4D_INIT_BUF_PARAM;


//------------------------------------------------------------------------------
//	CODEC RUN PARAMETER
//------------------------------------------------------------------------------
// MPEG2 DECODER
typedef struct _MP2D_RUN_PARAM_ {
	unsigned short Command;
	unsigned short CodecType;
	unsigned short StreamBufferPaddrL;
	unsigned short StreamBufferPaddrH;
} MP2D_RUN_PARAM;

// MPEG4 DECODER
typedef struct _MP4D_RUN_PARM_ {
    unsigned short Command;                // commands ARM940T Firmware. (codec.h)
    unsigned short CodecType;
    unsigned short Frames;                 // frame number in current received stream  /* not implemented */
    unsigned short StreamLength;           // stream length                            /* not implemented */
    unsigned short StreamLastMarkerL;      // high 16bits data of stream last marker   /* not implemented */
    unsigned short StreamLastMarkerH;      // low 16bits data of stream last marker    /* not implemented */
    unsigned short StreamBufferPaddrL;     // 16bits low address of stream address 
	unsigned short StreamBufferPaddrH;     // 16bits high address of stream address
} MP4D_RUN_PARAM;

// MPEG4 DECODER
typedef struct _MP4E_RUN_PARAM_ {
	unsigned short Command;
	unsigned short CodecType;
	unsigned short StreamBufferPaddrL;
	unsigned short StreamBufferPaddrH;
	unsigned long StreamBufferSize;
} MP4E_RUN_PARAM;

// MJPEG DECODER
typedef struct _MJPD_RUN_PARAM_ {
	unsigned short Command;
	unsigned short CodecType;
} MJPD_RUN_PARAM;


//------------------------------------------------------------------------------
//	CODEC SET OFFSET ADDR PARAMETER
//------------------------------------------------------------------------------
// MJPEG DECODER
typedef struct _MJPD_OFFSET_PARAM_ {
	unsigned short Command;
	unsigned short CodecType;
	unsigned short LU_Offset;
	unsigned short CB_Offset;
	unsigned short CR_Offset;
} MJPD_OFFSET_PARAM;


//------------------------------------------------------------------------------
//	CODEC RELEASE PARAMETER
//------------------------------------------------------------------------------
// MPEG2 DECODER
typedef struct _MP2D_REL_PARAM_ {
	unsigned short Command;
	unsigned short CodecType;
} MP2D_REL_PARAM;

// MPEG4 DECODER
typedef struct _MP4D_REL_PARAM_ {
	unsigned short Command;
	unsigned short CodecType;
} MP4D_REL_PARAM;

// MPEG4 ENCODER
typedef struct _MP4E_REL_PARAM_ {
	unsigned short Command;
	unsigned short CodecType;
} MP4E_REL_PARAM;

// MJPEG DECODER
typedef struct _MJPD_REL_PARAM_ {
	unsigned short Command;
	unsigned short CodecType;
} MJPD_REL_PARAM;


/* Command from ARM940 */

typedef struct _MP4D_DISPLAY_PARAM_ {
    unsigned short Command;                        // commands ARM940T Firmware. (codec.h)
    unsigned short BFrame;                         // b frame flag in p frame
    unsigned short DisplayFrames;                  // number of frame to display

//    unsigned short Number1;                        // #0 frame's number                /* not implemented */
    unsigned short Time1;                          // #0 frame's time                  /* not implemented */
//    unsigned short TimeWidth1;                     // #0 frame's time width            /* not implemented */
    unsigned short Y_Offset1;                      // #0 16bits luma offset address
    unsigned short CB_Offset1;                     // #0 16bits cb offset address
    unsigned short CR_Offset1;                     // #0 16bits cr offset address

//    unsigned short Number2;                        // #1 frame number                  /* not implemented */
    unsigned short Time2;                          // #1 frame's time                  /* not implemented */
//    unsigned short TimeWidth2;                     // #1 frame's time width            /* not implemented */
    unsigned short Y_Offset2;                      // #1 16bits luma offset address
    unsigned short CB_Offset2;                     // #1 16bits cb offset address
    unsigned short CR_Offset2;                     // #1 16bits cr offset address
	
//    unsigned short Number2;                        // #1 frame number                  /* not implemented */
    unsigned short Time;                          // #1 frame's time                  /* not implemented */
//    unsigned short TimeWidth2;                     // #1 frame's time width            /* not implemented */
    unsigned short Y_Offset;                      // #1 16bits luma offset address
    unsigned short CB_Offset;                     // #1 16bits cb offset address
    unsigned short CR_Offset;                     // #1 16bits cr offset address
	unsigned short ReadLength;
} MP4D_DISPLAY_PARAM;


#define IOCTL_DUALCPU_MP4D_INIT		_IOW('d', 0x01, MP4D_INIT_PARAM)
#define IOCTL_DUALCPU_MP4D_INIT_BUF	_IOW('d', 0x02, MP4D_INIT_BUF_PARAM)
#define IOCTL_DUALCPU_MP4D_RUN		_IOW('d', 0x03, MP4D_RUN_PARAM)
#define IOCTL_DUALCPU_MP4D_RELEASE	_IOW('d', 0x04, MP4D_REL_PARAM)
#define IOCTL_DUALCPU_MP4D_STATUS	_IOR('d', 0x05, MP4D_DISPLAY_PARAM)

#endif /* #ifndef _MMSP2_DUALCPU_H_ */
