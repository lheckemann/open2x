/*
 *  linux/kernel/drivers/char/vpp.c
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
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

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include "vpp.h"

static int vpp_count=0;

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
	unsigned short tmp1, tmp2;
	unsigned short SetSrcWidth, SetDestWidth;
	unsigned long vScale, hScale;
	FDC_INFO fdc_info;
	SC_INFO sc_info;
	YUV_INFO yuv_info;
	WINDOW_INFO win_info;
	int i, ret = 0;

	switch(cmd) {
	case IOCTL_MMSP2_SET_FDC:
		if(copy_from_user(&fdc_info, (FDC_INFO *)arg, sizeof(FDC_INFO)))
			return -EFAULT;
		if(fdc_info.SrcField==1)
			FDC_CNTL |= 0x0100;
		else
			FDC_CNTL &= (~0x0100);
		FDC_LUMA_OFFSET = fdc_info.Y_Offset;
		FDC_CB_OFFSET = fdc_info.CB_Offset;
		FDC_CR_OFFSET = fdc_info.CR_Offset;
		if(fdc_info.ToScaler==1) {
			FDC_CNTL |= 0x0800;
			if(fdc_info.DstField==1)
				FDC_CNTL |= 0x0080;
			else
				FDC_CNTL &= (~0x0080);
		}
		else {
			FDC_CNTL &= (~0x0800);
			if(fdc_info.DstField==1)
				FDC_CNTL |= 0x0080;
			else
				FDC_CNTL &= (~0x0080);
			FDC_DST_BASE_L	= (unsigned short)(fdc_info.DstBaseAddr & 0xFFFF);	
			FDC_DST_BASE_H	= (unsigned short)((fdc_info.DstBaseAddr >> 16) & 0xFFFF);	
		}
		FDC_CNTL = (FDC_CNTL & ~(0x03<<3)) | (fdc_info.Rotate<<3);
		tmp1 = (fdc_info.Width >> 4) - 1;
		tmp2= (fdc_info.Height >> 4) - 1;
		FDC_FRAME_SIZE = (tmp2 << 8) | tmp1;
		FDC_CNTL = (FDC_CNTL & ~(0x03<<1)) | (fdc_info.Chroma<<1);
		if(fdc_info.VSync==1)
			FDC_CNTL |= 0x40;
		else
			FDC_CNTL &= (~0x40);
		break;
	case IOCTL_MMSP2_START_FDC:
		FDC_CNTL = (FDC_CNTL & 0xEFFF) | 1;
		FDC_CNTL &= 0xFFFE;
		break;
	case IOCTL_MMSP2_FDC_BUSY:
		if(FDC_CNTL & 0x01) {
			ret = 1;
		}
		break;
	case IOCTL_MMSP2_STOP_FDC://This function has some bugs on HW. Don't use it!!
		FDC_CNTL &= (~0x40);
		FDC_CNTL |= 0x8000;
		while(FDC_CNTL & 0x8000) {
			for(i=0; i<0x1000; i++);
			FDC_CNTL &= (~0x8000);
		}
		FDC_CNTL = 0;
		break;

	case IOCTL_MMSP2_SET_SC:
		if(copy_from_user(&sc_info, (SC_INFO *)arg, sizeof(SC_INFO)))
			return -EFAULT;

		SC_DELAY = (0x03 << 4) | 0x01;
		SC_MEM_CNTR	= (SC_MEM_CNTR & (~0x7F1F)) | (0x00<<8) | 0x10;    
		SC_STATUS = (SC_STATUS & ~(0x1<<9)) | (0x0<<9);
		SC_STATUS = (SC_STATUS & ~(0x3<<11)) | (0x2<<11);//ISP_VSync???

		tmp1 = 0;
		if(sc_info.Src==SC_FRM_MEM)
			tmp1 = (tmp1 & ~(0x7<<10)) | (0x0<<10);
		else if(sc_info.Src==SC_FRM_MLC)
			tmp1 = (tmp1 & ~(0x7<<10)) | (0x1<<10);
		else if(sc_info.Src==SC_FRM_ISP)
			tmp1 = (tmp1 & ~(0x7<<10)) | (0x2<<10);
		else
			tmp1 = (tmp1 & ~(0x7<<10)) | (0x4<<10);

		if(sc_info.Dst==SC_TO_MEM)
			tmp1 = (tmp1 & ~(0x3<<1)) | (0x2<<1);
		else
			tmp1 = (tmp1 & ~(0x3<<1)) | (0x1<<1);
		SC_CMD = (SC_CMD & (~((0x7<<10) | (0x3<<1)))) | tmp1;

		if(sc_info.NumOfFrame==SC_ONE_TIME)
			tmp1 = 0x1 << 8;
		else
			tmp1 = 0x0 << 8;
		if(sc_info.WriteForm==SC_YCBYCR)
			tmp1 = (tmp1 & ~(0x1<<0)) | (0x0<<0);
		else
			tmp1 = (tmp1 & ~(0x1<<0)) | (0x1<<0);
		if(sc_info.WriteForm==SC_SEP_F422)
			tmp1 = (tmp1 & ~(0x1<<3)) | (0x1<<3);
		else
			tmp1 = (tmp1 & ~(0x1<<3)) | (0x0<<3);
		SC_CMD = (SC_CMD & (~((0x1<<8) | (0x1<<3) | (0x1<<0)))) | tmp1;
				
		if(sc_info.WriteField==SC_WR_FIELD)
			SC_STATUS = (SC_STATUS & ~(0x1<<10)) | (0x1<<10);
		else
			SC_STATUS = (SC_STATUS & ~(0x1<<10)) | (0x0<<10);

		if(sc_info.WriteForm!=SC_YCBYCR) {
			tmp1 = (sc_info.DstWidth + 15) & 0xFFF0;
			tmp2 = (sc_info.DstHeight + 1) & 0xFFFE;
		}
		else {
			tmp1 = sc_info.DstWidth;
			tmp2 = sc_info.DstHeight;
		}

		if(sc_info.SrcWidth & 0x1)
			SetSrcWidth	= sc_info.SrcWidth - 1;
		else
			SetSrcWidth	= sc_info.SrcWidth;
		if(sc_info.DstWidth & 0x1)
			SetDestWidth = tmp1 - 1;
		else
			SetDestWidth = tmp1;
		SC_SRC_PXL_HEIGHT = sc_info.SrcHeight;
		SC_DST_PXL_WIDTH = SetDestWidth - 1;
		SC_DST_PXL_HEIGHT = tmp2;//- 1;

		if((sc_info.SrcWidth==tmp1) && (sc_info.SrcHeight==tmp2)) {
			SC_CMD = (SC_CMD & ~(0x1<<7)) | (0x0<<7);
			SC_CMD = (SC_CMD & ~(0x1<<6)) | (0x0<<6);
			SC_SRC_PXL_REQCNT = SetSrcWidth - 1;
			SC_PRE_HRATIO = 0x1000;
			hScale = 0x4000;
		}
		else if(sc_info.SrcWidth > tmp1) {
			if(sc_info.SrcWidth > 1024) {
				SC_CMD = (SC_CMD & ~(0x1<<7)) | (0x1<<7);
				SC_CMD = (SC_CMD & ~(0x1<<6)) | (0x1<<6);
				SC_PRE_HRATIO = 0x0800;

				if((sc_info.SrcWidth>>1) > tmp1) {
					SC_CMD = (SC_CMD & ~(0x1<<4)) | (0x0<<4);
					SC_SRC_PXL_REQCNT = (sc_info.SrcWidth>>1) - 1;
					hScale = (0x4000 * (sc_info.SrcWidth>>1)) / tmp1;
				}
				else {
					SC_CMD = (SC_CMD & ~(0x1<<4)) | (0x1<<4);
					SC_SRC_PXL_REQCNT = (sc_info.SrcWidth>>1) - 1;
					hScale = (0x4000 * (sc_info.SrcWidth>>1)) / tmp1;
				}
			}
			else {
				SC_CMD = (SC_CMD & ~(0x1<<7)) | (0x0<<7);
				SC_CMD = (SC_CMD & ~(0x1<<6)) | (0x1<<6);
				SC_CMD = (SC_CMD & ~(0x1<<4)) | (0x0<<4);
				SC_SRC_PXL_REQCNT = SetSrcWidth - 1;
				SC_PRE_HRATIO = 0x400;
				hScale = (0x4000 * sc_info.SrcWidth) / tmp1;
			}
		}
		else {
			SC_CMD = (SC_CMD & ~(0x1<<7)) | (0x0<<7);
			SC_CMD = (SC_CMD & ~(0x1<<6)) | (0x1<<6);
			SC_CMD = (SC_CMD & ~(0x1<<4)) | (0x1<<4);
			SC_SRC_PXL_REQCNT = SetSrcWidth - 1;
			SC_PRE_HRATIO = 0x400;
			hScale = (0x4000 * sc_info.SrcWidth) / tmp1;
		}
		SC_POST_HRATIOL	= (unsigned short)(hScale & 0xFFFF);
		SC_POST_HRATIOH	= (unsigned short)(hScale >> 16);
	
		if((sc_info.SrcWidth==tmp1) && (sc_info.SrcHeight==tmp2)) {
		}
		else if(sc_info.SrcHeight > tmp2) {
			SC_CMD = (SC_CMD & ~(0x1<<5)) | (0x0<<5);
		}
		else {
			SC_CMD = (SC_CMD & ~(0x1<<5)) | (0x1<<5);
		}
		vScale = (0x4000 * sc_info.SrcHeight) / (tmp2 + 1);

		SC_PRE_VRATIO = 0x400;
		SC_POST_VRATIOL	= (unsigned short)(vScale & 0xFFFF);
		SC_POST_VRATIOH	= (unsigned short)(vScale >> 16);

		if(sc_info.hSrcMirror==SC_MIRROR_OFF)
			SC_MIRROR = (SC_MIRROR & ~(0x1<<0)) | (0x0<<0);
		else
			SC_MIRROR = (SC_MIRROR & ~(0x1<<0)) | (0x1<<0);
		if(sc_info.vSrcMirror==SC_MIRROR_OFF)
			SC_MIRROR = (SC_MIRROR & ~(0x1<<1)) | (0x0<<1);
		else
			SC_MIRROR = (SC_MIRROR & ~(0x1<<1)) | (0x1<<1);

		if(sc_info.hDstMirror==SC_MIRROR_OFF)
			SC_MIRROR = (SC_MIRROR & ~(0x1<<2)) | (0x0<<2);
		else
			SC_MIRROR = (SC_MIRROR & ~(0x1<<2)) | (0x1<<2);
		if(sc_info.vDstMirror==SC_MIRROR_OFF)
			SC_MIRROR = (SC_MIRROR & ~(0x1<<3)) | (0x0<<3);
		else
			SC_MIRROR = (SC_MIRROR & ~(0x1<<3)) | (0x1<<3);

		SC_SRC_EVEN_ADDRL = (unsigned short)(sc_info.SrcEvenAddr & 0xFFFF);
		SC_SRC_EVEN_ADDRH = (unsigned short)(sc_info.SrcEvenAddr >> 16);
		SC_SRC_ODD_ADDRL = (unsigned short)(sc_info.SrcOddAddr & 0xFFFF);
		SC_SRC_ODD_ADDRH = (unsigned short)(sc_info.SrcOddAddr >> 16);
		SC_SRC_PXL_WIDTH = (unsigned short)((sc_info.SrcStride >> 1) - 1);

		SC_DST_ADDRL = (unsigned short)(sc_info.DstAddr & 0xFFFF);
		SC_DST_ADDRH = (unsigned short)(sc_info.DstAddr >> 16);
		SC_DST_WPXL_WIDTH = (unsigned short)(sc_info.DstStride >> 1);

		SC_LUMA_OFFSET = sc_info.Y_Offset;
		SC_CB_OFFSET = sc_info.CB_Offset;
		SC_CR_OFFSET = sc_info.CR_Offset;
		break;
	case IOCTL_MMSP2_ENABLE_SC:
		if(arg==SC_ENABLE) {
			SC_CMD = (SC_CMD & ~(0x1<<13)) | (0x1<<13);
		}
		else {
			if(((SC_CMD & (0x7<<10)) == (0x0<<10) || (SC_CMD & (0x3<<1)) == (0x2<<1)) && 
				(SC_CMD & (0x1<<13))) {
				SC_CMD = (SC_CMD & ~(0x1<<8)) | (0x1<<8);
				SC_IRQ = (SC_IRQ & ~(0x1<<3)) | (0x0<<3);
			}
			else {
				SC_CMD = (SC_CMD & ~(0x1<<13)) | (0x0<<13);
			}
		}
		break;
	case IOCTL_MMSP2_START_SC:
		SC_CMD = (SC_CMD & ~(0x1<<14)) | (0x1<<14);
		break;
	case IOCTL_MMSP2_STOP_SC:
		if(((SC_CMD & (0x7<<10)) == (0x0<<10) || (SC_CMD & (0x3<<1)) == (0x2<<1)) && 
			(SC_CMD & (0x1<<13))) {
			SC_CMD = (SC_CMD & ~(0x1<<8)) | (0x1<<8);
			SC_IRQ = (SC_IRQ & ~(0x1<<3)) | (0x0<<3);
		}
		else {
			SC_CMD = (SC_CMD & ~(0x1<<13)) | (0x0<<13);
		}
		break;
	case IOCTL_MMSP2_SC_BUSY:
		if(SC_STATUS & (0x1<<3))
			ret = 1;
		break;

	case IOCTL_MMSP2_SET_YUVA:
		if(copy_from_user(&yuv_info, (YUV_INFO *)arg, sizeof(YUV_INFO)))
			return -EFAULT;

		if(yuv_info.EndY==yuv_info.BottomY)
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<4)) | (0x0<<4);
		else
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<4)) | (0x1<<4);
		if(yuv_info.Src==0)
			MLC_YUV_CNTL = (MLC_YUV_CNTL & ~(0x1<<12)) | (0x0<<12);
		else
			MLC_YUV_CNTL = (MLC_YUV_CNTL & ~(0x1<<12)) | (0x1<<12);
		if(yuv_info.Priority==MLC_YUV_PRIO_A)
			MLC_YUV_CNTL = (MLC_YUV_CNTL & ~(0x1<<10)) | (0x0<<10);
		else
			MLC_YUV_CNTL = (MLC_YUV_CNTL & ~(0x1<<10)) | (0x1<<10);

		if(yuv_info.hMirrorTop==MLC_YUV_MIRROR_OFF)
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<3)) | (0x0<<3);
		else
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<3)) | (0x1<<3);
		if(yuv_info.vMirrorTop==MLC_YUV_MIRROR_OFF)
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<2)) | (0x0<<2);
		else
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<2)) | (0x1<<2);

		if(yuv_info.hMirrorBot==MLC_YUV_MIRROR_OFF)
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<1)) | (0x0<<1);
		else
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<1)) | (0x1<<1);
		if(yuv_info.vMirrorBot==MLC_YUV_MIRROR_OFF)
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<0)) | (0x0<<0);
		else
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<0)) | (0x1<<0);

		MLC_YUVA_TP_OADRL = (unsigned short)(yuv_info.TopAddr & 0xFFFF);
		MLC_YUVA_TP_OADRH = (unsigned short)(yuv_info.TopAddr >> 16);
		MLC_YUVA_TP_EADRL = (unsigned short)(yuv_info.TopAddr & 0xFFFF);
		MLC_YUVA_TP_EADRH = (unsigned short)(yuv_info.TopAddr >> 16);

		MLC_YUVA_BT_OADRL = (unsigned short)(yuv_info.BotAddr & 0xFFFF);
		MLC_YUVA_BT_OADRH = (unsigned short)(yuv_info.BotAddr >> 16);
		MLC_YUVA_BT_EADRL = (unsigned short)(yuv_info.BotAddr & 0xFFFF);
		MLC_YUVA_BT_EADRH = (unsigned short)(yuv_info.BotAddr >> 16);

		hScale = ((unsigned long)yuv_info.SrcWidthTop * 1024) / yuv_info.DstWidthTop;
		vScale = ((unsigned long)yuv_info.SrcHeightTop * yuv_info.StrideTop >> 1) / yuv_info.DstHeightTop;
		MLC_YUVA_TP_PXW = yuv_info.StrideTop >> 1;
		MLC_YUVA_TP_HSC = (unsigned short)(hScale & 0xFFFF);
		MLC_YUVA_TP_VSCL = (unsigned short)(vScale & 0xFFFF);
		MLC_YUVA_TP_VSCH = (unsigned short)(vScale >> 16);

		hScale = ((unsigned long)yuv_info.SrcWidthBot * 1024) / yuv_info.DstWidthBot;
		vScale = ((unsigned long)yuv_info.SrcHeightBot * yuv_info.StrideBot >> 1) / yuv_info.DstHeightBot;
		MLC_YUVA_BT_PXW = yuv_info.StrideBot >> 1;
		MLC_YUVA_BT_HSC = (unsigned short)(hScale & 0xFFFF);
		MLC_YUVA_BT_VSCL = (unsigned short)(vScale & 0xFFFF);
		MLC_YUVA_BT_VSCH = (unsigned short)(vScale >> 16);

		MLC_YUVA_STX = yuv_info.StartX;
		MLC_YUVA_ENDX = yuv_info.EndX - 1;
		MLC_YUVA_TP_STY = yuv_info.StartY;
		MLC_YUVA_TP_ENDY = yuv_info.EndY - 1;
		MLC_YUVA_BT_ENDY = yuv_info.BottomY - 1;
		break;
	case IOCTL_MMSP2_SET_YUVB:
		if(copy_from_user(&yuv_info, (YUV_INFO *)arg, sizeof(YUV_INFO)))
			return -EFAULT;

		if(yuv_info.EndY==yuv_info.BottomY)
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<9)) | (0x0<<9);
		else
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<9)) | (0x1<<9);
		if(yuv_info.Src==0)
			MLC_YUV_CNTL = (MLC_YUV_CNTL & ~(0x1<<11)) | (0x0<<11);
		else
			MLC_YUV_CNTL = (MLC_YUV_CNTL & ~(0x1<<11)) | (0x1<<11);
		if(yuv_info.Priority==MLC_YUV_PRIO_A)
			MLC_YUV_CNTL = (MLC_YUV_CNTL & ~(0x1<<10)) | (0x0<<10);
		else
			MLC_YUV_CNTL = (MLC_YUV_CNTL & ~(0x1<<10)) | (0x1<<10);

		if(yuv_info.hMirrorTop==MLC_YUV_MIRROR_OFF)
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<8)) | (0x0<<8);
		else
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<8)) | (0x1<<8);
		if(yuv_info.vMirrorTop==MLC_YUV_MIRROR_OFF)
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<7)) | (0x0<<7);
		else
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<7)) | (0x1<<7);

		if(yuv_info.hMirrorBot==MLC_YUV_MIRROR_OFF)
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<6)) | (0x0<<6);
		else
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<6)) | (0x1<<6);
		if(yuv_info.vMirrorBot==MLC_YUV_MIRROR_OFF)
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<5)) | (0x0<<5);
		else
			MLC_YUV_EFFECT = (MLC_YUV_EFFECT & ~(0x1<<5)) | (0x1<<5);

		MLC_YUVB_TP_OADRL = (unsigned short)(yuv_info.TopAddr & 0xFFFF);
		MLC_YUVB_TP_OADRH = (unsigned short)(yuv_info.TopAddr >> 16);
		MLC_YUVB_TP_EADRL = (unsigned short)(yuv_info.TopAddr & 0xFFFF);
		MLC_YUVB_TP_EADRH = (unsigned short)(yuv_info.TopAddr >> 16);

		MLC_YUVB_BT_OADRL = (unsigned short)(yuv_info.BotAddr & 0xFFFF);
		MLC_YUVB_BT_OADRH = (unsigned short)(yuv_info.BotAddr >> 16);
		MLC_YUVB_BT_EADRL = (unsigned short)(yuv_info.BotAddr & 0xFFFF);
		MLC_YUVB_BT_EADRH = (unsigned short)(yuv_info.BotAddr >> 16);

		if(yuv_info.Src==1) {
			hScale = ((unsigned long)yuv_info.SrcWidthTop * 1024) / yuv_info.DstWidthTop;
			vScale = ((unsigned long)yuv_info.SrcHeightTop * 512) / yuv_info.DstHeightTop; 
			MLC_YUVB_TP_PXW = yuv_info.SrcWidthTop;
			MLC_YUVB_TP_HSC = (unsigned short)(hScale & 0xFFFF);
			MLC_YUVB_TP_VSCL = (unsigned short)(vScale & 0xFFFF);
			MLC_YUVB_TP_VSCH = (unsigned short)(vScale >> 16);
		}
		else {
			hScale = ((unsigned long)yuv_info.SrcWidthTop * 1024) / yuv_info.DstWidthTop;
			vScale = ((unsigned long)yuv_info.SrcHeightTop * yuv_info.StrideTop >> 1) / yuv_info.DstHeightTop;
			MLC_YUVB_TP_PXW = yuv_info.StrideTop >> 1;
			MLC_YUVB_TP_HSC = (unsigned short)(hScale & 0xFFFF);
			MLC_YUVB_TP_VSCL = (unsigned short)(vScale & 0xFFFF);
			MLC_YUVB_TP_VSCH = (unsigned short)(vScale >> 16);
		}

		hScale = ((unsigned long)yuv_info.SrcWidthBot * 1024) / yuv_info.DstWidthBot;
		vScale = ((unsigned long)yuv_info.SrcHeightBot * yuv_info.StrideBot >> 1) / yuv_info.DstHeightBot;
		MLC_YUVB_BT_PXW = yuv_info.StrideBot >> 1;
		MLC_YUVB_BT_HSC = (unsigned short)(hScale & 0xFFFF);
		MLC_YUVB_BT_VSCL = (unsigned short)(vScale & 0xFFFF);
		MLC_YUVB_BT_VSCH = (unsigned short)(vScale >> 16);

		MLC_YUVB_STX = yuv_info.StartX;
		MLC_YUVB_ENDX = yuv_info.EndX - 1;
		MLC_YUVB_TP_STY = yuv_info.StartY;
		MLC_YUVB_TP_ENDY = yuv_info.EndY - 1;
		MLC_YUVB_BT_ENDY = yuv_info.BottomY - 1;
		break;
	case IOCTL_MMSP2_YUVA_ON:
		MLC_OVLAY_CNTR = (MLC_OVLAY_CNTR & ~(0x1<<0)) | (0x1<<0);
		break;
	case IOCTL_MMSP2_YUVA_OFF:
		MLC_OVLAY_CNTR = (MLC_OVLAY_CNTR & ~(0x1<<0)) | (0x0<<0);
		break;
	case IOCTL_MMSP2_YUVB_ON:
		MLC_OVLAY_CNTR = (MLC_OVLAY_CNTR & ~(0x1<<1)) | (0x1<<1);
		break;
	case IOCTL_MMSP2_YUVB_OFF:
		MLC_OVLAY_CNTR = (MLC_OVLAY_CNTR & ~(0x1<<1)) | (0x0<<1);
		break;
	case IOCTL_MMSP2_RGB_ON:
		if(arg>=0 && arg <=4) {
			MLC_OVLAY_CNTR = (MLC_OVLAY_CNTR & ~(0x1<<(arg+2))) | (0x1<<(arg+2));
		}
		break;
	case IOCTL_MMSP2_RGB_OFF:
		if(arg>=0 && arg<=4) {
			MLC_OVLAY_CNTR = (MLC_OVLAY_CNTR & ~(0x1<<(arg+2))) | (0x0<<(arg+2));
		}
		break;

	case IOCTL_MMSP2_CALC_WINDOW:
		if(copy_from_user(&win_info, (WINDOW_INFO *)arg, sizeof(WINDOW_INFO)))
			return -EFAULT;

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

		if(copy_to_user((WINDOW_INFO *)arg, (void *)&win_info, sizeof(WINDOW_INFO)))
			return -EFAULT;
		break;

	case IOCTL_MMSP2_JIFFIES:
		if(copy_to_user((unsigned long *)arg, (void *)&jiffies, sizeof(unsigned long)))
			return -EFAULT;
		break;

	default:
		ret = -EINVAL;
	}
	return ret;
}

static struct file_operations vpp_fops = {
	owner:		THIS_MODULE,
	ioctl:		vpp_ioctl,
	open:		vpp_open,
	release:	vpp_release,
};

static struct miscdevice vpp_miscdev = {
	253, "vpp", &vpp_fops
};

static int __init vpp_init(void)
{
	misc_register(&vpp_miscdev);
	return 0;
}

static void __exit vpp_exit(void)
{
	misc_deregister(&vpp_miscdev);
}

module_init(vpp_init);
module_exit(vpp_exit);
