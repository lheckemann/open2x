/*
 * HvTypes.h
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
#ifndef _HVTYPES_H
#define _HVTYPES_H

//===========================================================================
//                                                             Header File Id
// Name______________:  HvTypes.H
//
// Description_______:
//
//	General typedefs for the hypervisor.
//
// Declared Class(es):
//
//===========================================================================

#include <asm/types.h>

//-------------------------------------------------------------------
// Typedefs
//-------------------------------------------------------------------
typedef __u8	HvLpIndex;
typedef __u16	HvLpInstanceId;
typedef __u64     HvLpTOD;
typedef __u64     HvLpSystemSerialNum;
typedef __u8      HvLpDeviceSerialNum[12];
typedef __u16     HvLpSanHwSet;
typedef __u16     HvLpBus;
typedef __u16     HvLpBoard;
typedef __u16     HvLpCard;
typedef __u8      HvLpDeviceType[4];
typedef __u8      HvLpDeviceModel[3];
typedef __u64     HvIoToken;
typedef __u8      HvLpName[8];
typedef __u32	HvIoId;
typedef __u64	HvRealMemoryIndex;
typedef __u32     HvLpIndexMap;		// Must hold HvMaxArchitectedLps bits!!!
typedef __u16	HvLpVrmIndex;
typedef __u32	HvXmGenerationId;
typedef __u8	HvLpBusPool;			
typedef __u8	HvLpSharedPoolIndex;		
typedef __u16	HvLpSharedProcUnitsX100;
typedef __u8	HvLpVirtualLanIndex;
typedef __u16	HvLpVirtualLanIndexMap;	// Must hold HvMaxArchitectedVirtualLans bits!!!
typedef __u16	HvBusNumber;		// Hypervisor Bus Number
typedef __u8	HvSubBusNumber;		// Hypervisor SubBus Number
typedef __u8	HvAgentId;		// Hypervisor DevFn


#define HVMAXARCHITECTEDLPS 32
#define HVMAXARCHITECTEDVIRTUALLANS 16
#define HVMAXARCHITECTEDVIRTUALDISKS 32
#define HVMAXARCHITECTEDVIRTUALCDROMS 8
#define HVMAXARCHITECTEDVIRTUALTAPES 8
#define HVCHUNKSIZE 256 * 1024
#define HVPAGESIZE 4 * 1024
#define HVLPMINMEGSPRIMARY 256
#define HVLPMINMEGSSECONDARY 64
#define HVCHUNKSPERMEG 4
#define HVPAGESPERMEG 256
#define HVPAGESPERCHUNK 64
 
#define HvMaxArchitectedLps 		((HvLpIndex)HVMAXARCHITECTEDLPS)
#define HvMaxArchitectedVirtualLans	((HvLpVirtualLanIndex)16)
#define HvLpIndexInvalid		((HvLpIndex)0xff)

//--------------------------------------------------------------------
// Enums for the sub-components under PLIC
// Used in HvCall  and HvPrimaryCall
//--------------------------------------------------------------------
enum   HvCallCompIds
{
	HvCallCompId = 0,
	HvCallCpuCtlsCompId = 1,
	HvCallCfgCompId = 2,
	HvCallEventCompId = 3,
	HvCallHptCompId = 4,
	HvCallPciCompId = 5,
	HvCallSlmCompId = 6,
	HvCallSmCompId = 7,
	HvCallSpdCompId = 8,
	HvCallXmCompId = 9,
	HvCallRioCompId = 10, 
	HvCallRsvd3CompId = 11,
	HvCallRsvd2CompId = 12,
	HvCallRsvd1CompId = 13,
	HvCallMaxCompId = 14,
	HvPrimaryCallCompId = 0,    
	HvPrimaryCallCfgCompId = 1,
	HvPrimaryCallPciCompId = 2,    
	HvPrimaryCallSmCompId = 3,
	HvPrimaryCallSpdCompId = 4,
	HvPrimaryCallXmCompId = 5,
	HvPrimaryCallRioCompId = 6, 
	HvPrimaryCallRsvd7CompId = 7,
	HvPrimaryCallRsvd6CompId = 8,
	HvPrimaryCallRsvd5CompId = 9,
	HvPrimaryCallRsvd4CompId = 10,
	HvPrimaryCallRsvd3CompId = 11,
	HvPrimaryCallRsvd2CompId = 12,
	HvPrimaryCallRsvd1CompId = 13,
	HvPrimaryCallMaxCompId = HvCallMaxCompId     
};

struct HvLpBufferList {
	__u64 addr;
	__u64 len;
};

#endif /* _HVTYPES_H */
