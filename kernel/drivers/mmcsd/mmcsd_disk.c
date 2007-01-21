/*
 * drivers/mmcsd/mmcsd_disk.c
 *
 * Copyright (C) 2005,2006 Gamepark Holdings, Inc. (www.gp2x.com)
 * Hyun <hyun3678@gp2x.com>
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * Kim,SungHo < shkim@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * Block Device Driver for MMC/SD Memory Card
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/blkpg.h>
#include <linux/hdreg.h>
#include <linux/genhd.h>
#include <linux/sched.h>
#include <linux/delay.h>

#include <asm/uaccess.h>

#include "mmcsd.h"

#include <asm/arch/proto_gpio.h>

#define MMCSD_SD_SHIFT      2  /* max 4 partition */
#define MMCSD_SD_NRDEV      (1 << MMCSD_SD_SHIFT)
#define MMCSD_SD_NDISK      (MAX_MMCSD_SLOTS << MMCSD_SD_SHIFT)

#define MAJOR_NR          60
#define DEVICE_NAME       "mmcsd"
#define DEVICE_REQUEST    do_mmcsd_request
#define DEVICE_NR(device) (MINOR(device) >> MMCSD_SD_SHIFT)
#define DEVICE_ON(device)
#define DEVICE_OFF(device)
#define DEVICE_NO_RANDOM

#include <linux/blk.h>
static int xflag=0;
static int mmcsd_sizes[MMCSD_SD_NDISK];
static int mmcsd_blksizes[MMCSD_SD_NDISK];
static struct hd_struct mmcsd_part[MMCSD_SD_NDISK];
static char mmcsd_gendisk_flags[MAX_MMCSD_SLOTS];
static struct gendisk mmcsd_gendisk = {
	major:              MAJOR_NR,
	major_name:         DEVICE_NAME,
	minor_shift:        MMCSD_SD_SHIFT,
	max_p:              MMCSD_SD_NRDEV,
	part:               mmcsd_part,
	sizes:              mmcsd_sizes,
	flags:              mmcsd_gendisk_flags,
};

static struct mmcsd_disk {
	int use_cnt;
	struct mmcsd_slot *slot;
} mmcsd_disk[MAX_MMCSD_SLOTS];

#define SD_PRIV(x)	((struct mmcsd_disk *)(mmcsd_disk) + (x))
#define SD_SLOT(x)	(SD_PRIV(x)->slot)
#define IS_EMPTY(x)	(SD_SLOT(x) == NULL)

#define bSDI(Nb)	__REGW(0xC0001500 + (Nb))
#define SDIDSTA		bSDI(0x28)		/* SDI Data Status Register */
#define SDIDSTA_TOUT	(1 << 5)
#define SDIDSTA_BFIN	(1 << 3)

/************************************************************************
 * MMC/SD data read/write
 *
 * Description: This function read/write MMC/SD data
 *
 * Arguments  : rd(r/w flag)
 *
 * Returns    : none
 *
*************************************************************************/
static int do_mmcsd_rw(struct request *req, int rd)
{
	int ret = 0;
	int dev = DEVICE_NR(req->rq_dev);
	unsigned int sect_first, sect_last;
	unsigned long blksize, from;
	char *buffer;
	unsigned int stat;
	int ncnt;

	/* read_len must be 512 bytes */
	blksize = SD_SLOT(dev)->read_len;
	buffer = req->buffer;
	sect_first = req->sector + mmcsd_part[MINOR(req->rq_dev)].start_sect;
	sect_last = sect_first + req->current_nr_sectors;
	from = sect_first * blksize;

	if(xflag)
	{
		do
		{
			stat = SDIDSTA;
			if (stat & SDIDSTA_TOUT)
			{
				ret = -ETIMEDOUT;
				SDIDSTA = stat;
				printk("Time busy error\n");
				return ret;
			}

		}while (!(stat & SDIDSTA_BFIN));

		SDIDSTA = stat;
		xflag=0;
	}


	if(rd)
	{
		while (sect_first < sect_last)
		{
			ret = SD_SLOT(dev)->transferMultblock(SD_SLOT(dev), rd, from, buffer,1);

			if (ret)
			{
				if (ret == -EBUSY)
				{
					schedule_timeout(HZ/20); /* prevent busy waiting */
					continue;
				}else break;
			}

			sect_first++;
			from += blksize;
			buffer += blksize;
		}
	}
	else
	{
		/*	ret = SD_SLOT(dev)->transferMultblock(SD_SLOT(dev), rd, from, buffer,req->current_nr_sectors); */

		ncnt=0;

		if(xflag)
		{

			do
			{
				stat = SDIDSTA;
				if (stat & SDIDSTA_TOUT)
				{
					ret = -ETIMEDOUT;
					SDIDSTA = stat;
					printk("Time busy error\n");
					return ret;
				}

				ncnt++;
			}while (!(stat & SDIDSTA_BFIN));

			SDIDSTA = stat;
			xflag=0;

		}

		if(req->current_nr_sectors > 1)
			xflag=1;

		ret = SD_SLOT(dev)->transferMultblock(SD_SLOT(dev), rd, from, buffer,req->current_nr_sectors);

	}

	return ret;
}

/************************************************************************
 * MMC/SD regisger
 *
 * Description: This function regisger MMC/SD device to kernel
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
static void do_mmcsd_request(request_queue_t * q)
{
	int dev, ret, res;
	struct request *req;
	int rd;
	int nbit=0;

	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	while (1) {
		INIT_REQUEST;	/* blk.h */
		req = CURRENT;

		/* We can do this because the generic code knows not to
		   touch the request at the head of the queue */
		spin_unlock_irq(&io_request_lock);

		/* check if the device is valid */
		dev = DEVICE_NR(req->rq_dev);
		if (dev >= MAX_MMCSD_SLOTS || IS_EMPTY(dev) ||
		    mmcsd_sizes[MINOR(req->rq_dev)] == 0) {
			res = 0;
			goto endreq;
		}

		if( mmsp_get_usbCONN()) nbit=mmsp_get_sdsize();

		if (req->sector + req->current_nr_sectors >  (mmcsd_part[MINOR(req->rq_dev)].nr_sects << nbit) )
		{
			printk("a:%d   b:%d \n",req->sector + req->current_nr_sectors,(mmcsd_part[MINOR(req->rq_dev)].nr_sects << nbit));
			/* request past end of device */
			res = 0;
			goto endreq;
		}

		if (req->cmd == READ) {
			rd = 1;
		} else if (req->cmd == WRITE) {
			rd = 0;
		} else {
			/* invalid request command */
			res = 0;
			goto endreq;
		}

		DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
		ret = do_mmcsd_rw(req, rd);
		if (ret) {
			res = 0;
			goto endreq;
		}
		res = 1;

	endreq:
		spin_lock_irq(&io_request_lock);
		end_request(res);
	}
}

/************************************************************************
 * MMC/SD ioctl
 *
 * Description: This function is MMC/SD ioctl function
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
static int mmcsd_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int dev = DEVICE_NR(inode->i_rdev);
	int minor = MINOR(inode->i_rdev);

	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	switch (cmd)
	{
	case BLKGETSIZE:
		return put_user(mmcsd_gendisk.part[minor].nr_sects, (long *) arg);
#undef BLKGETSIZE64
#ifdef BLKGETSIZE64
	case BLKGETSIZE64:
		return put_user((u64)mmcsd_gendisk.part[minor].nr_sects,
				(u64 *)arg);
#endif
	case BLKFLSBUF:
		if (!capable(CAP_SYS_ADMIN)) return -EACCES;
		invalidate_device(inode->i_rdev, 1);
		return 0;
	case HDIO_GETGEO: {
		struct hd_geometry geo;
		geo.cylinders = 1;
		geo.heads = 1;
		geo.sectors = mmcsd_gendisk.part[minor].nr_sects;
		geo.start = mmcsd_gendisk.part[minor].start_sect;
		return copy_to_user((void *)arg, &geo, sizeof(geo)) ?
			-EFAULT : 0;
	}
	case BLKRRPART: {
		int i;
		if (!capable(CAP_SYS_ADMIN)) return -EACCES;
		if (SD_PRIV(dev)->use_cnt > 1) return -EBUSY;
		for (i = mmcsd_gendisk.max_p - 1; i >= 0; i--) {
			if (mmcsd_gendisk.part[i].nr_sects)
			{
				kdev_t devid = MKDEV(MAJOR(inode->i_rdev),
						     minor + i);
				invalidate_device(devid, 1);
				mmcsd_gendisk.part[minor + i].start_sect = 0;
				mmcsd_gendisk.part[minor + i].nr_sects = 0;
			}
		}
		grok_partitions(&mmcsd_gendisk, dev, mmcsd_gendisk.max_p,
				SD_SLOT(dev)->size / SD_SLOT(dev)->read_len);
		return 0;
	}
	case BLKROSET:
	case BLKROGET:
	case BLKSSZGET:
		return blk_ioctl(inode->i_rdev, cmd, arg);
	default:
		return -EINVAL;
	}
}

/************************************************************************
 * MMC/SD open
 *
 * Description: This function open MMC/SD device driver
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
static int mmcsd_open(struct inode *inode, struct file *filp)
{
	int dev = DEVICE_NR(inode->i_rdev);

	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	if (dev >= MAX_MMCSD_SLOTS) return -ENODEV;
	if (IS_EMPTY(dev)) return -ENODEV;

	if ((filp->f_mode & FMODE_WRITE) && SD_SLOT(dev)->readonly) {
		printk("%s readonly\n", SD_SLOT(dev)->sd ? "SD": "MMC");
		return -EROFS;
	}

	SD_PRIV(dev)->use_cnt++;
	MOD_INC_USE_COUNT;
	return 0;
}

/************************************************************************
 * Release MMC/SD device driver
 *
 * Description: This function Release MMC/SD device driver
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
static int mmcsd_release(struct inode * inode, struct file * filp)
{
	int dev = DEVICE_NR(inode->i_rdev);

	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	invalidate_device(inode->i_rdev, 1);
	SD_PRIV(dev)->use_cnt--;
	if (!mmcsd_sizes[MINOR(inode->i_rdev)]) {
		SD_SLOT(dev) = NULL;
	}
	MOD_DEC_USE_COUNT;
	return 0;
}

static struct block_device_operations mmcsd_fops = {
	owner:			THIS_MODULE,
	open:			mmcsd_open,
	release:		mmcsd_release,
	ioctl:			mmcsd_ioctl,
};

/************************************************************************
 * Notify MMC/SD
 *
 * Description: This function register MMC/SD
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
static void mmcsd_notify_add(struct mmcsd_slot *slot)
{
	int i, dev, minor;
	loff_t		size;
	unsigned int nsize;


	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	for (i = 0, dev = -1; i < MAX_MMCSD_SLOTS; i++) {
		if (IS_EMPTY(i)) {
			if (dev == -1) dev = i;
		} else if (SD_SLOT(i)->id == slot->id) {
			/* already mounted */
			dev = i;
			break;
		}
	}
	if (dev == -1) return;

	SD_SLOT(dev) = slot;
	minor = dev << mmcsd_gendisk.minor_shift;
	mmcsd_gendisk.part[minor].start_sect = 0;
	mmcsd_gendisk.nr_real++;

	nsize=SD_SLOT(dev)->read_len;

	register_disk(&mmcsd_gendisk, MKDEV(MAJOR_NR, minor),
		       mmcsd_gendisk.max_p, mmcsd_gendisk.fops,
		      SD_SLOT(dev)->size / SD_SLOT(dev)->read_len);

	/* Check usb */
	switch(nsize)
	{
		case 512:
			mmsp_set_sdsize(0);
			break;
		case 1024:
			mmsp_set_sdsize(1);
			break;
		case 2048:
			mmsp_set_sdsize(2);
			break;
		case 4096:
			mmsp_set_sdsize(3);
			break;
		case 8192:
			mmsp_set_sdsize(4);
			break;
	}

	printk("Register %s: %ldMsB\n", SD_SLOT(dev)->sd ? "SD": "MMC", SD_SLOT(dev)->size/(1024 * 1024));

}

/************************************************************************
 * Remove MMC/SD
 *
 * Description: This function Remove MMC/SD
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
static void mmcsd_notify_remove(struct mmcsd_slot *slot)
{
	int dev, minor;

	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	for (dev = 0; dev < MAX_MMCSD_SLOTS; dev++) {
		if (SD_SLOT(dev) == slot) {
			break;
		}
	}
	if (dev >= MAX_MMCSD_SLOTS) return;

	minor = dev << mmcsd_gendisk.minor_shift;
	/* Unregister the related device files at the /dev/mmcsd directory,
	   grok_partitions(*, *, *, 0) does not work here */
	devfs_register_partitions (&mmcsd_gendisk, minor, 1);
	mmcsd_gendisk.nr_real--;
	if (!(SD_PRIV(dev)->use_cnt)) {
		SD_SLOT(dev) = NULL;
	}

	for (; minor < (dev + 1) << mmcsd_gendisk.minor_shift; minor++) {
		if (mmcsd_sizes[minor]) {
			mmcsd_sizes[minor] = 0;
		}
		mmcsd_gendisk.part[minor].nr_sects = 0;
	}

	printk("Unregister %s\n", slot->sd ? "SD": "MMC");
}

static struct mmcsd_notifier disk_notifier = {
	add:    mmcsd_notify_add,
	remove: mmcsd_notify_remove,
};

/************************************************************************
 * Initialization MMC/SD device driver
 *
 * Description: This function Initializ MMC/SD device driver
 *
 * Arguments  : none
 *
 * Returns    : none
 *
*************************************************************************/
static int __init init_mmcsd_disk(void)
{
	/* MMC/SD regisger to block device driver */
	if (devfs_register_blkdev(MAJOR_NR, DEVICE_NAME, &mmcsd_fops))
	{
		printk("MMC/SD Disk: failed to register major %d\n", MAJOR_NR);
		return -EBUSY;
	}

	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	mmcsd_gendisk.fops = &mmcsd_fops;
	memset(mmcsd_gendisk.flags, GENHD_FL_REMOVABLE, MAX_MMCSD_SLOTS);

	blksize_size[MAJOR_NR] = mmcsd_blksizes;	/* Size of all block-devices */
	blk_size[MAJOR_NR] = mmcsd_sizes;

#if 0
	read_ahead [MAJOR_NR] = 8;
#endif

	blk_init_queue(BLK_DEFAULT_QUEUE(MAJOR_NR), DEVICE_REQUEST);

	add_gendisk(&mmcsd_gendisk);			/* Register the partitioning information */

	register_mmcsd_user(&disk_notifier);

	return 0;
}

static void __exit exit_mmcsd_disk(void)
{
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	unregister_mmcsd_user(&disk_notifier);

	devfs_unregister_blkdev(MAJOR_NR, DEVICE_NAME);

	blk_cleanup_queue(BLK_DEFAULT_QUEUE(MAJOR_NR));

	del_gendisk(&mmcsd_gendisk);

	blk_size[MAJOR_NR] = NULL;
	blksize_size[MAJOR_NR] = NULL;
}

module_init(init_mmcsd_disk);
module_exit(exit_mmcsd_disk);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
MODULE_LICENSE("Not GPL, Proprietary License");
MODULE_DESCRIPTION("MMC/SD Memory Card block device driver");
