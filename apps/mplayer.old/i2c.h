#ifndef _I2C_H_
#define _I2C_H_

typedef struct _i2c_write_byte_info {
	unsigned char id;
	unsigned char addr;
	unsigned char data;
} I2C_WRITE_BYTE_INFO;

typedef struct _i2c_read_byte_info {
	unsigned char id;
	unsigned char addr;
	unsigned char *pdata;
} I2C_READ_BYTE_INFO;

#endif // _I2C_H_
