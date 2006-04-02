/*
 *  i2c_mmsp2.h
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */
#ifndef _I2C_MMSP2_H_
#define _I2C_MMSP2_H_

struct i2c_algo_mmsp2_data
{
    void (*write_byte) (u8 value);
    u8   (*read_byte) (void);
    void (*stop) (void);
    int  (*wait_bus_not_busy) (void);
    int  (*wait_for_interrupt) (void);
    void (*init) (void);

	int udelay;
	int timeout;
};

#define DEF_TIMEOUT             3
#define BUS_ERROR               (-EREMOTEIO)
#define ACK_DELAY               10       /* time to delay before checking bus error */
#define MAX_MESSAGES            65536   /* maximum number of messages to send */

#define I2C_SLEEP_TIMEOUT       2       /* time to sleep for on i2c transactions */
#define I2C_RETRY               (-2000) /* an error has occurred retry transmit */
#define I2C_TRANSMIT		1
#define I2C_RECEIVE		0
#define I2C_MMSP2_SLAVE_ADDR      0x1    /* slave pxa unit address */

// i2c interrupt type
#define TXRX_FINISHED		1
#define GENERAL_CALL		2
#define SLAVE_MATCH			3
#define BUS_ARBIT_FAIL		4

#endif
