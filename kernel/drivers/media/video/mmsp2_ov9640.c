/*
 *  linux/drivers/media/video/mmsp2_ov9640.c
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 *  OV9640 Image Sensor Driver
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

#include "ov9640.h"

extern int i2c_set;
static int ov9640_count=0;

unsigned long cambuffer;
unsigned char *cam;

static unsigned char OV9640_Data[]	=
{
//	   0	 1	   2	 3	   4	 5	   6	 7	   8	 9	   A	 B	   C	 D	   E	 F
	0x00, 0x49,	0x30, 0x4A,	0x02, 0x66,	0x60, 0x03,	0x86, 0x01,	0x96, 0x49,	0x00, 0x40,	0x01, 0x4F,	// 0x00	- 0x0F
	0xF9, 0x00,	0x40, 0x8D,	0x0A, 0x00,	0x03, 0x25,	0xC5, 0x01,	0xF1, 0x01,	0x7F, 0xA2,	0x20, 0x00,	// 0x10	- 0x1F
	0x10, 0x8E,	0x95, 0x0F,	0x78, 0x70,	0xD3, 0x90,	0x80, 0x80,	0x00, 0x00,	0x80, 0x00,	0x00, 0x60,	// 0x20	- 0x2F
	0x08, 0x30,	0xA4, 0x08,	0x03, 0x90,	0xC9, 0x04,	0x12, 0xF0,	0x08, 0x00,	0x46, 0x90,	0x02, 0xF6,	// 0x30	- 0x3F
	0xC0, 0x02,	0xC8, 0xF0,	0x10, 0x5C,	0x70, 0x46,	0x4D, 0x00,	0x00, 0x00,	0x00, 0x00,	0x00, 0x70,	// 0x40	- 0x4F
	0x5E, 0x12,	0x2C, 0x86,	0xB4, 0x40,	0x40, 0x40,	0x0F, 0x1F,	0x55, 0x43,	0x7C, 0x54,	0x0A, 0x05,	// 0x50	- 0x5F
	0x14, 0xCE,	0x00, 0x00,	0x10, 0x80,	0x00, 0x80,	0x80, 0x00,	0x00, 0x3A,	0x50, 0x60,	0x58, 0x58,	// 0x60	- 0x6B
	0x58, 0x50,	0x50, 0x50,	0x50, 0x50,	0x4c, 0x4c,	0x45, 0x3c,	0x2c, 0x24,	0x05, 0x0b,	0x16, 0x2c,	// 0x70	- 0x7F
	0x37, 0x41,	0x4b, 0x55,	0x5f, 0x69,	0x7c, 0x8f,	0xb1, 0xcf,	0xe5, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	// 0x80	- 0x8F
	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	// 0x90	- 0x9F
	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	// 0xA0	- 0xAF
	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	// 0xB0	- 0xBF
	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	// 0xC0	- 0xCF
	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0xE6,	0xE6, 0x0F,	0x0F, 0x0F,	// 0xD0	- 0xDF
	0x0F, 0x0F,	0x0F, 0x0F,	0x0F, 0x0F,	0x0F, 0x0F,	0x0F, 0x0F,	0x0F, 0x0F,	0x0F, 0x0F,	0x0F, 0x0F,	// 0xE0	- 0xEF
	0x0F, 0x0F,	0x0F, 0x0F,	0x0F, 0x0F,	0x0F, 0x0F,	0x0F, 0x0F,	0x0F, 0x0F,	0x0F, 0x0F,	0x0F		// 0xF0	- 0xFF
};

extern int I2C_wbyte( unsigned char id, unsigned char addr, unsigned char data );
extern int I2C_rbyte( unsigned char id, unsigned char addr, unsigned char *pData );

//---------------------------//
// OmniVison OV9640 & OV7620 //
//---------------------------//

static int ov9640_open(struct inode *inode, struct file *file)
{
	if(ov9640_count==0) {
		set_gpio_ctrl(CIS_PWDN, GPIOMD_OUT, GPIOPU_EN);
		write_gpio_bit(CIS_PWDN, 0);	// Camera Module Power On.

		set_gpio_ctrl(CIS_RESET, GPIOMD_OUT, GPIOPU_EN);
		write_gpio_bit(CIS_RESET, 1);	// CIS Reset
		mdelay(1);
		write_gpio_bit(CIS_RESET, 0);
		// OV9640 ID & VER Check
		{
			unsigned char data;
			I2C_rbyte(OV9640_ID,0x0A,&data); 				//id check
			if(data==0x96)
				printk("OV9640 Cam ID = %2x \n",data);
			else
				printk("OV9640 Cam Error = %2x \n",data);
			I2C_rbyte(OV9640_ID,0x0B,&data); 				//Ver check
			if(data==0x49)
				printk("OV9640 Cam Ver = %2x \n",data);
			else
				printk("OV9640 Cam Ver Error = %2x \n",data);
		}
		// Reset
		I2C_wbyte(OV9640_ID, 0x12, 0x80	);	// Reset
		mdelay(10);

		ov9640_count = 1;
		return 0;
	}
	else
		return -EBUSY;
}

static int ov9640_release(struct inode *inode, struct file *file)
{
	write_gpio_bit(CIS_RESET, 1);
	set_gpio_ctrl(CIS_RESET, GPIOMD_OUT, GPIOPU_DIS);

	write_gpio_bit(CIS_PWDN, 1); 						//Camera Module Power Off
	set_gpio_ctrl(CIS_PWDN, GPIOMD_OUT, GPIOPU_DIS); 	// CIS Power Down Mode

	ov9640_count = 0;
	return 0;
}

static int ov9640_ioctl(struct inode *inode, struct file *file,
                     unsigned int cmd, unsigned long arg)
{
	int ret=0,i;
	I2C_WRITE_BYTE_INFO i2c_write_info;
	I2C_READ_BYTE_INFO i2c_read_info;

	switch(cmd)
	{
		case OV9640_I2C_WRITE_BYTE:
				if(copy_from_user(&i2c_write_info, (I2C_WRITE_BYTE_INFO *)arg, sizeof(I2C_WRITE_BYTE_INFO)))
					return -EFAULT;
				if(i2c_write_info.id != OV9640_ID) i2c_write_info.id = OV9640_ID;
				mdelay(1);
				I2C_wbyte(i2c_write_info.id, i2c_write_info.addr, i2c_write_info.data);
				ret = 1;
				break;

		case OV9640_I2C_READ_BYTE :
				if(copy_from_user(&i2c_read_info, (I2C_READ_BYTE_INFO *)arg, sizeof(I2C_READ_BYTE_INFO)))
					return -EFAULT;
				if(i2c_write_info.id != OV9640_ID) i2c_write_info.id = OV9640_ID;
				I2C_rbyte(i2c_read_info.id, i2c_read_info.addr, i2c_read_info.pdata);
				ret = 1;
				break;
		case IOCTL_OV9640_RESET :
				mdelay(10);
				I2C_wbyte(OV9640_ID, 0x12, 0x80	);	// Soft Reset
				mdelay(10);

				for( i=0 ; i<=0xFE ; i++ ) // OV9640 Camera Data Init
				{
					I2C_wbyte(OV9640_ID, i, OV9640_Data[i]);
				}
				ret = 1;
				break;
		case IOCTL_OV9640_CAMADDR :
				if(copy_to_user((unsigned long *)arg,&cambuffer, sizeof(unsigned long)))
					return -EFAULT;
				ret = 1 ;
		default:
			ret = -EINVAL;
	}
	return ret;
}

static struct file_operations ov9640_fops = {
	owner:		THIS_MODULE,
	ioctl:		ov9640_ioctl,
	open:		ov9640_open,
	release:	ov9640_release,
};

static struct miscdevice ov9640_miscdev = {
	OV9640_MINOR, "ov9640", &ov9640_fops
};

static int __init ov9640_init(void)
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


	/* SETUP for RGB/CCIR 656 Format DATA[7:0] */
	GPIOCALTFNLOW = 0xaaaa;

	/* setup GPIO alt.function CMOS Image Sensor */
	set_gpio_ctrl(CIS_CLKO,  GPIOMD_ALT1, GPIOPU_NOSET); // VIN_ICLKOUT
	set_gpio_ctrl(CIS_VSYNC, GPIOMD_ALT1, GPIOPU_NOSET); // VIN_VCLK0
	set_gpio_ctrl(CIS_HSYNC, GPIOMD_ALT1, GPIOPU_NOSET); // VIN_HSYNC0
	set_gpio_ctrl(CIS_CLKI,  GPIOMD_ALT1, GPIOPU_NOSET); // VIN_VSYNC0

	set_gpio_ctrl(CIS_PWDN, GPIOMD_OUT, GPIOPU_DIS);
	set_gpio_ctrl(CIS_RESET, GPIOMD_OUT, GPIOPU_DIS);

	cam = (u_char *) VA_CAM_BASE;			// see include/asm/arch/hardware.h
	cambuffer = (unsigned long) PA_CAM_BASE;	// Cam address fix..

	misc_register(&ov9640_miscdev);
	return 0;
}

static void __exit ov9640_exit(void)
{
	misc_deregister(&ov9640_miscdev);
}

module_init(ov9640_init);
module_exit(ov9640_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
