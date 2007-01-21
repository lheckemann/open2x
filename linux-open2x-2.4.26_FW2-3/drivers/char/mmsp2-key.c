/*
 * drivers/char/mmsp2-key.c
 *
 * Copyright (C) 2005,2006 Gamepark Holdings, Inc. (www.gp2x.com)
 * Hyun <hyun3678@gp2x.com>
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/arch/proto_gpio.h>
#include <asm/arch/mmsp20.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/ioport.h>
#include <linux/slab.h>     // kmalloc()
#include <linux/poll.h>     // poll

#include <asm/delay.h>
#include <asm/ioctl.h>
#include <asm/unistd.h>


#define MMSP2_KEY_MAJOR 	253
#define MMSP2_KEY_NAME     	"MMSP2 GPIO KEY"
#define MMSP2_KEY_VERSION  	"Version 0.1"
#define DRIVER_NAME     	"MMSP2 GPIO KEY"

#define FUNC1_KEY_BASE		GPIO_C8
#define FUNC2_KEY_BASE		GPIO_M0
#define VOL_UP				GPIO_D7
#define VOL_DOWN			GPIO_D6
#define TACT_SW				GPIO_D11

#define MAX_FUNC_KEY		8

/* IOCTL  CMD*/
#define BACK_LIGHT_OFF		0
#define BACK_LIGHT_ON		1
#define BATT_LED_ON			2
#define BATT_LED_OFF		3

#define FCLK_200			10
#define FCLK_166			11
#define FCLK_133			12
#define FCLK_100			13
#define FCLK_78				14
#define FCLK_64				15
#define FCLK_DEFUALT		16
#define SD_CLK_MODE_0		17		/* 5Mhz */
#define SD_CLK_MODE_1		18		/* 15Mhz */
#define SD_CLK_MODE_2		19		/* 25Mhz */

/* UCLK = 95.xxxMHz. It's default value. */
#define U_MDIV		(0x60)
#define U_PDIV		(0x0)
#define U_SDIV		(0x2)

/* ACLK = 147456000. It's default value. */
#define A_MDIV 		0x98
#define A_PDIV 		0x0
#define A_SDIV 		0x2


struct timeval cur_tv;
static unsigned int old_tv_usec = 0;

//This function returns 0 if the allowed microseconds have elapsed since the last call to ths function, otherwise it returns 1 to indicate a bounce condition
static unsigned int bounce()
{

    unsigned int elapsed_time;

    do_gettimeofday (&cur_tv);

    if (!old_tv_usec)
    {
        //init condition
        old_tv_usec = cur_tv.tv_usec;
        return 0;
    }

    if(cur_tv.tv_usec > old_tv_usec)
    {
        // If there hasn't been rollover
        elapsed_time =  ((cur_tv.tv_usec - old_tv_usec));
    }
    else
    {
        // Accounting for rollover
        elapsed_time =  ((1000000 - old_tv_usec + cur_tv.tv_usec));
    }

    if (elapsed_time > 250000)
    {
        old_tv_usec = 0;	//reset the bounce time
        return 0;
    }

    return 1;

}


int MMSP2key_open(struct inode *inode, struct file *filp)
{
	MOD_INC_USE_COUNT;
	return (0);          /* success */
}

int MMSP2key_release(struct inode *inode, struct file *filp)
{
	MOD_DEC_USE_COUNT;
	return (0);
}

ssize_t MMSP2key_read(struct file *filp, char *Putbuf, size_t length, loff_t *f_pos)
{
	int i;
	unsigned long keyValue=0;
	unsigned char keyTemp=0;

		/* Check KEY */
	for(i=0;i<MAX_FUNC_KEY;i++)
		keyTemp|=((read_gpio_bit(FUNC2_KEY_BASE+i)) << i);
	keyValue=(unsigned long)~keyTemp;

	keyTemp=0;
	for(i=0;i<MAX_FUNC_KEY;i++)
		keyTemp|=((read_gpio_bit(FUNC1_KEY_BASE+i)) << i);
	keyValue&=(unsigned long)(((~keyTemp) << 8)|0xFF);

	keyTemp=0;
	keyTemp=read_gpio_bit(VOL_UP);
	keyTemp|=read_gpio_bit(VOL_DOWN)<<1;
	keyTemp|=read_gpio_bit(TACT_SW)<<2;
	keyValue&=(unsigned long)(((~keyTemp)<<16)|0xFFFF);

/*
	if(bounce())
		return 1;
*/
   	copy_to_user( Putbuf, &keyValue, 4);

	return length;
}


int MMSP2key_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,unsigned long arg)
{
	MMSP20_CLOCK_POWER *pPMR_REG;
	pPMR_REG = MMSP20_GetBase_CLOCK_POWER();

	switch(arg)
	{
		case 0:
			write_gpio_bit(GPIO_H1,0);		//LCD VGH oFF
			write_gpio_bit(GPIO_H2,0);		//LCD Back oFF
			break;
		case 1:
			write_gpio_bit(GPIO_H1,1);		//LCD VGH on
			write_gpio_bit(GPIO_H2,1);		//LCD Back on
			break;
		case BATT_LED_ON:
			write_gpio_bit(GPIO_H4,0);		//Batt LED on
			break;
		case BATT_LED_OFF:
			write_gpio_bit(GPIO_H4,1);		//Batt LED oFF
			break;
		case FCLK_200:
			pPMR_REG->FPLLSETVREG  = ((0x49 << 8) + (1 << 2) + 0);
			udelay(4000);
			break;
		case FCLK_166:
			pPMR_REG->FPLLSETVREG  = ((0x3B << 8) + (1 << 2) + 0);
			udelay(4000);
			break;
		case FCLK_133:
			pPMR_REG->FPLLSETVREG  = ((0x2E << 8) + (1 << 2) + 0);
			udelay(4000);
			break;
		case FCLK_100:
			pPMR_REG->FPLLSETVREG  = ((0x20 << 8) + (1 << 2) + 0);
			udelay(4000);
			break;
		case FCLK_78:
			pPMR_REG->FPLLSETVREG  = ((0x18 << 8) + (1 << 2) + 0);
			udelay(4000);
			break;
		case FCLK_64:
			pPMR_REG->FPLLSETVREG  = ((0x12 << 8) + (1 << 2) + 0);
			udelay(4000);
			break;
		case SD_CLK_MODE_0:
			mmsp_set_SDCLK(0);
			break;
		case SD_CLK_MODE_1:
			mmsp_set_SDCLK(1);
			break;
		case SD_CLK_MODE_2:
			mmsp_set_SDCLK(2);
			break;
		case FCLK_DEFUALT:
			break;
	}

	return 0;
}


struct file_operations MMSP2key_fops = {
	open:       MMSP2key_open,
	read:       MMSP2key_read,
	ioctl:      MMSP2key_ioctl,
	release:    MMSP2key_release,
};


static int __init MMSP2key_init(void)
{
	int i;

	for(i=0;i<MAX_FUNC_KEY;i++) set_gpio_ctrl(FUNC1_KEY_BASE,GPIOMD_IN,GPIOPU_EN);
	for(i=0;i<MAX_FUNC_KEY;i++) set_gpio_ctrl(FUNC2_KEY_BASE,GPIOMD_IN,GPIOPU_EN);

	set_gpio_ctrl(VOL_UP,GPIOMD_IN,GPIOPU_EN);
	set_gpio_ctrl(VOL_DOWN,GPIOMD_IN,GPIOPU_EN);
	set_gpio_ctrl(TACT_SW,GPIOMD_IN,GPIOPU_EN);


	if( !register_chrdev( MMSP2_KEY_MAJOR, MMSP2_KEY_NAME, &MMSP2key_fops ) ) {
#ifdef CONFIG_MACH_GP2X_DEBUG
		printk(" register device %s OK\n", MMSP2_KEY_NAME );
#endif
	}else{
		printk(" unable to get major %d for %s \n", MMSP2_KEY_MAJOR, MMSP2_KEY_NAME );
		return -EBUSY;
	}

	return 0;
}

static void __exit MMSP2key_exit(void)
{
    unregister_chrdev( MMSP2_KEY_MAJOR, MMSP2_KEY_NAME );
}

module_init(MMSP2key_init);
module_exit(MMSP2key_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
