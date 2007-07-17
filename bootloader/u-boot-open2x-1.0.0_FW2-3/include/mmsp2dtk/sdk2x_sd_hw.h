#ifndef SDK2X_SD_HW_H
#define SDK2X_SD_HW_H

/* **************************************************************************
 *    Copyright (C) 2005 by Rob Brown                                       *
 *    rob@cobbleware.com                                                    *
 *                                                                          *
 *    C Header: sdk2x_sd_hw                                                 *
 *                                                                          *
 *    This program is free software; you can redistribute it and/or modify  *
 *    it under the terms of version 2 (and only version 2) of the GNU       *
 *    General Public License as published by the Free Software Foundation.  *
 *                                                                          *
 *    This program is distributed in the hope that it will be useful,       *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *    GNU General Public License for more details.                          *
 *                                                                          *
 *    You should have received a copy of the GNU General Public License     *
 *    along with this program; if not, write to the                         *
 *    Free Software Foundation, Inc.,                                       *
 *    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 * **************************************************************************/
#include <common.h>

/* If updating this list of errors, make sure that you renumber correctly, and update SDK2X_ERR_SDHW_MAX below
 * and also syscalls.c */

#define SDK2X_ERR_SDHW_MAX								(20)

#define SDK2X_ERR_SDHW_OK								(0)
#define SDK2X_ERR_SDHW_TOO_MANY_ITERATIONS		(1)
#define SDK2X_ERR_SDHW_TIMEOUT						(2)
#define SDK2X_ERR_SDHW_CRC								(3)
#define SDK2X_ERR_SDHW_GET_ID							(4)
#define SDK2X_ERR_SDHW_RW_MISMATCH					(5)
#define SDK2X_ERR_SDHW_GET_CSD						(6)
#define SDK2X_ERR_SDHW_CARD_SELECT					(7)
#define SDK2X_ERR_SDHW_SET_BLOCK_SIZE				(8)
#define SDK2X_ERR_SDHW_PARAMETER						(9)
#define SDK2X_ERR_SDHW_READ_MULTIPLE				(10)
#define SDK2X_ERR_SDHW_READ_MULTIPLE_STOP			(11)
#define SDK2X_ERR_SDHW_NOT_READY						(12)
#define SDK2X_ERR_SDHW_WRITE_MULTIPLE				(13)
#define SDK2X_ERR_SDHW_WRITE_MULTIPLE_STOP		(14)
#define SDK2X_ERR_SDHW_FIFO_TIMEOUT					(15)
#define SDK2X_ERR_SDHW_TRANSFER_ERROR				(16)
#define SDK2X_ERR_SDHW_TRANSFER_ERROR_AT_STOP	(17)
#define SDK2X_ERR_SDHW_WRITE_PROTECT				(18)
#define SDK2X_ERR_SDHW_FIFO_TIMEOUT_AT_END		(19)
#define SDK2X_ERR_SDHW_NO_RESPONSE					(20)
#define SDK2X_ERR_SDHW_UNSUPPORTED					(21)

int sdk2x_SdHwInit (void);
int sdk2x_SdReadPages (unsigned int address, void *dest, unsigned short numblocks);
int sdk2x_SdWritePages (unsigned int address, const void *source, unsigned short numblocks);

/* u-boot MMC API */
int mmc_init(int verbose);
int mmc_read(ulong src, uchar *dst, int size);
int mmc_write(uchar *src, ulong dst, int size);
int mmc2info(ulong addr);

#endif /* SDK2X_SD_HW_H */

