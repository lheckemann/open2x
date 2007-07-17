//------------------------------------------------------------------------------
//
// MMSP2 DMA prototype Header file
// Copyright (C) MagicEyes Digital Co. 2003
//
// Module     : MMSP2 DMA Controller interface
// File       : proto_dma.h
// Description: low level interface of MMSP2 DMA Controller
// Author     : Goofy
// Export     :
// History    :
//		2003/11/17 Brian	DMA_PERIPHERAL_INDEX_EXTxx Ãß°¡
//		2003/10/27 Goofy	Redefine types
//		2003/10/20 Brian	Insert "#define DMA_CHANNEL_COUNT	16" for convenience
// 		2003/10/08 Goofy	Modify TDMAC_RequestSource type
//							Remove DMA_ClrChannelIOIndex function
//    	2003/10/02 Goofy	First Release
//------------------------------------------------------------------------------
#ifndef __PROTO_DMA_H__
#define __PROTO_DMA_H__


#include "../sound/typedef.h"
#include "../sound/mes_macro.h"


#define DMA_MEMTYPE_IOMASK		(0x0000FFFF)
#define DMA_MEMTYPE_MEMMASK 	(0xFFFF0000)

#define DMA_MEMTYPE_UNKNOWN		0
#define DMA_MEMTYPE_IO_BYTE		_BIT( 0)
#define DMA_MEMTYPE_IO_HWORD	_BIT( 1)
#define DMA_MEMTYPE_MEM_LSHWORD	_BIT(16)
#define DMA_MEMTYPE_MEM_MSHWORD _BIT(17)
#define DMA_MEMTYPE_MEM_WORD	_BIT(18)

#define DMA_BURSTTYPE_NOBURST	0
#define DMA_BURSTTYPE_4WORD		_BIT( 0)
#define DMA_BURSTTYPE_8WORD		_BIT( 1)
#define DMA_BURSTTYPE_DUAL4WORD _BIT( 2)

#define DMA_INTTYPE_NONE		0
#define DMA_INTTYPE_END			_BIT( 0)
#define DMA_INTTYPE_STOP		_BIT( 1)

#define DMA_STATUSFLAG_NONE		0
#define DMA_STATUSFLAG_STOPINT	_BIT( 0)
#define DMA_STATUSFLAG_ENDINT	_BIT( 1)
#define DMA_STATUSFLAG_REQUEST	_BIT( 2)
#define DMA_STATUSFLAG_RUN		_BIT(10)

#define DMA_CHANNEL_COUNT		16

typedef enum
{
	DMA_PERIPHERAL_INDEX_UART0_RX	= 0,
	DMA_PERIPHERAL_INDEX_UART0_TX,
	DMA_PERIPHERAL_INDEX_UART1_RX,
	DMA_PERIPHERAL_INDEX_UART1_TX,
	DMA_PERIPHERAL_INDEX_UART2_RX,
	DMA_PERIPHERAL_INDEX_UART2_TX,
	DMA_PERIPHERAL_INDEX_UART3_RX,
	DMA_PERIPHERAL_INDEX_UART3_TX,
	DMA_PERIPHERAL_INDEX_IRDA_RX	= 10,
	DMA_PERIPHERAL_INDEX_IRDA_TX,
	DMA_PERIPHERAL_INDEX_USBD_EP1,
	DMA_PERIPHERAL_INDEX_USBD_EP2,
	DMA_PERIPHERAL_INDEX_USBD_EP3,
	DMA_PERIPHERAL_INDEX_USBD_EP4,
	DMA_PERIPHERAL_INDEX_DECSS_RD	= 18,
	DMA_PERIPHERAL_INDEX_DECSS_WR,
	DMA_PERIPHERAL_INDEX_MMCSD_RX,
	DMA_PERIPHERAL_INDEX_MMCSD_TX,
	DMA_PERIPHERAL_INDEX_MSTICK,
	DMA_PERIPHERAL_INDEX_AC97_PCMOUT = 24,
	DMA_PERIPHERAL_INDEX_AC97_SOUT,
	DMA_PERIPHERAL_INDEX_AC97_CWOUT,
	DMA_PERIPHERAL_INDEX_AC97_MICIN,
	DMA_PERIPHERAL_INDEX_AC97_PCMIN,
	DMA_PERIPHERAL_INDEX_TOUCH,
	DMA_PERIPHERAL_INDEX_I2S_RX,
	DMA_PERIPHERAL_INDEX_I2S_TX,
	DMA_PERIPHERAL_INDEX_SPDIF_IN	= 36,
	DMA_PERIPHERAL_INDEX_SPDIF_OUT,
	DMA_PERIPHERAL_INDEX_SSP_RD,
	DMA_PERIPHERAL_INDEX_SSP_WR,
	DMA_PERIPHERAL_INDEX_EXT0		= 50,
	DMA_PERIPHERAL_INDEX_EXT1,
	DMA_PERIPHERAL_INDEX_EXT2,
	DMA_PERIPHERAL_INDEX_EXT3,
	DMA_PERIPHERAL_INDEX_MPEGIF		= 54,
	DMA_PERIPHERAL_INDEX_SP,
	DMA_PERIPHERAL_INDEX_HUFFMAN	= 60,
	DMA_PERIPHERAL_INDEX_RVLD
} TDMAC_RequestSource;

typedef struct
{
	U32 				dwType;
	S32 				iStep;
	U32					dwAddr;
	CBOOL				bFlyBy;
	TDMAC_RequestSource	IOIndex;
} TDMAC_Format;

typedef struct
{
	S32				iChannel;
	TDMAC_Format 	Source;
	TDMAC_Format 	Target;
	U32				dwDataSize;
	U32				dwBurstType;
	U32				dwIntType;
} TDMAC_ChannelData;

typedef void (DMAIRQSubHandlerFn)(void);

CBOOL	DMA_Initialize( U32 VirAddr );
CBOOL	DMA_Close(void);
CBOOL	DMA_InitHardware( void );
CBOOL	DMA_CloseHardware( void );
void	DMA_GetDeviceInfo( MES_DEVINFO *pDevInfo );

void	DMA_RegisterIRQHandler( U32 iChannel, DMAIRQSubHandlerFn* pHandler );

void	DMA_SetChannel( TDMAC_ChannelData* pDMAC_ChannelData );
void	DMA_RunChannel( U32 iChannel );
void	DMA_StopChannel( U32 iChannel );
CBOOL	DMA_IsRunChannel( U32 iChannel );

U32		DMA_GetChannelStatus( U32 iChannel );
void	DMA_SetSrcAddr( U32 iChannel, U32 dwAddr );
U32 	DMA_GetSrcAddr( U32 iChannel );
void	DMA_SetTrgAddr( U32 iChannel, U32 dwAddr );
U32 	DMA_GetTrgAddr( U32 iChannel );
void	DMA_SetDataSize( U32 iChannel, U32 dwSize );
U32 	DMA_GetDataSize( U32 iChannel );
void	DMA_SetChannelIOIndex( U32 iChannel, CBOOL IsSource, TDMAC_RequestSource Index );

void	DMA_SetInterruptEnb( U32 iChannel, U32 dwType );
CBOOL	DMA_GetInterruptPend( U32 iChannel );
void	DMA_ClrInterruptPend( U32 iChannel );
U16		DMA_GetInterruptPendAll( void );

#endif // __PROTO_DMA_H__
