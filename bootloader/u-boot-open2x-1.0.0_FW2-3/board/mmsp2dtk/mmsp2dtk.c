/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
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
#include "./video/gp2xlcd.h"
#include "./video/hardware.h"
#include <mmsp2dtk/sdk2x_system.h>


/*------------------------------------------------------------------------- */

/* FCLK = 199065600 */
#define F_MDIV		(0x49)
#define F_PDIV		(0x1)
#define F_SDIV		(0x0)

/* FCLK = 98304000 */
#define F_MDIV100	(0x20)
#define F_PDIV100	(0x1)
#define F_SDIV100	(0x0)

/* UCLK = 95.xxxMHz. It's default value. */
#define U_MDIV		(0x60)
#define U_PDIV		(0x0)
#define U_SDIV		(0x2)

/* ACLK = 147456000. It's default value. */
#define A_MDIV 		0x98
#define A_PDIV 		0x0
#define A_SDIV 		0x2

#define DEBUG_SRC		0x3		/*APLL*/
#define DEBUG_DIVIDE 	9	/*DEBUG SOURCE CLOCK DIVIDE */

#define IO_IN		0x0
#define IO_OUT		0x1
#define ALTFN1		0x2
#define ALTFN2		0x3

/* GPIOH */
#define LCD_VGH			(1<<1)
#define BACK_LIGHT		(1<<2)
#define AVDD_5V			(1<<3)
#define BATT_LED		(1<<4)

/* GPIOB*/
#define TV_SLEEP		(1<<2)
#define LCDTV_RESET		(1<<3)
#define LCD_VSYNC		(1<<4)
#define LCD_HSYNC		(1<<5)
#define	LCD_DE			(1<<6)
#define LCD_PCLK		(1<<7)

/* SYSTEM CLOCK ENABLE BIT(0xc0000904)   */
#define ESYSCLK			(1<<14)
#define ADCCLK			(1<<13)
#define PWMCLK			(1<<12)
#define FASTIOCLK		(1<<10)
#define DMACLK			(1<<9)
#define DMACLK_GOFF		(1<<8)		/* DISABLE BIT*/
#define TIMERCLK		(1<<7)
#define PERI_CLK		(1<<6)  	/* chcke off */
#define MEMCCLK			(1<<3)
#define MEMACLK			(1<<1)
#define A940TCLK		(1<<0)

/* COMMUNICATION DEVICE CLOCK ENABLE REGISTER (0xc000 0908) */
#define SSPCLK		(1<<9)
#define I2CCLK		(1<<8)
#define UART3CLK	(1<<7)
#define UART2CLK	(1<<6)
#define UART1CLK	(1<<5)
#define UART0CLK	(1<<4)
#define OWMCLK		(1<<3)
#define UDCCLK		(1<<1)
#define UHCCLK		(1<<0)
#define COMMNICATION_CLK_MASK 0x2FF

/* VIDEO & GRAPHICS DEVICE CLOCK ENABLE REGISTER VGCLKENREG (0xc000 090a) */
#define DBLKCLK		(1<<15)
#define MPGIFCLK	(1<<14)
#define DECSSCLK	(1<<13)
#define MECLK		(1<<12)
#define REFWCLK		(1<<11)
#define RVLDCLK		(1<<10)
#define QGMCLK		(1<<9)
#define MPMCLK		(1<<8)
#define PDFCLK		(1<<7)
#define HUFFCLK		(1<<6)
#define VPCLKGOFF	(1<<5)	/* DISABLE BIT*/
#define VPTOPCLK	(1<<4)
#define ISPCLK		(1<<3)
#define GRPCLK		(1<<2)
#define SCALECLK	(1<<1)
#define DISPCLK		(0<<1)
#define ALL_VIDEO_CLK_MASK	0xFFFF
/* AUDIO & STORAGE DEVICE CLOCK ENABLE REGISTER (0xC000 090c)*/

#define CDROMCLK	(1<<13)
#define IDECLK		(1<<12)
#define MSTICKCLK	(1<<9)
#define MMCCLK		(1<<8)
#define SPDIFOCLK	(1<<3)
#define SPDIFICLK	(1<<2)
#define I2SCLK		(1<<1)
#define AC97CLK		(0<<0)
#define AUDDIO_STORAGE_CLK_MASK 0x3fff


/* NET2272 interface timing*/
#define Wait		(0x01<<4)
#define Tacs2		(0x01<<4)
#define Tcos2		(0x01<<4)
#define Tcoh2		(0x01<<4)
#define Tcah2		(0x01<<4)
#define Tacc2		(0x01<<8)

#define BASE_USB	0x88000000
#define USBCTL0		0x18
#define LOCCTL		0x22
#define IRQSTAT1	0x03
#define USB_SUSPEND	0
#define SOUND_LOGO_PROCESS	1

#define writeb(b,addr) (*(volatile unsigned char *) (addr) = (b))

#define DEBUG_MMSP2DTK		0
#if DEBUG_MMSP2DTK
	#define DPRINTF(args...) printf(args)
#else
	#define DPRINTF(args...) { }
#endif /* DEBUG_CMD_NAND */

/****************************************************************************/
/* Hacks for sound file support */
ulong g_filesize = 0;
extern int kernel_load; // /board/mmsp2dtk/sound/sound_logo.c
/****************************************************************************/

extern int mmc_init_once;


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
	MMSP20_BANC_MEMCTL * const memctl = MMSP20_GetBaseC_MEMCTL();
	MMSP20_CLOCK_POWER * const clk_power = MMSP20_GetBase_CLOCK_POWER();
	MMSP20_GPIO * const gpio = MMSP20_GetBase_GPIO();
	MMSP20_RTC  * const rtc = MMSP20_GetBase_RTC();

	/* Configure clock and poower */
	/* configure FPLL */
	clk_power->FPLLSETVREG  = ((F_MDIV << 8) + (F_PDIV << 2) + F_SDIV);
	/* configure UPLL */
	clk_power->UPLLSETVREG  = ((U_MDIV << 8) + (U_PDIV << 2) + U_SDIV);
	/* configure APLL */
	clk_power->APLLSETVREG  = ((A_MDIV << 8) + (A_PDIV << 2) + A_SDIV);

	/*APLL, UPLL operation*/
	rtc->CLKMGRREG = (rtc->CLKMGRREG & 0xfffC) + 0x3;

#if 0
	clk_power->SYSCLKENREG &= (~PWMCLK);
//	clk_power->COMCLKENREG &= (((~COMMNICATION_CLK_MASK)|UART0CLK|I2CCLK)) & COMMNICATION_CLK_MASK;
	clk_power->VGCLKENREG  &= (~(ISPCLK|DECSSCLK)) & ALL_VIDEO_CLK_MASK;
	clk_power->ASCLKENREG  &= (~(CDROMCLK|IDECLK|MSTICKCLK|SPDIFOCLK|SPDIFICLK|I2SCLK)) &
								AUDDIO_STORAGE_CLK_MASK;
#else
	//clk_power->SYSCLKENREG &= ~(ESYSCLK|ADCCLK|PWMCLK|FASTIOCLK|MEMCCLK|A940TCLK);
	//clk_power->SYSCLKENREG &= ~(ESYSCLK|ADCCLK|PWMCLK|FASTIOCLK|A940TCLK);
	clk_power->SYSCLKENREG &= ~(ESYSCLK|PWMCLK|FASTIOCLK|A940TCLK);
	//clk_power->COMCLKENREG &= ~(SSPCLK|I2CCLK|UART3CLK|UART2CLK|UART1CLK|OWMCLK|UDCCLK|UHCCLK);
	clk_power->COMCLKENREG &= ~(SSPCLK|UART3CLK|UART2CLK|UART1CLK|OWMCLK|UDCCLK|UHCCLK);
	clk_power->VGCLKENREG  &= ~(DBLKCLK|MPGIFCLK|DECSSCLK|MECLK|REFWCLK|RVLDCLK|QGMCLK|MPMCLK|PDFCLK
								|HUFFCLK|VPCLKGOFF|VPTOPCLK|ISPCLK|GRPCLK|SCALECLK);
	clk_power->ASCLKENREG  &= ~(CDROMCLK|IDECLK|MSTICKCLK|SPDIFOCLK|SPDIFICLK|I2SCLK);
#endif

	/* configure Debug port */
	clk_power->URTCSETREG[0] = ((DEBUG_SRC << 6) + (DEBUG_DIVIDE << 0));
	/* some delay between MPLL and UPLL */
	delay (4000);

/* GPIO configuration */
	/*Set MCU BUS*/
	gpio->GPIOALTFNLOW['E'-'A'] = ((ALTFN1 << 14)+(ALTFN1 << 12)+(ALTFN1 << 10)+(ALTFN1 << 8)+(ALTFN1 << 6)+(ALTFN1 << 4)+(ALTFN1 << 2)+(ALTFN1));
	gpio->GPIOALTFNHI['E'-'A'] = ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)+(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNLOW['F'-'A'] = ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)+(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNHI['F'-'A'] = ((gpio->GPIOALTFNHI['F'-'A'] & ~0x3F)+ (ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNLOW['G'-'A'] = ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)+(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNHI['G'-'A'] = ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)+(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));

	/* Set Lcd BUS */
	gpio->GPIOALTFNHI['B'-'A'] = ((ALTFN1<< 14)+(ALTFN1<< 12)+(ALTFN1<< 10)+(ALTFN1<< 8)+(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1<< 0));
	gpio->GPIOALTFNLOW['A'-'A'] = ((ALTFN1<< 14)+(ALTFN1<< 12)+(ALTFN1<< 10)+(ALTFN1<< 8)+(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1<< 0));
	gpio->GPIOALTFNHI['A'-'A'] = ((ALTFN1<< 14)+(ALTFN1<< 12)+(ALTFN1<< 10)+(ALTFN1<< 8)+(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1<< 0));

	/*Set MCU Con*/
	gpio->GPIOALTFNLOW['I'-'A'] = ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)+(IO_IN<< 6)+(IO_IN<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNHI['I'-'A'] = ((gpio->GPIOALTFNHI['I'-'A'] & 0xC000)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)+(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNLOW['J'-'A'] = ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)+(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));
	gpio->GPIOALTFNHI['J'-'A'] = ((IO_IN<<14)+(IO_OUT<<12)+(IO_OUT<<10)+(ALTFN1<<8)+(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));

	/*Set UART * USB2.0_RST(bit4)*/
	gpio->GPIOALTFNLOW['D'-'A'] = ((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(IO_OUT<<8)+(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(gpio->GPIOALTFNLOW['D'-'A'] & 0x3));
	gpio->GPIOALTFNHI['D'-'A'] = ((gpio->GPIOALTFNHI['D'-'A'] & ~0x3F)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1));

	/*GP2X GPIO */
	gpio->GPIOALTFNHI['C'-'A'] = ((IO_IN<< 14)+(IO_IN<< 12)+(IO_IN<< 10)+(IO_IN<< 8)+(IO_IN<< 6)+(IO_IN<< 4)+(IO_IN<< 2)+(IO_IN<< 0));

	/* AC97 & SD  */
	gpio->GPIOALTFNLOW['L'-'A'] =((ALTFN1<<14)+(ALTFN1<<12)+(ALTFN1<<10)+(ALTFN1<<8)
								+(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1<< 0));
	gpio->GPIOALTFNHI['L'-'A']  =((IO_IN<< 6)+(ALTFN1<<4)+(ALTFN1<<2)+(ALTFN1<<0)+((gpio->GPIOALTFNHI['L'-'A'] & 0xFF00)));

	gpio->GPIOPUENB['L'-'A'] |= 0x003F;

	/* Sleep is tv(low) or reset(lcd,tv) high & lcd sync on*/
	gpio->GPIOALTFNLOW['B'-'A'] = ((gpio->GPIOALTFNLOW['B'-'A'] & ~0xFFF0)+(ALTFN1<< 14)+(ALTFN1<< 12)+(ALTFN1<< 10)+(ALTFN1<< 8)+(IO_OUT<< 6)+(IO_OUT<< 4));

	/* lcd sync no pullup */
	gpio->GPIOPUENB['B'-'A'] = ((gpio->GPIOPUENB['B'-'A'] & ~0xF0));

	/* TV sleep */
	gpio->GPIOOUT['B'-'A'] = ((gpio->GPIOOUT['B'-'A'] & ~0x08)| TV_SLEEP );

	/*Set Volatege GPIOH control (TV,LCD) */
	gpio->GPIOALTFNLOW['H'-'A'] = ((gpio->GPIOALTFNLOW['H'-'A'] & ~0x3FF)+(IO_OUT<< 8)+(IO_OUT<< 6)+(IO_OUT<< 4)+(IO_OUT<< 2)+(IO_OUT));

	/*LCD BACK OFF & LCD_VGH_OFF & BATT_FAULT_LED_OFF & TV_SLEEP& LCD_AVDD_ON */
	gpio->GPIOOUT['H'-'A'] = ((gpio->GPIOOUT['H'-'A'] & ~0xF)+ AVDD_5V + BATT_LED );
	delay(10);

	/* LCD_VGH_ON */
	gpio->GPIOOUT['H'-'A'] =gpio->GPIOOUT['H'-'A'] | LCD_VGH;
	delay(10);

	/* usb reset */
	gpio->GPIOALTFNLOW['D'-'A'] =
		((gpio->GPIOALTFNLOW['D'-'A'] & (~(3<<8)))
			+(IO_OUT<< 8));
#if 1
	gpio->GPIOOUT['D'-'A'] =
		((gpio->GPIOOUT['D'-'A'] & ~(1<<4)));
#else
	gpio->GPIOOUT['D'-'A'] =
		((gpio->GPIOOUT['D'-'A'] & ~(1<<4))|(1<<4));
#endif

	/* LCD reset */
	gpio->GPIOOUT['B'-'A']|=LCDTV_RESET;
	/* LCD_BACK_LIGHT_ON */
	gpio->GPIOOUT['H'-'A']|=BACK_LIGHT;

	gpio->GPIOALTFNLOW['L'-'A'] = (gpio->GPIOALTFNLOW['L'-'A'] & 0xF000) + (ALTFN1<<10)+(ALTFN1<<8)+(ALTFN1<< 6)+(ALTFN1<< 4)+(ALTFN1<< 2)+(ALTFN1<< 0);

	gpio->GPIOPUENB['L'-'A'] |= 0x003F;


	/* BUS Interface setup for CS8900(Bank1) Ethernet Interface */
	memctl->MEMCFGW &= ~(0x02);
	memctl->MEMCFGW |= 0x02; // 16bit


	/* set up the I/O ports -> move nand bootstrap*/
	/* arch number of MMSP20-Board */
	gd->bd->bi_arch_number = 395;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00100100;

	icache_enable();
	dcache_enable();

	setup_async();

	return 0;
}

int dram_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0 ;
}

int board_late_init(void)
{
	block_dev_desc_t *dev_desc=NULL;
	int dev = 0;

	if(0 != sdk2x_SdHwInit())
	{
		return 0; // No SD/MMC found !!
	}

	dev_desc=get_dev("mmc",0);

	if (dev_desc)
	{
		if(fat_register_device(dev_desc, 0)!=0)
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	return 1;
}

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
extern ulong nand_probe (ulong physadr);
void nand_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	MMSP20_BANC_MEMCTL * const  bankc;
	MMSP20_NAND        * const  nand;
	ulong totlen = 0;

	DPRINTF("++[%s:%d]->%s():\n", __FILE__,__LINE__,__FUNCTION__);
	bankc = MMSP20_GetBaseC_MEMCTL();
	nand  = MMSP20_GetBase_NAND();

	//bankc->MEMCFGW = bankc->MEMCFGW & ~(0x80);  /* 8bit bandwidth,  bit 7, 0: 8bit, 1:16bit*/
	bankc->MEMNANDCTRLW = 0;
	bankc->MEMNANDTIMEW = 0x7f8;
	bankc->MEMNANDCTRLW &= (~NAND_INT_DETRnB);

	printf("Probing at 0x%.8x\n", NAND_MEM_BASE_ADDR);
	totlen= nand_probe(NAND_MEM_BASE_ADDR);
}
#endif

// shkim add
#if (CONFIG_COMMANDS & CFG_CMD_LOADUB)

void ChangeUPllValue(int mdiv,int pdiv,int sdiv)
{
	MMSP20_CLOCK_POWER * const clk_power = MMSP20_GetBase_CLOCK_POWER();

	DPRINTF("++[%s:%d]->%s():\n", __FILE__,__LINE__,__FUNCTION__);
	/* configure UPLL */
	/*
		UPLL(MHz)= ((U_MDIV+8)*Fin) / ((U_PDIV+2) * 2^U_SDIV)
		Fin = 7.3728 MHz
	*/

	/* UCLK = 95.xxxMHz. It's default value. */
	/* U_MDIV=0x60 U_PDIV=0x0 U_SDIV=0x2 */
	//clk_power->UPLLSETVREG  = ((U_MDIV << 8) + (U_PDIV << 2) + U_SDIV);

	/* UCLK = 48 MHz */
	/* U_MDIV=0x60 U_PDIV=0x0 U_SDIV=0x3 */
	clk_power->UPLLSETVREG  = ((0x60 << 8) + (0 << 0x2) + 0x2);
	clk_power->UIRMCSETREG=((clk_power->UIRMCSETREG)&~(0x3<<6)) | (0x2<<6); // UPLL_CLK
	clk_power->UIRMCSETREG=((clk_power->UIRMCSETREG)&~(0x3f<<0)) | (0x1<<0); // devide set value

	DPRINTF("++[%s:%d]->%s(): clk_power->PWMODEREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->PWMODEREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->CLKCHGSTREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->CLKCHGSTREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->SYSCLKENREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->SYSCLKENREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->COMCLKENREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->COMCLKENREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->VGCLKENREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->VGCLKENREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->ASCLKENREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->ASCLKENREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->FPLLSETVREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->FPLLSETVREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->FPLLVSETREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->FPLLVSETREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->UPLLSETVREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->UPLLSETVREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->UPLLVSETREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->UPLLVSETREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->APLLSETVREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->APLLSETVREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->APLLVSETREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->APLLVSETREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->SYSCSETREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->SYSCSETREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->ESYSCSETREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->ESYSCSETREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->UIRMCSETREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->UIRMCSETREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->AUDICSETREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->AUDICSETREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->DISPCSETREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->DISPCSETREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->IMAGCSETREG=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->IMAGCSETREG);
	DPRINTF("++[%s:%d]->%s(): clk_power->URTCSETREG[0]=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->URTCSETREG[0]);
	DPRINTF("++[%s:%d]->%s(): clk_power->URTCSETREG[1]=0x%x\n",__FILE__,__LINE__,__FUNCTION__,clk_power->URTCSETREG[1]);
}

void pullup_en(void)
{
	MMSP20_GPIO * const gpio = MMSP20_GetBase_GPIO();

	DPRINTF("++[%s:%d]->%s():\n", __FILE__,__LINE__,__FUNCTION__);

	/* GPIOK7 output high */
	gpio->GPIOALTFNLOW['K'-'A'] = (gpio->GPIOALTFNLOW['K'-'A'] & ~(0x3<<14)) | (0x1<<14);
	gpio->GPIOOUT['K'-'A'] = gpio->GPIOOUT['K'-'A'] | (0x1<<7);
}

void pullup_dis(void)
{
	MMSP20_GPIO * const gpio = MMSP20_GetBase_GPIO();

	DPRINTF("++[%s:%d]->%s():\n", __FILE__,__LINE__,__FUNCTION__);

	/* GPIOK7 output low */
	gpio->GPIOALTFNLOW['K'-'A'] = (gpio->GPIOALTFNLOW['K'-'A'] & ~(0x3<<14)) | (0x1<<14);
	gpio->GPIOOUT['K'-'A'] = gpio->GPIOOUT['K'-'A'] & ~(0x1<<7);
}

void usbd_en(void)
{
	MMSP20_GPIO * const gpio = MMSP20_GetBase_GPIO();
	MMSP20_CLOCK_POWER * const clk_power = MMSP20_GetBase_CLOCK_POWER();

	DPRINTF("++[%s:%d]->%s():\n", __FILE__,__LINE__,__FUNCTION__);

	clk_power->COMCLKENREG=((clk_power->COMCLKENREG)&~(0x1<<1)) | (0x1<<1); // USB device clock enable
	gpio->GPIOPADSEL=((gpio->GPIOPADSEL)&~(1<<1)) | (1<<1); // USB PAD3 Type = USB Device
}

void usbd_dis(void)
{
	MMSP20_CLOCK_POWER * const clk_power = MMSP20_GetBase_CLOCK_POWER();

	DPRINTF("++[%s:%d]->%s():\n", __FILE__,__LINE__,__FUNCTION__);

	clk_power->COMCLKENREG=((clk_power->COMCLKENREG)&~(0x1<<1)) | (0x0<<1); // USB device clock disable
}



void display_logo(void)
{
	show_logo();
}

void sound_logo(void)
{
	int sFlag;

	MMSP20_GPIO * const gpio = MMSP20_GetBase_GPIO();
	sFlag=( (gpio->GPIOPINLVL['L'-'A']) & (1<<11) ) ? 0 : 1 ;  /* JACK IN='1' */

	PlayerSoundLogo(sFlag);
}

/* Removed as we now shows users upgrade status */
/*
void display_Firmware(void)
{
	show_Firmware();
}
*/

#ifdef CONFIG_SHOW_BOOT_PROGRESS

	#define RGB565_BLUE  (0x001F)
	#define RGB565_GREEN (0x07E0)
	#define RGB565_RED   (0xF800)
	#define RGB565_BLACK (0x0000)
	#define RGB565_WHITE (0xFFFF)

	void lcd_draw_rectangle (unsigned int start_x, unsigned int start_y, unsigned int width, unsigned int height, unsigned short colour)
	{
		int x, y;
		unsigned short * usPtr;
		unsigned short * fbaddr = (unsigned short*)PA_FB0_BASE;
		//printf("lcd_draw_rectangle: x:%d, y:%d, w:%d, h:%d, col:%d", start_x, start_y, width, height, colour);
	    if (start_x<320 && start_y<240)
	    {
		    if (start_x+width < 320 && start_y+height < 240)
		    {
				for (y=start_y; y<start_y+height; y++)
				{
					usPtr = fbaddr;
					usPtr += (start_x + (y*320));
					for (x=start_x; x<start_x+width; x++)
					{
						*usPtr++ = colour;
					}
				}
			}
		}
	}

	void show_boot_progress(int arg)
	{
		/* 108x175 to 284x193 is the box on the logo */

		if (arg >= 0)
		{
		    lcd_draw_rectangle(108, 175, 177, 19, RGB565_BLACK);
		    lcd_draw_rectangle(108, 175, ((177 * arg) / 15), 19, RGB565_GREEN);
		}
		else
		{
		    lcd_draw_rectangle(108, 175, 177, 19, RGB565_BLACK);
		}
	}
#endif

void attempt_firmware_upgrade(void)
{
	int press = 0;
	int keys = 0;
	int selected = 0;

	// Initialise an SD / SDHC if present
	sdk2x_SdHwInit();

	// Check that SD card has been found (with a FAT filesystem).
	if(mmc_init_once == 1)
	{
		int size = 0;
		char *gp2xbootimg[] 	= { "fatload", "mmc", "0", "0x1000000", "open2xboot.img"	};
		char *gp2xkernelimg[] 	= { "fatload", "mmc", "0", "0x1000000", "gp2xkernel.img"	};
		char *gp2xyaffsimg[] 	= { "fatload", "mmc", "0", "0x1000000", "gp2xyaffs.img"		};
		char *gp2xfileimg[] 	= { "fatload", "mmc", "0", "0x1000000", "gp2xyaffs.img"		};
		char *gp2xsoundlogo[]   = { "fatload", "mmc", "0", "0x1000000", "gp2xsound.wav"     };
		unsigned char bShowLogo = 0;

		/* Just blank the screen */
		unsigned char *fbaddr = (unsigned char*)PA_FB0_BASE;
		int x, y;

		for(y=0; y<240; y++)
		{
			for(x=0; x<320; x++)
			{
				*fbaddr++ = 0x00;
				*fbaddr++ = 0x00;
			}
		}


		sdk2x_LcdTextOut (100, 20, "Firmware Flashing", 0xFFFF);
		sdk2x_LcdTextOut (100, 27, "-----------------", 0xFFFF);
		sdk2x_LcdTextOut (100, 40, "                 ", 0xFFFF);

		printf("Updating  ...\n");

		/* Reflash U-Boot Image (open2xboot.img) */

		printf("Bootloader Update ---\n");
		size = do_fat_fsload(NULL, 0, 5, gp2xbootimg);
		printf("1 : g_filesize %x\n", g_filesize);

		if(size != -1)
		{
			int ret_gp2xboot_erase = 0;
			int ret_gp2xboot_write = 0;
			ulong length = 0x4000 * ((size/0x4000)+1);		// 0x4000 단위로 나누고 이걸 1더해서 곱하면 0x4000의 배수가 된다.
			printf("open2xboot.img Update %d 0x%x \n", size, length);

			sdk2x_LcdTextOut (0, 60, "The bootloader is about to be upgraded.", 0xF800);
			sdk2x_LcdTextOut (0, 70, "Are you sure? (SELECT = Yes, B = Cancel)", 0xF800);

			while (!selected)
			{
				if (SDK2X_KEY_SELECT & sdk2x_SysReadKeys()) /* Check to see if the SELECT key is pressed */
				{

					do
					{
						do
						{
							ret_gp2xboot_erase = nand_erase_func(0, 0x80000);
						}
						while(ret_gp2xboot_erase); // 0이면 지원진 것이다.

							ret_gp2xboot_write = nand_write_func(0x1000000, 0, length);
						}while(ret_gp2xboot_write);	// 0이면 쓴 것이다.

					sdk2x_LcdTextOut (0, 100, "open2xboot.img - Flashed.", 0xF800);
					selected = 1;
				}
				else if (SDK2X_KEY_B & sdk2x_SysReadKeys())
				{
					sdk2x_LcdTextOut (0, 100, "Skipping bootloader upgrade.", 0xF800);
					selected = 1;
				}
			}
		}
		else
		{
			printf("open2xboot.img not found\n");
			sdk2x_LcdTextOut (0, 100, "open2xboot.img - Not Found.", 0xF800);
		}

		/* Reflash Linux Kernel Image (gp2xkernel.img) */

		printf("Kernel Update ---\n");
		size = -1;
		size = do_fat_fsload(NULL, 0, 5, gp2xkernelimg);
		printf("2 : g_filesize %x\n", g_filesize);

		if(size != -1)
		{
			// kernel를 찾으면 업데이트 한다.
			int ret_gp2xkernel_erase = 0;
			int ret_gp2xkernel_write = 0;
			ulong length = 0x4000 * ((size/0x4000)+1);		// 0x4000 단위로 나누고 이걸 1더해서 곱하면 0x4000의 배수가 된다.
			printf("gp2xkernel.img Update %d 0x%x \n", size, length);

			//if(bShowLogo == 0)
			//{
			//	bShowLogo = 1;
			//	display_Firmware();
			//}

			do
			{
				do
				{
					ret_gp2xkernel_erase = nand_erase_func(0x80000, 0x120000);
				}
				while(ret_gp2xkernel_erase);		// 0이면 지원진 것이다.

				ret_gp2xkernel_write = nand_write_func(0x1000000, 0x80000, length);
			}while(ret_gp2xkernel_write);	// 0이면 쓴 것이다.

			sdk2x_LcdTextOut (0, 110, "gp2xkernel.img - Flashed.", 0xF800);
		}
		else
		{
			printf("gp2xkernel.img not found\n");
			sdk2x_LcdTextOut (0, 110, "gp2xkernel.img - Not Found.", 0xF800);
		}

		/* Reflash Boot sound (gp2xsound.wav) */

		printf("Boot Sound Update ---\n");
		size = -1;
		size = do_fat_fsload(NULL, 0, 5, gp2xsoundlogo);
		printf("2 : g_filesize %x\n", g_filesize);
		if(size > 0x50000)
		{
			printf("Sound file to big!\n");
			size=-1;
		}
		if(size != -1)
		{
			int ret_gp2xsound_erase = 0;
			int ret_gp2xsound_write = 0;
			ulong length = 0x4000 * ((size/0x4000)+1);		// 0x4000 단위로 나누고 이걸 1더해서 곱하면 0x4000의 배수가 된다.
			printf("gp2xsound.wav Update %d 0x%x \n", size, length);

			do
			{
				do
				{

					ret_gp2xsound_erase = nand_erase_func(0x1A0000, 0x60000);
				}
				while(ret_gp2xsound_erase);		// 0이면 지원진 것이다.

				ret_gp2xsound_write = nand_write_jffs2_func(0x1000000, 0x1A0000, length);
			}while(ret_gp2xsound_write);	// 0이면 쓴 것이다.

			sdk2x_LcdTextOut (0, 120, "gp2xsound.wav - Flashed.", 0xF800);

		}
		else
		{
			printf("gp2xsound.wav not found\n");
			sdk2x_LcdTextOut (0, 120, "gp2xsound.wav - Not Found.", 0xF800);
		}

		/* Reflash Old FW1 Style JFFS2 Image (gp2xfile.img) */

		printf("JFFS2 File System Update ---\n");
		size = -1;
		size = do_fat_fsload(NULL, 0, 5, gp2xfileimg);
		printf("3 : g_filesize %x\n", g_filesize);
		if(size != -1)
		{
			// disk file을 찾으면 업데이트 한다.
			int ret_gp2xfile_erase = 0;
			int ret_gp2xfile_write = 0;
			ulong length = 0x4000 * ((size/0x4000)+1);		// 0x4000 단위로 나누고 이걸 1더해서 곱하면 0x4000의 배수가 된다.
			printf("gp2xfile.img Update %d 0x%x \n", size, length);

			do
			{
				do
				{
					ret_gp2xfile_erase = nand_erase_func(0x240000, 0x3200000);
				}
				while(ret_gp2xfile_erase);
				ret_gp2xfile_write = nand_write_jffs2_func(0x1000000, 0x240000, length);


				if(ret_gp2xfile_write) 	/* REWRITE */
					ret_gp2xfile_write = nand_write_jffs2_func(0x1000000, 0x240000, length);

			}while(ret_gp2xfile_write);

			sdk2x_LcdTextOut (0, 130, "gp2xfile.img - Flashed.", 0xF800);
		}
		else
		{
			printf("gp2xfile.img not found\n");
			sdk2x_LcdTextOut (0, 130, "gp2xfile.img - Not Found.", 0xF800);
		}

		/* Reflash FW2 > Style YAFFS Image (gp2xyaffs.img) */

		printf("YAFFS File System Update ---\n");
		size = -1;
		size = do_fat_fsload(NULL, 0, 5, gp2xyaffsimg);
		printf("3 : g_filesize %x\n", g_filesize);
		if(size != -1)
		{
			// disk file을 찾으면 업데이트 한다.
			int ret_gp2xyaffs_erase = 0;
			int ret_gp2xyaffs_write = 0;
			ulong length = 0x4000 * ((size/0x4000)+1);		// 0x4000 단위로 나누고 이걸 1더해서 곱하면 0x4000의 배수가 된다.
			printf("gp2xyaffs.img Update %d 0x%x \n", size, length);

			do
			{
				do
				{
					ret_gp2xyaffs_erase = nand_erase_func(0x200000, 0x1E00000);	 //32MB - 2MB(Boot+kernel)=30MB
				}
				while(ret_gp2xyaffs_erase);		// 0이면 지원진 것이다.
				ret_gp2xyaffs_write = nand_write_yaffs_func(0x1000000, 0x220000, length); //A
				//ret_gp2xfile_write = nand_write_yaffs_func(0x1000000, 0x240000, length); //B

				if(ret_gp2xyaffs_write) 	/* REWRITE */
					ret_gp2xyaffs_write=nand_write_yaffs_func(0x1000000, 0x220000, length); 	//A
			//		ret_gp2xfile_write=nand_write_yaffs_func(0x1000000, 0x240000, length);  //B

			}while(ret_gp2xyaffs_write);	// 0이면 쓴 것이다.

			sdk2x_LcdTextOut (0, 130, "gp2xyaffs.img - Flashed.", 0xF800);
		}
		else
		{
			printf("gp2xyaffs.img not found\n");
			sdk2x_LcdTextOut (0, 130, "gp2xyaffs.img - Not Found.", 0xF800);
		}
	}
	sdk2x_LcdTextOut (0, 150, "Complete...", 0x07E0);
	sdk2x_LcdTextOut (0, 160, "Press power off your GP2X and restart.", 0x07E0);

	/* Wait for the user to press B after flash
	   Allows user to see all flash info */

	while (!press)
	{
		if (SDK2X_KEY_B & sdk2x_SysReadKeys()) /* Check to see if the B key is pressed */
		{
			return;
		}
	}
}

#endif
