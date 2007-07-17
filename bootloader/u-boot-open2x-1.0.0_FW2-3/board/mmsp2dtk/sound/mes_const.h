//
//	Copyright (C) 2003 MagicEyes Digital Co., All Rights Reserved
//	MagicEyes Digital Co. Proprietary & Confidential
//
//	MMSP 2.0 BASE
//
//	- Constant for system setting
//
//	Charlie Myung,
//
//	history
//		2004/01/09 Tony  DMA_CH08 이 08(8진수) 로 정의되어 있는문제 수정
//

#ifndef	_MMSP2CONST_H
#define	_MMSP2CONST_H



//------------------------------------------------------------------------
// System Constants
//------------------------------------------------------------------------
#define	NUM_OF_IRQ			32
#define	NUM_OF_DMA			16
//
#define	CPU_ARM920			0x0920
#define	CPU_ARM940			0x0940
//
#define PAGE_TABLESIZE		0x1000


//------------------------------------------------------------------------
// Program Status Register
//------------------------------------------------------------------------
#define	SVC_MODE			0x13
#define	UNDEF_MODE			0x1B
#define	ABORT_MODE			0x17
#define	IRQ_MODE			0x12
#define	FIQ_MODE			0x11
#define	MODE_MASK			0x1F
#define	NO_INT				0xC0


//------------------------------------------------------------------------
// Coprocessor Register 1
//------------------------------------------------------------------------
#define	MMU_ON				0x0001
#define	DCACHE_ON			0x0004
#define	ICACHE_ON			0x1000


//------------------------------------------------------------------------
// Interrupt Controller
//------------------------------------------------------------------------
// IRQ number
#define	IRQ_DMA_NUM			09
#define	IRQ_MPEGIF_NUM			11

// IRQ vectors
#define	IRQ_DMA				0x00000200


//------------------------------------------------------------------------
// DMA
//------------------------------------------------------------------------
// DMA channels
#define	DMA_CH00			0
#define	DMA_CH01			1
#define	DMA_CH02			2
#define	DMA_CH03			3
#define	DMA_CH04			4
#define	DMA_CH05			5
#define	DMA_CH06			6
#define	DMA_CH07			7
#define	DMA_CH08			8
#define	DMA_CH09			9
#define	DMA_CH10			10
#define	DMA_CH11			11
#define	DMA_CH12			12
#define	DMA_CH13			13
#define	DMA_CH14			14
#define	DMA_CH15			15

// DMA channelIO source
#define	DMA_MAP_VALID		0x0020		// map valid bit
#define	DMAIO_UART0			0x0002		// UART0 receive/transmit
#define	DMAIO_UART1			0x0003		// UART1 receive/transmit
#define	DMAIO_AC97MIC		0x0006		// AC97 microphone in
#define	DMAIO_AC97OUT		0x0007		// AC97 audio left/right out
#define	DMAIO_AC97SOUT		0x0008		// AC97 audio s-left/s-right out
#define	DMAIO_AC97IN		0x0009		// AC97 audio left/right in
// DMA command register 0
#define	BRSTOPRSIZE_8W		0x8000		// 8 words
#define	BRSTOPRSIZE_4W		0x4000		// 4 words
#define	BRSTOPRSIZE_1W		0x0000		// 1 word
#define	INCSRCADDR			0x2000		// increment (src)
#define	FLOWSRC_RQST		0x1000		// wait for a request signal (src)
#define	SRCMEMFMT_MHW		0x0800		// MS half-word
#define	SRCMEMFMT_LHW		0x0400		// LS half-word
#define	SRCMEMFMT_WRD		0x0000		// word format
#define	SRCPERIWIDTH_HW		0x0100		// half-word
#define	SRCPERIWIDTH_B		0x0000		// byte
#define	INCTRGADDR			0x0020		// increment (trg)
#define	FLOWTRG_RQST		0x0010		// wait for a request signal (trg)
#define	TRGMEMFMT_MHW		0x0008		// MS half-word
#define	TRGMEMFMT_LHW		0x0004		// LS half-word
#define	TRGMEMFMT_WRD		0x0000		// word
#define	TRGPERIWIDTH_HW		0x0001		// half-word
#define	TRGPERIWIDTH_B		0x0000		// byte
// DMA control/status register
#define	DMARUN				0x0400		// start the channel
#define	ENDIRQEN			0x0200		// enable end-state irq
#define	STOPIRQEN			0x0100		// enable stop-state irq
#define	FLYBYWAITCYC_7		0x00E0		// fly-by operation wait cycle
#define	FLYBYWAITCYC_6		0x00C0		// 6 wait cycles
#define	FLYBYWAITCYC_5		0x00A0		// 5 wait cycles
#define	FLYBYWAITCYC_4		0x0080		// 4 wait cycles
#define	FLYBYWAITCYC_3		0x0060		// 3 wait cycles
#define	FLYBYWAITCYC_2		0x0040		// 2 wait cycles
#define	FLYBYWAITCYC_1		0x0020		// 1 wait cycles
#define	FLYBYWAITCYC_0		0x0000		// 0 wait cycles
#define	FLYBYS_THROUGH		0x0000		// flow-through mode (src)
#define	FLYBYS_BY			0x0010		// fly-by mode (src)
#define	FLYBYT_THROUGH		0x0000		// flow-through mode (src)
#define	FLYBYT_BY			0x0008		// fly-by mode (src)
#define	REQPEND				0x0004		// pending request
#define	ENDINTR				0x0002
#define	STOPINTR			0x0001

#endif
