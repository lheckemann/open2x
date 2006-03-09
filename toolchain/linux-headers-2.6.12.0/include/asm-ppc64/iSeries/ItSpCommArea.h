/*
 * ItSpCommArea.h
 * Copyright (C) 2001  Mike Corrigan IBM Corporation
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */


#ifndef _ITSPCOMMAREA_H
#define _ITSPCOMMAREA_H


struct SpCommArea
{
	__u32	xDesc;			// Descriptor (only in new formats)	000-003
	__u8	xFormat;		// Format (only in new formats)		004-004
	__u8	xRsvd1[11];		// Reserved				005-00F
	__u64	xRawTbAtIplStart;	// Raw HW TB value when IPL is started	010-017
	__u64	xRawTodAtIplStart;	// Raw HW TOD value when IPL is started	018-01F
	__u64	xBcdTimeAtIplStart;	// BCD time when IPL is started		020-027
	__u64	xBcdTimeAtOsStart;	// BCD time when OS passed control	028-02F
	__u8	xRsvd2[80];		// Reserved				030-07F
};

extern struct SpCommArea xSpCommArea;

#endif /* _ITSPCOMMAREA_H */
