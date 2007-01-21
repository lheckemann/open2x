/*
 *  usb_ctl.c
 *
 *  MMSP2 USB controller core driver.
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/tqueue.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include "mmsp2_usb.h"
#include "usb_ctl.h"

#if DEBUG
static unsigned int usb_debug = DEBUG;
#else
#define usb_debug 0     /* gcc will remove all the debug code for us */
#endif

//////////////////////////////////////////////////////////////////////////////
// Prototypes
//////////////////////////////////////////////////////////////////////////////

int usbctl_next_state_on_event( int event );
static void udc_int_hndlr(int, void *, struct pt_regs *);
static void initialize_descriptors( void );
static void soft_connect_hook( int enable );
static void udc_disable(void);
static void udc_enable(void);

#if CONFIG_PROC_FS
#define PROC_NODE_NAME "sausb"
static int usbctl_read_proc(char *page, char **start, off_t off,
							int count, int *eof, void *data);
#endif

//////////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////////
static const char pszMe[] = "usbctl: ";
struct usb_info_t usbd_info;  /* global to ep0, usb_recv, usb_send */

/* device descriptors */
static desc_t desc;

#define MAX_STRING_DESC 8
static string_desc_t * string_desc_array[ MAX_STRING_DESC ];
static string_desc_t sd_zero;  /* special sd_zero holds language codes */

// called when configured
static usb_notify_t configured_callback = NULL;

enum {	kStateZombie  = 0,  kStateZombieSuspend  = 1,
		kStateDefault = 2,  kStateDefaultSuspend = 3,
		kStateAddr    = 4,  kStateAddrSuspend    = 5,
		kStateConfig  = 6,  kStateConfigSuspend  = 7
};

static int device_state_machine[8][6] = {
//                suspend               reset          resume     adddr config deconfig
/* zombie */  {  kStateZombieSuspend,  kStateDefault, kError,    kError, kError, kError },
/* zom sus */ {  kError, kStateDefault, kStateZombie, kError, kError, kError },
/* default */ {  kStateDefaultSuspend, kError, kStateDefault, kStateAddr, kError, kError },
/* def sus */ {  kError, kStateDefault, kStateDefault, kError, kError, kError },
/* addr */    {  kStateAddrSuspend, kStateDefault, kError, kError, kStateConfig, kError },
/* addr sus */{  kError, kStateDefault, kStateAddr, kError, kError, kError },
/* config */  {  kStateConfigSuspend, kStateDefault, kError, kError, kError, kStateAddr },
/* cfg sus */ {  kError, kStateDefault, kStateConfig, kError, kError, kError }
};

/* "device state" is the usb device framework state, as opposed to the
   "state machine state" which is whatever the driver needs and is much
   more fine grained
*/
static int sm_state_to_device_state[8] =
//  zombie           zom suspend          default            default sus
{ USB_STATE_POWERED, USB_STATE_SUSPENDED, USB_STATE_DEFAULT, USB_STATE_SUSPENDED,
// addr              addr sus             config                config sus
  USB_STATE_ADDRESS, USB_STATE_SUSPENDED, USB_STATE_CONFIGURED, USB_STATE_SUSPENDED
};

static char * state_names[8] =
{ "zombie", "zombie suspended", "default", "default suspended",
  "address", "address suspended", "configured", "config suspended"
};

static char * event_names[6] =
{ "suspend", "reset", "resume",
  "address assigned", "configure", "de-configure"
};

static char * device_state_names[] =
{ "not attached", "attached", "powered", "default",
  "address", "configured", "suspended" };

static int sm_state = kStateZombie;

//////////////////////////////////////////////////////////////////////////////
// Async
//////////////////////////////////////////////////////////////////////////////
/* The reg contains mask and interrupt status bits,
 * so using '|=' isn't safe as it may ack an interrupt.
 */

void udc_set_mask(int mask)
{
	USB_INT_EN_REG &= (~mask);
}

void udc_clear_mask(int mask)
{
	USB_INT_EN_REG |= mask;
}

void udc_ack_int(int mask)
{
	USB_INT_REG = mask;
}

static void
udc_int_hndlr(int irq, void *dev_id, struct pt_regs *regs)
{
  	__u32 status = USB_INT_REG;
  	__u32 ir_status = EP_INT_REG;

	//mask ints
	udc_set_mask(SUSPEND_INT | RESET_INT);

	if(usb_debug > 2) {
		printk("%s--- udc_int_hndlr\n"
			"USB_INT_REG=0x%08x EP_INT_REG=0x%08x EP_INT_EN_REG=0x%08x\n", 
		    pszMe, status, ir_status, EP_INT_EN_REG);
	}

	/* SUSpend Interrupt Request */
	if(status & SUSPEND_INT) {
		if(usb_debug) printk("%sSuspend...\n", pszMe);
		usbctl_next_state_on_event(kEvSuspend);
		udc_ack_int(SUSPEND_INT);
	}

	/* RESume Interrupt Request */
	if(status & RESUME_INT) {
		if(usb_debug) printk("%sResume...\n", pszMe);
		usbctl_next_state_on_event(kEvResume);
		udc_ack_int(RESUME_INT);
	}

	/* ReSeT Interrupt Request - UDC has been reset */
	if(status & RESET_INT) {
		if(usbctl_next_state_on_event(kEvReset) != kError) {
			/* starting reset sequence now... */
			if(usb_debug) printk("%sResetting\n", pszMe);
			PWR_REG = 0x00;
			
			ep0_reset();
			ep_bulk_in1_reset();
			ep_bulk_out1_reset();

			EP_INT_REG = 0x1F;
			USB_INT_REG = 0x07;
			EP_INT_EN_REG = EP0_INT|EP1_INT|EP2_INT;
			USB_INT_EN_REG = RESET_INT;
		}
		else {
			printk("%sUnexpected reset\n", pszMe);
		}
		/* clear the reset interrupt */
		udc_ack_int(RESET_INT);
	}
	else {
		/* ep0 int */
		if(ir_status & EP0_INT)
			ep0_int_hndlr();

		/* transmit bulk */
		if(ir_status & EP2_INT)
			ep_bulk_in1_int_hndlr(ir_status);

		/* receive bulk */
		if(ir_status & EP1_INT)
			ep_bulk_out1_int_hndlr(ir_status);
	}

	udc_clear_mask(SUSPEND_INT | RESET_INT);

	/* clear all endpoint ints */
	EP_INT_REG = 0x1F;

	if(usb_debug > 2) {
		printk("%s udc_int_hndlr\n"
			"USB_INT_REG=0x%08x EP_INT_REG=0x%08x EP_INT_EN_REG=0x%08x\n", 
		    pszMe, USB_INT_REG, EP_INT_REG, EP_INT_EN_REG);
	}
}

//////////////////////////////////////////////////////////////////////////////
// Public Interface
//////////////////////////////////////////////////////////////////////////////

/* Open MMSP2 usb core on behalf of a client, but don't start running */

int
mmsp2_usb_open(const char * client)
{
	if(usbd_info.client_name != NULL) {
		printk( "%sUnable to register %s (%s already registered).\n", 
			pszMe, client, usbd_info.client_name );
		return -EBUSY;
	}

	usbd_info.client_name = (char*)client;
	memset(&usbd_info.stats, 0, sizeof(struct usb_stats_t));
	memset(string_desc_array, 0, sizeof(string_desc_array));

	/* hack to start in zombie suspended state */
	sm_state = kStateZombieSuspend;
	usbd_info.state = USB_STATE_SUSPENDED;

	/* create descriptors for enumeration */
	initialize_descriptors();

	printk( "%s%s registered.\n", pszMe, client );
	return 0;
}

/* Start running. Must have called usb_open (above) first */
int
mmsp2_usb_start( void )
{
	if(usbd_info.client_name == NULL) {
		printk( "%s%s - no client registered\n",
				pszMe, __FUNCTION__ );
		return -EPERM;
	}

	/* start UDC internal machinery running */
	udc_enable();
	udelay(100);

	/* flush DMA and fire through some -EAGAINs */
	ep_bulk_out1_init(usbd_info.dmach_rx);
	ep_bulk_in1_init(usbd_info.dmach_tx);

	/* give endpoint notification we are starting */
	ep_bulk_out1_state_change_notify(USB_STATE_SUSPENDED);
	ep_bulk_in1_state_change_notify(USB_STATE_SUSPENDED);

	/* enable any platform specific hardware */
	soft_connect_hook(1);

	/* enable suspend/resume, reset */
	udc_clear_mask(SUSPEND_INT | RESET_INT);

	/* enable ep0, ep1, ep2 */
	EP_INT_EN_REG = (EP0_INT | EP1_INT | EP2_INT); 

	if(usb_debug) printk("%sStarted %s\n", pszMe, usbd_info.client_name);
	return 0;
}

/* Stop USB core from running */
int
mmsp2_usb_stop( void )
{
	if(usbd_info.client_name == NULL) {
		printk( "%s%s - no client registered\n",
				pszMe, __FUNCTION__ );
		return -EPERM;
	}
	/* mask everything */
	/* disable suspend/resume, reset */
	udc_set_mask(SUSPEND_INT | RESET_INT); 
	/* disable ep0, ep1, ep2 */
	EP_INT_EN_REG &= ~(EP0_INT | EP1_INT | EP2_INT); 

	ep_bulk_out1_reset();
	ep_bulk_in1_reset();

	udc_disable();
	if(usb_debug) printk("%sStopped %s\n", pszMe, usbd_info.client_name);
	return 0;
}

/* Tell MMSP2 core client is through using it */
int
mmsp2_usb_close(void)
{
	 if(usbd_info.client_name == NULL) {
		  printk("%s%s - no client registered\n",
				  pszMe, __FUNCTION__);
		  return -EPERM;
	 }
	 printk("%s%s closed.\n", pszMe, (char*)usbd_info.client_name);
	 usbd_info.client_name = NULL;
	 return 0;
}

/* set a proc to be called when device is configured */
usb_notify_t mmsp2_set_configured_callback(usb_notify_t func)
{
	 usb_notify_t retval = configured_callback;
	 configured_callback = func;
	 return retval;
}

/*====================================================
 * Descriptor Manipulation.
 * Use these between open() and start() above to setup
 * the descriptors for your device.
 *
 */

/* get pointer to static default descriptor */
desc_t *
mmsp2_usb_get_descriptor_ptr(void) { return &desc; }

/* optional: set a string descriptor */
int
mmsp2_usb_set_string_descriptor(int i, string_desc_t *p)
{
	 int retval;
	 if(i < MAX_STRING_DESC) {
		  string_desc_array[i] = p;
		  retval = 0;
	 } else {
		  retval = -EINVAL;
	 }
	 return retval;
}

/* optional: get a previously set string descriptor */
string_desc_t *
mmsp2_usb_get_string_descriptor(int i)
{
	 return (i < MAX_STRING_DESC)
		    ? string_desc_array[i]
		    : NULL;
}

/* optional: kmalloc and unicode up a string descriptor */
string_desc_t *
mmsp2_usb_kmalloc_string_descriptor(const char *p)
{
	string_desc_t * pResult = NULL;

	if(p) {
		int len = strlen(p);
		int uni_len = len * sizeof( __u16 );
		pResult = (string_desc_t*) kmalloc( uni_len + 2, GFP_KERNEL ); /* ugh! */
		if(pResult != NULL) {
			int i;
			pResult->bLength = uni_len + 2;
			pResult->bDescriptorType = USB_DESC_STRING;
			for( i = 0; i < len ; i++ ) {
				pResult->bString[i] = make_word( (__u16) p[i] );
			}
		}
	}
	return pResult;
}

//////////////////////////////////////////////////////////////////////////////
// Exports to rest of driver
//////////////////////////////////////////////////////////////////////////////

/* called by the int handler here and the two endpoint files when interesting
   .."events" happen */

int
usbctl_next_state_on_event(int event)
{
	int next_state = device_state_machine[ sm_state ][ event ];
	if(next_state != kError) {
		int next_device_state = sm_state_to_device_state[ next_state ];
		if(usb_debug) printk( "%s%s --> [%s] --> %s. Device in %s state.\n",
			pszMe, state_names[ sm_state ], event_names[ event ],
			state_names[ next_state ], device_state_names[ next_device_state ] );

		sm_state = next_state;
		if(usbd_info.state != next_device_state) {
			if(configured_callback != NULL &&
				next_device_state == USB_STATE_CONFIGURED &&
				usbd_info.state != USB_STATE_SUSPENDED) {
				configured_callback();
			}
			usbd_info.state = next_device_state;

			ep_bulk_out1_state_change_notify(next_device_state);
			ep_bulk_in1_state_change_notify(next_device_state);
		}
	}
#if 0
	else
		printk( "%s%s --> [%s] --> ??? is an error.\n",
			pszMe, state_names[ sm_state ], event_names[ event ]);
#endif
	return next_state;
}

//////////////////////////////////////////////////////////////////////////////
// Private Helpers
//////////////////////////////////////////////////////////////////////////////

/* setup default descriptors */

static void
initialize_descriptors(void)
{
	desc.dev.bLength               = sizeof( device_desc_t );
	desc.dev.bDescriptorType       = USB_DESC_DEVICE;
	desc.dev.bcdUSB                = 0x100; /* 1.0 */
	desc.dev.bDeviceClass          = 0xFF;	/* vendor specific */
	desc.dev.bDeviceSubClass       = 0;
	desc.dev.bDeviceProtocol       = 0;
	desc.dev.bMaxPacketSize0       = 8; /* ep0 max fifo size */
	desc.dev.idVendor              = 0;	/* vendor ID undefined */
	desc.dev.idProduct             = 0; /* product */
	desc.dev.bcdDevice             = 0; /* vendor assigned device release num */
	desc.dev.iManufacturer         = 0;	/* index of manufacturer string */
	desc.dev.iProduct              = 0; /* index of product description string */
	desc.dev.iSerialNumber         = 0;	/* index of string holding product s/n */
	desc.dev.bNumConfigurations    = 1;

	desc.b.cfg.bLength             = sizeof( config_desc_t );
	desc.b.cfg.bDescriptorType     = USB_DESC_CONFIG;
	desc.b.cfg.wTotalLength        = make_word_c( sizeof(struct cdb) );
	desc.b.cfg.bNumInterfaces      = 1;
	desc.b.cfg.bConfigurationValue = 1;
	desc.b.cfg.iConfiguration      = 0;
	desc.b.cfg.bmAttributes        = USB_CONFIG_BUSPOWERED;
	desc.b.cfg.MaxPower            = USB_POWER( 500 );

	desc.b.intf.bLength            = sizeof( intf_desc_t );
	desc.b.intf.bDescriptorType    = USB_DESC_INTERFACE;
	desc.b.intf.bInterfaceNumber   = 0; /* unique intf index*/
	desc.b.intf.bAlternateSetting  = 0;
	desc.b.intf.bNumEndpoints      = 2;
	desc.b.intf.bInterfaceClass    = 0xFF; /* vendor specific */
	desc.b.intf.bInterfaceSubClass = 0;
	desc.b.intf.bInterfaceProtocol = 0;
	desc.b.intf.iInterface         = 0;

	desc.b.ep1.bLength             = sizeof( ep_desc_t );
	desc.b.ep1.bDescriptorType     = USB_DESC_ENDPOINT;
	desc.b.ep1.bEndpointAddress    = USB_EP_ADDRESS( 1, USB_OUT );
	desc.b.ep1.bmAttributes        = USB_EP_BULK;
	desc.b.ep1.wMaxPacketSize      = make_word_c( 64 );
	desc.b.ep1.bInterval           = 0;

	desc.b.ep2.bLength             = sizeof( ep_desc_t );
	desc.b.ep2.bDescriptorType     = USB_DESC_ENDPOINT;
	desc.b.ep2.bEndpointAddress    = USB_EP_ADDRESS( 2, USB_IN );
	desc.b.ep2.bmAttributes        = USB_EP_BULK;
	desc.b.ep2.wMaxPacketSize      = make_word_c( 64 );
	desc.b.ep2.bInterval           = 0;

	/* set language */
	/* See: http://www.usb.org/developers/data/USB_LANGIDs.pdf */
	sd_zero.bDescriptorType = USB_DESC_STRING;
	sd_zero.bLength         = sizeof( string_desc_t );
	sd_zero.bString[0]      = make_word_c( 0x409 ); /* American English */
	mmsp2_usb_set_string_descriptor( 0, &sd_zero );
}

/* soft_connect_hook()
 * Some devices have platform-specific circuitry to make USB
 * not seem to be plugged in, even when it is. This allows
 * software to control when a device 'appears' on the USB bus
 * (after Linux has booted and this driver has loaded, for
 * example). If you have such a circuit, control it here.
 */
static void
soft_connect_hook(int enable)
{
}

/* disable the UDC at the source */
static void
udc_disable(void)
{
	soft_connect_hook( 0 );

	/* Disable clock for USB device */
	COMCLKEN &= ~(1 << 1);
}

/*  enable the udc at the source */
static void
udc_enable(void)
{
	/* Enable clock for USB device */
	COMCLKEN |= (1 << 1);

	/* try to clear these bits before we enable the udc */
	udc_ack_int(SUSPEND_INT);
	udc_ack_int(RESET_INT);
	udc_ack_int(RESUME_INT);
}

//////////////////////////////////////////////////////////////////////////////
// Proc Filesystem Support
//////////////////////////////////////////////////////////////////////////////

#if CONFIG_PROC_FS

#define SAY( fmt, args... )  p += sprintf(p, fmt, ## args )
#define SAYV(  num )         p += sprintf(p, num_fmt, "Value", num )
#define SAYC( label, yn )    p += sprintf(p, yn_fmt, label, yn )
#define SAYS( label, v )     p += sprintf(p, cnt_fmt, label, v )

static int usbctl_read_proc(char *page, char **start, off_t off,
			    int count, int *eof, void *data)
{
	 const char * cnt_fmt   = "%25.25s: %lu\n";
	 char * p = page;
	 int len;

 	 SAY( "MMSP2 USB Controller Core\n" );
 	 SAY( "Active Client: %s\n", usbd_info.client_name ? usbd_info.client_name : "none");
	 SAY( "USB state: %s (%s) %d\n",
		  device_state_names[ sm_state_to_device_state[ sm_state ] ],
		  state_names[ sm_state ],
		  sm_state );

	 SAYS( "ep0 bytes read", usbd_info.stats.ep0_bytes_read );
	 SAYS( "ep0 bytes written", usbd_info.stats.ep0_bytes_written );
	 SAYS( "ep0 FIFO read failures", usbd_info.stats.ep0_fifo_write_failures );
	 SAYS( "ep0 FIFO write failures", usbd_info.stats.ep0_fifo_write_failures );

	 SAY( "\n" );

	 len = ( p - page ) - off;
	 if ( len < 0 )
		  len = 0;
	 *eof = ( len <=count ) ? 1 : 0;
	 *start = page + off;
	 return len;
}

#endif  /* CONFIG_PROC_FS */

//////////////////////////////////////////////////////////////////////////////
// Module Initialization and Shutdown
//////////////////////////////////////////////////////////////////////////////
/*
 * usbctl_init()
 * Module load time. Allocate dma and interrupt resources. Setup /proc fs
 * entry. Leave UDC disabled.
 */
int __init usbctl_init( void )
{
	int retval = 0;

	udc_disable();

	memset( &usbd_info, 0, sizeof( usbd_info ) );
#if CONFIG_PROC_FS
	create_proc_read_entry ( PROC_NODE_NAME, 0, NULL, usbctl_read_proc, NULL);
#endif

	/* now allocate the IRQ. */
	retval = request_irq(IRQ_USBD, udc_int_hndlr, SA_INTERRUPT, "USB core", NULL);
	if(retval) {
		printk("%sCouldn't request USB irq rc=%d\n",pszMe, retval);
		goto err_irq;
	}

	printk( "MMSP2 USB Controller Core Initialized\n");
	return 0;

err_irq:
	return retval;
}
/*
 * usbctl_exit()
 * Release DMA and interrupt resources
 */
void __exit usbctl_exit(void)
{
	printk("Unloading MMSP2 USB Controller\n");

	udc_disable();

#if CONFIG_PROC_FS
	remove_proc_entry ( PROC_NODE_NAME, NULL);
#endif

	free_irq(IRQ_USBD, NULL);
}

module_init( usbctl_init );
module_exit( usbctl_exit );

EXPORT_SYMBOL( mmsp2_usb_open );
EXPORT_SYMBOL( mmsp2_usb_start );
EXPORT_SYMBOL( mmsp2_usb_stop );
EXPORT_SYMBOL( mmsp2_usb_close );
EXPORT_SYMBOL( mmsp2_usb_get_descriptor_ptr );
EXPORT_SYMBOL( mmsp2_usb_set_string_descriptor );
EXPORT_SYMBOL( mmsp2_usb_get_string_descriptor );
EXPORT_SYMBOL( mmsp2_usb_kmalloc_string_descriptor );
