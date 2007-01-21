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

	/*usb reset */
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



unsigned char scratch[4096]; // buffer for SD read/write
unsigned int scratch1[4096]; // buffer for SD read/write
int SD_FS(void)  // Check Partition table
{
	int MBR_exist;
	int i;
	
	MBR_exist = 0;
	SD_Read(scratch, 0 , 1);
	//printf("SD Read #0 done\n");	
#if 0 /* org */
	if(scratch[511]==0xAA && scratch[510]==0x55)
	{

//	    if((scratch[450] == 0x01) || (scratch[450] == 0x04) || (scratch[450] == 0x06) ||
//	       (scratch[450] == 0x0B) ||   /* FAT32 Partition */
//	       (scratch[450] == 0x0C) ||   /* FAT32 Partition */
//	       (scratch[450] == 0x55) )    /* FAT32 Partition */
//		       	MBR_exist=1;
	    if((scratch[450] == 0x0B) ||   /* FAT32 Partition */
	       (scratch[450] == 0x0C) ||   /* FAT32 Partition */
	       (scratch[450] == 0x55) )    /* FAT32 Partition */
	       {
		       	MBR_exist=1;
		   }
		   else
		   {
		       	MBR_exist=0;
				mmc_init_once = 0;
		   		printf("mmc_init_once %d\n", mmc_init_once);
		   }
	} 

	if(MBR_exist) 	printf("(with MBR)\n\n"); 
	else 			printf("(without MBR)\n\n");
#else /* shkim patch */
	if(scratch[511]==0xAA && scratch[510]==0x55)
	{

	    if( (scratch[0x36] == 'F') && (scratch[0x37] == 'A') && (scratch[0x38] == 'T'))
	    {
			/* PBR only for FAT16 */ 
			mmc_init_once = 1;
			/* Although NO MBR, we asume MBR exist */
		       	MBR_exist=1;
	    }
	    else if( (scratch[0x52] == 'F') && (scratch[0x53] == 'A') && (scratch[0x54] == 'T'))
	    {
			/* PBR only for FAT32 */ 
			mmc_init_once = 1;
			/* Although NO MBR, we asume MBR exist */
		       	MBR_exist=1;
	    }
	    else if((scratch[450] == 0x01) || (scratch[450] == 0x04) || (scratch[450] == 0x06) ||
	       (scratch[450] == 0x0B) ||   /* FAT32 Partition */
	       (scratch[450] == 0x0C) ||   /* FAT32 Partition */
	       (scratch[450] == 0x55) )    /* FAT32 Partition */
	    {
			mmc_init_once = 1;
		       	MBR_exist=1;
	    }
	    else
	    {
			mmc_init_once = 0;
		       	MBR_exist=0;
	    }
	}

	if(MBR_exist) 	printf("(with MBR)\n\n"); 
	else		printf("(without MBR)\n\n");

#endif

}
   
int board_late_init(void)
{	
	int uFlag;
	MMSP20_GPIO * const gpio = MMSP20_GetBase_GPIO();
	
	uFlag=( (gpio->GPIOPINLVL['C'-'A']) & ((1<<8)|(1<<9)) ) ? 1 : 0; 
	
	if(uFlag) return 1;
	
	
	if(!SD_card_init()) 
	{
		return 0; // No SD/MMC found !!
	}
	else
	{
		SD_FS(); // Check Master boot record !!
	}

	//SD_FS(); // Check Master boot record !!

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
	//bankc->MEMNANDTIMEW = 0x7ff; 
	//bankc->MEMNANDTIMEW = 0xfFF; 
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

// 수정 : 미스콜이아.
void display_Firmware(void)
{
	show_Firmware();
}


#endif
