/*
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * this file is written for MMSP20 Linux projects
 */

/************************************************
 * NAME	    : mmsp20.h
 * Version  : 08.1.2004
 * Based on MMSP20 User's manual Rev 0.13
 ************************************************/

#ifndef __MMSP20_H__
#define __MMSP20_H__

//#include <asm/hardware.h> // mmsp2 register map in kernel src

#define MMSP20_UART_CHANNELS    4 	

/* MMSP20 only supports 512 Byte HW ECC */
#define MMSP20_ECCSIZE		512
#define MMSP20_ECCBYTES		3

typedef enum {
	MMSP20_UART0,
	MMSP20_UART1,
	MMSP20_UART2,
	MMSP20_UART3
} MMSP20_UARTS_NR;

//#define IO_BASE				0xC0000000 // physical address for u-boot
#define IO_BASE				0xf0000000 // physical address for kernel == VIO_BASE, see asm/arch/hardware.h

/* MMSP20 device base addresses */ 
#define MMSP20_BANC_MEMCTL_BASE		(IO_BASE + 0x3A00)
#define MMSP20_BANA_MEMCTL_BASE		(IO_BASE + 0x3800)
#define MMSP20_DUALCPU_BASE 	    (IO_BASE + 0x3B00)
#define MMSP20_CLOCK_POWER_BASE		(IO_BASE + 0x0900)
#define MMSP20_INTERRUPT_BASE		(IO_BASE + 0x0800) 
#define MMSP20_DMA_BASE				(IO_BASE + 0x0200)
#define MMSP20_DMAREQ_BASE			(IO_BASE + 0x0100)			
#define MMSP20_UART_BASE			(IO_BASE + 0x1200)
#define MMSP20_TIMER_WATCHDOG_BASE	(IO_BASE + 0x0A00)
#define MMSP20_GPIO_BASE			(IO_BASE + 0x1020)
#define MMSP20_RTC_BASE				(IO_BASE + 0x0C00)
#define MMSP20_FDC_BASE				(IO_BASE + 0x1838)
#define MMSP20_SC_BASE              (IO_BASE + 0x1800)
#define MMSP20_MLC_BASE				(IO_BASE + 0x2880) /* multi layer controller */
#define MMSP20_DPC_BASE				(IO_BASE + 0x2800) /* diplay controller      */
#define MMSP20_PWM_BASE				(IO_BASE + 0x0B00) /* PWM controller      */
#define MMSP20_ISP_BASE				(IO_BASE + 0x3000) /* ISP */

#define MMSP20_NAND_BASE		0xfc000000 // for kernel: see asm/arch/hardware.h

/* include common stuff */
#include "mmsp20_type.h"

/*
static inline MMSP20_BANA_MEMCTL * const MMSP20_GetBaseA_MEMCTL(void)
{
	return (MMSP20_BANA_MEMCTL * const)MMSP20_BANA_MEMCTL_BASE;
}
*/
static inline MMSP20_BANC_MEMCTL * const MMSP20_GetBaseC_MEMCTL(void)
{
	return (MMSP20_BANC_MEMCTL * const)MMSP20_BANC_MEMCTL_BASE;
}
/*
static inline MMSP20_INTERRUPT * const MMSP20_GetBase_INTERRUPT(void)
{
	return (MMSP20_INTERRUPT * const)MMSP20_INTERRUPT_BASE;
}
*/
static inline MMSP20_DMAS * const MMSP20_GetBase_DMA(int channel)	// channel : 0~15
{
	return (MMSP20_DMAS * const)(MMSP20_DMA_BASE + 0x10*channel);
}
static inline MMSP20_DMAREQ * const MMSP20_GetBase_DMAREQ(void)
{
	return (MMSP20_DMAREQ * const)MMSP20_DMAREQ_BASE;
}
static inline MMSP20_DUALCPU * const MMSP20_GetBase_DualCPU(void)
{
	return (MMSP20_DUALCPU * const)MMSP20_DUALCPU_BASE;
}
static inline MMSP20_CLOCK_POWER * const MMSP20_GetBase_CLOCK_POWER(void)
{
	return (MMSP20_CLOCK_POWER * const)MMSP20_CLOCK_POWER_BASE;
}
/*
static inline MMSP20_NAND * const MMSP20_GetBase_NAND(void)
{
	return (MMSP20_NAND * const)MMSP20_NAND_BASE;
}
*/
static inline MMSP20_UART * const MMSP20_GetBase_UART(void)
{
	return (MMSP20_UART * const)MMSP20_UART_BASE;
}
/*
static inline MMSP20_TIMER_WATCHDOG * const MMSP20_GetBase_TIMERS(void)
{
	return (MMSP20_TIMER_WATCHDOG * const)MMSP20_TIMER_WATCHDOG_BASE;
}
*/
static inline MMSP20_GPIO * const MMSP20_GetBase_GPIO(void)
{
	return (MMSP20_GPIO * const)MMSP20_GPIO_BASE;
}
/*
static inline MMSP20_RTC * const MMSP20_GetBase_RTC(void)
{
	return (MMSP20_RTC * const)MMSP20_RTC_BASE;
}
*/
static inline MMSP20_FDC * const MMSP20_GetBase_FDC(void)
{
	return (MMSP20_FDC * const)MMSP20_FDC_BASE;
}

static inline MMSP20_SC * const MMSP20_GetBase_SC(void)
{
        return (MMSP20_SC * const)MMSP20_SC_BASE;
}

static inline MMSP20_MLC * const MMSP20_GetBase_MLC(void)
{
	return (MMSP20_MLC * const)MMSP20_MLC_BASE;
}

static inline MMSP20_DPC * const MMSP20_GetBase_DPC(void)
{
	return (MMSP20_DPC * const)MMSP20_DPC_BASE;
}

static inline MMSP20_PWM * const MMSP20_GetBase_PWM(void)
{
	return (MMSP20_PWM * const)MMSP20_PWM_BASE;
}

static inline MMSP20_ISP * const MMSP20_GetBase_ISP(void)
{
	return (MMSP20_ISP * const)MMSP20_ISP_BASE;
}

#define DEBUG
/*
 *  Debug macros
 */
#ifdef DEBUG
#  define dprintk(fmt, args...) printk("%s: " fmt, __FUNCTION__ , ## args)
#else
#  define dprintk(fmt, args...)
#endif

#endif /*__MMSP20_H__*/
