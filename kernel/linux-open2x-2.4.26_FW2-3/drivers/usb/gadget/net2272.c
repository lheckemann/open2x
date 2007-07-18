/*
 * Driver for PLX NET2272 USB device controller
 *
 * Copyright (C) 2005-2006 PLX Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// #define PLX_PCI_RDK             /* Using the PLX 2272 PCI bus RDK version 1 board for development */
// #define PLX_PCI_RDK2            /* Using the PLX 2272 PCI bus RDK version 2 board for development */
                                /* If neither RDK defined will assume base I/O and irq supplied to driver */

#include <linux/config.h>
#include <linux/module.h>

#if defined(PLX_PCI_RDK) | defined(PLX_PCI_RDK2)
#include <linux/pci.h>
#endif

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>

#include <linux/usb_ch9.h>
#include <linux/usb_gadget.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/unaligned.h>

// #define	DEBUG			/* messages on error and most fault paths */
// #define	VERBOSE			/* extra debug messages (success too) */

#define VDEBUG(dev,fmt,args...) \
	do { } while (0)

#if defined(PLX_PCI_RDK)
#define DRIVER_DESC		"PLX NET2272 PCI-RDK version 1 USB Peripheral Controller"
#endif
#if defined(PLX_PCI_RDK2)
#define DRIVER_DESC		"PLX NET2272 PCI-RDK version 2 USB Peripheral Controller"
#endif
#if !defined(PLX_PCI_RDK) &&  !defined(PLX_PCI_RDK2)
#define DRIVER_DESC		"PLX NET2272 USB Peripheral Controller"
#endif

#define DRIVER_VERSION		"2006 October 17"

#define DMA_ADDR_INVALID	(~(dma_addr_t)0)

static const char driver_name [] = "net2272";
static const char driver_desc [] = DRIVER_DESC;

static const char ep0name [] = "ep0";
static const char *ep_name [] = {
	ep0name,
	"ep-a", "ep-b", "ep-c",
};

/*---------------------------------------------------------------------------*/
#if !defined(PLX_PCI_RDK) && !defined(PLX_PCI_RDK2)
/* base - net2272 base address */
#ifndef CONFIG_MACH_GP2X
static unsigned long base = 0;
MODULE_PARM (base, "i");
MODULE_PARM_DESC (base, "NET2272 base address");

/* irq - net2272 irq */
static ushort irq = 0;
MODULE_PARM (irq, "i");
MODULE_PARM_DESC (irq, "NET2272 IRQ");
#else
static unsigned long base = 0;
static ushort irq = 0;
#endif
#endif

/* enable_suspend -- When enabled, the driver will respond to
 * USB suspend requests by powering down the NET2272.  Otherwise,
 * USB suspend requests will be ignored.  This is acceptible for
 * self-powered devices. For bus powered devices set this to 1
 */
static int enable_suspend = 1;

/* "modprobe net2272 enable_suspend=1" etc */
MODULE_PARM (enable_suspend, "i");
MODULE_PARM_DESC (enable_suspend, "enable net2272 suspend");

/* If present, the NET2272 can use an external DMA controller.
 * Note that since there is no generic DMA api, some functions, notably
 * request_dma, start_dma, and cancel_dma will need to be modified for your
 * platform's particular dma controller.
 *
 * If use_dma is disabled, pio will be used instead.
 */
static int use_dma = 0;
MODULE_PARM (use_dma, "i");
MODULE_PARM_DESC (use_dma, "enable net2272 dma");

/* dma_ep: selects the endpoint for use with dma (1=ep-a, 2=ep-b)
 * The NET2272 can only use dma for a single endpoint at a time.  At some point
 * this could be modified to allow either endpoint to take control of dma as it
 * becomes available.
 *
 * Note that DMA should not be used on OUT endpoints unless it can be
 * guaranteed that no short packets will arrive on an IN endpoint while the
 * DMA operation is pending.  Otherwise the OUT DMA will terminate
 * prematurely (See NET2272 Errata 630-0213-0101)
 */
static ushort dma_ep = 1;
MODULE_PARM (dma_ep, "i");
MODULE_PARM_DESC (dma_em, "Select dma endpoint: 1=ep-a, 2=ep-b");

/* dma_mode - net2272 dma mode setting (see LOCCTL1 definiton):
 *	mode 0 == Slow DREQ mode
 *	mode 1 == Fast DREQ mode
 *	mode 2 == Burst mode
 */
static ushort dma_mode = 2;
MODULE_PARM (dma_mode, "i");
MODULE_PARM_DESC (dma_mode, "DMA Mode: 0=Slow DREQ, 1=Fast DREQ, 2=Burst");

/* fifo_mode - net2272 buffer configuration:
 *      mode 0 == ep-{a,b,c} 512db each
 *      mode 1 == ep-a 1k, ep-{b,c} 512db
 *      mode 2 == ep-a 1k, ep-b 1k, ep-c 512db
 *      mode 3 == ep-a 1k, ep-b disabled, ep-c 512db
 */
static ushort fifo_mode = 0;

/* "modprobe net2272 fifo_mode=1" etc */
MODULE_PARM (fifo_mode, "i");
MODULE_PARM_DESC (fifo_mode, "NET2272 Buffer Mode");


#ifdef CONFIG_MACH_GP2X	
#include <asm/arch/mmsp20.h>
typedef struct 
{
	u16	MEMCFGW;
	u16	MEMTIMEW0;
	u16	MEMTIMEW1;
	u16	MEMTIMEW2;
	u16	MEMTIMEW3;
	u16	MEMTIMEW4;
	u16	MEMTIMEW5;
	u16	MEMWAITCTRLW;
	u16	MEMPAGEW;
	u16	MEMIDETIMEW;
	u16	MEMPCMCIAMW;
	u16	MEMPCMCIAAW;
	u16	MEMPCMCIAIW;
	u16	MEMPCMCIAWAITW;
	u16	MEMIDEWAITW;
	u16 Reserved;
	u16	MEMDTIMEOUTW;
	u16	MEMDMACTRLW;
	u16	MEMDMAPOLW;
	u16	MEMDMATIMEW0;
	u16	MEMDMATIMEW1;
	u16	MEMDMATIMEW2;
	u16	MEMDMATIMEW3;
	u16	MEMDMATIMEW4;
	u16	MEMDMATIMEW5;
	u16	MEMDMATIMEW6;
	u16	MEMDMATIMEW7;
	u16	MEMDMATIMEW8;
	u16	MEMDMASTRBW;
	u16	MEMNANDCTRLW;
	u16	MEMNANDTIMEW;
	u16	MEMNANDECC0W;
	u16	MEMNANDECC1W;
	u16	MEMNANDECC2W;
	u16	MEMNANDCNTW;
}MCUWREG;	


//Timeging

#define Wait		(0x01<<4)
#define Tacs2		(0x01<<4)
#define Tcos2		(0x01<<4)
#define Tcoh2		(0x01<<4)
#define Tcah2		(0x01<<4)
#define Tacc2		(0x01<<8)



#if 0
#define Tacs2		(0x02<<4)	/* TIME 0*/
#define Tcos2		(0x00<<4)	/* TIME 1*/
#define Tcoh2		(0x00<<4)	/* TIME 2*/
#define Tcah2		(0x04<<4)	/* TIME 3*/
#define Tacc2		(0x05<<8)  	/* TIME 4*/
#endif

// MODE0 @ 60Mhz
#define	tD			0x7
#define tK			0x3
#define tACK		0x2
#define tENV		0x2
#define tRP			0xf
#define tSS			0x5
#define tMLI		0x2
#define STROBE		0x5

#define _MEMCFG 	0xc0003a00
static volatile MCUWREG	*pMCUWREG;
#define IRQ_USB_INT	IRQ_GPIO_J(14)
#define BASE_USB	0x88000000
#endif	

#include "net2272.h"

/*---------------------------------------------------------------------------*/

#define DIR_STRING(bAddress) (((bAddress) & USB_DIR_IN) ? "in" : "out")

#if defined(CONFIG_USB_GADGET_DEBUG_FILES) || defined(DEBUG)
static char *type_string (u8 bmAttributes)
{
	switch ((bmAttributes) & USB_ENDPOINT_XFERTYPE_MASK) {
	case USB_ENDPOINT_XFER_BULK:    return "bulk";
	case USB_ENDPOINT_XFER_ISOC:    return "iso";
	case USB_ENDPOINT_XFER_INT:     return "intr";
	};
	return "control";
}

static char *buf_state_string (unsigned state)
{
	switch (state) {
	case BUFF_FREE:		return "free";
	case BUFF_VALID:	return "valid";
	case BUFF_LCL:		return "local";
	case BUFF_USB:		return "usb";
	};
	return "unknown";
}
#endif

static char *dma_mode_string (void)
{
	switch (dma_mode) {
	case 0:
		return "SLOW DREQ";
	case 1:
		return "FAST DREQ";
	case 2:
		return "BURST";
	default:
		return "invalid";
	}
}

/*---------------------------------------------------------------------------*/

static int
net2272_enable (struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc)
{
	struct net2272		*dev;
	struct net2272_ep	*ep;
	u32			max;
	u8			tmp;
	unsigned long		flags;

	ep = container_of (_ep, struct net2272_ep, ep);
	if (!_ep || !desc || ep->desc || _ep->name == ep0name
			|| desc->bDescriptorType != USB_DT_ENDPOINT)
		return -EINVAL;
	dev = ep->dev;
	if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN)
		return -ESHUTDOWN;
        
        // carful here that le16_to_cpu is correct for your cpu if it is big endian
	max = le16_to_cpu (desc->wMaxPacketSize) & 0x1fff;
	
	spin_lock_irqsave (&dev->lock, flags);
	_ep->maxpacket = max & 0x7fff;
	ep->desc = desc;

	/* ep_reset() has already been called */
	ep->stopped = 0;

	/* set speed-dependent max packet */
	net2272_ep_write (ep, EP_MAXPKT0, max & 0xff);
	net2272_ep_write (ep, EP_MAXPKT1, (max & 0xff00) >> 8);

	/* set type, direction, address; reset fifo counters */
	net2272_ep_write (ep, EP_STAT1, 1 << BUFFER_FLUSH);
	tmp = (desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK);
	if (tmp == USB_ENDPOINT_XFER_BULK) {
		/* catch some particularly blatant driver bugs */
		if ((dev->gadget.speed == USB_SPEED_HIGH
					&& max != 512)
				|| (dev->gadget.speed == USB_SPEED_FULL
					&& max > 64)) {
			spin_unlock_irqrestore (&dev->lock, flags);
			return -ERANGE;
		}
	}
	ep->is_iso = (tmp == USB_ENDPOINT_XFER_ISOC) ? 1 : 0;
	tmp <<= ENDPOINT_TYPE;
	tmp |= ((desc->bEndpointAddress & 0x0f) << ENDPOINT_NUMBER);
	tmp |= (((desc->bEndpointAddress & USB_DIR_IN) ? 
				1 : 0) << ENDPOINT_DIRECTION);
	tmp |= (1 << ENDPOINT_ENABLE);

	/* for OUT transfers, block the rx fifo until a read is posted */
	ep->is_in = (desc->bEndpointAddress & USB_DIR_IN) != 0;
	if (!ep->is_in)
		net2272_ep_write (ep, EP_RSPSET, 1 << ALT_NAK_OUT_PACKETS);

	net2272_ep_write (ep, EP_CFG, tmp);
	
	/* enable irqs */
	tmp = (1 << ep->num) | net2272_read (dev, IRQENB0);
	net2272_write (dev, IRQENB0, tmp);

	tmp = (1 << DATA_PACKET_RECEIVED_INTERRUPT_ENABLE)
		| (1 << DATA_PACKET_TRANSMITTED_INTERRUPT_ENABLE)
		| net2272_ep_read (ep, EP_IRQENB);
	net2272_ep_write (ep, EP_IRQENB, tmp);

	tmp = desc->bEndpointAddress;
	DEBUG (dev, "enabled %s (ep%d%s-%s) max %04x cfg %02x\n",
			_ep->name, tmp & 0x0f, DIR_STRING (tmp),
			type_string (desc->bmAttributes), max,
			net2272_ep_read (ep, EP_CFG));

	spin_unlock_irqrestore (&dev->lock, flags);
	return 0;
}

static struct usb_ep_ops net2272_ep_ops;

static void ep_reset (struct net2272_ep *ep)
{
	u8			tmp;

	ep->desc = NULL;
	INIT_LIST_HEAD (&ep->queue);

	ep->ep.maxpacket = ~0;
	ep->ep.ops = &net2272_ep_ops;

	/* disable irqs, endpoint */
	net2272_ep_write (ep, EP_IRQENB, 0);

	/* init to our chosen defaults, notably so that we NAK OUT
	 * packets until the driver queues a read.
	 */
	tmp = (1 << NAK_OUT_PACKETS_MODE) | (1 << ALT_NAK_OUT_PACKETS);
	net2272_ep_write (ep, EP_RSPSET, tmp);

	tmp = (1 << INTERRUPT_MODE) | (1 << HIDE_STATUS_PHASE);
	if (ep->num != 0)
		tmp |= (1 << ENDPOINT_TOGGLE) | (1 << ENDPOINT_HALT);

	net2272_ep_write (ep, EP_RSPCLR, tmp);

	/* scrub most status bits, and flush any fifo state */
	net2272_ep_write (ep, EP_STAT0,
			  (1 << DATA_IN_TOKEN_INTERRUPT)
			| (1 << DATA_OUT_TOKEN_INTERRUPT)
			| (1 << DATA_PACKET_TRANSMITTED_INTERRUPT)
			| (1 << DATA_PACKET_RECEIVED_INTERRUPT)
			| (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT));

	net2272_ep_write (ep, EP_STAT1,
			    (1 << TIMEOUT)
			  | (1 << USB_OUT_ACK_SENT)
			  | (1 << USB_OUT_NAK_SENT)
			  | (1 << USB_IN_ACK_RCVD)
			  | (1 << USB_IN_NAK_SENT)
			  | (1 << USB_STALL_SENT)
			  | (1 << LOCAL_OUT_ZLP)
			  | (1 << BUFFER_FLUSH));

	/* fifo size is handled seperately */
}

static void nuke (struct net2272_ep *);

static int net2272_disable (struct usb_ep *_ep)
{
	struct net2272_ep	*ep;
	unsigned long		flags;

	ep = container_of (_ep, struct net2272_ep, ep);
	if (!_ep || !ep->desc || _ep->name == ep0name)
		return -EINVAL;

	spin_lock_irqsave (&ep->dev->lock, flags);
	nuke (ep);
	ep_reset (ep);

	VDEBUG (ep->dev, "disabled %s\n", _ep->name);

	spin_unlock_irqrestore (&ep->dev->lock, flags);
	return 0;
}

/*---------------------------------------------------------------------------*/

static struct usb_request *
net2272_alloc_request (struct usb_ep *_ep, int gfp_flags)
{
	struct net2272_ep	*ep;
	struct net2272_request	*req;

	if (!_ep)
		return NULL;
	ep = container_of (_ep, struct net2272_ep, ep);

	req = kmalloc (sizeof *req, gfp_flags);
	if (!req)
		return NULL;

	memset (req, 0, sizeof *req);
	req->req.dma = DMA_ADDR_INVALID;
	INIT_LIST_HEAD (&req->queue);

	return &req->req;
}

static void
net2272_free_request (struct usb_ep *_ep, struct usb_request *_req)
{
	struct net2272_ep	*ep;
	struct net2272_request	*req;

	ep = container_of (_ep, struct net2272_ep, ep);
	if (!_ep || !_req)
		return;

	req = container_of (_req, struct net2272_request, req);
	WARN_ON (!list_empty (&req->queue));
	kfree (req);
}

static void *
net2272_alloc_buffer (
	struct usb_ep	*_ep,
	unsigned	bytes,
	dma_addr_t	*dma,
	int		gfp_flags
)
{
	void			*retval;
	struct net2272_ep	*ep;

	ep = container_of (_ep, struct net2272_ep, ep);
	if (!_ep)
		return NULL;
	*dma = DMA_ADDR_INVALID;
	retval = kmalloc (bytes, gfp_flags);
	if (retval)
		*dma = virt_to_phys(retval);

	return retval;
}

static void
net2272_free_buffer (
	struct usb_ep		*_ep,
	void			*buf,
	dma_addr_t		dma,
	unsigned		bytes
)
{
	kfree (buf);
}

static void
done (struct net2272_ep *ep, struct net2272_request *req, int status)
{
	struct net2272		*dev;
	unsigned		stopped = ep->stopped;
	
	if (ep->num == 0) {
		if (ep->dev->protocol_stall) {
			ep->stopped = 1;
			set_halt (ep);
		}
		allow_status (ep);
	}

	list_del_init (&req->queue);

	if (req->req.status == -EINPROGRESS)
		req->req.status = status;
	else
		status = req->req.status;

	dev = ep->dev;
	
	if (status && status != -ESHUTDOWN)
		VDEBUG (dev, "complete %s req %p stat %d len %u/%u buf %p\n",
			ep->ep.name, &req->req, status,
			req->req.actual, req->req.length, req->req.buf);

	/* don't modify queue heads during completion callback */
	ep->stopped = 1;
	spin_unlock (&dev->lock);
	req->req.complete (&ep->ep, &req->req);
	spin_lock (&dev->lock);
	ep->stopped = stopped;
}

static int
write_packet (struct net2272_ep *ep, 
		u8 *buf, struct net2272_request *req, unsigned max)
{
	u16		__iomem *ep_data = ep->dev->base_addr + EP_DATA;
	unsigned	length, count;
	u16		*bufp;
	u8		tmp;

	length = min (req->req.length - req->req.actual, max);
	req->req.actual += length;

#if 0
	VDEBUG (ep->dev, "write packet locctl %02X %s req %p max %u len %u avail %u\n",
		        net2272_read (ep->dev, LOCCTL),
			ep->ep.name, req, max, length, 
			(net2272_ep_read (ep, EP_AVAIL1) << 8) 
			| net2272_ep_read (ep, EP_AVAIL0));
#endif
	count = length;
	bufp = (u16 *)buf;
	
	while (likely (count >= 2)) {
		/* ep fifo data width and byte swap set during usb_reset */
		writew (*bufp++, ep_data);
		count -= 2;
	}
	buf = (u8 *)bufp;

	/* write final byte by placing the NET2272 into 8-bit mode */
	if (unlikely (count)) {
		tmp = net2272_read (ep->dev, LOCCTL);
		net2272_write (ep->dev, LOCCTL, tmp & ~(1 << DATA_WIDTH));
		writew (*buf, ep_data);
		net2272_write (ep->dev, LOCCTL, tmp);
	}
	return length;
}

static int kick_dma (struct net2272_ep *ep, struct net2272_request *req);

/* returns: 0: still running, 1: completed, negative: errno */
static int write_fifo (struct net2272_ep *ep, struct net2272_request *req)
{
	u8		*buf;
	unsigned	count, max;
	int		is_last = 0;
	int		status;

#if 0
        VDEBUG (ep->dev, "write_fifo %s actual %d len %d\n", ep->ep.name, req->req.actual, req->req.length);    
#endif	

        // Keep loading the endpoint until the final packet is loaded, or endpoint buffer full.
top: 
        // Clear interrupt status
        //  - Packet Transmitted interrupt will become set again when the host 
        //    successfully takes another packet
	net2272_ep_write (ep, EP_STAT0, (1 << DATA_PACKET_TRANSMITTED_INTERRUPT));    
	while (!(net2272_ep_read (ep, EP_STAT0) & (1 << BUFFER_FULL))) {
		buf = req->req.buf + req->req.actual;
		prefetch (buf);

		/* force pagesel */
		net2272_ep_read (ep, EP_STAT0);
		
		max = (net2272_ep_read (ep, EP_AVAIL1) << 8) |
			(net2272_ep_read (ep, EP_AVAIL0));

		if (max < ep->ep.maxpacket)
			max = (net2272_ep_read (ep, EP_AVAIL1) << 8) 
				| (net2272_ep_read (ep, EP_AVAIL0));

		count = write_packet (ep, buf, req, max);
                // see if we are done
                if (req->req.length == req->req.actual)
                {
                        /* validate short or zlp packet */
                        if (count < ep->ep.maxpacket)
                        {
        			set_fifo_bytecount (ep, 0);
                        }
			done (ep, req, 0);

			if (!list_empty (&ep->queue)) {
				req = list_entry (ep->queue.next,
						struct net2272_request,
						queue);
				status = kick_dma (ep, req);

				if (status < 0)
					if ((net2272_ep_read (ep, EP_STAT0) 
							& (1 << BUFFER_EMPTY)))
						goto top;	
			}
			return 1;
		}
                net2272_ep_write (ep, EP_STAT0, (1 << DATA_PACKET_TRANSMITTED_INTERRUPT));    
	}
	return 0;
}

static void out_flush (struct net2272_ep *ep)
{
	ASSERT_OUT_NAKING (ep);

	net2272_ep_write (ep, EP_STAT0, (1 << DATA_OUT_TOKEN_INTERRUPT)
			| (1 << DATA_PACKET_RECEIVED_INTERRUPT));
	net2272_ep_write (ep, EP_STAT1, 1 << BUFFER_FLUSH);
}

static int
read_packet (struct net2272_ep *ep, 
		u8 *buf, struct net2272_request *req, unsigned avail)
{
	u16			__iomem *ep_data = ep->dev->base_addr + EP_DATA;
	unsigned		is_short;
	u16			*bufp;
	
	req->req.actual += avail;
#if 0
	DEBUG (ep->dev, "read packet %s req %p len %u avail %u\n",
			ep->ep.name, req, avail, 
			(net2272_ep_read (ep, EP_AVAIL1) << 8) 
				| net2272_ep_read (ep, EP_AVAIL0));
#endif	
	is_short = (avail < ep->ep.maxpacket);

	if (unlikely (avail == 0)) {
		/* remove any zlp from the buffer */
		(void)readw (ep_data);
		return is_short;
	}
	
	/* Ensure we get the final byte */
	if (unlikely (avail % 2))
		avail++;
	bufp = (u16 *)buf;
	
	do {
		*bufp++ = readw (ep_data);
		avail -= 2;
	} while (avail);
	
	// To avoid false endpoint available race condition must read ep stat0 twice in the case
	// of a short transfer
	if (net2272_ep_read (ep, EP_STAT0) & (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT))
	{
		net2272_ep_read (ep, EP_STAT0);		
	}

	return is_short;
}

static int read_fifo (struct net2272_ep *ep, struct net2272_request *req)
{
	u8		*buf;
	unsigned	is_short;
	int		count;
	int		tmp;
	int		cleanup = 0;
	int		status = -1;

#if 0
	DEBUG (ep->dev, "read_fifo %s actual %d len %d\n",
			ep->ep.name, req->req.actual, req->req.length);
#endif	
top:
	do {
		buf = req->req.buf + req->req.actual;
		prefetchw (buf);

		count = (net2272_ep_read (ep, EP_AVAIL1) << 8)
			| net2272_ep_read (ep, EP_AVAIL0);
		
		net2272_ep_write (ep, EP_STAT0,
				(1 << SHORT_PACKET_TRANSFERRED_INTERRUPT)
			      | (1 << DATA_PACKET_RECEIVED_INTERRUPT));

		tmp = req->req.length - req->req.actual;

		if (count > tmp) {
			if ((tmp % ep->ep.maxpacket) != 0) {
				ERROR (ep->dev,
					"%s out fifo %d bytes, expected %d\n",
						ep->ep.name, count, tmp);
				cleanup = 1;
			}
			count = (tmp > 0) ? tmp : 0;
		}
		
		is_short = read_packet (ep, buf, req, count);

		/* completion */
		if (unlikely (cleanup || is_short || 
				((req->req.actual == req->req.length)
				 && !req->req.zero))) {

			if (cleanup) {
				out_flush (ep);
				done (ep, req, -EOVERFLOW);
			} else
				done (ep, req, 0);

			/* re-initialize endpoint transfer registers
			 * otherwise they may result in erroneous pre-validation
			 * for subsequent control reads
			 */
			if (unlikely (ep->num == 0)) {
				net2272_ep_write (ep, EP_TRANSFER2, 0);
				net2272_ep_write (ep, EP_TRANSFER1, 0);
				net2272_ep_write (ep, EP_TRANSFER0, 0);
			}

			if (!list_empty (&ep->queue)) {
				req = list_entry (ep->queue.next,
						struct net2272_request, queue);
				status = kick_dma (ep, req);
				if ((status < 0) && !net2272_ep_read (ep,
							EP_STAT0) &
						(1 << BUFFER_EMPTY))
					goto top;
			}
			return 1;	
		}
	} while (!(net2272_ep_read (ep, EP_STAT0) & (1 << BUFFER_EMPTY)));
	return 0;
}

static inline void
pio_advance (struct net2272_ep *ep)
{
	struct net2272_request	*req;

	if (unlikely (list_empty (&ep->queue)))
	{
		// Leave the data in the chip, don't handle until next queue request
		// The data could be a zero length packet!
		return;
	}
	req = list_entry (ep->queue.next, struct net2272_request, queue);
	(ep->is_in ? write_fifo : read_fifo)(ep, req);
}

/* returns 0 on success, else negative errno */
static inline int 
request_dma (struct net2272 *dev, unsigned ep, u32 buf, unsigned len, 
		unsigned dir)
{
	DEBUG (dev, "request_dma ep %d buf %08x len %d dir %d\n",
			ep, buf, len, dir);

	/* The NET2272 only supports a single dma channel */
	if (dev->dma_busy)
		return -EBUSY;
	/* EP_TRANSFER (used to determine the number of bytes received
	 * in an OUT transfer) is 24 bits wide; don't ask for more than that.
	 */
	if ((dir == 1) && (len > 0x1000000))
		return -EINVAL;

	dev->dma_busy = 1;
	
	/* initialize platform's dma */
	/* NET2272 addr, buffer addr, length, etc. */
#if defined (PLX_PCI_RDK)
	/* Setup PLX 9054 DMA mode */
	writel (  (1 << LOCAL_BUS_WIDTH)
		| (1 << TA_READY_INPUT_ENABLE)
		| (0 << LOCAL_BURST_ENABLE)
		| (1 << DONE_INTERRUPT_ENABLE)
		| (1 << LOCAL_ADDRESSING_MODE)
		| (1 << DEMAND_MODE)
		| (1 << DMA_EOT_ENABLE)
		| (1 << FAST_SLOW_TERMINATE_MODE_SELECT)
		| (1 << DMA_CHANNEL_INTERRUPT_SELECT),
		dev->plx9054_base_addr + DMAMODE0);

	writel (0x100000, dev->plx9054_base_addr + DMALADR0);
	writel (buf, dev->plx9054_base_addr + DMAPADR0);
	writel (len, dev->plx9054_base_addr + DMASIZ0);
	writel ((dir << DIRECTION_OF_TRANSFER)
			| (1 << INTERRUPT_AFTER_TERMINAL_COUNT),
			dev->plx9054_base_addr + DMADPR0);
	writel ((1 << LOCAL_DMA_CHANNEL_0_INTERRUPT_ENABLE) |
			readl (dev->plx9054_base_addr + INTCSR),
			dev->plx9054_base_addr + INTCSR);
#endif
	
	net2272_write (dev, DMAREQ,
		  (0 << DMA_BUFFER_VALID)
		| (1 << DMA_REQUEST_ENABLE)
		| (1 << DMA_CONTROL_DACK)
		| (dev->dma_eot_polarity << EOT_POLARITY)
		| (dev->dma_dack_polarity << DACK_POLARITY)
		| (dev->dma_dreq_polarity << DREQ_POLARITY)
		| ((ep >> 1) << DMA_ENDPOINT_SELECT));

#if defined(PLX_PCI_RDK)
	(void) net2272_read (dev, SCRATCH);
#endif
	
	return 0;
}

static inline void
start_dma (struct net2272 *dev)
{
	/* start platform's dma controller */
#if defined(PLX_PCI_RDK)
	writeb ((1 << CHANNEL_ENABLE) | (1 << CHANNEL_START),
			dev->plx9054_base_addr + DMACSR0);
#endif
}

/* returns 0 on success, else negative errno */
static int kick_dma (struct net2272_ep *ep, struct net2272_request *req)
{
	unsigned	size;
	u8		tmp;
	
	if (!use_dma || (ep->num < 1) || (ep->num > 2) || !ep->dma)
		return -EINVAL;

	/* don't use dma for odd-length transfers
	 * otherwise, we'd need to deal with the last byte with pio
	 */
	if (req->req.length & 1)
		return -EINVAL;

	DEBUG (ep->dev, "kick_dma %s req %p dma %08x\n",
			ep->ep.name, req, req->req.dma);

	net2272_ep_write (ep, EP_RSPSET, 1 << ALT_NAK_OUT_PACKETS);

	/* The NET2272 can only use DMA on one endpoint at a time */
	if (ep->dev->dma_busy)
		return -EBUSY;

	/* Make sure we only DMA an even number of bytes (we'll use
	 * pio to complete the transfer)
	 */
	size = req->req.length;
	size &= ~1;
	
	/* device-to-host transfer */
	if (ep->is_in) {
		/* initialize platform's dma controller */
		if ((request_dma (ep->dev, ep->num, req->req.dma, size, 0)) 
				< 0) {
			/* unable to obtain DMA channel; return error and use
			 * pio mode.
			 */
			return -EBUSY;
		}
		req->req.actual += size;

	/* host-to-device transfer */
	} else {
		tmp = net2272_ep_read (ep, EP_STAT0);

		/* initialize platform's dma controller */
		if ((request_dma (ep->dev, ep->num, req->req.dma, size, 1)) 
				< 0) {
			/* unable to obtain DMA channel; return error and use
			 * pio mode.
			 */
			return -EBUSY;
		}

		if (!(tmp & (1 << BUFFER_EMPTY)))
			ep->not_empty = 1;
		else
			ep->not_empty = 0;


		/* allow the endpoint's buffer to fill */
		net2272_ep_write (ep, EP_RSPCLR, 1 << ALT_NAK_OUT_PACKETS);

		/* this transfer completed and data's already in the fifo
		 * return error so pio gets used.
		 */
		if (tmp & (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT)) {

			/* deassert dreq */
			net2272_write (ep->dev, DMAREQ,
				  (0 << DMA_BUFFER_VALID)
				| (0 << DMA_REQUEST_ENABLE)
				| (1 << DMA_CONTROL_DACK)
				| (ep->dev->dma_eot_polarity << EOT_POLARITY)
				| (ep->dev->dma_dack_polarity << DACK_POLARITY)
				| (ep->dev->dma_dreq_polarity << DREQ_POLARITY)
				| ((ep->num >> 1) << DMA_ENDPOINT_SELECT));

			return -EBUSY;
		}
	}

	/* Don't use per-packet interrupts: use dma interrupts only */
	net2272_ep_write (ep, EP_IRQENB, 0);

	start_dma (ep->dev);

	return 0;
}

static inline void cancel_dma (struct net2272 *dev)
{
	/* put your platform specific dma cancel below */
#if defined(PLX_PCI_RDK)
	writeb (0, dev->plx9054_base_addr + DMACSR0);
	writeb (1 << CHANNEL_ABORT, dev->plx9054_base_addr + DMACSR0);
	while (!(readb (dev->plx9054_base_addr + DMACSR0) 
				& (1 << CHANNEL_DONE)))
		;	/* wait for dma to stabalize */

	/* dma abort generates an interrupt */
	writeb (1 << CHANNEL_CLEAR_INTERRUPT, dev->plx9054_base_addr + DMACSR0);
#endif
	dev->dma_busy = 0;
}

/*---------------------------------------------------------------------------*/

static int
net2272_queue (struct usb_ep *_ep, struct usb_request *_req, int gfp_flags)
{
	struct net2272_request	*req;
	struct net2272_ep	*ep;
	struct net2272		*dev;
	unsigned long		flags;
	int			status = -1;
	u8			s;
	
	req = container_of (_req, struct net2272_request, req);
	if (!_req || !_req->complete || !_req->buf
			|| !list_empty (&req->queue))
		return -EINVAL;
	ep = container_of (_ep, struct net2272_ep, ep);
	if (!_ep || (!ep->desc && ep->num != 0))
		return -EINVAL;
	dev = ep->dev;
	if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN)
		return -ESHUTDOWN;

	/* FIXME caller may already have done the dma mapping */
	/* UNAVAILABLE IN 2.4 */
	/* _req->dma = dma_map_single (dev->gadget.dev.parent,
			_req->buf, _req->length,
			ep->is_in ? DMA_FROM_DEVICE : DMA_TO_DEVICE); */
#if defined(PLX_PCI_RDK)
        _req->dma = pci_map_single(dev->pdev,
                                   _req->buf, _req->length,                                   
                                   ep->is_in ? PCI_DMA_FROMDEVICE : PCI_DMA_TODEVICE);
#endif
	
#if 1
	DEBUG (dev, "%s queue req %p, len %d buf %p dma %08x %s\n",
			_ep->name, _req, _req->length, _req->buf,
			_req->dma, _req->zero ? "zero" : "!zero");
	
	DEBUG (ep->dev, "%s ack ep_stat0 %02x, ep_stat1 %02x, req %p avail %u\n",
			ep->ep.name, net2272_ep_read (ep, EP_STAT0), net2272_ep_read (ep, EP_STAT1), req ? &req->req : 0,
		(net2272_ep_read (ep, EP_AVAIL1) << 8) | net2272_ep_read (ep, EP_AVAIL0)
	);
#endif

	spin_lock_irqsave (&dev->lock, flags);

	_req->status = -EINPROGRESS;
	_req->actual = 0;

	/* kickstart this i/o queue? */
	if (list_empty (&ep->queue) && !ep->stopped) {
		/* maybe there's no control data, just status ack */
		if (ep->num == 0 && _req->length == 0) {
			done (ep, req, 0);
			DEBUG (dev, "%s status ack\n", ep->ep.name);
			goto done;
		}

		// Return zlp, don't let it block subsequent packets
		s = net2272_ep_read (ep, EP_STAT0);
		if (s & (1 << BUFFER_EMPTY)) 
		{
			// Buffer is empty check for a blocking zlp, handle it 
			if ((s & (1 << NAK_OUT_PACKETS)) && net2272_ep_read (ep, EP_STAT1) & (1 << LOCAL_OUT_ZLP))
			{
				DEBUG(dev, "WARNING: returning ZLP short packet termination!\n");
				// Request is going to terminate with a short packet
				// hope client is ready for it!
				status = read_fifo (ep, req);	
				// clear short packet naking
				net2272_ep_write (ep, EP_STAT0, (1 << NAK_OUT_PACKETS));
				goto done;
			}
		}		
		
		/* try dma first */
		status = kick_dma (ep, req);

		if (status < 0) {
			/* dma failed (most likely in use by another endpoint) 
			 * fallback to pio
			 */
			status = 0;
			
			if (ep->is_in) 
				status = write_fifo (ep, req);
			else {
				s = net2272_ep_read (ep, EP_STAT0);
				if ((s & (1 << BUFFER_EMPTY)) == 0) 
					status = read_fifo (ep, req);
			}

			if (unlikely (status != 0)) {
				if (status > 0)
					status = 0;
				req = NULL;
			}
		}
	}
	if (likely (req != 0))
		list_add_tail (&req->queue, &ep->queue);

	if (likely (!list_empty (&ep->queue)))
		net2272_ep_write (ep, EP_RSPCLR, 1 << ALT_NAK_OUT_PACKETS);
done:
	spin_unlock_irqrestore (&dev->lock, flags);

	return 0;
}

/* dequeue ALL requests */
static void nuke (struct net2272_ep *ep)
{
	struct net2272_request	*req;

	/* called with spinlock held */
	ep->stopped = 1;

	while (!list_empty (&ep->queue)) {
		req = list_entry (ep->queue.next,
				struct net2272_request,
				queue);
		done (ep, req, -ESHUTDOWN);
	}
}

/* dequeue JUST ONE request */
static int net2272_dequeue (struct usb_ep *_ep, struct usb_request *_req)
{
	struct net2272_ep		*ep;
	struct net2272_request		*req;
	unsigned long			flags;
	int				stopped;

	ep = container_of (_ep, struct net2272_ep, ep);
	if (!_ep || (!ep->desc && ep->num != 0) || !_req)
		return -EINVAL;

	spin_lock_irqsave (&ep->dev->lock, flags);
	stopped = ep->stopped;
	ep->stopped = 1;

	/* make sure it's still queued on this endpoint */
	list_for_each_entry (req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	if (&req->req != _req) {
		spin_unlock_irqrestore (&ep->dev->lock, flags);
		return -EINVAL;
	}

	/* queue head may be partially complete */
	if (ep->queue.next == &req->queue) {
		DEBUG (ep->dev, "unlink (%s) pio\n", _ep->name);
		done (ep, req, -ECONNRESET);
	}
	req = NULL;
	ep->stopped = stopped;

	spin_unlock_irqrestore (&ep->dev->lock, flags);
	return 0;
}

/*---------------------------------------------------------------------------*/

static int net2272_fifo_status (struct usb_ep *_ep);

static int
net2272_set_halt (struct usb_ep *_ep, int value)
{
	struct net2272_ep	*ep;
	unsigned long		flags;
	int			retval = 0;

	ep = container_of (_ep, struct net2272_ep, ep);
	if (!_ep || (!ep->desc && ep->num != 0))
		return -EINVAL;
	if (!ep->dev->driver || ep->dev->gadget.speed == USB_SPEED_UNKNOWN)
		return -ESHUTDOWN;
	if (ep->desc /* not ep0 */ && (ep->desc->bmAttributes & 0x03)
			== USB_ENDPOINT_XFER_ISOC)
		return -EINVAL;

	spin_lock_irqsave (&ep->dev->lock, flags);
	if (!list_empty (&ep->queue))
		retval = -EAGAIN;
	else if (ep->is_in && value && net2272_fifo_status (_ep) != 0)
		retval = -EAGAIN;
	else {
		VDEBUG (ep->dev, "%s %s halt\n", _ep->name,
				value ? "set" : "clear");
		/* set/clear */
		if (value) {
			if (ep->num == 0)
				ep->dev->protocol_stall = 1;
			else
				set_halt (ep);
		}
		else
			clear_halt (ep);
	}
	spin_unlock_irqrestore (&ep->dev->lock, flags);

	return retval;
}

static int
net2272_fifo_status (struct usb_ep *_ep)
{
	struct net2272_ep	*ep;
	u16			avail;

	ep = container_of (_ep, struct net2272_ep, ep);
	if (!_ep || (!ep->desc && ep->num != 0))
		return -ENODEV;
	if (!ep->dev->driver || ep->dev->gadget.speed == USB_SPEED_UNKNOWN)
		return -ESHUTDOWN;

	avail = net2272_ep_read (ep, EP_AVAIL1) << 8;
	avail |= net2272_ep_read (ep, EP_AVAIL0);
	if (avail > ep->fifo_size)
		return -EOVERFLOW;
	if (ep->is_in)
		avail = ep->fifo_size - avail;
	return avail;
}

static void
net2272_fifo_flush (struct usb_ep *_ep)
{
	struct net2272_ep	*ep;

	ep = container_of (_ep, struct net2272_ep, ep);
	if (!_ep || (!ep->desc && ep->num != 0))
		return;
	if (!ep->dev->driver || ep->dev->gadget.speed == USB_SPEED_UNKNOWN)
		return;

	net2272_ep_write (ep, EP_STAT1, 1 << BUFFER_FLUSH);
}

static struct usb_ep_ops net2272_ep_ops = {
	.enable		= net2272_enable,
	.disable	= net2272_disable,
	
	.alloc_request	= net2272_alloc_request,
	.free_request	= net2272_free_request,

	.alloc_buffer	= net2272_alloc_buffer,
	.free_buffer	= net2272_free_buffer,

	.queue		= net2272_queue,
	.dequeue	= net2272_dequeue,

	.set_halt	= net2272_set_halt,
	.fifo_status	= net2272_fifo_status,
	.fifo_flush	= net2272_fifo_flush,
};

/*---------------------------------------------------------------------------*/

static int net2272_get_frame (struct usb_gadget *_gadget)
{
	struct net2272		*dev;
	unsigned long		flags;
	u16			retval;

	if (!_gadget)
		return -ENODEV;
	dev = container_of (_gadget, struct net2272, gadget);
	spin_lock_irqsave (&dev->lock, flags);
	
	retval = net2272_read (dev, FRAME1) << 8;
	retval |= net2272_read (dev, FRAME0);
	
	spin_unlock_irqrestore (&dev->lock, flags);
	return retval;
}

static int net2272_wakeup (struct usb_gadget *_gadget)
{
	struct net2272		*dev;
	u8			tmp;
	unsigned long		flags;

	if (!_gadget)
		return 0;
	dev = container_of (_gadget, struct net2272, gadget);

	spin_lock_irqsave (&dev->lock, flags);
	tmp = net2272_read (dev, USBCTL0);
	if (tmp & (1 << IO_WAKEUP_ENABLE))
		net2272_write (dev, USBCTL1, (1 << GENERATE_RESUME));

	spin_unlock_irqrestore (&dev->lock, flags);
        DEBUG (dev, "Wakeup %08x\n", tmp);
    

	return 0;
}

static int net2272_set_selfpowered (struct usb_gadget *_gadget, int value)
{
	struct net2272		*dev;

	if (!_gadget)
		return -ENODEV;
	dev = container_of (_gadget, struct net2272, gadget);

	dev->is_selfpowered = value;

	return 0;
}

static int net2272_pullup (struct usb_gadget *_gadget, int is_on)
{
	struct net2272		*dev;
	u8			tmp;
	unsigned long		flags;

	if (!_gadget)
		return -ENODEV;
	dev = container_of (_gadget, struct net2272, gadget);

	spin_lock_irqsave (&dev->lock, flags);
	tmp = net2272_read (dev, USBCTL0);
	dev->softconnect = (is_on != 0);
	if (is_on)
		tmp |= (1 << USB_DETECT_ENABLE);
	else
		tmp &= ~(1 << USB_DETECT_ENABLE);
	net2272_write (dev, USBCTL0, tmp);
	spin_unlock_irqrestore (&dev->lock, flags);
        DEBUG (dev, "Pullup %08x \n", tmp); 
	return 0;
}

static const struct usb_gadget_ops net2272_ops = {
	.get_frame		= net2272_get_frame,
	.wakeup			= net2272_wakeup,
	.set_selfpowered	= net2272_set_selfpowered,
};

/*---------------------------------------------------------------------------*/

static void set_fifo_mode (struct net2272 *dev, int mode)
{
	u8	tmp;

	tmp = net2272_read (dev, LOCCTL) & 0x3f;
	tmp |= (mode << 6);
	net2272_write (dev, LOCCTL, tmp);

	INIT_LIST_HEAD (&dev->gadget.ep_list);
	
	/* always ep-a, ep-c ... maybe not ep-b */
	list_add_tail (&dev->ep [1].ep.ep_list, &dev->gadget.ep_list);
	
	switch (mode) {
	case 0:
		list_add_tail (&dev->ep [2].ep.ep_list, &dev->gadget.ep_list);
		dev->ep [1].fifo_size = dev->ep [2].fifo_size = 512;
		break;
	case 1:
		list_add_tail (&dev->ep [2].ep.ep_list, &dev->gadget.ep_list);
		dev->ep [1].fifo_size = 1024;
		dev->ep [2].fifo_size = 512;
		break;
	case 2:
		list_add_tail (&dev->ep [2].ep.ep_list, &dev->gadget.ep_list);
		dev->ep [1].fifo_size = dev->ep [2].fifo_size = 1024;
		break;
	case 3:
		dev->ep [1].fifo_size = 1024;
		break;
	}

	/* ep-c is always 2 512 byte buffers */
	list_add_tail (&dev->ep [3].ep.ep_list, &dev->gadget.ep_list);
	dev->ep [3].fifo_size = 512;
}

/*---------------------------------------------------------------------------*/

static struct net2272	*the_controller;

static void usb_reset (struct net2272 *dev)
{
	dev->gadget.speed = USB_SPEED_UNKNOWN;
	cancel_dma (dev);	
	net2272_write (dev, IRQENB0, 0);
	net2272_write (dev, IRQENB1, 0);

	/* clear irq state */
	net2272_write (dev, IRQSTAT0, 0xff);
	net2272_write (dev, IRQSTAT1, ~(1 << SUSPEND_REQUEST_INTERRUPT));
	
	net2272_write (dev, DMAREQ, 
		  (0 << DMA_BUFFER_VALID)
		| (0 << DMA_REQUEST_ENABLE)
		| (1 << DMA_CONTROL_DACK)
		| (dev->dma_eot_polarity << EOT_POLARITY)
		| (dev->dma_dack_polarity << DACK_POLARITY)
		| (dev->dma_dreq_polarity << DREQ_POLARITY)
		| ((dma_ep >> 1) << DMA_ENDPOINT_SELECT));
	
#if defined(PLX_PCI_RDK)
	/* disable split dma bus mode */
	*((u8 *)dev->base_addr + EPLD_DMA_CONTROL_REGISTER) = 
		(dma_mode << EPLD_DMA_MODE);
#endif
	set_fifo_mode (dev, (fifo_mode <= 3) ? fifo_mode : 0);
	
	/* Set the NET2272 ep fifo data width to 16-bit mode and for correct byte swapping 
	 * note that the higher level gadget drivers are expected to convert data to little endian.
	 * Enable byte swap for your local bus/cpu if needed by setting BYTE_SWAP in LOCCTL here
	 */
	net2272_write (dev, LOCCTL, net2272_read (dev, LOCCTL) | (1 << DATA_WIDTH));
	net2272_write (dev, LOCCTL1, (dma_mode << DMA_MODE));
}

static void usb_reinit (struct net2272 *dev)
{
	int	i;

	/* basic endpoint init */
	for (i = 0; i < 4; i++) {
		struct net2272_ep	*ep = &dev->ep [i];

		ep->ep.name = ep_name [i];
		ep->dev = dev;
		ep->num = i;
		ep->not_empty = 0;

		if (ep->num == dma_ep)
			ep->dma = 1;

		if (i > 0 && i <= 3)
			ep->fifo_size = 512;
		else
			ep->fifo_size = 64;
		ep_reset (ep);
	}
	dev->ep [0].ep.maxpacket = 64;

	dev->gadget.ep0 = &dev->ep [0].ep;
	dev->ep [0].stopped = 0;
	INIT_LIST_HEAD (&dev->gadget.ep0->ep_list);
}

static void ep0_start (struct net2272 *dev)
{
	struct net2272_ep	*ep0 = &dev->ep [0];
        unsigned char tmp;
    
	net2272_ep_write (ep0, EP_RSPSET,
			  (1 << NAK_OUT_PACKETS_MODE)
			| (1 << ALT_NAK_OUT_PACKETS));
	net2272_ep_write (ep0, EP_RSPCLR, 
			  (1 << HIDE_STATUS_PHASE)
			| (1 << CONTROL_STATUS_PHASE_HANDSHAKE));
	net2272_write (dev, USBCTL0, (dev->softconnect << USB_DETECT_ENABLE)
			| (1 << USB_ROOT_PORT_WAKEUP_ENABLE)
			| (1 << IO_WAKEUP_ENABLE));
	net2272_write (dev, IRQENB0, 
			  (1 << SETUP_PACKET_INTERRUPT_ENABLE)
			| (1 << ENDPOINT_0_INTERRUPT_ENABLE)
			| (1 << DMA_DONE_INTERRUPT_ENABLE));
	net2272_write (dev, IRQENB1, 
			  (1 << VBUS_INTERRUPT_ENABLE)
                       | (1 << ROOT_PORT_RESET_INTERRUPT_ENABLE)
		       | (1 << SUSPEND_REQUEST_CHANGE_INTERRUPT_ENABLE));
}

/* when a driver is successfully registered, it will receive
 * control requests including set_configuration(), which enables
 * non-control requests.  then usb traffic follows until a
 * disconnect is reported.  then a host may connect again, or
 * the driver might get unbound.
 */
int usb_gadget_register_driver (struct usb_gadget_driver *driver)
{
	struct net2272		*dev = the_controller;
	int			retval;
	unsigned		i;

	if (!driver
			|| driver->speed != USB_SPEED_HIGH
			|| !driver->bind
			|| !driver->unbind
			|| !driver->setup)
		return -EINVAL;
	if (!dev)
		return -ENODEV;
	if (dev->driver)
		return -EBUSY;
	for (i = 0; i < 4; i++)
		dev->ep [i].irqs = 0;
	/* hook up the driver ... */
        dev->softconnect = 1;
	dev->driver = driver;
	retval = driver->bind (&dev->gadget);
	if (retval) {
		DEBUG (dev, "bind to driver %s --> %d\n",
				driver->driver.name, retval);
		dev->driver = NULL;
		return retval;
	}

	/* ... then enable host detection and ep0; and we're ready
	 * for set_configuration as well as eventual disconnect.
	 */
	ep0_start (dev);
        
	DEBUG (dev, "%s ready\n", driver->driver.name);

	return 0;
}
EXPORT_SYMBOL (usb_gadget_register_driver);

static void
stop_activity (struct net2272 *dev, struct usb_gadget_driver *driver)
{
	int		i;

	/* don't disconnect if it's not connected */
	if (dev->gadget.speed == USB_SPEED_UNKNOWN)
		driver = NULL;

	/* stop hardware; prevent new request submissions;
	 * and kill any outstanding requests.
	 */
	usb_reset (dev);
	for (i = 0; i < 4; i++)
		nuke (&dev->ep [i]);

	/* report disconnect; the driver is already quiesced */
	if (driver) {
		spin_unlock (&dev->lock);
		driver->disconnect (&dev->gadget);
		spin_lock (&dev->lock);
	
	}
	usb_reinit (dev);
}

int usb_gadget_unregister_driver (struct usb_gadget_driver *driver)
{
	struct net2272	*dev = the_controller;
	unsigned long	flags;

	if (!dev)
		return -ENODEV;
	if (!driver || driver != dev->driver)
		return -EINVAL;

	spin_lock_irqsave (&dev->lock, flags);
	stop_activity (dev, driver);
	spin_unlock_irqrestore (&dev->lock, flags);

	net2272_pullup (&dev->gadget, 0);

	driver->unbind (&dev->gadget);
	dev->driver = NULL;

	DEBUG (dev, "unregistered driver '%s'\n", driver->driver.name);
	return 0;
}
EXPORT_SYMBOL (usb_gadget_unregister_driver);

/*---------------------------------------------------------------------------*/
/* handle ep-a/ep-b dma completions */
static void handle_dma (struct net2272_ep *ep)
{
	struct net2272_request	*req;
	unsigned		len;
	int			status;

	if (!list_empty (&ep->queue))
		req = list_entry (ep->queue.next,
				struct net2272_request, queue);
	else
	{	
		req = NULL;
		WARN (ep->dev, "ep queue empty ignoring completion interrupt\n");
		return;
	}
	
	/* Ensure DREQ is de-asserted */
	net2272_write (ep->dev, DMAREQ,
		(0 << DMA_BUFFER_VALID)
	      | (0 << DMA_REQUEST_ENABLE)
	      | (1 << DMA_CONTROL_DACK)
	      | (ep->dev->dma_eot_polarity << EOT_POLARITY)
	      | (ep->dev->dma_dack_polarity << DACK_POLARITY)
	      | (ep->dev->dma_dreq_polarity << DREQ_POLARITY)
	      | ((ep->dma >> 1) << DMA_ENDPOINT_SELECT));

	VDEBUG (ep->dev, "handle_dma %s req %p avail %08X\n", ep->ep.name, req,
		(net2272_ep_read (ep, EP_AVAIL1) << 8) | net2272_ep_read (ep, EP_AVAIL0));

	ep->dev->dma_busy = 0;

	net2272_ep_write (ep, EP_IRQENB, 
		  (1 << DATA_PACKET_RECEIVED_INTERRUPT_ENABLE)
		| (1 << DATA_PACKET_TRANSMITTED_INTERRUPT_ENABLE)
		| net2272_ep_read (ep, EP_IRQENB));
	
	/* device-to-host transfer completed */
	if (ep->is_in) {
		/* validate a short packet or zlp if necessary */
		if ((req->req.length % ep->ep.maxpacket != 0) ||
				req->req.zero)
		{
			set_fifo_bytecount (ep, 0);
		}

		done (ep, req, 0);
		if (!list_empty (&ep->queue)) {
			req = list_entry (ep->queue.next, 
					struct net2272_request, queue);
			status = kick_dma (ep, req);
			if (status < 0)
				pio_advance (ep);
		}

	/* host-to-device transfer completed */
	} else {
		/* terminated with a short packet? */
		if (net2272_read (ep->dev, IRQSTAT0) & 
				(1 << DMA_DONE_INTERRUPT)) {
			/* abort system dma */
			cancel_dma (ep->dev);
		}
		
		/* EP_TRANSFER will contain the number of bytes
		 * actually received.
		 * NOTE: There is no overflow detection on EP_TRANSFER:
		 * We can't deal with transfers larger than 2^24 bytes!
		 */
		len = (net2272_ep_read (ep, EP_TRANSFER2) << 16)
			| (net2272_ep_read (ep, EP_TRANSFER1) << 8)
			| (net2272_ep_read (ep, EP_TRANSFER0));

		if (ep->not_empty)
			len += 4;

		req->req.actual += len;

		/* get any remaining data */
		pio_advance (ep);
	}
}

/*---------------------------------------------------------------------------*/

static void handle_ep (struct net2272_ep *ep)
{
	struct net2272_request	*req;
	u8			stat0, stat1;

	if (!list_empty (&ep->queue))
		req = list_entry (ep->queue.next,
			struct net2272_request, queue);
	else
		req = NULL;

	/* ack all, and handle what we care about */
	stat0 = net2272_ep_read (ep, EP_STAT0);
	stat1 = net2272_ep_read (ep, EP_STAT1);
	ep->irqs++;
#if 0
	DEBUG (ep->dev, "handle ep %s ack ep_stat0 %02x, ep_stat1 %02x, req %p avail %u\n",
			ep->ep.name, stat0, stat1, req ? &req->req : 0,
		(net2272_ep_read (ep, EP_AVAIL1) << 8) | net2272_ep_read (ep, EP_AVAIL0)
	);
#endif
	net2272_ep_write (ep, EP_STAT0, stat0 & 
			~((1 << NAK_OUT_PACKETS) 
			| (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT)));
	net2272_ep_write (ep, EP_STAT1, stat1);
		

	/* data packet(s) received (in the fifo, OUT) 
	 * direction must be validated, otherwise control read status phase
	 * could be interpreted as a valid packet
	 */
	if (!ep->is_in && (stat0 & (1 << DATA_PACKET_RECEIVED_INTERRUPT)))
		pio_advance (ep);
	/* data packet(s) transmitted (IN) */
	else if (stat0 & (1 << DATA_PACKET_TRANSMITTED_INTERRUPT))
		pio_advance (ep);
}

static struct net2272_ep *
get_ep_by_addr (struct net2272 *dev, u16 wIndex)
{
	struct net2272_ep	*ep;

	if ((wIndex & USB_ENDPOINT_NUMBER_MASK) == 0)
		return &dev->ep [0];
	
	list_for_each_entry (ep, &dev->gadget.ep_list, ep.ep_list) {
		u8	bEndpointAddress;

		if (!ep->desc)
			continue;
		bEndpointAddress = ep->desc->bEndpointAddress;
		if ((wIndex ^ bEndpointAddress) & USB_DIR_IN)
			continue;
		if ((wIndex & 0x0f) == (bEndpointAddress & 0x0f))
			return ep;
	}
	return NULL;
}

/* USB Test Packet:
 * JKJKJKJK * 9
 * JJKKJJKK * 8
 * JJJJKKKK * 8
 * JJJJJJJKKKKKKK * 8
 * JJJJJJJK * 8
 * {JKKKKKKK * 10}, JK
 */
u8 test_packet [] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
	0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD,
	0xFC, 0x7E, 0xBF, 0xDF, 0xEF, 0xF7, 0xFD, 0x7E
};

static void set_test_mode (struct net2272 *dev, int mode)
{
	int		i;

	/* Disable all net2272 interrupts:
	 * Nothing but a power cycle should stop the test.
	 */
	net2272_write (dev, IRQENB0, 0x00);
	net2272_write (dev, IRQENB1, 0x00);

	/* Force tranceiver to high-speed */
	net2272_write (dev, XCVRDIAG, 1 << FORCE_HIGH_SPEED);

	net2272_write (dev, PAGESEL, 0);
	net2272_write (dev, EP_STAT0, 1 << DATA_PACKET_TRANSMITTED_INTERRUPT);
	net2272_write (dev, EP_RSPCLR, 
			  (1 << CONTROL_STATUS_PHASE_HANDSHAKE)
			| (1 << HIDE_STATUS_PHASE));
	net2272_write (dev, EP_CFG, 1 << ENDPOINT_DIRECTION);
	net2272_write (dev, EP_STAT1, 1 << BUFFER_FLUSH);

	/* wait for status phase to complete */
	while (!(net2272_read (dev, EP_STAT0) & 
				(1 << DATA_PACKET_TRANSMITTED_INTERRUPT)))
		;	

	/* Enable test mode */
	net2272_write (dev, USBTEST, mode);

	/* load test packet */
	if (mode == TEST_PACKET) {
		/* switch to 8 bit mode */
		net2272_write (dev, LOCCTL, net2272_read (dev, LOCCTL) &
				~(1 << DATA_WIDTH));

		for (i = 0; i < sizeof test_packet; i++)
			net2272_write (dev, EP_DATA, test_packet [i]);

		/* Validate test packet */
		net2272_write (dev, EP_TRANSFER0, 0);
	}
}

static void handle_stat0_irqs (struct net2272 *dev, u8 stat)
{
	struct net2272_ep       *ep;
	u8			num, scratch;

	/* starting a control request? */
	if (unlikely (stat & (1 << SETUP_PACKET_INTERRUPT))) {
		union {
			u8			raw [8];
			struct usb_ctrlrequest	r;
		} u;
		int				tmp = 0;
		struct net2272_request	*req;

		if (dev->gadget.speed == USB_SPEED_UNKNOWN) {
			if (net2272_read (dev, USBCTL1) & (1 << USB_HIGH_SPEED))
				dev->gadget.speed = USB_SPEED_HIGH;
			else
				dev->gadget.speed = USB_SPEED_FULL;
			DEBUG (dev, "%s speed\n",
					(dev->gadget.speed == USB_SPEED_HIGH)
					? "high" : "full");
		}

		ep = &dev->ep [0];
		ep->irqs++;

		/* make sure any leftover interrupt state is cleared */
		stat &= ~(1 << ENDPOINT_0_INTERRUPT);
		while (!list_empty (&ep->queue)) {
			req = list_entry (ep->queue.next,
					struct net2272_request, queue);
			done (ep, req, (req->req.actual == req->req.length)
					? 0 : -EPROTO);
		}
		ep->stopped = 0;
		dev->protocol_stall = 0;
		net2272_ep_write (ep, EP_STAT0,
			    (1 << DATA_IN_TOKEN_INTERRUPT)
			  | (1 << DATA_OUT_TOKEN_INTERRUPT)
			  | (1 << DATA_PACKET_TRANSMITTED_INTERRUPT)
			  | (1 << DATA_PACKET_RECEIVED_INTERRUPT)
			  | (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT));
		net2272_ep_write (ep, EP_STAT1,
			    (1 << TIMEOUT)
			  | (1 << USB_OUT_ACK_SENT)
			  | (1 << USB_OUT_NAK_SENT)
			  | (1 << USB_IN_ACK_RCVD)
			  | (1 << USB_IN_NAK_SENT)
			  | (1 << USB_STALL_SENT)
			  | (1 << LOCAL_OUT_ZLP));

		// Ensure Control Read pre-validation setting is beyond maximum size
		//  - Control Writes can leave non-zero values in EP_TRANSFER. If
		//    an EP0 transfer following the Control Write is a Control Read, 
		//    the NET2272 sees the non-zero EP_TRANSFER as an unexpected 
		//    pre-validation count. 
		//  - Setting EP_TRANSFER beyond the maximum EP0 transfer size ensures
		//    the pre-validation count cannot cause an unexpected validatation
		net2272_write (dev, PAGESEL, 0);
		net2272_write (dev, EP_TRANSFER2, 0xff);
		net2272_write (dev, EP_TRANSFER1, 0xff);
		net2272_write (dev, EP_TRANSFER0, 0xff);
		
		u.raw [0] = net2272_read (dev, SETUP0);
		u.raw [1] = net2272_read (dev, SETUP1);
		u.raw [2] = net2272_read (dev, SETUP2);
		u.raw [3] = net2272_read (dev, SETUP3);
		u.raw [4] = net2272_read (dev, SETUP4);
		u.raw [5] = net2272_read (dev, SETUP5);
		u.raw [6] = net2272_read (dev, SETUP6);
		u.raw [7] = net2272_read (dev, SETUP7);
                // If you have a big endian cpu make sure le16_to_cpus
                // performs the proper byte swapping here...
		le16_to_cpus (&u.r.wValue);
		le16_to_cpus (&u.r.wIndex);
		le16_to_cpus (&u.r.wLength);
		/* ack the irq */
		net2272_write (dev, IRQSTAT0, 1 << SETUP_PACKET_INTERRUPT);
		stat ^= (1 << SETUP_PACKET_INTERRUPT);

		/* watch control traffic at the token level, and force
		 * synchronization before letting the status phase happen.
		 */
		ep->is_in = (u.r.bRequestType & USB_DIR_IN) != 0;
		if (ep->is_in) {
			scratch = (1 << DATA_PACKET_TRANSMITTED_INTERRUPT_ENABLE)
				| (1 << DATA_OUT_TOKEN_INTERRUPT_ENABLE)
				| (1 << DATA_IN_TOKEN_INTERRUPT_ENABLE);
			stop_out_naking (ep);
		} else
			scratch = (1 << DATA_PACKET_RECEIVED_INTERRUPT_ENABLE)
				| (1 << DATA_OUT_TOKEN_INTERRUPT_ENABLE)
				| (1 << DATA_IN_TOKEN_INTERRUPT_ENABLE);
		net2272_ep_write (ep, EP_IRQENB, scratch);

		if ((u.r.bRequestType & USB_TYPE_MASK) != USB_TYPE_STANDARD)
			goto delegate;
		switch (u.r.bRequest) {
		case USB_REQ_GET_STATUS: {
			struct net2272_ep	*e;
			u16			status = 0;

			if ((u.r.bRequestType & USB_RECIP_MASK) 
					== USB_RECIP_ENDPOINT) {
				if ((e = get_ep_by_addr (dev, u.r.wIndex)) == 0
						|| u.r.wLength > 2)
					goto do_stall;
				if (net2272_ep_read (e, EP_RSPSET)
						& (1 << ENDPOINT_HALT))
					status = __constant_cpu_to_le16 (1);
				else
					status = __constant_cpu_to_le16 (0);

				/* don't bother with a request object! */
				net2272_ep_write (&dev->ep [0],
						EP_IRQENB, 0);
				writew (status, dev->base_addr + EP_DATA);
				set_fifo_bytecount (&dev->ep [0], 0);
				allow_status (ep);
				VDEBUG (dev, "%s stat %02x\n", ep->ep.name,
						status);
				goto next_endpoints;
			} else if ((u.r.bRequestType & USB_RECIP_MASK)
						== USB_RECIP_DEVICE) {
				if (u.r.wLength > 2)
					goto do_stall;
				if (dev->is_selfpowered)
					status = (1 << USB_DEVICE_SELF_POWERED);

				/* don't bother with a request object! */
				net2272_ep_write (&dev->ep [0],
						EP_IRQENB, 0);
				writew (status, dev->base_addr + EP_DATA);
				set_fifo_bytecount (&dev->ep [0], 0);
				allow_status (ep);
				VDEBUG (dev, "device stat %02x\n",
						status);
				goto next_endpoints;
			} else if ((u.r.bRequestType & USB_RECIP_MASK)
						== USB_RECIP_INTERFACE) {
				if (u.r.wLength > 2)
					goto do_stall;

				/* don't bother with a request object! */
				net2272_ep_write (&dev->ep [0],
						EP_IRQENB, 0);
				writew (status, dev->base_addr + EP_DATA);
				set_fifo_bytecount (&dev->ep [0], 0);
				allow_status (ep);
				VDEBUG (dev, "interface status %02x\n",
						status);
				goto next_endpoints;
			}
		}
		case USB_REQ_CLEAR_FEATURE: {
			struct net2272_ep	*e;

			if (u.r.bRequestType != USB_RECIP_ENDPOINT)
				goto delegate;
			if (u.r.wValue != USB_ENDPOINT_HALT
					|| u.r.wLength != 0)
				goto do_stall;
			if ((e = get_ep_by_addr (dev, u.r.wIndex)) == 0)
				goto do_stall;
			clear_halt (e);
			allow_status (ep);
			VDEBUG (dev, "%s clear halt\n", ep->ep.name);	
			goto next_endpoints;
			}
		case USB_REQ_SET_FEATURE: {
			struct net2272_ep	*e;
			
			if (u.r.bRequestType == USB_RECIP_DEVICE) {
				if (u.r.wIndex != NORMAL_OPERATION)
					set_test_mode (dev, (u.r.wIndex >> 8));
				allow_status (ep);
				VDEBUG (dev, "test mode: %d\n", (u.r.wIndex >> 8));
				goto next_endpoints;
			} else if (u.r.bRequestType != USB_RECIP_ENDPOINT)
				goto delegate;
			if (u.r.wValue != USB_ENDPOINT_HALT
					|| u.r.wLength != 0)
				goto do_stall;
			if ((e = get_ep_by_addr (dev, u.r.wIndex)) == 0)
				goto do_stall;
			set_halt (e);
			allow_status (ep);
			VDEBUG (dev, "%s set halt\n", ep->ep.name);
			goto next_endpoints;
			}
		case USB_REQ_SET_ADDRESS: {
			net2272_write (dev, OURADDR, u.r.wValue & 0xff);
			allow_status (ep);
			break;
			}
		default:
delegate:
			VDEBUG (dev, "setup %02x.%02x v%04x i%04x "
				"ep_cfg %08x\n",
				u.r.bRequestType, u.r.bRequest,
				u.r.wValue, u.r.wIndex,
				net2272_ep_read (ep, EP_CFG));
			spin_unlock (&dev->lock);
			tmp = dev->driver->setup (&dev->gadget, &u.r);
			spin_lock (&dev->lock);
		}

		/* stall ep0 on error */
		if (tmp < 0) {
do_stall:
			VDEBUG (dev, "req %02x.%02x protocol STALL; stat %d\n",
					u.r.bRequestType, u.r.bRequest, tmp);
			dev->protocol_stall = 1;
		}
	/* endpoint dma irq? */
	} else if (stat & (1 << DMA_DONE_INTERRUPT)) {
		cancel_dma (dev);
		net2272_write (dev, IRQSTAT0, 1 << DMA_DONE_INTERRUPT);
		stat &= ~(1 << DMA_DONE_INTERRUPT);
		num = (net2272_read (dev, DMAREQ) & (1 << DMA_ENDPOINT_SELECT))
			? 2 : 1;

		ep = &dev->ep [num];
		handle_dma (ep);
	}

next_endpoints:
	/* endpoint data irq? */
	scratch = stat & 0x0f;
	stat &= ~0x0f;
	for (num = 0; scratch; num++) {
		u8	t;

		/* does this endpoint's FIFO and queue need tending? */
		t = 1 << num;
		if ((scratch & t) == 0)
			continue;
		scratch ^= t;

		ep = &dev->ep [num];
		handle_ep (ep);
	}
	
	/* some interrupts we can just ignore */
	stat &= ~(1 << SOF_INTERRUPT);

	if (stat)
		DEBUG (dev, "unhandled irqstat0 %02x\n", stat);
}

static void handle_stat1_irqs (struct net2272 *dev, u8 stat)
{
	u8			tmp, mask;

	/* after disconnect there's nothing else to do! */
	tmp = (1 << VBUS_INTERRUPT) | (1 << ROOT_PORT_RESET_INTERRUPT);
	mask = (1 << USB_HIGH_SPEED) | (1 << USB_FULL_SPEED);

	if (stat & tmp) {
		net2272_write (dev, IRQSTAT1, tmp);
		if ((((stat & (1 << ROOT_PORT_RESET_INTERRUPT)) &&
				(( net2272_read (dev, USBCTL1) & mask) == 0))
			|| ((net2272_read (dev, USBCTL1) & (1 << VBUS_PIN)) 
				== 0)) 
				&& (dev->gadget.speed != USB_SPEED_UNKNOWN)) {
			DEBUG (dev, "disconnect %s\n",
				dev->driver->driver.name);
			stop_activity (dev, dev->driver);
                        // ep0_start will handle disabling pull up for D+ on vbus false as per spec
			ep0_start (dev);
			return;
		}
		stat &= ~tmp;

		if (!stat)
			return;
	}

	tmp = (1 << SUSPEND_REQUEST_CHANGE_INTERRUPT);
	if (stat & tmp) {
                DEBUG (dev, "Suspend change irq \n");
		net2272_write (dev, IRQSTAT1, tmp);
		if (stat & (1 << SUSPEND_REQUEST_INTERRUPT)) {
			if (dev->driver->suspend)
                        {
                            DEBUG (dev, "Suspend driver \n");
                            dev->driver->suspend (&dev->gadget);
                        }
			if (!enable_suspend)
                        {
				stat &= ~(1 << SUSPEND_REQUEST_INTERRUPT);
                                DEBUG (dev, "Suspend disabled, ignoring \n");
                        }
       		} else {
			if (dev->driver->resume)
                        {                            
                                DEBUG (dev, "Resume call \n");                            
				dev->driver->resume (&dev->gadget);
                        }
                }
                stat &= ~tmp;                        
	}

	/* clear any other status/irqs */
	if (stat)
		net2272_write (dev, IRQSTAT1, stat);

	/* some status we can just ignore */
	stat &= ~((1 << CONTROL_STATUS_INTERRUPT)
			| (1 << SUSPEND_REQUEST_INTERRUPT)
			| (1 << RESUME_INTERRUPT));
	if (!stat)
		return;
	else
		DEBUG (dev, "unhandled irqstat1 %02x\n", stat);
}

static irqreturn_t net2272_irq (int irq, void *_dev, struct pt_regs * r)
{
	struct net2272		*dev = _dev;
#if defined(PLX_PCI_RDK) | defined(PLX_PCI_RDK2)
	u32			intcsr;
	u8			dmareq;	
#endif
	
	spin_lock (&dev->lock);
	
#if defined(PLX_PCI_RDK)
	intcsr = readl (dev->plx9054_base_addr + INTCSR);

	if ((intcsr & LOCAL_INTERRUPT_TEST) == LOCAL_INTERRUPT_TEST) {
		writel (intcsr & ~(1 << PCI_INTERRUPT_ENABLE),
				dev->plx9054_base_addr + INTCSR);
		handle_stat1_irqs (dev, net2272_read (dev, IRQSTAT1));
		handle_stat0_irqs (dev, net2272_read (dev, IRQSTAT0));
		intcsr = readl (dev->plx9054_base_addr + INTCSR);
		writel (intcsr | (1 << PCI_INTERRUPT_ENABLE),
			dev->plx9054_base_addr + INTCSR);
	} 
	if ((intcsr & DMA_CHANNEL_0_TEST) == DMA_CHANNEL_0_TEST) {
		writeb ((1 << CHANNEL_CLEAR_INTERRUPT | (0 << CHANNEL_ENABLE)),
				dev->plx9054_base_addr + DMACSR0);

		dmareq = net2272_read (dev, DMAREQ);
		if (dmareq & 0x01)
			handle_dma (&dev->ep [2]);
		else
			handle_dma (&dev->ep [1]);
	}	
#endif
#if defined(PLX_PCI_RDK2)
        // see if PCI int for us by checking irqstat
	intcsr = readl (dev->fpga_base_addr + RDK2_IRQSTAT);
        if (!intcsr & (1 << NET2272_PCI_IRQ))
        {
            return IRQ_NONE;        
        }
        // check dma interrupts
#endif
// Platform interrupt handling
#if !defined(PLX_PCI_RDK)
	handle_stat1_irqs (dev, net2272_read (dev, IRQSTAT1));
	handle_stat0_irqs (dev, net2272_read (dev, IRQSTAT0));
#endif
	spin_unlock (&dev->lock);
	
	return IRQ_HANDLED;
}

/*---------------------------------------------------------------------------*/

int net2272_present(struct net2272 *dev)
{   // Quick test to see if CPU can communicate properly with the NET2272
	//  - Verifies connection using writes and reads to write/read and read-only registers

	// This routine is strongly recommended especially during early bring-up of new
	// hardware, however for designs that do not apply Power On System Tests (POST) 
	// it may discarded (or perhaps minimized).
	unsigned int ii;
	u8 Val, RefVal;

	// Verify NET2272 write/read SCRATCH register can write and read
	RefVal = (u8)net2272_read(dev, SCRATCH);
	for (ii = 0; ii < 0x100; ii += 7)
	{
		net2272_write(dev, SCRATCH, ii);
		if ((Val = net2272_read(dev, SCRATCH)) != ii)
		{
			DEBUG(dev, "AreYouThere(): write/read SCRATCH register test failed: wrote:0x%2.2x, read:0x%2.2x\n",
			      ii, Val
			);
			return -EINVAL;
		}
	}
	// To be nice, we write the original SCRATCH value back:
	net2272_write(dev, SCRATCH, RefVal);

	// Verify NET2272 CHIPREV register is read-only:
	RefVal = net2272_read(dev, CHIPREV_2272);
	for (ii = 0; ii < 0x100; ii += 7)
	{
		net2272_write(dev, CHIPREV_2272, ii);
		if ((Val = net2272_read(dev, CHIPREV_2272)) != RefVal)
		{
			DEBUG(dev, "AreYouThere(): write/read CHIPREV register test failed: wrote 0x%2.2x, read:0x%2.2x expected:0x%2.2x\n",
			      ii, Val, RefVal);
			return -EINVAL;
		}
	}

	// Verify NET2272's "NET2270 legacy revision" register
	//  - NET2272 has two revision registers. The NET2270 legacy revision register should
	//    read the same value, regardless of the NET2272 silicon revision. (The legacy
	//    register applies to NET2270 firmware being applied to the NET2272)
	Val = net2272_read(dev, CHIPREV_LEGACY);
	if (Val != NET2270_LEGACY_REV)
	{   // Unexpected legacy revision value
		//  - Perhaps the chip is a NET2270?
		DEBUG(dev,
		      "\nNcDev_AreYouThere(): WARNING: UNEXPECTED NET2272 LEGACY REGISTER VALUE:\n"
		      " - CHIPREV_LEGACY: expected 0x%2.2x, got:0x%2.2x. (Not NET2272?)\n\n",
		      NET2270_LEGACY_REV, 
		      Val
		);
		// Return Success, even though the chip does not appear to be a NET2272
		return -EINVAL;
	}

	// Verify NET2272 silicon revision
	//  - This revision register is appropriate for the silicon version of the NET2272
	Val = net2272_read(dev, CHIPREV_2272);

	switch (Val)
	{   // This NET2272 firmware is designed for these versions of NET2272 silicon:
		case CHIPREV_NET2272_R1:
			// NET2272 Rev 1 has DMA related errata:
			//  - Newer silicon (Rev 1A or better) required
			DEBUG(dev,
			      "NcDev_AreYouThere(): NET2272 Rev 1 detected:\n"
			      " - Newer silicon recommended for DMA support.\n");
			break;
		case CHIPREV_NET2272_R1A:
			break;
		default:
			// NET2272 silicon version *may* not work with this firmware
			//  - Show warning:
			DEBUG(dev,
			      "NcDev_AreYouThere(): Unexpected NET2272 silicon revision register value:\n"
			      " - CHIPREV_2272: 0x%2.2x\n",
			      Val
			);
			// Return Success, even though the chip rev is not an expected value
			//  - Older, pre-built firmware can attempt to operate on newer silicon
			//  - Often, new silicon is perfectly compatible
	}

	// Success: NET2272 checks out OK
	return 0;
}

/*---------------------------------------------------------------------------*/

#if defined(PLX_PCI_RDK) 

static void net2272_rdk_remove (struct pci_dev *pdev)
{
	struct net2272		*dev = pci_get_drvdata (pdev);
	int			i;

	/* start with the driver above us */
	if (dev->driver) {
		/* should have been done already by driver model core */
		WARN (dev, "pci remove, driver '%s' is still registered\n",
				dev->driver->driver.name);
		usb_gadget_unregister_driver (dev->driver);
	}

	/* disable PLX 9054 interrupts */
	writel (readl (dev->plx9054_base_addr + INTCSR) &
			~(1 << PCI_INTERRUPT_ENABLE),
			dev->plx9054_base_addr + INTCSR);

	/* clean up resources allocated during probe() */
	if (dev->got_irq)
		free_irq (pdev->irq, dev);
	if (dev->plx9054_base_addr)
		iounmap (dev->plx9054_base_addr);
	if (dev->epld_base_addr)
		iounmap (dev->epld_base_addr);
	if (dev->base_addr)
		iounmap (dev->base_addr);
	for (i = 0; i < 4; i++) {
		if (i == 1)
			continue;	/* BAR1 unused */
		release_mem_region (pci_resource_start (pdev, i),
				pci_resource_len (pdev, i));
	}
	if (dev->enabled)
		pci_disable_device (pdev);
	
	INFO (dev, "unbind\n");
        
        kfree(dev);
	the_controller = NULL;
}

#elif defined(PLX_PCI_RDK2) 

static void net2272_rdk_remove (struct pci_dev *pdev)
{
    struct net2272		*dev;
	int			i;

        dev = pci_get_drvdata (pdev);
    
	/* start with the driver above us */
	if (dev->driver) {
		/* should have been done already by driver model core */
		WARN (dev, "pci remove, driver '%s' is still registered\n",
				dev->driver->driver.name);
		usb_gadget_unregister_driver (dev->driver);
	}

	/* disable fpga interrupts */
	// writel (readl (dev->plx9054_base_addr + INTCSR) &
	//		~(1 << PCI_INTERRUPT_ENABLE),
	//		dev->plx9054_base_addr + INTCSR);

	/* clean up resources allocated during probe() */
	if (dev->got_irq)
        {
            INFO (dev, "free irq %d\n", pdev->irq);
            free_irq (pdev->irq, dev);
        }
	if (dev->fpga_base_addr)
        {
            INFO (dev, "unmapping fpga, %08X\n", dev->fpga_base_addr);
            iounmap (dev->fpga_base_addr);
        }
	if (dev->base_addr)
        {
            INFO (dev, "unmapping 2272 %08X\n", dev->base_addr);
            iounmap (dev->base_addr);
        }
	for (i = 0; i < 2; i++) {
		release_mem_region (pci_resource_start (pdev, i),
				pci_resource_len (pdev, i));
	}
	if (dev->enabled)
		pci_disable_device (pdev);
	
	INFO (dev, "unbind\n");
        
        kfree(dev);
	the_controller = NULL;
}
#endif
#if defined(PLX_PCI_RDK) | defined(PLX_PCI_RDK2)

/* RDK card specific probes */
static int
net2272_rdk_probe (struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct net2272		*dev;
	unsigned long		resource, len;
	void			__iomem *mem_mapped_addr[4];
	int			retval, i;
	char			buf [8], *bufp;
	u32	tmp;	

	/* if you want to support more than one controller in a system,
	 * usb_gadget_driver_{register,unregister}() must change.
	 */
        if (the_controller) {
		WARN (the_controller, "ignoring %s\n", pdev->slot_name);
		return -EBUSY;
	}

	/* alloc, and start init */
	dev = kmalloc (sizeof *dev, SLAB_KERNEL);
	if (dev == NULL) {
		retval = -ENOMEM;
		goto done;
	}

	memset (dev, 0, sizeof *dev);
	spin_lock_init (&dev->lock);
	dev->pdev = pdev;
	dev->gadget.ops = &net2272_ops;
	dev->gadget.is_dualspeed = 1;
        
	dev->gadget.dev.bus_id = pdev->slot_name;
	dev->gadget.name = driver_name;
	
	if (pci_enable_device (pdev) < 0) {
		retval = -ENODEV;
		goto done;
	}
	dev->enabled = 1;

	pci_set_master (pdev);
        
#if defined(PLX_PCI_RDK)
	/* BAR 0 holds PLX 9054 config registers
	 * BAR 1 is i/o memory; unused here
	 * BAR 2 holds EPLD config registers
	 * BAR 3 holds NET2272 registers
	 */
	
	/* Find and map all address spaces */
	for (i = 0; i < 4; i++) {
		if (i == 1) 
			continue;	/* BAR1 unused */
		
		resource = pci_resource_start (pdev, i);
		len = pci_resource_len (pdev, i);

		if (!request_mem_region (resource, len, driver_name)) {
			DEBUG(dev, "controller already in use\n");
			retval = -EBUSY;
			goto done;
		}

		mem_mapped_addr [i] = ioremap_nocache (resource, len);
		if (mem_mapped_addr [i] == NULL) {
			DEBUG (dev, "can't map memory\n");
			retval = -EFAULT;
			goto done;
		}
	}// debug SL

	dev->plx9054_base_addr = mem_mapped_addr [0];
	dev->epld_base_addr = mem_mapped_addr [2];
	dev->base_addr = mem_mapped_addr [3];

	dev->indexed_threshold = 1 << 5;
	
	dev->dma_eot_polarity  = 0;
	dev->dma_dack_polarity = 0;
	dev->dma_dreq_polarity = 0;
	dev->dma_busy = 0;
	
	/* Set PLX 9054 bus width (16 bits) */
	tmp = readl (dev->plx9054_base_addr + LBRD1);
	writel ((tmp & ~(3 << MEMORY_SPACE_LOCAL_BUS_WIDTH)) | W16_BIT,
			dev->plx9054_base_addr + LBRD1);

	/* Enable PLX 9054 Interrupts */
	writel (readl (dev->plx9054_base_addr + INTCSR) |
			(1 << PCI_INTERRUPT_ENABLE) |
			(1 << LOCAL_INTERRUPT_INPUT_ENABLE),
			dev->plx9054_base_addr + INTCSR);
	
	writeb ((1 << CHANNEL_CLEAR_INTERRUPT | (0 << CHANNEL_ENABLE)),
			dev->plx9054_base_addr + DMACSR0);

	/* reset */
	*((u8 *)dev->base_addr + EPLD_IO_CONTROL_REGISTER) = 
		  (1 << EPLD_DMA_ENABLE) 
		| (1 << DMA_CTL_DACK)
		| (1 << DMA_TIMEOUT_ENABLE)
		| (1 << USER)
		| (0 << MPX_MODE)
		| (1 << BUSWIDTH)
		| (1 << NET2272_RESET);

	mb ();
	*((u8 *)dev->base_addr + EPLD_IO_CONTROL_REGISTER) &=
			~(1 << NET2272_RESET);
#elif defined(PLX_PCI_RDK2)
	// BAR 0 holds FGPA config registers
	// BAR 1 holds NET2272 registers
	
	/* Find and map all address spaces, bar2-3 unused in rdk 2 */
	for (i = 0; i < 2; i++) {
		resource = pci_resource_start (pdev, i);
		len = pci_resource_len (pdev, i);

		if (!request_mem_region (resource, len, driver_name)) {
			DEBUG(dev, "controller already in use\n");
			retval = -EBUSY;
			goto done;
		}

		mem_mapped_addr [i] = ioremap_nocache (resource, len);
		if (mem_mapped_addr [i] == NULL) {
			DEBUG (dev, "can't map memory\n");
			retval = -EFAULT;
			goto done;
		}
	}

	dev->fpga_base_addr = mem_mapped_addr [0];
	dev->base_addr = mem_mapped_addr [1];

	dev->indexed_threshold = 1 << 5;
	
	dev->dma_eot_polarity  = 0;
	dev->dma_dack_polarity = 0;
	dev->dma_dreq_polarity = 0;
	dev->dma_busy = 0;
	
        mb ();
        /* Set 2272 bus width (16 bits) and reset */
        writel ((1 << CHIP_RESET), dev->fpga_base_addr + RDK2_LOCCTLRDK);
        udelay(200);  
        writel ((1 << BUS_WIDTH), dev->fpga_base_addr + RDK2_LOCCTLRDK);
        // Print fpga version number
        INFO(dev, "RDK2 FPGA version %08x\n", readl(dev->fpga_base_addr + RDK2_FPGAREV));
	/* Enable FPGA Interrupts */
        writel ((1 << NET2272_PCI_IRQ), dev->fpga_base_addr + RDK2_IRQENB);
#endif

	// See if there...
	if (net2272_present(dev))
	{
		WARN(dev, "2272 not found!\n");	
		retval = -ENODEV;
		goto done;		
	}
	
	usb_reset (dev);
	usb_reinit (dev);

	if (!pdev->irq) {
		DEBUG (dev, "No IRQ!\n");
		retval = -ENODEV;
		goto done;
	}
#ifndef __sparc__
	snprintf (buf, sizeof buf, "%d", pdev->irq);
	bufp = buf;
#else
	bufp = __irq_itoa (pdev->irq);
#endif
	if (request_irq (pdev->irq, net2272_irq, SA_SHIRQ, driver_name, dev)
			!= 0) {
		ERROR(dev, "request interrupt %s failed\n", bufp);
		retval = -EBUSY;
		goto done;
	}
	dev->got_irq = 1;

	dev->chiprev = net2272_read (dev, CHIPREV_2272);

	/* done */
	pci_set_drvdata (pdev, dev);
	INFO (dev, "%s\n", driver_desc);
        char *modestring = dma_mode_string();
	INFO (dev, "irq %s, pci mem %p, chip rev %04x, dma_ep=%d, dma enable=%d, dma %d %s\n",
			bufp, dev->base_addr, dev->chiprev,
			dma_ep, use_dma, dma_mode, modestring);
	INFO (dev, "version: %s\n", DRIVER_VERSION);

	the_controller = dev;

	return 0;
done:
        pci_set_drvdata (pdev, dev);
	if (dev)
		net2272_rdk_remove (pdev);
	return retval;
}

/*---------------------------------------------------------------------------*/

/* Table of matching PCI IDs */
static struct pci_device_id __devinitdata pci_ids [] = { {
	.class			= ((PCI_CLASS_BRIDGE_OTHER << 8) | 0xfe),
	.class_mask		= 0,
	.vendor			= 0x10b5,
#if defined (PLX_PCI_RDK)
	.device			= 0x9054,       // RDK 1 card
#else
        .device                 = 0x3272,       // RDK 2 card
#endif
	.subvendor		= PCI_ANY_ID,
	.subdevice		= PCI_ANY_ID,
}, { 0 /* end: all zeros */ }
};
MODULE_DEVICE_TABLE (pci, pci_ids);

static struct pci_driver net2272_driver = {
	.name			= (char *)driver_name,
	.id_table		= pci_ids,

	.probe			= net2272_rdk_probe,
	.remove			= net2272_rdk_remove,
};
#endif

/*---------------------------------------------------------------------------*/

#if !defined(PLX_PCI_RDK) && !defined(PLX_PCI_RDK2)
// Platform remove
static int net2272_remove (struct net2272 *dev)
{
	/* start with the driver above us */
	if (dev->driver) {
		/* should have been done already by driver model core */
		WARN (dev, "remove, driver '%s' is still registered\n",
				dev->driver->driver.name);
		usb_gadget_unregister_driver (dev->driver);
	}

	
	// clean up resources allocated during probe() 
	if (dev->got_irq)
		free_irq (dev->irq, dev);

	if (dev->base_addr)
		iounmap (dev->base_addr);
	INFO (dev, "unbind\n");

	the_controller = NULL;

	return 0;
}

/* platform probe, TODO add are you there */
static int net2272_probe (void)
{
	struct net2272		*dev;
	int			retval;

	if (the_controller) {
		printk ("ignoring\n");
		return -EBUSY;
	}

#ifdef CONFIG_MACH_GP2X
	irq=IRQ_USB_INT;
	base=BASE_USB;
#endif	

	if (!base || !irq) {
		printk ("must provide base/irq as parameters! %lu %d\n", base, irq);
		return -EINVAL;
	}

	/* alloc, and start init */
	dev = kmalloc (sizeof *dev, SLAB_KERNEL);
	if (dev == NULL) {
		printk ("can't allocate memory!\n");
		retval = -ENOMEM;
		goto done;
	}

	memset (dev, 0, sizeof *dev);
	spin_lock_init (&dev->lock);
	dev->gadget.ops = &net2272_ops;
	dev->gadget.is_dualspeed = 1;

	/* the "gadget" abstracts/virtualizes the controller */
	dev->gadget.name = driver_name;

	dev->enabled = 1;

	dev->base_addr = ioremap_nocache (base, 256);
	if (!dev->base_addr) {
		DEBUG (dev, "can't map memory\n");
		retval = -EFAULT;
		goto done;
	}

	dev->indexed_threshold = 1 << 5;
	dev->dma_eot_polarity = 0;
	dev->dma_dack_polarity = 0;
	dev->dma_dreq_polarity = 0;
	dev->dma_busy = 0;

	// See if there..., can remove this test for production code
	if (net2272_present(dev))
	{
		WARN(dev, "2272 not found!\n");	
		retval = -ENODEV;
		goto done;		
	}
	usb_reset (dev);
	usb_reinit (dev);

	if (request_irq (irq, net2272_irq, 0, driver_name, dev) != 0) {
		ERROR (dev, "request interrupt %d failed\n", irq);
		retval = -EBUSY;
		goto done;
	}

	dev->got_irq = 1;
	dev->irq = irq;

	dev->chiprev = net2272_read (dev, CHIPREV_2272);

	/* done */
	INFO (dev, "%s\n", driver_desc);
	INFO (dev, "irq %d, mapped mem %p, chip rev %04x\n",
	      dev->irq, dev->base_addr, dev->chiprev);
	INFO (dev, "running in 16-bit, byte %s bus mode\n",
	      (net2272_read (dev, LOCCTL) & (1 << BYTE_SWAP)) ?
	      "swapped" : "normal");
	INFO (dev, "version: %s\n", DRIVER_VERSION);

	the_controller = dev;

	return 0;

	done:
	if (dev)
		net2272_remove (dev);

	return retval;
}

#endif

/*---------------------------------------------------------------------------*/

static int __init init (void)
{
#if defined(PLX_PCI_RDK) || defined(PLX_PCI_RDK2)
	return pci_module_init (&net2272_driver);	
#else
#ifdef CONFIG_MACH_GP2X	
	set_gpio_ctrl(GPIO_J12, GPIOMD_ALT1, GPIOPU_NOSET); // nPOE->NET2272_IOR
	set_gpio_ctrl(GPIO_J11, GPIOMD_ALT1, GPIOPU_NOSET); // nPWE->NET2272_IOW
	set_gpio_ctrl(GPIO_I10, GPIOMD_ALT1, GPIOPU_NOSET); // nSCS2->NET2272_CS
	set_gpio_ctrl(GPIO_K1, 	GPIOMD_ALT1, GPIOPU_NOSET);  // DVAL1 ->NET2272_DACK
	set_gpio_ctrl(GPIO_K5,  GPIOMD_ALT1, GPIOPU_NOSET);  // DREQ1 ->NET2272_DREQ
	
	set_gpio_ctrl(GPIO_G1, GPIOMD_ALT1, GPIOPU_NOSET); // ZA1
	set_gpio_ctrl(GPIO_G2, GPIOMD_ALT1, GPIOPU_NOSET); // ZA2
	set_gpio_ctrl(GPIO_G3, GPIOMD_ALT1, GPIOPU_NOSET); // ZA3
	set_gpio_ctrl(GPIO_G4, GPIOMD_ALT1, GPIOPU_NOSET); // ZA4
	set_gpio_ctrl(GPIO_G5, GPIOMD_ALT1, GPIOPU_NOSET); // ZA5

	set_gpio_ctrl(GPIO_E0, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[0]
	set_gpio_ctrl(GPIO_E1, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[1]
	set_gpio_ctrl(GPIO_E2, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[2]
	set_gpio_ctrl(GPIO_E3, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[3]
	set_gpio_ctrl(GPIO_E4, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[4]
	set_gpio_ctrl(GPIO_E5, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[5]
	set_gpio_ctrl(GPIO_E6, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[6]
	set_gpio_ctrl(GPIO_E7, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[7]
	set_gpio_ctrl(GPIO_E8, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[8]
	set_gpio_ctrl(GPIO_E9, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[9]
	set_gpio_ctrl(GPIO_E10, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[10]
	set_gpio_ctrl(GPIO_E11, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[11]
	set_gpio_ctrl(GPIO_E12, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[12]
	set_gpio_ctrl(GPIO_E13, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[13]
	set_gpio_ctrl(GPIO_E14, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[14]
	set_gpio_ctrl(GPIO_E15, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[15]
	
#if 1	
	pMCUWREG	= (volatile MCUWREG *)io_p2v(_MEMCFG);
	
		/* Timing Register seting */
	pMCUWREG->MEMCFGW		= (pMCUWREG->MEMCFGW & 0xfffb) | 0x04;		// DataBitWidth=16
	pMCUWREG->MEMTIMEW0		= (pMCUWREG->MEMTIMEW0 & 0xffcf)| Tacs2;
	pMCUWREG->MEMTIMEW1		= (pMCUWREG->MEMTIMEW1 & 0xffcf)| Tcos2;
	pMCUWREG->MEMTIMEW2		= (pMCUWREG->MEMTIMEW2 & 0xffcf)| Tcoh2;
	pMCUWREG->MEMTIMEW3		= (pMCUWREG->MEMTIMEW3 & 0xffcf)| Tcah2;
	pMCUWREG->MEMTIMEW4		= (pMCUWREG->MEMTIMEW4 & 0xf0ff)| Tacc2;
	
	pMCUWREG->MEMWAITCTRLW	= (pMCUWREG->MEMWAITCTRLW & 0xffcf )|Wait;
#endif		
	
	set_gpio_ctrl(GPIO_D4, GPIOMD_OUT, GPIOPU_NOSET); 
	write_gpio_bit(GPIO_D4, 0); // USB_RESET
	udelay(10);	
	write_gpio_bit(GPIO_D4, 1); // USB_RESET Done
	udelay(300);	
	
	set_external_irq(IRQ_USB_INT, EINT_LOW_LEVEL, GPIOPU_DIS); 
#endif
	return net2272_probe ();
#endif
}

static void __exit cleanup (void)
{
#if defined(PLX_PCI_RDK) || defined(PLX_PCI_RDK2)
	pci_unregister_driver (&net2272_driver);	
#else
	net2272_remove (the_controller);
#endif
}
module_init (init);
module_exit (cleanup);

MODULE_DESCRIPTION (DRIVER_DESC);
MODULE_AUTHOR ("PLX Technology, Inc.");
MODULE_LICENSE ("GPL");

