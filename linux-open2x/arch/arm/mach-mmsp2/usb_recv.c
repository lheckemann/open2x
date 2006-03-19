/*
 * linux/arch/arm/mach-mmsp2/usb_recv.c
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * Generic receive layer for the MMSP2 USB client function
 *
 * Based on linux/kernel/arch/arm/mach-sa1100/usb_recv.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <asm/system.h>

#include "mmsp2_usb.h"
#include "usb_ctl.h"

#define CLR_EP1_OUT_PKT_RDY()	OUT_CSR1_REG=((OUT_CSR1_REG & (~EPO_WR_BITS))& \
					(~EPO_OUT_PKT_RDY))
#define SET_EP1_SEND_STALL()	OUT_CSR1_REG=((OUT_CSR1_REG & (~EPO_WR_BITS))| \
					EPO_SEND_STALL)
#define CLR_EP1_SENT_STALL()	OUT_CSR1_REG=((OUT_CSR1_REG & (~EPO_WR_BITS))& \
					(~EPO_SENT_STALL))
#define FLUSH_EP1_FIFO() 	OUT_CSR1_REG=((OUT_CSR1_REG & (~EPO_WR_BITS))| \
					EPO_FIFO_FLUSH)

#if DEBUG
static unsigned int usb_debug = DEBUG;
#else
#define usb_debug 0     /* gcc will remove all the debug code for us */
#endif

static char *ep_bulk_out1_buf;
static int   ep_bulk_out1_len;
static int   ep_bulk_out1_remain;
static usb_callback_t ep_bulk_out1_callback;
static int rx_pktsize;

static void
ep_bulk_out1_start(void)
{
	/* disable DMA */
	EP1_DMA_CON &= ~(0x01);

	/* enable interrupts for endpoint 1 (bulk out) */
	EP_INT_EN_REG |=  EP1_INT;
}

static void
ep_bulk_out1_done(int flag)
{
	int size = ep_bulk_out1_len - ep_bulk_out1_remain;

	if (!ep_bulk_out1_len)
		return;

	ep_bulk_out1_len = 0;
	if (ep_bulk_out1_callback) {
		ep_bulk_out1_callback(flag, size);
	}
}

void
ep_bulk_out1_state_change_notify(int new_state)
{
}

void
ep_bulk_out1_stall(void)
{
	/* SET_FEATURE force stall at UDC */
	__u32 i;

	i = INDEX_REG;
	INDEX_REG = 0x01;
	SET_EP1_SEND_STALL();
	INDEX_REG = i;
}

int
ep_bulk_out1_init(int chn)
{
	desc_t * pd = mmsp2_usb_get_descriptor_ptr();
	rx_pktsize = __le16_to_cpu(pd->b.ep1.wMaxPacketSize);
	ep_bulk_out1_done(-EAGAIN);
	return 0;
}

void
ep_bulk_out1_reset(void)
{
	__u32 i;
	desc_t * pd = mmsp2_usb_get_descriptor_ptr();
	rx_pktsize = __le16_to_cpu(pd->b.ep1.wMaxPacketSize);

	i = INDEX_REG;
	INDEX_REG = 0x01;
	MAXP_REG = FIFO_SIZE_64;
	IN_CSR1_REG = EPI_FIFO_FLUSH|EPI_CDT|EPI_BULK;
	IN_CSR2_REG = EPI_MODE_OUT|EPI_IN_DMA_INT_MASK;
	OUT_CSR1_REG = EPO_CDT;
	OUT_CSR2_REG = EPO_BULK|EPO_OUT_DMA_INT_MASK;
	INDEX_REG = i;
	ep_bulk_out1_done(-EINTR);
}

void
ep_bulk_out1_int_hndlr(int udcsr)
{
	int ind, i, count;

	ind = INDEX_REG;
	INDEX_REG = 0x01;

	if(OUT_CSR1_REG & EPO_OUT_PKT_RDY) {
		count = OUT_FIFO_CNT1_REG & 0x00FF;
		if(count==0) {
			/* zero-length packet */
		}
		else {
			char *buf = ep_bulk_out1_buf + ep_bulk_out1_len - ep_bulk_out1_remain;

			if(usb_debug) printk("usb_recv: len=%d out1_len=%d out1_remain=%d\n",
				count,ep_bulk_out1_len,ep_bulk_out1_remain);

			if(count > ep_bulk_out1_remain) {
				/* FIXME: if this happens, we need a temporary overflow buffer */
				printk("usb_recv: Buffer overwrite warning...\n");
				count = ep_bulk_out1_remain;
			}

			/* read data out of fifo */
			for( i=0; i<count; i++) {
				*buf++ = EP1_FIFO_REG & 0x00FF;
			}

			ep_bulk_out1_remain -= count;
			ep_bulk_out1_done((count) ? 0 : -EPIPE);
		}

		/* clear OUT_PKT_RDY */
		CLR_EP1_OUT_PKT_RDY();
	}
	INDEX_REG = ind;
	return;
}

int
mmsp2_usb_recv(char *buf, int len, usb_callback_t callback)
{
	int flags;

	if (ep_bulk_out1_len)
		return -EBUSY;

	local_irq_save(flags);
	ep_bulk_out1_buf = buf;
	ep_bulk_out1_len = len;
	ep_bulk_out1_callback = callback;
	ep_bulk_out1_remain = len;
	ep_bulk_out1_start();
	local_irq_restore(flags);

	return 0;
}

void
mmsp2_usb_recv_reset(void)
{
	ep_bulk_out1_reset();
}

void
mmsp2_usb_recv_stall(void)
{
	ep_bulk_out1_stall();
}

EXPORT_SYMBOL(mmsp2_usb_recv_stall);
EXPORT_SYMBOL(mmsp2_usb_recv);
EXPORT_SYMBOL(mmsp2_usb_recv_reset);
