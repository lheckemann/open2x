/*
 *  linux/drivers/media/video/mmsp2_saa7113h.c
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 *  SAA7113H Video Decoder Driver
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
#include <asm/arch/proto_pwrman.h>

#include "saa7113h.h"

extern int i2c_set;
static int saa7113h_count=0;

static unsigned char SAA7113H_Data[]	=
{
        0x01, 0x08,
        0x02, 0xc8,
        0x03, 0x33,
        0x04, 0x00,
        0x05, 0x00,
        0x06, 0xe9,
        0x07, 0x0d,

        0x08, 0xd8,
        0x09, 0x81,
        0x0a, 0x80,
        0x0b, 0x47,
        0x0c, 0x40,
        0x0d, 0x00,
        0x0e, 0x01,
        0x0f, 0x24,

        0x10, 0x00,
        0x11, 0x1C,
        0x12, 0x01,
        0x13, 0x00,
        0x15, 0x06,
        0x16, 0x00,
        0x17, 0x00,

        0x40, 0x82,
        0x58, 0x40,
        0x59, 0x54,
        0x5a, 0x0a,
        0x5b, 0x83,
        0x5e, 0x00,
};

extern int I2C_wbyte( unsigned char id, unsigned char addr, unsigned char data );
extern int I2C_rbyte( unsigned char id, unsigned char addr, unsigned char *pData );

//---------------------------//
// 		Philips SAA7113H 	 //
//---------------------------//

static int saa7113h_open(struct inode *inode, struct file *file)
{
	if(saa7113h_count==0) {
		int i;

		write_gpio_bit(GPIO_G15, 1); // Chip enable
		mdelay(1);
		// SAA7113H ID Check
		{
			unsigned char data;
			I2C_rbyte(SAA7113H_ID,0x00,&data); 				//id check
			printk("SAA7113H ID = %2x \n",data);			//11:success, ff:fail
		}

		for( i=0 ; i<28 ; i++ ) // SAA7113H Video Decoder Init
		{
			unsigned char data;
			I2C_wbyte(SAA7113H_ID, SAA7113H_Data[i*2], SAA7113H_Data[i*2+1]);
			I2C_rbyte(SAA7113H_ID,SAA7113H_Data[i*2], &data);
//			printk("(0x%x) = %x\n",SAA7113H_Data[i*2],data);
		}

		saa7113h_count = 1;
		return 0;
	}
	else
		return -EBUSY;
}

static int saa7113h_release(struct inode *inode, struct file *file)
{
	write_gpio_bit(GPIO_G15, 0); // Chip disable

	saa7113h_count = 0;
	return 0;
}

static int saa7113h_ioctl(struct inode *inode, struct file *file,
                     unsigned int cmd, unsigned long arg)
{
	int ret=0,i;
	I2C_WRITE_BYTE_INFO i2c_write_info;
	I2C_READ_BYTE_INFO i2c_read_info;

	switch(cmd)
	{
		case SAA7113H_I2C_WRITE_BYTE:
				if(copy_from_user(&i2c_write_info, (I2C_WRITE_BYTE_INFO *)arg, sizeof(I2C_WRITE_BYTE_INFO)))
					return -EFAULT;
				if(i2c_write_info.id != SAA7113H_ID) i2c_write_info.id = SAA7113H_ID;
				mdelay(1);
				I2C_wbyte(i2c_write_info.id, i2c_write_info.addr, i2c_write_info.data);
				ret = 1;
				break;

		case SAA7113H_I2C_READ_BYTE :
				if(copy_from_user(&i2c_read_info, (I2C_READ_BYTE_INFO *)arg, sizeof(I2C_READ_BYTE_INFO)))
					return -EFAULT;
				if(i2c_write_info.id != SAA7113H_ID) i2c_write_info.id = SAA7113H_ID;
				I2C_rbyte(i2c_read_info.id, i2c_read_info.addr, i2c_read_info.pdata);
				ret = 1;
				break;

		case SAA7113H_SET_SOURCE :
				if(arg == ISPSOURCE_SVIDEO_INPUT){
					I2C_wbyte(SAA7113H_ID, 0x02, 0xC8);
					I2C_wbyte(SAA7113H_ID, 0x08, 0xD8);	// Fase Locking mode
					I2C_wbyte(SAA7113H_ID, 0x09, 0x81);	// BYPASS = 1
				} else if (arg = ISPSOURCE_COMPOSITE_INPUT) {
					I2C_wbyte(SAA7113H_ID, 0x02, 0xC1);
					I2C_wbyte(SAA7113H_ID, 0x08, 0xD8); // Fast Locking mode
					I2C_wbyte(SAA7113H_ID, 0x09, 0x01);	// BYPASS = 0
				} else break;
				ret = 1;
				break;

		default:
			ret = -EINVAL;
	}
	return ret;
}

static struct file_operations saa7113h_fops = {
	owner:		THIS_MODULE,
	ioctl:		saa7113h_ioctl,
	open:		saa7113h_open,
	release:	saa7113h_release,
};

static struct miscdevice saa7113h_miscdev = {
	SAA7113H_MINOR, "saa7113h", &saa7113h_fops
};

static int __init saa7113h_init(void)
{
	tISPClkInfo ClkInfo;

	if(!i2c_set)
	{
		set_gpio_ctrl(GPIO_SCL, GPIOMD_IN, GPIOPU_EN);
		set_gpio_ctrl(GPIO_SDA, GPIOMD_IN, GPIOPU_EN);
		i2c_set=1;
	}

	ClkInfo.ISPCLK_SOURCE = 2;
	ClkInfo.ISPCLK_DIVIDER = 3;
	PMR_SetISPClk(&ClkInfo);	// clk = UPIL/(3+1) = 96Mhz / 4 = 24Mhz

	/* setup GPIO Video Decoder (SA7113H) */
	set_gpio_ctrl(GPIO_M0, GPIOMD_ALT1, GPIOPU_NOSET);	// V656_CLK
	set_gpio_ctrl(GPIO_M1, GPIOMD_ALT1, GPIOPU_NOSET); 	// V656_D0
	set_gpio_ctrl(GPIO_M2, GPIOMD_ALT1, GPIOPU_NOSET); 	// V656_D1
	set_gpio_ctrl(GPIO_M3, GPIOMD_ALT1, GPIOPU_NOSET); 	// V656_D2
	set_gpio_ctrl(GPIO_M4, GPIOMD_ALT1, GPIOPU_NOSET); 	// V656_D3
	set_gpio_ctrl(GPIO_M5, GPIOMD_ALT1, GPIOPU_NOSET); 	// V656_D4
	set_gpio_ctrl(GPIO_M6, GPIOMD_ALT1, GPIOPU_NOSET); 	// V656_D5
	set_gpio_ctrl(GPIO_M7, GPIOMD_ALT1, GPIOPU_NOSET); 	// V656_D6
	set_gpio_ctrl(GPIO_M8, GPIOMD_ALT1, GPIOPU_NOSET); 	// V656_D7

	set_gpio_ctrl(GPIO_G15, GPIOMD_OUT, GPIOPU_NOSET); 	// DEC_CE (Chip enable/disable)
	write_gpio_bit(GPIO_G15, 0); // Chip disable

	misc_register(&saa7113h_miscdev);
	return 0;
}

static void __exit saa7113h_exit(void)
{
	misc_deregister(&saa7113h_miscdev);
}

module_init(saa7113h_init);
module_exit(saa7113h_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
