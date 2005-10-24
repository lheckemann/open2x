/*
 * vivi/include/arch-mmsp2/mmsp2-regs.h:
 *   definition of registers
 *
 * Copyright (C) 2001-2004 MIZI Research, Inc. All rights reserved.
 *
 * Author:	Janghoon Lyu
 * Created:	March 30, 2004
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
 *
 * Last modified: $Date$ 
 *                $Revision$
 */

#ifndef _MMSP2_H
#define _MMSP2_H

#include "hardware.h"
#include "bitfield.h"

/* Clock */
#define FIN			7372800
#define FCLK		200000000	//199065600
#define DCLK		(FCLK)
#define BCLK		(DCLK/2)
#define PCLK		(BCLK/2)
#define UCLK		95846400
#define ACLK		147456000
#define UART_FIN	(ACLK/10)

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

#define NAND_READY_DETECT		(1 << 15)
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

#define CLKSRC_APLL	(3)
#define CLKSRC_UPLL	(2)
#define CLKSRC_FPLL	(1)
#define CLKSRC_NONE	(0)

#define PLL_MDIV	Fld(8, 8)
#define PLL_PDIV	Fld(6, 2)
#define PLL_SDIV	Fld(2, 0)

#define PLL_DDIV	Fld(3, 6)

/* Audio & Storage Clock */
#define CDROMCLK	(1 << 13)
#define IDECLK		(1 << 12)
#define MSTICKCLK	(1 << 9)
#define MMCCLK		(1 << 8)
#define SPDIFOCLK	(1 << 3)
#define SPDIFICLK	(1 << 2)
#define I2SCLK		(1 << 1)
#define AC97CLK		(1 << 0)

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

#define GPIOAOUT	__REGW(0xc0001060)
#define GPIOBOUT	__REGW(0xc0001062)
#define GPIOCOUT	__REGW(0xc0001064)
#define GPIODOUT	__REGW(0xc0001066)
#define GPIOEOUT	__REGW(0xc0001068)
#define GPIOFOUT	__REGW(0xc000106a)
#define GPIOGOUT	__REGW(0xc000106c)
#define GPIOHOUT	__REGW(0xc000106e)
#define GPIOIOUT	__REGW(0xc0001070)
#define GPIOJOUT	__REGW(0xc0001072)
#define GPIOKOUT	__REGW(0xc0001074)
#define GPIOLOUT	__REGW(0xc0001076)
#define GPIOMOUT	__REGW(0xc0001078)
#define GPIONOUT	__REGW(0xc000107a)
#define GPIOOOUT	__REGW(0xc000107c)

#define GPIOAEVTTYPLOW	__REGW(0xc0001080)
#define GPIOBEVTTYPLOW	__REGW(0xc0001082)
#define GPIOCEVTTYPLOW	__REGW(0xc0001084)
#define GPIODEVTTYPLOW	__REGW(0xc0001086)
#define GPIOEEVTTYPLOW	__REGW(0xc0001088)
#define GPIOFEVTTYPLOW	__REGW(0xc000108a)
#define GPIOGEVTTYPLOW	__REGW(0xc000108c)
#define GPIOHEVTTYPLOW	__REGW(0xc000108e)
#define GPIOIEVTTYPLOW	__REGW(0xc0001090)
#define GPIOJEVTTYPLOW	__REGW(0xc0001092)
#define GPIOKEVTTYPLOW	__REGW(0xc0001094)
#define GPIOLEVTTYPLOW	__REGW(0xc0001096)
#define GPIOMEVTTYPLOW	__REGW(0xc0001098)
#define GPIONEVTTYPLOW	__REGW(0xc000109a)
#define GPIOOEVTTYPLOW	__REGW(0xc000109c)

#define GPIOAEVTTYPHI	__REGW(0xc00010a0)
#define GPIOBEVTTYPHI	__REGW(0xc00010a2)
#define GPIOCEVTTYPHI	__REGW(0xc00010a4)
#define GPIODEVTTYPHI	__REGW(0xc00010a6)
#define GPIOEEVTTYPHI	__REGW(0xc00010a8)
#define GPIOFEVTTYPHI	__REGW(0xc00010aa)
#define GPIOGEVTTYPHI	__REGW(0xc00010ac)
#define GPIOHEVTTYPHI	__REGW(0xc00010ae)
#define GPIOIEVTTYPHI	__REGW(0xc00010b0)
#define GPIOJEVTTYPHI	__REGW(0xc00010b2)
#define GPIOKEVTTYPHI	__REGW(0xc00010b4)
#define GPIOLEVTTYPHI	__REGW(0xc00010b6)
#define GPIOMEVTTYPHI	__REGW(0xc00010b8)
#define GPIONEVTTYPHI	__REGW(0xc00010ba)
#define GPIOOEVTTYPHI	__REGW(0xc00010bc)

#define GPIOAPUENB	__REGW(0xc00010c0)
#define GPIOBPUENB	__REGW(0xc00010c2)
#define GPIOCPUENB	__REGW(0xc00010c4)
#define GPIODPUENB	__REGW(0xc00010c6)
#define GPIOEPUENB	__REGW(0xc00010c8)
#define GPIOFPUENB	__REGW(0xc00010ca)
#define GPIOGPUENB	__REGW(0xc00010cc)
#define GPIOHPUENB	__REGW(0xc00010ce)
#define GPIOIPUENB	__REGW(0xc00010d0)
#define GPIOJPUENB	__REGW(0xc00010d2)
#define GPIOKPUENB	__REGW(0xc00010d4)
#define GPIOLPUENB	__REGW(0xc00010d6)
#define GPIOMPUENB	__REGW(0xc00010d8)
#define GPIONPUENB	__REGW(0xc00010da)
#define GPIOOPUENB	__REGW(0xc00010dc)

#define GPIOAINTENB	__REGW(0xc00010e0)
#define GPIOBINTENB	__REGW(0xc00010e2)
#define GPIOCINTENB	__REGW(0xc00010e4)
#define GPIODINTENB	__REGW(0xc00010e6)
#define GPIOEINTENB	__REGW(0xc00010e8)
#define GPIOFINTENB	__REGW(0xc00010ea)
#define GPIOGINTENB	__REGW(0xc00010ec)
#define GPIOHINTENB	__REGW(0xc00010ee)
#define GPIOIINTENB	__REGW(0xc00010f0)
#define GPIOJINTENB	__REGW(0xc00010f2)
#define GPIOKINTENB	__REGW(0xc00010f4)
#define GPIOLINTENB	__REGW(0xc00010f6)
#define GPIOMINTENB	__REGW(0xc00010f8)
#define GPIONINTENB	__REGW(0xc00010fa)
#define GPIOOINTENB	__REGW(0xc00010fc)

#define GPIOAEVT	__REGW(0xc0001100)
#define GPIOBEVT	__REGW(0xc0001102)
#define GPIOCEVT	__REGW(0xc0001104)
#define GPIODEVT	__REGW(0xc0001106)
#define GPIOEEVT	__REGW(0xc0001108)
#define GPIOFEVT	__REGW(0xc000110a)
#define GPIOGEVT	__REGW(0xc000110c)
#define GPIOHEVT	__REGW(0xc000110e)
#define GPIOIEVT	__REGW(0xc0001110)
#define GPIOJEVT	__REGW(0xc0001112)
#define GPIOKEVT	__REGW(0xc0001114)
#define GPIOLEVT	__REGW(0xc0001116)
#define GPIOMEVT	__REGW(0xc0001118)
#define GPIONEVT	__REGW(0xc000111a)
#define GPIOOEVT	__REGW(0xc000111c)

#define GPIOPADSEL	__REGW(0xc0001120)
#define GPIOGREVTSTAT	__REGW(0xc0001140)

#define GPIOAPINLVL	__REGW(0xc0001180)
#define GPIOBPINLVL	__REGW(0xc0001182)
#define GPIOCPINLVL	__REGW(0xc0001184)
#define GPIODPINLVL	__REGW(0xc0001186)
#define GPIOEPINLVL	__REGW(0xc0001188)
#define GPIOFPINLVL	__REGW(0xc000118a)
#define GPIOGPINLVL	__REGW(0xc000118c)
#define GPIOHPINLVL	__REGW(0xc000118e)
#define GPIOIPINLVL	__REGW(0xc0001190)
#define GPIOJPINLVL	__REGW(0xc0001192)
#define GPIOKPINLVL	__REGW(0xc0001194)
#define GPIOLPINLVL	__REGW(0xc0001196)
#define GPIOMPINLVL	__REGW(0xc0001198)
#define GPIONPINLVL	__REGW(0xc000119a)
#define GPIOOPINLVL	__REGW(0xc000119c)

#define GPIOEOUT_ALIVE		__REGW(0xc00011a0)
#define GPIOEOUTENB_ALIVE	__REGW(0xc00011a2)
#define GPIOEPUENB_ALIVE	__REGW(0xc00011a4)
#define GPIOFOUT_ALIVE		__REGW(0xc00011a6)
#define GPIOFOUTENB_ALIVE	__REGW(0xc00011a8)
#define GPIOFPUENB_ALIVE	__REGW(0xc00011aa)
#define GPIOGOUT_ALIVE		__REGW(0xc00011ac)
#define GPIOGOUTENB_ALIVE	__REGW(0xc00011ae)
#define GPIOGPUENB_ALIVE	__REGW(0xc00011b0)
#define GPIOIOUT_ALIVE		__REGW(0xc00011b2)
#define GPIOIOUTENB_ALIVE	__REGW(0xc00011b4)
#define GPIOIPUENB_ALIVE	__REGW(0xc00011b6)
#define GPIOJOUT_ALIVE		__REGW(0xc00011b8)
#define GPIOJOUTENB_ALIVE	__REGW(0xc00011ba)
#define GPIOJPUENB_ALIVE	__REGW(0xc00011bc)

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
 * UART
 */
#define ULCON0			__REGW(0xc0001200)
#define UCON0			__REGW(0xc0001202)
#define UFCON0			__REGW(0xc0001204)
#define UMCON0			__REGW(0xc0001206)
#define UTRSTAT0		__REGW(0xc0001208)
#define UERRSTAT0		__REGW(0xc000120a)
#define UFIFOSTAT0		__REGW(0xc000120c)
#define UMODEMSTAT0		__REGW(0xc000120e)
#define UTHB0			__REGW(0xc0001210)
#define URHB0			__REGW(0xc0001212)
#define UBRD0			__REGW(0xc0001214)
#define UTIMEOUTREG0		__REGW(0xc0001216)

#define ULCON1			__REGW(0xc0001220)
#define UCON1			__REGW(0xc0001222)
#define UFCON1			__REGW(0xc0001224)
#define UMCON1			__REGW(0xc0001226)
#define UTRSTAT1		__REGW(0xc0001228)
#define UERRSTAT1		__REGW(0xc000122a)
#define UFIFOSTAT1		__REGW(0xc000122c)
#define UMODEMSTAT1		__REGW(0xc000122e)
#define UTHB1			__REGW(0xc0001230)
#define URHB1			__REGW(0xc0001232)
#define UBRD1			__REGW(0xc0001234)
#define UTIMEOUTREG1		__REGW(0xc0001236)

#define ULCON2			__REGW(0xc0001240)
#define UCON2			__REGW(0xc0001242)
#define UFCON2			__REGW(0xc0001244)
#define UMCON2			__REGW(0xc0001246)
#define UTRSTAT2		__REGW(0xc0001248)
#define UERRSTAT2		__REGW(0xc000124a)
#define UFIFOSTAT2		__REGW(0xc000124c)
#define UMODEMSTAT2		__REGW(0xc000124e)
#define UTHB2			__REGW(0xc0001250)
#define URHB2			__REGW(0xc0001252)
#define UBRD2			__REGW(0xc0001254)
#define UTIMEOUTREG2		__REGW(0xc0001256)

#define ULCON3			__REGW(0xc0001260)
#define UCON3			__REGW(0xc0001262)
#define UFCON3			__REGW(0xc0001264)
#define UMCON3			__REGW(0xc0001266)
#define UTRSTAT3		__REGW(0xc0001268)
#define UERRSTAT3		__REGW(0xc000126a)
#define UFIFOSTAT3		__REGW(0xc000126c)
#define UMODEMSTAT3		__REGW(0xc000126e)
#define UTHB3			__REGW(0xc0001270)
#define URHB3			__REGW(0xc0001272)
#define UBRD3			__REGW(0xc0001274)
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
#define BF_UCON_TX	Fld(2,2)	
#define UCON_TX		FMsk(BF_UCON_TX)
#define UCON_TX_DIS	FInsrt(0x0, BF_UCON_TX)
#define UCON_TX_INT	FInsrt(0x1, BF_UCON_TX)
#define UCON_TX_DMA	FInsrt(0x2, BF_UCON_TX)
#define BF_UCON_RX	Fld(2,0)	
#define UCON_RX		FMsk(BF_UCON_RX)
#define UCON_RX_DIS	FInsrt(0x0, BF_UCON_RX)
#define UCON_RX_INT	FInsrt(0x1, BF_UCON_RX)
#define UCON_RX_DMA	FInsrt(0x2, BF_UCON_RX)

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

#define UERSTAT_BRK	(1 << 3)	/* Break receive */
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
#define RSTST			__REGW(0xc0000c16)
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
#define IICCON		__REGW(0xc0000d00)
#define IICSTAT		__REGW(0xc0000d02)
#define IICADD		__REGW(0xc0000d04)
#define IICDS		__REGW(0xc0000d06)

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
#define AC_STA_WD	(1 << 6)
#define AC_STA_RD	(1 << 5)
#define AC_STA_FOUT_UF	(1 << 4)
#define AC_STA_ROUT_UF	(1 << 3)
#define AC_STA_PCMIN_OF	(1 << 2)
#define AC_STA_MICIN_OF	(1 << 1)
#define AC_STA_CLFE_UF	(1 << 0)

#define AC_GSR		__REGW(0xC0000E06)	/* Global Status Register */
#define AC_GSR_REDAY	(1 << 7)
#define AC_GSR_WD	(1 << 6)
#define AC_GSR_RD	(1 << 5)
#define AC_GSR_FOUT_UF	(1 << 4)
#define AC_GSR_ROUT_UF	(1 << 3)
#define AC_GSR_PCMIN_OF	(1 << 2)
#define AC_GSR_MICIN_OF	(1 << 1)
#define AC_GSR_CLFE_UF	(1 << 0)

#define AC_ST_MCH	__REGW(0xC0000E08)	/* State Machine */

#define AC_ADDR		__REGW(0xC0000E0C)	/* Codec Address Register */
#define AC_DATA		__REGW(0xC0000E0E)	/* Codec Read Data Register */
#define AC_CAR		__REGW(0xC0000E10)	/* Codec Access Register */
#define AC_CAR_CAIP	(1 << 0)		/* Codec Access In Progress */

#define AC_REG_BASE	__REG(0xC0000F00)	/* AC97 Codec Register Base */

/*
 * Pulse width modulator
 */
#define PWMPRES01		__REGW(0xc0000b00)
#define PWMDUTY0		__REGW(0xc0000b02)
#define PWMDUTY1		__REGW(0xc0000b04)
#define PWMPERIOD0		__REGW(0xc0000b06)
#define PWMPERIOD1		__REGW(0xc0000b08)
#define PWMPRES23		__REGW(0xc0000b10)
#define PWMDUTY2		__REGW(0xc0000b12)
#define PWMDUTY3		__REGW(0xc0000b14)
#define PWMPERIOD2		__REGW(0xc0000b16)
#define PWMPERIOD3		__REGW(0xc0000b18)

/*
 * Chapter 13
 * Video Post Processor
 */
#define MLC_OVLAY_CNTR		__REGW(0xc0002880)

#define MLC_YUV_EFFECT		__REGW(0xc0002882)
#define MLC_YUV_CNTL		__REGW(0xc0002884)

#define MLC_YUVA_TP_HSC		__REGW(0xc0002886)
#define MLC_YUVA_BT_HSC		__REGW(0xc0002888)
#define MLC_YUVA_TP_VSCL	__REGW(0xc000288a)
#define MLC_YUVA_TP_VSCH	__REGW(0xc000288c)
#define MLC_YUVA_BT_VSCL	__REGW(0xc000288e)
#define MLC_YUVA_BT_VSCH	__REGW(0xc0002890)
#define MLC_YUVA_TP_PXW		__REGW(0xc0002892)
#define MLC_YUVA_BT_PXW		__REGW(0xc0002894)
#define MLC_YUVA_STX		__REGW(0xc0002896)
#define MLC_YUVA_ENDX		__REGW(0xc0002898)
#define MLC_YUVA_TP_STY		__REGW(0xc000289a)
#define MLC_YUVA_TP_ENDY	__REGW(0xc000289c)
#define MLC_YUVA_BT_ENDY	__REGW(0xc000289e)
#define MLC_YUVA_TP_OADRL	__REGW(0xc00028a0)
#define MLC_YUVA_TP_OADRH	__REGW(0xc00028a2)
#define MLC_YUVA_TP_EADRL	__REGW(0xc00028a4)
#define MLC_YUVA_TP_EADRH	__REGW(0xc00028a6)
#define MLC_YUVA_BT_OADRL	__REGW(0xc00028a8)
#define MLC_YUVA_BT_OADRH	__REGW(0xc00028aa)
#define MLC_YUVA_BT_EADRL	__REGW(0xc00028ac)
#define MLC_YUVA_BT_EADRH	__REGW(0xc00028ae)

#define MLC_YUVB_TP_HSC		__REGW(0xc00028b0)
#define MLC_YUVB_BT_HSC		__REGW(0xc00028b2)
#define MLC_YUVB_TP_VSCL	__REGW(0xc00028b4)
#define MLC_YUVB_TP_VSCH	__REGW(0xc00028b6)
#define MLC_YUVB_BT_VSCL	__REGW(0xc00028b8)
#define MLC_YUVB_BT_VSCH	__REGW(0xc00028ba)
#define MLC_YUVB_TP_PXW		__REGW(0xc00028bc)
#define MLC_YUVB_BT_PXW		__REGW(0xc00028be)
#define MLC_YUVB_STX		__REGW(0xc00028c0)
#define MLC_YUVB_ENDX		__REGW(0xc00028c2)
#define MLC_YUVB_TP_STY		__REGW(0xc00028c4)
#define MLC_YUVB_TP_ENDY	__REGW(0xc00028c6)
#define MLC_YUVB_BT_ENDY	__REGW(0xc00028c8)
#define MLC_YUVB_TP_OADRL	__REGW(0xc00028ca)
#define MLC_YUVB_TP_OADRH	__REGW(0xc00028cc)
#define MLC_YUVB_TP_EADRL	__REGW(0xc00028ce)
#define MLC_YUVB_TP_EADRH	__REGW(0xc00028d0)
#define MLC_YUVB_BT_OADRL	__REGW(0xc00028d2)
#define MLC_YUVB_BT_OADRH	__REGW(0xc00028d4)
#define MLC_YUVB_BT_EADRL	__REGW(0xc00028d6)
#define MLC_YUVB_BT_EADRH	__REGW(0xc00028d8)

#define MLC_STL_CNTL		__REGW(0xc00028da)
#define MLC_STL_MIXMUX		__REGW(0xc00028dc)
#define MLC_RGB_ALPHAL		__REGW(0xc00028de)
#define MLC_STL_ALPHAL		__REGW(0xc00028de)
#define MLC_STL_ALPHAH		__REGW(0xc00028e0)
#define MLC_STL1_STX		__REGW(0xc00028e2)
#define MLC_STL2_STX		__REGW(0xc00028e4)
#define MLC_STL3_STX		__REGW(0xc00028e6)
#define MLC_STL4_STX		__REGW(0xc00028e8)
#define MLC_STL1_ENDX		__REGW(0xc00028ea)
#define MLC_STL2_ENDX		__REGW(0xc00028ec)
#define MLC_STL3_ENDX		__REGW(0xc00028ee)
#define MLC_STL4_ENDX		__REGW(0xc00028f0)
#define MLC_STL1_STY		__REGW(0xc00028f2)
#define MLC_STL2_STY		__REGW(0xc00028f4)
#define MLC_STL3_STY		__REGW(0xc00028f6)
#define MLC_STL4_STY		__REGW(0xc00028f8)
#define MLC_STL1_ENDY		__REGW(0xc00028fa)
#define MLC_STL2_ENDY		__REGW(0xc00028fc)
#define MLC_STL3_ENDY		__REGW(0xc00028fe)
#define MLC_STL4_ENDY		__REGW(0xc0002900)
#define MLC_STL_CKEY_GR		__REGW(0xc0002902)
#define MLC_STL_CKEY_R		__REGW(0xc0002904)
#define MLC_STL_HSC		__REGW(0xc0002906)
#define MLC_STL_VSCL		__REGW(0xc0002908)
#define MLC_STL_VSCH		__REGW(0xc000290a)
#define MLC_STL_HW		__REGW(0xc000290c)
#define MLC_STL_OADRL		__REGW(0xc000290e)
#define MLC_STL_OADRH		__REGW(0xc0002910)
#define MLC_STL_EADRL		__REGW(0xc0002912)
#define MLC_STL_EADRH		__REGW(0xc0002914)

#define MLC_OSD_OADRL		__REGW(0xc0002916)
#define MLC_OSD_OADRH		__REGW(0xc0002918)
#define MLC_OSD_EADRL		__REGW(0xc000291a)
#define MLC_OSD_EADRH		__REGW(0xc000291c)

#define MLC_HWC_CNTR		__REGW(0xc000291e)
#define MLC_HWC_STX		__REGW(0xc0002920)
#define MLC_HWC_STY		__REGW(0xc0002922)
#define MLC_HWC_FGR		__REGW(0xc0002924)
#define MLC_HWC_FB		__REGW(0xc0002926)
#define MLC_HWC_BGR		__REGW(0xc0002928)
#define MLC_HWC_BB		__REGW(0xc000292a)
#define MLC_HWC_OADRL		__REGW(0xc000292c)
#define MLC_HWC_OADRH		__REGW(0xc000292e)
#define MLC_HWC_EADRL		__REGW(0xc0002930)
#define MLC_HWC_EADRH		__REGW(0xc0002932)

#define MLC_LUMA_ENH		__REGW(0xc0002934)
#define MLC_HUECB1AB		__REGW(0xc0002936)
#define MLC_HUECB2AB		__REGW(0xc0002938)
#define MLC_HUECB3AB		__REGW(0xc000293a)
#define MLC_HUECB4AB		__REGW(0xc000293c)
#define MLC_HUECR1AB		__REGW(0xc000293e)
#define MLC_HUECR2AB		__REGW(0xc0002940)
#define MLC_HUECR3AB		__REGW(0xc0002942)
#define MLC_HUECR4AB		__REGW(0xc0002944)
#define MLC_DITHER		__REGW(0xc0002946)
// reserved 0xc0002948 ~ 0xc000294a
#define MLC_OSD_PALLT_A		__REGW(0xc0002954)
#define MLC_OSD_PALLT_D		__REGW(0xc0002956)
#define MLC_STL_PALLT_A		__REGW(0xc0002958)
#define MLC_STL_PALLT_D		__REGW(0xc000295a)
#define MLC_GAMMA_A		__REGW(0xc000295c)
#define MLC_GAMMA_D		__REGW(0xc000295e)
// reserved 0xc0002960 ~ 0xc0002972
#define MLC_SPU_CTRL		__REGW(0xc0002974)
#define MLC_SPU_DELAY		__REGW(0xc0002976)
#define MLC_SPU_BASEI_LADDR	__REGW(0xc0002978)
#define MLC_SPU_BASEI_HADDR	__REGW(0xc000297a)
#define MLC_SPU_HLI_LADDR	__REGW(0xc000297c)
#define MLC_SPU_HLI_HADDR	__REGW(0xc000297e)
#define MLC_SPU_PAL		__REGW(0xc0002980)

/*
 * Chapter 14
 * Display Controller
 */
#define DPC_CNTL		__REGW(0xc0002800)
#define DPC_FPICNTL		__REGW(0xc0002802)
#define DPC_FPIPOL1		__REGW(0xc0002804)
#define DPC_FPIPOL2		__REGW(0xc0002806)
#define DPC_FPIPOL3		__REGW(0xc0002808)
#define DPC_FPIATV1		__REGW(0xc000280a)
#define DPC_FPIATV2		__REGW(0xc000280c)
#define DPC_FPIATV3		__REGW(0xc000280e)
#define DPC_X_MAX		__REGW(0xc0002816)
#define DPC_Y_MAX		__REGW(0xc0002818)
#define DPC_HS_WIDTH		__REGW(0xc000281a)
#define DPC_HS_STR		__REGW(0xc000281c)
#define DPC_HS_END		__REGW(0xc000281e)
#define DPC_V_SYNC		__REGW(0xc0002820)
#define DPC_V_END		__REGW(0xc0002822)
#define DPC_TVBNK		__REGW(0xc0002824)
#define DPC_DE			__REGW(0xc0002826)
#define DPC_PS			__REGW(0xc0002828)
#define DPC_FG			__REGW(0xc000282a)
#define DPC_LP			__REGW(0xc000282c)
#define DPC_CLKVH		__REGW(0xc000282e)
#define DPC_CLKVL		__REGW(0xc0002830)
#define DPC_POL			__REGW(0xc0002832)
#define DPC_CISSYNC		__REGW(0xc0002834)
#define DPC_MID_SYNC		__REGW(0xc0002836)
#define DPC_C_SYNC		__REGW(0xc0002838)
#define DPC_Y_BLANK		__REGW(0xc000283a)
#define DPC_C_BLANK		__REGW(0xc000283c)
#define DPC_YP_CSYNC		__REGW(0xc000283e)
#define DPC_YN_CSYNC		__REGW(0xc0002840)
#define DPC_CP_CSYNC		__REGW(0xc0002842)
#define DPC_CN_CSYNC		__REGW(0xc0002844)
#define DPC_INTR		__REGW(0xc0002846)
#define DPC_CLKCNTL		__REGW(0xc0002848)


/* USB Device */
#define FUNC_ADDR_REG		__REGW(0xc0001400)
#define PWR_REG			__REGW(0xc0001402)
#define EP_INT_REG		__REGW(0xc0001404)
#define USB_INT_REG		__REGW(0xc000140C)
#define EP_INT_EN_REG		__REGW(0xc000140E)
#define USB_INT_EN_REG		__REGW(0xc0001416)
#define FRAME_NUM1_REG		__REGW(0xc0001418)
#define FRAME_NUM2_REG		__REGW(0xc000141A)
#define INDEX_REG		__REGW(0xc000141C)
#define EP0_FIFO_REG		__REGW(0xc0001440)
#define EP1_FIFO_REG		__REGW(0xc0001442)
#define EP2_FIFO_REG		__REGW(0xc0001444)
#define EP3_FIFO_REG		__REGW(0xc0001446)
#define EP4_FIFO_REG		__REGW(0xc0001448)
#define EP1_DMA_CON		__REGW(0xc0001460)
#define EP1_DMA_FIFO		__REGW(0xc0001464)
#define EP1_DMA_TTC_L		__REGW(0xc0001466)
#define EP1_DMA_TTC_M		__REGW(0xc0001468)
#define EP1_DMA_TTC_H		__REGW(0xc000146A)
#define EP2_DMA_CON		__REGW(0xc000146C)
#define EP2_DMA_FIFO		__REGW(0xc0001470)
#define EP2_DMA_TTC_L		__REGW(0xc0001472)
#define EP2_DMA_TTC_M		__REGW(0xc0001474)
#define EP2_DMA_TTC_H		__REGW(0xc0001476)
#define EP3_DMA_CON		__REGW(0xc0001480)
#define EP3_DMA_FIFO		__REGW(0xc0001484)
#define EP3_DMA_TTC_L		__REGW(0xc0001486)
#define EP3_DMA_TTC_M		__REGW(0xc0001488)
#define EP3_DMA_TTC_H		__REGW(0xc000148A)
#define EP4_DMA_CON		__REGW(0xc000148C)
#define EP4_DMA_FIFO		__REGW(0xc0001490)
#define EP4_DMA_TTC_L		__REGW(0xc0001492)
#define EP4_DMA_TTC_M		__REGW(0xc0001494)
#define EP4_DMA_TTC_H		__REGW(0xc0001496)
#define MAXP_REG		__REGW(0xc0001420)
#define OUT_MAXP_REG		__REGW(0xc0001426)
#define EP0_CSR			__REGW(0xc0001422)
#define IN_CSR1_REG		__REGW(0xc0001422)
#define IN_CSR2_REG		__REGW(0xc0001424)
#define OUT_CSR1_REG		__REGW(0xc0001428)
#define OUT_CSR2_REG		__REGW(0xc000142A)
#define OUT_FIFO_CNT1_REG	__REGW(0xc000142C)
#define OUT_FIFO_CNT2_REG	__REGW(0xc000142E)


/* ADC/TP */
#define TPC_ADCCON		__REGL(0xc0004600)
#define TPC_ADCDAT		__REGL(0xc0004604)
#define TPC_CNTL		__REGW(0xc0004640)
#define TPC_INTR		__REGW(0xc0004644)
#define TPC_COMP_TP		__REGW(0xc0004648)
#define TPC_COMP_U1		__REGW(0xc000464c)
#define TPC_COMP_U2		__REGW(0xc0004650)
#define TPC_CLK_CNTL		__REGW(0xc0004654)
#define TPC_CH_SEL		__REGW(0xc0004658)
#define TPC_TIME_PARM1		__REGW(0xc000465c)
#define TPC_TIME_PARM2		__REGW(0xc0004660)
#define TPC_TIME_PARM3		__REGW(0xc0004664)
#define TPC_X_VALUE		__REGW(0xc0004668)
#define TPC_Y_VALUE		__REGW(0xc000466c)
#define TPC_AZ_VALUE		__REGW(0xc0004670)
#define TPC_U1_VALUE		__REGW(0xc0004674)
#define TPC_U2_VALUE		__REGW(0xc0004678)

#define TPC_XP_SW	0x01
#define TPC_YP_SW	0x02
#define TPC_XM_SW	0x04
#define TPC_YM_SW	0x08
#define TPC_PU_SW	0x10

#define TPC_ADC_CH_0	0
#define TPC_ADC_CH_1	1
#define TPC_ADC_CH_2	2
#define TPC_ADC_CH_3	3
#define TPC_ADC_CH_4	4
#define TPC_ADC_CH_5	5

#define TPC_PEN_DWN_INT_EN	0x0400
#define TPC_PEN_CMP_INT_EN	0x0200
#define TPC_PEN_DAT_INT_EN	0x0100
#define TPC_U2_CMP_INT_EN	0x0800
#define TPC_U1_CMP_INT_EN	0x1000

#define TPC_PEN_DWN_INT_FLAG	0x04
#define TPC_PEN_CMP_INT_FLAG	0x02
#define TPC_PEN_DAT_INT_FLAG	0x01
#define TPC_U2_CMP_INT_FLAG		0x08
#define TPC_U1_CMP_INT_FLAG		0x10

#define TPC_AZ_CH_EN	0x0200
#define TPC_XY_CH_EN	0x0400
#define TPC_U2_CH_EN	0x0800
#define TPC_U1_CH_EN	0x1000

#define TPC_GPIO_YM_ON		1
#define TPC_GPIO_YM_OFF		0
#define TPC_GPIO_nYP_ON		0
#define TPC_GPIO_nYP_OFF	1
#define TPC_GPIO_XM_ON		1
#define TPC_GPIO_XM_OFF		0
#define TPC_GPIO_nXP_ON		0
#define TPC_GPIO_nXP_OFF	1


/* Dual CPU Interface */
#define DUAL920DATA(x)	__REGW((0xC0003B00UL + (unsigned long)(x<<1)))
#define DUAL940DATA(x)	__REGW((0xC0003B20UL + (unsigned long)(x<<1)))
#define DUALINT920	__REGW(0xC0003B40)
#define DUALINT940	__REGW(0xC0003B42)
#define DUALPEND920	__REGW(0xC0003B44)
#define DUALPEND940	__REGW(0xC0003B46)
#define DUALCTRL940	__REGW(0xC0003B48)

/* FDC */
#define FDC_CNTL	__REGW(0xC0001838)
#define FDC_FRAME_SIZE	__REGW(0xC000183A)
#define FDC_LUMA_OFFSET	__REGW(0xC000183C)
#define FDC_CB_OFFSET	__REGW(0xC000183E)
#define FDC_CR_OFFSET	__REGW(0xC0001840)
#define FDC_DST_BASE_L	__REGW(0xC0001842)
#define FDC_DST_BASE_H	__REGW(0xC0001844)
#define FDC_STATUS	__REGW(0xC0001846)
#define FDC_DERING	__REGW(0xC0001848)
#define FDC_OCC_CNTL	__REGW(0xC000184A)

/* SCALE */
#define SC_CMD			__REGW(0xC0001800)
#define SC_STATUS		__REGW(0xC0001802)
#define SC_MIRROR		__REGW(0xC0001804)
#define SC_PRE_VRATIO	__REGW(0xC0001806)
#define SC_PRE_HRATIO	__REGW(0xC0001808)
#define SC_POST_VRATIOL	__REGW(0xC000180A)
#define SC_POST_VRATIOH	__REGW(0xC000180C)
#define SC_POST_HRATIOL	__REGW(0xC000180E)
#define SC_POST_HRATIOH	__REGW(0xC0001810)
#define SC_SRC_ODD_ADDRL	__REGW(0xC0001812)
#define SC_SRC_ODD_ADDRH	__REGW(0xC0001814)
#define SC_SRC_EVEN_ADDRL	__REGW(0xC0001816)
#define SC_SRC_EVEN_ADDRH	__REGW(0xC0001818)
#define SC_DST_ADDRL	__REGW(0xC000181A)
#define SC_DST_ADDRH	__REGW(0xC000181C)
#define SC_SRC_PXL_HEIGHT	__REGW(0xC000181E)
#define SC_SRC_PXL_WIDTH	__REGW(0xC0001820)
#define SC_SRC_PXL_REQCNT	__REGW(0xC0001822)
#define SC_DST_PXL_HEIGHT	__REGW(0xC0001824)
#define SC_DST_PXL_WIDTH	__REGW(0xC0001826)
#define SC_LUMA_OFFSET	__REGW(0xC0001828)
#define SC_CB_OFFSET	__REGW(0xC000182A)
#define SC_CR_OFFSET	__REGW(0xC000182C)
#define SC_DELAY		__REGW(0xC000182E)
#define SC_MEM_CNTR		__REGW(0xC0001830)
#define SC_DST_WPXL_WIDTH	__REGW(0xC0001832)
#define SC_IRQ			__REGW(0xC0001834)
#endif /* _MMSP2_H */
