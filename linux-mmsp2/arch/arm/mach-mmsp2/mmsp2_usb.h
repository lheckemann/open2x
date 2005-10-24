/*
 * mmsp2_usb.h
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
 *
 * Based on linux/kernel/arch/arm/mach-sa1100/sa1100_usb.h
 */

#ifndef _MMSP2_USB_H
#define _MMSP2_USB_H
#include <asm/byteorder.h>

/* Power Management Register */
#define DISABLE_SUSPEND		0x00
#define ENABLE_SUSPEND		0x01
#define SUSPEND_MODE		0x02
#define MCU_RESUME			0x04
#define USB_RESET			0x08
#define ISO_UPDATE			(1<<7)

/* MAXP Register */
#define FIFO_SIZE_0		0x00  /* 0x00 * 8 = 0  */
#define FIFO_SIZE_8		0x01  /* 0x01 * 8 = 8  */
#define FIFO_SIZE_16	0x02  /* 0x02 * 8 = 16 */
#define FIFO_SIZE_32	0x04  /* 0x04 * 8 = 32 */
#define FIFO_SIZE_64	0x08  /* 0x08 * 8 = 64 */

/* ENDPOINT0 CSR (Control Status Register) : Mapped to IN CSR1 */
#define EP0_OUT_PKT_RDY			0x01  /* USB sets, MCU clears by setting SERVICED_OUT_PKT_RDY */
#define EP0_IN_PKT_RDY			0x02  /* MCU sets, USB clears after sending FIFO */
#define EP0_SENT_STALL			0x04  /* USB sets */       
#define EP0_DATA_END			0x08  /* MCU sets */
#define EP0_SETUP_END			0x10  /* USB sets, MCU clears by setting SERVICED_SETUP_END */
#define EP0_SEND_STALL			0x20  /* MCU sets */
#define EP0_SERVICED_OUT_PKT_RDY 0x40  /* MCU writes 1 to clear OUT_PKT_READY */
#define EP0_SERVICED_SETUP_END	0x80  /* MCU writes 1 to clear SETUP_END        */

#define EP0_WR_BITS				0xc0  

//EP_INT_REG / EP_INT_EN_REG
#define EP0_INT	0x01  // Endpoint 0, Control
#define EP1_INT	0x02  // Endpoint 1, (Bulk-Out)
#define EP2_INT	0x04  // Endpoint 2, (Bulk-In)
#define EP3_INT	0x08  // Endpoint 3
#define EP4_INT	0x10  // Endpoint 4

//USB_INT_REG / USB_INT_EN_REG
#define SUSPEND_INT	0x01
#define RESUME_INT	0x02
#define RESET_INT	0x04

//IN_CSR1
#define EPI_IN_PKT_RDY	0x01  
#define EPI_UNDER_RUN	0x04
#define EPI_FIFO_FLUSH	0x08
#define EPI_SEND_STALL	0x10  
#define EPI_SENT_STALL	0x20  
#define EPI_CDT			0x40	
#define EPI_WR_BITS		(EPI_FIFO_FLUSH|EPI_IN_PKT_RDY|EPI_CDT)

//IN_CSR2
#define EPI_IN_DMA_INT_MASK	(1<<4)
#define EPI_MODE_IN			(1<<5)
#define EPI_MODE_OUT		(0<<5)
#define EPI_ISO				(1<<6)
#define EPI_BULK			(0<<6)
#define EPI_AUTO_SET		(1<<7)

//OUT_CSR1
#define EPO_OUT_PKT_RDY	0x01  
#define EPO_OVER_RUN	0x04  
#define EPO_DATA_ERROR	0x08  
#define EPO_FIFO_FLUSH	0x10
#define EPO_SEND_STALL	0x20  
#define EPO_SENT_STALL	0x40
#define EPO_CDT			0x80	
#define EPO_WR_BITS		(EPO_FIFO_FLUSH|EPO_SEND_STALL|EPO_CDT)

//OUT_CSR2
#define EPO_OUT_DMA_INT_MASK	(1<<5)
#define EPO_ISO					(1<<6)
#define EPO_BULK				(0<<6)
#define EPO_AUTO_CLR			(1<<7)

#define EP0_PKT_SIZE	8
#define EP1_PKT_SIZE	64
#define EP2_PKT_SIZE	64

typedef void (*usb_callback_t)(int flag, int size);

/* in usb_ctl.c (see also descriptor methods at bottom of file) */

// Open the USB client for client and initialize data structures
// to default values, but _do not_ start UDC.
int mmsp2_usb_open( const char * client_name );

// Start UDC running
int mmsp2_usb_start( void );

// Immediately stop udc, fire off completion routines w/-EINTR
int mmsp2_usb_stop( void ) ;

// Disconnect client from usb core
int mmsp2_usb_close( void ) ;

// set notify callback for when core reaches configured state
// return previous pointer (if any)
typedef void (*usb_notify_t)(void);
usb_notify_t mmsp2_set_configured_callback( usb_notify_t callback );

/* in usb_send.c */
int mmsp2_usb_xmitter_avail( void );
int mmsp2_usb_send(char *buf, int len, usb_callback_t callback);
void mmsp2_usb_send_reset(void);

/* in usb_recev.c */
int mmsp2_usb_recv(char *buf, int len, usb_callback_t callback);
void mmsp2_usb_recv_reset(void);

//////////////////////////////////////////////////////////////////////////////
// Descriptor Management
//////////////////////////////////////////////////////////////////////////////

#define DescriptorHeader \
	__u8 bLength;        \
	__u8 bDescriptorType


// --- Device Descriptor -------------------

typedef struct {
	 DescriptorHeader;
	 __u16 bcdUSB;		   	/* USB specification revision number in BCD */
	 __u8  bDeviceClass;	/* USB class for entire device */
	 __u8  bDeviceSubClass; /* USB subclass information for entire device */
	 __u8  bDeviceProtocol; /* USB protocol information for entire device */
	 __u8  bMaxPacketSize0; /* Max packet size for endpoint zero */
	 __u16 idVendor;        /* USB vendor ID */
	 __u16 idProduct;       /* USB product ID */
	 __u16 bcdDevice;       /* vendor assigned device release number */
	 __u8  iManufacturer;	/* index of manufacturer string */
	 __u8  iProduct;        /* index of string that describes product */
	 __u8  iSerialNumber;	/* index of string containing device serial number */
	 __u8  bNumConfigurations; /* number fo configurations */
} __attribute__ ((packed)) device_desc_t;

// --- Configuration Descriptor ------------

typedef struct {
	 DescriptorHeader;
	 __u16 wTotalLength;	    /* total # of bytes returned in the cfg buf 4 this cfg */
	 __u8  bNumInterfaces;      /* number of interfaces in this cfg */
	 __u8  bConfigurationValue; /* used to uniquely ID this cfg */
	 __u8  iConfiguration;      /* index of string describing configuration */
	 __u8  bmAttributes;        /* bitmap of attributes for ths cfg */
	 __u8  MaxPower;		    /* power draw in 2ma units */
} __attribute__ ((packed)) config_desc_t;

// bmAttributes:
enum { USB_CONFIG_REMOTEWAKE=0x20, USB_CONFIG_SELFPOWERED=0x40,
	   USB_CONFIG_BUSPOWERED=0x80 };
// MaxPower:
#define USB_POWER( x)  ((x)>>1) /* convert mA to descriptor units of A for MaxPower */

// --- Interface Descriptor ---------------

typedef struct {
	 DescriptorHeader;
	 __u8  bInterfaceNumber;   /* Index uniquely identfying this interface */
	 __u8  bAlternateSetting;  /* ids an alternate setting for this interface */
	 __u8  bNumEndpoints;      /* number of endpoints in this interface */
	 __u8  bInterfaceClass;    /* USB class info applying to this interface */
	 __u8  bInterfaceSubClass; /* USB subclass info applying to this interface */
	 __u8  bInterfaceProtocol; /* USB protocol info applying to this interface */
	 __u8  iInterface;         /* index of string describing interface */
} __attribute__ ((packed)) intf_desc_t;

// --- Endpoint  Descriptor ---------------

typedef struct {
	 DescriptorHeader;
	 __u8  bEndpointAddress;  /* 0..3 ep num, bit 7: 0 = 0ut 1= in */
	 __u8  bmAttributes;      /* 0..1 = 0: ctrl, 1: isoc, 2: bulk 3: intr */
	 __u16 wMaxPacketSize;    /* data payload size for this ep in this cfg */
	 __u8  bInterval;         /* polling interval for this ep in this cfg */
} __attribute__ ((packed)) ep_desc_t;

// bEndpointAddress:
enum { USB_OUT= 0, USB_IN=1 };
#define USB_EP_ADDRESS(a,d) (((a)&0xf) | ((d) << 7))
// bmAttributes:
enum { USB_EP_CNTRL=0, USB_EP_BULK=2, USB_EP_INT=3 };

// --- String Descriptor -------------------

typedef struct {
	 DescriptorHeader;
	 __u16 bString[1];		  /* unicode string .. actaully 'n' __u16s */
} __attribute__ ((packed)) string_desc_t;

/*=======================================================
 * Handy helpers when working with above
 *
 */
// these are x86-style 16 bit "words" ...
#define make_word_c( w ) __constant_cpu_to_le16(w)
#define make_word( w )   __cpu_to_le16(w)

// descriptor types
enum { USB_DESC_DEVICE=1, USB_DESC_CONFIG=2, USB_DESC_STRING=3,
	   USB_DESC_INTERFACE=4, USB_DESC_ENDPOINT=5 };


/*=======================================================
 * Default descriptor layout for SA-1100 and SA-1110 UDC
 */

/* "config descriptor buffer" - that is, one config,
   ..one interface and 2 endpoints */
struct cdb {
	 config_desc_t cfg;
	 intf_desc_t   intf;
	 ep_desc_t     ep1;
	 ep_desc_t     ep2;
} __attribute__ ((packed));


/* all SA device descriptors */
typedef struct {
	 device_desc_t dev;   /* device descriptor */
	 struct cdb b;        /* bundle of descriptors for this cfg */
} __attribute__ ((packed)) desc_t;


/*=======================================================
 * Descriptor API
 */

/* Get the address of the statically allocated desc_t structure
   in the usb core driver. Clients can modify this between
   the time they call mmsp2_usb_open() and mmsp2_usb_start()
*/
desc_t *
mmsp2_usb_get_descriptor_ptr( void );


/* Set a pointer to the string descriptor at "index". The driver
 ..has room for 8 string indicies internally. Index zero holds
 ..a LANGID code and is set to US English by default. Inidices
 ..1-7 are available for use in the config descriptors as client's
 ..see fit. This pointer is assumed to be good as long as the
 ..SA usb core is open (so statically allocate them). Returnes -EINVAL
 ..if index out of range */
int mmsp2_usb_set_string_descriptor( int index, string_desc_t * p );

/* reverse of above */
string_desc_t *
mmsp2_usb_get_string_descriptor( int index );

/* kmalloc() a string descriptor and convert "p" to unicode in it */
string_desc_t *
mmsp2_usb_kmalloc_string_descriptor( const char * p );






#endif /* _MMSP2_USB_H */
