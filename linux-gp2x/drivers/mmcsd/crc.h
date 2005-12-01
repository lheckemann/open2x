/*
 * drivers/mmcsd/crc.h
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 * 
 * Header for MMC/SD CRC routines
 *
 */

#ifndef MMCSD_CRC_H
#define MMCSD_CRC_H

/* 
 * CRC functions 
 */

extern __u8 get_crc7(const __u8 *buff, int len);
extern __u16 get_ccitt_crc(const __u8 *data, int size);
extern void str2crc(const __u8 *data, __u16 *crc);
extern void crc2str(__u8 *data, const __u16 *crc);
extern void get_ccitt_crc_wide(const __u8 *data, int size, __u16 *crc);
extern void str2crc_wide(const __u8 *data, __u16 *crc);
extern void crc2str_wide(__u8 *data, const __u16 *crc);

/* 
 * Misc functions
 */

/* Contruct a command buffer with the given argument */
static inline void mmcsd_mkcmd_arg(__u8 *buff, __u8 cmd, __u32 arg)
{
	buff[0] = cmd;
	buff[1] = (arg >> 24) & 0xff;
	buff[2] = (arg >> 16) & 0xff;
	buff[3] = (arg >> 8) & 0xff;
	buff[4] = (arg & 0xff);
	buff[5] = (__u8) ((get_crc7(buff, 5) << 1) | 0x01);
}

#endif /* ! MMCSD_CRC_H */
