/*
 *	Watchdog driver for the Altera Excalibur EPXA1DB
 *
 *      (c) Copyright 2003 Krzysztof Marianski <kmarian@konin.lm.pl>
 *          Based on SA11x0 Watchdog driver by Oleg Drokin <green@crimea.edu>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *	This material is provided "AS-IS" and at no charge
 *
 *	(c) Copyright 2003 Krzysztof Marianski <kmarian@konin.lm.pl>
 *
 *      1/08/2003 Initial release
 */

#include <linux/module.h>
#include <linux/config.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/reboot.h>
#include <linux/smp_lock.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/hardware.h>

#define WATCHDOG00_TYPE (volatile unsigned int*)
#include <asm/arch/watchdog00.h>
#include <asm/bitops.h>

#define TIMER_MARGIN	30		/* (secs) Default is 30 seconds */

static int margin = TIMER_MARGIN;	/* in seconds */
static int epxa1wdt_users;
static unsigned char last_written_byte;

#ifdef CONFIG_WATCHDOG_NOWAYOUT
static int nowayout=1;
#else
static int nowayout=0;
#endif

#ifdef MODULE
MODULE_PARM(margin,"i");
MODULE_PARM(nowayout, "i");
#endif

/*
 *	Allow only one person to hold it open
 */

static int epxa1dog_open(struct inode *inode, struct file *file)
{
	if(test_and_set_bit(1,&epxa1wdt_users))
		return -EBUSY;

	/* Reset the Watchdog, just to be sure we don't set
	    a value close to actual value of WDOG_COUNT register */
	*WDOG_RELOAD(IO_ADDRESS(EXC_WATCHDOG00_BASE))=WDOG_RELOAD_MAGIC_1;
	*WDOG_RELOAD(IO_ADDRESS(EXC_WATCHDOG00_BASE))=WDOG_RELOAD_MAGIC_2;

	/* Activate EPXA1DB Watchdog timer */
	*WDOG_CR(IO_ADDRESS(EXC_WATCHDOG00_BASE))= (EXC_INPUT_CLK_FREQUENCY * margin) & WDOG_CR_TRIGGER_MSK;

	last_written_byte = 'V'; //in case user opens it only to ioctl
	return 0;
}

static int epxa1dog_release(struct inode *inode, struct file *file)
{
	/*
	 *	Shut off the timer and set lock bit when no special
	 *	character 'V' was last written
	 */

	if ((last_written_byte != 'V') && (nowayout)) {
		*WDOG_CR(IO_ADDRESS(EXC_WATCHDOG00_BASE)) |= WDOG_CR_LK_MSK;
		printk("No special character 'V' was written to Watchdog just before closing it\n");
		printk("WATCHDOG LOCKED - Reboot expected!!!\n");
	} else
		*WDOG_CR(IO_ADDRESS(EXC_WATCHDOG00_BASE))=0;

	epxa1wdt_users = 0;

	return 0;
}

static ssize_t epxa1dog_write(struct file *file, const char *data, size_t len, loff_t *ppos)
{
	/*  Can't seek (pwrite) on this device  */
	if (ppos != &file->f_pos)
		return -ESPIPE;

	/* Reset Watchdog timer. */
	if(len) {
		*WDOG_RELOAD(IO_ADDRESS(EXC_WATCHDOG00_BASE))=WDOG_RELOAD_MAGIC_1;
		*WDOG_RELOAD(IO_ADDRESS(EXC_WATCHDOG00_BASE))=WDOG_RELOAD_MAGIC_2;
		last_written_byte = *data;
		return 1;
	}
	return 0;
}

static int epxa1dog_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	static struct watchdog_info ident = {
		identity: "EPXA Watchdog",
	};

	switch(cmd){
	default:
		return -ENOIOCTLCMD;
	case WDIOC_GETSUPPORT:
		return copy_to_user((struct watchdog_info *)arg, &ident, sizeof(ident));
//	case WDIOC_GETSTATUS: //TODO
//		return put_user(0,(int *)arg);
//	case WDIOC_GETBOOTSTATUS: //TODO
//		return 0;
	case WDIOC_KEEPALIVE:
		*WDOG_RELOAD(IO_ADDRESS(EXC_WATCHDOG00_BASE))=WDOG_RELOAD_MAGIC_1;
		*WDOG_RELOAD(IO_ADDRESS(EXC_WATCHDOG00_BASE))=WDOG_RELOAD_MAGIC_2;
		return 0;
	case WDIOC_SETTIMEOUT:
		*WDOG_CR(IO_ADDRESS(EXC_WATCHDOG00_BASE))= (EXC_INPUT_CLK_FREQUENCY * margin) & WDOG_CR_TRIGGER_MSK;
		return 0;
	case WDIOC_GETTIMEOUT:
		return put_user( ((*WDOG_CR(IO_ADDRESS(EXC_WATCHDOG00_BASE)))/EXC_INPUT_CLK_FREQUENCY), (int*)arg);
	}
}

static struct file_operations epxa1dog_fops = {
	.owner		= THIS_MODULE,
	.write		= epxa1dog_write,
	.ioctl		= epxa1dog_ioctl,
	.open		= epxa1dog_open,
	.release	= epxa1dog_release,
};

static struct miscdevice epxa1dog_miscdev=
{
	.minor	= WATCHDOG_MINOR,
	.name	= "EPXA watchdog",
	.fops	= &epxa1dog_fops
};

static int __init epxa1dog_init(void)
{
	int ret;

	ret = misc_register(&epxa1dog_miscdev);

	if (ret)
		return ret;

	printk("EPXA Watchdog Timer: timer margin %d sec\n", margin);
	printk("EPXA Watchdog Timer: no way out is %s\n", nowayout ? "enabled" : "disabled");

	return 0;
}

static void __exit epxa1dog_exit(void)
{
	misc_deregister(&epxa1dog_miscdev);
}

module_init(epxa1dog_init);
module_exit(epxa1dog_exit);

MODULE_AUTHOR("Krzysztof Marianski <kmarian@konin.lm.pl>");
MODULE_DESCRIPTION("EPXA Watchdog Timer");
MODULE_LICENSE("GPL");
