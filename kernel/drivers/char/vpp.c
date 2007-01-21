/*
 *  linux/kernel/drivers/char/vpp.c
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

#include <asm/arch/proto_fdc.h>
#include <asm/arch/proto_scale.h>
#include <asm/arch/proto_mlc.h>
#include <asm/arch/proto_dpc.h>
#include <asm/arch/mmsp2-regs.h>
#include "vpp.h"

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(x...) printk(x)
#else
#  define ds_printk(fmt, args...)
#endif

#ifdef DS_DEBUG
void print_fdc(FDC_INFO *fi)
{
    ds_printk("FDC_INFO ===============================\n");	
    ds_printk("VSync       = %d    \n", fi->VSync       );
    ds_printk("SrcField    = %d    \n", fi->SrcField    );
    ds_printk("Chroma      = %d    \n", fi->Chroma      );
    ds_printk("Rotate      = %d    \n", fi->Rotate      );
    ds_printk("DstField    = %d    \n", fi->DstField    );
    ds_printk("ToScaler    = %d    \n", fi->ToScaler    );
    ds_printk("Width       = %d    \n", fi->Width       );
    ds_printk("Height      = %d    \n", fi->Height      );
    ds_printk("Y_Offset    = 0x%04x  \n", fi->Y_Offset    );
    ds_printk("CB_Offset   = 0x%04x  \n", fi->CB_Offset   );
    ds_printk("CR_Offset   = 0x%04x  \n", fi->CR_Offset   );
    ds_printk("DstBaseAddr = 0x%08x\n", fi->DstBaseAddr );	
    ds_printk("========================================\n");	    
}

void print_sc(SC_INFO *si)
{
    ds_printk("SC_INFO ===============================\n");	
    ds_printk("Src         = %d\n", si->Src         );
    ds_printk("Dst         = %d\n", si->Dst         );
    ds_printk("NumOfFrame  = %d\n", si->NumOfFrame  );
    ds_printk("WriteForm   = %d\n", si->WriteForm   );
    ds_printk("WriteField  = %d\n", si->WriteField  );
    ds_printk("hSrcMirror  = %d\n", si->hSrcMirror  );
    ds_printk("vSrcMirror  = %d\n", si->vSrcMirror  );
    ds_printk("hDstMirror  = %d\n", si->hDstMirror  );
    ds_printk("vDstMirror  = %d\n", si->vDstMirror  );
    ds_printk("SrcWidth    = %d\n", si->SrcWidth    );
    ds_printk("SrcHeight   = %d\n", si->SrcHeight   ); 
    ds_printk("SrcStride   = %d\n", si->SrcStride   );
    ds_printk("DstWidth    = %d\n", si->DstWidth    );
    ds_printk("DstHeight   = %d\n", si->DstHeight   );
    ds_printk("DstStride   = %d\n", si->DstStride   );
    ds_printk("SrcOddAddr  = 0x%08x\n", si->SrcOddAddr  );
    ds_printk("SrcEvenAddr = 0x%08x\n", si->SrcEvenAddr );
    ds_printk("DstAddr     = 0x%08x\n", si->DstAddr     );
    ds_printk("Y_Offset    = 0x%04x\n", si->Y_Offset    );
    ds_printk("CB_Offset   = 0x%04x\n", si->CB_Offset   );
    ds_printk("CR_Offset   = 0x%04x\n", si->CR_Offset   );
    ds_printk("=======================================\n");	    
}

void print_yuv(YUV_INFO *yi)
{
    ds_printk("YUV_INFO ==============================\n");	
    ds_printk("Src          = %d\n",  yi->Src         );    
    ds_printk("Priority     = %d\n",  yi->Priority    );
    ds_printk("vMirrorTop   = %d\n",  yi->vMirrorTop  );
    ds_printk("hMirrorTop   = %d\n",  yi->hMirrorTop  ); 
    ds_printk("vMirrorBot   = %d\n",  yi->vMirrorBot  ); 
    ds_printk("hMirrorBot   = %d\n",  yi->hMirrorBot  ); 
    ds_printk("SrcWidthTop  = %d\n",  yi->SrcWidthTop ); 
    ds_printk("SrcHeightTop = %d\n",  yi->SrcHeightTop); 
    ds_printk("DstWidthTop  = %d\n",  yi->DstWidthTop ); 
    ds_printk("DstHeightTop = %d\n",  yi->DstHeightTop); 
    ds_printk("StrideTop    = %d\n",  yi->StrideTop   ); 
    ds_printk("SrcWidthBot  = %d\n",  yi->SrcWidthBot ); 
    ds_printk("SrcHeightBot = %d\n",  yi->SrcHeightBot); 
    ds_printk("DstWidthBot  = %d\n",  yi->DstWidthBot ); 
    ds_printk("DstHeightBot = %d\n",  yi->DstHeightBot); 
    ds_printk("StrideBot    = %d\n",  yi->StrideBot   ); 
    ds_printk("StartX       = %d\n",  yi->StartX      ); 
    ds_printk("StartY       = %d\n",  yi->StartY      ); 
    ds_printk("EndX         = %d\n",  yi->EndX        ); 
    ds_printk("EndY         = %d\n",  yi->EndY        ); 
    ds_printk("BottomY      = %d\n",  yi->BottomY     ); 
    ds_printk("TopAddr      = 0x%08x\n",  yi->TopAddr     ); 
    ds_printk("BotAddr      = 0x%08x\n",  yi->BotAddr     ); 
    ds_printk("=======================================\n");	        
}

void print_win(WINDOW_INFO *wi)
{
    ds_printk("WIN_INFO ==============================\n");	
    ds_printk("InputWidth    = %d\n", wi->InputWidth    );
    ds_printk("InputHeight   = %d\n", wi->InputHeight   );
    ds_printk("WindowWidth   = %d\n", wi->WindowWidth   );
    ds_printk("WindowHeight  = %d\n", wi->WindowHeight  );
    ds_printk("DisplayWidth  = %d\n", wi->DisplayWidth  );
    ds_printk("DisplayHeight = %d\n", wi->DisplayHeight );
    ds_printk("=======================================\n");	            
}
#endif

static int vpp_count=0;

extern int scaler_writed;

static int vpp_open(struct inode *inode, struct file *file)
{
    if(vpp_count==0) {
        vpp_count = 1;
        return 0;
    }
    else
        return -EBUSY;
}

static int vpp_release(struct inode *inode, struct file *file)
{
    vpp_count = 0;
    return 0;
}

static int vpp_ioctl(struct inode *inode, struct file *file,
                     unsigned int cmd, unsigned long arg)
{
    unsigned long vScale, hScale;
    FDC_INFO fdc_info;
    SC_INFO sc_info;
    YUV_INFO yuv_info;
    WINDOW_INFO win_info;
    int i, ret = 0;
    FDC_RUN_INFO fri;



    switch(cmd) {
    case IOCTL_MMSP2_SET_FDC:
        if(copy_from_user(&fdc_info, (FDC_INFO *)arg, sizeof(FDC_INFO)))
            return -EFAULT;
        
        ds_printk("IOCTL_MMSP2_SET_FDC\n");
		#ifdef DS_DEBUG
        print_fdc( &fdc_info );
		#endif
        
        FDC_Util
        (
                fdc_info.VSync, 
                fdc_info.SrcField, 
                fdc_info.Chroma, 
                fdc_info.Rotate, 
                fdc_info.DstField, 
                fdc_info.ToScaler, 
                fdc_info.Width, fdc_info.Height, 
                fdc_info.Y_Offset, fdc_info.CB_Offset, fdc_info.CR_Offset,  
                fdc_info.DstBaseAddr
        );
        break;
        
    case IOCTL_MMSP2_START_FDC:
        ds_printk("IOCTL_MMSP2_START_FDC\n"); 
        if(copy_from_user(&fri, (FDC_RUN_INFO *)arg, sizeof(FDC_RUN_INFO)))
            return -EFAULT;
		// for gplayer speed up..
        DFDC_LUMA_OFFSET = fri.Y_Offset; 
        DFDC_CB_OFFSET = fri.CB_Offset;
        DFDC_CR_OFFSET = fri.CR_Offset;
        
        DFDC_CNTL = (DFDC_CNTL & 0xEFFF) | 1;
        DFDC_CNTL &= 0xFFFE;
        break;
        
    case IOCTL_MMSP2_FDC_BUSY:
        ds_printk("IOCTL_MMSP2_FDC_BUSY\n");
        if(FDC_IsBusy()) {
            ret = 1;
        }
		ds_printk("DFDC_CNTL = 0x%x\n",DFDC_CNTL);
		ds_printk("DFDC_STATUS = 0x%x\n",DFDC_STATUS);
        break;
        
    case IOCTL_MMSP2_STOP_FDC://This function has some bugs on HW. Don't use it!!
        ds_printk("IOCTL_MMSP2_STOP_FDC\n"); 
        FDC_Stop();
        break;

    case IOCTL_MMSP2_SET_SC:
        if(copy_from_user(&sc_info, (SC_INFO *)arg, sizeof(SC_INFO)))
            return -EFAULT;

        ds_printk("IOCTL_MMSP2_SET_SC\n");
		#ifdef DS_DEBUG
        print_sc( &sc_info ); 
		#endif

        SC_Util(
                sc_info.Src, 
                sc_info.Dst, 
                sc_info.NumOfFrame, 
                sc_info.WriteForm, 
                sc_info.WriteField, 
                sc_info.hSrcMirror, sc_info.vSrcMirror, sc_info.hDstMirror, sc_info.vDstMirror,
                sc_info.SrcWidth, sc_info.SrcHeight, sc_info.SrcStride,
                sc_info.DstWidth, sc_info.DstHeight, sc_info.DstStride,
                sc_info.SrcOddAddr, sc_info.SrcEvenAddr,
                sc_info.DstAddr,
                sc_info.Y_Offset, sc_info.CB_Offset, sc_info.CR_Offset
        );

        break;
        
    case IOCTL_MMSP2_ENABLE_SC:
        ds_printk("IOCTL_MMSP2_ENABLE_SC: %s\n", (arg==CTRUE)?"scaler enable":"scaler disable");  
        SC_EnableScale(arg);
        break;
        
    case IOCTL_MMSP2_START_SC:
        ds_printk("IOCTL_MMSP2_START_SC\n"); 
#if 0
        SC_StartScale();
#else
        SC_StartScale(arg);	// hhsong 051121 modify
		scaler_writed = 1;
#endif
        break;
        
    case IOCTL_MMSP2_STOP_SC:
        ds_printk("IOCTL_MMSP2_STOP_SC\n");
        SC_StopScale();
        break;
        
    case IOCTL_MMSP2_SC_BUSY:
        ds_printk("IOCTL_MMSP2_SC_BUSY\n");
        if(SC_IsBusy())
            ret = 1;
        break;

    case IOCTL_MMSP2_SET_YUVA:
        if(copy_from_user(&yuv_info, (YUV_INFO *)arg, sizeof(YUV_INFO)))
            return -EFAULT;

        ds_printk("IOCTL_MMSP2_SET_YUVA\n");
		#ifdef DS_DEBUG
        print_yuv( &yuv_info );
		#endif

        MLC_Util_YUVA(
                yuv_info.Src,
                yuv_info.Priority,
                yuv_info.vMirrorTop, yuv_info.hMirrorTop, yuv_info.vMirrorTop, yuv_info.hMirrorBot,
                yuv_info.SrcWidthTop, yuv_info.SrcHeightTop,
                yuv_info.DstWidthTop, yuv_info.DstHeightTop, yuv_info.StrideTop,
                yuv_info.SrcWidthBot, yuv_info.SrcHeightBot,
                yuv_info.DstWidthBot, yuv_info.DstHeightBot, yuv_info.StrideBot,
                yuv_info.StartX, yuv_info.StartY, yuv_info.EndX, yuv_info.EndY, yuv_info.BottomY,
                yuv_info.TopAddr, yuv_info.BotAddr
		);
        break;
        
    case IOCTL_MMSP2_SET_YUVB:
        if(copy_from_user(&yuv_info, (YUV_INFO *)arg, sizeof(YUV_INFO)))
            return -EFAULT;

        ds_printk("IOCTL_MMSP2_SET_YUVB\n");
		#ifdef DS_DEBUG
        print_yuv( &yuv_info );
		#endif
		
        MLC_Util_YUVB(
                yuv_info.Src,
                yuv_info.Priority,
                yuv_info.vMirrorTop, yuv_info.hMirrorTop, yuv_info.vMirrorBot, yuv_info.hMirrorBot,
                yuv_info.SrcWidthTop, yuv_info.SrcHeightTop,
                yuv_info.DstWidthTop, yuv_info.DstHeightTop, yuv_info.StrideTop,
                yuv_info.SrcWidthBot, yuv_info.SrcHeightBot,
                yuv_info.DstWidthBot, yuv_info.DstHeightBot, yuv_info.StrideBot,
                yuv_info.StartX, yuv_info.StartY, yuv_info.EndX, yuv_info.EndY, yuv_info.BottomY,
                yuv_info.TopAddr, yuv_info.BotAddr
				);
        break;
        
    case IOCTL_MMSP2_YUVA_ON:
        ds_printk("IOCTL_MMSP2_YUVA_ON\n"); 
        MLC_YUVAOn();
        break;
        
    case IOCTL_MMSP2_YUVA_OFF:
        ds_printk("IOCTL_MMSP2_YUVA_OFF\n");
        MLC_YUVAOff();
        break;
        
    case IOCTL_MMSP2_YUVB_ON:
        ds_printk("IOCTL_MMSP2_YUVB_ON\n"); 
		MLC_YUVBOn();
        break;
        
    case IOCTL_MMSP2_YUVB_OFF:
        ds_printk("IOCTL_MMSP2_YUVB_OFF\n");
        MLC_YUVBOff();
        break;
        
    case IOCTL_MMSP2_RGB_ON:
        ds_printk("IOCTL_MMSP2_RGB_ON: arg = %d\n", arg);
        MLC_RGBOn((U8)arg);         // arg = 0~4
        break;
		
    case IOCTL_MMSP2_RGB_OFF:
        ds_printk("IOCTL_MMSP2_RGB_OFF: arg = %d\n", arg);
        MLC_RGBOff((U8)arg);            // arg = 0~4
        break;

    case IOCTL_MMSP2_CALC_WINDOW:
        if(copy_from_user(&win_info, (WINDOW_INFO *)arg, sizeof(WINDOW_INFO)))
            return -EFAULT;
        
        ds_printk("IOCTL_MMSP2_SET_YUVB, pre\n");
		#ifdef DS_DEBUG
        print_win( &win_info );
		#endif
        i = (1 << 15);
        hScale = ((unsigned long)win_info.WindowWidth << 15)/win_info.InputWidth;
        vScale = ((unsigned long)win_info.WindowHeight << 15)/win_info.InputHeight;
        if((win_info.InputWidth<win_info.WindowWidth) && (win_info.InputHeight<win_info.WindowHeight))
            i = (hScale > vScale) ? vScale : hScale;
        else if((win_info.InputWidth<win_info.WindowWidth) && (win_info.InputHeight>win_info.WindowHeight))
            i = vScale;
        else if((win_info.InputWidth>win_info.WindowWidth) && (win_info.InputHeight<win_info.WindowHeight))
            i = hScale;
        else if((win_info.InputWidth>win_info.WindowWidth) && (win_info.InputHeight>win_info.WindowHeight))
            i = (hScale > vScale) ? vScale : hScale;
        else if((win_info.InputWidth==win_info.WindowWidth) || (win_info.InputHeight==win_info.WindowHeight))
            i = (hScale > vScale) ? vScale : hScale;
        win_info.DisplayWidth = (unsigned short)(((i*win_info.InputWidth) + (1<<14)) >> 15);
        win_info.DisplayWidth &= ~(0x01);
        win_info.DisplayHeight = (unsigned short)(((i*win_info.InputHeight) + (1<<14)) >> 15);
        win_info.DisplayHeight &= ~(0x01);
        
        ds_printk("IOCTL_MMSP2_SET_YUVB, aft\n");
		#ifdef DS_DEBUG
        print_win( &win_info );
		#endif
        if(copy_to_user((WINDOW_INFO *)arg, (void *)&win_info, sizeof(WINDOW_INFO)))
            return -EFAULT;
        break;

    case IOCTL_MMSP2_JIFFIES:
        ds_printk("IOCTL_MMSP2_JIFFIES: jiffies = %u\n", jiffies);
        if(copy_to_user((unsigned long *)arg, (void *)&jiffies, sizeof(unsigned long)))
            return -EFAULT;
        break;
// 051025 hhsong add
	case IOCTL_MMSP2_DPC_VSYNC_INTERRUPT_ENABLE:
		DPC_SetInterruptEnable(2,0,1); // IntEn, IntHSyncEdgePos , IntVSyncEdgePos
		// vsync second edge enable
		break;
	
	case IOCTL_MMSP2_DPC_VSYNC_INTERRUPT_DISABLE:
		DPC_SetInterruptEnable(0,0,0); // IntEn, IntHSyncEdgePos , IntVSyncEdgePos
		break;

    default:
        ret = -EINVAL;
    }
    return ret;
}

static struct file_operations vpp_fops = {
    owner:      THIS_MODULE,
    ioctl:      vpp_ioctl,
    open:       vpp_open,
    release:    vpp_release,
};

static struct miscdevice vpp_miscdev = {
    VPP_MINOR, "vpp", &vpp_fops
};

static int __init vpp_init(void)
{
	SC_Initialize();
	FDC_Initialize();
    misc_register(&vpp_miscdev);
#ifdef CONFIG_MACH_GP2X_DEBUG    
	printk("MMSP2 Video Post Processor Driver\n");
#endif
    return 0;
}

static void __exit vpp_exit(void)
{
    misc_deregister(&vpp_miscdev);
}

module_init(vpp_init);
module_exit(vpp_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
