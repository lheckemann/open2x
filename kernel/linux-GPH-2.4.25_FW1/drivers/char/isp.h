/*
 *  linux/kernel/drivers/char/isp.h
 *
 *  Image Signal Processor include file
 */
 
#ifndef _MMSP2_ISP_H_
#define _MMSP2_ISP_H_

//------------------------------------------------------------------------------
// ISP_Initialize¿« Parameter º≥¡§ 
//------------------------------------------------------------------------------

typedef struct _isp_init {
	unsigned char	InputDataFormat;
	unsigned char	MasterMode;
	unsigned char	SrcClock;
	unsigned char	DestClock;
	unsigned char	GenField;
	unsigned char	VerSyncStPnt;
	unsigned char	HorSyncPol;
	unsigned char	VerSyncPol;
	unsigned short	HorSyncPeriod;
	unsigned char	HorSyncPulse;
	unsigned char	HorEAVtoSyncStart;
	unsigned short	VerSyncPeriod;
	unsigned short	VerSyncPulse;
	unsigned char	VerEAVtoSyncStart;
	unsigned char	ExtBitWidth;
	unsigned char	MemBitWidth;
	unsigned char	SrcRGBBPP;
	unsigned short	SrcRGBPixWid;
	unsigned char	DestRGBBPP;
	unsigned short	DestRGBPixWid;	
	unsigned char	BayerRGBArray;
	unsigned char	rRecov;
	unsigned char	gRecov;
	unsigned char	bRecov;
	unsigned char	RGBOrder;
	unsigned char	CrCbDeciFtOnOff;
	unsigned char	ChromaDelay;
	unsigned char	CbCrSwap;
	unsigned char	VideoRange;
	unsigned char	UseGamma;
	unsigned char	vInterruptType;
	unsigned char	hInterruptType;
} ISP_INIT;

typedef struct _sync_info {
	unsigned short GenField;
	unsigned short VerSyncStPnt;
	unsigned short HorSyncPol;
	unsigned short VerSyncPol;
	unsigned short HorSyncPeriod;
	unsigned short HorSyncPulse;
	unsigned short HorEAVtoSyncStart;
	unsigned short VerSyncPeriod;
	unsigned short VerSyncPulse;
	unsigned short VerEAVtoSyncStart;
} SYNC_INFO;

typedef struct _pre_window_info {
	unsigned short RGBHorStart;
	unsigned short RGBHorWidth;
	unsigned short RGBVerStart;
	unsigned short RGBVerHeight;
} PRE_WINDOW_INFO;

typedef struct _av_window_info {
	unsigned short AVHorStart;
	unsigned short AVHorWidth;
	unsigned short AVVerStart;
	unsigned short AVVerHeight;
} AV_WINDOW_INFO;

/* ISP IOCTL Commands */
#define ISP_SET_INPUT_DATA_FORMAT	_IOW('v', 0x00, unsigned char)
#define ISP_SET_SYNC				_IOW('v', 0x01, SYNC_INFO)
#define ISP_SET_PRE_WINDOW			_IOW('v', 0x02, PRE_WINDOW_INFO)
#define ISP_SET_AV_WINDOW			_IOW('v', 0x03, AV_WINDOW_INFO)
#define ENABLE_ISP					_IOW('v', 0x04, unsigned char) // 1:Enable 0:Disable
#define START_ISP					_IO('v', 0x05) 
#define ISP_SET_OPER_MODE			_IOW('v', 0x06, unsigned char)
#define ISP_SET_BAYER_RGB_ARRAY		_IOW('v', 0x07, unsigned char)

#endif	// _MMSP2_ISP_H

