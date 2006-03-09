/*
 * ItIplParmsReal.h
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
#ifndef _ITIPLPARMSREAL_H
#define _ITIPLPARMSREAL_H

//==============================================================================
//
//	This struct maps the IPL Parameters DMA'd from the SP.                  
//
// Warning:
//	This data must map in exactly 64 bytes and match the architecture for
//	the IPL parms
//
//=============================================================================


//-------------------------------------------------------------------
// Standard Includes
//-------------------------------------------------------------------
#include <asm/types.h>

struct ItIplParmsReal
{
	__u8	xFormat;		// Defines format of IplParms		x00-x00
	__u8	xRsvd01:6;		// Reserved				x01-x01
	__u8	xAlternateSearch:1;	// Alternate search indicator		...
	__u8	xUaSupplied:1;		// UA Supplied on programmed IPL	...
	__u8	xLsUaFormat;		// Format byte for UA			x02-x02
	__u8	xRsvd02;		// Reserved				x03-x03
	__u32	xLsUa;			// LS UA				x04-x07
	__u32	xUnusedLsLid;		// First OS LID to load			x08-x0B
	__u16	xLsBusNumber;		// LS Bus Number			x0C-x0D
	__u8	xLsCardAdr;		// LS Card Address			x0E-x0E
	__u8	xLsBoardAdr;		// LS Board Address			x0F-x0F
	__u32	xRsvd03;		// Reserved				x10-x13
	__u8	xSpcnPresent:1;		// SPCN present				x14-x14
	__u8	xCpmPresent:1;		// CPM present				...
	__u8	xRsvd04:6;		// Reserved				...
	__u8	xRsvd05:4;		// Reserved				x15-x15
	__u8	xKeyLock:4;		// Keylock setting			...
	__u8	xRsvd06:6;		// Reserved				x16-x16
	__u8	xIplMode:2;		// Ipl mode (A|B|C|D)			...
	__u8	xHwIplType;		// Fast v slow v slow EC HW IPL		x17-x17
	__u16	xCpmEnabledIpl:1;	// CPM in effect when IPL initiated	x18-x19
	__u16	xPowerOnResetIpl:1;	// Indicate POR condition		...
	__u16	xMainStorePreserved:1;	// Main Storage is preserved		...
	__u16	xRsvd07:13;		// Reserved				...
	__u16	xIplSource:16;		// Ipl source				x1A-x1B
	__u8	xIplReason:8;		// Reason for this IPL			x1C-x1C
	__u8	xRsvd08;		// Reserved				x1D-x1D
	__u16	xRsvd09;		// Reserved				x1E-x1F
	__u16	xSysBoxType;		// System Box Type			x20-x21
	__u16	xSysProcType;		// System Processor Type		x22-x23
	__u32	xRsvd10;		// Reserved				x24-x27
	__u64	xRsvd11;		// Reserved				x28-x2F
	__u64	xRsvd12;		// Reserved				x30-x37
	__u64	xRsvd13;		// Reserved				x38-x3F
};

#endif /* _ITIPLPARMSREAL_H */
