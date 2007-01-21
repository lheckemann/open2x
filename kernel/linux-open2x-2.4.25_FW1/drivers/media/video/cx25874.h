#ifndef _CX25874_H_
#define _CX25874_H_

#include "i2c.h"

/* CX25874 & I2C IOCTL Commands */
#define CX25874_I2C_WRITE_BYTE	_IOW('v', 0x00, I2C_WRITE_BYTE_INFO)
#define CX25874_I2C_READ_BYTE	_IOW('v', 0x01, I2C_READ_BYTE_INFO)
#define IOCTL_CX25874_DISPLAY_MODE_SET	_IOW('v', 0x02, unsigned char)

#define IOCTL_CX25874_TV_MODE_POSITION	_IOW('v', 0x0A, unsigned char)		

#define TV_POS_LEFT		0
#define TV_POS_RIGHT	1
#define TV_POS_UP		2
#define TV_POS_DOWN		3


#define CX25874_ID 	0x8A

#define DISPLAY_LCD 0x1
#define DISPLAY_MONITOR 0x2
#define DISPLAY_TV_NTSC 0x3
#define DISPLAY_TV_PAL 	0x4
#define DISPLAY_TV_GAME_NTSC 0x05

#define TV_MODE_NTSC			0
#define TV_MODE_PAL				1

#endif	// _CX25874_H_
