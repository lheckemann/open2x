#ifndef __M4ENC_ENCORE_H__
#define __M4ENC_ENCORE_H__

typedef struct tag_CMDPARAMS
{
	unsigned short Command;
	unsigned short CodecType;
	
	unsigned short Handle;
	unsigned short cmd;
	
	unsigned short _dumy[12];
} CMDPARAMS;

typedef struct tag_OPENHANDLEPARAMS
{
	unsigned short Command;
	unsigned short CodecType;
	
	unsigned short Handle;
	unsigned short cmd;
	
	unsigned short mode;
	unsigned short format;
	
	unsigned short do_ratectrl;
	unsigned short sp_dma_channel;
	unsigned short vlc_dma_channel;
	
	unsigned short x_dim; // the x dimension of the frames to be encoded
	unsigned short y_dim; // the y dimension of the frames to be encoded
	
	unsigned short _dumy[5];
} OPENHANDLEPARAMS;

typedef struct tag_REFBUFPARAMS
{
	unsigned short Command;
	unsigned short CodecType;
	
	unsigned short Handle;
	unsigned short cmd;
	
	unsigned short RefBufCount;
	unsigned short REF0LuOffset;
	unsigned short REF0CbOffset;
	unsigned short REF0CrOffset;
	unsigned short REF1LuOffset;
	unsigned short REF1CbOffset;
	unsigned short REF1CrOffset;
	unsigned short REF2LuOffset;
	unsigned short REF2CbOffset;
	unsigned short REF2CrOffset;
	
	unsigned short _dumy[2];
} REFBUFPARAMS;

typedef struct tag_ETCBUFPARAMS
{
	unsigned short Command;
	unsigned short CodecType;
	
	unsigned short Handle;
	unsigned short cmd;
	
	unsigned long  BufQLBAddr;
	unsigned long  BufQWMAddr;

	unsigned long  BufRL0Addr;
	unsigned long  BufRL1Addr;

	unsigned short BufMEOffset;
	unsigned short BufMB0Offset;
	unsigned short BufMB1Offset;
	
	unsigned short _dumy;
} ETCBUFPARAMS;

typedef struct tag_RCPARAMS
{
	unsigned short Command;
	unsigned short CodecType;
	
	unsigned short Handle;
	unsigned short cmd;
	
	long		   bitrate;				// the birtate of the target encoded stream
	
	unsigned short framerate;			// the frame rate of the sequence to be encoded

	unsigned short rc_period;			// the intended rate control averaging period
	unsigned short rc_reaction_period;	// the reation period for rate control
	unsigned short rc_reaction_ratio;	// the ratio for down/up rate control

	unsigned short max_key_interval;	// the maximum interval between key frame

	unsigned short QScale;				
	unsigned short max_quantizer;		// the upper limit of the quantizer
	unsigned short min_quantizer;		// the lower limit of the quantizer
	unsigned short search_range;		// the forward search range for motion estimation

	unsigned short _dumy;
} RCPARAMS;

typedef struct _ENC_FRAME_
{
	unsigned short Command;
	unsigned short CodecType;
	
	unsigned short Handle;
	unsigned short cmd;
	
	unsigned short force_iframe;

	unsigned short ImageLuOffset;
	unsigned short ImageCbOffset;
	unsigned short ImageCrOffset;
	
	unsigned long  StreamOutAddr; // the buffer for encoded bitstream
	
	unsigned short _dumy[6];
} ENC_FRAME;

typedef struct _ENC_RESULT_
{
	unsigned short Command;
	unsigned short CodecType;
	
	unsigned short Handle;
	unsigned short cmd;
	
	unsigned short _dumy0;
	unsigned short IsKeyFrame; // the current frame is encoded as a key frame
	
	unsigned long  FrameCounter; 
	unsigned long  StreamLength; 
	unsigned long  TotalStreamLength; 
	
	int			   errorcode; 
	
	unsigned short _dumy1[2];
} ENC_RESULT;

#define ENCORE_MAINCMD_INIT			0x0100
#define ENCORE_MAINCMD_RUN			0x0200
#define ENCORE_MAINCMD_RELEASE		0x0400

#define ENCORE_CMD_OPENHANDLE		(ENCORE_MAINCMD_INIT | 1)
#define ENCORE_CMD_REFBUF			(ENCORE_MAINCMD_INIT | 2)
#define ENCORE_CMD_ETCBUF			(ENCORE_MAINCMD_INIT | 3)
#define ENCORE_CMD_RCPARM			(ENCORE_MAINCMD_INIT | 4)
#define ENCORE_CMD_INITDEVICE		(ENCORE_MAINCMD_INIT | 5)

#define MP4E_MAX_INSTANCE			16

#define MP4E_ENCODE_MODE_MPEG4		0
#define MP4E_ENCODE_MODE_H263		1

// encore options (the enc_opt parameter of encore())
#define ENC_OPT_WRITE				1024	// write the reconstruct image to files (for debuging)
#define ENC_OPT_INIT				32768	// initialize the encoder for an handle
#define ENC_OPT_RELEASE				65536	// release all the resource associated with the handle

// return code of encore()
#define ENC_OK						0
#define ENC_FAIL					0x80000000
#define MP4E_ERRORCODE_NOERROR		0x00000000
#define MP4E_ERRORCODE_FAIL			0x80000000
#define MP4E_ERRORCODE_EXCEEDOPENHANDLE	0x80000001
#define MP4E_ERRORCODE_OUTOFMEMORY	0x80000003
#define MP4E_ERRORCODE_INVALIDHANDLE	0x80000003
#define MP4E_ERRORCODE_INVALIDPARAMETER	0x80000004

#define ENC_VERSION					1

#endif // __M4ENC_ENCORE_H__
