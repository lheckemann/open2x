#ifndef _SAA7113H_H_
#define _SAA7113H_H_

#include "i2c.h"

/* SAA7113H & I2C IOCTL Commands */
#define SAA7113H_I2C_WRITE_BYTE	_IOW('v', 0x00, I2C_WRITE_BYTE_INFO)
#define SAA7113H_I2C_READ_BYTE	_IOW('v', 0x01, I2C_READ_BYTE_INFO)
#define SAA7113H_SET_SOURCE		_IOW('v', 0x02, unsigned long)
//#define IOCTL_SAA7113H_RESET		_IO('v', 0x02)
//#define IOCTL_SAA7113H_CAMADDR	_IOR('v', 0x03, unsigned long)

#define SAA7113H_ID 	0x4A

#define ISPSOURCE_SVIDEO_INPUT 		0
#define ISPSOURCE_COMPOSITE_INPUT 	1

#endif	// _SAA7113H_H_
