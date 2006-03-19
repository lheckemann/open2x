/******************************************************
  NAME     : mmsp20_type.h
  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
  Kane Ahn < hbahn@dignsys.com >
  hhsong < hhsong@dignsys.com >
*******************************************************/

#ifndef __MMSP20_TYPE_H__
#define __MMSP20_TYPE_H__

#include <asm/arch/bitfield.h>

typedef volatile unsigned char     MMSP20_REG8;
typedef volatile unsigned short    MMSP20_REG16;
typedef volatile unsigned int      MMSP20_REG32;

/* BANK C MEMORY CONTROLLER REGISTER */

typedef struct {
	MMSP20_REG16	MEMCFGW;  	// 0xC0003A00, Configration register 
#define IDEBUF		0x0800		// IDE	
					// 0:Bypass, 1:Buffered
#define CDBUF		0x0400		// PCMCIA
					// 0:Bypass, 1:Buffered
#define NFBUF		0x0200		// NAND Flash 
					// 0:Bypass, 1:Buffered
#define SRBUF		0x0100		// Static 
					// 0:Bypass, 1:Buffered
#define NANDBW		0x0080		// NAND Flash
					// 0:Byte, 1:Half-Word
#define SR3BW		0x0008		// STATIC #3 Data bit 
					// 0:Byte, 1:Half-Word
#define SR2BW		0x0004		// STATIC #2 Data bit
					// 0:Byte, 1:Half-Word
#define SR1BW		0x0002		// STATIC #1 Data bit
					// 0:Byte, 1:Half-Word
#define SR0BW		0x0001		// STATIC #0 Data bit
					// 0:Byte, 1:Half-Word
//------------------------------------------------------------------------
	MMSP20_REG16	MEMTIMEW0;	// 0xC0003A02, Timing register 0
#define BF_tACS3	Fld(2,6)	//
#define tACS3		FMsk(BF_tACS3)	// tACS of STATIC #3
#define BF_tACS2	Fld(2,4)
#define tACS2		FMsk(BF_tACS2)	// tACS of STATIC #2
#define BF_tACS1	Fld(2,2)
#define tACS1		FMsk(BF_tACS1)	// tACS of STATIC #1
#define BF_tACS0	Fld(2,0)	
#define tACS0		FMsk(BF_tACS0)	// tACS of STATIC #0 
//------------------------------------------------------------------------
	MMSP20_REG16	MEMTIMEW1;	// 0xC0003A04, Timing register 1
#define BF_tCOS3	Fld(2,6)
#define tCOS3		FMsk(BF_tCOS3)	// tCOS of STATIC #3
#define BF_tCOS2	Fld(2,4)	
#define tCOS2		FMsk(BF_tCOS2)	// tCOS of STATIC #2
#define BF_tCOS1	Fld(2,2)
#define tCOS1		FMsk(BF_tCOS1)	// tCOS of STATIC #1
#define BF_tCOS_S0	Fld(2,0)
#define tCOS0		FMsk(BF_tCOS0)	// tCOS of STATIC #0
//------------------------------------------------------------------------
	MMSP20_REG16	MEMTIMEW2;	// 0xC0003A06, Timing register 2
#define BF_tOCH3	Fld(2,6)
#define tOCH3		FMsk(BF_tOCH3)	// tOCH of STATIC #3
#define BF_tOCH2	Fld(2,4)
#define tOCH2		FMsk(BF_tOCH2)	// tOCH of STATIC #2
#define BF_tOCH1	Fld(2,2)
#define tOCH1		FMsk(BF_tOCH1)	// tOCH of STATIC #1
#define BF_tOCH0	Fld(2,0)
#define tOCH0		FMsk(BF_tOCH0)	// tOCH of STATIC #0
//------------------------------------------------------------------------
	MMSP20_REG16	MEMTIMEW3;	// 0xC0003A08, Timing register 3
#define BF_tCAH3	Fld(2,6)
#define tCAH3		FMsk(BF_tCAH3)	// tCAH of STATIC #3
#define BF_tCAH2	Fld(2,4)
#define tCAH2		FMsk(BF_tCAH2)	// tCAH of STATIC #2
#define BF_tCAH1	Fld(2,2)
#define tCAH1		FMsk(BF_tCAH1)	// tCAH of STATIC #1
#define BF_tCAH0	Fld(2,0)
#define tCAH0		FMsk(BF_tCAH0)	// tCAH of STATIC #0
//------------------------------------------------------------------------
	MMSP20_REG16	MEMTIMEW4;	// 0xC0003A0A, Timing register 4
#define BF_tACC3	Fld(2,6)
#define tACC3		FMsk(BF_tACC3)	// tACC of STATIC #3
#define BF_tACC2	Fld(2,4)	
#define tACC2		FMsk(BF_tACC2)	// tACC of STATIC #2
#define BF_tACC1	Fld(2,2)
#define tACC1		FMsk(BF_tACC1)	// tACC of STATIC #1
#define BF_tACC0	Fld(2,0)
#define tACC0		FMsk(BF_tACC0)	// tACC of STATIC #0
//------------------------------------------------------------------------
	MMSP20_REG16	RESERVE1;	/* RESERVE */
//------------------------------------------------------------------------
	MMSP20_REG16	MEMWAITCTRLW;	// 0xC0003A0E, Wait control register 
#define BF_WAITENB3	Fld(2,6)
#define WAITENB3	FMsk(BF_WAITENB3)	// Wait Control Enable of STATIC#3
#define BF_WAITENB2	Fld(2,4)
#define WAITENB2	FMsk(BF_WAITENB2)	// Wait Control Enable of STATIC#2
#define BF_WAITENB1	Fld(2,2)
#define WAITENB1	FMsk(BF_WAITENB1)	// Wait Control Enable of STATIC#1
#define BF_WAITENB0	Fld(2,0)
#define WAITENB0	FMsk(BF_WAITENB0)	// Wait Control Enable of STATIC#0
//------------------------------------------------------------------------
	MMSP20_REG16	MEMPAGEW;	// 0xC0003A10, Page Control register
#define SRPAGE3		0x0800		// SRPAGE of STATIC #3
#define SRPAGE2		0x0400		// SRPAGE of STATIC #2
#define SRPAGE1		0x0200		// SRPAGE of STATIC #1
#define SRPAGE0		0x0100		// SRPAGE of STATIC #0
#define IOIS16ENB	0x0040		// Enable IOIS16 Control of PCMCIA Region
//------------------------------------------------------------------------
	MMSP20_REG16	MEMIDETIMEW;	// 0xC0003A12, IDE Timinig Control Register
#define BF_tIHOLD	Fld(5,10)
#define tIHOLD		FMsk(BF_tIHOLD)	// Data Hold Time of IDE
#define BF_tISETUP	Fld(5,5)
#define tISETUP		FMsk(BF_tISETUP)// Data Setup Time of IDE
#define BF_tIACC	Fld(5,0)
#define tIACC		FMsk(BF_tIACC)	// Data Access Time of IDE
//------------------------------------------------------------------------
	MMSP20_REG16	MEMPCMCIAMW;	// 0xC0003A14, PCMCIA Memory Timinig Control Register 
#define BF_tPMHOLD	Fld(5,10)
#define tPMHOLD		FMsk(BF_tPMHOLD)	// Data Hold Time of PCMCIA Memory Region
#define BF_tPMSETUP	Fld(5,5)
#define tPMSETUP	FMsk(BF_tPMSETUP)	// Data Setup Time of PCMCIA Memory Region
#define BF_tPMACC	Fld(5,0)
#define tPMACC		FMsk(BF_tPMACC)		// Data Access Time of PCMCIA Memory Region
//------------------------------------------------------------------------
	MMSP20_REG16	MEMPCMCIAAW;    // 0xC0003A16, PCMCIA Attribute Timnig control Register
#define BF_tPAHOLD	Fld(5,10)
#define tPAHOLD		FMsk(BF_tPAHOLD)	// Data Hold Time of PCMCIA Attribute Region
#define BF_tPASETUP	Fld(5,5)
#define tPASETUP	FMsk(BF_tPASETUP)	// Data Setup Time of PCMCIA Attribute Region
#define BF_tPAACC	Fld(5,0)
#define tPAACC		FMsk(BF_tPAACC)		// Data Access Time of PCMCIA Attribute Region
//------------------------------------------------------------------------
	MMSP20_REG16	MEMPCMCIAIW;	// 0xC0003A18, PCMCIA I/O Timinig Register
#define BF_tPIHOLD	Fld(5,10)
#define tPIHOLD		FMsk(BF_tPIHOLD)	// Data Hold Time of PCMCIA I/O Region
#define BF_tPISETUP	Fld(5,5)
#define tPISETUP	FMsk(BF_tPISETUP)	// Data Setup Time of PCMCIA I/O Region
#define BF_tPIACC	Fld(5,0)
#define tPIACC		FMsk(BF_tPIACC)		// Data Access Time of PCMCIA I/O Region
//------------------------------------------------------------------------
	MMSP20_REG16	MEMPCMCIAWAITW; // 0xC0003A1A, PCMCIA Wait Timinig Register #0
#define BF_PCMWAITM	Fld(5,10)
#define PCMWAITM	FMsk(BF_PCMWAITM)	// PCMCIA Wait Control for Memory Region
#define BF_PCMWAITA	Fld(5,5)
#define PCMWAITA	FMsk(BF_PCMWAITA)	// PCMCIA Wait Control for Attribute Region
#define BF_PCMWAITI	Fld(5,0)
#define PCMWAITI	FMsk(BF_PCMWAITI)	// PCMCIA Wait Control for I/O Region
//------------------------------------------------------------------------
	MMSP20_REG16	MEMIDEWAITW;	// 0xC0003A1C, IDE Wait timing Register #1
#define BF_IDEWAIT	Fld(5,0)
#define IDEWAIT		FMsk(BF_IDEWAIT)
//------------------------------------------------------------------------	
	MMSP20_REG16	RESERVE2;	/* RESERVE C000 1Eh */
//------------------------------------------------------------------------
	MMSP20_REG16	MEMDTIMEOUTW;	// 0xC0003A20, DMA Timeout Register
#define BF_DMATIMEOUT	Fld(5,10)
#define DMATIMEOUT	FMsk(BF_DMATIMEOUT)	// Timeout 
#define BF_DMALIMIT	Fld(10,0)
#define DMALIMIT	FMsk(BF_DMALIMIT)	// IDE
//------------------------------------------------------------------------
	MMSP20_REG16	MEMDMACTRLW;    // 0xC0003A22, DMA Control Register
#define DMAENB1		0x2000		// DMA Enable Channel 1
					// 0:Disable, 1:Enable
#define DMAENB0		0x1000		// DMA Enable Channel 0
					// 0:Disable, 1:Enable
#define DMAWIDTH1	0x0200		// Channel 1 DMA
					// 0:8bit, 1:16bit
#define DMAWIDTH0	0x0100		// Channel 1 DMA
					// 0:8bit, 1:16bit
#define BF_DMAMODE1	Fld(2,2)	
#define DMAMODE1	FMsk(BF_DMAMODE1)	// DMA Mode Channel 1
					// 00:Multi-Word DMA Read
					// 01:Multi-Word DMA Write
					// 10:Ultra-DMA Read
					// 11:Ultra-DMA Write
#define DMAMODE0	FMsk(BF_DMAMODE0)	// DMA Mode Channel 0
					// 00:Multi-Word DMA Read
					// 01:Multi-Word DMA Write
					// 10:Ultra-DMA Read
					// 11:Ultra-DMA Write
//------------------------------------------------------------------------
	MMSP20_REG16	MEMDMAPOLW;	// 0xC0003A24 DMA Polarity Register
#define DMAREQPOL1	0x2000		// DMA Ch1 WDREQ1 polarity
					// 0:Low Active, 1:High Active
#define DMAREQPOL0	0x1000		// DMA Ch0 WDREQ0 polarity
					// 0:Low Active, 1:High Active
#define DMAACKPOL1	0x0200		// DMA Ch1 WDACK1 polarity
					// 0:Low Active, 1:High Active
#define DMAACKPOL0	0x0100		// DMA Ch0 WDACK0 polarity
					// 0:Low Active, 1:High Active
#define BF_DMAPATH1	Fld(2,2)	
#define DMAPATH1	FMsk(BF_DMAPATH1)	// DMA Ch1 Path
					// 00:Static Memory
					// 01:IDE
					// 10:PCMCIA #0
					// 11:PCMCIA #1
#define BF_DMAPATH0	Fld(2,0)
#define DMAPATH0	FMsk(BF_DMAPATH0)	// DMA Ch0 Path
					// 00:Static Memory
					// 01:IDE
					// 10:PCMCIA #0
					// 11:PCMCIA #1
//------------------------------------------------------------------------
	MMSP20_REG16	MEMDMATIMEW0; 	// 0xC0003A26, DMA Timing Register 0 
#define BF_DMAtD1	Fld(5,5)
#define DMAtD1		FMsk(BF_DMAtD1)	// tD Timing Channel1 
#define BF_DMAtD0	Fld(5,0)
#define DMAtD0		FMsk(BF_DMAtD0)	// tD Timing Channel0
//------------------------------------------------------------------------
	MMSP20_REG16	MEMDMATIMEW1;	// 0xC0003A28, DMA Timing Register 1
#define BF_DMAtK1	Fld(5,5)	
#define DMAtK1		FMsk(BF_DMAtK1)	// tK Timing Channel1 
#define BF_DMAtK0	Fld(5,0)
#define DMAtK0		FMsk(BF_DMAtK0)	// tK Timing Channel0 
//------------------------------------------------------------------------
	MMSP20_REG16	MEMDMATIMEW2;	// 0xC0003A2A, DMA Timing Register 2
#define BF_DMAtACK1	Fld(5,5)
#define DMAtACK1	FMsk(BF_DMAtACK1)	// tACK Timing Channel1 
#define BF_DMAtACK0	Fld(5,0)
#define DMAtACK0	FMsk(BF_DMAtACK0)	// tACK Timing Channel0 
//------------------------------------------------------------------------
	MMSP20_REG16	MEMDMATIMEW3;	// 0xC0003A2C, DMA Timing Register 3
#define BF_DMAtENV1	Fld(5,5)
#define DMAtENV1	FMsk(BF_DMAtENV1)	// tENV Timing Channel1 
#define BF_DMAtENV0	Fld(5,0)
#define DMAtENV0	FMsk(BF_DMAtENV0)	// tENV Timing Channel0 
//------------------------------------------------------------------------
	MMSP20_REG16	MEMDMATIMEW4;	// 0xC0003A2E, DMA Timing Register 4
#define BF_DMAtRP1	Fld(5,5)
#define DMAtRP1		FMsk(BF_DMAtRP1)	// tRP Timing Channel1 
#define BF_DMAtRP0	Fld(5,0)
#define DMAtRP0		FMsk(BF_DMAtRP0)	// tRP Timing Channel0
//------------------------------------------------------------------------
	MMSP20_REG16	MEMDMATIMEW5;	// 0xC0003A30, DMA Timing Register 5
#define BF_DMAtSS1	Fld(5,5)
#define DMAtSS1		FMsk(BF_DMAtSS1)	// tSS Timing Channel1 
#define BF_DMAtSS0	Fld(5,0)
#define DMAtSS0		FMsk(BF_DMAtSS0)	// tSS Timing Channel0 
//------------------------------------------------------------------------
	MMSP20_REG16 	MEMDMATIMEW6;	// 0xC0003A32, DMA Timing Register 6
#define BF_DMAtMLI1	Fld(5,5)
#define DMAtMLI1	FMsk(BF_DMAtMLI1)	// tMLI Timing Channel1 
#define BF_DMAtMLI0	Fld(5,0)
#define DMAtMLI0	FMsk(BF_DMAtMLI0)	// tMLI Timing Channel0 
//------------------------------------------------------------------------
	MMSP20_REG16	MEMDMATIMEW7;	// 0xC0003A34, DMA Timing Register 7
//------------------------------------------------------------------------
	MMSP20_REG16	MEMDMATIMEW8;	// 0xC0003A36, DMA Timing Register 8
#define DMARST		0x1000		// Software Reset of DMA
					// 0:Idle, 1:Reset
//------------------------------------------------------------------------
	MMSP20_REG16	MEMDMASTRBW; 	// 0xC0003A38, DMA Strobe Control Register 
#define BF_DMAtCYC1	Fld(4,4)	
#define DMAtCYC1	FMsk(BF_DMAtCYC1)	// HSTROBE(1)pulse width
#define BF_DMAtCYC0	Fld(4,0)
#define DMAtCYC0	FMsk(BF_DMAtCYC0)	// HSTROBE(0)pulse width
//------------------------------------------------------------------------
	MMSP20_REG16	MEMNANDCTRLW;   // 0xC0003A3A, NAND CTRL register
#define IRQPEND		0x8000		// RnB Rising Detect Pending Register
#define RnB		0x0100		// NAND Flash Status
					// 1:Ready, 0:Busy
#define IRQENB		0x0080		// NAND Flash RnB rising edge
					// 0:Disable, 1:Enable
#define NFBOOTENB	0x0040		// NAND booting
					// 0:Disable, 1:Enable
#define NFBOOTSIZE	0x0020		// NAND booting
					// 0:2048bytes, 1:4096bytes
#define NFBUSWIDTH	0x0010		// NAND boot NAND flash Bus width
					// 0:8bit, 1:16bit
#define NFTYPE		0x0008		// NAND boot NAND flash Type
					// 0:3 address NAND, 1:4 address NAND
#define BF_NFBANK	Fld(3,0)
#define NFBANK		FMsk(BF_NFBANK)	// Access NAND flash Bank 
//------------------------------------------------------------------------ 
	MMSP20_REG16	MEMNANDTIMEW; 	// 0xC0003A3C, NAND Timing Register 
#define BF_NFtSETUP	Fld(4,8)
#define NFtSETUP	FMsk(BF_NFtSETUP)	// NAND Flash Data Setup time
#define BF_NFtACC	Fld(4,4)
#define NFtACC		FMsk(BF_NFtACC)	// NAND Flash Data Access time
#define BF_NFtHOLD	Fld(4,0)
#define NFtHOLD		FMsk(BF_NFtHOLD)	// NAND Flash Data Hold time
//------------------------------------------------------------------------
	MMSP20_REG16	MEMNANDECC0W; 	// 0xC0003A3E, NAND ECC Register 0 
#define BF_ECC0UPPER	Fld(8,8)
#define ECC0UPPER	FMsk(BF_ECC0UPPER)	// 16bit NAND flash
#define BF_ECC0LOWER	Fld(8,0)
#define ECC0LOWER	FMsk(BF_ECC0LOWER)	// 8bit NAND flash
//------------------------------------------------------------------------
	MMSP20_REG16	MEMNANDECC1W;	// 0xC0003A40, NAND ECC Register 1
#define BF_ECC1UPPER	Fld(8,8)
#define ECC1UPPER	FMsk(BF_ECC1UPPER)
#define BF_ECC1LOWER	Fld(8,0)
#define ECC1LOWER	FMsk(BF_ECC1LOWER)
//------------------------------------------------------------------------
	MMSP20_REG16	MEMNANDECC2W;	// 0xC0003A42, NAND ECC Register 2
#define BF_ECC2UPPER	Fld(8,8)
#define ECC2UPPER	FMsk(BF_ECC2UPPER)
#define BF_ECC2LOWER	Fld(8,0)
#define ECC2LOWER	FMsk(BF_ECC2LOWER)
//------------------------------------------------------------------------
	MMSP20_REG16	MEMNANDCNTW; 	// 0xC0003A44, NAND Data Counter Register 
#define BF_DATACNT	Fld(10,0)
#define DATACNT		FMsk(BF_DATACNT)	
/*	MMSP20_REG16	MEMCHIPSELTW;*/	/* CHIP selection control register -> none */ 
} MMSP20_BANC_MEMCTL; 		/* address C000 00h ~ 46h */

#if 0
/* BANK C Memory NAND Memory control */
typedef struct {
	/* address no sequential */
	MMSP20_REG16	NFDATA;		/* NAND Flash Data Register 1C000000h or 9C000000h*/
	MMSP20_REG16	RESERVE1[7];    /* 02, 04, 06, 08, 0a, 0c, 0e */
	MMSP20_REG16	NFCMD;		/* NAND Flash Command register 1C000010h or 9C000010h*/
	MMSP20_REG16	RESERVE2[3];    /* 12, 14, 16 */
	MMSP20_REG16	NFADDR;		/* NAND Flash Address Register 1C000018h or 9C000018h*/
} MMSP20_NAND;

/* BANK A Memory Controller register C000h */
typedef struct {
	MMSP20_REG16	MEMCFGX;	/* SDRAM Configuration Register 5400h */
	MMSP20_REG16	MEMTIMEX[2];	/* Timinig register 0, 1 */
	MMSP20_REG16	MEMACTPWDX; 	/* Active power down control register */
	MMSP20_REG16	MEMREFX;	/* Refresh Control register */
} MMSP20_BANA_MEMCTL;			/* C0005400h ~ C0005408 */
#endif

/* Dual cpu control register CXXX */
typedef struct {
	MMSP20_REG16 	DUAL920DATA[16];        /* ARM920 Data register 0 ~ 15 */
	MMSP20_REG16	DUAL940DATA[16];        /* ARM940 Data register 0 ~ 15 */
	MMSP20_REG16	DUALINT920;	        /* ARM920 interrupt enable register */
	MMSP20_REG16	DUALINT940;	        /* ARM940 interrupt enable register */
	MMSP20_REG16	DUALPEND920;	        /* ARM920 interrupt pending register */
	MMSP20_REG16	DUALPEND940;	        /* ARM940 interrupt pending register */
	MMSP20_REG16	DUALCTRL940;	        /* ARM940 control register */
} MMSP20_DUALCPU; /* CXXX0000 ~ 0048 */

/* Clock & Power Manager Register BASE C000 0900 */

typedef struct {
/* power */	
	MMSP20_REG16	PWMODEREG; 	/* Power mode register, 00h */
#define APLLDOWN	0x0200		// APLL Power down
#define UPLLDOWN	0x0100		// UPLL Power down
#define nBATTFSTOP	0x0010		// BATT_FAULTn , ARM920T CPU STOP mode
#define STOPMODE	0x0008		// ARM920T CPU STOP mode
#define DSLEEPMODE	0x0004		// ARM920T CPU DEEP SLEEP mode
#define SLEEPMODE	0x0002		// ARM920T CPU SLEEP mode
#define IDLEMODE	0x0001		// ARM920T CPU IDLE mode	
//---------------------------------------------------------------------------------------------
/* clock */
	MMSP20_REG16	CLKCHGSTREG;	/* Clock Change Status Register, 02h */
#define APLLCHG		0x0020		
#define UPLLCHG		0x0010
#define APLLCHGST	0x0004
#define UPLLCHGST	0x0002
#define FPLLCHGST	0x0001
//----------------------------------------------------------------------------------------------	
	MMSP20_REG16	SYSCLKENREG;	/* Clock Enable Register, 04h */
#define BF_ESYSCLK		Fld(1,14)
#define BF_ADCCLK		Fld(1,13)
#define BF_PWMCLK		Fld(1,12)
#define BF_FASTIOCLK	Fld(1,10)
#define BF_DMACLK		Fld(1,9)
#define BF_DMA_GATE_DISABLE		Fld(1,8)
#define BF_TIMERCLK		Fld(1,7)
#define BF_PERICLK		Fld(1,6)
#define BF_GPIOCLK		Fld(1,5)
#define BF_CORP_GATE_DISABLE	Fld(1,4)
#define BF_MEMCCLK		Fld(1,3)
#define BF_MEMACLK		Fld(1,1)
#define BF_A940TCLK		Fld(1,0)
//----------------------------------------------------------------------------------------------	
	MMSP20_REG16	RESERVE1;	/* RESERVE1 C000 0906h */
//----------------------------------------------------------------------------------------------	
	MMSP20_REG16	COMCLKENREG; 	/* Communication Device Clock Enable Register, 08h */
/*#define SSPCLK		0x0200
#define I2CCLK		0x0100
#define UART3CLK	0x0080
#define UART2CLK	0x0040
#define UART1CLK	0x0020
#define UART0CLK	0x0010
#define OWMCLK		0x0008
#define IrDACLK		0x0004
#define UDCCLK		0x0002
#define UHCCLK		0x0001
*/
//----------------------------------------------------------------------------------------------	
	MMSP20_REG16	VGCLKENREG;	/* Video & Graphic Device Clock Enable Register, 0Ah, */
/*
#define DBLKCLK		0x8000
#define MPGIFCLK	0x4000
#define DECSSCLK	0x2000
#define MECLK		0x1000
#define REFWCLK		0x0800
#define RVLDCLK		0x0400
#define QGMCCLK		0x0200
#define MPMCCLK		0x0100
#define PDFCLK		0x0080
#define HUFFCLK		0x0040
#define VPCLKGOFF	0x0020
#define VPTOPCLK	0x0010
#define ISPCLK		0x0008
#define GRPCLK		0x0004
#define SCALECLK	0x0002
#define DISPCLK		0x0001
*/
//----------------------------------------------------------------------------------------------	
	MMSP20_REG16	ASCLKENREG;	/* Audio & Storage Device clock Enable Register, 0Ch */
#define CDROMCLK	0x2000
#define IDECLK		0x1000
#define MSTICKCLK	0x0200
#define MMCCLK		0x0100
#define SPDIFOCLK	0x0008
#define SPDIFICLK	0x0004
#define I2SCLK		0x0002
#define AC97CLK		0x0001

//-----------------------------------------------------------------------------------------------	
	MMSP20_REG16	RESERVE2;	/* RESERVE2 C000 090Eh */
//-----------------------------------------------------------------------------------------------	
	MMSP20_REG16	FPLLSETVREG;	/* FCLK PLL Setting Value Write Register, 10h */
#define BF_FMDIV	Fld(8,8)
#define FMDIV		FMsk(BF_FMDIV)
#define BF_FPDIV	Fld(6,2)
#define FPDIV		FMsk(BF_FPDIV)
#define BF_FSDIV	Fld(2,0)
#define FSDIV		FMsk(BF_FSDIV)
//-----------------------------------------------------------------------------------------------	
	MMSP20_REG16	FPLLVSETREG;	/* FCLK PLL Value Setting Register, 12h */
#define BF_FMDIVR	Fld(8,8)
#define FMDIVR		FMsk(BF_FMDIVR)
#define BF_FPDIVR	Fld(6,2)
#define FPDIVR		FMsk(BF_FPDIVR)
#define BF_FSDIVR	Fld(2,0)
#define FSDIVR		FMsk(BF_FSDIVR)
//-----------------------------------------------------------------------------------------------	
	MMSP20_REG16	UPLLSETVREG;	/* UCLK PLL Setting Value Write Register, 14h */
#define BF_UMDIV	Fld(8,8)
#define UMDIV		FMsk(BF_UMDIV)
#define BF_UPDIV	Fld(6,2)
#define UPDIV		FMsk(BF_UPDIV)
#define BF_USDIV	Fld(2,0)
#define USDIV		FMsk(BF_USDIV)
//-----------------------------------------------------------------------------------------------	
	MMSP20_REG16	UPLLVSETREG;	/* UCLK PLL Value Setting Register, 16h */
#define BF_UMDIVR	Fld(8,8)
#define UMDIVR		FMsk(BF_UMDIVR)
#define BF_UPDIVR	Fld(6,2)
#define UPDIVR		FMsk(BF_UPDIVR)
#define BF_USDIVR	Fld(2,0)
#define USDIVR		FMsk(BF_USDIVR)
//-----------------------------------------------------------------------------------------------	
	MMSP20_REG16	APLLSETVREG;	/* ACLK PLL Setting Value Write Register, 18h */
#define BF_AMDIV	Fld(8,8)
#define AMDIV		FMsk(BF_AMDIV)
#define BF_APDIV	Fld(6,2)
#define APDIV		FMsk(BF_APDIV)
#define BF_ASDIV	Fld(2,0)
#define ASDIV		FMsk(BF_ASDIV)
//-----------------------------------------------------------------------------------------------	
	MMSP20_REG16	APLLVSETREG;	/* ACLK PLL Value Setting Register, 1Ah */
#define BF_AMDIVR	Fld(8,8)
#define AMDIVR		FMsk(BF_AMDIVR)
#define BF_APDIVR	Fld(6,2)
#define APDIVR		FMsk(BF_APDIVR)
#define BF_ASDIVR	Fld(2,0)
#define ASDIVR		FMsk(BF_ASDIVR)
//-----------------------------------------------------------------------------------------------	
	MMSP20_REG16	SYSCSETREG;	/*System Clock PLL Divide Value Set Register */
#define BF_DCLKDIV	Fld(3,6)
#define DCLKDIV		FMsk(BF_DCLKDIV)
#define BF_A940TFDIV	Fld(3,3)
#define A940TFDIV	FMsk(BF_A940TFDIV)
#define BF_A920TFDIV	Fld(3,0)
#define A920TFDIV	FMsk(BF_A920TFDIV)
//-----------------------------------------------------------------------------------------------	
	MMSP20_REG16	ESYSCSETREG;	/* External System Clock Time Set Register */
#define BF_ESYSCLKSRC	Fld(2,14)
#define BF_ESYSDIV		Fld(6,8)
#define BF_ESYSPOL		Fld(1,7)
#define BF_ESYSDLY		Fld(3,4)
#define BF_SDRCLKPOL	Fld(1,3)
#define BF_SDRCLKDLY	Fld(3,0)
//-----------------------------------------------------------------------------------------------	
	MMSP20_REG16	UIRMCSETREG;	/* USB/IRDA/MMC Clock Generation U-PLL Divide Value Set Register */
#define BF_MMCCLKSRC	Fld(2,14)
#define BF_MMCCLKDIV	Fld(6,8)
#define BF_USBIRCLKSRC	Fld(2,6)
#define BF_USBIRDIV		Fld(6,0)
//-----------------------------------------------------------------------------------------------
	MMSP20_REG16	AUDICSETREG;	/* Audio Controller clock generation PLL Divide Value Set */
#define BF_AUDCLKSRC	Fld(2,6)
#define AUDCLKSRC		FMsk(BF_AUDCLKSRC)
#define BF_AUDCLKDIV	Fld(6,0)	
#define AUDCLKDIV		FMsk(BF_AUDCLKDIV)
//-----------------------------------------------------------------------------------------------
	MMSP20_REG16	DISPCSETREG;	/* Display Clock Generation PLL Divide Value Set Register */
#define BF_DISPCLKSRC	Fld(2,14)
#define BF_DISPCLKDIV	Fld(6,8)
#define BF_DISPCLKPOL	Fld(1,7)
//-----------------------------------------------------------------------------------------------
	MMSP20_REG16	IMAGCSETREG;	/* Image pixel clock generation PLL Divide value set register */
#define BF_IMGCLKSRC	Fld(2,14)
#define BF_IMGCLKDIV	Fld(6,8)
//-----------------------------------------------------------------------------------------------
	MMSP20_REG16	URTCSETREG[2];	/* UART0/1, 2/3  Tx/Rx clock generaiton PLL Divide value set register */
#define BF_URT1CLKSRC	Fld(2,14)
#define BF_URT1CLKDIV	Fld(6,8)
#define BF_URT0CLKSRC	Fld(2,6)
#define BF_URT0CLKDIV	Fld(6,0)
#define BF_URT3CLKSRC	Fld(2,14)
#define BF_URT3CLKDIV	Fld(6,8)
#define BF_URT2CLKSRC	Fld(2,6)
#define BF_URT2CLKDIV	Fld(6,0)
//-----------------------------------------------------------------------------------------------
/* power */
	MMSP20_REG16	ARM940TMODEREG; /* ARM940T CPU power manage mode register */
#define A940TIDLE		0x0001	
/* clock */
//-----------------------------------------------------------------------------------------------
	MMSP20_REG16	SPDICSETREG;	/* SPDIF Controller clock generation PLL Divide Value Set Register */
#define BF_SPDIFINSRC	Fld(2,14)
#define BF_SPDIFINDIV	Fld(6,8)
#define BF_SPDIFOUTSRC	Fld(2,6)
#define BF_SPDIFOUTDIV	Fld(6,0)
} MMSP20_CLOCK_POWER; /*C0000900h ~ C000092Ch*/

/* DMA Controller C000 0200h */
typedef struct {
	MMSP20_REG16	DMACOM0;	/* 0xc0000200, DMA Command register 0 */
#define SRCADDRINC	0x2000		// Source Address Increment Setting
								// 0:disable, 1:enable increament/decreament
#define FLOWSRC		0x1000		// 0:no wait, start the data transfer immediately
								// 1:wait for the request signal before initiating
#define BF_SRCMEMFMT	Fld(2,10)
#define SRCMEMFMT	FMsk(BF_SRCMEMFMT)	// Source Memory Data Format
#define BF_SRCPERIWD	Fld(2,8)
#define SRCPERIWD	FMsk(BF_SRCPERIWD)	// Source IO Device Data Width
#define TRGADDRINC	0x0020
#define FLOWTRG		0x0010
#define BF_TRGMEMFMT	Fld(2,2)
#define TRGMEMFMT	FMsk(BF_TRGMEMFMT)
#define BF_TRGPERIWD	Fld(2,0)
#define TRGPERIWD	FMsk(BF_TRGPERIWD)
//----------------------------------------------------------------------------------	
	MMSP20_REG16	DMACOM1;	/* 0xc0000202, DMA Command register 1 */
#define BF_SRCADDRSTEP	Fld(8,8)
#define SRCADDRSTEP	FMsk(BF_SRCADDRSTEP)
#define BF_TRGADDRSTEP	Fld(8,0)
#define TRGADDRSTEP	FMsk(BF_TRGADDRSTEP)	
//----------------------------------------------------------------------------------	
	MMSP20_REG16	DMACOM2;	/* 0xc0000204, DMA Command register 2 */
//----------------------------------------------------------------------------------	
	MMSP20_REG16	DMACONS;	/* 0xc0000206, DMA control/status register */
#define DMARUN		0x0400		// 1:DMA Start, 0:DMA End
#define ENDIRQEN	0x0200		// End Interrupt Enable	
#define FLYBYS		0x0010		// 0:Flow-through, 1:Fly-By for Memory Read Operation
#define FLYBYT		0x0008		// 0:Flow-through, 1:Fly-By for Memory Write Operation
#define REQPEND		0x0004		// 0:no pending request, 1:channel has a pending request
#define ENDINTR		0x0002		// Interrupt Status Register
//----------------------------------------------------------------------------------	
	MMSP20_REG16	SRCLADDR;	/* 0xc0000208, DMA source Low address register */
	MMSP20_REG16	SRCHADDR;	/* 0xc000020a, DMA source High address register */
	MMSP20_REG16	TRGLADDR;	/* 0xc000020c, DMA target Low address register */
	MMSP20_REG16	TRGHADDR;	/* 0xc000020e, DMA target High address register */
} MMSP20_DMAS; 

/*
typedef struct {
	struct MMSP20_DMA_1 CH[16];
} MMSP20_DMA;
*/

struct MMSP20_DCHSTRM{		/* Type DMA Chanel source/target i/o request map register */
	MMSP20_REG16	DCHSRM;		/* DMA Chanel Source I/o Request Map register */
	MMSP20_REG16	DCHTRM;		/* DMA Chanel Tareet i/O Request Map Register */
};

typedef struct {
/* Not Sequence C000 0100h */
	struct MMSP20_DCHSTRM 	DCHSTRM[16];	/* DMA Channel Source/Target i/o request map register 0~ 15 */
} MMSP20_DMAREQ;

#if 0
/* Interrupt control register C000 0800h */
typedef struct {
	MMSP20_REG32	SRCPND;		/* source pending register */
	MMSP20_REG32	INTMOD;		/* Interrupt mode register */
	MMSP20_REG32	INTMASK;	/* Interrupt Mask register */
	MMSP20_REG32	PRIORITY;	/* Interrupt priorit register */	
	MMSP20_REG32	INTPND;		/* Interrupt pendig register */
	MMSP20_REG8	INTOFFSET;	/* Interrupt offset register */
} MMSP20_INTERRUPT;

/* Timer & WatchDog */
typedef struct {
	MMSP20_REG32	TCOUNT;		/* Timer Counter register */
	MMSP20_REG32	TMATCH[4];	/* Timer match 0 ~ 3 register */
	MMSP20_REG16	TCONTROL;	/* Watchdog timer enable register */
	MMSP20_REG16	TSTATUS;	/* Timer status reigister */
	MMSP20_REG8	TINTEN;		/*Timer interrupt enable register */
} MMSP20_TIMER_WATCHDOG;
#endif

/* UART */
struct MMSP20_UART_REG {
	MMSP20_REG16	LCON;		/* UART LINE Control register */ 
	MMSP20_REG16	UCON;		/* UART control register */
	MMSP20_REG16	FCON;		/* UART Fifo control register */
	MMSP20_REG16	MCON;		/* UART Model control register */
	MMSP20_REG16	TRSTATUS;	/* Uart Tx/Rx Status register */
	MMSP20_REG16	ESTATUS;	/* UART Error status register */
	MMSP20_REG16	FSTATUS;	/* Uart Fifo status register */
	MMSP20_REG16	MSTATUS;	/* Uart modem status register */
	MMSP20_REG16	THB;		/* Uart Transmit buffer register 8bit */
	MMSP20_REG16	RHB;		/* Uart Receive buffer register 8bit */
	MMSP20_REG16	BRD;		/* Uart baud rate divisor register */
	MMSP20_REG16	TIMEOUTREG;	/* Uart receive timeout register */
	MMSP20_REG16	RESERVE[4];
};

typedef struct {
	struct MMSP20_UART_REG MMSP20_UART_NR[4]; /* UART REGISTER 0 ~ 3 */

	MMSP20_REG16	INTSTATREG;	/* Uart interrupt status register */
	MMSP20_REG16	PORTCON; 	/* Uart port confrol register */
} MMSP20_UART;

/* GPIO configration register */
typedef struct {
	MMSP20_REG16	GPIOALTFNLOW[15];	/* 0xc0001020~0xC000103c, GPIOx alternate function low regiter */
	MMSP20_REG16	RESERVE1;
	
	MMSP20_REG16	GPIOALTFNHI[15];	/* 0xc0001040~0xc000105c, GPIOx alternate function hi regiter */
	MMSP20_REG16	RESERVE2;

	MMSP20_REG16	GPIOOUT[15];		/* 0xc0001060~0xc000107c, GPIOx outpur level register */
	MMSP20_REG16	RESERVE3;

	MMSP20_REG16	GPIOEVTTYPLOW[15];	/* 0xc0001080~0xc000109c, GPIOx Event type low regiter */
	MMSP20_REG16	RESERVE4;

	MMSP20_REG16	GPIOEVTTYPHI[15];	/* 0xc00010a0~0xc00010bc, GPIOx Event type hi regiter */
	MMSP20_REG16	RESERVE5;	

	MMSP20_REG16	GPIOPUENB[15];		/* 0xc00010c0~0xc00010dc, GPIOx Pull Up Enable Register */
	MMSP20_REG16	RESERVE6;

	MMSP20_REG16	GPIOINTENB[15];		/* 0xc00010e0~0xc00010fc, GPIOx Interrupt Enable Register */
	MMSP20_REG16	RESERVE7;

	MMSP20_REG16	GPIOEVT[15];		/* 0xc0001100~0xc000111c, GPIOx Event Register */
	MMSP20_REG16	RESERVE8;

	MMSP20_REG16	GPIOPADSEL;		/* 0xc0001120, GPIOx PAD selection register*/
#define MPEG_IF_PAD		0x8		// MPEG Interface PAD Selection, 0:GPIOM[8:4], 1:GPION[7:3]
#define CDROM_IF_PAD	0x4		// CDROM Interface PAD Selection, 0:GPIOM[8:6], 1:GPION[5:3]
#define USB_PAD_3T		0x2		// USB PAD3 Type, 0:USB Host, 1:USB Device
#define USB_PAD_1T		0x1		// USB PAD1 Type, 0:USB Host, 1:USB Device

	MMSP20_REG16	RESERVE9[15];

	MMSP20_REG16	GPIOGREVTSTAT;		/* 0xc0001140, GPIO Group Event Regiter */
	MMSP20_REG16	RESERVE10[31];
 
	MMSP20_REG16	GPIOPINLVL[15];		/* 0xc0001180~0xc000119c, GPIOx Pin Level Register */
	MMSP20_REG16	RESERVE11;
 
	MMSP20_REG16	GPIOEOUT_ALIVE;		/* 0xc00011a0, GPIOE Output Alibe register */
	MMSP20_REG16	GPIOEOUTENB_ALIVE;	/* 0xc00011a2, GPIOE Output Enable Alibe register */
	MMSP20_REG16	GPIOEPUENB_ALIVE;	/* 0xc00011a4, GPIOE Pull up Enable Alibe register */

	MMSP20_REG16	GPIOFOUT_ALIVE;		/* 0xc00011a6, GPIOF Output Alibe register */
	MMSP20_REG16	GPIOFOUTENB_ALIVE;	/* 0xc00011a8, GPIOF Output Enable Alibe register */
	MMSP20_REG16	GPIOFPUENB_ALIVE;	/* 0xc00011aa, GPIOF Pull up Enable Alibe register */

	MMSP20_REG16	GPIOGOUT_ALIVE;		/* 0xc00011ac, GPIOG Output Alibe register */
	MMSP20_REG16	GPIOGOUTENB_ALIVE;	/* 0xc00011ae, GPIOG Output Enable Alibe register */
	MMSP20_REG16	GPIOGPUENB_ALIVE;	/* 0xc00011b0, GPIOG Pull up Enable Alibe register */

	MMSP20_REG16	GPIOIOUT_ALIVE;		/* 0xc00011b2, GPIOI Output Alibe register */
	MMSP20_REG16	GPIOIOUTENB_ALIVE;	/* 0xc00011b4, GPIOI Output Enable Alibe register */
	MMSP20_REG16	GPIOIPUENB_ALIVE;	/* 0xc00011b6, GPIOI Pull up Enable Alibe register */

	MMSP20_REG16	GPIOJOUT_ALIVE;		/* 0xc00011b8, GPIOJ Output Alibe register */
	MMSP20_REG16	GPIOJOUTENB_ALIVE;	/* 0xc00011ba, GPIOJ Output Enable Alibe register */
	MMSP20_REG16	GPIOJPUENB_ALIVE;	/* 0xc00011bc, GPIOJ Pull up Enable Alibe register */

} MMSP20_GPIO;


/* VIdeo Post processor NOT DEFINE EMPTY ADDRESS*/

typedef struct {
	MMSP20_REG16	FDC_CNTL; 		/* control register */
#define FDC_STOP        0x8000
#define FDC_FPC         0x4000                  // Field Polarity Control
#define BF_FDC_LC       Fld(2,12)
#define FDC_LC          FMsk(BF_FDC_LC)         // Lock control
  #define FDC_LC_0      FInsrt(0x0, BF_FDC_LC)  // No Lock Control
  #define FDC_LC_1      FInsrt(0x1, BF_FDC_LC)  // 4 words
  #define FDC_LC_2      FInsrt(0x2, BF_FDC_LC)  // 8 words
  #define FDC_LC_3      FInsrt(0x3, BF_FDC_LC)  // Lock Off
#define FDC_WRMOD       0x0800                  // 0: Ext. mem -> Ext. mem, 1: Ext. mem -> scaler/display
#define FDC_SBFLD       0x0400                  // Bottom field flag for Source Frame
#define FDC_DBFLD       0x0200                  // Bottom field flag for Destination Frame
#define FDC_SFF         0x0100                  // Source Frame Format, 0: frame, 1: field
#define FDC_DFF         0x0080                  // Destination Frame Format, 0: frame, 1:field
#define FDC_VSE         0x0040                  // Vertical Sync Enable
#define FDC_IsBIG       0x0020                  // Endian, 0:Little-Endian, 1:Big-Endian
#define BF_FDC_ROT      Fld(2,3)
#define FDC_ROT         FMsk(BF_FDC_ROT)
  #define FDC_ROT000    FInsrt(0x0, BF_FDC_ROT)
  #define FDC_ROT090    FInsrt(0x1, BF_FDC_ROT)
  #define FDC_ROT180    FInsrt(0x2, BF_FDC_ROT)
  #define FDC_ROT270    FInsrt(0x3, BF_FDC_ROT)
#define BF_FDC_CHROMA   Fld(2,1)
#define FDC_CHROMA      FMsk(BF_FDC_CHROMA)
  #define FDC_CHROMA420 FInsrt(0x0, BF_FDC_CHROMA)
  #define FDC_CHROMA422 FInsrt(0x1, BF_FDC_CHROMA)
  #define FDC_CHROMA444 FInsrt(0x2, BF_FDC_CHROMA)
#define FDC_START       0x0001          // FC Start/FC Status
//----------------------------------------------------------------------------------
	MMSP20_REG16	FDC_FRAME_SIZE;		/* Frame size */
#define BF_FDC_MBY      Fld(8,8)
#define FDC_MBY         FMsk(BF_FDC_MBY)        // Maximum Macro block number in Y direction
#define BF_FDC_MBX      Fld(8,0)
#define FDC_MBX         FMsk(BF_FDC_MBX)        // Maximum Macro block number in X direction
//----------------------------------------------------------------------------------
	MMSP20_REG16	FDC_LUMA_OFFSET;	/* Luminance Offset register */
#define BF_FDC_LUSO     Fld(8,8)
#define FDC_LUSO        FMsk(BF_FDC_LUSO)       // Screen offset
#define BF_FDC_LUYO     Fld(4,4)
#define FDC_LUYO        FMsk(BF_FDC_LUYO)       // Y offset
#define BF_FDC_LUXO     Fld(4,0)
#define FDC_LUXO        FMsk(BF_FDC_LUXO)       // X offset
//----------------------------------------------------------------------------------
	MMSP20_REG16	FDC_CB_OFFSET;		/* Cb Offset Register */
#define BF_FDC_CBSO     Fld(8,8)
#define FDC_CBSO        FMsk(BF_FDC_CBSO)       // Screen offset
#define BF_FDC_CBYO     Fld(4,4)                
#define FDC_CBYO        FMsk(BF_FDC_CBYO)       // Y offset
#define BF_FDC_CBXO     Fld(4,0)
#define FDC_CBXO        FMsk(BF_FDC_CBXO)       // X offset
//----------------------------------------------------------------------------------
	MMSP20_REG16	FDC_CR_OFFSET;		/* Cr Offset Register */
#define BF_FDC_CRSO     Fld(8,8)
#define FDC_CRSO        FMsk(BF_FDC_CRSO)       // Screen offset
#define BF_FDC_CRYO     Fld(4,4)
#define FDC_CRYO        FMsk(BF_FDC_CRYO)       // Y offset
#define BF_FDC_CRXO     Fld(4,0)
#define FDC_CRXO        FMsk(BF_FDC_CRXO)       // X offset
//----------------------------------------------------------------------------------
	MMSP20_REG16	FDC_DST_BASE_L;		/* Target start address */
	MMSP20_REG16	FDC_DST_BASE_H;		/* Target start address */
//----------------------------------------------------------------------------------
	MMSP20_REG16	FDC_STATUS;		/* status register */
#define FDC_BSTEN       0x8000          // Burst Enable (when WRMOD = 1)
#define FDC_REQDI       0x0080          // Request from Display
#define FDC_REQSC       0x0040          // Request from Scaler
#define FDC_BFLD        0x0020          // Bottom field
#define BF_FDC_BUSY     Fld(5,0)
#define FDC_BUSY        FMsk(BF_FDC_BUSY)       // FC status
//----------------------------------------------------------------------------------
	MMSP20_REG16	FDC_DERING;		/* Dering control register */
#define FDC_FOE         0x0002          // Filter Off Enable
#define FDC_DREN        0x0001          // Dering Enable
//----------------------------------------------------------------------------------
	MMSP20_REG16	FDC_OCC_CNTL;		/* Occurrence control register */
#define BF_FDC_OCCL     Fld(8,0)
#define FDC_OCCL        FMsk(BF_FDC_OCCL)       // Occurrence Control
//----------------------------------------------------------------------------------
} MMSP20_FDC;

/* Scale Process register */

typedef struct {
	MMSP20_REG16	SC_CMD;			/* 0xc0001800, scale control register */
#define SC_DERING       0x8000          //De-Ring Filter Enable in Scale Processor
#define SC_START        0x4000          // Start Signal
#define SC_ENABLE       0x2000          // Scale Processor Enable Signal
#define BF_SC_SRC      Fld(3,10)
#define SC_SRC         FMsk(BF_SC_SRC)        // Source of Scale Processor
  #define SC_SRC_MEM   FInsrt(0x0, BF_SC_SRC)         // from External memory
  #define SC_SRC_DISP  FInsrt(0x1, BF_SC_SRC)         // from Display Processor
  #define SC_SRC_ISP   FInsrt(0x2, BF_SC_SRC)         // from ISP
  #define SC_SRC_FMT   FInsrt(0x4, BF_SC_SRC)         // from FC
#define SC_ONE_TIME     0x0100          // the number of Scaling and buffering per Frame
#define SC_PRESCLE      0x0080          // Pre-Scaling Down Enable Signal, 0:disable, 1:enable
#define SC_POSTSCLE     0x0040          // Post-Scaling Enable Signal, 0:disable, 1:enable
#define SC_POSTVSCLE    0x0020          // Scale Down or Scale Up in the vertical direction, 0:scale down, 1:scale up
#define SC_POSTHSCLE    0x0010          // Scale Down or Scale Up in the horizontal direction, 0:scale down, 1:scale up
#define SC_F422         0x0008          // Separated Writing Format(Y/Cb/Cr)
#define BF_SC_DEST      Fld(2,1)
#define SC_DEST         FMsk(BF_SC_DEST)
  #define SC_DEST_MEM   FInsrt(0x2, BF_SC_DEST)
  #define SC_DEST_DISP  FInsrt(0x1, BF_SC_DEST)
#define SC_WR_TYPE      0x0001          // Write Data(Y/Cb/Cr) separately, 0:Data[31:0]={Cr,Y,Cb,Y}, 1:Separated Data(Y/Cb/Cr)
  #define SC_SEP_DATA     0x0001
//----------------------------------------------------------------------------------
	MMSP20_REG16	SC_STATUS;		/* 0xc0001802, Scale status rgister */
#define SC_FIELD        0x0400          // Scale Processor가 외부 메모리에 write할 경우, frame/field 선택, 0:Frame mode, 1:Field mode
#define SC_FIELD_POL    0x0200          // Field signal polarity, 0:memory, 1:inversion
#define SC_ISP_FIELD    0x0100          // ISP의 field status
#define SC_DISP_FIELD   0x0080          // MLC의 field status
#define SC_BUSY         0x0008          // Scaling Busy status, 0:idle, 1:busy
#define SC_DONE         0x0004          // Scaling Done status, 0:active, 1:done
//----------------------------------------------------------------------------------
	MMSP20_REG16	SC_MIRROR;		/* 0xc0001804, Scaling status rgister */
#define BF_SC_DATA_FMT  Fld(2,4)
#define SC_DATA_FMT     FMsk(BF_SC_DATA_FMT)    // Scale Source Data Format
  #define YUY2          FInsrt(0x0, BF_SC_DATA_FMT)     // DATA[31:0]={V0,Y1,U0,Y0}
  #define YVYU          FInsrt(0x1, BF_SC_DATA_FMT)     // DATA[31:0]={U0,Y1,V0,Y0}
  #define UYVY          FInsrt(0x2, BF_SC_DATA_FMT)     // DATA[31:0]={Y1,V0,Y0,U0}
  #define VYUY          FInsrt(0x3, BF_SC_DATA_FMT)     // DATA[31:0]={Y1,U0,Y0,V0}
#define SC_DST_VMRR     0x0008          // Vertical Mirror of Destination Data, 0:disable, 1:mirror
#define SC_DST_HMRR     0x0004          // Horizontal Mirror of Destination Data, 0:disable, 1:mirror
#define SC_SRC_VMRR     0x0002          // Vertical Mirror of Source Data, 0:disable, 1:mirror
#define SC_SRC_HMRR     0x0001          // Horizontal Mirror of Source Data, 0:disable, 1:mirror
//----------------------------------------------------------------------------------
	MMSP20_REG16	SC_PRE_VRATIO;		/* 0xc0001806, Pre-scale verical ratio */
	MMSP20_REG16	SC_PRE_HRATIO;		/* 0xc0001808, Pre-scale horizontal ratio */
	MMSP20_REG16	SC_POST_VRATIOL;	/* 0xc000180a, Vertical Post-Scaling ratio (Low) */
	MMSP20_REG16	SC_POST_VRATIOH;	/* 0xc000180c, Vertical Post-Scaling ratio (High) */
	MMSP20_REG16	SC_POST_HRATIOL;	/* 0xc000180e, Horizontal Post-Scaling ratio (Low) */
	MMSP20_REG16	SC_POST_HRATIOH;	/* 0xc0001810, Horizontal Post-Scaling ratio (High)*/
	MMSP20_REG16	SC_SRC_ODD_ADDRL;	/* 0xc0001812, Source Address Starting point of Odd Field (Low) */
	MMSP20_REG16	SC_SRC_ODD_ADDRH;	/* 0xc0001814, Source Address Starting point of Odd Field (High) */
	MMSP20_REG16	SC_SRC_EVEN_ADDRL;	/* 0xc0001816, Source Address Starting point of Even Field (Low) */	
	MMSP20_REG16	SC_SRC_EVEN_ADDRH;	/* 0xc0001818, Source Address Starting point of Even Field (High) */
	MMSP20_REG16	SC_DST_ADDRL;		/* 0xc000181a, Destination Address Starting point of Odd Field(Low) */
	MMSP20_REG16	SC_DST_ADDRH;		/* 0xc000181c, Destination Address Starting point of Odd Field(High) */
	MMSP20_REG16	SC_SRC_PXL_HEIGHT;	/* 0xc000181e, Vertical Source Pixel Height */
	MMSP20_REG16	SC_SRC_PXL_WIDTH;	/* 0xc0001820, Horizontal Source Pixel Width */
	MMSP20_REG16	SC_SRC_PXL_REQCNT;	/* 0xc0001822,When Pre Scale Mode, Horizontal Pixel width of 
						Prescale output
						When No pre-scale mode, Horizontal Source Pixel size*/
	MMSP20_REG16	SC_DST_PXL_HEIGHT;	/* 0xc0001824, Vertical Destination Pixel Height. */
	MMSP20_REG16	SC_DST_PXL_WIDTH;	/* 0xc0001826, Horizontal Destination Pixel Width. */
//---------------------------------------------------------------------------------        
//	MMSP20_REG16	SC_SEP_ADDR[3];		/* Two Dimensional Destination Address of 
//						Luminance and Chrominance */
  MMSP20_REG16     SC_LUMA_OFFSET;		/* 0xc0001828, Luminance Offset */
#define BF_SC_LUSO      Fld(8,8)
#define SC_LUSO         FMsk(BF_SC_LUSO)
#define BF_SC_LUYO      Fld(4,4)
#define SC_LUYO         FMsk(BF_SC_LUYO)
#define BF_SC_LUXO      Fld(4,0)
#define SC_LUXO         FMsk(BF_SC_LUXO)
//---------------------------------------------------------------------------------
  MMSP20_REG16     SC_CB_OFFSET;	/* 0xc000182a, Cb Offset */
#define BF_SC_CBSO      Fld(8,8)
#define SC_CBSO         FMsk(BF_SC_CBSO)
#define BF_SC_CBYO      Fld(4,4)
#define SC_CBYO         FMsk(BF_SC_CBYO)
#define BF_SC_CBXO      Fld(4,0)
#define SC_CBXO         FMsk(BF_SC_CBXO)
//---------------------------------------------------------------------------------
  MMSP20_REG16     SC_CR_OFFSET;	/* 0xc000182c, Cr Offset */
#define BF_SC_CRSO      Fld(8,8)
#define SC_CRSO         FMsk(BF_SC_CRSO)
#define BF_SC_CRYO      Fld(4,4)
#define SC_CRYO         FMsk(BF_SC_CRYO)
#define BF_SC_CRXO      Fld(4,0)
#define SC_CRXO         FMsk(BF_SC_CRXO)
//---------------------------------------------------------------------------------
	MMSP20_REG16	SC_DELAY;		/* 0xc000182e, Luminance and Chrominance path Delay */
#define BF_LUMA_DELAY   Fld(4,4)
#define LUMA_DELAY      FMsk(BF_LUMA_DELAY)
#define BF_CHROMA_DELAY Fld(4,0)
#define CHROMA_DELAY    FMsk(BF_CHROMA_DELAY)
//---------------------------------------------------------------------------------
	MMSP20_REG16	SC_MEM_CNTR;		/* 0xc0001830, Memory control */
#define SC_BRST_OFF     0x8000          // Memory Burst Mode Disable, 0:Burst Mode, 1:Non burst mode
#define BF_SC_OCCURANCE Fld(7,8)
#define SC_OCCURANCE    FMsk(BF_SC_OCCURANCE)   // data book에 없다.
#define BF_SC_MEM_RD_SZ Fld(4,0)
#define SC_MEM_RD_SZ    FMsk(BF_SC_MEM_RD_SZ)   // Memory Read Request Size by word, (8~16 word/req)
//---------------------------------------------------------------------------------
	MMSP20_REG16	SC_DST_WPXL_WIDTH;	/* 0xc0001832, Horizontal Wide Destination Pixel Width. */
//---------------------------------------------------------------------------------
	MMSP20_REG16	SC_IRQ;			/* 0xc0001834, Interrupt Control */
#define SC_IRQ_EN               0x0008          // Interrupt control of SC, 0:disable, 1:enable
#define SC_IRQ_CLEAR         0x0004             // Scale Interrupt Status and Clear bit, 0:no interrupt pending, 1:interrupt pending
#define FC_IRQ_EN               0x0002          // Interrupt control of FDC, 0:disable, 1:enable
#define FC_IRQ_CLEAR            0x0001          // FDC interrupt status and clear bits, 0:no interrupt pending, 1:interrupt pending
//---------------------------------------------------------------------------------
} MMSP20_SC;


/* VIDEO POST PROCESSOR: MULTI LAYER CONTROLLER */
typedef struct
{
    MMSP20_REG16  OVLAY_CNTR;        //0xC0002880    Overlay Control register    0x21
#define DISP_BOTH_PATH  0x8000          // overlay data path
#define DISP_OVLY2SCLE  0x4000          // overlay data scale
#define DISP_FLD_POS    0x2000          // Field mode, Display Sync. Generation Block, field signal
#define DISP_GAMM_BYPATH        0x1000  //RGB Gamma table bypath
#define DISP_SWAP       0x0800          // OSD Sub-picture
#define DISP_CURSOR     0x0200          // Cursor Enable/Disable
#define DISP_SUBPICTURE 0x0100          // Sub-Picture Enable/Disable
#define DISP_OSD        0x0080          // OSD Enable/Disable
#define DISP_STL5EN     0x0040          // Still Image Layer Region5 Enable/Disable
#define DISP_STL4EN     0x0020          // Still Image Layer Region4 Enable/Disable
#define DISP_STL3EN     0x0010          // Still Image Layer Region3 Enable/Disable
#define DISP_STL2EN     0x0008          // Still Image Layer Region2 Enable/Disable
#define DISP_STL1EN     0x0004          // Still Image Layer Region1 Enable/Disable
#define DISP_VLBON      0x0002          // YUV Layer Region B Enable/Disable
#define DISP_VLAON      0x0001          // YUV Layer Region A Enable/Disable
//---------------------------------------------------------------------------------------
    MMSP20_REG16  YUV_EFECT;         //0xC0002882    Video Image(Y/Cb/Cr Data) Effect 0x88
#define MLC_VLB_BT      0x0200          // YUV Layer Region B Top Bottom region
#define MLC_VLB_TP_MR_H 0x0100  // YUV Layer Top Region B를 Horizontal Mirror
#define MLC_VLB_TP_MR_V 0x0080  // YUV Layer Top Region B를 Vertical Mirror
#define MLC_VLB_BT_MR_H 0x0040  // YUV Layer Bottom Region B를 Horizontal Mirror
#define MLC_VLB_BT_MR_V 0x0020  // YUV Layer Bottom Region B를 Vertical Mirror
#define MLC_VLA_BT      0x0010  // YUV Layer Region A Top Bottom region�
#define MLC_VLA_TP_MR_H 0x0008  // YUV Layer Top Region A Horizontal Mirror
#define MLC_VLA_TP_MR_V 0x0004  // YUV Layer Top Region A Vertical Mirror
#define MLC_VLA_BT_MR_H 0x0002  // YUV Layer Bottom Region A Horizontal Mirror
#define MLC_VLA_BT_MR_V 0x0001  // YUV Layer Bottom Region A Vertial Mirror
//---------------------------------------------------------------------------------------
    MMSP20_REG16  YUV_CNTL;          //0xC0002884    Video Image(Y/Cb/Cr) Control register   0x00
#define MLC_VLA_ALP1SKP 0x2000  // YUV Layer Region Alpha blending skip
#define MLC_SC2DP_A     0x1000  // YUV Layer Region A path
// 0:Ext mem, 1:Scale Processor
#define MLC_FDC2DP_B    0x0800  // YUV Layer Region B path
// 0:Ext mem, 1:FDC
#define MLC_VL_PRIO     0x0400  // YUB Region A B Priority
// 0:Region A, 1:Region B
//---------------------------------------------------------------------------------------
    MMSP20_REG16  YUVA_TP_HSC;       //0xC0002886    
    MMSP20_REG16  YUVA_BT_HSC;       //0xC0002888    
    MMSP20_REG16  YUVA_TP_VSC_L;     //0xC000288a 
    MMSP20_REG16  YUVA_TP_VSC_H;     //0xC000288c
    MMSP20_REG16  YUVA_BT_VSC_L;     //0xC000288e
    MMSP20_REG16  YUVA_BT_VSC_H;     //0xC0002890
    MMSP20_REG16  YUVA_TP_PXW;       //0xC0002892  
    MMSP20_REG16  YUVA_BT_PXW;       //0xC0002894 
    MMSP20_REG16  YUVA_STX;          //0xC0002896 
    MMSP20_REG16  YUVA_ENDX;         //0xC0002898
    MMSP20_REG16  YUVA_TP_STY;       //0xC000289a
    MMSP20_REG16  YUVA_TP_ENDY;      //0xC000289c 
    MMSP20_REG16  YUVA_BT_ENDY;      //0xC000289e 
    MMSP20_REG16  YUVA_TP_OADR_L;    //0xC00028a0 
    MMSP20_REG16  YUVA_TP_OADR_H;    //0xC00028a2 
    MMSP20_REG16  YUVA_TP_EADR_L;    //0xC00028a4 
    MMSP20_REG16  YUVA_TP_EADR_H;    //0xC00028a6 
    MMSP20_REG16  YUVA_BT_OADR_L;    //0xC00028a8
    MMSP20_REG16  YUVA_BT_OADR_H;    //0xC00028aa
    MMSP20_REG16  YUVA_BT_EADR_L;    //0xC00028ac 
    MMSP20_REG16  YUVA_BT_EADR_H;    //0xC00028ae
    MMSP20_REG16  YUVB_TP_HSC;       //0xC00028b0
    MMSP20_REG16  YUVB_BT_HSC;       //0xC00028b2
    MMSP20_REG16  YUVB_TP_VSC_L;     //0xC00028b4
    MMSP20_REG16  YUVB_TP_VSC_H;     //0xC00028b6 
    MMSP20_REG16  YUVB_BT_VSC_L;     //0xC00028b8
    MMSP20_REG16  YUVB_BT_VSC_H;     //0xC00028ba
    MMSP20_REG16  YUVB_TP_PXW;       //0xC00028bc 
    MMSP20_REG16  YUVB_BT_PXW;       //0xC00028be   
    MMSP20_REG16  YUVB_STX;          //0xC00028c0  
    MMSP20_REG16  YUVB_ENDX;         //0xC00028c2  
    MMSP20_REG16  YUVB_TP_STY;       //0xC00028c4 
    MMSP20_REG16  YUVB_TP_ENDY;      //0xC00028c6 
    MMSP20_REG16  YUVB_BT_ENDY;      //0xC00028c8 
    MMSP20_REG16  YUVB_TP_OADR_L;    //0xC00028ca 
    MMSP20_REG16  YUVB_TP_OADR_H;    //0xC00028cc 
    MMSP20_REG16  YUVB_TP_EADR_L;    //0xC00028ce 
    MMSP20_REG16  YUVB_TP_EADR_H;    //0xC00028d0 
    MMSP20_REG16  YUVB_BT_OADR_L;    //0xC00028d2 
    MMSP20_REG16  YUVB_BT_OADR_H;    //0xC00028d4 
    MMSP20_REG16  YUVB_BT_EADR_L;    //0xC00028d6 
    MMSP20_REG16  YUVB_BT_EADR_H;    //0xC00028d8 
    MMSP20_REG16  RGB_CNTL;          //0xC00028da 
#define BF_MLC_STL_BPP  Fld(2,9)
#define MLC_STL_BPP     FMsk(BF_MLC_STL_BPP)    // RGB Layer Bpp
#define MLC_STL5ACT     0x0100          // Activate Region5 of STL
#define MLC_STL4ACT     0x0040          // Activate Region4 of STL
#define MLC_STL3ACT     0x0010          // Activate Region3 of STL
#define MLC_STL2ACT     0x0004          // Activate Region2 of STL
#define MLC_STL1ACT     0x0001          // Activate Region1 of STL
//--------------------------------------------------------------------------------------
    MMSP20_REG16  RGB_MIXMUX;        //0xC00028dc    Mix or Mux Control Register 0x00
#define BF_MLC_STL5_MIXMUX      Fld(2,8)
#define MLC_STL5_MIXMUX FMsk(BF_MLC_STL5_MIXMUX)        // 0:STL, 1:Color Key, 2:Alpha Blending
#define BF_MLC_STL4_MIXMUX      Fld(2,6)
#define MLC_STL4_MIXMUX FMsk(BF_MLC_STL4_MIXMUX)
#define BF_MLC_STL3_MIXMUX      Fld(2,4)
#define MLC_STL3_MIXMUX FMsk(BF_MLC_STL3_MIXMUX)
#define BF_MLC_STL2_MIXMUX      Fld(2,2)
#define MLC_STL2_MIXMUX FMsk(BF_MLC_STL2_MIXMUX)
#define BF_MLC_STL1_MIXMUX      Fld(2,0)
#define MLC_STL1_MIXMUX FMsk(BF_MLC_STL1_MIXMUX)
//--------------------------------------------------------------------------------------    
    MMSP20_REG16  RGB_ALPHA_L;       //0xC00028de    Alpha Value of Still Image  0x00
#define BF_MLC_STL3_ALPHA      Fld(4,8)
#define MLC_STL3_ALPHA  FMsk(BF_MLC_STL3_ALPHA)
#define BF_MLC_STL2_ALPHA       Fld(4,4)
#define MLC_STL2_ALPHA  FMsk(BF_MLC_STL2_ALPHA)
#define BF_MLC_STL1_ALPHA       Fld(4,0)
#define MLC_STL1_ALPHA  FMsk(BF_MLC_STL1_ALPHA)
//--------------------------------------------------------------------------------------
    MMSP20_REG16  RGB_ALPHA_H;       //0xC00028e0    Alpha Value of Still Image  0x00
#define BF_MLC_STL5_ALPHA       Fld(4,4)
#define MLC_STL5_ALPHA  FMsk(BF_MLC_STL5_ALPHA)
#define BF_MLC_STL4_ALPHA       Fld(4,0)
#define MLC_STL4_ALPHA  FMsk(BF_MLC_STL4_ALPHA)
//--------------------------------------------------------------------------------------
    MMSP20_REG16  RGB1_STX;          //0xC00028e2  
    MMSP20_REG16  RGB1_ENDX;         //0xC00028e4 
    MMSP20_REG16  RGB1_STY;          //0xC00028e6 
    MMSP20_REG16  RGB1_ENDY;         //0xC00028e8 
    MMSP20_REG16  RGB2_STX;          //0xC00028ea 
    MMSP20_REG16  RGB2_ENDX;         //0xC00028ec 
    MMSP20_REG16  RGB2_STY;          //0xC00028ee 
    MMSP20_REG16  RGB2_ENDY;         //0xC00028f0 
    MMSP20_REG16  RGB3_STX;          //0xC00028f2
    MMSP20_REG16  RGB3_ENDX;         //0xC00028f4
    MMSP20_REG16  RGB3_STY;          //0xC00028f6
    MMSP20_REG16  RGB3_ENDY;         //0xC00028f8
    MMSP20_REG16  RGB4_STX;          //0xC00028fa
    MMSP20_REG16  RGB4_ENDX;         //0xC00028fc
    MMSP20_REG16  RGB4_STY;          //0xC00028fe
    MMSP20_REG16  RGB4_ENDY;         //0xC0002900
//--------------------------------------------------------------------------------------
    MMSP20_REG16  RGB_CKEY_GB;       //0xC0002902  
#define BF_MLC_STL_CKEYG        Fld(8,8)
#define MLC_STL_CKEYG   FMsk(BF_MLC_STL_CKEYG)
#define BF_MLC_STL_CKEYB        Fld(8,0)
#define MLC_STL_CKEYB   FMsk(BF_MLC_STL_CKEYB)
//--------------------------------------------------------------------------------------
    MMSP20_REG16  RGB_CKEY_R;        //0xC0002904 
    MMSP20_REG16  RGB_HSC;           //0xC0002906 
    MMSP20_REG16  RGB_VSC_L;         //0xC0002908 
    MMSP20_REG16  RGB_VSC_H;         //0xC000290a 
    MMSP20_REG16  RGB_HW;            //0xC000290c 
    MMSP20_REG16  RGB_OADR_L;        //0xC000290e 
    MMSP20_REG16  RGB_OADR_H;        //0xC0002910 
    MMSP20_REG16  RGB_EADR_L;        //0xC0002912 
    MMSP20_REG16  RGB_EADR_H;        //0xC0002914 
    MMSP20_REG16  OSD_OADR_L;        //0xC0002916 
    MMSP20_REG16  OSD_OADR_H;        //0xC0002918 
    MMSP20_REG16  OSD_EADR_L;        //0xC000291a 
    MMSP20_REG16  OSD_EADR_H;        //0xC000291c 
//--------------------------------------------------------------------------------------
    MMSP20_REG16  HWC_CNTR;          //0xC000291e
#define BF_MLC_HWC_FALPH        Fld(4,12)
#define MLC_HWC_FALPH   FMsk(BF_MLC_HWC_FALPH)
#define BF_MLC_HWC_BALPH        Fld(4,8)
#define MLC_HWC_BALPH   FMsk(BF_MLC_HWC_BALPH)
#define BF_MLC_HWC_SZ   Fld(7,0)
#define MLC_HWC_SZ      FMsk(BF_MLC_HWC_SZ)
//--------------------------------------------------------------------------------------    
    MMSP20_REG16  HWC_STX;           //0xC0002920 
    MMSP20_REG16  HWC_STY;           //0xC0002922 
//--------------------------------------------------------------------------------------
    MMSP20_REG16  HWC_FGR;           //0xC0002924 
#define BF_MLC_HWC_FG   Fld(8,8)
#define MLC_HWC_FG      FMsk(BF_MLC_HWC_FG)
#define BF_MLC_HWC_FR   Fld(8,0)
#define MLC_HWC_FR      FMsk(BF_MLC_HWC_FR)
//--------------------------------------------------------------------------------------
    MMSP20_REG16  HWC_FB;            //0xC0002926   
//--------------------------------------------------------------------------------------    
    MMSP20_REG16  HWC_BGR;           //0xC0002928  
#define BF_MLC_HWC_BG   Fld(8,8)
#define MLC_HWC_BG      FMsk(BF_MLC_HWC_BG)
#define BF_MLC_HWC_BR   Fld(8,0)
#define MLC_HWC_BR      FMsk(BF_MLC_HWC_BR)
//--------------------------------------------------------------------------------------
    MMSP20_REG16  HWC_BB;            //0xC000292a  
    MMSP20_REG16  HWC_OADR_L;        //0xC000292c  
    MMSP20_REG16  HWC_OADR_H;        //0xC000292e  
    MMSP20_REG16  HWC_EADR_L;        //0xC0002930  
    MMSP20_REG16  HWC_EADR_H;        //0xC0002932  
//--------------------------------------------------------------------------------------
    MMSP20_REG16  LUMA_ENH;          //0xC0002934 
#define BF_MLC_CNTRST   Fld(3,8)
#define MLC_CNTRST      FMsk(BF_MLC_CNTRST)
#define BF_MLC_BRIGHT   Fld(8,0)
#define MLC_BRIGHT      FMsk(BF_MLC_BRIGHT)
//--------------------------------------------------------------------------------------
    MMSP20_REG16  HUECB1AB;          //0xC0002936 
#define BF_MLC_HUECB1A  Fld(8,8)
#define MLC_HUECB1A     FMsk(BF_MLC_HUECB1A)
#define BF_MLC_HUECB1B  Fld(8,0)
#define MLC_HUECB1B     FMsk(BF_MLC_HUECB1B)
//--------------------------------------------------------------------------------------
    MMSP20_REG16  HUECR1AB;          //0xC0002938 
#define BF_MLC_HUECR1A  Fld(8,8)
#define MLC_HUECR1A     FMsk(BF_MLC_HUECR1A)
#define BF_MLC_HUECR1B  Fld(8,0)
#define MLC_HUECR1B     FMsk(BF_MLC_HUECR1B)
//--------------------------------------------------------------------------------------
    MMSP20_REG16  HUECB2AB;          //0xC000293a 
#define BF_MLC_HUECB2A  Fld(8,8)
#define MLC_HUECB2A     FMsk(BF_MLC_HUECB2A)
#define BF_MLC_HUECB2B  Fld(8,0)
#define MLC_HUECB2B     FMsk(BF_MLC_HUECB2B)
//--------------------------------------------------------------------------------------    
    MMSP20_REG16  HUECR2AB;          //0xC000293c 
#define BF_MLC_HUECR2A  Fld(8,8)
#define MLC_HUECR2A     FMsk(BF_MLC_HUECR2A)
#define BF_MLC_HUECR2B  Fld(8,0)
#define MLC_HUECR2B     FMsk(BF_MLC_HUECR2B)
//--------------------------------------------------------------------------------------    
    MMSP20_REG16  HUECB3AB;          //0xC000293e 
#define BF_MLC_HUECB3A  Fld(8,8)
#define MLC_HUECB3A     FMsk(BF_MLC_HUECB3A)
#define BF_MLC_HUECB3B  Fld(8,0)
#define MLC_HUECB3B     FMsk(BF_MLC_HUECB3B)
//-------------------------------------------------------------------------------------    
    MMSP20_REG16  HUECR3AB;          //0xC0002940 
#define BF_MLC_HUECR3A  Fld(8,8)
#define MLC_HUECR3A     FMsk(BF_MLC_HUECR3A)
#define BF_MLC_HUECR3B  Fld(8,0)
#define MLC_HUECR3B     FMsk(BF_MLC_HUECR3B)
//-------------------------------------------------------------------------------------    
    MMSP20_REG16  HUECB4AB;          //0xC0002942 
#define BF_MLC_HUECB4A  Fld(8,8)
#define MLC_HUECB4A     FMsk(BF_MLC_HUECB4A)
#define BF_MLC_HUECB4B  Fld(8,0)
#define MLC_HUECB4B     FMsk(BF_MLC_HUECB4B)
//-------------------------------------------------------------------------------------    
    MMSP20_REG16  HUECR4AB;          //0xC0002944 
#define BF_MLC_HUECR4A  Fld(8,8)
#define MLC_HUECR4A     FMsk(BF_MLC_HUECR4A)
#define BF_MLC_HUECR4B  Fld(8,0)
#define MLC_HUECR4B     FMsk(BF_MLC_HUECR4B)
//-------------------------------------------------------------------------------------
    MMSP20_REG16  DITHER;            //0xC0002946 
#define MLC_DITHER_ON   0x0002  // Dithering skip enable. (0:dither ON, 1:dither OFF)
#define MLC_DITHER_TY   0x0001  // Dithering Type (0:2x2, 1:4x4)
//-------------------------------------------------------------------------------------    
    MMSP20_REG16  Reserved2948;      //0xC0002948        
    MMSP20_REG16  Reserved294a;      //0xC000294a    
    MMSP20_REG16  Reserved294c;      //0xC000294c
    MMSP20_REG16  Reserved294e;      //0xC000294e
    MMSP20_REG16  Reserved2950;      //0xC0002950
    MMSP20_REG16  Reserved2952;      //0xC0002952    
//-------------------------------------------------------------------------------------    
    MMSP20_REG16  OSD_PALLT_A;       //0xC0002954 
    MMSP20_REG16  OSD_PALLT_D;       //0xC0002956 
    MMSP20_REG16  RGB_PALLT_A;       //0xC0002958 
    MMSP20_REG16  RGB_PALLT_D;       //0xC000295a 
    MMSP20_REG16  GAMMA_A;           //0xC000295c 
    MMSP20_REG16  GAMMA_D;           //0xC000295e
//-------------------------------------------------------------------------------------
    MMSP20_REG16  Reserved2960;      //0xC0002960
    MMSP20_REG16  Reserved2962;      //0xC0002962
    MMSP20_REG16  Reserved2964;      //0xC0002964
    MMSP20_REG16  Reserved2966;      //0xC0002966
    MMSP20_REG16  Reserved2968;      //0xC0002968
    MMSP20_REG16  Reserved296a;      //0xC000296a
    MMSP20_REG16  Reserved296c;      //0xC000296c
    MMSP20_REG16  Reserved296e;      //0xC000296e
    MMSP20_REG16  Reserved2970;      //0xC0002970
    MMSP20_REG16  Reserved2972;      //0xC0002972
//-------------------------------------------------------------------------------------    
    MMSP20_REG16  SPU_CTRL;          //0xc0002974
#define MLC_SPU_HL_ON   0x0004  // Highlight mode on/off
#define MLC_SPU_DISP_MODE       0x0002  // Frame/Field display mode selection
#define MLC_SPU_ON      0x0001  // SPU block enable/disable bit
//-------------------------------------------------------------------------------------
    MMSP20_REG16  SPU_START_DELAY;   //0xc0002976
    MMSP20_REG16  SPU_BASE_ADDRL;    //0xc0002978
    MMSP20_REG16  SPU_BASE_ADDRH;    //0xc000297a
    MMSP20_REG16  SPU_ALT_ADDRL;     //0xc000297c
    MMSP20_REG16  SPU_ALT_ADDRH;     //0xc000297e
    MMSP20_REG16  SPU_PALLET[32];    //0xc0002980    
} MMSP20_MLC;


/* MMSP2 Display Controller ================================================ */
/*
    ------------------------------------------------
	 DatOutType      0       1       2       3
    ------------------------------------------------
     RGB             4:4:4   5:6:5   6:6:6   8:8:8
     MRGB            5:5:5   5:6:5   8:8:8a  8:8:8b
     YCbCr           CCIR656 CCIR601 3x8Mode 3x10Mode
    ------------------------------------------------
*/
typedef struct {
	MMSP20_REG16 DPC_CNTL;		// 0xc0002800, DPC Control 
#define SLVHV		0x8000		// Slave Sync Mode
#define SLAVE		0x4000		// Slave Mode 0:Master Mode, 1:Slave Mode
#define CSYNC		0x2000		// COMP SYNC Mode
#define PAL		0x1000		// 0:NTSC, 1:PAL mode
#define	CISCYNC		0x0800		// CIS Sync Mode
#define HDTV		0x0400		// TV Type 0:SDTV, 1:HDTV
#define ENC		0x0200		// External Device Mode 0:DAC, 1:TV Encoder
#define TVMODE		0x0100		// 0:LCD, 1:TV
#define BF_DOT		Fld(2,6)	
#define DOT		FMsk(BF_DOT)	// Data Output Type
  #define DOT_RGB	FInsrt(0x0, BF_DOT)	// 0:RGB
  #define DOT_MRGB	FInsrt(0x1, BF_DOT)	// 1:Mutiplexed RGB
  #define DOT_YCBCR	FInsrt(0x2, BF_DOT)	// 2:YCbCr
#define INTERLACE	0x0020		// TV Interace Mode 0:Progressive, 1:interlace
#define SYNCCBCR	0x0010		// Add sync level for Cb and Cr in DAC mode
#define ESAVEN		0x0008		// EAV/SAV Insertion Enable
#define BF_DOF		Fld(2,1)
#define DOF		FMsk(BF_DOF)	// Data Output Format
  #define DOF_0		FInsrt(0x0, BF_DOF)	// RGB(4:4:4), MRGB(5:5:5), YCbCr(CCIR656)
  #define DOF_1		FInsrt(0x1, BF_DOF)	// RGB(5:6:5), MRGB(5:6:5), YCbCr(CCIR601)
  #define DOF_2		FInsrt(0x2, BF_DOF)	// RGB(6:6:6), MRGB(888a), YCbCr(3x8 Mode)
  #define DOF_3		FInsrt(0x3, BF_DOF)	// RGB(8:8:8), MRGB(888b), YCbCr(3x10Mode)
#define ENB		0x0001		// LCDC Enable
//---------------------------------------------------------------------------
	MMSP20_REG16 DPC_FPICNTL;	// 0xc0002802, FPI Control
#define BF_CHPHASE	Fld(3,13) 
#define CHPHASE		FMsk(BF_CHPHASE)	// Select the phase for CLKH
#define CHMSKBNK	0x1000		// Mask pixel clock during blank area
#define PADSYNC		0x0004		// Sync Enable for ALL PAD Output 
					// 0:bypass, 1:1 clock delay
#define HSMSKBNK	0x0002		// Mask HSYNC during the vertical blank
#define PADHZ		0x0001		// Disable all PADs for LCDC
//---------------------------------------------------------------------------
	MMSP20_REG16 DPC_FPIPOL1;	// 0xc0002804, FPI Polarity Control 1
#define FLDPOL		0x0200		// Polarity Control for field signal
#define POLPOL		0x0100		// Polarity Control for POL/o_Y[0]
#define XDOFFPOL	0x0080		// Polarity Control for XDOFF_BLANK/o_Y[1]
#define PSPOL		0x0040		// Polarity Control for PS/o_Cb[0]
#define CVPOL		0x0020		// Polarity Control for CLKV/o_Cb[1]
#define CHPOL		0x0010		// Polarity Control for CLKH 
#define FGPOL		0x0008		// Polarity Control for FG_SYNC/o_Cr[0]
#define DEPOL		0x0004		// Polarity Control for DE/o_Cr[1]
#define HSPOL		0x0002		// Polarity Control for HSYNC
#define VSPOL		0x0001		// Polarity Control for VSYNC
//---------------------------------------------------------------------------
	MMSP20_REG16 DPC_FPIPOL2;	// 0xc0002806, FPI Polarity Control 2
#define BF_GREENPOL	Fld(8,16)
#define GREENPOL	FMsk(BF_GREENPOL)	// Polarity control Mask for green
#define BF_BLUEPOL	Fld(8,0)
#define BLUEPOL		FMsk(BF_BLUEPOL)	// Polarity control Mask for blue
//---------------------------------------------------------------------------
	MMSP20_REG16 DPC_FPIPOL3;	// 0xc0002808, FPI Polarity Control 3
#define BF_REDPOL	Fld(8,0)
#define REDPOL		FMsk(BF_REDPOL)		// Polarity control Mask for red
//---------------------------------------------------------------------------
#define DATA_FPIPOL2	0xFFFF
#define DATA_FPIPOL3	0xFF
//---------------------------------------------------------------------------
	MMSP20_REG16 DPC_FPIATV1;	// 0xc000280a, FPI Enable Control 1
#define FLDATV		0x0200		// Enable control for field
#define POLATV		0x0100		// Enable control for POL/o_Y[0]
#define XDOFFATV	0x0080		// Enable control for XDOFF_BLANK/o_Y[1]
#define PSATV		0x0040		// Enable control for PS/o_Cb[0]
#define CVATV		0x0020		// Enable control for CLKV/o_Cb[1]
#define CHATV		0x0010		// Enable control for CLKH
#define FGATV		0x0008		// Enable control for FG_SYNCT/o_Cr[0]
#define DEATV		0x0004		// Enable control for DE/o_Cr[1]
#define HSATV		0x0002		// Enable control for HSYNC
#define VSATV		0x0001		// Enable control for VSYNC
//---------------------------------------------------------------------------
	MMSP20_REG16 DPC_FPIATV2;	// 0xc000280c, FPI Enable Control 2
#define BF_VD1ATV	Fld(16,0)
#define VD1ATV		FMsk(BF_VD1ATV)		// Enable control for VD[15:0]
//---------------------------------------------------------------------------
	MMSP20_REG16 DPC_FPIATV3;	// 0xc000280e, FPI Enable Control 3
#define BF_VD2ATV	Fld(8,0)
#define VD2ATV		FMsk(BF_VD2ATV)		// Enable control for VD[23:16]
//---------------------------------------------------------------------------
	MMSP20_REG16 DPC_RSV100;	// 0xc0002810, reserved
	MMSP20_REG16 DPC_RSV101;	// 0xc0002812, reserved
	MMSP20_REG16 DPC_RSV102;	// 0xc0002814, reserved
	MMSP20_REG16 DPC_X_MAX;		// 0xc0002816, Active Width
	MMSP20_REG16 DPC_Y_MAX;		// 0xc0002818, Active Height
//---------------------------------------------------------------------------
	MMSP20_REG16 DPC_HS_WIDTH;	// 0xc000281a, HSYNC Width
#define SFTDIR		0x0400		// Shift direction control for pixel data (0:Right,1:Left)
#define BF_HSPADCFG	Fld(2,8)
#define HSPADCFG	FMsk(BF_HSPADCFG)	// HSYNC PAD Configuration
  #define HSPIN_HSYNC	FInsrt(0x0, BF_HSPADCFG)	// HSYNC pin to HSYNC
  #define HSPIN_LP	FInsrt(0x1, BF_HSPADCFG)	// HSYNC pin to Latch Pulse
  #define HSPIN_DE	FInsrt(0x2, BF_HSPADCFG)	// HSYNC pin to DE
  #define HSPIN_CSYNC	FInsrt(0x3, BF_HSPADCFG)	// HSYNC pin to CSYNC
#define BF_HSWID	Fld(8,0)
#define HSWID		FMsk(BF_HSWID)		// HSYNC Width
//---------------------------------------------------------------------------
	MMSP20_REG16 DPC_HS_STR;	// 0xc000281c, HSYNC Start
#define BF_HSSTR	Fld(8,0)
#define HSSTR		FMsk(BF_HSSTR)	// HSYNC Start position
//---------------------------------------------------------------------------
	MMSP20_REG16 DPC_HS_END;	// 0xc000281e, HSYNC End
#define BF_HSEND	Fld(8,0)
#define HSEND		FMsk(BF_HSEND)	// HSYNC End position
//---------------------------------------------------------------------------
	MMSP20_REG16 DPC_V_SYNC;	// 0xc0002820, VSYNC Control
#define BF_VSSTR	Fld(8,8)
#define VSSTR		FMsk(BF_VSSTR)	// VSYNC Start position
#define VSFLDEN		0x0080		// VSYNC Field Enable
#define VSFLDPOL	0x0040		// VSYNC Field polarity
#define BF_VSPADCFG	Fld(2,4)
#define VSPADCFG	FMsk(BF_VSPADCFG)	// VSYNC PAD Configuration
  #define VSPIN_VSYNC	FInsrt(0x0, BF_VSPADCFG)	// FVSYNC pin to VSYNC
  #define VSPIN_STV	FInsrt(0x1, BF_VSPADCFG)	// FVSYNC pin to the start of first line
  #define VSPIN_FIELD	FInsrt(0x2, BF_VSPADCFG)	// FVSYNC pin to the field
#define BF_VSWID	Fld(4,0)
#define VSWID		FMsk(BF_VSWID)	// VSYNC Width
//---------------------------------------------------------------------------
	MMSP20_REG16 DPC_V_END;		// 0xc0002822, VSYNC End
#define NOSYMBNKFLD	0x8000		// Select field has T9-1 Bank lines
#define NOSYMBNKLINE	0x4000		// Each field has not same lines for blank lines
#define NOSYMATVFLD	0x2000		// Select field has yMax-1 active lines.
#define NOSYMATVLINE	0x1000		// Each field has not same lines for active lines
#define SCANDIR		0x0800		// Scan direction control for line data (0:Down, 1:Up)
#define POLSTRLVL	0x0400		// Toggle start level of POL per frame
#define SMPTE274M	0x0200		// SMPTE274M interface flag
#define BF_VSEND	Fld(9,0)
#define VSEND		FMsk(BF_VSEND)	// VSYNC End position
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_TV_BNK;	// 0xc0002824, TV Blank End
#define BF_TVBNKSTR	Fld(9,7)
#define TVBNKSTR	FMsk(BF_TVBNKSTR)	// Blank Start position (Valid when TV mode is enabled)
#define BF_TVBNKEND	Fld(7,0)
#define TVBNKEND	FMsk(BF_TVBNKEND)	// Blank End position (Valid when TV mode is enabled)
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_DE;		// 0xc0002826, DE Control
#define BF_DEPADCFG	Fld(2,14)
#define DEPADCFG	FMsk(BF_DEPADCFG)	// DE PAD Configuration
  #define DEPIN_DE	FInsrt(0x0, BF_DEPADCFG)
  #define DEPIN_STH	FInsrt(0x1, BF_DEPADCFG)
  #define DEPIN_CSYNC	FInsrt(0x2, BF_DEPADCFG)
#define DEBNKEN		0x2000		// Enable DE during blank area
#define DEMOD		0x1000		// DE mode (0:Enable Mode,1:Pulse Mode)
#define BF_DESTR	Fld(8,4)
#define DESTR		FMsk(BF_DESTR)	// DE Start position
#define BF_DEWID	Fld(4,0)
#define DEWID		FMsk(BF_DEWID)	// STH Pulse Width(when DEMode=1 and DESel=0)
					// DE Delay(when DEMode=0 and DESel=0)
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_PS;		// 0xc0002828, PS Control
#define BF_PSSTR	Fld(4,12)
#define PSSTR		FMsk(BF_PSSTR)	// PS Start position
#define BF_PSEND	Fld(12,0)
#define PSEND		FMsk(BF_PSEND)	// PS End position
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_FG;		// 0xc000282a, FG Control
#define PSEN		0x8000		// Power Saving Enable
#define BF_XDFPLSWID	Fld(4,8)
#define XDFPLSWID	FMsk(BF_XDFPLSWID)	// XDOFF pulse width
#define BF_FGWID	Fld(4,0)
#define FGWID		FMsk(BF_FGWID)	// FG Width
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_LP;		// 0xc000282c, LP Control
#define LPEN		0x2000		// LP Enable
#define LPACTIVE	0x1000		// Disable the LP during blank area
#define BF_LPSTR	Fld(4,8)
#define LPSTR		FMsk(BF_LPSTR)	// LP start position
#define BF_LPWID	Fld(8,0)
#define LPWID		FMsk(BF_LPWID)	// LP Width
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_CLKVH;		// 0xc000282e, CLKV Control
#define BF_CVDLY	Fld(3,13)
#define CVDLY		FMsk(BF_CVDLY)	// CLKV Delay
#define CVATVAREA	0x1000		// CLKV Active area
#define BF_CVRISPOS	Fld(12,0)
#define CVRISPOS	FMsk(BF_CVRISPOS)	// CLKV risig edge position
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_CLKVL;		// 0xc0002830, CLKV Control 2
#define POLTGLEN	0x8000		// Vertical Toggling of POL
#define CVEN		0x2000		// CLKV Enable
#define CVMSKBNK	0x1000		// Disable CLKV during vertical blank time
#define BF_CVFALPOS	Fld(12,0)
#define CVFALPOS	FMsk(BF_CVFALPOS)	// CLKV falling edge position
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_POL;		// 0xc0002832, POL Control
#define POLEN		0x8000		// POL Enable
#define BF_POLPERIOD	Fld(2,13)
#define POLPERIOD	FMsk(BF_POLPERIOD)	// POL alternating period (1~4HD)
#define POLTGLAREA	0x1000		// POL transition zone
#define BF_POLTGLPOS	Fld(12,0)
#define POLTGLPOS	FMsk(BF_POLTGLPOS)	// POL toggle position
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_CISSYNC;	// 0xc0002834, CIS Sync Control
#define BF_CISDLY	Fld(10,0)
#define CISDLY		FMsk(BF_CISDLY)	// CIS Sync Delay (1~1024 clocks)
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_MID_SYNC;	// 0xc0002836, CSYNC Control
#define BF_MIDSYNCPOS	Fld(12,0)
#define MIDSYNCPOS	FMsk(BF_MIDSYNCPOS)	// MID Sync start position
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_C_SYNC;	// 0xc0002838, CSYNC Control
#define BF_CSYNCPOS	Fld(12,0)
#define CSYNCPOS	FMsk(BF_CSYNCPOS)	// C Sync start position
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_Y_BLANK;	// 0xc000283a, Blank Level for Luminance
#define BF_YBNKLVL	Fld(10,0)
#define YBNKLVL		FMsk(BF_YBNKLVL)	// blank for Y
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_C_BLANK;	// 0xc000283c, Blank Level for Chrominance
#define BF_CBNKLVL	Fld(10,0)
#define CBNKLVL		FMsk(BF_CBNKLVL)	// blank for Cb or Cr
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_YP_CSYNC;	// 0xc000283e, Positive Sync Level for Luminance
#define BF_YPOSSYNCLVL	Fld(10,0)
#define YPOSSYNCLVL	FMsk(BF_YPOSSYNCLVL)	// Positive Sync level for the luminance
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_YN_CSYNC;	// 0xc0002840, Negative Sync Level for Luminance
#define BF_YNEGSYNCLVL	Fld(10,0)
#define YNEGSYNCLVL	FMsk(BF_YNEGSYNCLVL)	// Negative Sync level for the luminance
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_CP_CSYNC;	// 0xc0002842, Positive Sync Level for Chrominance
#define BF_CPOSSYNCLVL	Fld(10,0)
#define CPOSSYNCLVL	FMsk(BF_CPOSSYNCLVL)	// Positive Sync level for the chrominance
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_CN_CSYNC;	// 0xc0002844, Negative Sync Level for Chrominance
#define BF_CNEGSYNCLVL	Fld(10,0)
#define CNEGSYNCLVL	FMsk(BF_CNEGSYNCLVL)	// Negative Sync level for the chrominance
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_INTR;		// 0xc0002846, Interrupt Controller
#define VSPLSSEL	0x0200		// Output selection for ver_sync
#define VSINTEN		0x0020		// VSYNC Interrupt Enable
#define HSINTEN		0x0010		// HSYNC Interrupt Enable
#define VSINTEDGE	0x0008		// Interrupt position for VSYNC(0:first edge, 1:second edge)
#define HSINTEDGE	0x0004		// Interrupt position for HSYNC
#define VSINTFLAG	0x0002		// VSYNC Interrupt Status
#define HSINTFLAG	0x0001		// HSYNC Interrupt Status
//--------------------------------------------------------------------------
	MMSP20_REG16 DPC_CLKCNTL;	// 0xc0002848, Clock Controller 
#define _656DATOUTEN	0x0040		// 656 Data Out Enable
#define _656CLKOUTEN	0x0020		// 656 CLK Out Enable
#define BF_CLKSRC	Fld(2,3)
#define CLKSRC		FMsk(BF_CLKSRC)	// Display Source Clock
  #define CLKSRC_EXT	FInsrt(0x0, BF_CLKSRC)	// External Video Clock Input(VCLKIN)
  #define CLKSRC_PLL	FInsrt(0x1, BF_CLKSRC)	// PLL
  #define CLKSRC_ISP	FInsrt(0x2, BF_CLKSRC)	// ISP clock
#define CLK2SEL		0x0004		// DClk2 Selection(0:1DCLK,1:1/2DCLK)
#define CLK1SEL		0x0002		// DClk1 Selection(0:1DCLK,1:1/2DCLK)
#define CLK1POL		0x0001		// DClk1 polarity control(0:Normal,1:Inversion)
//--------------------------------------------------------------------------
} MMSP20_DPC;

/* ISP */
typedef struct {
	MMSP20_REG16	CON_1;			//0xc0003000		Image Signal Processor control register	0x00
#define ISP_MEM_DON		(1 << 15)		// Memory Write Mode
#define BF_ISP_DT_SEL	Fld(2,13)
#define ISP_DT_SEL		FMsk(BF_ISP_DT_SEL)		//ISP Data/Clock Selection
  #define ISP_DT_SEL_0	FInsrt(0x0, BF_ISP_DT_SEL)	// Data = GPIOC[15:0], Clock = GPIOH[4]
  #define ISP_DT_SEL_1	FInsrt(0x1, BF_ISP_DT_SEL)	// <CCIR656 Format>	: Data[7:0] = {GPIOI[15:14],GPIOI[3:1],GPIOJ[15:13]}, Clock = GPIOK[2]
  	                                                // <Parallel RGB> : Data[23:0] = {GPIOI[15:14],GPIOI[3:1],GPIOJ[15:13],GPIOC[15:0]}, Clock = GPIOH[4]
  #define ISP_DT_SEL_2	FInsrt(0x2, BF_ISP_DT_SEL)	// Only CCIR656 Format : Data[7:0] = GPIOM[8:1], Clock = GPIOM[0]
#define ISP_EDT_WID		(1 << 12)		// ISP Data Bit Width, 0:Not 8 bits Data, 1:8 bits data
#define ISP_ST				(1 << 11)		// Starting Signal ("0" -> "1" -> "0")
#define ISP_ON				(1 << 10)		// ISP Enable
#define ISP_ACT				(1 << 9)		// ISP Active Signal
#define ISP_TB_BYPASS	(1 << 8)		// Gamma Table Bypass Signal, 0:Use Gamma Table, 1:Bypass Gammaa Table
#define ISP_MEM_WID		(1 << 6)		// RGB data 0:16bits, 1:8bits
#define BF_ISP_PATH		Fld(2,4)		
#define ISP_PATH			FMsk(BF_ISP_PATH)		// ISP Data Path
  #define ISP_TO_SC_YCE	FInsrt(0x0, BF_ISP_PATH)		// To Scale Processor With YC Enhancement
  #define ISP_TO_SC			FInsrt(0x1, BF_ISP_PATH)		// To Scale Processor Without YC Enhancement
  #define ISP_TO_MEM		FInsrt(0x2, BF_ISP_PATH)		// To Memory
#define BF_ISP_DT_FMT	Fld(4,0)
#define ISP_DT_FMT		FMsk(BF_ISP_DT_FMT)
  #define ISP_DT_422		FInsrt(0x0, BF_ISP_DT_FMT)	// 4:2:2 Format (16bits)
  #define ISP_DT_CCIR656I	FInsrt(0x3, BF_ISP_DT_FMT)	// CCIR656 Interlaced Format (8bits)
  #define ISP_DT_CCIR656P	FInsrt(0x4, BF_ISP_DT_FMT)	// CCIR656 Progressive Format (8bits)
  #define ISP_DT_CCIR656L	FInsrt(0x5, BF_ISP_DT_FMT)	// CCIR656 Like Format (8bits)
  #define ISP_DT_BAYER_DEV	FInsrt(0x6, BF_ISP_DT_FMT)	// Bayer RGB Format from External Device(8/10bits)
  #define ISP_DT_888		FInsrt(0x7, BF_ISP_DT_FMT)	// Parallel RGB(8:8:8) Format(24bits)
  #define ISP_DT_SERIAL		FInsrt(0x8, BF_ISP_DT_FMT)	// Serial RGB Format(10bits)
  #define ISP_DT_BAYER_MEM	FInsrt(0x9, BF_ISP_DT_FMT)	// Bayer RGB Format from Memory(8/16bits)
  #define ISP_DT_RGB_MEM	FInsrt(0xA, BF_ISP_DT_FMT)	// RGB Format from Memory(8/16 bits per Color)
//---------------------------------------------------------------------------------------	
	MMSP20_REG16	CON_2;			//0xc0003002		Image Signal Processor control register	0x00
#define ISP_LOCK_OFF	(1 << 13)	// Memory interface Lock disable signal, 0:lock on, 1:lock disable
#define ISP_BST_OFF	(1 << 12)	// Memory interface Burst disable signal, 0:burst on, 1:burst disable
#define ISP_VSYNC_ST_POL	(1 << 11)	// Select vertical starting point, 0:rising edge of vertical sync, 1:falling edge of vertical sync
#define BF_ISP_RGB_ORD	Fld(2,9)
#define ISP_RGB_ORD	FMsk(BF_ISP_RGB_ORD)	// Select RGB Order
  #define ORDER_RGB	FInsrt(0x0, BF_ISP_RGB_ORD)	// R,G,B
  #define ORDER_BRG	FInsrt(0x1, BF_ISP_RGB_ORD)	// B,R,G
  #define ORDER_GBR	FInsrt(0x2, BF_ISP_RGB_ORD)	// G,B,R
  #define ORDER_BGR	FInsrt(0x3, BF_ISP_RGB_ORD)	// B,G,R
#define ISP_FLD_GEN	(1 << 8)		// Generate Field Signal, 0:from field info(CCIR656),1:from External H/V Sync
#define BF_ISP_CLK_SRC	Fld(2,6)
#define ISP_CLK_SRC	FMsk(BF_ISP_CLK_SRC)	// Select clock source
  #define ISP_CLK_SRC_0	FInsrt(0x0, BF_ISP_CLK_SRC)	// GPIOH[4]
  #define ISP_CLK_SRC_1	FInsrt(0x1, BF_ISP_CLK_SRC)	// Clock=GPIOK[2]/GPIOM[0]
  #define ISP_CLK_SRC_2	FInsrt(0x2, BF_ISP_CLK_SRC)	// Divided Clock from Internal PLL clock
#define ISP_SYNC_DIR	(1 << 5)		// Select sync direction, 0:input from external sync, 1:output internal sync
#define ISP_CLK_MASK	(0x7 << 2)
#define ISP_CLK2_SYS	(1 << 4)		// Select ISP OUT clock source, 0:ISP divider clock, 1:system clock
#define ISP_CLK1_SYS	(1 << 3)		// Select ISP IN clock source, 0:ISP divider clock, 1:system clock
#define ISP_CLK_POL	(1 << 2)		// Select Input Clock Polarity, 0:normal, 1:inversion
#define ISP_VSYNC_POL	(1 << 1)		// VSYNC signal polarity, 0:normal, 1:inversion
#define ISP_HSYNC_POL	(1 << 0)		// HSYNC signal polarity, 0:normal, 1:inversion
//--------------------------------------------------------------------------------------	
	MMSP20_REG16	INT_CON;		//0xc0003004		ISP Interrupt control register	0x04
#define ISP_VSYNC	(1 << 14)	// Status of Vertical Sync Signal
#define ISP_HSYNC	(1 << 13)	// Status of Horizontal Sync Signal
#define ISP_VINT_EN	(1 << 5)	// Vertical Sync Interrupt Enable, 1:enable, 0:disable
#define ISP_VINT_TYPE	(1 << 4)	// Vertical Sync Interrupt Type, 0:falling edge, 1:rising edge
#define ISP_VINT_SC	(1 << 3)	// Vertical Sync Interrupt Status and Clear
#define ISP_HINT_EN	(1 << 2)	// Horizontal Sync Interrupt Enable
#define ISP_HINT_TYPE	(1 << 1)	// Horizontal Sync Interrupt Type
#define ISP_HINT_SC	(1 << 0)	// Horizontal Sync Interrupt Status and Clear
//---------------------------------------------------------------------------------------
	MMSP20_REG16	VPERIOD;		//0xc0003006		Vertical Sync Period	0x106
//---------------------------------------------------------------------------------------
	MMSP20_REG16	VPW;			//0xc0003008		Vertical Sync Pulse width	0x0A
#define BF_ISP_EAV2VSYNC	Fld(5,11)
#define ISP_EAV2VSYNC	FMsk(BF_ISP_EAV2SYNC)	// Vertical EAV to VSync Starting point. The unit is HSync. Effective when in CCIR656 mode
#define BF_ISP_VPW		Fld(10,0)
#define ISP_VPW		FMsk(BF_ISP_EAV2SYNC)	// VSync Pulse width. The unit is HSync. Effective when in Sync output mode(ISP_SYNC_DIR = 1)
//---------------------------------------------------------------------------------------
	MMSP20_REG16	HPERIOD;		//0xc000300a		Horizontal Sync period	0x187
//---------------------------------------------------------------------------------------
	MMSP20_REG16	HPW;			//0xc000300c		Horizontal Sync pulse width	0x0B
#define BF_ISP_EAV2HSYNC	Fld(7,9)	
#define ISP_EAV2HSYNC	FMsk(BF_ISP_EAV2HSYNC)	// Horizontal EAV to HSync Starting point. The unit is ISP_CLK. Effective when in CCIR656 Mode
#define BF_ISP_HPW		Fld(9,0)	
#define ISP_HPW		FMsk(BF_ISP_HPW)	// HSync Pulse width. The unit is ISP_CLK. Effective when in Sync output mode(ISP_SYNC_DIR = 1)
//---------------------------------------------------------------------------------------
	MMSP20_REG16	HACT_ST;		//0xc000300e		Horizontal Active Start Point.	0x0B
//---------------------------------------------------------------------------------------
	MMSP20_REG16	H_WIDTH;		//0xc0003010		Horizontal Active Width	0x14B
//---------------------------------------------------------------------------------------
	MMSP20_REG16	VACT_ST;		//0xc0003012		Vertical Active Start Point	0x0A
//---------------------------------------------------------------------------------------
	MMSP20_REG16	V_HEIGHT;		//0xc0003014		Vertical Active Height	0x0FA
//---------------------------------------------------------------------------------------
	MMSP20_REG16	SRC_HWIDTH;		//0xc0003016		ISP Horizontal Memory Width of Source Pixel	0x00
//---------------------------------------------------------------------------------------
	MMSP20_REG16	RGBSV;			//0xc0003018		Vertical Stating point of processing window	0x05
//---------------------------------------------------------------------------------------
	MMSP20_REG16	RGBEV;			//0xc000301a		Vertical Ending point of processing window	0xFF
//---------------------------------------------------------------------------------------
	MMSP20_REG16	RGBSH;			//0xc000301c		Horizontal Stating point of processing window	0x05
//-----------------------------------------------------------------------------------------
	MMSP20_REG16	RGBEH;			//0xc000301e		Horizontal Ending point of processing window	0x14F
//----------------------------------------------------------------------------------------
	MMSP20_REG16	AESV;			//0xc0003020		Vertical Starting Point for Auto Exposure/White Balance Accumulation.	0x0A
//-----------------------------------------------------------------------------------------
	MMSP20_REG16	AEEV;			//0xc0003022		Vertical Ending Point for Auto Exposure/White Balance Accumulation	0x0FA
//-----------------------------------------------------------------------------------------
	MMSP20_REG16	AESH;			//0xc0003024		Horizontal Starting Point for  Auto Exposure/White Balance Acc	0x0FA
//------------------------------------------------------------------------------------------
	MMSP20_REG16	AEEH;			//0xc0003026		Horizontal Ending Point for Auto Exposure/White Balance Accumulation	0x14A
//-------------------------------------------------------------------------------------------
	MMSP20_REG16	AELIM;			//0xc0003028		Upper and lower limit for auto exposure accumulation.	0xFF00
#define BF_ISP_AEULIM	Fld(8,8)
#define ISP_AEULIM	FMsk(BF_ISP_AEULIM)
#define BF_ISP_AELLIM	Fld(8,0)
#define ISP_AELLIM	FMsk(BF_ISP_AELLIM)
//-------------------------------------------------------------------------------------------
	MMSP20_REG16	AEVSTEP;		//0xc000302a		Vertical 4 block step size for auto exposure accumulation	0x3C
//--------------------------------------------------------------------------------------------
	MMSP20_REG16	AEHSTEP;		//0xc000302c		Horizontal 4 block step size for auto exposure accumulation	0x50
//-------------------------------------------------------------------------------------------
	MMSP20_REG16	AFSV;			//0xc000302e		Vertical starting window point for auto focus accumulation	0x0A
//--------------------------------------------------------------------------------------------
	MMSP20_REG16	AFEV;			//0xc0003030		Vertical ending window point for auto focus accumulation	0xFA
//--------------------------------------------------------------------------------------------
	MMSP20_REG16	AFSH;			//0xc0003032		Horizontal starting point for auto focus accumulation	0xFA
//--------------------------------------------------------------------------------------------
	MMSP20_REG16	AFEH;			//0xc0003034		Horizontal ending point for auto focus accumulation	0x14A
//--------------------------------------------------------------------------------------------
	MMSP20_REG16	CNTRSTGAIN;		//0xc0003036		Contrast Gain Control	0x0A
#define ISP_CNTRSTACCEN	(1 << 3)	// Accumulation enable signal for auto contrast control
#define BF_ISP_CNTRSTGAIN	Fld(3,0)
#define ISP_CNTRSTGAIN	FMsk(BF_ISP_CNTRSTGAIN)
//--------------------------------------------------------------------------------------------
	MMSP20_REG16	CNTRSTLIM;		//0xc0003038		Contrast Limit Control	0xFA
#define BF_ISP_CNTRUL	Fld(8,8)
#define ISP_CNTRUL	FMsk(BF_ISP_CNTRUL)
#define BF_ISP_CNTRLL	Fld(8,0)
#define ISP_CNTRLL	FMsk(BF_ISP_CNTRLL)
//--------------------------------------------------------------------------------------------
	MMSP20_REG16	RGBREC;			//0xc000303a		Bayer RGB Recovery Method	0x20
#define ISP_R_REC_TYPE	(1 << 8)	// R Pixel Recovery Type, 0:median filter, 1:average
#define BF_ISP_G_REC_TYPE	Fld(3,5)
#define ISP_G_REC_TYPE	FMsk(BF_ISP_G_REC_TYPE)	// G Pixel Recovery Type
#define ISP_B_REC_TYPE	(1 << 4)	// B Pixel Recovery Type, 0:median filter, 1:average
#define BF_ISP_BAYER_ODR	Fld(2,2)	
#define ISP_BAYER_ODR		FMsk(BF_ISP_BAYER_ODR)	// Bayer RGB Array Type, 0:first GB, 1:first BG, 2:first RG, 3:first GR
  #define FIRST_GB			FInsrt(0x0, BF_ISP_BAYER_ODR)
  #define FIRST_BG			FInsrt(0x1, BF_ISP_BAYER_ODR)
  #define FIRST_RG			FInsrt(0x2, BF_ISP_BAYER_ODR)
  #define FIRST_GR			FInsrt(0x3, BF_ISP_BAYER_ODR)
#define ISP_CBRSWAP			(1 << 1)	// Cb/Cr Swap, 0:normal, 1:swap
#define ISP_VID_RANGE		(1 << 0)	// Video Range, 0:Y=219xY(0~255)+16, Cb=224xCb(0~255), Cr=224xCr(0~255)
                                  // 1:Y=Y(0~255), Cb=Cb(0~255), Cr=Cr(0~255)
//-------------------------------------------------------------------------------------
	MMSP20_REG16	Reserved303c;		//0xc000303c
	MMSP20_REG16	Reserved303e;		//0xc000303e
//-------------------------------------------------------------------------------------	
	MMSP20_REG16	HUECB1AB;		//0xc0003040		Hue Control of First Coordinate	0x4000
	MMSP20_REG16	HUECR1AB;		//0xc0003042		Hue Control of First Coordinate	0x0040
	MMSP20_REG16	HUECB2AB;		//0xc0003044		Hue Control of Second Coordinate	0x4000
	MMSP20_REG16	HUECR2AB;		//0xc0003046		Hue Control of Second Coordinate	0x0040
	MMSP20_REG16	HUECB3AB;		//0xc0003048		Hue Control of Third Coordinate	0x4000
	MMSP20_REG16	HUECR3AB;		//0xc000304a		Hue Control of Third Coordinate	0x0040
	MMSP20_REG16	HUECB4AB;		//0xc000304c		Hue Control of Fourth Coordinate	0x4000
	MMSP20_REG16	HUECR4AB;		//0xc000304e		Hue Control of Fourth Coordinate	0x0040
#define BF_ISP_HUECBA		Fld(8,8)
#define ISP_HUECBA			FMsk(BF_ISP_HUECBA)
#define BF_ISP_HUECBB		Fld(8,0)
#define ISP_HUECBB			FMsk(BF_ISP_HUECBB)
#define BF_ISP_HUECRA		Fld(8,8)
#define ISP_HUECRA			FMsk(BF_ISP_HUECRA)
#define BF_ISP_HUECRB		Fld(8,0)
#define ISP_HUECRB			FMsk(BF_ISP_HUECRB)
//-------------------------------------------------------------------------------------
	MMSP20_REG16	Reserved3050;		//0xc0003050
	MMSP20_REG16	Reserved3052;		//0xc0003052
//-------------------------------------------------------------------------------------	
	MMSP20_REG16	YGAIN;			//0xc0003054		Horizontal Luminance Enhancement Gain	0x13
#define BF_ISP_YEDGEGAIN	Fld(3,4)
#define ISP_YEDGEGAIN			FMsk(BF_ISP_YEDGEGAIN)	// Horizontal Edge Gain
#define BF_ISP_YMAINGAIN	Fld(3,0)
#define ISP_YMAINGAIN			FMsk(BF_ISP_YMAINGAIN)	// Horizontal Main Path	
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	CDLY;			//0xc0003056		Horizontal Chrominance Delay	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	BRIGHT;			//0xc0003058		Brightness Control	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	YCLIP;			//0xc000305a		Clip Point	0xFFFF
#define BF_ISP_CLIP2		Fld(8,8)
#define ISP_CLIP2				FMsk(BF_ISP_CLIP2)	// High Clip, Range=0~255
#define BF_ISP_CLIP1		Fld(8,0)
#define ISP_CLIP1				FMsk(BF_ISP_CLIP1)	// Clip knee, Range=0~255	
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	YCLIPSLANT;		//0xc000305c		Clip Slant	0x40
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	CSUPSLANT;		//0xc000305e		Color Suppress Slant	0x20
#define ISP_CSLANT_EN		(1 << 15)
#define ISP_DECI_OFF		(1 << 14)
#define BF_ISP_CSLANT_PNT		Fld(3,0)
#define ISP_CSLANT_PNT	FMsk(BF_ISP_CSLANT_PNT)		// Suppress Gain Control	
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	SLICELMT;		//0xc0003060		Slice Limit	0x08
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBLMT;			//0xc0003062		Limit for White Balance Accumulation	0xFFFF
#define BF_ISP_WB_ULMT	Fld(8,8)
#define ISP_WB_ULMT			FMsk(BF_ISP_WB_ULMT)	// White balance accumulation Luminance level Upper Limit
#define BF_ISP_WB_LLMT	Fld(8,0)
#define ISP_WB_LLMT			FMsk(BF_ISP_WB_LLMT)	// White balance accumulation Luminance level Lower Limit
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBACCCNTL;		//0xc0003064		Count Accumulation for White Balance (Low)	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBACCCNTH;		//0xc0003066		Count Accumulation for White Balance (High)	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCBACCL;		//0xc0003068		Cb Accumulation for White Balance (Low)	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCBACCH;		//0xc000306a		Cb Accumulation for White Balance (High)	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCRACCL;		//0xc000306c		Cr Accumulation for White Balance (Low)	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCRACCH;		//0xc000306e		Cr Accumulation for White Balance (High)	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	AFACCL;			//0xc0003070		Auto Focus Accumulation	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	AFACCH;			//0xc0003072		Auto Focus Accumulation	0x00
//--------------------------------------------------------------------------------------
	MMSP20_REG16	Reserved74;		//0xc0003074
//--------------------------------------------------------------------------------------	
	MMSP20_REG16	WBCOORD0;		//0xc0003076		0~15 block setting, [0]:LSB, [15]:MSB	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD1;		//0xc0003078		16~31 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD2;		//0xc000307a		32~47 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD3;		//0xc000307c		48~63 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD4;		//0xc000307e		64~79 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD5;		//0xc0003080		80~95 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD6;		//0xc0003082		96~111 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD7;		//0xc0003084		112~127 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD8;		//0xc0003086		128~143 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD9;		//0xc0003088		144~159 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD10;		//0xc000308a		160~175 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD11;		//0xc000308c		176~191 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD12;		//0xc000308e		192~207 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD13;		//0xc0003090		208~223 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD14;		//0xc0003092		224~239 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	WBCOORD15;		//0xc0003094		240~255 block setting	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	MEMADDRL1;		//0xc0003096		Low Address of External Memory	0x00, Ext. Device --> Mem, Dst addr. Mem --> Mem Src addr.
	MMSP20_REG16	MEMADDRH1;		//0xc0003098		High Address of External Memory	0x00,
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	GAMMA_A;		//0xc000309a		Address Interface of Gamma Table	0x00
//-------------------------------------------------------------------------------------		
	MMSP20_REG16	GAMMA_D;		//0xc000309c		Data Interface of Gamma TableThe address will increase automatically after write the Data	0x00
//----------------------------------------------------------------------------------------------
	MMSP20_REG16	AEACC[32];		//0xc000309e		~ 0xc00030dc	HW	Auto Exposure Result29bit data (Low 16 bit, High 13bit)	0x00
//----------------------------------------------------------------------------------------------
	MMSP20_REG16	CNTST_LLCNTL;	//0xc00030de		~ 0xc00030e0 	HW	Low Level(1)  
	MMSP20_REG16	CNTST_LLCNTH;	//0xc00030e0
//----------------------------------------------------------------------------------------------
	MMSP20_REG16	CNTST_HLCNTL;	//0xc00030e2		~ 0xc00030e4	HW	High Level(1)  
	MMSP20_REG16	CNTST_HLCNTH;	//0xc00030e4
//----------------------------------------------------------------------------------------------
	MMSP20_REG16	CNTST_LLACCL;	//0xc00030e6		~ 0xc00030e8	HW	Low Level(1)  
	MMSP20_REG16	CNTST_LLACCH;	//0xc00030e8
//----------------------------------------------------------------------------------------------
	MMSP20_REG16	CNTST_HLACCL;	//0xc00030ea		~ 0xc00030ec	HW	High Level(1)  
	MMSP20_REG16	CNTST_HLACCH;	//0xc00030ec
//----------------------------------------------------------------------------------------------
	MMSP20_REG16	MEMADDRL2;		//0xc00030ee		Low Address of External Memory	0x00, Only Mem-->Mem, Dst addr.
	MMSP20_REG16	MEMADDRH2;		//0xc00030f0		High Address of External Memory	0x00
//----------------------------------------------------------------------------------------------	
	MMSP20_REG16	DST_HWIDTH;		//0xc00030f2		ISP Horizontal Memory Width of Destination Pixel	0x00
} MMSP20_ISP;


/* Pulse Width Modulator */
typedef struct {
	MMSP20_REG16 PWMPRE01;			// 0xc0000b00		PWM ch0/1 Prescale Register
#define BF_PWM1POL			Fld(1, 15)	
#define PWM1POL					FMsk(BF_PWM1POL)		// PWM ch1 polarity : 0:inverted, 1:bypass
#define BF_PWM1PRESCALE	Fld(7, 8)
#define PWM1PRESCALE		FMsk(BF_PWM1PRESCALE)		// PWM ch1 prescale 
#define BF_PWM0POL			Fld(1, 7)
#define PWM0POL					FMsk(BF_PWM0POL)		// PWM ch0 polarity : 0:inverted, 1:bypass
#define BF_PWM0PRESCALE	Fld(7, 0)
#define PWM0PRESCALE		FMsk(BF_PWM0PRESCALE)		// PWM ch0 prescale 
//----------------------------------------------------------------------------------------
	MMSP20_REG16 PWMDUTY0;			// 0xc0000b02		PWM ch0 Duty Cycle
	MMSP20_REG16 PWMDUTY1;			// 0xc0000b04		PWM ch1 Duty Cycle
	MMSP20_REG16 PWMPERD0;			// 0xc0000b06		PWM ch0 Period Cycle
	MMSP20_REG16 PWMPERD1;			// 0xc0000b08		PWM ch1 Duty Cycle
//----------------------------------------------------------------------------------------
	MMSP20_REG16 Reserved0b0a;
	MMSP20_REG16 Reserved0b0c;
	MMSP20_REG16 Reserved0b0e;
//----------------------------------------------------------------------------------------
	MMSP20_REG16 PWMPRE23;			// 0xc0000b10		PWM ch2/3 Prescale Register
#define BF_PWM3POL			Fld(1, 15)
#define PWM3POL					FMsk(BF_PWM3POL)		// PWM ch3 polarity  : 0:inverted, 1:bypass
#define BF_PWM3PRESCALE	Fld(7, 8)
#define PWM3PRESCALE		FMsk(BF_PWM1PRESCALE)		// PWM ch3 prescale
#define BF_PWM2POL			Fld(1, 7)
#define PWM2POL					FMsk(BF_PWM2POL)		// PWM ch2 polarity : 0:inverted, 1:bypass
#define BF_PWM2PRESCALE	Fld(7, 0)
#define PWM2PRESCALE		FMsk(BF_PWM0PRESCALE)		// PWM ch2 prescale 
//----------------------------------------------------------------------------------------
	MMSP20_REG16 PWMDUTY2;
	MMSP20_REG16 PWMDUTY3;
	MMSP20_REG16 PWMPERD2;
	MMSP20_REG16 PWMPERD3;
} MMSP20_PWM;



/***********************************************************************************
 * Modify to 13.3
 */

#endif
