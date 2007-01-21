/*
 *  Real Time Clock interface for Linux on MMSP2
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 *  Based on linux/kernel/drivers/char/sa1100-rtc.c
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <asm/bitops.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <linux/rtc.h>

static const unsigned char days_in_mo[] =
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define is_leap(year) \
	((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

static int rtc_count = 0;

/*
 * Converts seconds since 1970-01-01 00:00:00 to Gregorian date.
 */

static void decodetime (unsigned long t, struct rtc_time *tval)
{
	long days, month, year, rem;

	days = t / 86400;
	rem = t % 86400;
	tval->tm_hour = rem / 3600;
	rem %= 3600;
	tval->tm_min = rem / 60;
	tval->tm_sec = rem % 60;
	tval->tm_wday = (4 + days) % 7;

#define LEAPS_THRU_END_OF(y) ((y)/4 - (y)/100 + (y)/400)

	year = 1970 + days / 365;
	days -= ((year - 1970) * 365
			+ LEAPS_THRU_END_OF (year - 1)
			- LEAPS_THRU_END_OF (1970 - 1));
	if (days < 0) {
		year -= 1;
		days += 365 + is_leap(year);
	}
	tval->tm_year = year - 1900;
	tval->tm_yday = days + 1;

	month = 0;
	if (days >= 31) {
		days -= 31;
		month++;
		if (days >= (28 + is_leap(year))) {
			days -= (28 + is_leap(year));
			month++;
			while (days >= days_in_mo[month]) {
				days -= days_in_mo[month];
				month++;
			}
		}
	}
	tval->tm_mon = month;
	tval->tm_mday = days + 1;
}

static int rtc_open(struct inode *inode, struct file *file)
{
	if(rtc_count!=0)
		return -EBUSY;
	rtc_count = 1;
	return 0;
}

static int rtc_release(struct inode *inode, struct file *file)
{
	rtc_count = 0;
	return 0;
}

static ssize_t rtc_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	return count;
}

static int rtc_ioctl(struct inode *inode, struct file *file,
		     unsigned int cmd, unsigned long arg)
{
	struct rtc_time tm;
	struct mmsp2_rtc_info rtc;
	unsigned long count;

	switch (cmd) {
	case RTC_RD_TIME:
		decodetime (RTCTCNT, &tm);
		break;
	case RTC_SET_TIME:
		if (!capable(CAP_SYS_TIME))
			return -EACCES;
		if (copy_from_user (&tm, (struct rtc_time*)arg, sizeof (tm)))
			return -EFAULT;
		tm.tm_year += 1900;
		if (tm.tm_year < 1970 || (unsigned)tm.tm_mon >= 12 ||
		    tm.tm_mday < 1 || tm.tm_mday > (days_in_mo[tm.tm_mon] +
				(tm.tm_mon == 1 && is_leap(tm.tm_year))) ||
		    (unsigned)tm.tm_hour >= 24 ||
		    (unsigned)tm.tm_min >= 60 ||
		    (unsigned)tm.tm_sec >= 60)
			return -EINVAL;
		count = mktime (tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
				tm.tm_hour, tm.tm_min, tm.tm_sec);
		while(RTCSTCNT <= 0x200);
		RTCTSET = count;
		return 0;
	case RTC_EPOCH_READ:
		return put_user (1970, (unsigned long *)arg);
	case IOCTL_MMSP2_READ_RTC:
		rtc.sec_count = RTCTCNT;
		rtc.msec_count = RTCSTCNT;
		return copy_to_user ((void *)arg, &rtc, sizeof(rtc)) ? -EFAULT : 0;

	default:
		return -EINVAL;
	}
	return copy_to_user ((void *)arg, &tm, sizeof (tm)) ? -EFAULT : 0;
}

static struct file_operations rtc_fops = {
	owner:		THIS_MODULE,
	read:		rtc_read,
	ioctl:		rtc_ioctl,
	open:		rtc_open,
	release:	rtc_release,
};

static struct miscdevice mmsp2rtc_miscdev = {
	RTC_MINOR,
	"rtc",
	&rtc_fops
};

static int rtc_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	char *p = page;
	int len;
	struct rtc_time tm;

	decodetime (RTCTCNT, &tm);
	p += sprintf(p, "rtc_time\t: %02d:%02d:%02d\n"
			"rtc_date\t: %04d-%02d-%02d\n"
			"rtc_epoch\t: %04d\n",
			tm.tm_hour, tm.tm_min, tm.tm_sec,
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, 1970);

	len = (p - page) - off;
	if (len < 0)
		len = 0;

	*eof = (len <= count) ? 1 : 0;
	*start = page + off;

	return len;
}

static int __init rtc_init(void)
{
	misc_register (&mmsp2rtc_miscdev);
	create_proc_read_entry ("driver/rtc", 0, 0, rtc_read_proc, NULL);
#ifdef CONFIG_MACH_GP2X_DEBUG
	printk (KERN_INFO "MMSP2 Real Time Clock driver\n");
#endif
	return 0;
}

static void __exit rtc_exit(void)
{
	remove_proc_entry ("driver/rtc", NULL);
	misc_deregister (&mmsp2rtc_miscdev);
}

module_init(rtc_init);
module_exit(rtc_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
MODULE_DESCRIPTION("MMSP2 Realtime Clock Driver (RTC)");
EXPORT_NO_SYMBOLS;
