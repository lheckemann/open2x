/*
 * (C) Copyright 2004
 */

/************************************************
 * NAME	    : mmsp20_type.h
 * Version  : 08.01.2004
 * tukho.kim samsung electronics tukho.kim@smasung.com
 * 
 * common stuff for Magic eyes MMSP20 SoC
 ************************************************/

/*******************************************************
 * Modify . 12.01.2004 : Datasheet 0.9
 * 

*******************************************************/

#ifndef __MMSP20_TYPE_H__
#define __MMSP20_TYPE_H__

#include <asm/arch/bitfield.h>

typedef volatile u8		MMSP20_REG8;
typedef volatile u16	MMSP20_REG16;
typedef volatile u32	MMSP20_REG32;

/* BANK C MEMORY CONTROLLER REGISTER */
typedef struct {
	MMSP20_REG16	MEMCFGW;  	/* Configration register */
	MMSP20_REG16	MEMTIMEW[5];	/* Timinig register 0 ~ 5 */
	MMSP20_REG16	RESERVE1;	/* RESERVE */
	MMSP20_REG16	MEMWATICTRLW;	/* Wait control register */
	MMSP20_REG16	MEMPAGEW;	/* Page Control register */
	MMSP20_REG16	MEMIDETIMEW;	/* IDE Timinig Control Register */
	MMSP20_REG16	MEMPCMCIAMW;	/* PCMCIA Memory Timinig Control Register */
	MMSP20_REG16	MEMPCMCIAAW;    /* PCMCIA Attribute Timnig control Register */
	MMSP20_REG16	MEMPCMCIAIW;	/* PCMCIA I/O Timinig Register */
	MMSP20_REG16	MEMPCMCIAWAITW; /* PCMCIA Wait Timinig Register #0 */
	MMSP20_REG16	MEMIDEWAITW;	/* IDE Wait timing Register #1 */
	MMSP20_REG16	RESERVE2;	/* RESERVE C000 1Eh */
	MMSP20_REG16	MEMDTIMEOUTW;	/* DMA Timeout Register */
	MMSP20_REG16	MEMDMACTRLW;    /* DMA Control Register */
	MMSP20_REG16	MEMDMAPOLW;	/* DMA Polarity Register */
	MMSP20_REG16	MEMDMATIMEW[9]; /* DMA Timing Register 0 ~ 8 */
	MMSP20_REG16	MEMDMASTRBW; 	/* DMA Strobe Control Register */
	MMSP20_REG16	MEMNANDCTRLW;   /* NAND CTRL register */
	MMSP20_REG16	MEMNANDTIMEW; 	/* NAND Timing Register */
	MMSP20_REG16	MEMNANDECCW[3]; /* NAND ECC Register 0 ~ 2 */
	MMSP20_REG16	MEMNANDCNTW; 	/* NAND Data Counter Register */
/*	MMSP20_REG16	MEMCHIPSELTW;*/	/* CHIP selection control register -> none */ 
} MMSP20_BANC_MEMCTL; 		/* address C000 00h ~ 46h */

/* BANK C Memory NAND Memory control */
typedef struct {
	/* address no sequential */
	MMSP20_REG16	NFDATA;		/* NAND Flash Data Register 1C000000h or 9C000000h*/
	MMSP20_REG16	RESERVE1[7];
	MMSP20_REG16	NFCMD;		/* NAND Flash Command register 1C000010h or 9C000010h*/
	MMSP20_REG16	RESERVE2[3];
	MMSP20_REG16	NFADDR;		/* NAND Flash Address Register 1C000018h or 9C000018h*/
} MMSP20_NAND;

/* BANK A Memory Controller register C000h */
typedef struct {
	MMSP20_REG16	MEMCFGX;	/* SDRAM Configuration Register 5400h */
	MMSP20_REG16	MEMTIMEX[2];	/* Timinig register 0, 1 */
	MMSP20_REG16	MEMACTPWDX; 	/* Active power down control register */
	MMSP20_REG16	MEMREFX;	/* Refresh Control register */
} MMSP20_BANA_MEMCTL;			/* C0005400h ~ C0005408 */

/* Dual cpu control register CXXX */
typedef struct {
	MMSP20_REG16 	DUAL920DATA[16]; /* ARM920 Data register 0 ~ 15 */
	MMSP20_REG16	DUAL940DATA[16]; /* ARM940 Data register 0 ~ 15 */
	MMSP20_REG16	DUALINT920;	/* ARM920 interrupt enable register */
	MMSP20_REG16	DUALINT940;	/* ARM940 interrupt enable register */
	MMSP20_REG16	DUALPEND920;	/* ARM920 interrupt pending register */
	MMSP20_REG16	DUALPEND940;	/* ARM940 interrupt pending register */
	MMSP20_REG16	DUALCTRL940;	/* ARM940 control register */
} MMSP20_DUAL_CPUCTL; /* CXXX0000 ~ 0048 */

/* Clock & Power Manager Register BASE C000 0900 */

typedef struct {
/* power */	
	MMSP20_REG16	PWMODEREG; 		/* Power mode register */
/* clock */
	MMSP20_REG16	CLKCHGSTREG;	/* Clock Change Status Register */
	MMSP20_REG16	SYSCLKENREG;	/* Clock Enable Register */
	MMSP20_REG16	RESERVE1;		/* RESERVE1 C000 0906h */
	MMSP20_REG16	COMCLKENREG; 	/* Communication Device Clock Enable Register */
	MMSP20_REG16	VGCLKENREG;		/* Video & Graphic Device Clock Enable Register */
	MMSP20_REG16	ASCLKENREG;		/* Audio & Storage Device clock Enable Register */
	MMSP20_REG16	RESERVE2;		/* RESERVE2 C000 090Eh */
	MMSP20_REG16	FPLLSETVREG;	/* FCLK PLL Setting Value Write Register */
	MMSP20_REG16	FPLLVSETREG;	/* FCLK PLL Value Setting Register */
	MMSP20_REG16	UPLLSETVREG;	/* UCLK PLL Setting Value Write Register */
	MMSP20_REG16	UPLLVSETREG;	/* UCLK PLL Value Setting Register */
	MMSP20_REG16	APLLSETVREG;	/* ACLK PLL Setting Value Write Register */
	MMSP20_REG16	APLLVSETREG;	/* ACLK PLL Value Setting Register */
	MMSP20_REG16	SYSCSETREG;		/*System Clock PLL Divide Value Set Register */
	MMSP20_REG16	ESYSCSETREG;	/* External System Clock Time Set Register */
	MMSP20_REG16	UIRMCSETREG;	/* USB/IRDA/MMC Clock Generation U-PLL Divide Value Set Register */
	MMSP20_REG16	AUDICSETREG;	/* Audio Controller clock generation PLL Divide Value Set */
	MMSP20_REG16	DISPCSETREG;	/* Display Clock Generation PLL Divide Value Set Register */
	MMSP20_REG16	IMAGCSETREG;	/* Image pixel clock generation PLL Divide value set register */
	MMSP20_REG16	URTCSETREG[2];	/* UART0/1, 2/3  Tx/Rx clock generaiton PLL Divide value set register */
/* power */
	MMSP20_REG16	ARM940TMODEREG; /* ARM940T CPU power manage mode register */
/* clock */
	MMSP20_REG16	SPDICSETREG;	/* SPDIF Controller clock generation PLL Divide Value Set Register */
} MMSP20_CLOCK_POWER; /*C0000900h ~ C000092Ch*/

/* Real time clock */
typedef struct {
	MMSP20_REG32	RTCTSETREG;	/* RTC counter set register */
	MMSP20_REG32	RTCTCNTREG;	/* RTC time count register */
	MMSP20_REG16	RTCSTCNTREG;	/* RTC Sub-second time count register */
	MMSP20_REG16	TICKSETREG;	/* RTC Time Tick Set Register */	
	MMSP20_REG16	ALARMTREG;	/* ALARM Time set register */
	MMSP20_REG16	RESERVE;		/* RESERVE C000 0C0Eh */
/* manage */
	MMSP20_REG16	PWRMGRREG;	/* power manage register */
	MMSP20_REG16	CLKMGRREG;	/* clock manage register */
	MMSP20_REG16	RSTCTRLREG;	/* Reset enable control register */
	MMSP20_REG16	RSTSTREG;	/* Reset Event Status Register */
	MMSP20_REG16	BOOTCTRLREG;	/* Boot operation control register */
	MMSP20_REG16	LOCKTIMEREG;	/* Clock Locking Time Set Register */
	MMSP20_REG16	RSTTIMEREG;	/* Reset Width Time Set Register */
	MMSP20_REG16	EXTCTRLREG;	/* Ext. Input 0/1 Pin State Control register */
	MMSP20_REG16	STOPTSETREG;	/* Stop watch time */
	MMSP20_REG16	RTCCTRLREG;	/* RTC stoptime control register */
	MMSP20_REG16	RTSTRLREG;	/* RTC Interrupt status register */
} MMSP20_RTC;

/* DMA Controller C000 0200h */
typedef struct {
	MMSP20_REG16	DMACOM[3];	/* DMA Command register 0~2 */
	MMSP20_REG16	DMACONS;	/* DMA control/status register */
	MMSP20_REG16	SRCLADDR;	/* DMA source Low address register */
	MMSP20_REG16	SRCHADDR;	/* DMA source High address register */
	MMSP20_REG16	TRGLADDR;	/* DMA target Low address register */
	MMSP20_REG16	TRGHADDR;	/* DMA target High address register */
} MMSP20_DMAS; 

/* Not Sequence C000 0000h */
#define DMAINT 	*(MMSP20_REG16 *) (0xC0000000)	;		/* DMA Interrupt Status Register */

struct MMSP20_DCHSTRM{		/* Type DMA Chanel source/target i/o request map register */
	MMSP20_REG16	DCHSRM;		/* DMA Chanel Source I/o Request Map register */
	MMSP20_REG16	DCHTRM;		/* DMA Chanel Tareet i/O Request Map Register */
};

typedef struct {
/* Not Sequence C000 0100h */
	struct MMSP20_DCHSTRM 	DCHSTRM[16];	/* DMA Channel Source/Target i/o request map register 0~ 15 */
} MMSP20_DMAREQ;

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

	MMSP20_REG16	RESERVE[4];	/*  Reserve */
/* Not sequential C000 1280h */
	MMSP20_REG16	INTSTATREG;	/* Uart interrupt status register */
	MMSP20_REG16	PORTCON; 	/* Uart port confrol register */
} MMSP20_UART;

/* GPIO configration register */
typedef struct {
	MMSP20_REG16	GPIOALTFNLOW[15];	/* GPIOx alternate function low regiter */
	MMSP20_REG16	RESERVE1;
	
	MMSP20_REG16	GPIOALTFNHI[15];	/* GPIOx alternate function hi regiter */
	MMSP20_REG16	RESERVE2;

	MMSP20_REG16	GPIOOUT[15];		/* GPIOx outpur level register */
	MMSP20_REG16	RESERVE3;

	MMSP20_REG16	GPIOEVTTYPLOW[15];	/* GPIOx Event type low regiter */
	MMSP20_REG16	RESERVE4;

	MMSP20_REG16	GPIOEVTTYPHI[15];	/* GPIOx Event type hi regiter */
	MMSP20_REG16	RESERVE5;	

	MMSP20_REG16	GPIOPUENB[15];		/* GPIOx Pull Up Enable Register */
	MMSP20_REG16	RESERVE6;

	MMSP20_REG16	GPIOINTENB[15];		/* GPIOx Interrupt Enable Register */
	MMSP20_REG16	RESERVE7;

	MMSP20_REG16	GPIOEVT[15];		/* GPIOx Event Register */
	MMSP20_REG16	RESERVE8;

	MMSP20_REG16	GPIOPADSEL;		/* GPIOx PAD selection register*/
	MMSP20_REG16	RESERVE9[15];

	MMSP20_REG16	GPIOGREVTSTAT;		/* GPIO Group Event Regiter */
	MMSP20_REG16	RESERVE10[31];
 
	MMSP20_REG16	GPIOPINLVL[15];		/* GPIOx Pin Level Register */
	MMSP20_REG16	RESERVE11;
 
	MMSP20_REG16	GPIOEOUT_ALIVE;		/* GPIOE Output Alibe register */
	MMSP20_REG16	GPIOEOUTENB_ALIVE;	/* GPIOE Output Enable Alibe register */
	MMSP20_REG16	GPIOEPUENB_ALIVE;	/* GPIOE Pull up Enable Alibe register */

	MMSP20_REG16	GPIOFOUT_ALIVE;		/* GPIOF Output Alibe register */
	MMSP20_REG16	GPIOFOUTENB_ALIVE;	/* GPIOF Output Enable Alibe register */
	MMSP20_REG16	GPIOFPUENB_ALIVE;	/* GPIOF Pull up Enable Alibe register */

	MMSP20_REG16	GPIOGOUT_ALIVE;		/* GPIOG Output Alibe register */
	MMSP20_REG16	GPIOGOUTENB_ALIVE;	/* GPIOG Output Enable Alibe register */
	MMSP20_REG16	GPIOGPUENB_ALIVE;	/* GPIOG Pull up Enable Alibe register */

	MMSP20_REG16	GPIOIOUT_ALIVE;		/* GPIOI Output Alibe register */
	MMSP20_REG16	GPIOIOUTENB_ALIVE;	/* GPIOI Output Enable Alibe register */
	MMSP20_REG16	GPIOIPUENB_ALIVE;	/* GPIOI Pull up Enable Alibe register */

	MMSP20_REG16	GPIOJOUT_ALIVE;		/* GPIOJ Output Alibe register */
	MMSP20_REG16	GPIOJOUTENB_ALIVE;	/* GPIOJ Output Enable Alibe register */
	MMSP20_REG16	GPIOJPUENB_ALIVE;	/* GPIOJ Pull up Enable Alibe register */

} MMSP20_GPIO;


/* VIdeo Post processor NOT DEFINE EMPTY ADDRESS*/

/* FRAME Dimension Converter */
typedef struct {
	MMSP20_REG16	FDC_CNTL; 		/* control register */
	MMSP20_REG16	FDC_FRAME_SIZE;		/* Frame size */
	MMSP20_REG16	FDC_LUMA_OFFSET;	/* Luminance Offset register */
	MMSP20_REG16	FDC_CB_OFFSET;		/* Cb Offset Register */
	MMSP20_REG16	FDC_CR_OFFSET;		/* Cr Offset Register */
	MMSP20_REG16	FDC_DST_BASE_L;		/* Target start address */
	MMSP20_REG16	FDC_DST_BASE_H;		/* Target start address */
	MMSP20_REG16	FDC_STATUS;		/* status register */
	MMSP20_REG16	FDC_DERING;		/* Dering control register */
	MMSP20_REG16	FDC_OCC_CNTL;		/* Occurrence control register */
} MMSP20_FDCCTL;

/* Scale Process register */

typedef struct {
	MMSP20_REG16	SC_CMD;			/* scale control register */
	MMSP20_REG16	SC_STATUS;		/* Scale status rgister */
	MMSP20_REG16	SC_MIRROR;		/* Scaling status rgister */
	MMSP20_REG16	SC_PRE_VRATIO;		/* Pre-scale verical ratio */
	MMSP20_REG16	SC_PRE_HRATIO;		/* Pre-scale horizontal ratio */
	MMSP20_REG16	SC_POST_VRATIOL;	/* Vertical Post-Scaling ratio (Low) */
	MMSP20_REG16	SC_POST_VRATIOH;	/* Vertical Post-Scaling ratio (High) */
	MMSP20_REG16	SC_POST_HRATIOL;	/* Horizontal Post-Scaling ratio (Low) */
	MMSP20_REG16	SC_POST_HRATIOH;	/* Horizontal Post-Scaling ratio (High)*/
	MMSP20_REG16	SC_SRC_ODD_ADDRL;	/* Source Address Starting point of Odd Field (Low) */
	MMSP20_REG16	SC_SRC_ODD_ADDRH;	/* Source Address Starting point of Odd Field (High) */
	MMSP20_REG16	SC_SRC_EVEN_ADDRL;	/* Source Address Starting point of Even Field (Low) */	
	MMSP20_REG16	SC_SRC_EVEN_ADDRH;	/* Source Address Starting point of Even Field (High) */
	MMSP20_REG16	SC_DST_ADDRL;		/* Destination Address Starting point of Odd Field(Low) */
	MMSP20_REG16	SC_DST_ADDRH;		/* Destination Address Starting point of Odd Field(High) */
	MMSP20_REG16	SC_SRC_PXL_HEIGHT;	/* Vertical Source Pixel Height */
	MMSP20_REG16	SC_SRC_PXL_WIDTH;	/* Horizontal Source Pixel Width */
	MMSP20_REG16	SC_SRC_PXL_REQCNT;	/* When Pre Scale Mode, Horizontal Pixel width of 
						Prescale output
						When No pre-scale mode, Horizontal Source Pixel size*/
	MMSP20_REG16	SC_DST_PXL_HEIGHT;	/* Vertical Destination Pixel Height. */
	MMSP20_REG16	SC_DST_PXL_WIDTH;	/* Horizontal Destination Pixel Width. */
	MMSP20_REG16	SC_DST_WPXL_WIDTH;	/* Horizontal Wide Destination Pixel Width. */
	MMSP20_REG16	SC_SEP_ADDR[3];		/* Two Dimensional Destination Address of 
						Luminance and Chrominance */
	MMSP20_REG16	SC_DELAY;		/*Luminance and Chrominance path Delay */
	MMSP20_REG16	SC_MEM_CNTR;		/* Memory control */
	MMSP20_REG16	SC_IRQ;			/* Interrupt Control */
} MMSP20_SPRCTL;



/************************************************************************************/

#endif
