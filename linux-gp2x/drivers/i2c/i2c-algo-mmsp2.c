/*
 *  i2c-algo-mmsp2.c
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 *  I2C algorithm for the MP2520F I2C bus access.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/i2c.h>          /* struct i2c_msg and others */
#include <linux/i2c-id.h>

#include <asm/hardware.h>
#include "i2c-mmsp2.h"

/*
 * Set this to zero to remove all the debug statements via dead code elimination.
 */
//#define DEBUG		1

#if DEBUG
static unsigned int i2c_debug = DEBUG;
#else
#define i2c_debug	0
#endif

static int mmsp2_scan = 0;

/*********************************************************
** i2c_mmsp2_valid_messages
** <Description>
** i2c_msg의 validity check
*********************************************************/
static int i2c_mmsp2_valid_messages( struct i2c_msg msgs[], int num)
{
	int i;
	
	if (num < 1 || num > MAX_MESSAGES){
		if( i2c_debug)
			printk(KERN_INFO "Invalid number of messages (max=%d, num=%d)\n",
				MAX_MESSAGES, num);
				
		return -EINVAL;
	}

	/* check consistency of our messages */
	for (i=0;i<num;i++){
		if (&msgs[i]==NULL){
			if( i2c_debug) printk(KERN_INFO "Msgs is NULL\n");
			return -EINVAL;
		} else {
			if (msgs[i].len == 0 || msgs[i].buf == NULL){
				if( i2c_debug)printk(KERN_INFO "Length is less than zero");
				return -EINVAL;
			}
		}
	}

	return 1;
}

/**********************************************************************
** i2c_mmsp2_master_rx
** <Description>
** I2C device로부터 count Bytes만큼 읽어서 buf[]에 채운다.
** <Parameters>
** i2c_adap : i2c_adapter 주소
** msg : i2c_msg 주소
**********************************************************************/
static int i2c_mmsp2_master_rx(struct i2c_adapter *i2c_adap, struct i2c_msg *msg)
{
	int i;
	struct i2c_algo_mmsp2_data *adap = i2c_adap->algo_data;

	// Send Slave Address
	IICCON |= 0x80;		// ACK Enabled
	IICDS = (u8)( (0x7f & msg->addr) << 1 )|1;
	IICSTAT = I2C_MASTER_RX |I2C_STARTSTOP | I2C_OUT_ENB; // Master Rx mode, start transfer

	if (adap->wait_for_interrupt())
	{
		adap->stop();			// abort
		return I2C_RETRY;
	}

	for (i = 0; i < msg->len; i++) 
	{
		/* set ACK to NAK for last received byte */
		if (i == (msg->len-1)) 
		{
			IICCON &= ~(I2C_ACK_ENB | I2C_INTPEND_FLAG);	// last data read
		}else{
			IICCON &= ~I2C_INTPEND_FLAG;					// mid data read
		}
		//printk("IICCON=0x%x\n",IICCON);

		if (adap->wait_for_interrupt()){
			adap->stop();			// abort
			return I2C_RETRY;
		}
		//printk("IICDS=0x%x\n",IICDS);
		msg->buf[i] = adap->read_byte();
		
	}
	
	adap->stop();	
	
	return (i);
}

/**********************************************************************
** i2c_mmsp2_master_tx
**********************************************************************/
static int i2c_mmsp2_master_tx(struct i2c_adapter *i2c_adap, struct i2c_msg *msg)
{
	struct i2c_algo_mmsp2_data *adap = i2c_adap->algo_data;
	int i;
	int retval;

	
	// Send Slave Address
	IICCON |= 0x80;		// ACK Enabled
	IICDS = (u8)( (0x7f & msg->addr) << 1 );
	IICSTAT = I2C_MASTER_TX | I2C_STARTSTOP | I2C_OUT_ENB; // Master Tx mode, start transfer
	
	if (retval = adap->wait_for_interrupt())
	{
		adap->stop();			// abort
		return I2C_RETRY;
	}
	
	for (i = 0; i < msg->len; i++) 
	{
		adap->write_byte(msg->buf[i]);
		udelay(10);

		IICCON &= ~I2C_INTPEND_FLAG;	// data write

		if (adap->wait_for_interrupt())
		{
			adap->stop();			// abort
			return I2C_RETRY;
		}
	}
	
	adap->stop();
	return (i);
}

/**********************************************************************
** i2c_mmsp2_do_xfer
**********************************************************************/
static int i2c_mmsp2_do_xfer(struct i2c_adapter *i2c_adap, struct i2c_msg msgs[], int num)
{
	struct i2c_algo_mmsp2_data * adap;
	struct i2c_msg *pmsg=NULL;
	int i;
	int ret=0, timeout;

	
	adap = i2c_adap->algo_data;

	// bus busy check
	timeout = adap->wait_bus_not_busy();

	if (timeout) 
	{
		return I2C_RETRY;
	}

	for (i = 0;ret >= 0 && i < num; i++) 
	{
		pmsg = &msgs[i];

		/* Read */
		if (pmsg->flags & I2C_M_RD) 
		{
			/* read bytes into buffer*/
			ret = i2c_mmsp2_master_rx(i2c_adap, pmsg);
		} 
		/* Write */
		else 
		{ 
			
			/* write bytes into I2C device */
			ret = i2c_mmsp2_master_tx(i2c_adap, pmsg);
		}
		if(ret<0) return ret;
	}

	return i;
}

/**********************************************************************
** i2c_mmsp2_xfer
** <Description>
**
**********************************************************************/
static int i2c_mmsp2_xfer(struct i2c_adapter *i2c_adap, struct i2c_msg msgs[], int num)
{
	int retval;

	if(msgs[0].len == 0) return 0;
	
	retval = i2c_mmsp2_valid_messages( msgs, num);

	if( retval > 0)
	{
		int i;
		
		for (i=i2c_adap->retries; i>=0; i--)
		{
			retval = i2c_mmsp2_do_xfer(i2c_adap,msgs,num);
			
			if(retval != I2C_RETRY)
			{
				return retval;
			}
			
			if( i2c_debug)printk(KERN_INFO"Retrying transmission \n");
			udelay(100);
		}
		if( i2c_debug)printk(KERN_INFO"Retried %i times\n",i2c_adap->retries);
		return -EREMOTEIO;

	}
	
	return retval;
}

struct i2c_algorithm i2c_mmsp2_algorithm  = {
	name:                   "MMSP2-I2C-Algorithm",
	id:                     I2C_ALGO_MMSP2,
	master_xfer:            i2c_mmsp2_xfer,
	smbus_xfer:             NULL,
	slave_send:             NULL,
	slave_recv:             NULL,
	algo_control:           NULL,
};

/*
 * registering functions to load algorithms at runtime
 */
int i2c_mmsp2_add_bus(struct i2c_adapter *i2c_adap)
{
	struct i2c_algo_mmsp2_data *adap = i2c_adap->algo_data;

	printk(KERN_INFO"I2C: Adding %s.\n", i2c_adap->name);

	i2c_adap->algo = &i2c_mmsp2_algorithm;

	MOD_INC_USE_COUNT;

	/* register new adapter to i2c module... */
	i2c_add_adapter(i2c_adap);

	adap->init();

	/* scan bus */
	if(mmsp2_scan) 
	{
		int i;
		printk(KERN_INFO "I2C: Scanning bus ");
		for (i = 0x02; i < 0xff; i+=2) 
		{
			if( i==(I2C_MMSP2_SLAVE_ADDR<<1)) continue;

			if (adap->wait_bus_not_busy()) {
				printk(KERN_INFO "I2C: scanning bus %s - TIMEOUTed.\n",
						i2c_adap->name);
				return -EIO;
			}

			adap->write_byte(i);

			IICCON |= 0x80;		// ACK Enabled
			IICSTAT = I2C_MASTER_TX | I2C_STARTSTOP | I2C_OUT_ENB;

			if ((adap->wait_for_interrupt() != BUS_ERROR)) {
				if(i2c_debug)	printk(KERN_INFO "(%02x)",i>>1);
				adap->stop();
			} else {
//				printk(".");
				adap->stop();
			}
			udelay(adap->udelay);
		}
		printk("\n");
	}
	return 0;
}

int i2c_mmsp2_del_bus(struct i2c_adapter *i2c_adap)
{
	int res;
	if ((res = i2c_del_adapter(i2c_adap)) < 0)
		return res;

	MOD_DEC_USE_COUNT;

	printk(KERN_INFO "I2C: Removing %s.\n", i2c_adap->name);

	return 0;
}

static int __init i2c_algo_mmsp2_init (void)
{
	printk(KERN_INFO "I2C: MMSP2 algorithm module loaded.\n");
	return 0;
}

EXPORT_SYMBOL(i2c_mmsp2_add_bus);
EXPORT_SYMBOL(i2c_mmsp2_del_bus);

MODULE_PARM(mmsp2_scan, "i");
MODULE_PARM_DESC(mmsp2_scan, "Scan for active chips on the bus");
MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");

module_init(i2c_algo_mmsp2_init);
