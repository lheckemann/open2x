/*
 *  linux/kernel/char/dualcpu.c
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
 *
 *  MMSP2 DUAL CPU Interface driver
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/arch/memory.h>

#include "dualcpu.h"
#include "arm940firm.h"

#define DEBUG0(x)	printk x

static int dualcpu_used=0;
static int dualcpu_920_flag=0;
static MP4D_DISPLAY_PARAM mp4d_disp;
static MP4D_REL_PARAM mp4d_rel;
static MP4D_RUN_PARAM mp4d_run;
static MP4D_INIT_BUF_PARAM mp4d_init_buf;
static MP4D_INIT_PARAM mp4d_init;

static void DumpData(unsigned short *data, int num)
{
	int i;

	printk("DumpData (%d):\n", num);
	for(i=0; i<num; i++) {
		printk("0x%x, ", data[i]);
		if(((i+1) % 8) == 0) {
			printk("\n");
		}
	}
	printk("\n");
}

static void Set940DataReg(unsigned short *data, int num)
{
	int i;

	if((num<1) || (num>16)) {
		DEBUG0(("dualcpu.c : Set940DataReg() input mismatch\n"));
		return;
	}
	for(i=num-1; i>0; i--) {
		DUAL940DATA(i) = data[i];
	}
	DUALINT940 = 0x01;
	DUAL940DATA(0) = data[0];
}

static void Get920DataReg(unsigned short *data, int num)
{
	int i;

	if((num<1) || (num>16)) {
		DEBUG0(("dualcpu.c : Get920DataReg() input mismatch\n"));
		return;
	}
	for(i=num-1; i>=0; i--) {
		data[i] = DUAL920DATA(i);
	}
}

static void dualcpu_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	DUALPEND920 = 0xFF;
	DUALINT920 = 0x00;
	dualcpu_920_flag = 1;
}

static int dualcpu_ioctl(struct inode *inode, struct file *file, 
	unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	switch(cmd) {
	case IOCTL_DUALCPU_MP4D_INIT:
		if(dualcpu_920_flag==0) {
			if(copy_from_user(&mp4d_init, (MP4D_INIT_PARAM *)arg, sizeof(MP4D_INIT_PARAM)))
				return -EFAULT;
//			printk("INIT_CMD:\n");
//			DumpData((unsigned short *)&mp4d_init, sizeof(MP4D_INIT_PARAM)/2);
			Set940DataReg((unsigned short *)&mp4d_init, sizeof(MP4D_INIT_PARAM)/2);
			ret = 1;
		}
		break;
	case IOCTL_DUALCPU_MP4D_INIT_BUF:
		if(dualcpu_920_flag==0) {
			if(copy_from_user(&mp4d_init_buf, (MP4D_INIT_BUF_PARAM *)arg, sizeof(MP4D_INIT_BUF_PARAM)))
				return -EFAULT;
//			printk("INIT_BUF:\n");
//			DumpData((unsigned short *)&mp4d_init_buf, sizeof(MP4D_INIT_BUF_PARAM)/2);
			Set940DataReg((unsigned short *)&mp4d_init_buf, sizeof(MP4D_INIT_BUF_PARAM)/2);
			ret = 1;
		}
		break;
	case IOCTL_DUALCPU_MP4D_RUN:
		if(dualcpu_920_flag==0) {
			if(copy_from_user(&mp4d_run, (MP4D_RUN_PARAM *)arg, sizeof(MP4D_RUN_PARAM)))
				return -EFAULT;
//			printk("RUN_CMD:\n");
//			DumpData((unsigned short *)&mp4d_run, sizeof(MP4D_RUN_PARAM)/2);
			Set940DataReg((unsigned short *)&mp4d_run, sizeof(MP4D_RUN_PARAM)/2);
			ret = 1;
		}
		break;
	case IOCTL_DUALCPU_MP4D_RELEASE:
		if(dualcpu_920_flag==0) {
			if(copy_from_user(&mp4d_rel, (MP4D_REL_PARAM *)arg, sizeof(MP4D_REL_PARAM)))
				return -EFAULT;
//			printk("RELEASE_CMD:\n");
//			DumpData((unsigned short *)&mp4d_rel, sizeof(MP4D_REL_PARAM)/2);
			Set940DataReg((unsigned short *)&mp4d_rel, sizeof(MP4D_REL_PARAM)/2);
			ret = 1;
		}
		break;
	case IOCTL_DUALCPU_MP4D_STATUS:
		if(dualcpu_920_flag==1) {
			dualcpu_920_flag = 0;
			Get920DataReg((unsigned short *)&mp4d_disp, sizeof(MP4D_DISPLAY_PARAM)/2);
//			printk("STATUS:\n");
//			DumpData((unsigned short *)&mp4d_disp, sizeof(MP4D_DISPLAY_PARAM)/2);
			if(copy_to_user((MP4D_DISPLAY_PARAM *)arg, &mp4d_disp, sizeof(MP4D_DISPLAY_PARAM)))
				return -EFAULT;
			ret = 1;
		}
		break;
	default:
		return -EINVAL;
	}
	return ret;
}

static int dualcpu_open(struct inode *inode, struct file *filp)
{
	if(dualcpu_used==0) {
		dualcpu_used = 1;
		return 0;
	}
	else {
		return -EBUSY;
	}
}

static int dualcpu_release(struct inode *inode, struct file *filp)
{
	dualcpu_used = 0;
	return 0;
}

static struct file_operations mmsp2_dualcpu_fops = {
	owner:		THIS_MODULE,
	open:		dualcpu_open,
	release:	dualcpu_release,
	ioctl:		dualcpu_ioctl,
};

static struct miscdevice mmsp2_dualcpu = {
	254, "dualcpu", &mmsp2_dualcpu_fops
};

static int __init mmsp2_dualcpu_init(void)
{
	int i;
	volatile unsigned char *addr;

	DUALINT920 = 0;
	DUALPEND920 = 0xFF;
	DUALINT940 = 0;
	DUALPEND940 = 0xFF;

	DUALCTRL940 = (DUALCTRL940 & 0xFF80) | (PA_ARM940_BASE >> 24);
	DUALINT940 = 0x00;
	DUALPEND940 = 0xFF;

	addr = (volatile unsigned char *)VA_ARM940_BASE;
	memcpy((void *)addr, arm940code, sizeof(arm940code));
	DUALCTRL940 = (DUALCTRL940 & 0xFF7F) | (0 << 7);
	i = request_irq(IRQ_DUALCPU, dualcpu_interrupt, SA_INTERRUPT, "DUALCPU", NULL);
	if(i) {
		DEBUG0(("dualcpu.c : request_irq() failed\n"));
		return -EIO;
	}

	misc_register(&mmsp2_dualcpu);
	return 0;
}

static void __exit mmsp2_dualcpu_exit(void)
{
	DUALINT920 = 0;
	DUALPEND920 = 0xFF;
	DUALINT940 = 0;
	DUALPEND940 = 0xFF;

	free_irq(IRQ_DUALCPU, NULL);
	DUALCTRL940 |= (1 << 7);

	misc_deregister(&mmsp2_dualcpu);
}

module_init(mmsp2_dualcpu_init);
module_exit(mmsp2_dualcpu_exit);

MODULE_AUTHOR("Edward Kuo");
MODULE_DESCRIPTION("MMSP2 DUAL CPU Interface driver");

