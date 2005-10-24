/*
 * (C) Copyright 2001-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
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

/* This code should work for both the S3C2400 and the S3C2410
 * as they seem to have the same PLL and clock machinery inside.
 * The different address mapping is handled by the s3c24xx.h files below.
 */

#include <common.h>
/*
#if defined(CONFIG_S3C2400)
#include <s3c2400.h>
#elif defined(CONFIG_S3C2410)
#include <s3c2410.h>
#elif defined(CONFIG_MMSP20)
*/
#include <mmsp20.h>
/*
#endif
*/

#if defined (CONFIG_MMSP20)
#define FPLL 0
#define UPLL 1
#define APLL 2
#else
#define MPLL 0
#define UPLL 1
#endif

/* ------------------------------------------------------------------------- */
/* NOTE: This describes the proper use of this file.
 *
 * CONFIG_SYS_CLK_FREQ should be defined as the input frequency of the PLL.
 *
 * get_FCLK(), get_HCLK(), get_PCLK() and get_UCLK() return the clock of
 * the specified bus in HZ.
 */
/* ------------------------------------------------------------------------- */

#if defined (CONFIG_MMSP20)
static ulong get_PLLCLK(int pllreg)
{
    MMSP20_CLOCK_POWER * const clk_power = MMSP20_GetBase_CLOCK_POWER();
    ulong r,m,p,s;
    ulong i;

	
    if (pllreg == FPLL)
	r = clk_power->FPLLVSETREG;   
    else if (pllreg == UPLL)
	r = clk_power->UPLLVSETREG;
    else if (pllreg == APLL)
	r = clk_power->APLLVSETREG;
    else 
	hang();

    m = ((r & 0xFF00) >> 8) + 8;   	/* m = MDIV + * */
    p = ((r & 0xFC) >> 3) + 1;		/* p = PDIV + 2 */
    s = r & 0x03;			/* s = s */

    return ((CONFIG_SYS_CLK_FREQ * m) / (p << s));
}

ulong get_FCLK(void)
{
   	MMSP20_CLOCK_POWER * const clk_power = MMSP20_GetBase_CLOCK_POWER();
    	ulong i;
	
	i = clk_power->SYSCSETREG;
	i = (i & 0x07) + 1;
	return ( get_PLLCLK(FPLL) / i );
}
ulong get_DCLK(void)
{
   	MMSP20_CLOCK_POWER * const clk_power = MMSP20_GetBase_CLOCK_POWER();
    	ulong i;
	
	i = clk_power->SYSCSETREG;
	i = ((i >> 6) & 0x07) + 1;
	return ( get_PLLCLK(FPLL) / i );
}
/* BCLK is DCLK * 1/2 */
ulong get_BCLK(void)
{
	return ( get_DCLK() / 2 );
}
/* BCLK is DCLK * 1/4 */
ulong get_PCLK(void)
{
	return (get_DCLK() / 4);
}
ulong get_UCLK(void)
{
   	MMSP20_CLOCK_POWER * const clk_power = MMSP20_GetBase_CLOCK_POWER();
    	ulong i;
	
	i = clk_power->PWMODEREG;
	i = ((i >> 7) & 0x01) ;
	return ((i) ? 0 : get_PLLCLK(UPLL));
}
ulong get_ACLK(void)
{
   	MMSP20_CLOCK_POWER * const clk_power = MMSP20_GetBase_CLOCK_POWER();
    	ulong i;
	
	i = clk_power->PWMODEREG;
	i = (i >> 8) & 0x01;
	return ((i) ? 0 : get_PLLCLK(APLL));
}
ulong get_UART1_CLK(void)
{
	MMSP20_CLOCK_POWER * const clk_power = MMSP20_GetBase_CLOCK_POWER();
	
	unsigned int clk_source = (clk_power->URTCSETREG[0] >> 14) & 0x03;
	unsigned int divide = (clk_power->URTCSETREG[0] >> 8 ) & 0x03F;

	switch (clk_source){
	   case (1):	/* FPLL 01b */
		return (get_PLLCLK(FPLL)/divide);
		break;
	   case (2):    /* UPLL 10b */
		return (get_PLLCLK(UPLL)/divide);
		break;
	   case (3):    /* APLL 11b */
		return (get_PLLCLK(APLL)/divide);
		break;
	   default :
		return 0;
		break;
	}
}

void show_MMSP2_CLK(void)
{
	MMSP20_CLOCK_POWER * const clk_power = MMSP20_GetBase_CLOCK_POWER();
	unsigned int clk_source = (clk_power->URTCSETREG[0] >> 14) & 0x03;

        printf("MP2520F    FPLLCLK: %9lu Hz\n", get_PLLCLK(FPLL) );
	printf("MP2520F    UPLLCLK: %9lu Hz\n", get_PLLCLK(UPLL) );
	printf("MP2520F    APLLCLK: %9lu Hz\n", get_PLLCLK(APLL) );

        printf("MP2520F UART 1 CLK: %9lu Hz\n", get_UART1_CLK() );

	switch (clk_source){
	   case (1):	/* FPLL 01b */
        printf("     uart 1 clock src ==> FPLL\n");
		break;
	   case (2):    /* UPLL 10b */
        printf("     uart 1 clock src ==> UPLL\n");
		break;
	   case (3):    /* APLL 11b */
        printf("     uart 1 clock src ==> APLL\n");
		break;
	   default :
        printf("     uart 1 clock src ==> ?(don't know)\n");
		break;
	}

        printf("MP2520F       FCLK: %9lu Hz\n", get_FCLK() );
        printf("MP2520F       UCLK: %9lu Hz\n", get_UCLK() );
        printf("MP2520F       ACLK: %9lu Hz\n", get_ACLK() );
        printf("MP2520F       DCLK: %9lu Hz = FCLK/n\n", get_DCLK() );
        printf("MP2520F       BCLK: %9lu Hz = DCLK/2\n", get_BCLK() );
        printf("MP2520F       PCLK: %9lu Hz = BCLK/2 = DCLK/4\n", get_PCLK() );
}

#else
static ulong get_PLLCLK(int pllreg)
{
    S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
    ulong r, m, p, s;

    if (pllreg == MPLL)
	r = clk_power->MPLLCON;
    else if (pllreg == UPLL)
	r = clk_power->UPLLCON;
    else
	hang();

    m = ((r & 0xFF000) >> 12) + 8;
    p = ((r & 0x003F0) >> 4) + 2;
    s = r & 0x3;

    return((CONFIG_SYS_CLK_FREQ * m) / (p << s));
}


/* return FCLK frequency */
ulong get_FCLK(void)
{
    return(get_PLLCLK(MPLL));
}

/* return HCLK frequency */
ulong get_HCLK(void)
{
    S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();

    return((clk_power->CLKDIVN & 0x2) ? get_FCLK()/2 : get_FCLK());
}

/* return PCLK frequency */
ulong get_PCLK(void)
{
    S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();

    return((clk_power->CLKDIVN & 0x1) ? get_HCLK()/2 : get_HCLK());
}

/* return UCLK frequency */
ulong get_UCLK(void)
{
    return(get_PLLCLK(UPLL));
}
#endif
