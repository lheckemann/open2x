/*
 *  linux/arch/arm/mach-mmsp2/dma.c
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/errno.h>

#include <asm/irq.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/dma.h>

#include "dma.h"

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(s, arg...) printk("dma<%d>: "s, dma->device_id, ##arg)
#else
#  define ds_printk(x...)
#endif

#define DMA_INTTYPE_END			(1<<1)
#define DMA_INTTYPE_STOP		(1<<0)


static mmsp2_dma_t dma_chan[MAX_MMSP2_DMA_CHANNELS];

static dma_devinfo_t dma_devinfo[] = {
	[DMA_UART0_RX]		= { "UART0 RX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_UART0_TX]		= { "UART0 TX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_UART1_RX]		= { "UART1 RX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_UART1_TX]		= { "UART1 TX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_UART2_RX]		= { "UART2 RX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_UART2_TX]		= { "UART2 TX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_UART3_RX]		= { "UART3 RX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_UART3_TX]		= { "UART3 TX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_IRDA_RX]		= { "IRDA RX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_IRDA_TX]		= { "IRDA TX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_USB_EP1]		= { "USB EP1", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_USB_EP2]		= { "USB EP2", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_USB_EP3]		= { "USB EP3", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_USB_EP4]		= { "USB EP4", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_DECSS_RX]		= { "DECSS RX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_DECSS_TX]		= { "DECSS TX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_MMCSD_RX]		= { "MMCSD RX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_MMCSD_TX]		= { "MMCSD TX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_MSTICK]		= { "MSTICK", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },

	[DMA_AC97_LRPCM_OUT] = 
		{ "AC97 OUT",  DMA_TYPE_WRITE, 
		  ( REQ_ONCE_16BYTE | SRC_ADDRINCEN | SRC_MEMFMT_WORD | TRG_FLOW | TRG_MEMFMT_WORD | TRG_DW_HW ),
/* com1	*/	  (SRCADDR_INC | TRGADDR_FIX),
/* cons	*/	  (DMA_IRQ_DONE_EN),
		  ADDR_MEM,
		  ADDR_DEV_AC97 },

	[DMA_AC97_SLRPCM_OUT]	= { "AC97 OUT", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_AC97_CWPCM_OUT]	= { "AC97 OUT", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_AC97_MIC_IN]	= { "AC97 IN", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },  	// When use MIC_IN, used LRPCM_IN path..
																		
	[DMA_AC97_LRPCM_IN]	= 
		{ "AC97 LRPCM IN",  DMA_TYPE_READ,
		  ( REQ_ONCE_16BYTE | SRC_FLOW | SRC_MEMFMT_WORD | SRC_DW_HW | TRG_ADDRINCEN | TRG_MEMFMT_WORD ),
		  ( SRCADDR_FIX | TRGADDR_INC ),
		  (DMA_IRQ_DONE_EN),
		  ADDR_DEV_AC97,
		  ADDR_MEM },

	[DMA_TOUCH]		= { "TOUCH", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_I2S_RX]		= { "I2S RX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_I2S_TX]		= { "I2S TX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_SPDIF_RX]		= { "SPDIF RX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_SPDIF_TX]		= { "SPDIF TX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_SSP_RX]		= { "SSP RX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_SSP_TX]		= { "SSP_TX", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_MPEG]		= { "MPEG", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_SP]		= { "SP", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
	[DMA_HUFFMAN]		= { "HUFFMAN", 0, 0x0, 0x0, 0x0, 0x0, 0x0},
	[DMA_RVLD]		= { "RVLD", 0, 0x0, 0x0, 0x0, 0x0, 0x0 },
};


void DMA_SetInterruptEnb(mmsp2_dma_t *dma, u32 type)
{
	u16 temp;
	
	temp = dma->regs->CONS;
	if( type & DMA_INTTYPE_END ) 
	{
		// 0000 0010 0000 0000 = 0x0200
		temp |= 0x0200;
	}
	else					
	{
		temp &= (~(0x0200));
	}

	if( type & DMA_INTTYPE_STOP )		
	{
		// 0000 0001 0000 0000 = 0x0100
		temp |= 0x0100;
	}
	else
	{
		temp &= (~(0x0100));
	}
	dma->regs->CONS = temp;
}

void DMA_ClrInterruptPend(mmsp2_dma_t *dma)
{
	u16 temp;
	
	temp = dma->regs->CONS;
	temp &= (~(0x0003));
	dma->regs->CONS = temp;
}


static void dma_stop(mmsp2_dma_t *dma)
{
	dma->regs->CONS &= ~(1 << 10);
}

static void dma_start(mmsp2_dma_t *dma)
{
	dma->regs->CONS |= (1 << 10);
}

static inline void set_mem_addr(mmsp2_dma_t *dma, unsigned long addr)
{
	if (dma->devinfo->type == DMA_TYPE_WRITE) {	/* write, MEM -> DEV */
		dma->regs->SRCLADDR = (addr & 0x0000ffff);
		dma->regs->SRCHADDR = (addr >> 16);
		ds_printk("src addr = 0x%x%X\n", dma->regs->SRCHADDR, dma->regs->SRCLADDR);
	} else {		/* read, DEV -> MEM */
		dma->regs->TRGLADDR = (addr & 0x0000ffff);
		dma->regs->TRGHADDR = (addr >> 16);
		ds_printk("trg addr = 0x%x%X\n", dma->regs->TRGHADDR, dma->regs->TRGLADDR);
	}
}

static inline void set_tx_size(mmsp2_dma_t *dma, unsigned long size)
{
	dma->regs->COM2 = size-1;
}

static inline void process_dma(mmsp2_dma_t *dma)
{
	dma_buf_t *buf, *next_buf;

	buf = dma->head;

	ds_printk("process_dma 0, act: %d, q: %d\n", dma->active, dma->queue_count);
	if (buf && (!dma->active)) 
	{
        ds_printk("process_dma 1, act: %d, q: %d\n", dma->active, dma->queue_count);
		set_mem_addr(dma, buf->dma_start);
		set_tx_size(dma, buf->size);
		dma->curr = buf;

		dma_start(dma);
		next_buf = dma->head->next;
		dma->head = next_buf;
		if (!next_buf)
		{
			dma->tail = NULL;
		}
		dma->active = 1;
		dma->queue_count--;

        ds_printk("process_dma 3, act: %d, q: %d\n\n\n", dma->active, dma->queue_count);

		ds_printk("start dma_ptr=%#x size=%d\n", buf->dma_start, buf->size);
		ds_printk("P: number of buffers in queue: %ld\n", dma->queue_count);
	}
}



static void dma_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	mmsp2_dma_t *dma = (mmsp2_dma_t *)dev_id;
	dma_buf_t *buf = dma->curr;
	int i;

	// 0200 = 0000 0010 0000 0000 = ENDIRQEN
	ds_printk("irq: %d, dma->regs->CONS = 0x%04x\n", irq, dma->regs->CONS);
	ds_printk("%s\n", __FUNCTION__);
	if (dma->callback)
		dma->callback(buf->id, buf->size);
	kfree(buf);
	dma->active = 0;
	process_dma(dma); 
}

/*
 * DMA interface functions
 */

static int fill_devinfo(mmsp2_dma_t *dma, int dev_id)
{
	dma_devinfo_t *devinfo;

	if ((dev_id < 0) || (dev_id >= 64))  {
		printk(KERN_ERR "unknown device id (%d)\n", dev_id);
		return 1;
	}

	devinfo = &dma_devinfo[dev_id];

	if (!devinfo->name) {
		printk(KERN_WARNING "not supported device id (%d)\n", dev_id);
		return 1;
	}

	dma->devinfo = devinfo;

	ds_printk(KERN_INFO "found %s device information\n", devinfo->name);
	dma->regs->CONS = 0;
	dma->regs->COM0 = devinfo->com0;
	dma->regs->COM1 = devinfo->com1;
	dma->regs->CONS = devinfo->con;
	if (devinfo->srcaddr != ADDR_MEM) {
		dma->regs->SRCLADDR = (devinfo->srcaddr & 0x0000ffff);
		dma->regs->SRCHADDR = (devinfo->srcaddr >> 16);
	} 
	if (devinfo->trgaddr != ADDR_MEM) {
		dma->regs->TRGLADDR = (devinfo->trgaddr & 0x0000ffff);
		dma->regs->TRGHADDR = (devinfo->trgaddr >> 16);
	}

	if (dma->devinfo->type) {	/* write, MEM -> DEV */
		dma->iomap->SRM = 0;
		dma->iomap->TRM = ((1<<6) | (dev_id));
	} else {			/* read, DEV -> MEM */
		dma->iomap->SRM = ((1<<6) | (dev_id));
		dma->iomap->TRM = 0;
	}

	ds_printk("COM0 = 0x%x\n", dma->regs->COM0);
	ds_printk("COM1 = 0x%x\n", dma->regs->COM1);
	ds_printk("COM2 = 0x%x\n", dma->regs->COM2);
	ds_printk("CONS = 0x%x\n", dma->regs->CONS);
	ds_printk("SRCL = 0x%x\n", dma->regs->SRCLADDR);
	ds_printk("SRCH = 0x%x\n", dma->regs->SRCHADDR);
	ds_printk("TRGL = 0x%x\n", dma->regs->TRGLADDR);
	ds_printk("TRGH = 0x%x\n", dma->regs->TRGHADDR);
	ds_printk("SRM = 0x%x\n", dma->iomap->SRM);
	ds_printk("TRM = 0x%x\n", dma->iomap->TRM);

	return 0;
}

static int gstart=0; 
static spinlock_t dma_list_lock;
int mmsp2_request_dma(int device_id, dma_callback_t cb)
{
	int i, ret;
	mmsp2_dma_t *dma = NULL;

	spin_lock(&dma_list_lock);
	for (i = 0; i < MAX_MMSP2_DMA_CHANNELS; i++) {
		if(i!=0 && i!=3) { //reserved CH0 and CH3 for ARM940 MPEG codec
			dma = &dma_chan[i];
			if (!dma->in_use) {
				dma->channel = i;//
				dma->in_use = 1;
				ds_printk(KERN_INFO "DMA%d channel is allocated\n", i);
				break;
			}
		}
	}
	spin_unlock(&dma_list_lock);

	if (i == MAX_MMSP2_DMA_CHANNELS) {
		printk(KERN_WARNING "All DMA channle is busy\n");
		return -EBUSY;
	}

	if (fill_devinfo(dma, device_id)) {
		printk(KERN_ERR "unknown device id (%d)\n", device_id);
		dma->in_use = 0;
		return -1;
	}


	ds_printk("---> %d irq requested\n", dma->irq);
	ret = request_irq(dma->irq, dma_irq_handler, 0 * SA_INTERRUPT,
			  "DMA", (void *)dma);
	if (ret) {
		printk(KERN_ERR "[ID:%d]: unable to request IRQ %d"
				" for DMA channel %d\n",
				device_id, dma->irq, dma->channel);
		dma->in_use = 0;
		return ret;
	}

	dma->device_id = device_id;
	dma->head = dma->tail = dma->curr = NULL;
	dma->callback = cb;
	ds_printk("cb = %p\n", dma->callback);
       
	return dma->channel;
}

int mmsp2_dma_queue_buffer(dmach_t channel, void *buf_id, 
			   dma_addr_t data, int size)
{
	mmsp2_dma_t *dma;
	dma_buf_t *buf;
	int flags;

	if (size >= MAX_DMA_BUF_SIZE)
		return -EINVAL;

	if ((channel < 0) || (channel >= MAX_MMSP2_DMA_CHANNELS))
		return -EINVAL;

	dma = &dma_chan[channel];
	if (!dma->in_use)
		return -EINVAL;

	buf = kmalloc(sizeof(*buf), GFP_ATOMIC);
	if (!buf)
		return -ENOMEM;

	buf->next = NULL;
	buf->ref = 0;
	buf->dma_start = data;
	buf->size = size;
	buf->id = buf_id;
	ds_printk("queueing b=%#x, a=%#x, s=%d\n", (int)buf_id, data, size);

	local_irq_save(flags);
	if (dma->tail)
		dma->tail->next = buf;
	else
		dma->head = buf;
	dma->tail = buf;
	buf->next = NULL;
	dma->queue_count++;

	ds_printk("number of buffers in queue: %ld\n", dma->queue_count);
	ds_printk("queue_cnt: %ld\n", dma->queue_count);
	process_dma(dma);

	local_irq_restore(flags);

	return 0;
}

int mmsp2_dma_flush_all(dmach_t channel)
{
	mmsp2_dma_t *dma;
	dma_buf_t *buf, *next_buf;
	int flags;

	if ((channel < 0) || (channel >= MAX_MMSP2_DMA_CHANNELS))
		return -EINVAL;

	dma = &dma_chan[channel];
	if (!dma->in_use)
		return -EINVAL;

	while(1)				// hhsong 051125 add for mplayer seek DMA flush..
		if (dma->regs->COM2 == 0) break;

	local_irq_save(flags);
	dma_stop(dma);
	buf = dma->head;
	dma->head = dma->tail = dma->curr = NULL;
	dma->active = 0;
	dma->queue_count = 0;
	dma->active = 0;
	local_irq_restore(flags);
	while (buf) {
		next_buf = buf->next;
		kfree(buf);
		buf = next_buf;
	}
	ds_printk("flushed\n");
	return 0;
}

void mmsp2_free_dma(dmach_t channel)
{
	mmsp2_dma_t *dma;

	if ((channel < 0) || (channel >= MAX_MMSP2_DMA_CHANNELS))
		return;

	dma = &dma_chan[channel];
	if (!dma->in_use) {
		printk(KERN_ERR "Trying to free DMA%d\n", channel);
		return;
	}

	mmsp2_dma_flush_all(channel);

	free_irq(dma->irq, (void *)dma);

	dma->in_use = 0;

	ds_printk("free DMA%d\n", channel);
}

int mmsp2_dma_get_send_size(dmach_t channel)
{
	mmsp2_dma_t *dma;
	int flags, send_size;
	
	if ((channel < 0) || (channel >= MAX_MMSP2_DMA_CHANNELS))
		return -2;

	dma = &dma_chan[channel];
	if (!dma->in_use)
		return -1;
		
	local_irq_save(flags);
	send_size = (dma->curr->size - dma->regs->COM2 + 1);
	local_irq_restore(flags);
	
//	printk("send_size = %d, dma->curr->size = %d, dma->regs->COM2 = %d\n",send_size,dma->curr->size,dma->regs->COM2);
	return send_size;
}


EXPORT_SYMBOL(mmsp2_request_dma);
EXPORT_SYMBOL(mmsp2_dma_queue_buffer);
EXPORT_SYMBOL(mmsp2_dma_flush_all);
EXPORT_SYMBOL(mmsp2_free_dma);

static int __init mmsp2_init_dma(void)
{
	int chan;

	for (chan = 0; chan < MAX_MMSP2_DMA_CHANNELS; chan++) {
		dma_chan[chan].regs = (dma_regs_t *)io_p2v(0xc0000200 + (0x10 * chan));
		dma_chan[chan].iomap = (dma_iomap_t *)io_p2v(0xc0000100 + (0x4 * chan));
		dma_chan[chan].irq = IRQ_DMA_SUB(0) + chan;		//IRQ_DMA0 + chan;
		dma_chan[chan].channel = chan;

		memset(dma_chan[chan].regs, 0x00, sizeof(dma_regs_t));
		memset(dma_chan[chan].iomap, 0x00, sizeof(dma_iomap_t));
	}

	return 0;
}

__initcall(mmsp2_init_dma);
