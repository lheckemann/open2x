/*
 * drivers/char/char/dualcpu.c
 *
 * Copyright (C) 2005,2006 Gamepark Holdings, Inc. (www.gp2x.com)
 * Hyun <hyun3678@gp2x.com>
 *
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 *  MMSP2 DUAL CPU Interface driver
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

#include <asm/arch/proto_dualcpu.h>
#include <asm/arch/mmsp2-regs.h>
#include <asm/arch/mmsp20.h>
#include "dualcpu.h"

#include "arm940firm.h"
#include "m4enc_encore.h"

#define DEVICE_NAME "dualcpu"

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(x...) printk(x)
#else
#  define ds_printk(fmt, args...)
#endif

//#define OLD_IOCTL_STYLE
#undef OLD_IOCTL_STYLE

static int dualcpu_rdopen = 0;
static int dualcpu_wdopen = 0;

wait_queue_head_t	wait_rq;
wait_queue_head_t	wait_wq;

static int arm940_write_to_920_and_interrupt=0;

static MP4D_INIT_PARAM      mp4d_init;
static MP4D_INIT_BUF_PARAM  mp4d_init_buf;
static MP4D_RUN_PARAM       mp4d_run;
static MP4D_REL_PARAM       mp4d_rel;
static MP4D_DISPLAY_PARAM   mp4d_disp;

static CMDPARAMS cmd_params;
static OPENHANDLEPARAMS open_handle_params;
static REFBUFPARAMS ref_buf_params;
static ETCBUFPARAMS etc_buf_params;
static RCPARAMS rc_params;
static ENC_FRAME enc_frame;
static ENC_RESULT enc_result;

static MJPD_INIT_PARAM mjpd_init;
static MJPD_RUN_PARAM mjpd_run;
static MJPD_GETINFO_PARAM mjpd_getinfo;
static MJPD_REL_PARAM mjpd_rel;

static MJPE_RUN_PARAM mjpe_run;
static MJPE_GETINFO_PARAM mjpe_getinfo;
static MJPE_ENC_RESULT mjpe_status;


#define A940TCLK	(1<<0)
#define FASTIOCLK	(1<<10)


#define DBLKCLK		(1<<15)
#define MPGIFCLK	(1<<14)
#define DECSSCLK	(1<<13)
#define MECLK		(1<<12)
#define REFWCLK		(1<<11)
#define RVLDCLK		(1<<10)
#define QGMCLK		(1<<9)
#define MPMCLK		(1<<8)
#define PDFCLK		(1<<7)
#define HUFFCLK		(1<<6)
#define VPCLKGOFF	(1<<5)	/* DISABLE BIT*/
#define VPTOPCLK	(1<<4)
#define ISPCLK		(1<<3)
#define GRPCLK		(1<<2)
#define SCALECLK	(1<<1)
#define DISPCLK		(0<<1)
//#define ALL_VIDEO_CLK_MASK	0xFFFF
#define ALL_VIDEO_CLK_MASK	0xFFF7

/*
	MP3 static
*/
static MP3DecFrame 		mp3_decframe;
static MP3DecResult		mp3_decresult;
static MP3StreamInfo	mp3_streaminfo;



#ifdef DS_DEBUG
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
#endif

inline void Set940DataReg(unsigned short *data, int num)
{
    int i;

    for(i=num-1; i>0; i--) {
        D940DATA(i) = data[i];
    }
    DINT940 = 0x01;
    D940DATA(0) = data[0];
}

inline void Get920DataReg(unsigned short *data, int num)
{
    int i;

    for(i=num-1; i>=0; i--) {
        data[i] = D920DATA(i);
    }
}

static ssize_t dualcpu_read (struct file *filp, char *buf, size_t count, loff_t *f_pos)
{

	ds_printk("IOCTL_DUALCPU_MJPD_read arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
	if(wait_event_interruptible(wait_rq,(arm940_write_to_920_and_interrupt==1)))
	{
		return -ERESTARTSYS;
	}

    Get920DataReg((unsigned short *)&mp4d_disp, sizeof(MP4D_DISPLAY_PARAM)/2);

	if(copy_to_user(buf, &mp4d_disp, sizeof(MP4D_DISPLAY_PARAM)) )
	{
		printk("%s :%s failed \n", DEVICE_NAME,__FUNCTION__);
		return -EFAULT;
	}

    arm940_write_to_920_and_interrupt = 0;

	return 1;
}

#if 0
static ssize_t dualcpu_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	pamm_msg_t msg,*msgp=&msg;

	if(copy_from_user(msgp ,buf, count)){
		printk("%s :%s failed \n",DEVICE_NAME,__FUNCTION__);
                return -EFAULT;
	}

	switch(msgp->mtype){
		case MINFO_HOST_FAILED:
			ishostfailed=1;
			break;
		case MRES_REDIRECT_HOST:
			wq_redirect=1;
			wake_up_interruptible(&wait_redirect);
			return count;
		default:
			break;
	}
	if(enq_wq(msgp)<0){
		/* i hope this will not happen */
		return -EFAULT;
	}

	/*
	 * how check whether user msg arriave
	 * deq_wq != NULL,
	 *
	 */
	return count;
}
#endif



static void dualcpu_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
    //ARM920_ClearInterrupt();
    DPEND920 = 0xFF; // interrupt pending bit all disable
    DINT920 = 0x00;  // interrupt status register all clear

    arm940_write_to_920_and_interrupt = 1;

#ifndef OLD_IOCTL_STYLE
	wake_up_interruptible(&wait_rq);
#endif

}

static int dualcpu_ioctl(struct inode *inode, struct file *file,
    unsigned int cmd, unsigned long arg)
{
    int ret = 0;

// Delete following lines after
// "#define IOCTL_DUALCPU_MP4D_INIT     _IOW('d', 0x01, MP4D_INIT_PARAM)"
// is fixed both side - this & application
#define FAKE_CMD_FOR_UPSIZING_OF_MP4D_INIT_PARAM	0

    switch(cmd) {
#if FAKE_CMD_FOR_UPSIZING_OF_MP4D_INIT_PARAM
    case 1075602433:
#else
    case IOCTL_DUALCPU_MP4D_INIT:
#endif
        	ds_printk("IOCTL_DUALCPU_MP4D_INIT: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
        if(arm940_write_to_920_and_interrupt==0) {
			ds_printk("IOCTL_DUALCPU_MP4D_INIT\n");
            if(copy_from_user(&mp4d_init, (MP4D_INIT_PARAM *)arg, sizeof(MP4D_INIT_PARAM)))
                return -EFAULT;

            Set940DataReg((unsigned short *)&mp4d_init, sizeof(MP4D_INIT_PARAM)/2);
            ret = 1;
        }
        break;
    case IOCTL_DUALCPU_MP4D_INIT_BUF:
        	ds_printk("IOCTL_DUALCPU_MP4D_INIT_BUF: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
        if(arm940_write_to_920_and_interrupt==0) {
			ds_printk("IOCTL_DUALCPU_MP4D_INIT_BUF\n");
            if(copy_from_user(&mp4d_init_buf, (MP4D_INIT_BUF_PARAM *)arg, sizeof(MP4D_INIT_BUF_PARAM)))
                return -EFAULT;
            Set940DataReg((unsigned short *)&mp4d_init_buf, sizeof(MP4D_INIT_BUF_PARAM)/2);
            ret = 1;
        }
        break;
    case IOCTL_DUALCPU_MP4D_RUN:
        	ds_printk("IOCTL_DUALCPU_MP4D_RUN: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
        if(arm940_write_to_920_and_interrupt==0) {
			ds_printk("IOCTL_DUALCPU_MP4D_RUN\n");
            if(copy_from_user(&mp4d_run, (MP4D_RUN_PARAM *)arg, sizeof(MP4D_RUN_PARAM)))
                return -EFAULT;
            Set940DataReg((unsigned short *)&mp4d_run, sizeof(MP4D_RUN_PARAM)/2);
            ret = 1;
        }
        break;
    case IOCTL_DUALCPU_MP4D_RELEASE:
        	ds_printk("IOCTL_DUALCPU_MP4D_RELEASE: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
        if(arm940_write_to_920_and_interrupt==0) {
			ds_printk("IOCTL_DUALCPU_MP4D_RELEASE\n");
            if(copy_from_user(&mp4d_rel, (MP4D_REL_PARAM *)arg, sizeof(MP4D_REL_PARAM)))
                return -EFAULT;
            Set940DataReg((unsigned short *)&mp4d_rel, sizeof(MP4D_REL_PARAM)/2);
            ret = 1;
        }
        break;
#ifdef OLD_IOCTL_STYLE
    case IOCTL_DUALCPU_MP4D_STATUS:
        if(arm940_write_to_920_and_interrupt==1) 
        {
            arm940_write_to_920_and_interrupt = 0;
            Get920DataReg((unsigned short *)&mp4d_disp, sizeof(MP4D_DISPLAY_PARAM)/2);
            if(copy_to_user((MP4D_DISPLAY_PARAM *)arg, &mp4d_disp, sizeof(MP4D_DISPLAY_PARAM)))
                return -EFAULT;
            ret = 1;
        }
        break;
#endif
	case IOCTL_DUALCPU_MP4E_OPENHANDLE:
		if(arm940_write_to_920_and_interrupt==0) {
			if(copy_from_user(&open_handle_params, (OPENHANDLEPARAMS *)arg, sizeof(OPENHANDLEPARAMS)))
				return -EFAULT;
            #ifdef DS_DEBUG
			ds_printk("MP4E OPEN HANDLE:\n");
			DumpData((unsigned short *)&open_handle_params, sizeof(OPENHANDLEPARAMS)/2);
			#endif
			Set940DataReg((unsigned short *)&open_handle_params, sizeof(OPENHANDLEPARAMS)/2);
			ret = 1;
		}
		break;
	case IOCTL_DUALCPU_MP4E_REFBUF:
		if(arm940_write_to_920_and_interrupt==0) {
			if(copy_from_user(&ref_buf_params, (REFBUFPARAMS *)arg, sizeof(REFBUFPARAMS)))
				return -EFAULT;
            #ifdef DS_DEBUG
			ds_printk("MP4E REF BUFFER INIT:\n");
			DumpData((unsigned short *)&ref_buf_params, sizeof(REFBUFPARAMS)/2);
			#endif
			Set940DataReg((unsigned short *)&ref_buf_params, sizeof(REFBUFPARAMS)/2);
			ret = 1;
		}
		break;
	case IOCTL_DUALCPU_MP4E_ETCBUF:
		if(arm940_write_to_920_and_interrupt==0) {
			if(copy_from_user(&etc_buf_params, (ETCBUFPARAMS *)arg, sizeof(ETCBUFPARAMS)))
				return -EFAULT;
            #ifdef DS_DEBUG
			ds_printk("MP4E ETC BUFFER INIT:\n");
			DumpData((unsigned short *)&etc_buf_params, sizeof(ETCBUFPARAMS)/2);
			#endif
			Set940DataReg((unsigned short *)&etc_buf_params, sizeof(ETCBUFPARAMS)/2);
			ret = 1;
		}
		break;
	case IOCTL_DUALCPU_MP4E_RCPARM:
		if(arm940_write_to_920_and_interrupt==0) {
			if(copy_from_user(&rc_params, (RCPARAMS *)arg, sizeof(RCPARAMS)))
				return -EFAULT;
            #ifdef DS_DEBUG
			ds_printk("MP4E Rate Control INIT:\n");
			DumpData((unsigned short *)&rc_params, sizeof(RCPARAMS)/2);
			#endif
			Set940DataReg((unsigned short *)&rc_params, sizeof(RCPARAMS)/2);
			ret = 1;
		}
		break;
	case IOCTL_DUALCPU_MP4E_INITDEVICE:
		if(arm940_write_to_920_and_interrupt==0) {
			if(copy_from_user(&cmd_params, (CMDPARAMS *)arg, sizeof(CMDPARAMS)))
				return -EFAULT;
            #ifdef DS_DEBUG
			ds_printk("MP4E INIT DEVICE:\n");
			DumpData((unsigned short *)&cmd_params, sizeof(CMDPARAMS)/2);
			#endif
			Set940DataReg((unsigned short *)&cmd_params, sizeof(CMDPARAMS)/2);
			ret = 1;
		}
		break;
	case IOCTL_DUALCPU_MP4E_RUN:
		if(arm940_write_to_920_and_interrupt==0) {
			if(copy_from_user(&enc_frame, (ENC_FRAME *)arg, sizeof(ENC_FRAME)))
				return -EFAULT;
            #ifdef DS_DEBUG
			ds_printk("MP4E RUN:\n");
			DumpData((unsigned short *)&enc_frame, sizeof(ENC_FRAME)/2);
			#endif
			Set940DataReg((unsigned short *)&enc_frame, sizeof(ENC_FRAME)/2);
			ret = 1;
		}
		break;
	case IOCTL_DUALCPU_MP4E_RELEASE:
		if(arm940_write_to_920_and_interrupt==0) {
			if(copy_from_user(&cmd_params, (CMDPARAMS *)arg, sizeof(CMDPARAMS)))
				return -EFAULT;
            #ifdef DS_DEBUG
			ds_printk("MP4E RELEASE:\n");
			DumpData((unsigned short *)&cmd_params, sizeof(CMDPARAMS)/2);
			#endif
			Set940DataReg((unsigned short *)&cmd_params, sizeof(CMDPARAMS)/2);
			ret = 1;
		}
		break;
	case IOCTL_DUALCPU_MP4E_STATUS:
			if(wait_event_interruptible(wait_rq,(arm940_write_to_920_and_interrupt==1)))
			{
				return -ERESTARTSYS;
			}
			Get920DataReg((unsigned short *)&enc_result, sizeof(ENC_RESULT)/2);
            #ifdef DS_DEBUG
			ds_printk("MP4E RESULT:\n");
			DumpData((unsigned short *)&enc_result, sizeof(ENC_RESULT)/2);
			#endif
			if(copy_to_user((ENC_RESULT *)arg, &enc_result, sizeof(ENC_RESULT)))
				return -EFAULT;

			arm940_write_to_920_and_interrupt = 0;
			ret = 1;
		break;
	case IOCTL_DUALCPU_MJPD_INIT:
        	ds_printk("IOCTL_DUALCPU_MJPD_INIT: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
		if(arm940_write_to_920_and_interrupt==0) {
			if(copy_from_user(&mjpd_init, (MJPD_INIT_PARAM *)arg, sizeof(MJPD_INIT_PARAM)))
				return -EFAULT;
        	ds_printk("IOCTL_DUALCPU_MJPD_INIT: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
            #ifdef DS_DEBUG
			ds_printk("MJPD INIT_CMD:\n");
			DumpData((unsigned short *)&mjpd_init, sizeof(MJPD_INIT_PARAM)/2);
			#endif
			Set940DataReg((unsigned short *)&mjpd_init, sizeof(MJPD_INIT_PARAM)/2);
			arm940_write_to_920_and_interrupt = 0;
			ret = 1;
		}
		break;
	case IOCTL_DUALCPU_MJPD_RUN:
        	ds_printk("IOCTL_DUALCPU_MJPD_RUN: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
			if(wait_event_interruptible(wait_rq,(arm940_write_to_920_and_interrupt==1)))
			{
				return -ERESTARTSYS;
			}
        	ds_printk("IOCTL_DUALCPU_MJPD_RUN: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
			if(copy_from_user(&mjpd_run, (MJPD_RUN_PARAM *)arg, sizeof(MJPD_RUN_PARAM)))
				return -EFAULT;
            #ifdef DS_DEBUG
			ds_printk("MJPD RUN_CMD:\n");
			DumpData((unsigned short *)&mjpd_run, sizeof(MJPD_RUN_PARAM)/2);
			#endif
			Set940DataReg((unsigned short *)&mjpd_run, sizeof(MJPD_RUN_PARAM)/2);
			arm940_write_to_920_and_interrupt = 0;
			ret = 1;
		break;
	case IOCTL_DUALCPU_MJPD_GETINFO:
        	ds_printk("IOCTL_DUALCPU_MJPD_GETINFO: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
			if(wait_event_interruptible(wait_rq,(arm940_write_to_920_and_interrupt==1)))
			{
				return -ERESTARTSYS;
			}
        	ds_printk("IOCTL_DUALCPU_MJPD_GETINFO: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
			if(copy_from_user(&mjpd_getinfo, (MJPD_GETINFO_PARAM *)arg, sizeof(MJPD_GETINFO_PARAM)))
				return -EFAULT;
            #ifdef DS_DEBUG
			ds_printk("MJPD GETINFO_CMD:\n");
			DumpData((unsigned short *)&mjpd_getinfo, sizeof(MJPD_GETINFO_PARAM)/2);
			#endif
			Set940DataReg((unsigned short *)&mjpd_getinfo, sizeof(MJPD_GETINFO_PARAM)/2);
			arm940_write_to_920_and_interrupt = 0;
			ret = 1;
		break;
	case IOCTL_DUALCPU_MJPD_RELEASE:
        	ds_printk("IOCTL_DUALCPU_MJPD_RELEASE: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
			if(wait_event_interruptible(wait_rq,(arm940_write_to_920_and_interrupt==1)))
			{
				return -ERESTARTSYS;
			}
        	ds_printk("IOCTL_DUALCPU_MJPD_RELEASE: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
			if(copy_from_user(&mjpd_rel, (MJPD_REL_PARAM *)arg, sizeof(MJPD_REL_PARAM)))
				return -EFAULT;
            #ifdef DS_DEBUG
			ds_printk("MJPD RELEASE_CMD:\n");
			DumpData((unsigned short *)&mjpd_rel, sizeof(MJPD_REL_PARAM)/2);
			#endif
			Set940DataReg((unsigned short *)&mjpd_rel, sizeof(MJPD_REL_PARAM)/2);
			arm940_write_to_920_and_interrupt = 0;
			ret = 1;
		break;
	case IOCTL_DUALCPU_MJPE_RUN:
        	ds_printk("IOCTL_DUALCPU_MJPE_RUN: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
		if(arm940_write_to_920_and_interrupt==0) {
			if(copy_from_user(&mjpe_run, (MJPE_RUN_PARAM *)arg, sizeof(MJPE_RUN_PARAM)))
				return -EFAULT;
			#ifdef DS_DEBUG
			ds_printk("MJPE RUN_CMD:\n");
			DumpData((unsigned short *)&mjpe_run, sizeof(MJPE_RUN_PARAM)/2);
			#endif
			Set940DataReg((unsigned short *)&mjpe_run, sizeof(MJPE_RUN_PARAM)/2);
			arm940_write_to_920_and_interrupt=0;
			ret = 1;
		}
		break;
	case IOCTL_DUALCPU_MJPE_GETINFO:
        	ds_printk("IOCTL_DUALCPU_MJPE_GETINFO: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
			if(wait_event_interruptible(wait_rq,(arm940_write_to_920_and_interrupt==1)))
			{
				return -ERESTARTSYS;
			}
        	ds_printk("IOCTL_DUALCPU_MJPE_GETINFO: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
			if(copy_from_user(&mjpe_getinfo, (MJPE_GETINFO_PARAM *)arg, sizeof(MJPE_GETINFO_PARAM)))
				return -EFAULT;
			#ifdef DS_DEBUG
			ds_printk("MJPE GETINFO_CMD:\n");
			DumpData((unsigned short *)&mjpe_getinfo, sizeof(MJPE_GETINFO_PARAM)/2);
			#endif
			Set940DataReg((unsigned short *)&mjpe_getinfo, sizeof(MJPE_GETINFO_PARAM)/2);
			arm940_write_to_920_and_interrupt=0;
			ret = 1;
		break;
    case IOCTL_DUALCPU_MJPE_STATUS:
        	ds_printk("IOCTL_DUALCPU_MJPE_STATUS: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
			if(wait_event_interruptible(wait_rq,(arm940_write_to_920_and_interrupt==1)))
			{
				return -ERESTARTSYS;
			}
			ds_printk("IOCTL_DUALCPU_MJPE_STATUS: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
            Get920DataReg((unsigned short *)&mjpe_status, sizeof(MJPE_ENC_RESULT)/2);
			#ifdef DS_DEBUG
			ds_printk("MJPE STATUS:\n");
			DumpData((unsigned short *)&mjpe_status, sizeof(MJPE_ENC_RESULT)/2);
			#endif
            if(copy_to_user((MJPE_ENC_RESULT *)arg, &mjpe_status, sizeof(MJPE_ENC_RESULT)))
                return -EFAULT;
			arm940_write_to_920_and_interrupt=0;
            ret = 1;
		break;
/*-----------------------------------------------------------------------------------------------------
//	For MP3 Decoder API
-----------------------------------------------------------------------------------------------------*/
	case IOCTL_DUALCPU_MP3D_SEND_DECFRAME:
        	ds_printk("IOCTL_DUALCPU_MP3D_SEND_DECFRAME: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
        if(arm940_write_to_920_and_interrupt==0) {
        	arm940_write_to_920_and_interrupt=0;

			ds_printk("IOCTL_DUALCPU_MP3D_SEND_DECFRAME\n");
            if(copy_from_user(&mp3_decframe, (MP3DecFrame *)arg, sizeof(MP3DecFrame)))
                return -EFAULT;

            Set940DataReg((unsigned short *)&mp3_decframe, sizeof(MP3DecFrame)/2);
            ret = 1;
        }
        break;

	case IOCTL_DUALCPU_MP3D_GET_DECRESULT:
			ds_printk("IOCTL_DUALCPU_MP3D_GET_DECRESULT: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
			if(wait_event_interruptible(wait_rq,(arm940_write_to_920_and_interrupt==1)))
			{
				return -ERESTARTSYS;
			}
			ds_printk("IOCTL_DUALCPU_MP3D_GET_DECRESULT: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
            Get920DataReg((unsigned short *)&mp3_decresult, sizeof(MP3DecResult)/2);
            if(copy_to_user((MP3DecResult *)arg, &mp3_decresult, sizeof(MP3DecResult)))
                return -EFAULT;
			arm940_write_to_920_and_interrupt=0;
            ret = 1;
		break;
	case IOCTL_DUALCPU_MP3D_GET_MP3STREAMINFO:
			ds_printk("IOCTL_DUALCPU_MP3D_GET_MP3STREAMINFO: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
			if(wait_event_interruptible(wait_rq,(arm940_write_to_920_and_interrupt==1)))
			{
				return -ERESTARTSYS;
			}
			ds_printk("IOCTL_DUALCPU_MP3D_GET_MP3STREAMINFO: arm940_write_to_920_and_interrupt = %d\n", arm940_write_to_920_and_interrupt);
            Get920DataReg((unsigned short *)&mp3_streaminfo, sizeof(MP3StreamInfo)/2);
            if(copy_to_user((MP3StreamInfo *)arg, &mp3_streaminfo, sizeof(MP3StreamInfo)))
                return -EFAULT;
			arm940_write_to_920_and_interrupt=0;
            ret = 1;
		break;
/*-----------------------------------------------------------------------------------------------------
//	For MP3 Decoder API
-----------------------------------------------------------------------------------------------------*/
    default:
        return -EINVAL;
    }
    return ret;
}


static int dualcpu_open(struct inode *inode, struct file *filp)
{
#if 1
	MMSP20_CLOCK_POWER *pPMR_REG;
	pPMR_REG = MMSP20_GetBase_CLOCK_POWER();
#endif

	if ((filp->f_flags & O_ACCMODE) & (O_WRONLY | O_RDWR))
	{
        if (dualcpu_wdopen)
        {
		    printk("%s :Device already open for writing\n",DEVICE_NAME);
        	return -EBUSY;
        }
		else
            ++dualcpu_wdopen;
	}
	else
	{
		if (dualcpu_rdopen)
		{
		    printk("%s :Device already open for reading\n",DEVICE_NAME);
			return -EBUSY;
        }
		else
            ++dualcpu_rdopen;
	}

	MOD_INC_USE_COUNT;


#if 1
	pPMR_REG->SYSCLKENREG|=A940TCLK;
	mdelay(200);
	DualCPU_Initialize();
    DualCPU_InitHardware();

    DualCPU_operate_mode_on();
#endif

#if 1
	pPMR_REG->SYSCLKENREG|=FASTIOCLK;
	pPMR_REG->VGCLKENREG=ALL_VIDEO_CLK_MASK;
	mdelay(300);
#endif
	return 0;

}

static int dualcpu_release(struct inode *inode, struct file *filp)
{
#if 1
	MMSP20_CLOCK_POWER *pPMR_REG;
	pPMR_REG = MMSP20_GetBase_CLOCK_POWER();
#endif
	if ((filp->f_flags & O_ACCMODE) & (O_WRONLY | O_RDWR))
		--dualcpu_wdopen;
	else
		--dualcpu_rdopen;

	arm940_write_to_920_and_interrupt = 0; // hhsong 050708

	MOD_DEC_USE_COUNT;
#if 1
	pPMR_REG->SYSCLKENREG&=~(FASTIOCLK|A940TCLK);
	pPMR_REG->VGCLKENREG=0x01;
	mdelay(500);
#endif

    return 0;
}

static struct file_operations mmsp2_dualcpu_fops = {
    owner:      THIS_MODULE,
    open:       dualcpu_open,
    release:    dualcpu_release,
    read:       dualcpu_read,
    ioctl:      dualcpu_ioctl,
};


static struct miscdevice mmsp2_dualcpu = {
    DUALCPU_MINOR, "dualcpu", &mmsp2_dualcpu_fops
};

static int __init mmsp2_dualcpu_init(void)
{
    int i;
	volatile unsigned char *addr;

#if 0
	DualCPU_Initialize();
    DualCPU_InitHardware();

    addr = (volatile unsigned char *)VA_ARM940_BASE;
   	memcpy((void *)addr, arm940code, sizeof(arm940code));

    DualCPU_operate_mode_on();
#else
 	addr = (volatile unsigned char *)VA_ARM940_BASE;
  	memcpy((void *)addr, arm940code, sizeof(arm940code));
#endif

    i = request_irq(IRQ_DUALCPU, dualcpu_interrupt, SA_INTERRUPT, "DUALCPU", NULL);
    if(i) {
        ds_printk("dualcpu.c : request_irq() failed\n");
        return -EIO;
    }

    misc_register(&mmsp2_dualcpu);

	init_waitqueue_head(&wait_rq);

	printk("MMSP2 DualCPU Interface Driver\n");

    return 0;
}

static void __exit mmsp2_dualcpu_exit(void)
{
    DualCPU_CloseHardware();

    free_irq(IRQ_DUALCPU, NULL);
    //DUALCTRL940 |= (1 << 7);

    misc_deregister(&mmsp2_dualcpu);
}

module_init(mmsp2_dualcpu_init);
module_exit(mmsp2_dualcpu_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
MODULE_DESCRIPTION("MMSP2 DUAL CPU Interface driver");
