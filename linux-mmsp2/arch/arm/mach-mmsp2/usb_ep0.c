/*
 *  usb_ep0.c - MMSP2 USB controller driver.
 *              Endpoint zero management
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
 *
 * Based on linux/kernel/arch/arm/mach-sa1100/usb_ep0.c
 */

#include <linux/delay.h>

#include "mmsp2_usb.h"  /* public interface */
#include "usb_ctl.h"     /* private stuff */

#define CLR_EP0_OUT_PKT_RDY()	EP0_CSR=((EP0_CSR & (~EP0_WR_BITS))| \
						EP0_SERVICED_OUT_PKT_RDY)
#define CLR_EP0_OUTPKTRDY_DATAEND()	EP0_CSR=((EP0_CSR & (~EP0_WR_BITS))| \
						(EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END))

#define SET_EP0_IN_PKT_RDY()	EP0_CSR=((EP0_CSR & (~EP0_WR_BITS))| \
						EP0_IN_PKT_RDY)
#define SET_EP0_INPKTRDY_DATAEND()	EP0_CSR=((EP0_CSR & (~EP0_WR_BITS))| \
						(EP0_IN_PKT_RDY|EP0_DATA_END))
#define SET_EP0_DATA_END()	EP0_CSR=((EP0_CSR & (~EP0_WR_BITS))| \
						EP0_DATA_END)

#define CLR_EP0_SETUP_END()		EP0_CSR=((EP0_CSR & (~EP0_WR_BITS))| \
						EP0_SERVICED_SETUP_END)

#define CLR_EP0_SENT_STALL()	EP0_CSR=((EP0_CSR & (~EP0_WR_BITS))& \
						(~EP0_SENT_STALL))
#define SET_EP0_SEND_STALL()	EP0_CSR=((EP0_CSR & (~EP0_WR_BITS))| \
						(EP0_SERVICED_OUT_PKT_RDY|EP0_SEND_STALL))

#define FLUSH_EP0_FIFO() 	{while(OUT_FIFO_CNT1_REG) EP0_FIFO_REG;}

// 1 == lots of trace noise,  0 = only "important' stuff
#define VERBOSITY 0

enum { true = 1, false = 0 };
typedef int bool;
#ifndef MIN
#define MIN( a, b ) ((a)<(b)?(a):(b))
#endif

#if 1 && !defined( ASSERT )
#  define ASSERT(expr) \
if(!(expr)) { \
	printk( "Assertion failed! %s,%s,%s,line=%d\n",\
#expr,__FILE__,__FUNCTION__,__LINE__); \
}
#else
#  define ASSERT(expr)
#endif

#if VERBOSITY
#define PRINTKD(fmt, args...) printk( fmt , ## args)
#else
#define PRINTKD(fmt, args...)
#endif

/*================================================
 * USB Protocol Stuff
 */

/* Request Codes   */
enum { GET_STATUS=0,         CLEAR_FEATURE=1,     SET_FEATURE=3,
	   SET_ADDRESS=5,        GET_DESCRIPTOR=6,	  SET_DESCRIPTOR=7,
	   GET_CONFIGURATION=8,  SET_CONFIGURATION=9, GET_INTERFACE=10,
	   SET_INTERFACE=11 };


/* USB Device Requests */
typedef struct
{
    __u8 bmRequestType;
    __u8 bRequest;
    __u16 wValue;
    __u16 wIndex;
    __u16 wLength;
} usb_dev_request_t  __attribute__ ((packed));

/***************************************************************************
Prototypes
***************************************************************************/
/* "setup handlers" -- the main functions dispatched to by the
   .. isr. These represent the major "modes" of endpoint 0 operaton */
/* "setup handlers" -- the main functions dispatched to by the
   .. isr. These represent the major "modes" of endpoint 0 operaton */
static void sh_setup_begin(void);				/* setup begin (idle) */
static void sh_write( void );      				/* writing data */
static void sh_write_with_empty_packet( void ); /* empty packet at end of xfer*/
/* called before both sh_write routines above */
static void common_write_preamble( void );

/* other subroutines */
static __u32  queue_and_start_write( void * p, int req, int act );
static void write_fifo( void );
static int read_fifo( usb_dev_request_t * p );
static void get_descriptor( usb_dev_request_t * pReq );

/* some voodo helpers  01Mar01ww */
static void set_cs_bits( __u32 set_bits );

/***************************************************************************
Inline Helpers
***************************************************************************/
/* Data extraction from usb_request_t fields */
enum { kTargetDevice=0, kTargetInterface=1, kTargetEndpoint=2 };
static inline int request_target( __u8 b ) { return (int) ( b & 0x0F); }

static inline int windex_to_ep_num( __u16 w ) { return (int) ( w & 0x000F); }
inline int type_code_from_request( __u8 by ) { return (( by >> 4 ) & 3); }

/* following is hook for self-powered flag in GET_STATUS. Some devices
   .. might like to override and return real info */
static inline bool self_powered_hook( void ) { return true; }

/* print string descriptor */
static inline void psdesc(string_desc_t * p)
{
	int i;
	int nchars = ( p->bLength - 2 ) / sizeof( __u16 );
	printk( "'" );
	for( i = 0 ; i < nchars ; i++ ) {
		printk( "%c", (char) p->bString[i] );
	}
	printk( "'\n" );
}

#if VERBOSITY
/* "pcs" == "print control status" */
static inline void pcs(void)
{
	int s, foo;
	s = INDEX_REG;
	INDEX_REG = 0x00;
	foo = EP0_CSR;
	printk( "%08x: %s %s %s %s\n",
		foo,
		foo & EP0_SETUP_END ? "SA" : "",
		foo & EP0_OUT_PKT_RDY ? "OPR" : "",
		foo & EP0_SENT_STALL ? "SST" : "",
		foo & EPO_SEND_STALL ? "FST" : "");
	INDEX_REG = s;
}

static inline void preq(usb_dev_request_t * pReq)
{
	static char * tnames[] = { "dev", "intf", "ep", "oth" };
	static char * rnames[] = { "std", "class", "vendor", "???" };
	char * psz;
	switch( pReq->bRequest ) {
		case GET_STATUS:        psz = "get stat"; break;
		case CLEAR_FEATURE:     psz = "clr feat"; break;
		case SET_FEATURE:       psz = "set feat"; break;
		case SET_ADDRESS:       psz = "set addr"; break;
		case GET_DESCRIPTOR:    psz = "get desc"; break;
		case SET_DESCRIPTOR:    psz = "set desc"; break;
		case GET_CONFIGURATION: psz = "get cfg"; break;
		case SET_CONFIGURATION: psz = "set cfg"; break;
		case GET_INTERFACE:     psz = "get intf"; break;
		case SET_INTERFACE:     psz = "set intf"; break;
		default:                psz = "unknown"; break;
	}
	printk( "- [%s: %s req to %s. dir=%s]\n", psz,
			rnames[ (pReq->bmRequestType >> 5) & 3 ],
			tnames[ pReq->bmRequestType & 3 ],
			( pReq->bmRequestType & 0x80 ) ? "in" : "out" );
}

#else
static inline void pcs(void) {}
static inline void preq(usb_dev_request_t *x) {}
#endif

/***************************************************************************
Globals
***************************************************************************/
static const char pszMe[] = "usbep0: ";

/* pointer to current setup handler */
static void (*current_handler)(void) = sh_setup_begin;

/* global write struct to keep write
   ..state around across interrupts */
static struct {
	unsigned char *p;
	int bytes_left;
} wr;

/***************************************************************************
Public Interface
***************************************************************************/

/* reset received from HUB (or controller just went nuts and reset by itself!)
  so udc core has been reset, track this state here  */
void ep0_reset(void)
{
	PRINTKD( "%sep0_reset\n", pszMe);
	/* reset state machine */
	current_handler = sh_setup_begin;
	wr.p = NULL;
	wr.bytes_left = 0;
	usbd_info.address=0;
}


/* handle interrupt for endpoint zero */
void ep0_int_hndlr( void )
{
	PRINTKD( "%sep0_int_hndlr\n", pszMe);
	pcs();

	if ( current_handler != sh_setup_begin )
		common_write_preamble();

	(*current_handler)();

	PRINTKD( "---\n" );
	pcs();
	PRINTKD( "\\/\n" );
}

/***************************************************************************
Setup Handlers
***************************************************************************/
/*
 * sh_setup_begin()
 * This setup handler is the "idle" state of endpoint zero. It looks for OPR
 * (OUT packet ready) to see if a setup request has been been received from the
 * host. Requests without a return data phase are immediately handled. Otherwise,
 * in the case of GET_XXXX the handler may be set to one of the sh_write_xxxx
 * data pumpers if more than 8 bytes need to get back to the host.
 *
 */
static void sh_setup_begin(void)
{
	unsigned char status_buf[2];  /* returned in GET_STATUS */
	usb_dev_request_t req;
	int request_type;
	int n;
	__u32 cs_bits;
	__u32 address;
	__u32 ind;

	PRINTKD( "%ssh_setup_begin\n", pszMe);

	ind = INDEX_REG;
	INDEX_REG = 0x00;
	if(EP0_CSR & EP0_SENT_STALL) {
		PRINTKD( "%ssetup begin: sent stall. Continuing\n", pszMe );
		CLR_EP0_SENT_STALL();
	}

	if(EP0_CSR & EP0_SETUP_END) {
		PRINTKD( "%ssetup begin: Early term of setup. Continuing\n", pszMe );
		CLR_EP0_SETUP_END();	/* clear setup end */
	}

	/* Be sure out packet ready, otherwise something is wrong */
	if((EP0_CSR & EP0_OUT_PKT_RDY)==0) {
		/* we can get here early...if so, we'll int again in a moment  */
		PRINTKD( "%ssetup begin: no OUT packet available. Exiting\n", pszMe );
		goto sh_sb_end;
	}

	/* read the setup request */
	n = read_fifo( &req );
	if ( n != sizeof( req ) ) {
		printk( "%ssetup begin: fifo READ ERROR wanted %d bytes got %d. "
			" Stalling out...\n",
			pszMe, sizeof( req ), n );
		/* force stall, serviced out */
		SET_EP0_SEND_STALL();
		goto sh_sb_end;
	}

	/* Is it a standard request? (not vendor or class request) */
	request_type = type_code_from_request( req.bmRequestType );
	if ( request_type != 0 ) {
		printk( "%ssetup begin: unsupported bmRequestType: %d ignored\n",
			pszMe, request_type );
		CLR_EP0_OUTPKTRDY_DATAEND();
		goto sh_sb_end;
	}

#if VERBOSITY
	{
		unsigned char * pdb = (unsigned char *) &req;
		PRINTKD( "%2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X ",
			pdb[0], pdb[1], pdb[2], pdb[3], pdb[4], pdb[5], pdb[6], pdb[7]);
		preq( &req );
	}
#endif

	/* Handle it */
	switch( req.bRequest ) {

		/* This first bunch have no data phase */

	case SET_ADDRESS:
		address = (__u32) (req.wValue & 0x7F);
		/* when SO and DE sent, UDC will enter status phase and ack,
			..propagating new address to udc core. Next control transfer
			..will be on the new address. You can't see the change in a
			..read back of CAR until then. (about 250us later, on my box).
			..The original Intel driver sets S0 and DE and code to check
			..that address has propagated here. I tried this, but it
			..would only work sometimes! The rest of the time it would
			..never propagate and we'd spin forever. So now I just set
			..it and pray...
		 */
		FUNC_ADDR_REG = (address & 0x7F) | 0x80;
		usbd_info.address = address;
		usbctl_next_state_on_event( kEvAddress );
		CLR_EP0_OUTPKTRDY_DATAEND();	/* no data phase */
		printk( "%sI have been assigned address: %d\n", pszMe, address );
		break;

	case SET_CONFIGURATION:
		if ( req.wValue == 1 ) {
			/* configured */
			if (usbctl_next_state_on_event( kEvConfig ) != kError) {
				/* (re)set the out and in max packet sizes */
				desc_t * pDesc = mmsp2_usb_get_descriptor_ptr();
				__u32 out = __le16_to_cpu( pDesc->b.ep1.wMaxPacketSize );
				__u32 in  = __le16_to_cpu( pDesc->b.ep2.wMaxPacketSize );
				__u32 i = INDEX_REG;
				INDEX_REG = 0x01;
				MAXP_REG = (out >> 3);
				INDEX_REG = 0x02;
				MAXP_REG = (in >> 3);
				INDEX_REG = i;
				printk( "%sConfigured (OMP=%8.8X IMP=%8.8X)\n", pszMe, out, in );
			}
		} else if ( req.wValue == 0 ) {
			/* de-configured */
			if (usbctl_next_state_on_event( kEvDeConfig ) != kError )
				printk( "%sDe-Configured\n", pszMe );
		} else {
			printk( "%ssetup phase: Unknown "
				"\"set configuration\" data %d\n",
				pszMe, req.wValue );
		}
		CLR_EP0_OUTPKTRDY_DATAEND();	/* no data phase */
		break;

	case CLEAR_FEATURE:
		/* could check data length, direction...26Jan01ww */
		if ( req.wValue == 0 ) { /* clearing ENDPOINT_HALT/STALL */
			int ep = windex_to_ep_num( req.wIndex );
			if ( ep == 1 ) {
				printk( "%sclear feature \"endpoint halt\" "
					" on receiver\n", pszMe );
				ep_bulk_out1_reset();
			}
			else if ( ep == 2 ) {
				printk( "%sclear feature \"endpoint halt\" "
					"on xmitter\n", pszMe );
				ep_bulk_in1_reset();
			} else {
				printk( "%sclear feature \"endpoint halt\" "
					"on unsupported ep # %d\n",
					pszMe, ep );
			}
		} else {
			printk( "%sUnsupported feature selector (%d) "
				"in clear feature. Ignored.\n" ,
				pszMe, req.wValue );
		}
		CLR_EP0_OUTPKTRDY_DATAEND();	/* no data phase */
		break;

	case SET_FEATURE:
		if ( req.wValue == 0 ) { /* setting ENDPOINT_HALT/STALL */
			int ep = windex_to_ep_num( req.wValue );
			if ( ep == 1 ) {
				printk( "%set feature \"endpoint halt\" "
					"on receiver\n", pszMe );
				ep_bulk_out1_stall();
			}
			else if ( ep == 2 ) {
				printk( "%sset feature \"endpoint halt\" "
					" on xmitter\n", pszMe );
				ep_bulk_in1_stall();
			} else {
				printk( "%sset feature \"endpoint halt\" "
					"on unsupported ep # %d\n",
					pszMe, ep );
			}
		}
		else {
			printk( "%sUnsupported feature selector "
				"(%d) in set feature\n",
				pszMe, req.wValue );
		}
		CLR_EP0_OUTPKTRDY_DATAEND();	/* no data phase */
		break;

		/* The rest have a data phase that writes back to the host */
	case GET_STATUS:
		/* return status bit flags */
		status_buf[0] = status_buf[1] = 0;
		n = request_target(req.bmRequestType);
		switch( n ) {
		case kTargetDevice:
			if ( self_powered_hook() )
				status_buf[0] |= 1;
			break;
		case kTargetInterface:
			break;
		case kTargetEndpoint:
			/* return stalled bit */
			n = windex_to_ep_num( req.wIndex );
			if ( n == 1 ) {
				__u32 i = INDEX_REG;
				INDEX_REG = 0x01;
				status_buf[0] |= (OUT_CSR1_REG & 0x20) >> 5;
				INDEX_REG = i;
			}
			else if ( n == 2 ) {
				__u32 i = INDEX_REG;
				INDEX_REG = 0x02;
				status_buf[0] |= (IN_CSR1_REG & 0x10) >> 4;
				INDEX_REG = i;
			}
			else {
				printk( "%sUnknown endpoint (%d) "
					"in GET_STATUS\n", pszMe, n );
			}
			break;
		default:
			printk( "%sUnknown target (%d) in GET_STATUS\n",
				pszMe, n );
			/* fall thru */
			break;
		}
		cs_bits  = queue_and_start_write( status_buf, req.wLength,
			sizeof( status_buf ) );
		set_cs_bits( cs_bits );
		break;

	case GET_DESCRIPTOR:
		get_descriptor( &req );
		break;

	case GET_CONFIGURATION:
		status_buf[0] = (usbd_info.state ==  USB_STATE_CONFIGURED)
			 ? 1 : 0;
		cs_bits = queue_and_start_write( status_buf, req.wLength, 1 );
		set_cs_bits( cs_bits );
		break;
	case GET_INTERFACE:
		printk( "%sfixme: get interface not supported\n", pszMe );
		cs_bits = queue_and_start_write( NULL, req.wLength, 0 );
		set_cs_bits( cs_bits );
		break;
	case SET_INTERFACE:
		printk( "%sfixme: set interface not supported\n", pszMe );
		CLR_EP0_OUTPKTRDY_DATAEND();
		break;
	default :
		printk("%sunknown request 0x%x\n", pszMe, req.bRequest);
		break;
	} /* switch( bRequest ) */

sh_sb_end:
	return;

}

/*
 * common_wrtie_preamble()
 * Called before execution of sh_write() or sh_write_with_empty_packet()
 * Handles common abort conditions.
 *
 */
static void common_write_preamble( void )
{
	/* If "setup end" has been set, the usb controller has
		..terminated a setup transaction before we set DE. This
		..happens during enumeration with some hosts. For example,
		..the host will ask for our device descriptor and specify
		..a return of 64 bytes. When we hand back the first 8, the
		..host will know our max packet size and turn around and
		..issue a new setup immediately. This causes the UDC to auto-ack
		..the new setup and set SE. We must then "unload" (process)
		..the new setup, which is what will happen after this preamble
		..is finished executing.
	 */
	__u32 i;
	i = INDEX_REG;
	INDEX_REG = 0x00;

	if ( EP0_CSR & EP0_SETUP_END ) {
		PRINTKD( "%swrite_preamble(): Early termination of setup\n", pszMe );
		CLR_EP0_SETUP_END();	/* clear setup end */
		current_handler = sh_setup_begin;
	}

	if ( EP0_CSR & EP0_SENT_STALL ) {
		PRINTKD( "%swrite_preamble(): UDC sent stall\n", pszMe );
		CLR_EP0_SENT_STALL();	/* clear setup end */
		current_handler = sh_setup_begin;
	}

	if ( EP0_CSR & EP0_OUT_PKT_RDY ) {
		PRINTKD( "%swrite_preamble(): see OPR. Stopping write to "
			"handle new SETUP\n", pszMe );
		/* very rarely, you can get OPR and leftover IPR. Try to clear */
		EP0_CSR = (EP0_CSR & (~EP0_WR_BITS)) & (~EP0_IN_PKT_RDY);
		current_handler = sh_setup_begin;
	}
	INDEX_REG = i;
}

/*
 * sh_write()
 * This is the setup handler when we are in the data return phase of
 * a setup request and have as much (or more) data than the host
 * requested. If we enter this routine and bytes left is zero, the
 * last data packet has gone (int is because IPR was just cleared)
 * so we just set DE and reset. Otheriwse, we write another packet
 * and set IPR.
 */
static void sh_write()
{
	__u32 i;

	PRINTKD( "W\n" );

	i = INDEX_REG;
	INDEX_REG = 0x00;

	if ( EP0_CSR & EP0_IN_PKT_RDY ) {
		PRINTKD( "%ssh_write(): IPR set, exiting\n", pszMe );
		return;
	}

	/* If bytes left is zero, we are coming in on the
		..interrupt after the last packet went out. And
		..we know we don't have to empty packet this transfer
		..so just set DE and we are done */

	if ( 0 == wr.bytes_left ) {
		/* that's it, so data end  */
		SET_EP0_DATA_END();
		wr.p = NULL;  				/* be anal */
		current_handler = sh_setup_begin;
	} else {
		/* Otherwise, more data to go */
		write_fifo();
		SET_EP0_IN_PKT_RDY();
	}
	INDEX_REG = i;
}

/*
 * sh_write_with_empty_packet()
 * This is the setup handler when we don't have enough data to
 * satisfy the host's request. After we send everything we've got
 * we must send an empty packet (by setting IPR and DE) so the
 * host can perform "short packet retirement" and not stall.
 *
 */
static void sh_write_with_empty_packet( void )
{
	__u32 i;
	PRINTKD( "WE\n" );

	i = INDEX_REG;
	INDEX_REG = 0x00;
	if ( EP0_CSR & EP0_IN_PKT_RDY ) {
		PRINTKD( "%ssh_write(): IPR set, exiting\n", pszMe );
		return;
	}

	/* If bytes left is zero, we are coming in on the
		..interrupt after the last packet went out.
		..we must do short packet suff, so set DE and IPR
	 */

	if ( 0 == wr.bytes_left ) {
		SET_EP0_INPKTRDY_DATAEND();
		wr.p = NULL;
		current_handler = sh_setup_begin;
		PRINTKD( "%ssh_write empty() Sent empty packet \n", pszMe );
	}  else {
		write_fifo();			/* send data */
		SET_EP0_IN_PKT_RDY();				/* flag a packet is ready */
	}
	EP0_CSR = 0;
	INDEX_REG = i;
}

/***************************************************************************
Other Private Subroutines
***************************************************************************/
/*
 * queue_and_start_write()
 * p == data to send
 * req == bytes host requested
 * act == bytes we actually have
 * Returns: bits to be flipped in ep0 control/status register
 *
 * Called from sh_setup_begin() to begin a data return phase. Sets up the
 * global "wr"-ite structure and load the outbound FIFO with data.
 * If can't send all the data, set appropriate handler for next interrupt.
 *
 */
static __u32  queue_and_start_write( void * in, int req, int act )
{
	__u32 i, cs_reg_bits = EP0_IN_PKT_RDY;
	unsigned char * p = (unsigned char*) in;

	PRINTKD( "Qr=%d a=%d\n",req,act );

	i = INDEX_REG;
	INDEX_REG = 0x00;

	/* thou shalt not enter data phase until the serviced OUT is clear */
	CLR_EP0_OUT_PKT_RDY();
	if ( EP0_CSR & EP0_OUT_PKT_RDY ) {
		printk( "%sSO did not clear OPR\n", pszMe );
		return ( EP0_DATA_END | EP0_SERVICED_OUT_PKT_RDY ) ;
	}
	INDEX_REG = i;
	wr.p = p;
	wr.bytes_left = MIN( act, req );

	write_fifo();

	if ( 0 == wr.bytes_left ) {
		cs_reg_bits |= EP0_DATA_END;	/* out in 1 so data end */
		wr.p = NULL;  			/* be anal */
	}
	else if ( act < req )   /* we are going to short-change host */
		current_handler = sh_write_with_empty_packet; /* so need nul to not stall */
	else /* we have as much or more than requested */
		current_handler = sh_write;

	return cs_reg_bits; /* note: IPR was set uncondtionally at start of routine */
}

/*
 * write_fifo()
 * Stick bytes in the 8 bytes endpoint zero FIFO.
 * This version uses a variety of tricks to make sure the bytes
 * are written correctly. 1. The count register is checked to
 * see if the byte went in, and the write is attempted again
 * if not. 2. An overall counter is used to break out so we
 * don't hang in those (rare) cases where the UDC reverses
 * direction of the FIFO underneath us without notification
 * (in response to host aborting a setup transaction early).
 *
 */
static void write_fifo( void )
{
	int bytes_this_time = MIN( wr.bytes_left, 8 );
	int bytes_written = 0;
	int i=0, ind;

	PRINTKD( "WF=%d: ", bytes_this_time );

	ind = INDEX_REG;
	INDEX_REG = 0x00;

	while( bytes_this_time-- ) {
		PRINTKD( "%2.2X ", *wr.p );
		i = 0;
		do {
			EP0_FIFO_REG = *wr.p;
			udelay(1);
			i++;
		} while( OUT_FIFO_CNT1_REG == bytes_written && i < 10 );
		if ( i == 10 ) {
			printk( "%swrite_fifo: write failure\n", pszMe );
			usbd_info.stats.ep0_fifo_write_failures++;
		}
		wr.p++;
		bytes_written++;
	}
	wr.bytes_left -= bytes_written;

	/* following propagation voodo so maybe caller writing IPR in
		..a moment might actually get it to stick 28Feb01ww */
	udelay( 300 );
	usbd_info.stats.ep0_bytes_written += bytes_written;
	PRINTKD( "L=%d WCR=%8.8X\n", wr.bytes_left, OUT_FIFO_CNT1_REG );
	INDEX_REG = ind;
}

/*
 * read_fifo()
 * Read 1-8 bytes out of FIFO and put in request.
 * Called to do the initial read of setup requests
 * from the host. Return number of bytes read.
 *
 * Like write fifo above, this driver uses multiple
 * reads checked agains the count register with an
 * overall timeout.
 *
 */
static int
read_fifo( usb_dev_request_t * request )
{
	int bytes_read = 0;
	int fifo_count;
	int i, ind;

	unsigned char * pOut = (unsigned char*) request;

	ind = INDEX_REG;
	INDEX_REG = 0x00;
	fifo_count = ( OUT_FIFO_CNT1_REG & 0xFF );

	ASSERT( fifo_count <= 8 );
	PRINTKD( "RF=%d ", fifo_count );

	while( fifo_count-- ) {
		i = 0;
		do {
			*pOut = (unsigned char)(EP0_FIFO_REG & 0x00FF);
			//udelay(1);
			i++;
		} while( ( OUT_FIFO_CNT1_REG & 0xFF ) != fifo_count && i < 10 );
		if ( i == 10 ) {
			printk( "%sread_fifo(): read failure\n", pszMe );
			usbd_info.stats.ep0_fifo_read_failures++;
		}
		pOut++;
		bytes_read++;
	}

	PRINTKD( "fc=%d\n", bytes_read );
	usbd_info.stats.ep0_bytes_read += bytes_read;
	INDEX_REG = ind;
	return bytes_read;
}

/*
 * get_descriptor()
 * Called from sh_setup_begin to handle data return
 * for a GET_DESCRIPTOR setup request.
 */
static void get_descriptor( usb_dev_request_t * pReq )
{
	__u32 cs_bits = 0;
	string_desc_t * pString;
	ep_desc_t * pEndpoint;

	desc_t * pDesc = mmsp2_usb_get_descriptor_ptr();
	int type = pReq->wValue >> 8;
	int idx  = pReq->wValue & 0xFF;

	switch( type ) {
	case USB_DESC_DEVICE:
		cs_bits = queue_and_start_write( &pDesc->dev, pReq->wLength,
			pDesc->dev.bLength );
		break;

		// return config descriptor buffer, cfg, intf, 2 ep
	case USB_DESC_CONFIG:
		cs_bits = queue_and_start_write( &pDesc->b, pReq->wLength,
			sizeof( struct cdb ) );
		break;

		// not quite right, since doesn't do language code checking
	case USB_DESC_STRING:
		pString = mmsp2_usb_get_string_descriptor( idx );
		if ( pString ) {
			if ( idx != 0 ) {  // if not language index
				//printk( "%sReturn string %d: ", pszMe, idx );
				psdesc( pString );
			}
			cs_bits = queue_and_start_write( pString, pReq->wLength,
				pString->bLength );
		}
		else {
			printk("%sunkown string index %d Stall.\n", pszMe, idx );
			cs_bits = ( EP0_DATA_END | EP0_SERVICED_OUT_PKT_RDY | EP0_SEND_STALL );
		}
		  break;

	case USB_DESC_INTERFACE:
		if ( idx == pDesc->b.intf.bInterfaceNumber ) {
			cs_bits = queue_and_start_write( &pDesc->b.intf, pReq->wLength,
				pDesc->b.intf.bLength );
		}
		break;

	case USB_DESC_ENDPOINT: /* correct? 21Feb01ww */
		if ( idx == 1 )
			pEndpoint = &pDesc->b.ep1;
		else if ( idx == 2 )
			pEndpoint = &pDesc->b.ep2;
		else
			pEndpoint = NULL;
		if ( pEndpoint ) {
			cs_bits = queue_and_start_write( pEndpoint, pReq->wLength,
				pEndpoint->bLength );
		} else {
			printk("%sunkown endpoint index %d Stall.\n", pszMe, idx );
			cs_bits = ( EP0_DATA_END | EP0_SERVICED_OUT_PKT_RDY | EP0_SEND_STALL );
		}
		break;

	default :
		printk("%sunknown descriptor type %d. Stall.\n", pszMe, type );
		cs_bits = ( EP0_DATA_END | EP0_SERVICED_OUT_PKT_RDY | EP0_SEND_STALL );
		break;

	}
	set_cs_bits( cs_bits );
}

/* some voodo I am adding, since the vanilla macros just aren't doing it  1Mar01ww */

#define BOTH_BITS (EP0_IN_PKT_RDY | EP0_DATA_END)

static void set_cs_bits( __u32 bits )
{
	__u32 ind;

	ind = INDEX_REG;
	INDEX_REG = 0x00;
	if ( bits & ( EP0_SERVICED_SETUP_END | EP0_SERVICED_OUT_PKT_RDY | EP0_SEND_STALL ) ) {
		EP0_CSR = (EP0_CSR & (~EP0_WR_BITS)) | bits;
	}
	else if ( (bits & BOTH_BITS) == BOTH_BITS )
		SET_EP0_INPKTRDY_DATAEND();
	else if ( bits & EP0_IN_PKT_RDY )
		SET_EP0_IN_PKT_RDY();
	else if ( bits & EP0_DATA_END )
		SET_EP0_DATA_END();
	INDEX_REG = ind;
}

/* end usb_ep0.c */

