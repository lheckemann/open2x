/*
 * include/asm-arm/arch-mmsp2/mmsp2-regs.h
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 */

#ifndef _MMSP2_H
#define _MMSP2_H

#include "hardware.h"
#include "bitfield.h"


/*
 * BANK C (Static) Memory Control Register
 */
#define MEMCFG		__REGW(0xc0003a00)	/* BANK C configuration */
#define MEMTIME0	__REGW(0xc0003a02)	/* BANK C Timing #0 */
#define MEMTIME1	__REGW(0xc0003a04)	/* BANK C Timing #1 */
#define MEMTIME2	__REGW(0xc0003a06)	/* BANK C Timing #2 */
#define MEMTIME3	__REGW(0xc0003a08)	/* BANK C Timing #3 */
#define MEMTIME4	__REGW(0xc0003a0a)	/* BANK C Timing #4 */
#define MEMWAITCTRL	__REGW(0xc0003a0e)	/* BANK C Wait Control */
#define MEMPAGE		__REGW(0xc0003a10)	/* BANK C Page Control */
#define MEMIDETIME	__REGW(0xc0003a12)	/* BANK C IDE Timing Control */

#define MEMPCMCIAM	__REGW(0xc0003a14)	/* BANK C PCMCIA Timing */
#define MEMPCMCIAA	__REGW(0xc0003a16)	/* PCMCIA Attribute Timing */
#define MEMPCMCIAI	__REGW(0xc0003a18)	/* PCMCIA I/O Timing */
#define MEMPCMCIAWAIT	__REGW(0xc0003a1a)	/* PCMCIA Wait Timing */

#define MEMEIDEWAIT	__REGW(0xc0003a1c)	/* IDE Wait Timing */

#define MEMDTIMEOUT	__REGW(0xc0003a20)	/* DMA Timeout */
#define MEMDMACTRL	__REGW(0xc0003a22)	/* DMA Control */
#define MEMDMAPOL	__REGW(0xc0003a24)	/* DMA Polarity */
#define MEMDMATIME0	__REGW(0xc0003a26)	/* DMA Timing #0 */
#define MEMDMATIME1	__REGW(0xc0003a28)	/* DMA Timing #1 */
#define MEMDMATIME2	__REGW(0xc0003a2a)	/* DMA Timing #2 */
#define MEMDMATIME3	__REGW(0xc0003a2c)	/* DMA Timing #3 */
#define MEMDMATIME4	__REGW(0xc0003a2e)	/* DMA Timing #4 */
#define MEMDMATIME5	__REGW(0xc0003a30)	/* DMA Timing #5 */
#define MEMDMATIME6	__REGW(0xc0003a32)	/* DMA Timing #6 */
#define MEMDMATIME7	__REGW(0xc0003a34)	/* DMA Timing #7 */
#define MEMDMATIME8	__REGW(0xc0003a36)	/* DMA Timing #8 */
#define MEMDMASTRB	__REGW(0xc0003a38)	/* DMA Strobe Control */

#define MEMNANDCTRL	__REGW(0xc0003a3a)	/* NAND FLASH Control */
#define MEMNANDTIME	__REGW(0xc0003a3c)	/* NAND FLASH Timing */
#define MEMNANDECC0	__REGW(0xc0003a3e)	/* NAND FLASH ECC0 */
#define MEMNANDECC1	__REGW(0xc0003a40)	/* NAND FLASH ECC1 */
#define MEMNANDECC2	__REGW(0xc0003a42)	/* NAND FLASH ECC2 */
#define MEMNANDCNT	__REGW(0xc0003a44)	/* NAND FLASH Data Counter */

#ifndef __NF_REGW
#error "are you crazy ?"
#endif

#ifndef __NF_REGB
#error "are you crazy ?"
#endif

/* refer to asm/arch/hardware.h */
#define NFDATAb		__NF_REGB(NF_PIO_BASE + 0x00)
#define NFDATAl		__NF_REGL(NF_PIO_BASE + 0x00)
#define NFDATA		__NF_REGW(NF_PIO_BASE + 0x00)	/* NAND FLASH Data */
#define NFCMD		__NF_REGB(NF_PIO_BASE + 0x10)	/* NAND FLASH Command */
#define NFADDR		__NF_REGB(NF_PIO_BASE + 0x18)	/* NAND FLASH Address */

#define NAND_READY_DETECT	(1 << 15)
#define NAND_IRQEN			(1 << 6)
#define NAND_RNB			(1 << 7)

/* Bank A Memory (SDRAM) Control Register */
#define MEMCFGX		__REGW(0xc0003800)	/* SDRAM Configuration */
#define MEMTIMEX0	__REGW(0xc0003802)	/* SDRAM Timing #0 */
#define MEMTIMEX1	__REGW(0xc0003804)	/* SDRAM Timing #1 */
#define MEMACTPWDX	__REGW(0xc0003806)	/* Active Power Down Ctrl */
#define MEMREFX		__REGW(0xc0003808)	/* Refresh Ctrl */


/*
 * Chapter 5
 * Clocks and Power Manager
 */
#define PWMODE		__REGW(0xc0000900)	/* Power Mode */
#define CLKCHGST	__REGW(0xc0000902)	/* Clock Change Status */
#define SYSCLKEN	__REGW(0xc0000904)	/* System Clock Enable */
#define COMCLKEN	__REGW(0xc0000908) /* Communication Device Clk En */
#define VGCLKEN		__REGW(0xc000090a) /* Video & Graphic Device Clk En */
#define ASCLKEN		__REGW(0xc000090c) /* Audio & Storage Device Clk En */
#define FPLLSETV	__REGW(0xc0000910) /* FCLK PLL Setting Value Write */
#define FPLLVSET	__REGW(0xc0000912) /* FCLK PLL Value Setting */
#define UPLLSETV	__REGW(0xc0000914) /* UCLK PLL Setting Value Write */
#define UPLLVSET	__REGW(0xc0000916) /* UCLK PLL Value Setting */
#define APLLSETV	__REGW(0xc0000918) /* ACLK PLL Setting Value Write */
#define APLLVSET	__REGW(0xc000091a) /* ACLK PLL Value Setting */
#define SYSCSET		__REGW(0xc000091c) /* System CLK PLL Divide Value */
#define ESYSCSET	__REGW(0xc000091e) /* External System Clk Time Set */
#define UIRMCSET	__REGW(0xc0000920) /* USB/IRDA/MMC Clk Gen */
#define AUDICSET	__REGW(0xc0000922) /* Audio Ctrl Clk Gen */
#define SPDICSET	__REGW(0xc000092e) /* SPDIF Ctrl Clk Gen */
#define DISPCSET	__REGW(0xc0000924) /* Display Clk Gen */
#define IMAGCSET	__REGW(0xc0000926) /* Image Pixel Clk Gen */
#define URT0CSET	__REGW(0xc0000928) /* UART 0/1 Clk Gen */
#define UAR1CSET	__REGW(0xc000092a) /* UART 2/3 Clk Gen */
#define A940TMODE	__REGW(0xc000092c) /* ARM940T CPU Power Manage Mode */


/* Audio & Storage Clock */
/*
#define CDROMCLK	(1 << 13)
#define IDECLK		(1 << 12)
#define MSTICKCLK	(1 << 9)
#define MMCCLK		(1 << 8)
#define SPDIFOCLK	(1 << 3)
#define SPDIFICLK	(1 << 2)
#define I2SCLK		(1 << 1)
#define AC97CLK		(1 << 0)
*/
/*
 * Interrupt
 */
#define SRCPEND			__REG(0xc0000800)
#define INTMOD			__REG(0xc0000804)
#define INTMASK			__REG(0xc0000808)
#define IPRIORITY		__REG(0xc000080c)
#define INTPEND			__REG(0xc0000810)
#define INTOFFSET		__REG(0xc0000814)

/*
 * DMA
 */
#define DMAINT			__REGW(0xc0000000)

/*
 * UART
 */

#define ULCON0			__REGW(0xc0001200)
#define UCON0				__REGW(0xc0001202)
#define UFCON0			__REGW(0xc0001204)
#define UMCON0			__REGW(0xc0001206)
#define UTRSTAT0		__REGW(0xc0001208)
#define UERRSTAT0		__REGW(0xc000120a)
#define UFIFOSTAT0	__REGW(0xc000120c)
#define UMODEMSTAT0	__REGW(0xc000120e)
#define UTHB0				__REGW(0xc0001210)
#define URHB0				__REGW(0xc0001212)
#define UBRD0				__REGW(0xc0001214)
#define UTIMEOUTREG0		__REGW(0xc0001216)

#define ULCON1			__REGW(0xc0001220)
#define UCON1				__REGW(0xc0001222)
#define UFCON1			__REGW(0xc0001224)
#define UMCON1			__REGW(0xc0001226)
#define UTRSTAT1		__REGW(0xc0001228)
#define UERRSTAT1		__REGW(0xc000122a)
#define UFIFOSTAT1	__REGW(0xc000122c)
#define UMODEMSTAT1	__REGW(0xc000122e)
#define UTHB1				__REGW(0xc0001230)
#define URHB1				__REGW(0xc0001232)
#define UBRD1				__REGW(0xc0001234)
#define UTIMEOUTREG1		__REGW(0xc0001236)

#define ULCON2			__REGW(0xc0001240)
#define UCON2				__REGW(0xc0001242)
#define UFCON2			__REGW(0xc0001244)
#define UMCON2			__REGW(0xc0001246)
#define UTRSTAT2		__REGW(0xc0001248)
#define UERRSTAT2		__REGW(0xc000124a)
#define UFIFOSTAT2		__REGW(0xc000124c)
#define UMODEMSTAT2		__REGW(0xc000124e)
#define UTHB2				__REGW(0xc0001250)
#define URHB2				__REGW(0xc0001252)
#define UBRD2				__REGW(0xc0001254)
#define UTIMEOUTREG2		__REGW(0xc0001256)

#define ULCON3			__REGW(0xc0001260)
#define UCON3				__REGW(0xc0001262)
#define UFCON3			__REGW(0xc0001264)
#define UMCON3			__REGW(0xc0001266)
#define UTRSTAT3		__REGW(0xc0001268)
#define UERRSTAT3		__REGW(0xc000126a)
#define UFIFOSTAT3		__REGW(0xc000126c)
#define UMODEMSTAT3		__REGW(0xc000126e)
#define UTHB3				__REGW(0xc0001270)
#define URHB3				__REGW(0xc0001272)
#define UBRD3				__REGW(0xc0001274)
#define UTIMEOUTREG3		__REGW(0xc0001276)

#define UINTSTAT		__REGW(0xc0001280)
#define UPORTCON		__REGW(0xc0001282)

#define UTRSTAT_TX_EMPTY	(1 << 2)
#define UTRSTAT_RX_READY	(1 << 0)
#define UART_ERR_MASK		0xf

#define ULCON_IR		(1 << 6)	/* set to IrDA mode */
#define BF_ULCON_PAR		Fld(3, 3)
#define ULCON_PAR		FMsk(BF_ULCON_PAR)
#define ULCON_PAR_NONE		FInsrt(0x0, BF_ULCON_PAR)
#define ULCON_PAR_ODD		FInsrt(0x4, BF_ULCON_PAR)
#define ULCON_PAR_EVEN		FInsrt(0x5, BF_ULCON_PAR)
#define ULCON_PAR_FC1		FInsrt(0x6, BF_ULCON_PAR)
#define ULCON_PAR_FC2		FInsrt(0x7, BF_ULCON_PAR)
#define ULCON_STOP		(1 << 2)
#define ULCON_ONE_STOP		(0 << 2)
#define ULCON_TWO_STOP		(1 << 2)
#define BF_ULCON_WL		Fld(2, 0)
#define ULCON_WL		FMsk(BF_ULCON_WL)
#define ULCON_WL5		FInsrt(0x0, BF_ULCON_WL)
#define ULCON_WL6		FInsrt(0x1, BF_ULCON_WL)
#define ULCON_WL7		FInsrt(0x2, BF_ULCON_WL)
#define ULCON_WL8		FInsrt(0x3, BF_ULCON_WL)
#define ULCON_CFGMASK		(ULCON_IR | ULCON_PAR | ULCON_WL)

#define UCON_TX_INT_TYPE	(1 << 9) /* TX Interrupt request type */
#define UCON_TX_INT_PLS		(0 << 9) /* Pulse */
#define UCON_TX_INT_LVL		(1 << 9) /* Level */
#define UCON_RX_INT_TYPE	(1 << 8) /* RX Interrupt request type */
#define UCON_RX_INT_PLS		(0 << 8) /* Pulse */
#define UCON_RX_INT_LVL		(1 << 8) /* Level */
#define UCON_RX_TIMEOUT		(1 << 7) /* RX timeout enable */
#define UCON_RX_ERR_INT		(1 << 6) /* RX error status interrupt enable */
#define UCON_LOOPBACK		(1 << 5)
#define UCON_BRK_SIG		(1 << 4)
#define BF_UCON_TX		Fld(2,2)
#define UCON_TX			FMsk(BF_UCON_TX)
#define UCON_TX_DIS		FInsrt(0x0, BF_UCON_TX)
#define UCON_TX_INT		FInsrt(0x1, BF_UCON_TX)
#define UCON_TX_DMA		FInsrt(0x2, BF_UCON_TX)
#define BF_UCON_RX		Fld(2,0)
#define UCON_RX			FMsk(BF_UCON_RX)
#define UCON_RX_DIS		FInsrt(0x0, BF_UCON_RX)
#define UCON_RX_INT		FInsrt(0x1, BF_UCON_RX)
#define UCON_RX_DMA		FInsrt(0x2, BF_UCON_RX)

#define BF_UFCON_TX_TR	Fld(2,6)	/* trigger level of transmit FIFO */
#define UFCON_TX_TR	FMsk(BF_UFCON_TX_TR)
#define UFCON_TX_TR0	FInsrt(0x0, BF_UFCON_TX_TR)	/* Empty */
#define UFCON_TX_TR4	FInsrt(0x1, BF_UFCON_TX_TR)	/* 4-byte */
#define UFCON_TX_TR8	FInsrt(0x2, BF_UFCON_TX_TR)	/* 8-byte */
#define UFCON_TX_TR12	FInsrt(0x3, BF_UFCON_TX_TR)	/* 12-byte */
#define UFCON_TX_REQ	(1 << 2)	/* auto-cleared after resetting FIFO */
#define UFCON_RX_REQ	(1 << 1)	/* auto-cleared after resetting FIFO */
#define UFCON_TX_CLR	(1 << 2)	/* auto-cleared after resetting FIFO */
#define UFCON_RX_CLR	(1 << 1)	/* auto-cleared after resetting FIFO */
#define UFCON_FIFO_EN	(1 << 0)	/* FIFO Enable */
#define BF_UFCON_RX_TR	Fld(2,4)	/* trigger level of receive FIFO */
#define UFCON_RX_TR	FMsk(BF_fUFCON_RX_TR)
#define UFCON_RX_TR0	FInsrt(0x0, BF_UFCON_RX_TR)	/* 0-byte */
#define UFCON_RX_TR4	FInsrt(0x1, BF_UFCON_RX_TR)	/* 4-byte */
#define UFCON_RX_TR8	FInsrt(0x2, BF_UFCON_RX_TR)	/* 8-byte */
#define UFCON_RX_TR12	FInsrt(0x3, BF_UFCON_RX_TR)	/* 12-byte */

#define UTRSTAT_TR_EMP	(1 << 2)
#define UTRSTAT_TX_EMP	(1 << 1)
#define UTRSTAT_RX_RDY	(1 << 0)

#define UERSTAT_BRK		(1 << 3)	/* Break receive */
#define UERSTAT_FRAME	(1 << 2)	/* Frame Error */
#define UERSTAT_PARITY	(1 << 1)	/* Parity Error */
#define UERSTAT_OVERRUN	(1 << 0)	/* Overrun Error */

#define UFSTAT_RX_ERR		(1 << 10)
#define UFSTAT_TX_FULL		(1 << 9)
#define UFSTAT_RX_FULL		(1 << 8)
#define BF_UFSTAT_TX_CNT	Fld(4,4)
#define UFSTAT_TX_CNT		FMsk(BF_UFSTAT_TX_CNT)
#define BF_UFSTAT_RX_CNT	Fld(4,0)
#define UFSTAT_RX_CNT		FMsk(BF_UFSTAT_RX_CNT)

/*
 * Timer / Watch-dog
 */
#define TCOUNT			__REG(0xc0000a00)
#define TMATCH0			__REG(0xc0000a04)
#define TMATCH1			__REG(0xc0000a08)
#define TMATCH2			__REG(0xc0000a0c)
#define TMATCH3			__REG(0xc0000a10)
#define TCONTROL		__REGW(0xc0000a14)
#define TSTATUS			__REGW(0xc0000a16)
#define TINTEN			__REGW(0xc0000a18)

#define TCNT0			(1 << 0)
#define TCNT1			(1 << 1)
#define TCNT2			(1 << 2)
#define TCNT3			(1 << 3)

#define TIMER_EN		(1 << 0)
#define WDT_EN			(1 << 1)

/*
 * Real Time Clock (RTC)
 */
#define RTCTSET			__REG(0xc0000c00)
#define RTCTCNT			__REG(0xc0000c04)
#define RTCSTCNT		__REGW(0xc0000c08)
#define TICKSET			__REGW(0xc0000c0a)
#define ALARMT			__REGW(0xc0000c0c)
#define PWRMGR			__REGW(0xc0000c10)
#define CLKMGR			__REGW(0xc0000c12)
#define RSTCTRL			__REGW(0xc0000c14)
#define RSTST				__REGW(0xc0000c16)
#define BOOTCTRL		__REGW(0xc0000c18)
#define LOCKTIME		__REGW(0xc0000c1a)
#define RSTTIME			__REGW(0xc0000c1c)
#define EXTCTRL			__REGW(0xc0000c1e)
#define STOPTSET		__REGW(0xc0000c20)
#define RTCCTRL			__REGW(0xc0000c22)
#define RTSTRL			__REGW(0xc0000c24)

#define RST_WDR		(1 << 8)	/* Watchdog Reset */
#define RST_GPR		(1 << 7)	/* GPIO Reset */
#define RST_BTR		(1 << 5)	/* Battery Fault Reset */
#define RST_E1R		(1 << 4)	/* RTC Externl1 Reset */
#define RST_E0R		(1 << 3)	/* RTC Extern 0 Reset */
#define RST_ARR		(1 << 2)	/* RTC Alarm Reset */
#define RST_SWR		(1 << 1)	/* RTC Stop Watch Reset */
#define RST_HWR		(1 << 0)	/* Harware Reset */

#define RTC_LSE		(1 << 15)	/* Enable Leve shifter */
#define RTC_T1E		(1 << 14)	/* Enable quick test mode 1 irq */
#define RTC_T0E		(1 << 13)	/* Enable quick test mode 0 irq */
#define RTC_TAE		(1 << 9)	/* Enable Time TICK All value */
#define RTC_TIE		(1 << 8)	/* Enable Time TICK interrupt */
#define RTC_GIE		(1 << 7)	/* Enable GPIO Input interrupt */
#define RTC_BFE		(1 << 5)	/* Enable Battery falut interrupt */
#define RTC_E1E		(1 << 4)	/* Enable External Pin 1 interrupt */
#define RTC_E0E		(1 << 3)	/* Enable External Pin 0 interrupt */
#define RTC_ALE		(1 << 2)	/* Enable RTC ALARM interrupt */
#define RTC_STE		(1 << 1)	/* Enable RTC STOP watch interrupt */
#define RTC_1HE		(1 << 0)	/* Enable 1Hz interrupt */

/*
 * I2C
 */
#define IICCON		__REGB(0xc0000d00)
  #define I2C_ACK_ENB		(1 << 7)		// 0:disable ack generation, 1:enable ack generation
  #define I2C_TX_CLK_SRC	(1 << 6)	// Tx clock source selection bit
  										// 0 = PCLK/16, 1 = PCLK/512
  #define I2C_TXRXINT_ENB	(1 << 5)	// Tx/Rx Interrupt enable/disable bit
  										// 0 = disable interrupt, 1 = enable interrupt
  #define I2C_INTPEND_FLAG	(1 << 4)	// Interrupt Pending Flag
  										// 0 = Read - No interrupt pending
  										//     Write - Clear pending condition and resume the operation
  										// 1 = Read - Interrupt is pending
  										//     Write - N/A
  #define BF_I2C_TRNS_CLK_VAL	Fld(4,0)
  #define I2C_TRNS_CLK_VAL	FMsk(BF_I2C_TRNS_CLK_VAL)	// Tx Clock prescaler
//------------------------------------------------------------------------------------------------------
#define IICSTAT		__REGB(0xc0000d02)
  #define BF_I2C_MODE_SEL	Fld(2,6)
  #define I2C_MODE_SEL		FMsk(BF_I2C_MODE_SEL)	// I2C-bus master/slave Tx/Rx mode select bits
    #define I2C_SLAVE_RX	FInsrt(0x0, BF_I2C_MODE_SEL)	// 00: Slave receive mode
    #define I2C_SLAVE_TX	FInsrt(0x1, BF_I2C_MODE_SEL)	// 01: Slave transmit mode
    #define I2C_MASTER_RX	FInsrt(0x2, BF_I2C_MODE_SEL)	// 10: Master receive mode
    #define I2C_MASTER_TX	FInsrt(0x3, BF_I2C_MODE_SEL)	// 11: Master transmit mode
  #define I2C_STARTSTOP		(1 << 5)		// 0:STOP signal generation, 1:START signal generation (Write)
  #define I2C_BUSBUSY		(1 << 5)		// 0:I2C-bus not busy, 1:I2C-bus busy
  #define I2C_OUT_ENB		(1 << 4)		// I2C-bus data output enable/disable
  #define I2C_ARBIT_STAT_FLAG	(1 << 3)	// I2C-bus arbitration procedure state flag bit
  											// 0:bus arbitration successful, 1:bus arbitration failed
  #define I2C_ADDR_SLV_FLAG	(1 << 2)		// I2C-bus address-as-slave status flag bit
  #define I2C_ADDR_ZERO_FLAG	(1 << 1)	// I2C-bus address zero status flag bit
  #define I2C_LRB_STAT		(1 << 0)		// I2C-bus last-received bit status flag bit
//------------------------------------------------------------------------------------------------------
#define IICADD		__REGB(0xc0000d04)
#define IICDS			__REGB(0xc0000d06)

/*
 * AC97
 */
#define AC_CTL		__REGW(0xC0000E00)	/* Control Register */
#define AC_CTL_NORM_OP		(1 << 2)	/* 0:cold reset, 1:normal op */
#define AC_CTL_WARM_RST		(1 << 1)
#define AC_CTL_ACLINK_OFF	(1 << 0)

#define AC_CONFIG	__REGW(0xC0000E02)	/* Config Register */
#define AC_STA_EN	__REGW(0xC0000E04)	/* Status Enable Register */
#define AC_STA_REDAY	(1 << 7)
#define AC_STA_WD		(1 << 6)
#define AC_STA_RD		(1 << 5)
#define AC_STA_FOUT_UF	(1 << 4)
#define AC_STA_ROUT_UF	(1 << 3)
#define AC_STA_PCMIN_OF	(1 << 2)
#define AC_STA_MICIN_OF	(1 << 1)
#define AC_STA_CLFE_UF	(1 << 0)

#define AC_GSR		__REGW(0xC0000E06)	/* Global Status Register */
#define AC_GSR_REDAY	(1 << 7)
#define AC_GSR_WD		(1 << 6)
#define AC_GSR_RD		(1 << 5)
#define AC_GSR_FOUT_UF	(1 << 4)
#define AC_GSR_ROUT_UF	(1 << 3)
#define AC_GSR_PCMIN_OF	(1 << 2)
#define AC_GSR_MICIN_OF	(1 << 1)
#define AC_GSR_CLFE_UF	(1 << 0)

#define AC_ST_MCH	__REGW(0xC0000E08)	/* State Machine */

#define AC_ADDR		__REGW(0xC0000E0C)	/* Codec Address Register */
#define AC_DATA		__REGW(0xC0000E0E)	/* Codec Read Data Register */
#define AC_CAR		__REGW(0xC0000E10)	/* Codec Access Register */
#define AC_CAR_CAIP		(1 << 0)		/* Codec Access In Progress */

#define AC_REG_BASE	__REG(0xC0000F00)	/* AC97 Codec Register Base */


/* USB Device */
#define FUNC_ADDR_REG		__REGW(0xc0001400)
#define PWR_REG				__REGW(0xc0001402)
#define EP_INT_REG			__REGW(0xc0001404)
#define USB_INT_REG			__REGW(0xc000140C)
#define EP_INT_EN_REG		__REGW(0xc000140E)
#define USB_INT_EN_REG		__REGW(0xc0001416)
#define FRAME_NUM1_REG		__REGW(0xc0001418)
#define FRAME_NUM2_REG		__REGW(0xc000141A)
#define INDEX_REG			__REGW(0xc000141C)
#define EP0_FIFO_REG		__REGW(0xc0001440)
#define EP1_FIFO_REG		__REGW(0xc0001442)
#define EP2_FIFO_REG		__REGW(0xc0001444)
#define EP3_FIFO_REG		__REGW(0xc0001446)
#define EP4_FIFO_REG		__REGW(0xc0001448)
#define EP1_DMA_CON			__REGW(0xc0001460)
#define EP1_DMA_FIFO		__REGW(0xc0001464)
#define EP1_DMA_TTC_L		__REGW(0xc0001466)
#define EP1_DMA_TTC_M		__REGW(0xc0001468)
#define EP1_DMA_TTC_H		__REGW(0xc000146A)
#define EP2_DMA_CON			__REGW(0xc000146C)
#define EP2_DMA_FIFO		__REGW(0xc0001470)
#define EP2_DMA_TTC_L		__REGW(0xc0001472)
#define EP2_DMA_TTC_M		__REGW(0xc0001474)
#define EP2_DMA_TTC_H		__REGW(0xc0001476)
#define EP3_DMA_CON			__REGW(0xc0001480)
#define EP3_DMA_FIFO		__REGW(0xc0001484)
#define EP3_DMA_TTC_L		__REGW(0xc0001486)
#define EP3_DMA_TTC_M		__REGW(0xc0001488)
#define EP3_DMA_TTC_H		__REGW(0xc000148A)
#define EP4_DMA_CON			__REGW(0xc000148C)
#define EP4_DMA_FIFO		__REGW(0xc0001490)
#define EP4_DMA_TTC_L		__REGW(0xc0001492)
#define EP4_DMA_TTC_M		__REGW(0xc0001494)
#define EP4_DMA_TTC_H		__REGW(0xc0001496)
#define MAXP_REG			__REGW(0xc0001420)
#define OUT_MAXP_REG		__REGW(0xc0001426)
#define EP0_CSR				__REGW(0xc0001422)
#define IN_CSR1_REG			__REGW(0xc0001422)
#define IN_CSR2_REG			__REGW(0xc0001424)
#define OUT_CSR1_REG		__REGW(0xc0001428)
#define OUT_CSR2_REG		__REGW(0xc000142A)
#define OUT_FIFO_CNT1_REG	__REGW(0xc000142C)
#define OUT_FIFO_CNT2_REG	__REGW(0xc000142E)


/* ADC/TP */
#define TPC_ADCCON			__REGL(0xc0004600)
#define TPC_ADCDAT			__REGL(0xc0004604)
#define TPC_CNTL			__REGW(0xc0004640)
#define AD_COV_EN			(1 << 15)		// AD Converter Enable
#define TPC_PAD_EN			(1 << 14)		// Touch Pad Enable
#define TPC_AUTO_MODE		(1 << 13)		// AUTO mode enable
	#define TPC_U1_CH_EN	(1 << 12)
	#define TPC_U2_CH_EN	(1 << 11)
	#define TPC_XY_CH_EN	(1 << 10)
	#define TPC_AZ_CH_EN	(1 << 9)
  #define COMPARE_PD		(1 << 6)		// Comparator Power-Down Mode
  #define TPC_DET_EN		(1 << 5)		// Touch Detection Enable Mode
  #define TPC_FIFO_CLR	(1 << 4)		// FIFO Clear
  #define TPC_TOUCHED		(1 << 3)    // Touch Status (Read Only)
  #define TPC_FIFO_FULL	(1 << 2)		// FIFO Full Status (Read Only)
  #define TPC_FIFO_EMPTY	(1 << 1)		// FIFO Empty Status (Read Only)
  #define ADC_STBY			(1 << 0)		// ADC Status (Read Only)
//-----------------------------------------------------------------------------------------
#define TPC_INTR			__REGW(0xc0004644)
  #define TPC_U1_CMP_INT_EN		0x1000
  #define TPC_U2_CMP_INT_EN		0x0800
  #define TPC_PEN_DWN_INT_EN	0x0400
  #define TPC_PEN_CMP_INT_EN	0x0200
  #define TPC_PEN_DAT_INT_EN	0x0100
	#define TPC_U1_CMP_INT_FLAG		0x10
	#define TPC_U2_CMP_INT_FLAG		0x08
	#define TPC_PEN_DWN_INT_FLAG	0x04
	#define TPC_PEN_CMP_INT_FLAG	0x02
	#define TPC_PEN_DAT_INT_FLAG	0x01
//------------------------------------------------------------------------------------------
#define TPC_COMP_TP			__REGW(0xc0004648)
#define TPC_COMP_U1			__REGW(0xc000464c)
#define TPC_COMP_U2			__REGW(0xc0004650)
#define TPC_CLK_CNTL		__REGW(0xc0004654)
#define TPC_CH_SEL			__REGW(0xc0004658)
#define TPC_TIME_PARM1		__REGW(0xc000465c)
#define TPC_TIME_PARM2		__REGW(0xc0004660)
#define TPC_TIME_PARM3		__REGW(0xc0004664)
#define TPC_X_VALUE			__REGW(0xc0004668)
#define TPC_Y_VALUE			__REGW(0xc000466c)
#define TPC_AZ_VALUE		__REGW(0xc0004670)
#define TPC_U1_VALUE		__REGW(0xc0004674)
#define TPC_U2_VALUE		__REGW(0xc0004678)

#define TPC_ADC_CH_0	0
#define TPC_ADC_CH_1	1
#define TPC_ADC_CH_2	2
#define TPC_ADC_CH_3	3
#define TPC_ADC_CH_4	4
#define TPC_ADC_CH_5	5

#define TPC_XP_SW	0x01
#define TPC_YP_SW	0x02
#define TPC_XM_SW	0x04
#define TPC_YM_SW	0x08
#define TPC_PU_SW	0x10

#define TPC_GPIO_YM_ON		1
#define TPC_GPIO_YM_OFF		0
#define TPC_GPIO_nYP_ON		0
#define TPC_GPIO_nYP_OFF	1
#define TPC_GPIO_XM_ON		1
#define TPC_GPIO_XM_OFF		0
#define TPC_GPIO_nXP_ON		0
#define TPC_GPIO_nXP_OFF	1


/* Dual CPU Interface: mmsp20_type.h */
#define D920DATA(x)	__REGW((0xC0003B00UL + (unsigned long)(x<<1)))
#define D940DATA(x)	__REGW((0xC0003B20UL + (unsigned long)(x<<1)))
#define DINT920		__REGW(0xC0003B40)
#define DINT940		__REGW(0xC0003B42)
#define DPEND920		__REGW(0xC0003B44)
#define DPEND940		__REGW(0xC0003B46)
#define DCTRL940		__REGW(0xC0003B48)


/* FDC: mmsp20_type.h */
#define DFDC_CNTL		__REGW(0xC0001838)
#define DFDC_FRAME_SIZE	__REGW(0xC000183A)
#define DFDC_LUMA_OFFSET	__REGW(0xC000183C)
#define DFDC_CB_OFFSET	__REGW(0xC000183E)
#define DFDC_CR_OFFSET	__REGW(0xC0001840)
#define DFDC_DST_BASE_L	__REGW(0xC0001842)
#define DFDC_DST_BASE_H	__REGW(0xC0001844)
#define DFDC_STATUS		__REGW(0xC0001846)
#define DFDC_DERING		__REGW(0xC0001848)
#define DFDC_OCC_CNTL	__REGW(0xC000184A)

/*
 * Chapter 11
 * General Purpose I/O (GPIO)
 */
#define GPIOAALTFNLOW	__REGW(0xc0001020)
#define GPIOBALTFNLOW	__REGW(0xc0001022)
#define GPIOCALTFNLOW	__REGW(0xc0001024)
#define GPIODALTFNLOW	__REGW(0xc0001026)
#define GPIOEALTFNLOW	__REGW(0xc0001028)
#define GPIOFALTFNLOW	__REGW(0xc000102a)
#define GPIOGALTFNLOW	__REGW(0xc000102c)
#define GPIOHALTFNLOW	__REGW(0xc000102e)
#define GPIOIALTFNLOW	__REGW(0xc0001030)
#define GPIOJALTFNLOW	__REGW(0xc0001032)
#define GPIOKALTFNLOW	__REGW(0xc0001034)
#define GPIOLALTFNLOW	__REGW(0xc0001036)
#define GPIOMALTFNLOW	__REGW(0xc0001038)
#define GPIONALTFNLOW	__REGW(0xc000103a)
#define GPIOOALTFNLOW	__REGW(0xc000103c)

#define GPIOAALTFNHI	__REGW(0xc0001040)
#define GPIOBALTFNHI	__REGW(0xc0001042)
#define GPIOCALTFNHI	__REGW(0xc0001044)
#define GPIODALTFNHI	__REGW(0xc0001046)
#define GPIOEALTFNHI	__REGW(0xc0001048)
#define GPIOFALTFNHI	__REGW(0xc000104a)
#define GPIOGALTFNHI	__REGW(0xc000104c)
#define GPIOHALTFNHI	__REGW(0xc000104e)
#define GPIOIALTFNHI	__REGW(0xc0001050)
#define GPIOJALTFNHI	__REGW(0xc0001052)
#define GPIOKALTFNHI	__REGW(0xc0001054)
#define GPIOLALTFNHI	__REGW(0xc0001056)
#define GPIOMALTFNHI	__REGW(0xc0001058)
#define GPIONALTFNHI	__REGW(0xc000105a)
#define GPIOOALTFNHI	__REGW(0xc000105c)

/* CPLD SHADOW */
#define	S0_STSR		__REGW(PERI_VREG_BASE + 0x00)	// CF Socket0 Staus Reg
#define S0_CONR		__REGW(PERI_VREG_BASE + 0x02)	// CF Socket0 Control Reg
#define S1_STSR		__REGW(PERI_VREG_BASE + 0x04)	// CF Socket1 Status Reg
#define S1_CONR		__REGW(PERI_VREG_BASE + 0x06)	// CF Socket1 Control Reg
#define SD_MDR		__REGW(PERI_VREG_BASE + 0x0e)	// SD Mode Reg

#define	CDINTENR	__REGW(INTR_VREG_BASE + 0x00)	// Card Detect INT Enable Reg
#define	NORINTENR	__REGW(INTR_VREG_BASE + 0x02)	// Normal INT Enable Reg
#define	CDINTMDR	__REGW(INTR_VREG_BASE + 0x04)	// Card Detect INT Mode Reg
#define	NORINTMDR	__REGW(INTR_VREG_BASE + 0x06)	// Normal INT Mode Reg
#define	CDINTPNR	__REGW(INTR_VREG_BASE + 0x08)	// Card Detect INT Pending Reg
#define	NORINTPNR	__REGW(INTR_VREG_BASE + 0x0a)	// Normal INT Pending Reg

/* PCMCIA/CF S0 Status Register bit */

#define S0_VS2		1<<5
#define S0_VS1		1<<4
#define S0_nCD		1<<3
#define S0_RDY		1<<2
#define S0_BVD		1<<1
#define S0_nIOIS16	1<<0

/* PCMCIA/CF S1 Status Register bit */
#define S1_VS2		1<<5
#define S1_VS1		1<<4
#define S1_nCD		1<<3
#define S1_RDY		1<<2
#define S1_BVD		1<<1
#define S1_nIOIS16	1<<0


/* PCMCIA/CF S0 Control Register bit */
#define S0_VPPVCC_EN_H	1<<4
#define S0_VPPPGM_EN_H	1<<3
#define S0_VCC5EN_H	1<<2
#define S0_VCC3EN_H	1<<1
#define S0_RESET_H	1<<0
#define S0_VPPVCC_EN_L	~(1<<4)
#define S0_VPPPGM_EN_L	~(1<<3)
#define S0_VCC5EN_L	~(1<<2)
#define S0_VCC3EN_L	~(1<<1)
#define S0_RESET_L	~(1<<0)

/* PCMCIA/CF S1 Control Register bit */
#define S1_VPPVCC_EN_H	1<<4
#define S1_VPPPGM_EN_H	1<<3
#define S1_VCC5EN_H	1<<2
#define S1_VCC3EN_H	1<<1
#define S1_RESET_H	1<<0
#define S1_VPPVCC_EN_L	~(1<<4)
#define S1_VPPPGM_EN_L	~(1<<3)
#define S1_VCC5EN_L	~(1<<2)
#define S1_VCC3EN_L	~(1<<1)
#define S1_RESET_L	~(1<<0)

/* Card Detect Interrupt Enable Register bit */
#define SD_CD_EN		1<<2
#define PCMIA_CD_EN	1<<1
#define CF_CD_EN		1<<0
#define SD_CD_DIS	~(1<<2)
#define PCMCIA_CD_DIS	~(1<<1)
#define CF_CD_DIS	~(1<<0)

/* Normal Interrupt Enable Register bit */
#define PCMCIA_RDY_EN	1<<4
#define CF_RDY_EN	1<<3
#define PCMCIA_BVD_EN	1<<2
#define CF_BVD_EN	1<<1
#define EX_IRQ_EN 	1<<0
#define PCMCIA_RDY_DIS	~(1<<4)
#define CF_RDY_DIS	~(1<<3)
#define PCMCIA_BVD_DIS	~(1<<2)
#define CF_BVD_DIS	~(1<<1)
#define EX_IRQ_DIS 	~(1<<0)

/* Card Detect Interrupt Mode Register bit */
#define SD_CDMD_H	1<<2
#define PCMCIA_CDMD_H	1<<1
#define CF_CDMD_H	1<<0
#define SD_CDMD_L	~(1<<2)
#define PCMCIA_CDMD_L	~(1<<1)
#define CF_CDMD_L	~(1<<0)

/* Normal Interrupt Mode Register bit */
#define PCMCIA_RDYMOD_H		1<<4
#define CF_RDYMOD_H		1<<3
#define PCMCIA_BVDMOD_H		1<<2
#define CF_BVDMOD_H		1<<1
#define EX_IRQMOD_H 		1<<0
#define PCMCIA_RDYMOD_L		~(1<<4)
#define CF_RDYMOD_L		~(1<<3)
#define PCMCIA_BVDMOD_L		~(1<<2)
#define CF_BVDMOD_L		~(1<<1)
#define EX_IRQMOD_L 		~(1<<0)

/* Card Detect Interrupt Pending Register Clear bit */
#define SD_CDPN_CL	1<<2
#define PCMCIA_CDPN_CL	1<<1
#define CF_CDPN_CL	1<<0

/* Normal Interrupt Pending  Register Clear bit */
#define PCMCIA_RDYPN_CL 1<<4
#define CF_RDYPN_CL 1<<3
#define PCMCIA_BVDPN_CL 1<<2
#define CF_BVDPN_CL 1<<1
#define EX_IRQPN_CL 1<<0

/* Card Detect Interrupt Pending Register Pend bit */
#define SD_CDPN     1<<2
#define PCMCIA_CDPN 1<<1
#define CF_CDPN     1<<0

/* Normal Interrupt Pending  Register Pend bit */
#define PCMCIA_RDYPN	1<<4
#define CF_RDYPN	1<<3
#define PCMCIA_BVDPN	1<<2
#define CF_BVDPN	1<<1
#define EX_IRQ_PN	1<<0

/* SD Mode Register bit */
#define GPIO_SDCD_H	1<<1
#define GPIO_SDWP_H	1<<0
#define GPIO_SDCD_L	~(1<<1)
#define GPIO_SDWP_L	~(1<<0)

#endif /* _MMSP2_H */
