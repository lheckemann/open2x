//------------------------------------------------------------------------------
//
// MMSP2 DMA prototype C Source file
// Copyright (C) MagicEyes Digital Co. 2003
//
// Module     : MMSP2 DMA Controller interface
// File       : mmsp2dma.c
// Description: low level interface of MMSP2 DMA Controller
// Author     : Goofy
// Export     :
// History    :
//		2004/03/15 Goofy	change _WINDEF_ to UNDER_CE & DONT_USE_IRQHANDLER
//		2003/11/04 Goofy	Fix bug In DMA_SetChannel when Target is IO
//		2003/10/28 Brian
//							#define DMA_PROTO_DEBUG_MSG	MES_DEBUG 문장은
//							MES_DEBUG가 일정 상수를 갖는 Define일 때
//							RETAILMSG()를 처리할 수 있으므로 옛날로 복귀
//							(Goofy 확인요) -> 확인
//		2003/10/27 Goofy	Redefine types
//							Add InitHardware & CloseHardware functions
//							Use _WINDEF_ to remove some code for WINCE system
// 		2003/10/08 Goofy	Remove gpDMAChannelMaster variable
//							Modify DMA_IRQHandler function
//							Modify DMA_ClrInterruptPend function
//							Remove DMA_ClrChannelIOIndex function
//							Remove DMA_ClrInterruptPendAll function
//    	2003/09/17 Goofy	First Release
//------------------------------------------------------------------------------
#include <common.h>
#include "../sound/proto_dma.h"
#include "../sound/mmsp2dma.h"
#include "../sound/mes_iomap.h"

#ifdef UNDER_CE
#ifndef DONT_USE_IRQHANDLER
#define DONT_USE_IRQHANDLER
#endif
#endif

#ifndef DONT_USE_IRQHANDLER
#include "../sound/irq_manager.h"
#endif

//#define DMA_PROTO_DEBUG_MSG             1
#if defined(MES_DEBUG)
#define DMA_PROTO_DEBUG_MSG			1
#else
#define DMA_PROTO_DEBUG_MSG			0
#endif

TDMAC_Reg* gpDMAC = NULL;

/*----------------------------------------------------------------------------*/
void
DMA_GetDeviceInfo
(
	MES_DEVINFO *pDevInfo 	// [in] :
)
{
	static U8 DMADevName[] = "MMSP2 DMA Controller v1.0";

	pDevInfo->pDevName = (const U8 *)DMADevName;
	pDevInfo->IOBaseAddress = MMSP2_DMAC_PHYSICAL_BASEADDRESS;
	pDevInfo->IORegionSize = sizeof(TDMAC_Reg);
	pDevInfo->Cap = MES_DEV_INT;
}

#ifndef DONT_USE_IRQHANDLER
DMAIRQSubHandlerFn* DMA_ChannelIRQHandler[DMAC_CHANNEL_COUNT] = { NULL, };

/*----------------------------------------------------------------------------*/
void DMA_IRQHandler( void )
{
	U16 temp;
	int i;

	temp = gpDMAC->DMAINT;
	for( i=0 ; (i<DMAC_CHANNEL_COUNT) && (temp!=0) ; i++, temp >>=1 )
	{
		if( temp & 0x0001 )
		{
			if(DMA_ChannelIRQHandler[i] == NULL) printf(" DMA_IRQHandler : Handler(%d) is NULL\n",i);

			(DMA_ChannelIRQHandler[i])();
			DMA_ClrInterruptPend( i );
		}
	}
}



/*----------------------------------------------------------------------------*/
void DMA_RegisterIRQHandler
(
	U32 iChannel, 					// [in] :
	DMAIRQSubHandlerFn* pHandler 	// [in] :
)
{
	DMA_ChannelIRQHandler[iChannel] = pHandler;
}

#endif // DONT_USE_IRQHANDLER
/*----------------------------------------------------------------------------*/
CBOOL DMA_Initialize( U32 VirAddr) //in
{
	gpDMAC = (TDMAC_Reg*)VirAddr;

#ifdef CONFIG_USE_IRQ
	UnmaskIRQ( IRQ_DMAINT );		// Enable DMA interrupt
#else
#ifndef DONT_USE_IRQHANDLER
	// Register DMA IRQ handler
	RegisterIRQHandler( IRQ_DMAINT, (IRQSubhandler*)DMA_IRQHandler );
	UnmaskIRQ( IRQ_DMAINT );		// Enable DMA interrupt
#endif // DONT_USE_IRQHANDLER

#endif

	return CTRUE;
}


/*----------------------------------------------------------------------------*/
CBOOL
DMA_InitHardware( void )
{
	register TDMAChannel_Reg* pDMAChannel;
	int i;

	// Initalize DMA Registers
	for( i=0 ; i<DMAC_CHANNEL_COUNT ; i++ )
	{
		gpDMAC->IOMap[i].SRM = 0;	// Source I/O Request Map Register
		gpDMAC->IOMap[i].TRM = 0;	// Target I/O Request Map Register

		pDMAChannel = &(gpDMAC->Channel[i]);
		pDMAChannel->DMACOM0 = 0;		// 0x00 : Command Register 0
		pDMAChannel->DMACOM1 = 0;		// 0x02 : Command Register 1
		pDMAChannel->DMACOM2 = 0;		// 0x04 : Command Register 2
		pDMAChannel->DMACONS = 0;		// 0x06 : Control/Status Register
		pDMAChannel->SRCLADDR = 0;		// 0x08 : Source Low Address Register
		pDMAChannel->SRCHADDR = 0;		// 0x0A : Source High Address Register
		pDMAChannel->TRGLADDR = 0;		// 0x0C : Target Low Address Register
		pDMAChannel->TRGHADDR = 0;		// 0x0E : Target High Address Register

#ifndef DONT_USE_IRQHANDLER
#ifndef CONFIG_USE_IRQ
		DMA_RegisterIRQHandler( i, (DMAIRQSubHandlerFn*)NULL );
#endif	//DONT_IRQ
#endif // DONT_USE_IRQHANDLER
	}
	return CTRUE;
}


/*----------------------------------------------------------------------------*/
CBOOL
DMA_CloseHardware( void )
{
	return CTRUE;
}


/*----------------------------------------------------------------------------*/
CBOOL
DMA_Close( void )
{
#ifndef DONT_USE_IRQHANDLER
	MaskIRQ( IRQ_DMAINT );		// Disable Interrupt
#endif // DONT_USE_IRQHANDLER

	// Uninitialize global variables
	gpDMAC = (TDMAC_Reg*)NULL;

	return CTRUE;
}


/*----------------------------------------------------------------------------*/
void
DMA_SetChannel
(
	TDMAC_ChannelData* pDMAC_ChannelData 	// [in] :
)
{
	register TDMAChannel_Reg* pDMAChannel;
	TDMAC_Format* pSrcFormat;
	TDMAC_Format* pTrgFormat;
	U16 temp0, temp1, temp2;
	TDMARequestMap_Reg* pRequestMap;

	pDMAChannel = &(gpDMAC->Channel[pDMAC_ChannelData->iChannel]);
	pRequestMap = &(gpDMAC->IOMap[pDMAC_ChannelData->iChannel]);
	pSrcFormat = &(pDMAC_ChannelData->Source);
	pTrgFormat = &(pDMAC_ChannelData->Target);

	switch( pDMAC_ChannelData->dwBurstType )
	{
		case DMA_BURSTTYPE_NOBURST :
			temp0 = SET_VALUE(DMAC_BURSTSIZE, DMAC_BURSTSIZE_NOBURST);
			break;
		case DMA_BURSTTYPE_4WORD :
			temp0 = SET_VALUE(DMAC_BURSTSIZE, DMAC_BURSTSIZE_4WORD);
			break;
		case DMA_BURSTTYPE_8WORD :
			temp0 = SET_VALUE(DMAC_BURSTSIZE, DMAC_BURSTSIZE_8WORD);
			break;
		default : ; //ERRORMSG( 1, (TEXT("   DMAC_SetChannel : BurstType(0x%08lX) is NOT Support.\r\n"), pDMAC_ChannelData->dwBurstType));

	}

	// Config source format
	if( pSrcFormat->iStep == 0 )
	{
		temp0 |= SET_VALUE(DMAC_SRCADDRINC, 0 );
		temp1 = SET_VALUE(DMAC_SRCADDRSTEP, 0 );
	}
	else
	{
		temp0 |= SET_VALUE(DMAC_SRCADDRINC, 1);
		temp1 = SET_VALUE(DMAC_SRCADDRSTEP, (char)pSrcFormat->iStep);
	}

	if( pSrcFormat->dwType & DMA_MEMTYPE_IOMASK )
	{
		temp0 |= SET_VALUE(DMAC_FLOWSRC, 1);
		pRequestMap->SRM = (volatile U16)(SET_VALUE(DMAC_DCHSRM_VALID, 1)
						 | SET_VALUE(DMAC_DCHSRM_NUM, pSrcFormat->IOIndex));
		if( pSrcFormat->dwType & DMA_MEMTYPE_IO_HWORD )
		{
			temp0 |= SET_VALUE(DMAC_SRCPERIWD, DMAC_SRCPERIWD_HALFWORD);
		}
	}
	else
	{
		if( pSrcFormat->dwType & DMA_MEMTYPE_MEM_LSHWORD )
			temp0 |= SET_VALUE(DMAC_SRCMEMFMT, DMAC_SRCMEMFMT_LSHALFWORD);
		else if( pSrcFormat->dwType & DMA_MEMTYPE_MEM_MSHWORD )
			temp0 |= SET_VALUE(DMAC_SRCMEMFMT, DMAC_SRCMEMFMT_MSHALFWORD);
		pRequestMap->SRM = 0;
	}

	// Config target format
	if( pTrgFormat->iStep == 0 )
	{
		temp0 |= SET_VALUE(DMAC_TRGADDRINC, 0 );
		temp1 |= SET_VALUE(DMAC_TRGADDRSTEP, 0 );
	}
	else
	{
		temp0 |= SET_VALUE(DMAC_TRGADDRINC, 1);
		temp1 |= SET_VALUE(DMAC_TRGADDRSTEP, (char)pTrgFormat->iStep);
	}

	if( pTrgFormat->dwType & DMA_MEMTYPE_IOMASK )
	{
		temp0 |= SET_VALUE(DMAC_FLOWTRG, 1);
		pRequestMap->TRM = (volatile U16)(SET_VALUE(DMAC_DCHTRM_VALID, 1)
						 | SET_VALUE(DMAC_DCHTRM_NUM, pTrgFormat->IOIndex));
		if( pTrgFormat->dwType & DMA_MEMTYPE_IO_HWORD )
		{
			temp0 |= SET_VALUE(DMAC_TRGPERIWD, DMAC_TRGPERIWD_HALFWORD);
		}
	}
	else
	{
		if( pTrgFormat->dwType & DMA_MEMTYPE_MEM_LSHWORD )
			temp0 |= SET_VALUE(DMAC_TRGMEMFMT, DMAC_TRGMEMFMT_LSHALFWORD);
		else if( pTrgFormat->dwType & DMA_MEMTYPE_MEM_MSHWORD )
			temp0 |= SET_VALUE(DMAC_TRGMEMFMT, DMAC_TRGMEMFMT_MSHALFWORD);
		pRequestMap->TRM = 0;
	}

	pDMAChannel->DMACOM0 = temp0;
	pDMAChannel->DMACOM1 = temp1;
//	ERRORMSG( pDMAC_ChannelData->dwDataSize > 0xFFFF, (TEXT("   DMA_SetChannel : DataSize(%ld) is greater than 64KB\r\n"), pDMAC_ChannelData->dwDataSize));
	pDMAChannel->DMACOM2 = (U16)(pDMAC_ChannelData->dwDataSize);
	temp2 = 0;
	if( pSrcFormat->bFlyBy )	temp2 |= SET_VALUE(DMAC_FLYBYS, 1);
	if( pTrgFormat->bFlyBy )	temp2 |= SET_VALUE(DMAC_FLYBYT, 1);
	if( pDMAC_ChannelData->dwIntType & DMA_INTTYPE_END )
		temp2 |= SET_VALUE(DMAC_ENDIRQEN, 1);
	if( pDMAC_ChannelData->dwIntType & DMA_INTTYPE_STOP )
		temp2 |= SET_VALUE(DMAC_STOPIRQEN, 1);
	pDMAChannel->DMACONS = temp2;
	pDMAChannel->SRCLADDR = (U16)((pSrcFormat->dwAddr & 0x0000FFFF)>> 0);
	pDMAChannel->SRCHADDR = (U16)((pSrcFormat->dwAddr & 0xFFFF0000)>>16);
	pDMAChannel->TRGLADDR = (U16)((pTrgFormat->dwAddr & 0x0000FFFF)>> 0);
	pDMAChannel->TRGHADDR = (U16)((pTrgFormat->dwAddr & 0xFFFF0000)>>16);


}

/*----------------------------------------------------------------------------*/
void
DMA_RunChannel
(
	U32 iChannel	// [in] :
)
{
	SET_BITS( gpDMAC->Channel[iChannel].DMACONS, DMAC_DMARUN, 1);
}

/*----------------------------------------------------------------------------*/
void
DMA_StopChannel
(
	U32 iChannel 	// [in] :
)
{
	SET_BITS( gpDMAC->Channel[iChannel].DMACONS, DMAC_DMARUN, 0);
}

/*----------------------------------------------------------------------------*/
CBOOL
DMA_IsRunChannel
(
	U32 iChannel 	// [in] :
)
{
	return (CBOOL)GET_BOOL( gpDMAC->Channel[iChannel].DMACONS, DMAC_DMARUN);
}

/*----------------------------------------------------------------------------*/
void
DMA_SetSrcAddr
(
	U32 iChannel, 	// [in] :
	U32 dwAddr 	// [in] :
)
{
	gpDMAC->Channel[iChannel].SRCLADDR = (U16)((dwAddr & 0x0000FFFF)>> 0);
	gpDMAC->Channel[iChannel].SRCHADDR = (U16)((dwAddr & 0xFFFF0000)>>16);
}

/*----------------------------------------------------------------------------*/
U32
DMA_GetSrcAddr
(
	U32 iChannel 	// [in] :
)
{
	return ((U32)gpDMAC->Channel[iChannel].SRCLADDR)
		 | (((U32)gpDMAC->Channel[iChannel].SRCHADDR)<<16);
}

/*----------------------------------------------------------------------------*/
void
DMA_SetTrgAddr
(
	U32 iChannel, 	// [in] :
	U32 dwAddr 		// [in] :
)
{
	gpDMAC->Channel[iChannel].TRGLADDR = (U16)((dwAddr & 0x0000FFFF)>> 0);
	gpDMAC->Channel[iChannel].TRGHADDR = (U16)((dwAddr & 0xFFFF0000)>>16);
}

/*----------------------------------------------------------------------------*/
U32
DMA_GetTrgAddr
(
	U32 iChannel 	// [in] :
)
{
	return ((U32)gpDMAC->Channel[iChannel].TRGLADDR)
		 | (((U32)gpDMAC->Channel[iChannel].TRGHADDR)<<16);
}

/*----------------------------------------------------------------------------*/
void
DMA_SetDataSize
(
	U32 iChannel, 	// [in] :
	U32 dwSize 		// [in] :
)
{
//	ERRORMSG( dwSize > 0xFFFF, (TEXT("   DMA_SetDataSize : DataSize(%ld) is greater than 64KB\r\n"), dwSize));
	gpDMAC->Channel[iChannel].DMACOM2 = (U16)(dwSize);
}

/*----------------------------------------------------------------------------*/
U32
DMA_GetDataSize
(
	U32 iChannel 	// [in] :
)
{
	return (U32)gpDMAC->Channel[iChannel].DMACOM2;
}

/*----------------------------------------------------------------------------*/
U32
DMA_GetChannelStatus
(
	U32 iChannel 	// [in] :
)
{
	return (U32)(gpDMAC->Channel[iChannel].DMACONS);
}

/*----------------------------------------------------------------------------*/
void
DMA_SetInterruptEnb
(
	U32 iChannel,	// [in] :
	U32 Type 		// [in] :
)
{
	U16 temp;

	temp = gpDMAC->Channel[iChannel].DMACONS;
	if( Type & DMA_INTTYPE_END )		SET_BITS( temp, DMAC_ENDIRQEN, 1);
	else								SET_BITS( temp, DMAC_ENDIRQEN, 0);
	if( Type & DMA_INTTYPE_STOP )		SET_BITS( temp, DMAC_STOPIRQEN, 1);
	else								SET_BITS( temp, DMAC_STOPIRQEN, 0);
	gpDMAC->Channel[iChannel].DMACONS = temp;
}

/*----------------------------------------------------------------------------*/
CBOOL
DMA_GetInterruptPend
(
	U32 iChannel 	// [in] :
)
{
	return (CBOOL)((gpDMAC->DMAINT & (1 << iChannel)) != 0);
}

/*----------------------------------------------------------------------------*/
void
DMA_ClrInterruptPend
(
	U32 iChannel 	// [in] :
)
{
	U16 temp;

	temp = gpDMAC->Channel[iChannel].DMACONS;
	SET_BITS( temp, DMAC_ESINTR, 0 );
	gpDMAC->Channel[iChannel].DMACONS = temp;
}

/*----------------------------------------------------------------------------*/
U16
DMA_GetInterruptPendAll( void )
{
	return (U16)(gpDMAC->DMAINT);
}

/*----------------------------------------------------------------------------*/
void
DMA_SetChannelIOIndex
(
	U32 iChannel,				// [in] :
	CBOOL IsSource, 			// [in] :
	TDMAC_RequestSource Index 	// [in] :
)
{
	register TDMAChannel_Reg* pDMAChannel;
	pDMAChannel = &(gpDMAC->Channel[iChannel]);

	if( IsSource )
	{
	//ERRORMSG( pDMAChannel->DMACOM0 & BIT_MASK(DMAC_FLOWSRC), (TEXT("   DMA_SetChannelIOIndex : A channel source format is NOT IO.(0x%04X)\r\n"), pDMAChannel->DMACOM0));
		gpDMAC->IOMap[iChannel].SRM = (volatile U16)(SET_VALUE(DMAC_DCHSRM_VALID, 1)
									| SET_VALUE(DMAC_DCHSRM_NUM, Index));
	}
	else
	{
//		ERRORMSG( pDMAChannel->DMACOM0 & BIT_MASK(DMAC_FLOWTRG), (TEXT("   DMA_SetChannelIOIndex : A channel target format is NOT IO.(0x%04X)\r\n"), pDMAChannel->DMACOM0));
		gpDMAC->IOMap[iChannel].TRM = (volatile U16)(SET_VALUE(DMAC_DCHTRM_VALID, 1)
									| SET_VALUE(DMAC_DCHTRM_NUM, Index));
	}
}
