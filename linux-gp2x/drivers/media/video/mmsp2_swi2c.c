/*
 *  MMSP2 Software I2C Driver using GPIO
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#	define SCCB_SCL_INPUT	set_gpio_ctrl(GPIO_SCL, GPIOMD_IN, GPIOPU_NOSET)
#	define SCCB_SCL_OUTPUT	set_gpio_ctrl(GPIO_SCL, GPIOMD_OUT, GPIOPU_NOSET)
#	define SCCB_SCL_LOW		write_gpio_bit(GPIO_SCL, 0)
#	define SCCB_SCL_HIGH	write_gpio_bit(GPIO_SCL, 1)
#	define SCCB_SCL_DATA	read_gpio_bit(GPIO_SCL)
                                
#	define SCCB_SDA_INPUT	set_gpio_ctrl(GPIO_SDA, GPIOMD_IN, GPIOPU_NOSET)
#	define SCCB_SDA_OUTPUT	set_gpio_ctrl(GPIO_SDA, GPIOMD_OUT, GPIOPU_NOSET)
#	define SCCB_SDA_LOW		write_gpio_bit(GPIO_SDA, 0)
#	define SCCB_SDA_HIGH	write_gpio_bit(GPIO_SDA, 1)
#	define SCCB_SDA_DATA	read_gpio_bit(GPIO_SDA)

#define SCCB_DELAY		I2C_delay()

int i2c_set;

void I2C_delay( void );
int I2C_failed(void);
int I2C_wbyte( unsigned char id, unsigned char addr, unsigned char data );
int I2C_rbyte( unsigned char id, unsigned char addr, unsigned char *pData );

void I2C_delay( void )
{
	volatile int i, j, k;
	for( i=0 ; i<20 ; i++ )
		for( j=0 ; j<10 ; j++ )
			k = i+j;
}

//==============================================================================
// I2C GPIO
//------------------------------------------------------------------------------
// SCCB Code
// 	Modified code from proto_i2c_gpio.c because OV9640 doesn't support 
//	repeated start. I2C_wbyte is same as I2C_UTIL_WriteByte
//  except to issue STOP at first, but I2C_rbyte is different 
//  from I2C_UTIL_ReadByte because SCCB doesn't support repeated start.
//		by Goofy.2004.05.20
//------------------------------------------------------------------------------

int	I2C_wbyte( unsigned char id, unsigned char addr, unsigned char data ) 
{
	int i;

	SCCB_SDA_HIGH;
	SCCB_SCL_HIGH;
	SCCB_SCL_OUTPUT;
	SCCB_SDA_OUTPUT;
	SCCB_DELAY;

	// STOP	
	SCCB_SCL_LOW;
	SCCB_SDA_LOW;
	SCCB_DELAY;
	SCCB_SCL_HIGH;
	SCCB_DELAY;
	SCCB_SDA_HIGH;

	SCCB_DELAY;
	SCCB_DELAY;
	SCCB_DELAY;
	
	// START
	SCCB_SDA_LOW;
	SCCB_DELAY;	
	SCCB_SCL_LOW;

	// Write Slave ID
	for( i=7 ; i>=0 ; i-- )
	{
		if( id & (1<<i) )			SCCB_SDA_HIGH;
		else						SCCB_SDA_LOW;
		SCCB_DELAY;	
		SCCB_SCL_HIGH;
		SCCB_DELAY;
		SCCB_SCL_LOW;
	}
	SCCB_SDA_INPUT;
	SCCB_DELAY;	
	SCCB_SCL_HIGH;
	
	if( SCCB_SDA_DATA )		
	{
//		printk(KERN_ERR"I2C_wbyte : No Ack at id(0x%02X) transfer\r\n", id );
		I2C_failed();
	}
	SCCB_DELAY;	
	
	// Write Slave Address
	SCCB_SCL_LOW;
	SCCB_SDA_OUTPUT;
	for( i=7 ; i>=0 ; i-- )
	{
		if( addr & (1<<i) )	SCCB_SDA_HIGH;
		else				SCCB_SDA_LOW;
		SCCB_DELAY;	
		SCCB_SCL_HIGH;
		SCCB_DELAY;
		SCCB_SCL_LOW;
	}
	SCCB_SDA_INPUT;
	SCCB_DELAY;	
	SCCB_SCL_HIGH;
	if( SCCB_SDA_DATA )
	{
//		printk(KERN_ERR"I2C_wbyte : No Ack at addr(0x%02X) transfer\r\n", addr );
		I2C_failed();
	}
	SCCB_DELAY;	

	// Write Slave data
	SCCB_SCL_LOW;
	SCCB_SDA_OUTPUT;
	for( i=7 ; i>=0 ; i-- )
	{
		if( data & (1<<i) )	SCCB_SDA_HIGH;
		else				SCCB_SDA_LOW;
		SCCB_DELAY;	
		SCCB_SCL_HIGH;
		SCCB_DELAY;
		SCCB_SCL_LOW;
	}
	SCCB_SDA_INPUT;
	SCCB_DELAY;	
	SCCB_SCL_HIGH;
	if( SCCB_SDA_DATA )
	{
//		printk(KERN_ERR"I2C_wbyte : No Ack at data(0x%02X) transfer\r\n", data );
		I2C_failed();
	}
	SCCB_DELAY;	
	
	// STOP	
	SCCB_SCL_LOW;
	SCCB_SDA_LOW;
	SCCB_DELAY;
	SCCB_SCL_HIGH;
	SCCB_DELAY;
	SCCB_SDA_HIGH;
	
	SCCB_SCL_INPUT;
	SCCB_SDA_INPUT;
	
	return 1;
}


//------------------------------------------------------------------------------
int I2C_rbyte( unsigned char id, unsigned char addr, unsigned char *pData ) //
{
	unsigned char temp;
	int i;
	
	SCCB_SDA_HIGH;
	SCCB_SCL_HIGH;
	SCCB_SCL_OUTPUT;
	SCCB_SDA_OUTPUT;
	SCCB_DELAY;

	// STOP	
	SCCB_SCL_LOW;
	SCCB_SDA_LOW;
	SCCB_DELAY;
	SCCB_SCL_HIGH;
	SCCB_DELAY;
	SCCB_SDA_HIGH;

	SCCB_DELAY;
	SCCB_DELAY;
	SCCB_DELAY;
	
	// START
	SCCB_SDA_LOW;
	SCCB_DELAY;	
	SCCB_SCL_LOW;
	
	// Write Slave ID
	for( i=7 ; i>=0 ; i-- )
	{
		if( id & (1<<i) )	SCCB_SDA_HIGH;
		else				SCCB_SDA_LOW;
		SCCB_DELAY;	
		SCCB_SCL_HIGH;
		SCCB_DELAY;
		SCCB_SCL_LOW;
	}
	SCCB_SDA_INPUT;
	SCCB_DELAY;	
	SCCB_SCL_HIGH;
	if( SCCB_SDA_DATA )		
	{
//		printk(KERN_ERR"I2C_rbyte : No Ack at id(0x%02X) transfer\r\n", id );
		I2C_failed();
	}
	SCCB_DELAY;	

	// Write Slave Address
	SCCB_SCL_LOW;
	SCCB_SDA_OUTPUT;
	for( i=7 ; i>=0 ; i-- )
	{
		if( addr & (1<<i) )	SCCB_SDA_HIGH;
		else				SCCB_SDA_LOW;
		SCCB_DELAY;	
		SCCB_SCL_HIGH;
		SCCB_DELAY;
		SCCB_SCL_LOW;
	}
	SCCB_SDA_INPUT;
	SCCB_DELAY;	
	SCCB_SCL_HIGH;
	if( SCCB_SDA_DATA )
	{
//		printk(KERN_ERR"I2C_rbyte : No Ack at addr(0x%02X) transfer\r\n", addr );
		I2C_failed();
	}
	SCCB_DELAY;	
	
	// STOP	
	SCCB_SCL_LOW;
	SCCB_SDA_LOW;
	SCCB_SDA_OUTPUT;
	SCCB_DELAY;
	SCCB_SCL_HIGH;
	SCCB_DELAY;
	SCCB_SDA_HIGH;

	SCCB_DELAY;
	SCCB_DELAY;
	SCCB_DELAY;
	
	// START
	SCCB_SDA_LOW;
	SCCB_DELAY;	
	SCCB_SCL_LOW;

	// Write Slave ID
	id |= 1;
	for( i=7 ; i>=0 ; i-- )
	{
		if( id & (1<<i) )	SCCB_SDA_HIGH;
		else				SCCB_SDA_LOW;
		SCCB_DELAY;	
		SCCB_SCL_HIGH;
		SCCB_DELAY;
		SCCB_SCL_LOW;
	}
	SCCB_SDA_INPUT;
	SCCB_DELAY;	
	SCCB_SCL_HIGH;
	if( SCCB_SDA_DATA )
	{
//		printk(KERN_ERR"I2C_rbyte : No Ack at Repeated Start(0x%02X) transfer\r\n", id );
		I2C_failed();
	}
	SCCB_DELAY;	
	
	// Read Data
	SCCB_SCL_LOW;
	temp = 0;
	for( i=7 ; i>=0 ; i-- )
	{
		SCCB_DELAY;
		SCCB_SCL_HIGH;
		if( SCCB_SDA_DATA )	temp |= (1<<i);
		else				temp |= (0<<i);
		SCCB_DELAY;
		SCCB_SCL_LOW;
	}
	SCCB_SDA_HIGH;		// No ACK
	SCCB_SDA_OUTPUT;
	SCCB_DELAY;
	SCCB_SCL_HIGH;
	SCCB_DELAY;
	
	// STOP	
	SCCB_SCL_LOW;
	SCCB_SDA_LOW;
	SCCB_DELAY;
	SCCB_SCL_HIGH;
	SCCB_DELAY;
	SCCB_SDA_HIGH;
	
	SCCB_SCL_INPUT;
	SCCB_SDA_INPUT;
	
	*pData = temp;

	return 1;
}

int I2C_failed(void)
{
	// STOP	
	SCCB_SCL_LOW;
	SCCB_SDA_LOW;
	SCCB_SDA_OUTPUT;
	SCCB_DELAY;
	SCCB_SCL_HIGH;
	SCCB_DELAY;
	SCCB_SDA_HIGH;
	
	SCCB_SCL_INPUT;
	SCCB_SDA_INPUT;		
	
	return 0;
}

