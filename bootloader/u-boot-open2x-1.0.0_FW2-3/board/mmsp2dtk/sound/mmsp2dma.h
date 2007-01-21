//------------------------------------------------------------------------------
//
// MMSP2 DMA Private Header file
// Copyright (C) MagicEyes Digital Co. 2003
//
// Module     : MMSP2 DMA Controller Private Header
// File       : mmsp2dma.h
// Description: Register definition of MMSP2 DMA Controller
// Author     : Goofy
// Export     : None
// History    : 
//		2003/10/27 Goofy	Redefine types
//		2003/10/08 Goofy	Change SRM & TRM Register Format
//							Change TDMAC_Reg format
//							Change DMAC_CHANNEL_COUNT to 16
//		2003/10/02 Goofy	First Release
//------------------------------------------------------------------------------
#ifndef __MMSP2DMA_H__
#define __MMSP2DMA_H__

#include "../sound/typedef.h"
#include "../sound/mes_macro.h"

#define		DMAC_CHANNEL_COUNT		16

//-----------------------------------------------------------------------------
//	DMA COMMAND 0 Register
//-----------------------------------------------------------------------------
// Burst Operation Request Size
#define	bsDMAC_BURSTSIZE		14
#define	bwDMAC_BURSTSIZE		2
#define	bmDMAC_BURSTSIZE		BIT_MASK(DMAC_BURSTSIZE)

	#define DMAC_BURSTSIZE_NOBURST	0
	#define DMAC_BURSTSIZE_4WORD	1
	#define DMAC_BURSTSIZE_8WORD	2

// Source Address Increment Setting
// 0 : Disable
// 1 : Enable Increment/Decrement
#define	bsDMAC_SRCADDRINC		13
#define	bwDMAC_SRCADDRINC		1
#define	bmDMAC_SRCADDRINC		BIT_MASK(DMAC_SRCADDRINC)

// Source Flow Control
// 0 : No wait. Start the data transfer immediately
// 1 : Wait for the request signal before initializing the data transfer
#define	bsDMAC_FLOWSRC			12
#define	bwDMAC_FLOWSRC			1
#define	bmDMAC_FLOWSRC			BIT_MASK(DMAC_FLOWSRC)

// Source Memory Data Format
#define	bsDMAC_SRCMEMFMT		10
#define	bwDMAC_SRCMEMFMT		2
#define	bmDMAC_SRCMEMFMT		BIT_MASK(DMAC_SRCMEMFMT)

	#define DMAC_SRCMEMFMT_WORD			0
	#define DMAC_SRCMEMFMT_LSHALFWORD	1
	#define DMAC_SRCMEMFMT_MSHALFWORD	2

// Source IO Device Data Width
#define	bsDMAC_SRCPERIWD		8
#define	bwDMAC_SRCPERIWD		2
#define	bmDMAC_SRCPERIWD		BIT_MASK(DMAC_SRCPERIWD)

	#define DMAC_SRCPERIWD_BYTE		0
	#define DMAC_SRCPERIWD_HALFWORD	1

// Target Address Increment/Decrement Setting
// 0 : Disable
// 1 : Enable Increment/Decrement
#define	bsDMAC_TRGADDRINC		5
#define	bwDMAC_TRGADDRINC		1
#define	bmDMAC_TRGADDRINC		BIT_MASK(DMAC_TRGADDRINC)

// Target Flow Control
// 0 : No wait. Start the data transfer immediately
// 1 : Wait for the request signal before initializing the data transfer
#define	bsDMAC_FLOWTRG			4
#define	bwDMAC_FLOWTRG			1
#define	bmDMAC_FLOWTRG			BIT_MASK(DMAC_FLOWTRG)

// Target Memory Data Format
#define	bsDMAC_TRGMEMFMT		2
#define	bwDMAC_TRGMEMFMT		2
#define	bmDMAC_TRGMEMFMT		BIT_MASK(DMAC_TRGMEMFMT)

	#define DMAC_TRGMEMFMT_WORD			0
	#define DMAC_TRGMEMFMT_LSHALFWORD	1
	#define DMAC_TRGMEMFMT_MSHALFWORD	2

// Target IO Device Data Width
#define	bsDMAC_TRGPERIWD		0
#define	bwDMAC_TRGPERIWD		2
#define	bmDMAC_TRGPERIWD		BIT_MASK(DMAC_TRGPERIWD)

	#define DMAC_TRGPERIWD_BYTE		0
	#define DMAC_TRGPERIWD_HALFWORD	1

//-----------------------------------------------------------------------------
//	DMA COMMAND 1 Register
//-----------------------------------------------------------------------------
// Source Address Increment/Decrement by word
#define bsDMAC_SRCADDRSTEP		8
#define bwDMAC_SRCADDRSTEP		8
#define bmDMAC_SRCADDRSTEP		BIT_MASK(DMAC_SRCADDRSTEP)

#define bsDMAC_TRGADDRSTEP		0
#define bwDMAC_TRGADDRSTEP		8
#define bmDMAC_TRGADDRSTEP		BIT_MASK(DMAC_TRGADDRSTEP)

//-----------------------------------------------------------------------------
//	DMA COMMAND 2 Register
//-----------------------------------------------------------------------------
#define bsDMAC_DMASIZE			0
#define bwDMAC_DMASIZE			16
#define bmDMAC_DMASIZE			BIT_MASK(DMAC_DMASIZE)


//-----------------------------------------------------------------------------
//	DMA Control/Satus Register
//-----------------------------------------------------------------------------
// DMA Enable
#define	bsDMAC_DMARUN			10
#define	bwDMAC_DMARUN			1
#define	bmDMAC_DMARUN			BIT_MASK(DMAC_DMARUN)

// End Interrupt Enable
#define	bsDMAC_ENDIRQEN			9
#define	bwDMAC_ENDIRQEN			1
#define	bmDMAC_ENDIRQEN			BIT_MASK(DMAC_ENDIRQEN)

// Enable the interrupt if the channel enters stop state
#define	bsDMAC_STOPIRQEN		8
#define	bwDMAC_STOPIRQEN		1
#define	bmDMAC_STOPIRQEN		BIT_MASK(DMAC_STOPIRQEN)

// Fly-By Operation Wait Cycle
#define	bsDMAC_FLYBYWTCYC		5
#define	bwDMAC_FLYBYWTCYC		3
#define	bmDMAC_FLYBYWTCYC		BIT_MASK(DMAC_FLYBYWTCYC)

// Fly-By for Memory Read
#define	bsDMAC_FLYBYS			4
#define	bwDMAC_FLYBYS			1
#define	bmDMAC_FLYBYS			BIT_MASK(DMAC_FLYBYS)

// Fly-By for Memory Write
#define	bsDMAC_FLYBYT			3
#define	bwDMAC_FLYBYT			1
#define	bmDMAC_FLYBYT			BIT_MASK(DMAC_FLYBYT)

// Status for a Pending Request
#define	bsDMAC_REQPEND			2
#define	bwDMAC_REQPEND			1
#define	bmDMAC_REQPEND			BIT_MASK(DMAC_REQPEND)

// Status for a End Interrupt
#define	bsDMAC_ENDINTR			1
#define	bwDMAC_ENDINTR			1
#define	bmDMAC_ENDINTR			BIT_MASK(DMAC_ENDINTR)

// Status for a Stop Interrupt
#define	bsDMAC_STOPINTR			0
#define	bwDMAC_STOPINTR			1
#define	bmDMAC_STOPINTR			BIT_MASK(DMAC_STOPINTR)

// Status for END & STOP Interrupt
#define	bsDMAC_ESINTR			0
#define	bwDMAC_ESINTR			2
#define	bmDMAC_ESINTR			BIT_MASK(DMAC_ESINTR)

//-----------------------------------------------------------------------------
//	DMA Channel Source I/O Request Map Register
//-----------------------------------------------------------------------------
// Valid bit
#define	bsDMAC_DCHSRM_VALID		6
#define	bwDMAC_DCHSRM_VALID		1
#define	bmDMAC_DCHSRM_VALID		BIT_MASK(DMAC_DCHSRM_VALID)
// Peripheral Device Number
#define	bsDMAC_DCHSRM_NUM		0
#define	bwDMAC_DCHSRM_NUM		6
#define	bmDMAC_DCHSRM_NUM		BIT_MASK(DMAC_DCHSRM_NUM)


//-----------------------------------------------------------------------------
//	DMA Channel Target I/O Request Map Register
//-----------------------------------------------------------------------------
// Valid bit
#define	bsDMAC_DCHTRM_VALID		6
#define	bwDMAC_DCHTRM_VALID		1
#define	bmDMAC_DCHTRM_VALID		BIT_MASK(DMAC_DCHTRM_VALID)
// Peripheral Device Number
#define	bsDMAC_DCHTRM_NUM		0
#define	bwDMAC_DCHTRM_NUM		6
#define	bmDMAC_DCHTRM_NUM		BIT_MASK(DMAC_DCHTRM_NUM)


typedef struct
{
	U16 volatile SRM;			// Source I/O Request Map Register
	U16 volatile TRM;			// Target I/O Request Map Register
} TDMARequestMap_Reg;

typedef struct 
{
	U16 volatile DMACOM0; 			// 0x00 : Command Register 0
	U16 volatile DMACOM1;			// 0x02 : Command Register 1
	U16 volatile DMACOM2;			// 0x04 : Command Register 2
	U16 volatile DMACONS;			// 0x06 : Control/Status Register
	U16 volatile SRCLADDR;			// 0x08 : Source Low Address Register
	U16 volatile SRCHADDR;			// 0x0A : Source High Address Register
	U16 volatile TRGLADDR;			// 0x0C : Target Low Address Register
	U16 volatile TRGHADDR;			// 0x0E : Target High Address Register
} TDMAChannel_Reg;

typedef struct
{
	// Base : 0xC0000000
	U16	volatile 		DMAINT;							// 0x000 : Interrupt Status Register
	U8	volatile 		_reserved0[0xFE];				// 0x002 ~ 0x0FF 
	TDMARequestMap_Reg 	IOMap[DMAC_CHANNEL_COUNT];		// 0x100 ~ 0x13F : I/O Request MAP Register
	U8	volatile 		_reserved1[0xC0];				// 0x140 ~ 0x1FF 
	TDMAChannel_Reg		Channel[DMAC_CHANNEL_COUNT];	// 0x200 ~ 0x2FF: DMA Channel 0~15
} TDMAC_Reg;


// define DMA_BASE in mes_iomap.h
#define MMSP2_DMAC_PHYSICAL_BASEADDRESS		(DMA_BASE)

#endif // __MMSP2DMA_H__
