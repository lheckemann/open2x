#ifndef _OV9640_H_
#define _OV9640_H_

#include "i2c.h"

/* OV9640 & I2C IOCTL Commands */
#define OV9640_I2C_WRITE_BYTE	_IOW('v', 0x00, I2C_WRITE_BYTE_INFO)
#define OV9640_I2C_READ_BYTE	_IOW('v', 0x01, I2C_READ_BYTE_INFO)
#define IOCTL_OV9640_RESET		_IO('v', 0x02)
#define IOCTL_OV9640_CAMADDR	_IOR('v', 0x03, unsigned long)

#define OV9640_ID 	0x60

#endif	// _OV9640_H_
