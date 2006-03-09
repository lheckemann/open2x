/************************************************************************/
/* Provides the Hypervisor PCI calls for iSeries Linux Parition.        */
/* Copyright (C) 2001  <Wayne G Holm> <IBM Corporation>                 */
/*                                                                      */
/* This program is free software; you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation; either version 2 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */ 
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */ 
/* along with this program; if not, write to the:                       */
/* Free Software Foundation, Inc.,                                      */ 
/* 59 Temple Place, Suite 330,                                          */ 
/* Boston, MA  02111-1307  USA                                          */
/************************************************************************/
/* Change Activity:                                                     */
/*   Created, Jan 9, 2001                                               */
/************************************************************************/

#ifndef _HVCALLPCI_H
#define _HVCALLPCI_H

#include <asm/iSeries/HvCallSc.h>
#include <asm/iSeries/HvTypes.h>

/*
 * DSA == Direct Select Address
 * this struct must be 64 bits in total
 */
struct HvCallPci_DsaAddr {
	__u16		busNumber;		/* PHB index? */
	__u8		subBusNumber; 		/* PCI bus number? */
	__u8		deviceId;     		/* device and function? */
	__u8		barNumber;
	__u8		reserved[3];
};

union HvDsaMap {
	__u64	DsaAddr;
	struct HvCallPci_DsaAddr Dsa;
};

struct HvCallPci_LoadReturn {
	__u64		rc;
	__u64		value;
};

enum HvCallPci_DeviceType {
	HvCallPci_NodeDevice	= 1,
	HvCallPci_SpDevice	= 2,	
	HvCallPci_IopDevice     = 3,	
	HvCallPci_BridgeDevice	= 4,	
	HvCallPci_MultiFunctionDevice = 5,	
	HvCallPci_IoaDevice	= 6	
};


struct HvCallPci_DeviceInfo {
	__u32	deviceType;		// See DeviceType enum for values
};
    
struct HvCallPci_BusUnitInfo {
	__u32	sizeReturned;		// length of data returned
	__u32	deviceType;		// see DeviceType enum for values
};

struct HvCallPci_BridgeInfo {
	struct HvCallPci_BusUnitInfo busUnitInfo;  // Generic bus unit info
	__u8		subBusNumber;		// Bus number of secondary bus
	__u8		maxAgents;		// Max idsels on secondary bus
        __u8              maxSubBusNumber;        // Max Sub Bus
	__u8		logicalSlotNumber;	// Logical Slot Number for IOA 
};
    

//  Maximum BusUnitInfo buffer size.  Provided for clients so they can allocate
//  a buffer big enough for any type of bus unit.  Increase as needed.
enum {HvCallPci_MaxBusUnitInfoSize = 128};

struct HvCallPci_BarParms {
	__u64		vaddr;
	__u64		raddr;
	__u64		size;
	__u64		protectStart;
	__u64		protectEnd;
	__u64		relocationOffset;
	__u64		pciAddress;		
	__u64		reserved[3];
};					

enum HvCallPci_VpdType {
	HvCallPci_BusVpd		= 1,
	HvCallPci_BusAdapterVpd	= 2
};

#define HvCallPciConfigLoad8		HvCallPci + 0
#define HvCallPciConfigLoad16		HvCallPci + 1
#define HvCallPciConfigLoad32		HvCallPci + 2
#define HvCallPciConfigStore8		HvCallPci + 3
#define HvCallPciConfigStore16		HvCallPci + 4
#define HvCallPciConfigStore32		HvCallPci + 5
#define HvCallPciEoi			HvCallPci + 16
#define HvCallPciGetBarParms		HvCallPci + 18
#define HvCallPciMaskFisr		HvCallPci + 20
#define HvCallPciUnmaskFisr		HvCallPci + 21
#define HvCallPciSetSlotReset		HvCallPci + 25
#define HvCallPciGetDeviceInfo		HvCallPci + 27
#define HvCallPciGetCardVpd		HvCallPci + 28
#define HvCallPciBarLoad8		HvCallPci + 40
#define HvCallPciBarLoad16		HvCallPci + 41
#define HvCallPciBarLoad32		HvCallPci + 42
#define HvCallPciBarLoad64		HvCallPci + 43
#define HvCallPciBarStore8		HvCallPci + 44
#define HvCallPciBarStore16		HvCallPci + 45
#define HvCallPciBarStore32		HvCallPci + 46
#define HvCallPciBarStore64		HvCallPci + 47
#define HvCallPciMaskInterrupts		HvCallPci + 48
#define HvCallPciUnmaskInterrupts	HvCallPci + 49
#define HvCallPciGetBusUnitInfo		HvCallPci + 50

//============================================================================
static inline __u64 HvCallPci_configLoad8(__u16 busNumber, __u8 subBusNumber,
					__u8 deviceId, __u32 offset,
					__u8 *value)
{
	struct HvCallPci_DsaAddr dsa;
	struct HvCallPci_LoadReturn retVal;

	*((__u64*)&dsa) = 0;				

	dsa.busNumber = busNumber;
	dsa.subBusNumber = subBusNumber;
	dsa.deviceId = deviceId;

	HvCall3Ret16(HvCallPciConfigLoad8, &retVal, *(__u64 *)&dsa, offset, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	*value = retVal.value;

	return retVal.rc;
}
//============================================================================
static inline __u64 HvCallPci_configLoad16(__u16 busNumber, __u8 subBusNumber,
					 __u8 deviceId, __u32 offset,
					 __u16 *value)
{
	struct HvCallPci_DsaAddr dsa;
	struct HvCallPci_LoadReturn retVal;

	*((__u64*)&dsa) = 0;				

	dsa.busNumber = busNumber;
	dsa.subBusNumber = subBusNumber;
	dsa.deviceId = deviceId;

	HvCall3Ret16(HvCallPciConfigLoad16, &retVal, *(__u64 *)&dsa, offset, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	*value = retVal.value;

	return retVal.rc;
}
//============================================================================
static inline __u64	HvCallPci_configLoad32(__u16 busNumber, __u8 subBusNumber,
					      __u8 deviceId, __u32 offset,
					      __u32 *value)
{
	struct HvCallPci_DsaAddr dsa;
	struct HvCallPci_LoadReturn retVal;

	*((__u64*)&dsa) = 0;				

	dsa.busNumber = busNumber;
	dsa.subBusNumber = subBusNumber;
	dsa.deviceId = deviceId;

	HvCall3Ret16(HvCallPciConfigLoad32, &retVal, *(__u64 *)&dsa, offset, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	*value = retVal.value;

	return retVal.rc;
}
//============================================================================
static inline __u64	HvCallPci_configStore8(__u16 busNumber, __u8 subBusNumber,
					      __u8 deviceId, __u32 offset,
					      __u8  value)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;				

	dsa.busNumber = busNumber;
	dsa.subBusNumber = subBusNumber;
	dsa.deviceId = deviceId;

	retVal = HvCall4(HvCallPciConfigStore8, *(__u64 *)&dsa, offset, value, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================
static inline __u64	HvCallPci_configStore16(__u16 busNumber, __u8 subBusNumber,
					      __u8 deviceId, __u32 offset,
					      __u16  value)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;				

	dsa.busNumber = busNumber;
	dsa.subBusNumber = subBusNumber;
	dsa.deviceId = deviceId;

	retVal = HvCall4(HvCallPciConfigStore16, *(__u64 *)&dsa, offset, value, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================
static inline __u64	HvCallPci_configStore32(__u16 busNumber, __u8 subBusNumber,
					      __u8 deviceId, __u32 offset,
					      __u32  value)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;				

	dsa.busNumber = busNumber;
	dsa.subBusNumber = subBusNumber;
	dsa.deviceId = deviceId;

	retVal = HvCall4(HvCallPciConfigStore32, *(__u64 *)&dsa, offset, value, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================
static inline __u64	HvCallPci_barLoad8(__u16	busNumberParm,
					   __u8		subBusParm,
					   __u8		deviceIdParm,
					   __u8		barNumberParm,
					   __u64		offsetParm,
					   __u8*		valueParm)
{
	struct HvCallPci_DsaAddr dsa;
	struct HvCallPci_LoadReturn retVal;

	*((__u64*)&dsa) = 0;				

	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;
	dsa.barNumber = barNumberParm;

	HvCall3Ret16(HvCallPciBarLoad8, &retVal, *(__u64 *)&dsa, offsetParm, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	*valueParm = retVal.value;

	return retVal.rc;
}
//============================================================================
static inline __u64	HvCallPci_barLoad16(__u16	busNumberParm,
					   __u8		subBusParm,
					   __u8		deviceIdParm,
					   __u8		barNumberParm,
					   __u64		offsetParm,
					   __u16*		valueParm)
{
	struct HvCallPci_DsaAddr dsa;
	struct HvCallPci_LoadReturn retVal;

	*((__u64*)&dsa) = 0;				

	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;
	dsa.barNumber = barNumberParm;

	HvCall3Ret16(HvCallPciBarLoad16, &retVal, *(__u64 *)&dsa, offsetParm, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	*valueParm = retVal.value;

	return retVal.rc;
}
//============================================================================
static inline __u64	HvCallPci_barLoad32(__u16	busNumberParm,
					   __u8		subBusParm,
					   __u8		deviceIdParm,
					   __u8		barNumberParm,
					   __u64		offsetParm,
					   __u32*		valueParm)
{
	struct HvCallPci_DsaAddr dsa;
	struct HvCallPci_LoadReturn retVal;

	*((__u64*)&dsa) = 0;				

	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;
	dsa.barNumber = barNumberParm;

	HvCall3Ret16(HvCallPciBarLoad32, &retVal, *(__u64 *)&dsa, offsetParm, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	*valueParm = retVal.value;

	return retVal.rc;
}
//============================================================================
static inline __u64	HvCallPci_barLoad64(__u16	busNumberParm,
					   __u8		subBusParm,
					   __u8		deviceIdParm,
					   __u8		barNumberParm,
					   __u64		offsetParm,
					   __u64*		valueParm)
{
	struct HvCallPci_DsaAddr dsa;
	struct HvCallPci_LoadReturn retVal;

	*((__u64*)&dsa) = 0;				

	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;
	dsa.barNumber = barNumberParm;

	HvCall3Ret16(HvCallPciBarLoad64, &retVal, *(__u64 *)&dsa, offsetParm, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	*valueParm = retVal.value;

	return retVal.rc;
}
//============================================================================
static inline __u64	HvCallPci_barStore8(__u16	busNumberParm,
					    __u8		subBusParm,
					    __u8		deviceIdParm,
					    __u8		barNumberParm,
					    __u64		offsetParm,
					    __u8		valueParm)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;
				
	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;
	dsa.barNumber = barNumberParm;

	retVal = HvCall4(HvCallPciBarStore8, *(__u64 *)&dsa, offsetParm, valueParm, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================
static inline __u64	HvCallPci_barStore16(__u16	busNumberParm,
					     __u8		subBusParm,
					     __u8		deviceIdParm,
					     __u8		barNumberParm,
					     __u64	offsetParm,
					     __u16	valueParm)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;
				
	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;
	dsa.barNumber = barNumberParm;

	retVal = HvCall4(HvCallPciBarStore16, *(__u64 *)&dsa, offsetParm, valueParm, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================
static inline __u64	HvCallPci_barStore32(__u16	busNumberParm,
					     __u8		subBusParm,
					     __u8		deviceIdParm,
					     __u8		barNumberParm,
					     __u64	offsetParm,
					     __u32	valueParm)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;
				
	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;
	dsa.barNumber = barNumberParm;

	retVal = HvCall4(HvCallPciBarStore32, *(__u64 *)&dsa, offsetParm, valueParm, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================
static inline __u64	HvCallPci_barStore64(__u16	busNumberParm,
					     __u8		subBusParm,
					     __u8		deviceIdParm,
					     __u8		barNumberParm,
					     __u64	offsetParm,
					     __u64	valueParm)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;
				
	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;
	dsa.barNumber = barNumberParm;

	retVal = HvCall4(HvCallPciBarStore64, *(__u64 *)&dsa, offsetParm, valueParm, 0);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================
static inline __u64	HvCallPci_eoi(__u16	busNumberParm,
				      __u8	subBusParm,  
				      __u8	deviceIdParm)
{
	struct HvCallPci_DsaAddr dsa;
	struct HvCallPci_LoadReturn retVal;

	*((__u64*)&dsa) = 0;

	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;

	HvCall1Ret16(HvCallPciEoi, &retVal, *(__u64*)&dsa);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal.rc;
}
//============================================================================
static inline __u64	HvCallPci_getBarParms(__u16	busNumberParm,
					      __u8	subBusParm,  
					      __u8	deviceIdParm,
					      __u8	barNumberParm,
					      __u64	parms,
					      __u32	sizeofParms)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;

	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;
	dsa.barNumber = barNumberParm;

	retVal = HvCall3(HvCallPciGetBarParms, *(__u64*)&dsa, parms, sizeofParms);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================
static inline __u64	HvCallPci_maskFisr(__u16	busNumberParm,
					   __u8	subBusParm,  
					   __u8	deviceIdParm,
					   __u64	fisrMask)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;		

	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;

	retVal = HvCall2(HvCallPciMaskFisr, *(__u64*)&dsa, fisrMask);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================
static inline __u64	HvCallPci_unmaskFisr(__u16	busNumberParm,
					     __u8		subBusParm,  
					     __u8		deviceIdParm,
					     __u64	fisrMask)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;		

	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;

	retVal = HvCall2(HvCallPciUnmaskFisr, *(__u64*)&dsa, fisrMask);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================
static inline __u64	HvCallPci_setSlotReset(__u16		busNumberParm,
					       __u8		subBusParm,
					       __u8		deviceIdParm,
					       __u64		onNotOff)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;

	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;

	retVal = HvCall2(HvCallPciSetSlotReset, *(__u64*)&dsa, onNotOff);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================
static inline __u64	HvCallPci_getDeviceInfo(__u16	busNumberParm,
						__u8	subBusParm,  
						__u8	deviceNumberParm,
						__u64     parms,
						__u32	sizeofParms)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;

	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceNumberParm << 4;

	retVal = HvCall3(HvCallPciGetDeviceInfo, *(__u64*)&dsa, parms, sizeofParms);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================
static inline __u64	HvCallPci_maskInterrupts(__u16	busNumberParm,
						 __u8	subBusParm,  
						 __u8	deviceIdParm,
						 __u64	interruptMask)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;		

	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;

	retVal = HvCall2(HvCallPciMaskInterrupts, *(__u64*)&dsa, interruptMask);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================
static inline __u64	HvCallPci_unmaskInterrupts(__u16	busNumberParm,
						 __u8		subBusParm,  
						 __u8		deviceIdParm,
						 __u64		interruptMask)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;		

	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;

	retVal = HvCall2(HvCallPciUnmaskInterrupts, *(__u64*)&dsa, interruptMask);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================

static inline __u64	HvCallPci_getBusUnitInfo(__u16		busNumberParm,
						 __u8		subBusParm,  
						 __u8		deviceIdParm,
						 __u64            parms,
						 __u32		sizeofParms)
{
	struct HvCallPci_DsaAddr dsa;
	__u64 retVal;

	*((__u64*)&dsa) = 0;		

	dsa.busNumber = busNumberParm;
	dsa.subBusNumber = subBusParm;
	dsa.deviceId = deviceIdParm;

	retVal = HvCall3(HvCallPciGetBusUnitInfo, *(__u64*)&dsa, parms, sizeofParms);

	// getPaca()->adjustHmtForNoOfSpinLocksHeld();

	return retVal;
}
//============================================================================

static inline int HvCallPci_getBusVpd(__u16 busNumParm, __u64 destParm, __u16 sizeParm)
{
	int xRetSize;
	__u64 xRc = HvCall4(HvCallPciGetCardVpd, busNumParm, destParm, sizeParm, HvCallPci_BusVpd);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	if (xRc == -1)
		xRetSize = -1;
	else
		xRetSize = xRc & 0xFFFF;
	return xRetSize;
}
//============================================================================

static inline int HvCallPci_getBusAdapterVpd(__u16 busNumParm, __u64 destParm, __u16 sizeParm)
{
	int xRetSize;
	__u64 xRc = HvCall4(HvCallPciGetCardVpd, busNumParm, destParm, sizeParm, HvCallPci_BusAdapterVpd);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	if (xRc == -1)
		xRetSize = -1;
	else
		xRetSize = xRc & 0xFFFF;
	return xRetSize;
}
//============================================================================
#endif /* _HVCALLPCI_H */
