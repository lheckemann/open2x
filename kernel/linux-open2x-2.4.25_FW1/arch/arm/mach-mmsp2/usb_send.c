/*
 * linux/arch/arm/mach-mmsp2/usb_send.c
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * Generic xmit layer for the MMSP2 USB client function
 *
 * Based on linux/kernel/arch/arm/mach-sa1100/usb_send.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <asm/hardware.h>
#include <asm/system.h>
#include <asm/byteorder.h>

#include "mmsp2_usb.h"
#include "usb_ctl.h"

#define SET_EP2_IN_PKT_RDY()	IN_CSR1_REG=((IN_CSR1_REG & (~EPI_WR_BITS))| \
					EPI_IN_PKT_RDY)
#define SET_EP2_SEND_STALL()	IN_CSR1_REG=((IN_CSR1_REG & (~EPI_WR_BITS))| \
					EPI_SEND_STALL)
#define CLR_EP2_SEND_STALL()	IN_CSR1_REG=((IN_CSR1_REG & (~EPI_WR_BITS))& \
					(~EPI_SEND_STALL))
#define CLR_EP2_SENT_STALL()	IN_CSR1_REG=((IN_CSR1_REG & (~EPI_WR_BITS))& \
					(~EPI_SENT_STALL))
#define FLUSH_EP2_FIFO() 	IN_CSR1_REG=((IN_CSR1_REG & (~EPI_WR_BITS))| \
					EPI_FIFO_FLUSH)

#if DEBUG
static unsigned int usb_debug = DEBUG;
#else
#define usb_debug 0     /* gcc will remove all the debug code for us */
#endif

static char *ep_bulk_in1_buf;
static int   ep_bulk_in1_len;
static int   ep_bulk_in1_remain;
static usb_callback_t ep_bulk_in1_callback;
static int tx_pktsize;

/* device state is changing, async */
void
ep_bulk_in1_state_change_notify( int new_state )
{
}

/* set feature stall executing, async */
void
ep_bulk_in1_stall( void )
{
	__u32 i;

	i = INDEX_REG;
	INDEX_REG = 0x02;
	SET_EP2_SEND_STALL();
	INDEX_REG = i;
}

static void
ep_bulk_in1_send_packet(void)
{
	int i, ind;
	char *buf = ep_bulk_in1_buf + ep_bulk_in1_len - ep_bulk_in1_remain;
	int out_size = tx_pktsize;

	ind = INDEX_REG;
	INDEX_REG = 0x02;

	if(out_size > ep_bulk_in1_remain) {
		out_size = ep_bulk_in1_remain;
	}

	for(i=0; i<out_size; i++) {
		EP2_FIFO_REG = *buf++;
	}
	SET_EP2_IN_PKT_RDY();	// Set IN_PKT_RDY
	if(out_size < tx_pktsize) {
		/* short packet */
	}
	ep_bulk_in1_remain -= out_size;

	if(usb_debug) printk("ep_bulk_in1_send_packet: EP_INT_EN_REG=%x IN_CSR1_REG=%x send"
		" bytes=%d left=%d\n", EP_INT_EN_REG, IN_CSR1_REG, out_size, ep_bulk_in1_remain);
	INDEX_REG = ind;
}

static void
ep_bulk_in1_start(void)
{
	if(!ep_bulk_in1_len)
		return;

	EP_INT_EN_REG |= EP2_INT;

	ep_bulk_in1_send_packet();
}

static void
ep_bulk_in1_done(int flag)
{
	int size = ep_bulk_in1_len - ep_bulk_in1_remain;

	if(ep_bulk_in1_len) {
		ep_bulk_in1_len = 0;
		if (ep_bulk_in1_callback)
			ep_bulk_in1_callback(flag, size);
	}
}

int
ep_bulk_in1_init(int chn)
{
	desc_t * pd = mmsp2_usb_get_descriptor_ptr();
	tx_pktsize = __le16_to_cpu(pd->b.ep2.wMaxPacketSize);
	ep_bulk_in1_done(-EAGAIN);
	return 0;
}

void
ep_bulk_in1_reset(void)
{
	__u32 i;
	desc_t * pd = mmsp2_usb_get_descriptor_ptr();
	tx_pktsize = __le16_to_cpu(pd->b.ep2.wMaxPacketSize);

	i = INDEX_REG;
	INDEX_REG = 0x02;
	MAXP_REG = FIFO_SIZE_64;
	IN_CSR1_REG = EPI_FIFO_FLUSH|EPI_CDT;
	IN_CSR2_REG = EPI_MODE_IN|EPI_IN_DMA_INT_MASK|EPI_BULK;
	OUT_CSR1_REG = EPO_CDT;
	OUT_CSR2_REG = EPO_BULK|EPO_OUT_DMA_INT_MASK;
	INDEX_REG = i;
	ep_bulk_in1_done(-EINTR);
}

void
ep_bulk_in1_int_hndlr(int usir0)
{
	__u32 i;

	i = INDEX_REG;
	INDEX_REG = 0x02;

	if(IN_CSR1_REG & EPI_SENT_STALL) {
		if(usb_debug) printk("usb_send: sent stall.\n");
		CLR_EP2_SENT_STALL();
		return;
	}

	if (ep_bulk_in1_remain != 0) {
		/* more data to go */
		ep_bulk_in1_start();
	} else {
		ep_bulk_in1_done(0);
	}
	INDEX_REG = i;
}

int
mmsp2_usb_send(char *buf, int len, usb_callback_t callback)
{
	int flags;

	if(usb_debug) printk( "usb_send: data len=%d state=%d blen=%d\n",
		len, usbd_info.state, ep_bulk_in1_len);

	if(usbd_info.state != USB_STATE_CONFIGURED)
		return -ENODEV;

	if(ep_bulk_in1_len)
		return -EBUSY;

	local_irq_save(flags);
	ep_bulk_in1_buf = buf;
	ep_bulk_in1_len = len;
	ep_bulk_in1_callback = callback;
	ep_bulk_in1_remain = len;
	ep_bulk_in1_start();
	local_irq_restore(flags);

	return 0;
}

void
mmsp2_usb_send_reset(void)
{
	ep_bulk_in1_reset();
}

int
mmsp2_usb_xmitter_avail(void)
{
	if(usbd_info.state != USB_STATE_CONFIGURED)
		return -ENODEV;
	if(ep_bulk_in1_len)
		return -EBUSY;
	return 0;
}

EXPORT_SYMBOL(mmsp2_usb_xmitter_avail);
EXPORT_SYMBOL(mmsp2_usb_send);
EXPORT_SYMBOL(mmsp2_usb_send_reset);
