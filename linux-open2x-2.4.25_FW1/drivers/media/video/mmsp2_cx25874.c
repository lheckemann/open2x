/*
 *  linux/drivers/media/video/mmsp2_cx25874.c
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
		case DISPLAY_LCD : break;
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
					I2C_wbyte(CX25874_ID, 0xc6, 0x50);	// EN_DOT=1, VSYNCI = 1, HSYNCI= 0
					I2C_wbyte(CX25874_ID, 0xce, 0x24);
					I2C_wbyte(CX25874_ID, 0xa0, PLL_INT);
					I2C_wbyte(CX25874_ID, 0x9e, (PLL_FRACT>>8));
					I2C_wbyte(CX25874_ID, 0x9c, (PLL_FRACT&0xff));
					I2C_wbyte(CX25874_ID, 0xd4, 0x80);	//MODE2X Bit 7 : 1 = Divides selected input clock by two (allows for single edge rather than double-edge
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
					I2C_wbyte(CX25874_ID, 0x76, 0xb4);	//H_CLKO 180
					I2C_wbyte(CX25874_ID, 0x78, 0xd0);	//H_ACTIVE 280
					I2C_wbyte(CX25874_ID, 0x7a, 0x7e); 	//H_WIDTH  126
					I2C_wbyte(CX25874_ID, 0x7c, 0x90);  //H_BUST_BEGIN 144
					I2C_wbyte(CX25874_ID, 0x7e, 0x58);  //H_BUST_END 88
					I2C_wbyte(CX25874_ID, 0x80, 0x03);  //h_BLANKO 3
					I2C_wbyte(CX25874_ID, 0x82, 0x14);	//v_BLANKO 20
					I2C_wbyte(CX25874_ID, 0x84, 0xf0);	//v_active 132
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
					I2C_wbyte(CX25874_ID, 0xa0, (PLL_INT | 0x80));	// EN_XCLK=1
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
					
					I2C_wbyte(CX25874_ID, 0x38, 0x10);	//DIV2 SET
					I2C_wbyte(CX25874_ID, 0x76, 0xc0);	//total hclk 0x6c0(1728) (with 0x86 0~3bit)
					I2C_wbyte(CX25874_ID, 0x78, 0xd0);	//H_ACTIVE 0x2D0(720) (with 0x86 4~6bit) ok...
					I2C_wbyte(CX25874_ID, 0x7a, 0x7e); 	//H_WIDTH  0x7E(126)
					I2C_wbyte(CX25874_ID, 0x7c, 0x98);  //H_BUST_BEGIN 0x98(152)
					I2C_wbyte(CX25874_ID, 0x7e, 0x54);  //H_BUST_END 0x54(84)
					I2C_wbyte(CX25874_ID, 0x80, 0x15);  //h_BLANKO 0x115(277) (with 0x9a 8~9bit) 	
					
					I2C_wbyte(CX25874_ID, 0x82, 0x17);	//v_BLANKO 0x23
					I2C_wbyte(CX25874_ID, 0x84, 0x20);	//v_active 0x120(288==(576/2)) (with 0x86 7bit)	ok...
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
					I2C_wbyte(CX25874_ID, 0xa0, (PLL_INT | 0x80));	// EN_XCLK=1
					
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
					
					I2C_wbyte(CX25874_ID, 0x38, 0x40);	//DIV2 SET
					I2C_wbyte(CX25874_ID, 0x76, 0x00);	//total hclk 0x6c0(1728) (with 0x86 0~3bit)
					I2C_wbyte(CX25874_ID, 0x78, 0x80);	//H_ACTIVE 0x2D0(720) (with 0x86 4~6bit) ok...
					I2C_wbyte(CX25874_ID, 0x7a, 0x84); 	//H_WIDTH  0x7E(126)
					I2C_wbyte(CX25874_ID, 0x7c, 0x96);  //H_BUST_BEGIN 0x98(152)
					I2C_wbyte(CX25874_ID, 0x7e, 0x60);  //H_BUST_END 0x54(84)
					I2C_wbyte(CX25874_ID, 0x80, 0x7d);  //h_BLANKO 0x115(277) (with 0x9a 8~9bit) 	
					
					I2C_wbyte(CX25874_ID, 0x82, 0x22);	//v_BLANKO 0x23
					I2C_wbyte(CX25874_ID, 0x84, 0xD5);	//v_active 0x120(288==(576/2)) (with 0x86 7bit)	ok...
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

static int cx25874_open(struct inode *inode, struct file *file)
{

#ifdef CONFIG_MACH_MMSP2_DTK4	
#ifdef CONFIG_MACH_GP2X		
	set_gpio_ctrl(GPIO_B2, GPIOMD_OUT, GPIOPU_EN); 		// CX25874 Sleep Mode 
	write_gpio_bit(GPIO_B2, 0);							// Wake up...
	set_gpio_ctrl(GPIO_B3, GPIOMD_OUT, GPIOPU_EN); 		// CX25874 Reset Mode 
	write_gpio_bit(GPIO_B3, 0);							// CX25874 Reset
	mdelay(1);
	write_gpio_bit(GPIO_B3, 1);
	mdelay(1);

	write_gpio_bit(GPIO_H1,0);		//LCD VGH oFF
	write_gpio_bit(GPIO_H2,0);		//LCD Back oFF
#else
	set_gpio_ctrl(GPIO_B2, GPIOMD_OUT, GPIOPU_EN); 		// CX25874 Sleep Mode 
	write_gpio_bit(GPIO_B2, 0);							// Wake up...
	set_gpio_ctrl(GPIO_B3, GPIOMD_OUT, GPIOPU_EN); 		// CX25874 Reset Mode 
	write_gpio_bit(GPIO_B3, 0);							// CX25874 Reset
	mdelay(1);
	write_gpio_bit(GPIO_B3, 1);
	mdelay(1);
#endif	
#endif
	
	printk("sleep oFF\n");
	if(cx25874_count==0) 
	{
		unsigned char data;

		I2C_rbyte(CX25874_ID,0x00,&data);				// Read Vender ID
		if(data==0xC3)
			printk("CX25874 ID = %2x \n",data);
		else
			printk("CX25874 ID chack error = %2x \n",data);

		I2C_wbyte(CX25874_ID,0xba,0x80);  					// Software Reset
		mdelay(10);

		cx25874_count = 1;
		return 0;
	}
	else
		return -EBUSY;
}

static int cx25874_release(struct inode *inode, struct file *file)
{
	tDispClkInfo ClkInfo;
	
	I2C_wbyte(CX25874_ID,0xba,0x80);  	// Software Reset
	mdelay(10);

	// Restore DPC
	/* Display Controller setup */
	DPC_Stop();
  
    ClkInfo.DISPCLK_SOURCE  = 2;

#ifdef CONFIG_MACH_GP2X
	ClkInfo.DISPCLK_DIVIDER= 16;
#else
    ClkInfo.DISPCLK_DIVIDER= 7;
#endif

	ClkInfo.DISPCLK_POL = 0;     
    PMR_SetDispClk(&ClkInfo);
	DPC_InitHardware(DPC_RGB_666, CFALSE, CFALSE,CFALSE, 0, 0, DPC_USE_PLLCLK);
#ifdef CONFIG_MACH_MMSP2_DTK3	
    DPC_UTIL_HVSYNC (DPC_RGB_666, 800, 600, 128, 216, 40, CTRUE, 4, 10, 33, CTRUE);
#elif defined(CONFIG_MACH_MMSP2_DTK4)
#ifdef CONFIG_MACH_GP2X
	set_gpio_ctrl(GPIO_B2, GPIOMD_OUT, GPIOPU_EN); 		// CX25874 Sleep Mode 
	write_gpio_bit(GPIO_B2, 1);							// io sleep on...
	
	DPC_UTIL_HVSYNC_GPX320240(DPC_RGB_666, 320,240,30, 4,53, CFALSE,4, 4, 15,CTRUE
								,CFALSE,CFALSE,CTRUE); 
	
	write_gpio_bit(GPIO_H2,1);		//LCD Backlignt on
	write_gpio_bit(GPIO_H1,1);		//LCD Vgh ON		 
#else
	DPC_UTIL_HVSYNC (DPC_RGB_666, 640, 480, 96, 24, 40, CTRUE, 2, 10, 33, CTRUE);
	write_gpio_bit(GPIO_N5, 1);		// VS_2
	write_gpio_bit(GPIO_N6, 0);		// VS_1
	write_gpio_bit(GPIO_N0, 1);		// LCD Backlignt on					
#endif
#endif	
	DPC_Run();

	cx25874_count = 0;
	MLC_isTVCheck(CFALSE);
	
	printk("release \n");
	return 0;
}

static int cx25874_ioctl(struct inode *inode, struct file *file,
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
				if( arg == TV_POS_LEFT )
				{
					if(hShift < 80)
					{
						
						hShift++;
						I2C_wbyte(CX25874_ID, 0x8c,hShift);
					}						
						
				}
				else if ( arg == TV_POS_RIGHT )
				{
					
					if(hShift > 3){
						hShift--;
						I2C_wbyte(CX25874_ID, 0x8c,hShift);
					}					
				}	
				else if ( arg == TV_POS_UP )	
				{
					
					if(tvFlag == TV_MODE_NTSC)
					{
						
						if(vsh < 243)
						{	
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
				else if ( arg == TV_POS_DOWN )
				{
					if(tvFlag == TV_MODE_NTSC)//ntsc
					{
						if(vsh > 225)
						{
							vShift++,vsh--;
							DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, vsh, 32, 66, 40, CTRUE, 1, 1, vShift, CTRUE);	
						}
					}
					else
					{
#if 0						
						if(vsh > 265)
						{
							vShift++,vsh--;
							DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, vsh, 32, 66, 40, CTRUE, 2, 1, vShift, CTRUE);	
						}	
#endif	
					}
				}	
				ret=1;
				break;
				 		
		case IOCTL_CX25874_DISPLAY_MODE_SET :
				if( arg == DISPLAY_LCD ) 			// LCD
				{
					CX25874(arg);

					/* Display Controller setup */
					DPC_Stop();

				    ClkInfo.DISPCLK_SOURCE  = 1;
#ifdef CONFIG_MACH_GP2X
					ClkInfo.DISPCLK_DIVIDER= 25;
#else
    				ClkInfo.DISPCLK_DIVIDER= 7;
#endif
				    ClkInfo.DISPCLK_POL = 0;     

				    PMR_SetDispClk(&ClkInfo);
				    DPC_InitHardware(DPC_RGB_666, CFALSE, CFALSE,CFALSE, 0, 0, DPC_USE_PLLCLK);
#ifdef CONFIG_MACH_MMSP2_DTK3	
				    DPC_UTIL_HVSYNC (DPC_RGB_666, 800, 600, 128, 216, 40, CTRUE, 4, 10, 33, CTRUE);
#elif defined(CONFIG_MACH_MMSP2_DTK4)
#ifdef CONFIG_MACH_GP2X
					DPC_UTIL_HVSYNC_GPX320240(DPC_RGB_666, 320, 240,30, 20, 38, CTRUE,4, 15, 4,CTRUE 
								,CFALSE,CFALSE,CTRUE); 
					write_gpio_bit(GPIO_H2,1);		//LCD Backlignt on				 
#else
					DPC_UTIL_HVSYNC (DPC_RGB_666, 640, 480, 96, 24, 40, CTRUE, 2, 10, 33, CTRUE);
					write_gpio_bit(GPIO_N5, 1);		// VS_2
					write_gpio_bit(GPIO_N6, 0);		// VS_1
					write_gpio_bit(GPIO_N0, 1);		// LCD Backlignt on					
#endif
#endif
					DPC_Run();				
				}				
				else if (arg == DISPLAY_MONITOR )	// ARGB_800x600x16_60Hz
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
#ifdef CONFIG_MACH_MMSP2_DTK4
					write_gpio_bit(GPIO_N5, 1);		// VS_2
					write_gpio_bit(GPIO_N6, 1);		// VS_1
					write_gpio_bit(GPIO_N0, 0);		// LCD Backlignt off
#endif				
					
					DPC_Run();						
				}				
				else if ( arg == DISPLAY_TV_NTSC )		// TV_720x480Ix16_30Hz SVIDEO
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
					//DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, 240, 32, 96, 10, CTRUE, 5, 3, 15, CTRUE);	/* ORG */
					
					DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, 240, 32, 66, 40, CTRUE, 1, 1, 21, CTRUE);	//final
					hShift=40,vShift=21;
					vsh=240;
#ifdef CONFIG_MACH_GP2X				
					
					DPC_UTIL_DATA_INV(CFALSE);
					write_gpio_bit(GPIO_H2,0);		//LCD Backlignt off					
					MLC_isTVCheck(CTRUE);
					SetTvMode(TV_MODE_NTSC);	
					tvFlag=TV_MODE_NTSC;	
#endif		
#ifdef CONFIG_MACH_MMSP2_DTK4
#ifndef CONFIG_MACH_GP2X	
					write_gpio_bit(GPIO_N5, 0);		// VS_2
					write_gpio_bit(GPIO_N6, 1);		// VS_1
					write_gpio_bit(GPIO_N0, 0);		// LCD Backlignt off					
#endif
#endif				
					DPC_Run();
					
				}
				else if ( arg == DISPLAY_TV_PAL )		//TV_720x576Ix16_50Hz SVIDEO
				{
					CX25874(arg);
					/* Display Controller setup */					
					DPC_Stop();
					
				    ClkInfo.DISPCLK_SOURCE  = 0;
				    ClkInfo.DISPCLK_DIVIDER= 1;
				    ClkInfo.DISPCLK_POL = 0;     

				    PMR_SetDispClk(&ClkInfo);
				    //DPC_InitHardware(DPC_YCBCR_CCIR656, CTRUE, CFALSE,CFALSE, 0, 0, DPC_USE_EXTCLK);
															//PAL MODE
					DPC_InitHardware(DPC_YCBCR_CCIR656, CTRUE, CTRUE,CFALSE, 0, 0, DPC_USE_EXTCLK);
					/* total length 864 x 312 */									 
					//DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, 288, 22, 112, 10, CTRUE, 2, 1, 22, CTRUE);	
					//DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, 288, 22, 82, 40, CTRUE, 2, 1, 22, CTRUE);	
					DPC_UTIL_HVSYNC (DPC_YCBCR_CCIR656, 720, 288, 32, 72, 40, CTRUE, 2, 1, 22, CTRUE);	
					hShift=40,vShift=22;
					vsh=288;
#ifdef CONFIG_MACH_GP2X				
					DPC_UTIL_DATA_INV(CFALSE);
					write_gpio_bit(GPIO_H2,0);		//LCD Backlignt off					
					MLC_isTVCheck(CTRUE);	
					SetTvMode(TV_MODE_PAL);
					tvFlag=TV_MODE_PAL;	
#endif		
					DPC_Run();
					
				}
				else if ( arg == DISPLAY_TV_GAME_NTSC )		// TV_320x240Ix16_ SVIDEO
				{
					CX25874(arg);
					/* Display Controller setup */					
					DPC_Stop();
					
				    ClkInfo.DISPCLK_SOURCE  = 0;
				    ClkInfo.DISPCLK_DIVIDER= 1;
				    ClkInfo.DISPCLK_POL = 0;     

				    PMR_SetDispClk(&ClkInfo);
				   	DPC_InitHardware(DPC_MRGB_565, CFALSE, CFALSE,CFALSE, 0, 0, DPC_USE_EXTCLK);
					//DPC_InitHardware(DPC_MRGB_565, CFALSE, CFALSE,CFALSE, 0, 0, DPC_USE_PLLCLK);
					/* total length 1568 x 300 */									 
					DPC_UTIL_HVSYNC (DPC_MRGB_565, 320, 240, 378, 521, 349, CTRUE, 13, 10, 37, CTRUE);	
#ifdef CONFIG_MACH_GP2X				
					DPC_UTIL_DATA_INV(CFALSE);
					write_gpio_bit(GPIO_H2,0);		//LCD Backlignt off					
					MLC_isTVCheck(CTRUE);	
#endif		
					DPC_Run();
				}
				break;
			ret = 1;
		default:
			ret = -EINVAL;
	}
	return ret;
}

static struct file_operations cx25874_fops = {
	owner:		THIS_MODULE,
	ioctl:		cx25874_ioctl,
	open:		cx25874_open,
	release:	cx25874_release,
};

static struct miscdevice cx25874_miscdev = {
	CX25874_MINOR, "cx25874", &cx25874_fops
};

static int __init cx25874_init(void)
{
	unsigned char data;
	
	if(!i2c_set)
	{
		set_gpio_ctrl(GPIO_SCL, GPIOMD_IN, GPIOPU_EN);
		set_gpio_ctrl(GPIO_SDA, GPIOMD_IN, GPIOPU_EN); 
	}
	
	set_gpio_ctrl(GPIO_H5, GPIOMD_ALT1, GPIOPU_NOSET);	//External Clock Setting for Video Encoder

#if 1	
	set_gpio_ctrl(GPIO_B2, GPIOMD_OUT, GPIOPU_EN); 		// CX25874 Sleep Mode 
	write_gpio_bit(GPIO_B2, 0);							// io sleep off...
#endif
		

#ifdef CONFIG_MACH_MMSP2_DTK4	
#ifndef CONFIG_MACH_GP2X
	set_gpio_ctrl(GPIO_N5, GPIOMD_OUT, GPIOPU_NOSET); 	// Video Switch 2 (VS_2) 
	set_gpio_ctrl(GPIO_N6, GPIOMD_OUT, GPIOPU_NOSET); 	// Video Switch 1 (VS_1)
#endif
#endif
	
#if 1
	
	I2C_rbyte(CX25874_ID,0x00,&data);						//Read Vender ID
	if(data==0xC3)
		printk("CX25874 ID = %2x \n",data);
	else
		printk("CX25874 ID chack error = %2x \n",data);
#if 1
	I2C_wbyte(CX25874_ID,0x30,0x10);  						//sleep en
		mdelay(10);
	I2C_wbyte(CX25874_ID,0x30,0x80);  						//sleep en
		mdelay(10);
	I2C_wbyte(CX25874_ID,0x30,0x90);  						//sleep en
		mdelay(10);
#endif		
	
/*
	I2C_wbyte(CX25874_ID,0x30,0x30);  						//sleep en
		mdelay(10);
	I2C_wbyte(CX25874_ID,0x30,0xA0);  						//sleep en
		mdelay(10);
	I2C_wbyte(CX25874_ID,0x30,0xF0);  						//sleep en
		mdelay(10);
*/

	set_gpio_ctrl(GPIO_B2, GPIOMD_OUT, GPIOPU_EN); 		// CX25874 Sleep Mode 
	write_gpio_bit(GPIO_B2, 1);							// io sleep on...
	
#endif
	
	misc_register(&cx25874_miscdev);
	return 0;
}

static void __exit cx25874_exit(void)
{
	misc_deregister(&cx25874_miscdev);
}

module_init(cx25874_init);
module_exit(cx25874_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
