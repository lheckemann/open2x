/*
 *  i2c_adap_mmsp2.c
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 *  I2C adapter for the MMSP2 I2C bus access.
 */

#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/errno.h>

#include <asm/irq.h>
#include <asm/arch/irqs.h>              /* for IRQ_I2C */
#include <asm/hardware.h>

#include "i2c-mmsp2.h"

/*
 * Set this to zero to remove all debug statements via dead code elimination.
 */
//#define DEBUG       1

#if DEBUG
static unsigned int i2c_debug = DEBUG;
#else
#define i2c_debug	0
#endif

static int irq = 0;
static volatile int i2c_pending = 0;             /* interrupt pending when 1 */
static volatile int i2c_int_type = 0;

static wait_queue_head_t i2c_wait;
static void i2c_mmsp2_transfer( int lastbyte, int receive, int midbyte);

/* place a byte in the transmit register */
static void i2c_mmsp2_write_byte(u8 value)
{
	IICDS = value;
}

/* read byte in the receive register */
static u8 i2c_mmsp2_read_byte(void)
{
	return (u8)(IICDS);
}

static void i2c_mmsp2_stop(void)
{
	IICSTAT &= ~I2C_STARTSTOP;		// stop signal generation
	IICCON &= ~I2C_INTPEND_FLAG;	// interrupt pend flag¸¦ Áö¿î´Ù.
	udelay(100);
}

/******************************************
** i2c_mmsp2_wait_bus_not_busy
** : bus busy check
** <Return>
** 0 : not busy
** 1 : busy
*******************************************/
static int i2c_mmsp2_wait_bus_not_busy( void)
{
	int timeout = DEF_TIMEOUT;

	while (timeout-- && (IICSTAT & I2C_BUSBUSY)) {
		udelay(100); /* wait for 100 us */
	}

	return (timeout<=0);
}

/******************************************************************************
** i2c_mmsp2_wait_for_ite
** : 
******************************************************************************/
static void i2c_mmsp2_wait_for_ite(void)
{
	unsigned long flags;
	if (irq > 0) 
	{
		save_flags_cli(flags);
		if (i2c_pending == 0) 
		{
			interruptible_sleep_on_timeout(&i2c_wait, I2C_SLEEP_TIMEOUT );
		}
		i2c_pending = 0;
		restore_flags(flags);
	} else {
		udelay(100);
	}
}

/*********************************************************************************
** i2c_mmsp2_wait_for_int
** <Description>
** i2c_int_type == TXRX_FINISHED : return 0 --> data ¼Û/¼ö½Å ¿Ï·á.
**                 BUS_ARBIT_FAIL : return BUS_ERROR --> bus error ¹ß»ý.
** timeout < 0, --> return -EIO --> timeout error ¹ß»ý.
** signal_pending(current) != 0, --> return -ERESTARTSYS --> ?
**********************************************************************************/
static int i2c_mmsp2_wait_for_int(void)
{
	int timeout = DEF_TIMEOUT;

	do {
		i2c_mmsp2_wait_for_ite();
	} while (!(i2c_int_type & TXRX_FINISHED) && timeout-- && !signal_pending(current));

	udelay(ACK_DELAY);      /* this is needed for the bus error */

	if (i2c_int_type & BUS_ARBIT_FAIL)
	{
		i2c_int_type =0;
		if( i2c_debug > 2)printk("wait_for_int: error - no ack.\n");
		return BUS_ERROR;
	}

	if (signal_pending(current)) 
	{
		return (-ERESTARTSYS);
	} else if (timeout < 0) {
		if( i2c_debug > 2)printk("wait_for_int: timeout.\n");
		return(-EIO);
	} else
		return(0);
}

/*************************************************************
** i2c_mmsp2_init
*************************************************************/
static void i2c_mmsp2_init( void)
{
	
	/* abort any transfer currently under way */
	
	/* I2C Controller ¼³Á¤ */
	IICCON |= I2C_TX_CLK_SRC;		// clock source = PCLK/512
	IICCON &= ~I2C_TRNS_CLK_VAL;	// prescaler = 0;
	IICCON |= I2C_TXRXINT_ENB;		// Tx/Rx interrupt enabled

	/* clear any leftover states from prior transmissions */
	i2c_pending = 0;	// flag ÃÊ±âÈ­
	udelay(100);
}

/****************************************************************
** i2c_mmsp2_handler
****************************************************************/
static void i2c_mmsp2_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	if (IICCON & I2C_INTPEND_FLAG) 
	{
		i2c_pending = 1;

		if(IICSTAT & I2C_ARBIT_STAT_FLAG) i2c_int_type = BUS_ARBIT_FAIL;
		else if(IICSTAT & I2C_ADDR_SLV_FLAG) i2c_int_type = SLAVE_MATCH;
		else if(IICSTAT & I2C_ADDR_ZERO_FLAG) i2c_int_type = GENERAL_CALL;
		else i2c_int_type = TXRX_FINISHED;

		wake_up_interruptible(&i2c_wait);
	}
}

static int i2c_mmsp2_resource_init( void)
{
	init_waitqueue_head(&i2c_wait);

	if (request_irq(IRQ_IIC, &i2c_mmsp2_handler, SA_INTERRUPT, "I2C_MMSP2", 0) < 0) {
		irq = 0;
		if( i2c_debug)
			printk(KERN_INFO "I2C: Failed to register I2C irq %i\n", IRQ_IIC);
		return -ENODEV;
	}else{
		irq = IRQ_IIC;
		enable_irq(irq);
	}
	return 0;
}

static void i2c_mmsp2_resource_release( void)
{
	if( irq > 0)
	{
		disable_irq(irq);
		free_irq(irq,0);
		irq=0;
	}
}

static void i2c_mmsp2_inc_use(struct i2c_adapter *adap)
{
#ifdef MODULE
	MOD_INC_USE_COUNT;
#endif
}

static void i2c_mmsp2_dec_use(struct i2c_adapter *adap)
{
#ifdef MODULE
	MOD_DEC_USE_COUNT;
#endif
}

static int i2c_mmsp2_client_register(struct i2c_client *client)
{
	return 0;
}

static int i2c_mmsp2_client_unregister(struct i2c_client *client)
{
	return 0;
}

static struct i2c_algo_mmsp2_data i2c_mmsp2_data = {
	write_byte:		i2c_mmsp2_write_byte,
	read_byte:		i2c_mmsp2_read_byte,
	stop:			i2c_mmsp2_stop,
	wait_bus_not_busy:	i2c_mmsp2_wait_bus_not_busy,
	wait_for_interrupt:	i2c_mmsp2_wait_for_int,
	init:			i2c_mmsp2_init,

	udelay:			10,
	timeout:		DEF_TIMEOUT,
};

static struct i2c_adapter i2c_mmsp2_ops = {
	name:                   "MMSP2-I2C-Adapter",
	id:                     I2C_ALGO_MMSP2,
	algo_data:              &i2c_mmsp2_data,
	inc_use:                i2c_mmsp2_inc_use,
	dec_use:                i2c_mmsp2_dec_use,
	client_register:        i2c_mmsp2_client_register,
	client_unregister:      i2c_mmsp2_client_unregister,
	retries:                2,
};

extern int i2c_mmsp2_add_bus(struct i2c_adapter *);
extern int i2c_mmsp2_del_bus(struct i2c_adapter *);

static int __init i2c_adap_mmsp2_init(void)
{
	if( i2c_mmsp2_resource_init() == 0) {

		if (i2c_mmsp2_add_bus(&i2c_mmsp2_ops) < 0) {
			i2c_mmsp2_resource_release();
			printk(KERN_INFO "I2C: Failed to add bus\n");
			return -ENODEV;
		}
	} else {
		return -ENODEV;
	}

	printk(KERN_INFO "I2C: Successfully added bus\n");

	return 0;
}

static void i2c_adap_mmsp2_exit(void)
{
	i2c_mmsp2_del_bus( &i2c_mmsp2_ops);
	i2c_mmsp2_resource_release();

	printk(KERN_INFO "I2C: Successfully removed bus\n");
}

module_init(i2c_adap_mmsp2_init);
module_exit(i2c_adap_mmsp2_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
