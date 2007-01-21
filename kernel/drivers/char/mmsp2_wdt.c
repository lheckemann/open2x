/*
 *  Watchdog driver for the MMSP2
 *
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 *  Based on linux/kernel/drivers/char/sa1100_wdt.c
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
#include <asm/bitops.h>

#define TIMER_MARGIN	60		/* (secs) Default is 1 minute */

static int mmsp2_margin = TIMER_MARGIN;	/* in seconds */
static int mmsp2wdt_users;
static unsigned long pre_margin;
#ifdef MODULE
MODULE_PARM(mmsp2_margin,"i");
#endif

/*
 *	Allow only one person to hold it open
 */

static int mmsp2dog_open(struct inode *inode, struct file *file)
{
	if(test_and_set_bit(1,&mmsp2wdt_users))
		return -EBUSY;
	MOD_INC_USE_COUNT;
	/* Activate MMS2P2 Watchdog timer */
	pre_margin = 7372800 * mmsp2_margin;
	return 0;
}

static int mmsp2dog_release(struct inode *inode, struct file *file)
{
	/*
	 *	Shut off the timer.
	 * 	Lock it in if it's a module and we defined ...NOWAYOUT
	 */
	TMATCH3 = TCOUNT + pre_margin;
	TCONTROL &= ~WDT_EN;
	mmsp2wdt_users = 0;
	MOD_DEC_USE_COUNT;
	return 0;
}

static ssize_t mmsp2dog_write(struct file *file, const char *data, size_t len, loff_t *ppos)
{
	/*  Can't seek (pwrite) on this device  */
	if (ppos != &file->f_pos)
		return -ESPIPE;

	/* Refresh TMATCH3 timer. */
	if(len) {
		TMATCH3 = TCOUNT + pre_margin;
		return 1;
	}
	return 0;
}

static int mmsp2dog_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	static struct watchdog_info ident = {
		identity: "MMSP2 Watchdog",
	};
	int new_margin;

	switch(cmd){
	default:
		return -ENOIOCTLCMD;
	case WDIOC_GETSUPPORT:
		return copy_to_user((struct watchdog_info *)arg, &ident, sizeof(ident));
	case WDIOC_GETSTATUS:
		return put_user(0,(int *)arg);
	case WDIOC_GETBOOTSTATUS:
		return put_user((RSTST & RST_WDR) ? WDIOF_CARDRESET : 0, (int *)arg);
	case WDIOC_KEEPALIVE:
		TMATCH3 = TCOUNT + pre_margin;
		return 0;
	case WDIOC_SETTIMEOUT:
		if (get_user(new_margin, (int *)arg))
			return -EFAULT;
		if (new_margin < 1)
			return -EINVAL;
		mmsp2_margin = new_margin;
		pre_margin = 7372800 * mmsp2_margin;
		TMATCH3 = TCOUNT + pre_margin;
		TCONTROL |= WDT_EN;
		TSTATUS = TCNT3;
		TINTEN |= TCNT3;
		return 0;
	case WDIOC_GETTIMEOUT:
		return put_user(mmsp2_margin, (int *)arg);
	}
}

static struct file_operations mmsp2dog_fops=
{
	owner:		THIS_MODULE,
	write:		mmsp2dog_write,
	ioctl:		mmsp2dog_ioctl,
	open:		mmsp2dog_open,
	release:	mmsp2dog_release,
};

static struct miscdevice mmsp2dog_miscdev=
{
	WATCHDOG_MINOR,
	"watchdog",
	&mmsp2dog_fops
};

static int __init mmsp2dog_init(void)
{
	int ret;

	ret = misc_register(&mmsp2dog_miscdev);

	if (ret)
		return ret;

	printk("MMSP2 Watchdog Timer: timer margin %d sec\n", mmsp2_margin);

	return 0;
}

static void __exit mmsp2dog_exit(void)
{
	misc_deregister(&mmsp2dog_miscdev);
}

module_init(mmsp2dog_init);
module_exit(mmsp2dog_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
