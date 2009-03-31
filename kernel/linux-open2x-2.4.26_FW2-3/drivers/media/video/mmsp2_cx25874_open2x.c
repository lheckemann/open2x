// NOTE FROM SENQUACK:
// Under Open2X, there are now two separate cx25874 devices present.  They both 
// function the exact same, except that the open2x version does not reset the chip
// when first being opened.  Having a separate driver allows the TV tweaking daemon to
// always be allowed to talk to the chip, even if other programs have the device
// file open for writing. This just uses minor number 233.

/*
 *  linux/drivers/media/video/mmsp2_cx25874_open2x.c
 *
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 *  CX25874 Video Encoder Driver
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/arch/mmsp20.h>
#include <asm/arch/proto_dpc.h>
#include <asm/arch/proto_pwrman.h>
#include <asm/arch/proto_mlc.h>

#include "cx25874.h"

extern int i2c_set;
static int cx25874_count=0;

extern int I2C_wbyte( unsigned char id, unsigned char addr, unsigned char data );
extern int I2C_rbyte( unsigned char id, unsigned char addr, unsigned char *pData );
static void CX25874(unsigned char mode);

//DKS - I had to add these here to allow compilation
static void lcdTurnOn(void);
static void lcd_shutdown(void);

static int hShift=0;
static int vShift=0;
static int vsh=4;
static int tvFlag=0;

static void CX25874(unsigned char mode)
{
	unsigned short PLL_FRACT;
	unsigned char PLL_INT;

	I2C_wbyte(CX25874_ID,0xba,0x80);  	// Software Reset
	mdelay(10);

	switch(mode)
	{
		case DISPLAY_LCD :
			/* TV OFF MODE */
			I2C_wbyte(CX25874_ID,0x30,0x10);
			mdelay(10);
			I2C_wbyte(CX25874_ID,0x30,0x80);
			mdelay(10);
			I2C_wbyte(CX25874_ID,0x30,0x90);
			mdelay(10);
			write_gpio_bit(GPIO_B2, 1);         // io sleep on...
			MLC_isTVCheck(CFALSE);
			break;
		case DISPLAY_MONITOR :
			// 800 * 600 mode
			// 38,400,000 * 2 = 76,800,000 Hz
			// (76800000 * 6) / 13500000 = 34.1333...
			// 0.1333... * 2^16 = 8738.133331
			PLL_FRACT = 8738;
			PLL_INT = 34;

			I2C_wbyte(CX25874_ID, 0xd6, 0x0c);
			I2C_wbyte(CX25874_ID, 0x2e, 0xbd);
			I2C_wbyte(CX25874_ID, 0x32, 0x00);
			I2C_wbyte(CX25874_ID, 0x3c, 0x80);
			I2C_wbyte(CX25874_ID, 0x3e, 0x80);
			I2C_wbyte(CX25874_ID, 0x40, 0x80);
			I2C_wbyte(CX25874_ID, 0xc4, 0x01);
			I2C_wbyte(CX25874_ID, 0xc6, 0x50);      // EN_DOT=1, VSYNCI = 1, HSYNCI= 0
			I2C_wbyte(CX25874_ID, 0xce, 0x24);
			I2C_wbyte(CX25874_ID, 0xa0, PLL_INT);
			I2C_wbyte(CX25874_ID, 0x9e, (PLL_FRACT>>8));
			I2C_wbyte(CX25874_ID, 0x9c, (PLL_FRACT&0xff));
			I2C_wbyte(CX25874_ID, 0xd4, 0x80);      //MODE2X Bit 7 : 1 = Divides selected input clock by two (allows for single edge rather than double-edge
			I2C_wbyte(CX25874_ID, 0xa2, 0x00);
			I2C_wbyte(CX25874_ID, 0xba, 0x28);

			mdelay(10);

			I2C_wbyte(CX25874_ID, 0x6C, 0xC6);
			break;
		case DISPLAY_TV_NTSC:
			// svideo ntsc mode
			// 13,500,000 * 2 = 27,000,000 Hz
			// (27000000 * 6) / 13500000 = 12.0
			// 0 * 2^16 = 0

			PLL_FRACT = 0;
			PLL_INT = 12;

			I2C_wbyte(CX25874_ID, 0x38, 0x10);
			I2C_wbyte(CX25874_ID, 0x76, 0xb4);      //H_CLKO 180
			I2C_wbyte(CX25874_ID, 0x78, 0xd0);      //H_ACTIVE 280
			I2C_wbyte(CX25874_ID, 0x7a, 0x7e);      //H_WIDTH  126
			I2C_wbyte(CX25874_ID, 0x7c, 0x90);      //H_BUST_BEGIN 144
			I2C_wbyte(CX25874_ID, 0x7e, 0x58);  	//H_BUST_END 88
			I2C_wbyte(CX25874_ID, 0x80, 0x03);  	//h_BLANKO 3
			I2C_wbyte(CX25874_ID, 0x82, 0x14);      //v_BLANKO 20
			I2C_wbyte(CX25874_ID, 0x84, 0xf0);      //v_active 132
			I2C_wbyte(CX25874_ID, 0x86, 0x26);
			I2C_wbyte(CX25874_ID, 0x88, 0x15);

			I2C_wbyte(CX25874_ID, 0x8a, 0x5a);  //0x35a  858
			//I2C_wbyte(CX25874_ID, 0x8c, 0x0a);
			I2C_wbyte(CX25874_ID, 0x8c, 0x28);

			I2C_wbyte(CX25874_ID, 0x8e, 0x13);
			I2C_wbyte(CX25874_ID, 0x90, 0x06);

			I2C_wbyte(CX25874_ID, 0x92, 0x13);
			//I2C_wbyte(CX25874_ID, 0x92, 0x11);

			I2C_wbyte(CX25874_ID, 0x94, 0xf0);
			I2C_wbyte(CX25874_ID, 0x96, 0x31);
			I2C_wbyte(CX25874_ID, 0x98, 0x00);
			I2C_wbyte(CX25874_ID, 0x9a, 0x40);
			I2C_wbyte(CX25874_ID, 0x9c, (PLL_FRACT&0xff));
			I2C_wbyte(CX25874_ID, 0x9e, (PLL_FRACT>>8));
			I2C_wbyte(CX25874_ID, 0xa0, (PLL_INT | 0x80));  // EN_XCLK=1
			I2C_wbyte(CX25874_ID, 0xa2, 0x0a);
			I2C_wbyte(CX25874_ID, 0xa4, 0xe5);
			I2C_wbyte(CX25874_ID, 0xa6, 0x76);
			I2C_wbyte(CX25874_ID, 0xa8, 0xc1);
			I2C_wbyte(CX25874_ID, 0xaa, 0x89);
			I2C_wbyte(CX25874_ID, 0xac, 0x9a);
			I2C_wbyte(CX25874_ID, 0xae, 0x1f);
			I2C_wbyte(CX25874_ID, 0xb0, 0x7c);
			I2C_wbyte(CX25874_ID, 0xb2, 0xf0);
			I2C_wbyte(CX25874_ID, 0xb4, 0x21);
			I2C_wbyte(CX25874_ID, 0xc6, 0x06);
			I2C_wbyte(CX25874_ID, 0xc8, 0xc0);
			//I2C_wbyte(CX25874_ID, 0xc6, 0x86);
			mdelay(10);
			I2C_wbyte(CX25874_ID, 0x6C, 0xC6);
			break;

		case DISPLAY_TV_PAL:
			// 720*576 PAL-BDGHI mode
			// 13,500,000 * 2 = 27,000,000 Hz
			// (27000000 * 6) / 13500000 = 12.0
			// 0 * 2^16 = 0
			PLL_FRACT = 0;
			PLL_INT = 12;

			I2C_wbyte(CX25874_ID, 0x38, 0x10);      //DIV2 SET
			I2C_wbyte(CX25874_ID, 0x76, 0xc0);      //total hclk 0x6c0(1728) (with 0x86 0~3bit)
			I2C_wbyte(CX25874_ID, 0x78, 0xd0);      //H_ACTIVE 0x2D0(720) (with 0x86 4~6bit) ok...
			I2C_wbyte(CX25874_ID, 0x7a, 0x7e);      //H_WIDTH  0x7E(126)
			I2C_wbyte(CX25874_ID, 0x7c, 0x98);      //H_BUST_BEGIN 0x98(152)
			I2C_wbyte(CX25874_ID, 0x7e, 0x54);      //H_BUST_END 0x54(84)
			I2C_wbyte(CX25874_ID, 0x80, 0x15);      //h_BLANKO 0x115(277) (with 0x9a 8~9bit)

			I2C_wbyte(CX25874_ID, 0x82, 0x17);      //v_BLANKO 0x23
			I2C_wbyte(CX25874_ID, 0x84, 0x20);      //v_active 0x120(288==(576/2)) (with 0x86 7bit) ok...
			I2C_wbyte(CX25874_ID, 0x86, 0xA6);

			/* cpu tv control output */
			I2C_wbyte(CX25874_ID, 0x88, 0xFA);
			I2C_wbyte(CX25874_ID, 0x8a, 0x60); //total input hclk 0x360(864) (with 0x8E 0~2bit)

			//I2C_wbyte(CX25874_ID, 0x8c, 0x0a); //H_BLANKI 0x0a(10) h back porch
			I2C_wbyte(CX25874_ID, 0x8c, 0x28);
			I2C_wbyte(CX25874_ID, 0x8e, 0x13);
			I2C_wbyte(CX25874_ID, 0x90, 0x38); //total input vclk 0x138 (312) (with 0x96 0~1bit)
			I2C_wbyte(CX25874_ID, 0x92, 0x16); //v_BLAKNK 0x16(22)

			I2C_wbyte(CX25874_ID, 0x94, 0x20); //v_active input 0x120(288==576/2) (with 0x96 2~3bit)
			I2C_wbyte(CX25874_ID, 0x96, 0x35);
			I2C_wbyte(CX25874_ID, 0x98, 0x00);
			I2C_wbyte(CX25874_ID, 0x9a, 0x40);

			I2C_wbyte(CX25874_ID, 0x9c, (PLL_FRACT&0xff)); //0
			I2C_wbyte(CX25874_ID, 0x9e, (PLL_FRACT>>8));   //0
			I2C_wbyte(CX25874_ID, 0xa0, (PLL_INT | 0x80));  // EN_XCLK=1

			I2C_wbyte(CX25874_ID, 0xa2, 0x24); // 625LINE|PAL-MD
			I2C_wbyte(CX25874_ID, 0xa4, 0xf0);
			I2C_wbyte(CX25874_ID, 0xa6, 0x59);
			I2C_wbyte(CX25874_ID, 0xa8, 0xCF);
			I2C_wbyte(CX25874_ID, 0xaa, 0x93);
			I2C_wbyte(CX25874_ID, 0xac, 0xA4);
			I2C_wbyte(CX25874_ID, 0xae, 0xCB);
			I2C_wbyte(CX25874_ID, 0xb0, 0x8A);
			I2C_wbyte(CX25874_ID, 0xb2, 0x09);
			I2C_wbyte(CX25874_ID, 0xb4, 0x2A);


			I2C_wbyte(CX25874_ID, 0xc6, 0x06); //inmode ycrcb 16bit
			I2C_wbyte(CX25874_ID, 0xc8, 0xc0); //
			//I2C_wbyte(CX25874_ID, 0xc6, 0x86);

			mdelay(10);
			I2C_wbyte(CX25874_ID, 0x6C, 0xC6); //check point fild mode
			//I2C_wbyte(CX25874_ID, 0x6C, 0xc4); //check point fild mode

			break;
		case DISPLAY_TV_GAME_NTSC:
			// 320 x 240 ntsc mode
			PLL_FRACT = 0;
			PLL_INT = 12;

			I2C_wbyte(CX25874_ID, 0x38, 0x40);      //DIV2 SET
			I2C_wbyte(CX25874_ID, 0x76, 0x00);      //total hclk 0x6c0(1728) (with 0x86 0~3bit)
			I2C_wbyte(CX25874_ID, 0x78, 0x80);      //H_ACTIVE 0x2D0(720) (with 0x86 4~6bit) ok...
			I2C_wbyte(CX25874_ID, 0x7a, 0x84);      //H_WIDTH  0x7E(126)
			I2C_wbyte(CX25874_ID, 0x7c, 0x96);  //H_BUST_BEGIN 0x98(152)
			I2C_wbyte(CX25874_ID, 0x7e, 0x60);  //H_BUST_END 0x54(84)
			I2C_wbyte(CX25874_ID, 0x80, 0x7d);  //h_BLANKO 0x115(277) (with 0x9a 8~9bit)

			I2C_wbyte(CX25874_ID, 0x82, 0x22);      //v_BLANKO 0x23
			I2C_wbyte(CX25874_ID, 0x84, 0xD5);      //v_active 0x120(288==(576/2)) (with 0x86 7bit) ok...
			I2C_wbyte(CX25874_ID, 0x86, 0x27);

			/* cpu tv control output */
			I2C_wbyte(CX25874_ID, 0x88, 0x00);
			I2C_wbyte(CX25874_ID, 0x8a, 0x20); //total input hclk 0x360(864) (with 0x8E 0~2bit)
			I2C_wbyte(CX25874_ID, 0x8c, 0x5D); //H_BLANKI 0x0a(10) h back porch
			I2C_wbyte(CX25874_ID, 0x8e, 0x1E);
			I2C_wbyte(CX25874_ID, 0x90, 0x2C); //total input vclk 0x138 (312) (with 0x96 0~1bit)
			I2C_wbyte(CX25874_ID, 0x92, 0x25); //v_BLAKNK 0x16(22)
			I2C_wbyte(CX25874_ID, 0x94, 0xF0); //v_active input 0x120(288==576/2) (with 0x96 2~3bit)
			I2C_wbyte(CX25874_ID, 0x96, 0x31);
			I2C_wbyte(CX25874_ID, 0x98, 0x49);
			I2C_wbyte(CX25874_ID, 0x9a, 0x42);

			I2C_wbyte(CX25874_ID, 0x9c, 0x0E);
			I2C_wbyte(CX25874_ID, 0x9e, 0x88);
			I2C_wbyte(CX25874_ID, 0xa0, 0x0C);

			I2C_wbyte(CX25874_ID, 0xa2, 0x0A); // 625LINE|PAL-MD
			I2C_wbyte(CX25874_ID, 0xa4, 0xE5);
			I2C_wbyte(CX25874_ID, 0xa6, 0x76);
			I2C_wbyte(CX25874_ID, 0xa8, 0x79);
			I2C_wbyte(CX25874_ID, 0xaa, 0x44);
			I2C_wbyte(CX25874_ID, 0xac, 0x85);
			I2C_wbyte(CX25874_ID, 0xae, 0x00);
			I2C_wbyte(CX25874_ID, 0xb0, 0x00);
			I2C_wbyte(CX25874_ID, 0xb2, 0x80);
			I2C_wbyte(CX25874_ID, 0xb4, 0x20);

			I2C_wbyte(CX25874_ID, 0xc4, 0x01);
			I2C_wbyte(CX25874_ID, 0xc6, 0x41); //inmode rgb 16bit
			I2C_wbyte(CX25874_ID, 0xc8, 0xc0); //
			I2C_wbyte(CX25874_ID, 0xba, 0x20); //

			mdelay(10);
			I2C_wbyte(CX25874_ID, 0x6C, 0xC6); //check point fild mode
			//I2C_wbyte(CX25874_ID, 0x6C, 0xC0); //check point fild mode
			break;

	}
}

//senquack
//static int cx25874_open(struct inode *inode, struct file *file)
//{
//	write_gpio_bit(GPIO_B2, 0);                             // HW IO enable
//	mdelay(1);
//
//#ifdef CONFIG_MACH_GP2XF200
//	/* board version check */
//	if(!read_gpio_bit(GPIO_I12) ) lcd_shutdown();
//#endif
//
//	write_gpio_bit(GPIO_B3, 0);                             // CX25874 Reset
//	mdelay(1);
//	write_gpio_bit(GPIO_B3, 1);
//	mdelay(1);
//
//	if(cx25874_count==0)
//	{
//		unsigned char data;
//
//		I2C_rbyte(CX25874_ID,0x00,&data);                 	// Read Vender ID
//		if(data==0xC3)
//			printk("CX25874 ID = %2x \n",data);
//		else
//			printk("CX25874 ID check error = %2x \n",data);
//
//		I2C_wbyte(CX25874_ID,0xba,0x80);                  	// Software Reset
//		mdelay(10);
//
//		cx25874_count = 1;
//		return 0;
//	}else
//		return -EBUSY;
//}
static int cx25874_open2x_open(struct inode *inode, struct file *file)
{
//	if(cx25874_count==0)
//	{
//		cx25874_count = 1;
//		return 0;
//	}else
//		return -EBUSY;
	if(cx25874_count==0)
	{
		cx25874_count = 1;
	}

	return 0;
}

static int cx25874_open2x_release(struct inode *inode, struct file *file)
{
	/* Tv off :lcd mode(ioctl) */
	cx25874_count = 0;
//	printk("OPEN2X TV_OUT release \n");
	return 0;
}

static int cx25874_open2x_ioctl(struct inode *inode, struct file *file,
					 unsigned int cmd, unsigned long arg)
{
	tDispClkInfo ClkInfo;
	int ret=0;
	I2C_WRITE_BYTE_INFO i2c_write_info;
	I2C_READ_BYTE_INFO i2c_read_info;

	switch(cmd)
	{
		case CX25874_I2C_WRITE_BYTE:
			if(copy_from_user(&i2c_write_info, (I2C_WRITE_BYTE_INFO *)arg, sizeof(I2C_WRITE_BYTE_INFO)))
				return -EFAULT;
			if(i2c_write_info.id != CX25874_ID) i2c_write_info.id = CX25874_ID;
			mdelay(1);
			I2C_wbyte(i2c_write_info.id, i2c_write_info.addr, i2c_write_info.data);
			ret = 1;
			break;

		case CX25874_I2C_READ_BYTE :
			if(copy_from_user(&i2c_read_info, (I2C_READ_BYTE_INFO *)arg, sizeof(I2C_READ_BYTE_INFO)))
				return -EFAULT;
			if(i2c_write_info.id != CX25874_ID) i2c_write_info.id = CX25874_ID;
			I2C_rbyte(i2c_read_info.id, i2c_read_info.addr, i2c_read_info.pdata);
			ret = 1;
			break;

		case IOCTL_CX25874_TV_MODE_POSITION:
			if( arg == TV_POS_LEFT ){
				if(hShift < 80){
					hShift++;
					I2C_wbyte(CX25874_ID, 0x8c,hShift);
				}

			}
			else if ( arg == TV_POS_RIGHT ){
				if(hShift > 3){
					hShift--;
					I2C_wbyte(CX25874_ID, 0x8c,hShift);
				}
			}
			else if ( arg == TV_POS_UP ){
				if(tvFlag == TV_MODE_NTSC){
					if(vsh < 243){
						vShift--,vsh++;
						DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, vsh, 32, 66, 40, CTRUE, 1, 1, vShift, CTRUE);
					}
				}
				else
				{
#if 0
					if(vsh < 295)
					{
						vShift--,vsh++;
						DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, vsh, 32, 66, 40, CTRUE, 2, 1, vShift, CTRUE);
					}
#endif
				}
			}
			else if ( arg == TV_POS_DOWN ){
				if(tvFlag == TV_MODE_NTSC){//ntsc
					if(vsh > 225){
						vShift++,vsh--;
						DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, vsh, 32, 66, 40, CTRUE, 1, 1, vShift, CTRUE);
					}
				}
				else
				{
#if 0
					if(vsh > 265){
						vShift++,vsh--;
						DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, vsh, 32, 66, 40, CTRUE, 2, 1, vShift, CTRUE);
					}
#endif
				}
			}
			ret=1;
			break;

		case IOCTL_CX25874_DISPLAY_MODE_SET :
			if( (arg == DISPLAY_MONITOR) ||  (arg == DISPLAY_TV_NTSC)
						|| (arg == DISPLAY_TV_PAL) )
			{
		/* LCD OFF MDOE */
#ifndef CONFIG_MACH_GP2XF200
				write_gpio_bit(GPIO_H1,0);     	//LCD VGH Off
				write_gpio_bit(GPIO_H2,0);    	//LCD Back Off
#else
				write_gpio_bit(GPIO_H1,0);    	//LCD AVDD Off
				write_gpio_bit(GPIO_L11,0);    	//LCD Back Off
				write_gpio_bit(GPIO_F3,0);      //Sound to TV (AMP Off)
#endif
			}

			if( arg == DISPLAY_LCD )
			{
            	printk("******* lcd mode **************  \n");
				CX25874(arg);
				/* Display Controller setup */
				DPC_Stop();
				ClkInfo.DISPCLK_SOURCE  = 2;
				ClkInfo.DISPCLK_POL = 1;
#ifndef CONFIG_MACH_GP2XF200
				ClkInfo.DISPCLK_DIVIDER= 5;
#else
				if(!read_gpio_bit(GPIO_I12) ) ClkInfo.DISPCLK_DIVIDER = 13;
				else ClkInfo.DISPCLK_DIVIDER= 5;
#endif
				PMR_SetDispClk(&ClkInfo);
				DPC_InitHardware(DPC_RGB_666, CFALSE, CFALSE,CFALSE, 0, 0, DPC_USE_PLLCLK);
#ifndef CONFIG_MACH_GP2XF200
				/* ODT 408 * 262*/
				DPC_UTIL_HVSYNC_GPX320240(DPC_RGB_666, 320,240,30, 20, 38, CFALSE, 3, 5, 15,CFALSE,CTRUE,CFALSE,CTRUE);
#else
				if(!read_gpio_bit(GPIO_I12) )
					DPC_UTIL_HVSYNC_GPX320240(DPC_RGB_666, 320, 240, 30, 20, 38, CFALSE, 3, 5, 15,CFALSE,CFALSE,CTRUE,CFALSE);
				else
					DPC_UTIL_HVSYNC_GPX320240(DPC_RGB_666, 320, 240, 30, 20, 38, CFALSE, 3, 5, 15,CFALSE,CTRUE,CFALSE,CTRUE);
#endif

				DPC_Run();

#ifndef CONFIG_MACH_GP2XF200
    			write_gpio_bit(GPIO_H1,1);      	//LCD VGH ON
    			write_gpio_bit(GPIO_H2,1);         	//LCD Back ON
#else
				if(!read_gpio_bit(GPIO_I12) ) lcdTurnOn();

				write_gpio_bit(GPIO_H1,1);         	//LCD AVDD ON
    			write_gpio_bit(GPIO_L11,1);         //LCD Back ON
				write_gpio_bit(GPIO_F3,1);          //Sound on LCD(AMP_ON)
#endif
			}
			else if (arg == DISPLAY_MONITOR )       // ARGB_800x600x16_60Hz
			{
				CX25874(arg);
				/* Display Controller setup */
				DPC_Stop();

				ClkInfo.DISPCLK_SOURCE  = 0;
				ClkInfo.DISPCLK_DIVIDER= 1;
				ClkInfo.DISPCLK_POL = 0;

				PMR_SetDispClk(&ClkInfo);
				DPC_InitHardware(DPC_MRGB_888B, CFALSE, CFALSE,CFALSE, 0, 0, DPC_USE_EXTCLK);
				DPC_UTIL_HVSYNC (DPC_MRGB_888B, 800, 600, 80, 32, 112, CTRUE, 10, 5, 10, CFALSE);
				DPC_Run();
			}
			else if ( arg == DISPLAY_TV_NTSC )              // TV_720x480Ix16_30Hz SVIDEO
			{

				CX25874(arg);
				/* Display Controller setup */
				DPC_Stop();

				ClkInfo.DISPCLK_SOURCE  = 0;
				ClkInfo.DISPCLK_DIVIDER= 1;
				ClkInfo.DISPCLK_POL = 0;

				PMR_SetDispClk(&ClkInfo);
				DPC_InitHardware(DPC_YCBCR_CCIR656, CTRUE, CFALSE,CFALSE, 0, 0, DPC_USE_EXTCLK);
				/* 858 x 262*/
				//DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, 240, 32, 96, 10, CTRUE, 5, 3, 15, CTRUE);    /* ORG */

				DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, 240, 32, 66, 40, CTRUE, 1, 1, 21, CTRUE);      //final
				hShift=40,vShift=21;
				vsh=240;

				DPC_UTIL_DATA_INV(CFALSE);
				MLC_isTVCheck(CTRUE);
				SetTvMode(TV_MODE_NTSC);
				tvFlag=TV_MODE_NTSC;
				DPC_Run();

			}
			else if ( arg == DISPLAY_TV_PAL )               //TV_720x576Ix16_50Hz SVIDEO
			{
				CX25874(arg);
				/* Display Controller setup */
				DPC_Stop();

				ClkInfo.DISPCLK_SOURCE  = 0;
				ClkInfo.DISPCLK_DIVIDER= 1;
				ClkInfo.DISPCLK_POL = 0;

				PMR_SetDispClk(&ClkInfo);
				DPC_InitHardware(DPC_YCBCR_CCIR656, CTRUE, CTRUE,CFALSE, 0, 0, DPC_USE_EXTCLK);
				/* total length 864 x 312 */
				DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, 288, 32, 72, 40, CTRUE, 2, 1, 22, CTRUE);
				hShift=40,vShift=22;
				vsh=288;

				DPC_UTIL_DATA_INV(CFALSE);
				MLC_isTVCheck(CTRUE);
				SetTvMode(TV_MODE_PAL);
				tvFlag=TV_MODE_PAL;

				DPC_Run();
			}

			ret = 1;
			break;

		default:
			ret = -EINVAL;
	}
	return ret;
}

/* ********************************************************************************* */
typedef struct _lcd_seting {
	unsigned int reg;
	unsigned int* pVal;
}lcd_seting;

#define MAX_REG					24
#define LCDOFF_REG_NUMBER		2
#define DEVICEID_LGPHILIPS		0x70

//senquack - defined in normal version of driver, no need to have dupes here
//unsigned int REG_NO[]= {0x01, 0x2, 0x03, 0x04, 0x05, 0x06, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x16, 0x17, 0x1E,
//									0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x3A, 0x3B };
//
//unsigned int valueREG[] = {0x6300, 0x200, 0x117, 0x4c7, 0xf7C0, 0xe806, 0x4008, 0x0, 0x30, 0x2800, 0x0,
//								0x9F80, 0xA0F, 0xBD,0x300, 0x107, 0x0, 0x0, 0x707, 0x4, 0x302, 0x202, 0xA0D, 0x806 };
//
//
//unsigned int LCDOFF_REGNO[]= {0x05, 0x01 };
//unsigned int lcdoff_value[]= { 0, 0, };
extern unsigned int REG_NO[24];
extern unsigned int valueREG[24];
extern unsigned int LCDOFF_REGNO[2];
extern unsigned int lcdoff_value[2];



#define CS_LOW			write_gpio_bit(GPIO_G15, 0)
#define CS_HIGH			write_gpio_bit(GPIO_G15, 1)

#define SCL_LOW			write_gpio_bit(GPIO_M8, 0)
#define SCL_HIGH		write_gpio_bit(GPIO_M8, 1)

#define SDA_LOW			write_gpio_bit(GPIO_M7, 0)
#define SDA_HIGH		write_gpio_bit(GPIO_M7, 1)

//senquack - already defined in original version of driver, we'll use it
//extern void lcdSetWrite(unsigned char id, lcd_seting *p);
void lcdSetWrite(unsigned char id, lcd_seting *p)
{
	int i;

	// START INDEX
	CS_LOW;
	udelay(50);

	for(i=7 ; i>=0 ; i--){
		if( id & (1<<i) )	SDA_HIGH;
		else SDA_LOW;

 		SCL_LOW;
		udelay(50);
		SCL_HIGH;
		udelay(50);
 	}

	for(i=15 ; i>=0 ; i--){
		if( p->reg & (1<<i) )	SDA_HIGH;
		else SDA_LOW;

		SCL_LOW;
		udelay(50);
		SCL_HIGH;
		udelay(50);
	}

	// END INDEX
	SDA_HIGH;
	udelay(50);
	CS_HIGH;
	udelay(50);
	udelay(50);
	udelay(50);

	// START INSTRUCTION
	CS_LOW;
	udelay(50);

	id |= 0x02;
	for(i=7 ; i>=0 ; i--)
	{
		if( id & (1<<i) )	SDA_HIGH;
		else SDA_LOW;


		SCL_LOW;
		udelay(50);
		SCL_HIGH;
		udelay(50);
	}

	for(i=15 ; i>=0 ; i--)
	{
		if( *(p->pVal) & (1<<i) )	SDA_HIGH;
		else SDA_LOW;


		SCL_LOW;
		udelay(50);
		SCL_HIGH;
		udelay(50);
	}

	//END INSTRUCTION
	SDA_HIGH;
	udelay(50);
	CS_HIGH;
	udelay(50);
}

//senquack - already defined in original version of driver, we'll use it
//extern void lcdTurnOn(void);
void lcdTurnOn(void)
{
	unsigned int i;
	unsigned char deviceID;
	lcd_seting setLcd;

	SCL_HIGH;
	SDA_HIGH;
	CS_HIGH;

	write_gpio_bit(GPIO_B3, 0);  // lcd reset
    mdelay(1);
    write_gpio_bit(GPIO_B3, 1);
    mdelay(1);

	setLcd.pVal = valueREG;
	deviceID = DEVICEID_LGPHILIPS;

	for(i=0; i < MAX_REG ; i++){
		setLcd.reg = REG_NO[i];
		lcdSetWrite(deviceID, &setLcd);
		setLcd.pVal++;
	}
}

//senquack - already defined in original version of driver, we'll use it
//extern void lcd_shutdown(void);
void lcd_shutdown(void)
{
	unsigned int i;
	unsigned char deviceID;
	lcd_seting setLcd;

	setLcd.pVal = lcdoff_value;
	deviceID = DEVICEID_LGPHILIPS;

	for(i=0; i < LCDOFF_REG_NUMBER ; i++){
		setLcd.reg = LCDOFF_REGNO[i];
		lcdSetWrite(deviceID, &setLcd);
		setLcd.pVal++;
	}

}

/* ********************************************************************************* */


static struct file_operations cx25874_open2x_fops = {
	owner:          THIS_MODULE,
	ioctl:          cx25874_open2x_ioctl,
	open:           cx25874_open2x_open,
	release:        cx25874_open2x_release,
};

static struct miscdevice cx25874_open2x_miscdev = {
	CX25874_OPEN2X_MINOR, "cx25874_open2x", &cx25874_open2x_fops
};

//senquack
//static int __init cx25874_init(void)
//{
//	unsigned char data;
//#if defined(CONFIG_MACH_GP2X_SVIDEO_NTSC) || defined(CONFIG_MACH_GP2X_SVIDEO_PAL)
//	tDispClkInfo ClkInfo;
//#endif
//
//	if(!i2c_set)
//	{
//		set_gpio_ctrl(GPIO_SCL, GPIOMD_IN, GPIOPU_EN);
//		set_gpio_ctrl(GPIO_SDA, GPIOMD_IN, GPIOPU_EN);
//	}
//
//	set_gpio_ctrl(GPIO_H5, GPIOMD_ALT1, GPIOPU_NOSET);      	//External Clock Setting for Video Encoder
//	set_gpio_ctrl(GPIO_B2, GPIOMD_OUT, GPIOPU_EN);
//	write_gpio_bit(GPIO_B2, 0);                             	// io sleep off...
//
//#if defined(CONFIG_MACH_GP2X_SVIDEO_NTSC) || defined(CONFIG_MACH_GP2X_SVIDEO_PAL)
//	/**************    Boot on TV  *************************/
//	write_gpio_bit(GPIO_B2, 0);                             	// Wake up...
//	/* CX25874 Reset */
//	set_gpio_ctrl(GPIO_B3, GPIOMD_OUT, GPIOPU_EN);
//	write_gpio_bit(GPIO_B3, 0);
//	mdelay(1);
//	write_gpio_bit(GPIO_B3, 1);
//	mdelay(1);
//
//	/* u-boot setting gpio setting */
//#ifndef CONFIG_MACH_GP2XF200
//	write_gpio_bit(GPIO_H1,0);                              	//LCD VGH oFF
//	write_gpio_bit(GPIO_H2,0);                              	//LCD Back oFF
//#else
//	write_gpio_bit(GPIO_F3,0);                              	//Sound to TV (AMP OFF)
//	write_gpio_bit(GPIO_H1,0);                              	//LCD AVDD oFF
//	write_gpio_bit(GPIO_L11,0);                             	//LCD Back oFF
//#endif
//
//	if(cx25874_count==0)
//	{
//		unsigned char data;
//		I2C_rbyte(CX25874_ID,0x00,&data);               		// Read Vender ID
//		if(data==0xC3)
//			printk("CX25874 ID = %2x \n",data);
//		else
//			printk("CX25874 ID check error = %2x \n",data);
//
//		I2C_wbyte(CX25874_ID,0xba,0x80);               			// Software Reset
//		mdelay(10);
//
//		cx25874_count = 1;
//	}
//	else
//		return -EBUSY;
//
//
//#ifdef CONFIG_MACH_GP2X_SVIDEO_NTSC
//	CX25874(DISPLAY_TV_NTSC);
//#else
//	CX25874(DISPLAY_TV_PAL);
//#endif
//		/* Display Controller setup */
//	DPC_Stop();
//
//	ClkInfo.DISPCLK_SOURCE  = 0;
//	ClkInfo.DISPCLK_DIVIDER= 1;
//	ClkInfo.DISPCLK_POL = 0;
//
//	PMR_SetDispClk(&ClkInfo);
//
//#ifdef CONFIG_MACH_GP2X_SVIDEO_NTSC
//	DPC_InitHardware(DPC_YCBCR_CCIR656, CTRUE, CFALSE,CFALSE, 0, 0, DPC_USE_EXTCLK);
//	DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, 240, 32, 66, 40, CTRUE, 1, 1, 21, CTRUE);
//	hShift=40,vShift=21;
//	vsh=240;
//#else   /* PAL */
//	DPC_InitHardware(DPC_YCBCR_CCIR656, CTRUE, CTRUE,CFALSE, 0, 0, DPC_USE_EXTCLK);
//	DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, 288, 32, 72, 40, CTRUE, 2, 1, 22, CTRUE);
//	hShift=40,vShift=22;
//	vsh=288;
//#endif
//
//	DPC_UTIL_DATA_INV(CFALSE);
//	write_gpio_bit(GPIO_H2,0);              		//LCD Backlignt off
//	MLC_isTVCheck(CTRUE);
//
//#ifdef CONFIG_MACH_GP2X_SVIDEO_NTSC
//	SetTvMode(TV_MODE_NTSC);
//	tvFlag=TV_MODE_NTSC;
//#else
//	SetTvMode(TV_MODE_PAL);
//	tvFlag=TV_MODE_PAL;
//#endif
//	DPC_Run();
//
//#else   /*****************************  LCD MODE ****************************************** */
//
//	I2C_rbyte(CX25874_ID,0x00,&data);                      	//Read Vender ID
//	if(data==0xC3){
//#ifdef CONFIG_MACH_GP2X_DEBUG
//		printk("CX25874 ID = %2x \n",data);
//#endif
//	}
//	else printk("CX25874 ID check error = %2x \n",data);
//
//	I2C_wbyte(CX25874_ID,0x30,0x10);  			//sleep en
//	mdelay(10);
//	I2C_wbyte(CX25874_ID,0x30,0x80);  			//sleep en
//	mdelay(10);
//	I2C_wbyte(CX25874_ID,0x30,0x90);  			//sleep en
//	mdelay(10);
//
//	write_gpio_bit(GPIO_B2, 1);           		// hw io sleep en.
//
//
//#endif /* CONFIG_MACH_GP2X_SVIDEO_NTSC || CONFIG_MACH_GP2X_SVIDEO_PAL */
//
//	misc_register(&cx25874_miscdev);
//	return 0;
//}
static int __init cx25874_open2x_init(void)
{
	//senquack - just get the tv flag that's already been set by the main cx25874 driver
	tvFlag = GetTVCheck();
	
//	unsigned char data;
//#if defined(CONFIG_MACH_GP2X_SVIDEO_NTSC) || defined(CONFIG_MACH_GP2X_SVIDEO_PAL)
//	tDispClkInfo ClkInfo;
//#endif
//
//	if(!i2c_set)
//	{
//		set_gpio_ctrl(GPIO_SCL, GPIOMD_IN, GPIOPU_EN);
//		set_gpio_ctrl(GPIO_SDA, GPIOMD_IN, GPIOPU_EN);
//	}
//
//	set_gpio_ctrl(GPIO_H5, GPIOMD_ALT1, GPIOPU_NOSET);      	//External Clock Setting for Video Encoder
//	set_gpio_ctrl(GPIO_B2, GPIOMD_OUT, GPIOPU_EN);
//	write_gpio_bit(GPIO_B2, 0);                             	// io sleep off...
//
//#if defined(CONFIG_MACH_GP2X_SVIDEO_NTSC) || defined(CONFIG_MACH_GP2X_SVIDEO_PAL)
//	/**************    Boot on TV  *************************/
//	write_gpio_bit(GPIO_B2, 0);                             	// Wake up...
//	/* CX25874 Reset */
//	set_gpio_ctrl(GPIO_B3, GPIOMD_OUT, GPIOPU_EN);
//	write_gpio_bit(GPIO_B3, 0);
//	mdelay(1);
//	write_gpio_bit(GPIO_B3, 1);
//	mdelay(1);
//
//	/* u-boot setting gpio setting */
//#ifndef CONFIG_MACH_GP2XF200
//	write_gpio_bit(GPIO_H1,0);                              	//LCD VGH oFF
//	write_gpio_bit(GPIO_H2,0);                              	//LCD Back oFF
//#else
//	write_gpio_bit(GPIO_F3,0);                              	//Sound to TV (AMP OFF)
//	write_gpio_bit(GPIO_H1,0);                              	//LCD AVDD oFF
//	write_gpio_bit(GPIO_L11,0);                             	//LCD Back oFF
//#endif
//
	if(cx25874_count==0)
	{
//		unsigned char data;
//		I2C_rbyte(CX25874_ID,0x00,&data);               		// Read Vender ID
//		if(data==0xC3)
//			printk("CX25874 ID = %2x \n",data);
//		else
//			printk("CX25874 ID check error = %2x \n",data);
//
//		I2C_wbyte(CX25874_ID,0xba,0x80);               			// Software Reset
//		mdelay(10);
//
		cx25874_count = 1;
	}
//	else
//		return -EBUSY;


//#ifdef CONFIG_MACH_GP2X_SVIDEO_NTSC
//	CX25874(DISPLAY_TV_NTSC);
//#else
//	CX25874(DISPLAY_TV_PAL);
//#endif
//		/* Display Controller setup */
//	DPC_Stop();
//
//	ClkInfo.DISPCLK_SOURCE  = 0;
//	ClkInfo.DISPCLK_DIVIDER= 1;
//	ClkInfo.DISPCLK_POL = 0;
//
//	PMR_SetDispClk(&ClkInfo);
//
//#ifdef CONFIG_MACH_GP2X_SVIDEO_NTSC
//	DPC_InitHardware(DPC_YCBCR_CCIR656, CTRUE, CFALSE,CFALSE, 0, 0, DPC_USE_EXTCLK);
//	DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, 240, 32, 66, 40, CTRUE, 1, 1, 21, CTRUE);
//	hShift=40,vShift=21;
//	vsh=240;
//#else   /* PAL */
//	DPC_InitHardware(DPC_YCBCR_CCIR656, CTRUE, CTRUE,CFALSE, 0, 0, DPC_USE_EXTCLK);
//	DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, 288, 32, 72, 40, CTRUE, 2, 1, 22, CTRUE);
//	hShift=40,vShift=22;
//	vsh=288;
//#endif
//
//	DPC_UTIL_DATA_INV(CFALSE);
//	write_gpio_bit(GPIO_H2,0);              		//LCD Backlignt off
//	MLC_isTVCheck(CTRUE);
//
//#ifdef CONFIG_MACH_GP2X_SVIDEO_NTSC
//	SetTvMode(TV_MODE_NTSC);
//	tvFlag=TV_MODE_NTSC;
//#else
//	SetTvMode(TV_MODE_PAL);
//	tvFlag=TV_MODE_PAL;
//#endif
//	DPC_Run();
//
//#else   /*****************************  LCD MODE ****************************************** */
//
//	I2C_rbyte(CX25874_ID,0x00,&data);                      	//Read Vender ID
//	if(data==0xC3){
//#ifdef CONFIG_MACH_GP2X_DEBUG
//		printk("CX25874 ID = %2x \n",data);
//#endif
//	}
//	else printk("CX25874 ID check error = %2x \n",data);
//
//	I2C_wbyte(CX25874_ID,0x30,0x10);  			//sleep en
//	mdelay(10);
//	I2C_wbyte(CX25874_ID,0x30,0x80);  			//sleep en
//	mdelay(10);
//	I2C_wbyte(CX25874_ID,0x30,0x90);  			//sleep en
//	mdelay(10);
//
//	write_gpio_bit(GPIO_B2, 1);           		// hw io sleep en.
//
//
//#endif /* CONFIG_MACH_GP2X_SVIDEO_NTSC || CONFIG_MACH_GP2X_SVIDEO_PAL */
//
//	misc_register(&cx25874_miscdev);
	misc_register(&cx25874_open2x_miscdev);
	return 0;
}

static void __exit cx25874_open2x_exit(void)
{
	misc_deregister(&cx25874_open2x_miscdev);
}

module_init(cx25874_open2x_init);
module_exit(cx25874_open2x_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
