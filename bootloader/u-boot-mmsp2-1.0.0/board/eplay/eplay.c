/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <mmsp20.h>

/* ------------------------------------------------------------------------- */

#define A_MDIV 		0x98
#define A_PDIV 		0x0
#define A_SDIV 		0x2
#define DEBUG_SRC	0x3 /*APLL*/
#define DEBUG_DIVIDE 	9 /*DEBUG SOURCE CLOCK DIVIDE */

#define IO_IN		0x0
#define IO_OUT		0x1
#define ALTFN1		0x2
#define ALTFN2		0x3

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
	  "subs %0, %1, #1\n"
	  "bne 1b":"=r" (loops):"0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	MMSP20_CLOCK_POWER * const clk_power = MMSP20_GetBase_CLOCK_POWER();
	MMSP20_GPIO * const gpio = MMSP20_GetBase_GPIO();
	MMSP20_RTC  * const rtc = MMSP20_GetBase_RTC();

/* GPIO configuration */
	/*Set MCU BUS*/
	gpio->GPIOALTFNLOW['E'-'A'] =
	 ((ALTFN1 << 14)+(ALTFN1 << 12)+(ALTFN1 << 10)+(ALTFN1 << 8)
	+(ALTFN1 << 6)+(ALTFN1 << 4)+(ALTFN1 << 2)+(ALTFN1));
	gpio->GPIOALTFNHI['E'-'A'] =
	 ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)
         +(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNLOW['F'-'A'] =
	 ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)
         +(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNHI['F'-'A'] =
	 ((gpio->GPIOALTFNHI['F'-'A'] & ~0x3F)
          + (ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNLOW['G'-'A'] =
	 ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)
         +(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNHI['G'-'A'] =
	 ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)
         +(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	/*Set MCU Con*/
	gpio->GPIOALTFNLOW['I'-'A'] =
	 ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)
         +(IO_IN<< 6)+(IO_IN<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNHI['I'-'A'] =
	 ((gpio->GPIOALTFNHI['I'-'A'] & 0xC000)
		      +(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)
         +(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNLOW['J'-'A'] =
	 ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)
         +(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNHI['J'-'A'] =
	 ((IO_IN<<14)+(IO_OUT<<12)+(IO_OUT<<10)+(ALTFN1<<8)
         +(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	/*Set UART*/
	gpio->GPIOALTFNLOW['D'-'A'] =
	 ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)
         +(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)
	 		+(gpio->GPIOALTFNLOW['D'-'A'] & 0x3));
	gpio->GPIOALTFNHI['D'-'A'] =
	 ((gpio->GPIOALTFNHI['D'-'A'] & ~0x3F)
           +(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));

	/* Enable wait control of static region 3 for LAN91C111	*/
	*(MMSP20_REG16 *)(0xc000570E) |= 0xC0;

	/*APLL, UPLL operation*/
	rtc->CLKMGRREG = (rtc->CLKMGRREG & 0xfffC) + 0x3;

	/* configure APLL */
	clk_power->APLLSETVREG  = ((A_MDIV << 8) + (A_PDIV << 2) + A_SDIV);

	/* configure Debug port */
	clk_power->URTCSETREG[0] = ((DEBUG_SRC << 14) + (DEBUG_DIVIDE << 8));

	/* some delay between MPLL and UPLL */
	delay (4000);


	/* set up the I/O ports -> move nand bootstrap*/
	
	/* arch number of MMSP20-Board */
	gd->bd->bi_arch_number = 345;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00100100;

	icache_enable();
	dcache_enable();

	return 0;
}

int dram_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	
	return 0 ;
}
