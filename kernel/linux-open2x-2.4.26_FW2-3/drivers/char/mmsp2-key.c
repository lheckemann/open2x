/* PULLED FROM GPH 4.1.0 KERNEL SOURCE FOR OPEN2X F200 COMPATIBILITY- senquack */
/* dansilsby@gmail.com */

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

//senquack - moved this outside the ifdef below for stick click emulation purposes:
#define TACT_SW				GPIO_D11

#ifndef CONFIG_MACH_GP2XF200
#else
#define USB_CHECK			GPIO_F5
#define USB_TO_SD			GPIO_F2
#define CPU_TO_SD			GPIO_F6
#define USB_CPU_RESET		GPIO_F1
#define TACT_MASK		((1<<1)|(1<<3)|(1<<5)|(1<<7))
#endif

#define MAX_FUNC_KEY1			8
#ifndef CONFIG_MACH_GP2XF200
#define MAX_FUNC_KEY2			8
#else
#define MAX_FUNC_KEY2			4
#endif

/* IOCTL  CMD*/
//senquack - GPH's ioctl interface is here utterly broken, unused, and braindead.  Replacing
//it with my own.
//#define BACK_LIGHT_OFF			0
//#define BACK_LIGHT_ON			1
//#define BATT_LED_ON				2
//#define BATT_LED_OFF			3
//
//#define FCLK_200				10
//#define FCLK_166				11
//#define FCLK_133				12
//#define FCLK_100				13
//#define FCLK_78					14
//#define FCLK_64					15
//#define FCLK_DEFUALT			16
//#define SD_CLK_MODE_0			17		/* 5Mhz */
//#define SD_CLK_MODE_1			18		/* 15Mhz */
//#define SD_CLK_MODE_2			19		/* 25Mhz */
//#define GP2X_INFO_LSB			20
//#define GP2X_INFO_MSB			21
//#define GP2X_GET_USBHCHK		30
//#define USB_CONNECT_START		31
//#define USB_CONNECT_END			32
//senquack - new stick-click emulation controllable by these new ioctl commands:
#define GP2X_SET_STICK_CLICK_EMULATION_MODE		40
// senquack - new /dev/GPIO button remapping controllable by these commands:
#define GP2X_REMAP_BUTTON_00	50
#define GP2X_REMAP_BUTTON_01	51
#define GP2X_REMAP_BUTTON_02	52
#define GP2X_REMAP_BUTTON_03	53
#define GP2X_REMAP_BUTTON_04	54
#define GP2X_REMAP_BUTTON_05	55
#define GP2X_REMAP_BUTTON_06	56
#define GP2X_REMAP_BUTTON_07	57
#define GP2X_REMAP_BUTTON_08	58
#define GP2X_REMAP_BUTTON_09	59
#define GP2X_REMAP_BUTTON_10	60
#define GP2X_REMAP_BUTTON_11	61
#define GP2X_REMAP_BUTTON_12	62
#define GP2X_REMAP_BUTTON_13	63
#define GP2X_REMAP_BUTTON_14	64
#define GP2X_REMAP_BUTTON_15	65
#define GP2X_REMAP_BUTTON_16	66
#define GP2X_REMAP_BUTTON_17	67
#define GP2X_REMAP_BUTTON_18	68
#define GP2X_DISABLE_REMAPPING	69
//senquack - new option to allow caching of mmap'd upper memory (what mmuhack does)
#define GP2X_SET_UPPER_MEMORY_CACHING	80


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

#ifdef CONFIG_MACH_GP2XF200

/* GP2X JOSYSTICK SDL MAPING */
#define NON_KEY			 0
#define VK_UP			(1<<0)
#define VK_UP_LEFT		(1<<1)
#define VK_LEFT			(1<<2)
#define VK_DOWN_LEFT	(1<<3)
#define VK_DOWN			(1<<4)
#define VK_DOWN_RIGHT	(1<<5)
#define VK_RIGHT		(1<<6)
#define VK_UP_RIGHT		(1<<7)

/*
.... : NON_KEY
U... : VK_UP
.D.. : VK_DOWN
UD.. : NON_KEY

..L. : VK_LEFT
U.L. : VK_UP_LEFT
.DL. : VK_DOWN_LEFT
UDL. : VK_LEFT

...R : VK_RIGHT
U..R : VK_UP_RIGHT
.D.R : VK_DOWN_RIGHT
UD.R : VK_RIGHT

..LR : NON_KEY
U.LR : VK_UP
.DLR : VK_DOWN
UDLR : NON_KEY
*/

unsigned char KeyTable[16] =
	{
		NON_KEY, VK_UP, VK_DOWN, NON_KEY,
		VK_LEFT, VK_UP_LEFT, VK_DOWN_LEFT , VK_LEFT,
		VK_RIGHT, VK_UP_RIGHT ,VK_DOWN_RIGHT, VK_RIGHT,
		NON_KEY, VK_UP, VK_DOWN, NON_KEY
	};

unsigned char KeyPos[]={GPIO_M0,GPIO_M4,GPIO_M2,GPIO_M6}; // final

#endif

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
	unsigned long keyValue=0;
	int i;
	unsigned char keyTemp=0;

	for(i=0;i<MAX_FUNC_KEY2;i++)
#ifndef CONFIG_MACH_GP2XF200
		keyTemp|=((read_gpio_bit(FUNC2_KEY_BASE+i)) << i);
#else
		keyTemp|=((read_gpio_bit(KeyPos[i])) << i);
#endif

#ifndef CONFIG_MACH_GP2XF200
	keyValue=(unsigned long)~keyTemp;
#else
	keyValue=(unsigned long) KeyTable[(~keyTemp)& 0x0f];
#endif

	keyTemp=0;
	for(i=0;i<MAX_FUNC_KEY1;i++)
		keyTemp|=((read_gpio_bit(FUNC1_KEY_BASE+i)) << i);

#ifndef CONFIG_MACH_GP2XF200
	keyValue&=(unsigned long)(((~keyTemp) << 8)|0xFF);
#else
	keyValue|=(unsigned long) (((~keyTemp) & 0xFF) << 8);
#endif

#ifndef CONFIG_MACH_GP2XF200
	keyTemp=0;
	keyTemp|=read_gpio_bit(VOL_UP);
	keyTemp|=read_gpio_bit(VOL_DOWN)<<1;
	keyTemp|=read_gpio_bit(TACT_SW)<<2;
	keyTemp|=1<<3;
	keyValue&=(unsigned long)(((~keyTemp)<<16)|0xFFFF);
#else
	keyTemp=0;
	keyTemp|=read_gpio_bit(VOL_UP);
	keyTemp|=read_gpio_bit(VOL_DOWN)<<1;
	//senquack - added emulation of stick click so we now *do* check for TACT_SW:
//	keyTemp=(~keyTemp)& 0x03;
	if (g_stick_click_mode == OPEN2X_STICK_CLICK_DISABLED)
	{
		// force stick-click to be unpressed when emulation disabled on F200s
		keyTemp |= 1 << 2;	
	}
	else
	{
		keyTemp |= read_gpio_bit(TACT_SW) << 2;
	}

	keyTemp = (~keyTemp) & 0x07;


	keyTemp|=read_gpio_bit(USB_CHECK)<<3;
	keyValue|=(unsigned long) keyTemp << 16;
#endif

	//senquack - new support for button remapping:
	if (g_button_remapping)
	{
		unsigned long remapped_keyValue = (keyTemp & 0x80000);	// Store away lone USB_CHECK bit
		for (i = 18; i >= 0; i--)
		{
			// check if button is even enabled:
			if (g_button_mapping[i] != -1)
				remapped_keyValue |= ((keyValue >> i) & 0x1) << g_button_mapping[i];
		}
		copy_to_user( Putbuf, &remapped_keyValue, 4);
	}
	else
	{
		copy_to_user( Putbuf, &keyValue, 4);
	}
	return length;
}


//senquack - as far as I can tell, this ioctl interface is completely and utterly broken
//	and unused by even GPH themselves.  I am redoing it with my own commands.
//int MMSP2key_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,unsigned long arg)
//{
//	MMSP20_CLOCK_POWER *pPMR_REG;
//	pPMR_REG = MMSP20_GetBase_CLOCK_POWER();
//
//	switch(arg)
//	{
//		case 0:
//#ifndef CONFIG_MACH_GP2XF200
//			write_gpio_bit(GPIO_H1,0);		//LCD VGH oFF
//			write_gpio_bit(GPIO_H2,0);		//LCD Back oFF
//#else
//			write_gpio_bit(GPIO_H1,0);		//5V OFF
//			write_gpio_bit(GPIO_L11,0);		//LCD Back oFF
//#endif
//			break;
//		case 1:
//#ifndef CONFIG_MACH_GP2XF200
//			write_gpio_bit(GPIO_H1,1);		//LCD VGH on
//			write_gpio_bit(GPIO_H2,1);		//LCD Back on
//#else
//			write_gpio_bit(GPIO_H1,1);		//5V ON
//			write_gpio_bit(GPIO_L11,1);		//LCD Back ON
//#endif
//			break;
//		case BATT_LED_ON:
//#ifndef CONFIG_MACH_GP2XF200
//			write_gpio_bit(GPIO_H4,0);		//Batt LED on
//#endif
//			break;
//		case BATT_LED_OFF:
//#ifndef CONFIG_MACH_GP2XF200
//			write_gpio_bit(GPIO_H4,1);		//Batt LED oFF
//#endif
//			break;
//		case GP2X_INFO_LSB:
//			//senquack - this seems to be extraneous.. cannot find this function anywhere:
////			return GetGp2xInfo(1);
//		case GP2X_INFO_MSB:
//			//senquack - this seems to be extraneous.. cannot find this function anywhere:
////			return GetGp2xInfo(0);
//		case FCLK_200:
//			pPMR_REG->FPLLSETVREG  = ((0x49 << 8) + (1 << 2) + 0);
//			udelay(4000);
//			break;
//		case FCLK_166:
//			pPMR_REG->FPLLSETVREG  = ((0x3B << 8) + (1 << 2) + 0);
//			udelay(4000);
//			break;
//		case FCLK_133:
//			pPMR_REG->FPLLSETVREG  = ((0x2E << 8) + (1 << 2) + 0);
//			udelay(4000);
//			break;
//		case FCLK_100:
//			pPMR_REG->FPLLSETVREG  = ((0x20 << 8) + (1 << 2) + 0);
//			udelay(4000);
//			printk("fclk 100MHZ\n");
//			break;
//		case FCLK_78:
//			pPMR_REG->FPLLSETVREG  = ((0x18 << 8) + (1 << 2) + 0);
//			udelay(4000);
//			break;
//		case FCLK_64:
//			pPMR_REG->FPLLSETVREG  = ((0x12 << 8) + (1 << 2) + 0);
//			udelay(4000);
//			break;
//#ifdef CONFIG_MACH_GP2XF200
//		case USB_CONNECT_START:
//			/* Usb to SD mode */
//			write_gpio_bit(CPU_TO_SD,1);
//			write_gpio_bit(USB_TO_SD,0);
//			write_gpio_bit(USB_CPU_RESET,1);
//			udelay(50);
//			break;
//		case USB_CONNECT_END:
//			/* SD to usb mode */
//			write_gpio_bit(USB_CPU_RESET,0);
//			write_gpio_bit(CPU_TO_SD,0);
//			write_gpio_bit(USB_TO_SD,1);
//			break;
//#endif
//		case GP2X_GET_USBHCHK:
//			//senquack - this is not defined using the current headers, I am disabling it because
//			//it probably never worked anyway:
////			return GetUshHcon();
//
//		//senquack - new stick click emulation:
////#define GP2X_STICK_CLICK_EMULATION_DISABLED		43
////#define GP2X_STICK_CLICK_EMULATION_DPAD			44
////#define GP2X_STICK_CLICK_EMULATION_VOLUPDOWN		45
////int g_stick_click_mode = OPEN2X_STICK_CLICK_DISABLED;
//		case GP2X_STICK_CLICK_EMULATION_DISABLED:
//			// g_stick_click_mode global is buried in kernel/sys.c:
//			// (cannot store globals here, they get trashed)
//			printk("kernel: stick click emulation disabled\n");
//			g_stick_click_mode = OPEN2X_STICK_CLICK_DISABLED;
//			set_gpio_ctrl(TACT_SW,GPIOMD_IN,GPIOPU_EN);		// original GPH GPIO configuration
//			break;
//		case GP2X_STICK_CLICK_EMULATION_DPAD:
//			printk("kernel: stick click emulation DPAD\n");
//			g_stick_click_mode = OPEN2X_STICK_CLICK_DPAD;
//			set_gpio_ctrl(TACT_SW,GPIOMD_OUT,GPIOPU_DIS);	// new GPIO configuration for emulation
//			break;
//		case GP2X_STICK_CLICK_EMULATION_VOLUPDOWN:
//			printk("kernel: stick click emulation VOLUP+DOWN\n");
//			g_stick_click_mode = OPEN2X_STICK_CLICK_VOLUPDOWN;
//			set_gpio_ctrl(TACT_SW,GPIOMD_OUT,GPIOPU_DIS);	// new GPIO configuration for emulation
//			break;
//		case FCLK_DEFUALT:
//			break;
//	}
//
//	return 0;
//}
//#define GP2X_SET_STICK_CLICK_EMULATION_MODE		40
int MMSP2key_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int int_param;

	switch (cmd)
	{
		case GP2X_SET_STICK_CLICK_EMULATION_MODE:
			// g_stick_click_mode global is buried in kernel/sys.c:
			// (cannot store globals here, they get trashed)
			printk("kernel: set stick click emulation mode\n");
			if (get_user(int_param, (int *)arg))
				return -EFAULT;
			if (int_param == OPEN2X_STICK_CLICK_DISABLED)
			{
				printk("kernel: stick click emulation disabled\n");
				g_stick_click_mode = int_param;
				set_gpio_ctrl(TACT_SW,GPIOMD_IN,GPIOPU_EN);		// original GPH GPIO configuration
			}
			else if (int_param == OPEN2X_STICK_CLICK_DPAD)
			{
				printk("kernel: stick click emulation DPAD\n");
				g_stick_click_mode = int_param;
				set_gpio_ctrl(TACT_SW,GPIOMD_OUT,GPIOPU_DIS);	// new GPIO configuration for emulation
			} 
			else if (int_param == OPEN2X_STICK_CLICK_VOLUPDOWN)
			{
				printk("kernel: stick click emulation VOLUP+DOWN\n");
				g_stick_click_mode = int_param;
				set_gpio_ctrl(TACT_SW,GPIOMD_OUT,GPIOPU_DIS);	// new GPIO configuration for emulation
			}
			break;
		case GP2X_REMAP_BUTTON_00:
		case GP2X_REMAP_BUTTON_01:
		case GP2X_REMAP_BUTTON_02:
		case GP2X_REMAP_BUTTON_03:
		case GP2X_REMAP_BUTTON_04:
		case GP2X_REMAP_BUTTON_05:
		case GP2X_REMAP_BUTTON_06:
		case GP2X_REMAP_BUTTON_07:
		case GP2X_REMAP_BUTTON_08:
		case GP2X_REMAP_BUTTON_09:
		case GP2X_REMAP_BUTTON_10:
		case GP2X_REMAP_BUTTON_11:
		case GP2X_REMAP_BUTTON_12:
		case GP2X_REMAP_BUTTON_13:
		case GP2X_REMAP_BUTTON_14:
		case GP2X_REMAP_BUTTON_15:
		case GP2X_REMAP_BUTTON_16:
		case GP2X_REMAP_BUTTON_17:
		case GP2X_REMAP_BUTTON_18:
//extern int g_button_mapping[19];	// array of gp2x button mappings used in mmsp2-key.c (in kernel/sys.c)
//extern int g_button_remapping;		// When this is 0, remapping is off, 1 is on	(in kernel/sys.c)
			if (get_user(int_param, (int *)arg))
				return -EFAULT;
			if (int_param >= -1 && int_param <= 18)
			{
				// FYI: if user passed -1, that signifies the button is disabled
				g_button_remapping = 1;
				g_button_mapping[cmd - 50] = int_param;
			}
			break;
		case GP2X_DISABLE_REMAPPING:
			g_button_remapping = 0;
			int i;
			for (i = 0; i < 19; i++)
			{
				g_button_mapping[i] = i;
			}
			break;
		case GP2X_SET_UPPER_MEMORY_CACHING:
			// g_cache_high_memory is buried in kernel/sys.c:
			// (cannot store globals here, they get trashed)
			if (get_user(int_param, (int *)arg))
				return -EFAULT;
			if (int_param == 0)
			{
				printk("kernel: upper memory caching for future mmaps disabled\n");
				g_cache_high_memory = int_param;
			}
			else if (int_param == 1)
			{
				printk("kernel: upper memory caching for future mmaps enabled\n");
				g_cache_high_memory = int_param;
			}
			break;
		default:
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

	for(i=0;i<MAX_FUNC_KEY1;i++) set_gpio_ctrl(FUNC1_KEY_BASE+i,GPIOMD_IN,GPIOPU_EN);
#ifndef CONFIG_MACH_GP2XF200
	for(i=0;i<MAX_FUNC_KEY2;i++) set_gpio_ctrl(FUNC2_KEY_BASE+i,GPIOMD_IN,GPIOPU_EN);
#else
	for(i=0;i<MAX_FUNC_KEY2;i++) set_gpio_ctrl(KeyPos[i],GPIOMD_IN,GPIOPU_EN);

#if 0
	set_gpio_ctrl(FUNC2_KEY_BASE+0 ,GPIOMD_IN,GPIOPU_EN);
	set_gpio_ctrl(FUNC2_KEY_BASE+5 ,GPIOMD_IN,GPIOPU_EN);	/* ORG M4 */
	set_gpio_ctrl(FUNC2_KEY_BASE+2 ,GPIOMD_IN,GPIOPU_EN);
	set_gpio_ctrl(FUNC2_KEY_BASE+7 ,GPIOMD_IN,GPIOPU_EN);	/* ORG M6 */
#endif
#endif

	set_gpio_ctrl(VOL_UP,GPIOMD_IN,GPIOPU_EN);
	set_gpio_ctrl(VOL_DOWN,GPIOMD_IN,GPIOPU_EN);

	//senquack - new stick-click emulation:

#ifndef CONFIG_MACH_GP2XF200
	set_gpio_ctrl(TACT_SW,GPIOMD_IN,GPIOPU_EN);
#else
	set_gpio_ctrl(USB_CHECK,GPIOMD_IN,GPIOPU_EN);
	set_gpio_ctrl(CPU_TO_SD,GPIOMD_OUT,GPIOPU_EN);
	set_gpio_ctrl(USB_TO_SD,GPIOMD_OUT,GPIOPU_EN);
	set_gpio_ctrl(USB_CPU_RESET,GPIOMD_OUT,GPIOPU_EN);

	//senquack - stick click emulation:
//	printk("Enabling F200 stick-click emulation\n");
//	set_gpio_ctrl(TACT_SW,GPIOMD_OUT,GPIOPU_DIS);
	// configure this the same was as F100s initially.. it can be changed later to 
	// GPIOMD_OUT and GPIOPU_DIS when stick-click emulation is enabled after kernel boot:
	set_gpio_ctrl(TACT_SW,GPIOMD_IN,GPIOPU_EN);

	/* Cpu to sd mode */
	write_gpio_bit(CPU_TO_SD,0);
	write_gpio_bit(USB_TO_SD,1);
	write_gpio_bit(USB_CPU_RESET,0);
#endif
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
