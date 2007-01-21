/*
 * linux/arch/arm/mach-mmsp2/usb_ctl.h
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _USB_CTL_H
#define _USB_CTL_H

#include <asm/dma.h>  /* dmach_t */

/*
 * These states correspond to those in the USB specification v1.0
 * in chapter 8, Device Framework.
 */
enum {
	USB_STATE_NOTATTACHED	=0,
	USB_STATE_ATTACHED	=1,
	USB_STATE_POWERED	=2,
	USB_STATE_DEFAULT	=3,
	USB_STATE_ADDRESS	=4,
	USB_STATE_CONFIGURED	=5,
	USB_STATE_SUSPENDED	=6
};

struct usb_stats_t {
	 unsigned long ep0_fifo_write_failures;
	 unsigned long ep0_bytes_written;
	 unsigned long ep0_fifo_read_failures;
	 unsigned long ep0_bytes_read;
};

struct usb_info_t
{
	 char * client_name;
	 dmach_t dmach_tx, dmach_rx;
	 int state;
	 unsigned char address;
	 struct usb_stats_t stats;
};

/* in usb_ctl.c */
extern struct usb_info_t usbd_info;

/*
 * Function Prototypes
 */
enum {
	kError		=-1,
	kEvSuspend	=0,
	kEvReset	=1,
	kEvResume	=2,
	kEvAddress	=3,
	kEvConfig	=4,
	kEvDeConfig	=5
};
int usbctl_next_state_on_event( int event );

/* endpoint zero */
void ep0_reset(void);
void ep0_int_hndlr(void);

/* receiver */
void ep_bulk_out1_state_change_notify( int new_state );
int  ep_bulk_out1_recv(void);
int  ep_bulk_out1_init(int chn);
void ep_bulk_out1_int_hndlr(int status);
void ep_bulk_out1_reset(void);
void ep_bulk_out1_stall(void);

/* xmitter */
void ep_bulk_in1_state_change_notify( int new_state );
void ep_bulk_in1_reset(void);
int  ep_bulk_in1_init(int chn);
void ep_bulk_in1_int_hndlr(int status);
void ep_bulk_in1_stall(void);

#endif /* _USB_CTL_H */
