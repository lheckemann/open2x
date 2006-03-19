/*
 *  linux/kernel/drivers/char/isp.c
 *
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 *  Video Post Processor driver
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

#include <asm/arch/proto_isp.h>
#include <asm/arch/mmsp20.h>

#include "isp.h"

static int isp_count=0;

extern MMSP20_ISP *ISP_REG;

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(x...) printk(x)
#else
#  define ds_printk(fmt, args...)
#endif

static int isp_open(struct inode *inode, struct file *file)
{
	if(isp_count==0) {
		// Vsync, Hsync Interrupt Enable
		ISP_SetInterruptEnable  ( ISP_INT_VSYNC | ISP_INT_HSYNC);
		isp_count = 1;
		return 0;
	}
	else
		return -EBUSY;
}

static int isp_release(struct inode *inode, struct file *file)
{
	// Vsync, Hsync Interrupt Disable
	ISP_SetInterruptEnable  ( ISP_INT_NONE );
	isp_count = 0;
	return 0;
}

static int isp_ioctl(struct inode *inode, struct file *file,
                     unsigned int cmd, unsigned long arg)
{
	int ret=0;
	SYNC_INFO sync_info;
	PRE_WINDOW_INFO pre_win_info;
	AV_WINDOW_INFO av_win_info;
	
	switch(cmd)
	{
		case ISP_SET_INPUT_DATA_FORMAT :
				ISP_SetInputDataFormat(arg);
				break;

		case ISP_SET_OPER_MODE :
				ISP_SetOperMode(arg);
				break;				
				
		case ISP_SET_BAYER_RGB_ARRAY :
				ISP_SetBayerRGBArray(arg);
				break;
		
		case ISP_SET_SYNC : 
				if(copy_from_user(&sync_info, (SYNC_INFO *)arg, sizeof(SYNC_INFO)))
					return -EFAULT;
				ISP_SetSync(sync_info.GenField, sync_info.VerSyncStPnt, sync_info.HorSyncPol, sync_info.VerSyncPol,
							sync_info.HorSyncPeriod, sync_info.HorSyncPulse, sync_info.HorEAVtoSyncStart, 
							sync_info.VerSyncPeriod, sync_info.VerSyncPulse, sync_info.VerEAVtoSyncStart);
				break;

		case ISP_SET_PRE_WINDOW : 
				if(copy_from_user(&pre_win_info, (PRE_WINDOW_INFO *)arg, sizeof(PRE_WINDOW_INFO)))
					return -EFAULT;
				ISP_SetPreWindow(pre_win_info.RGBHorStart, pre_win_info.RGBHorWidth,
								 pre_win_info.RGBVerStart, pre_win_info.RGBVerHeight);
				break;

		case ISP_SET_AV_WINDOW : 
				if(copy_from_user(&av_win_info, (AV_WINDOW_INFO *)arg, sizeof(AV_WINDOW_INFO)))
					return -EFAULT;
				ISP_SetAvWindow(av_win_info.AVHorStart, av_win_info.AVHorWidth,
								av_win_info.AVVerStart, av_win_info.AVVerHeight);
				break;
				
		case ENABLE_ISP :
				ISP_EnableISP( arg );
				break;
				
		case START_ISP :	
				ISP_StartISP();
				break;

		default:
			ret = -EINVAL;
	}
	return ret;
}

static struct file_operations isp_fops = {
	owner:		THIS_MODULE,
	ioctl:		isp_ioctl,
	open:		isp_open,
	release:	isp_release,
};

static struct miscdevice isp_miscdev = {
	ISP_MINOR, "isp", &isp_fops
};

static int __init isp_init(void)
{
	ISP_Initialize();
	/* Image Signal Processor setup start */

	ISP_SetInputDataFormat(ISP_F656P_GPCH);
	ISP_SetMasterMode(ISP_SLAVE);
	ISP_Clock(ISP_CLK_DIV_NOR, ISP_CLK_DIV_NOR);
	ISP_SetSync(ISP_FIELD_F656, ISP_VSYNC_RIS, ISP_SYNC_POL_NOR, ISP_SYNC_POL_NOR, 0, 0, 0, 0, 0, 0);
	ISP_ExtDataBitWidth(ISP_EXT_BIT_8);
	ISP_MemDataBitWidth(ISP_MEM_BIT_8);
	ISP_SetMemRGBPixelWidth(ISP_RGB_24BPP, 640, ISP_RGB_24BPP, 640);
	ISP_SetBayerRGBArray(ISP_BAY_BG);
	ISP_SetRGBRecovType(ISP_RECV_MEDIAN, ISP_RECV_MEDIAN, ISP_RECV_MEDIAN);
	ISP_RGBOrder(ISP_ORD_RGB);
	ISP_SetCrCbDeciFtOn(ISP_C_DECI_OFF);
	ISP_SetChromaDelay(0);
	ISP_SetCbCrSwap(ISP_CRCBSWAP_OFF);
	ISP_SetVideoRange(ISP_VID_255);
	ISP_UseGamma(ISP_GAMMA_BYPASS);
	ISP_SetInterruptType(ISP_INT_SYNC_FAL, ISP_INT_SYNC_FAL);
	ISP_SetPreWindow(40, 640+100+20, 4, 480+4);
	ISP_SetAvWindow(132, 640, 8, 480);
	ISP_SetOperMode(ISP_EXT_BYP_YCBCR);
	/* Image Signal Processor setup end */	

#ifdef DS_DEBUG
	ds_printk("ISP_REG->CON_1 = 0x%x\n",ISP_REG->CON_1 );
	ds_printk("ISP_REG->CON_2 = 0x%x\n",ISP_REG->CON_2 );
	ds_printk("ISP_REG->HPERIOD = 0x%x\n",ISP_REG->HPERIOD );
	ds_printk("ISP_REG->HPW = 0x%x\n",ISP_REG->HPW );
	ds_printk("ISP_REG->VPERIOD = 0x%x\n",ISP_REG->VPERIOD );
	ds_printk("ISP_REG->VPW = 0x%x\n",ISP_REG->VPW );
	ds_printk("ISP_REG->SRC_HWIDTH = 0x%x\n",ISP_REG->SRC_HWIDTH );
	ds_printk("ISP_REG->DST_HWIDTH = 0x%x\n",ISP_REG->DST_HWIDTH );
	ds_printk("ISP_REG->RGBREC = 0x%x\n",ISP_REG->RGBREC );
	ds_printk("ISP_REG->CSUPSLANT = 0x%x\n",ISP_REG->CSUPSLANT );
	ds_printk("ISP_REG->CDLY = 0x%x\n",ISP_REG->CDLY );
	ds_printk("ISP_REG->INT_CON = 0x%x\n",ISP_REG->INT_CON );
	ds_printk("ISP_REG->RGBSV= 0x%x\n",ISP_REG->RGBSV);
	ds_printk("ISP_REG->RGBEV= 0x%x\n",ISP_REG->RGBEV);
	ds_printk("ISP_REG->RGBSH= 0x%x\n",ISP_REG->RGBSH);
	ds_printk("ISP_REG->RGBEH= 0x%x\n",ISP_REG->RGBEH);
	ds_printk("ISP_REG->VACT_ST= 0x%x\n",ISP_REG->VACT_ST);
	ds_printk("ISP_REG->V_HEIGHT= 0x%x\n",ISP_REG->V_HEIGHT);
	ds_printk("ISP_REG->HACT_ST= 0x%x\n",ISP_REG->HACT_ST);
	ds_printk("ISP_REG->H_WIDTH= 0x%x\n",ISP_REG->H_WIDTH);
#endif
	
	misc_register(&isp_miscdev);
	printk("MMSP2 Image Processer driver\n");
	return 0;
}

static void __exit isp_exit(void)
{
	misc_deregister(&isp_miscdev);
}

module_init(isp_init);
module_exit(isp_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
MODULE_DESCRIPTION("MMSP2 Image Signal Processer driver");
