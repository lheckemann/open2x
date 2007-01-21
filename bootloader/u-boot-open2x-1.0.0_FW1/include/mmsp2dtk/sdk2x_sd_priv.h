#ifndef SDK2X_SD_PRIV_H
#define SDK2X_SD_PRIV_H

/* **************************************************************************
 *    Copyright (C) 2005 by Rob Brown                                       *
 *    rob@cobbleware.com                                                    *
 *                                                                          *
 *    C Header: sdk2x_sd_priv                                               *
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

typedef struct _tagSdk2x_SdCardInfo
{
	int Type:1; 				/* 0 = MMC, 1 = SD */
	int IsSelected:1;

	int AccessTime;
	int AccessClocks;
	unsigned short RelativeCardAddress;
	unsigned int BlockSize;
	unsigned int NumBlocks;
	unsigned int EraseBlocks;
	unsigned int TransferSpeed;
	unsigned int WPBlocks;
	unsigned int WriteProtect;
} sdk2x_SdCardInfo_t;

extern sdk2x_SdCardInfo_t sdk2x_SdCardInfo;

#endif /* SDK2X_SD_PRIV_H */

