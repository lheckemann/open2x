//
//	Copyright (C) 2003 MagicEyes Digital Co., All Rights Reserved
//	MagicEyes Digital Co. Proprietary & Confidential
//
//	MMSP 2.0 BASE
//	
//	- MMSP 2.0 register map
//
//	Charlie Myung, 
//
//	history
//			2003/11/05 luke		add VLD_BASE
//
//	

#ifndef _MMSP2REG_H
#define	_MMSP2REG_H



//------------------------------------------------------------------------
//  Base Address
// -----------------------------------------------------------------------
#define	PCMCIASLOT0_BASE	0x20000000		// PCMCIA slot 0 base address
#define	PCMCIASLOT1_BASE	0x30000000		// PCMCIA slot 1 base address
#define	DRAMBANKA_BASE		0x40000000		// DRAM bankA base address
#define	DRAMBANKB_BASE		0x50000000		// DRAM bankB base address
#define	LOCALBUF_BASE		0x60000000		// local buffer base address
#define	IO_BASE				0xC0000000		// I/O base address
#define	CF_ATTRIB_BASE		0xB4000000		// CF (danny)
#define	CF_COMMON_BASE		0xB8000000		// CF (danny)
#define	VLD_BASE			0xE0000000		// VLD base address


//------------------------------------------------------------------------
//  IO Base Address
// -----------------------------------------------------------------------
#define	DMA_BASE			(0x0000+IO_BASE)	// DMA
#define	INTCON_BASE			(0x0800+IO_BASE)	// A920T interrupt controller
#define	TIMER_BASE			(0x0A00+IO_BASE)	// timer
#define	IIC_BASE			(0x0D00+IO_BASE)	// IIC
#define	AC97_BASE			(0x0E00+IO_BASE)	// AC97
#define	UART_BASE			(0x1200+IO_BASE)	// UART
#define	IRDA_BASE			(0x1300+IO_BASE)	// IrDA
#define	SD_BASE				(0x1500+IO_BASE)	// SD/MMC
#define	I2S_BASE			(0x4400+IO_BASE)	// I2S
#define A940T_INTCON_BASE   (0x4500+IO_BASE)	// A940T interrupt controller
#define	OWM_BASE			(0x4700+IO_BASE)	// OWM



//------------------------------------------------------------------------
// DMA Controller
//------------------------------------------------------------------------
#define	DMAINTREG			(*((volatile U16*)(DMA_BASE+0x000)))	// DMA interrupt status register
// base address
#define	DMACMD_BASE			(0x0200+DMA_BASE)						// DMA comman register base address
#define	DMASRCTRG_BASE		(0x0100+DMA_BASE)						// DMA src, trg request map reg. base address


#endif	// _REGMAP_H
