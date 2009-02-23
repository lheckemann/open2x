/*              
 *  DIGNSYS Inc. < www.dignsys.com >
 *  	Created, Developing from April 2005
 */

#ifndef _CX25874_H_
#define _CX25874_H_

#include "i2c.h"


#define CX25874_I2C_WRITE_BYTE	_IOW('v', 0x00, I2C_WRITE_BYTE_INFO)
#define CX25874_I2C_READ_BYTE	_IOW('v', 0x01, I2C_READ_BYTE_INFO)
#define IOCTL_CX25874_DISPLAY_MODE_SET	_IOW('v', 0x02, unsigned char)

#define CX25874_ID 	0x8A

#define DISPLAY_LCD 0x1
#define DISPLAY_MONITOR 0x2
#define DISPLAY_TV 0x3

#endif	
