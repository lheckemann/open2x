/*
 * HvCallEvent.h
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

/*
 *	This file contains the "hypervisor call" interface which is used to
 *	drive the hypervisor from the OS.
 */
#ifndef _HVCALLEVENT_H
#define _HVCALLEVENT_H

/*
 * Standard Includes
 */
#include <asm/iSeries/HvCallSc.h>
#include <asm/iSeries/HvTypes.h>
#include <asm/abs_addr.h>

struct HvLpEvent;

typedef __u8 HvLpEvent_Type;
typedef __u8 HvLpEvent_AckInd;
typedef __u8 HvLpEvent_AckType;

struct	HvCallEvent_PackedParms {
	__u8		xAckType:1;
	__u8		xAckInd:1;
	__u8		xRsvd:1;
	__u8		xTargetLp:5;
	__u8		xType;
	__u16		xSubtype;
	HvLpInstanceId	xSourceInstId;
	HvLpInstanceId	xTargetInstId;
};

typedef __u8 HvLpDma_Direction;
typedef __u8 HvLpDma_AddressType;

struct	HvCallEvent_PackedDmaParms {
	__u8		xDirection:1;
	__u8		xLocalAddrType:1;
	__u8		xRemoteAddrType:1;
	__u8		xRsvd1:5;
	HvLpIndex	xRemoteLp;
	__u8		xType;
	__u8		xRsvd2;
	HvLpInstanceId	xLocalInstId;
	HvLpInstanceId	xRemoteInstId;
};

typedef __u64 HvLpEvent_Rc;
typedef __u64 HvLpDma_Rc;

#define HvCallEventAckLpEvent				HvCallEvent +  0
#define HvCallEventCancelLpEvent			HvCallEvent +  1
#define HvCallEventCloseLpEventPath			HvCallEvent +  2
#define HvCallEventDmaBufList				HvCallEvent +  3
#define HvCallEventDmaSingle				HvCallEvent +  4
#define HvCallEventDmaToSp				HvCallEvent +  5 
#define HvCallEventGetOverflowLpEvents			HvCallEvent +  6
#define HvCallEventGetSourceLpInstanceId		HvCallEvent +  7
#define HvCallEventGetTargetLpInstanceId		HvCallEvent +  8
#define HvCallEventOpenLpEventPath			HvCallEvent +  9
#define HvCallEventSetLpEventStack			HvCallEvent + 10
#define HvCallEventSignalLpEvent			HvCallEvent + 11
#define HvCallEventSignalLpEventParms			HvCallEvent + 12
#define HvCallEventSetInterLpQueueIndex			HvCallEvent + 13
#define HvCallEventSetLpEventQueueInterruptProc		HvCallEvent + 14
#define HvCallEventRouter15				HvCallEvent + 15

static inline void HvCallEvent_getOverflowLpEvents(__u8 queueIndex)
{
	HvCall1(HvCallEventGetOverflowLpEvents,queueIndex);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
}

static inline void HvCallEvent_setInterLpQueueIndex(__u8 queueIndex)
{
	HvCall1(HvCallEventSetInterLpQueueIndex,queueIndex);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
}

static inline void HvCallEvent_setLpEventStack(__u8 queueIndex,
		char *eventStackAddr, __u32 eventStackSize)
{
	__u64 abs_addr;

	abs_addr = virt_to_abs(eventStackAddr);
	HvCall3(HvCallEventSetLpEventStack, queueIndex, abs_addr,
			eventStackSize);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
}

static inline void HvCallEvent_setLpEventQueueInterruptProc(__u8 queueIndex,
		__u16 lpLogicalProcIndex)
{
	HvCall2(HvCallEventSetLpEventQueueInterruptProc, queueIndex,
			lpLogicalProcIndex);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
}

static inline HvLpEvent_Rc HvCallEvent_signalLpEvent(struct HvLpEvent *event)
{
	__u64 abs_addr;
	HvLpEvent_Rc retVal;

#ifdef DEBUG_SENDEVENT
	printk("HvCallEvent_signalLpEvent: *event = %016lx\n ",
			(unsigned long)event);
#endif
	abs_addr = virt_to_abs(event);
	retVal = (HvLpEvent_Rc)HvCall1(HvCallEventSignalLpEvent, abs_addr);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retVal;
}

static inline HvLpEvent_Rc HvCallEvent_signalLpEventFast(HvLpIndex targetLp,
		HvLpEvent_Type type, __u16 subtype, HvLpEvent_AckInd ackInd,
		HvLpEvent_AckType ackType, HvLpInstanceId sourceInstanceId,
		HvLpInstanceId targetInstanceId, __u64 correlationToken,
		__u64 eventData1, __u64 eventData2, __u64 eventData3,
		__u64 eventData4, __u64 eventData5)
{
	HvLpEvent_Rc retVal;

	// Pack the misc bits into a single Dword to pass to PLIC
	union {
		struct HvCallEvent_PackedParms	parms;
		__u64		dword;
	} packed;
	packed.parms.xAckType	= ackType;
	packed.parms.xAckInd	= ackInd;
	packed.parms.xRsvd	= 0;
	packed.parms.xTargetLp	= targetLp;
	packed.parms.xType	= type;
	packed.parms.xSubtype	= subtype;
	packed.parms.xSourceInstId	= sourceInstanceId;
	packed.parms.xTargetInstId	= targetInstanceId;

	retVal = (HvLpEvent_Rc)HvCall7(HvCallEventSignalLpEventParms,
			packed.dword, correlationToken, eventData1,eventData2,
			eventData3,eventData4, eventData5);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retVal;
}

static inline HvLpEvent_Rc HvCallEvent_ackLpEvent(struct HvLpEvent *event)
{
	__u64 abs_addr;
	HvLpEvent_Rc retVal;

	abs_addr = virt_to_abs(event);
	retVal = (HvLpEvent_Rc)HvCall1(HvCallEventAckLpEvent, abs_addr);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retVal;
}

static inline HvLpEvent_Rc HvCallEvent_cancelLpEvent(struct HvLpEvent *event)
{
	__u64 abs_addr;
	HvLpEvent_Rc retVal;

	abs_addr = virt_to_abs(event);
	retVal = (HvLpEvent_Rc)HvCall1(HvCallEventCancelLpEvent, abs_addr);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retVal;
}

static inline HvLpInstanceId HvCallEvent_getSourceLpInstanceId(
		HvLpIndex targetLp, HvLpEvent_Type type)
{
	HvLpInstanceId retVal;	

	retVal = HvCall2(HvCallEventGetSourceLpInstanceId, targetLp, type);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retVal;
}

static inline HvLpInstanceId HvCallEvent_getTargetLpInstanceId(
		HvLpIndex targetLp, HvLpEvent_Type type)
{
	HvLpInstanceId retVal;	

	retVal = HvCall2(HvCallEventGetTargetLpInstanceId, targetLp, type);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retVal;
}

static inline void HvCallEvent_openLpEventPath(HvLpIndex targetLp,
		HvLpEvent_Type type)
{
	HvCall2(HvCallEventOpenLpEventPath, targetLp, type);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
}

static inline void HvCallEvent_closeLpEventPath(HvLpIndex targetLp,
		HvLpEvent_Type type)
{
	HvCall2(HvCallEventCloseLpEventPath, targetLp, type);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
}

static inline HvLpDma_Rc HvCallEvent_dmaBufList(HvLpEvent_Type type,
		HvLpIndex remoteLp, HvLpDma_Direction direction,
		HvLpInstanceId localInstanceId,
		HvLpInstanceId remoteInstanceId,
		HvLpDma_AddressType localAddressType,
		HvLpDma_AddressType remoteAddressType,
		/* Do these need to be converted to absolute addresses? */
		__u64 localBufList, __u64 remoteBufList, __u32 transferLength)
{
	HvLpDma_Rc retVal;
	// Pack the misc bits into a single Dword to pass to PLIC
	union {
		struct HvCallEvent_PackedDmaParms	parms;
		__u64		dword;
	} packed;

	packed.parms.xDirection		= direction;
	packed.parms.xLocalAddrType	= localAddressType;
	packed.parms.xRemoteAddrType	= remoteAddressType;
	packed.parms.xRsvd1		= 0;
	packed.parms.xRemoteLp		= remoteLp;
	packed.parms.xType		= type;
	packed.parms.xRsvd2		= 0;
	packed.parms.xLocalInstId	= localInstanceId;
	packed.parms.xRemoteInstId	= remoteInstanceId;

	retVal = (HvLpDma_Rc)HvCall4(HvCallEventDmaBufList,
			packed.dword, localBufList, remoteBufList,
			transferLength);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retVal;
}

static inline HvLpDma_Rc HvCallEvent_dmaSingle(HvLpEvent_Type type,
		HvLpIndex remoteLp, HvLpDma_Direction direction,
		HvLpInstanceId localInstanceId,
		HvLpInstanceId remoteInstanceId,
		HvLpDma_AddressType localAddressType,
		HvLpDma_AddressType remoteAddressType,
		__u64 localAddrOrTce, __u64 remoteAddrOrTce, __u32 transferLength)
{
	HvLpDma_Rc retVal;
	// Pack the misc bits into a single Dword to pass to PLIC
	union {
		struct HvCallEvent_PackedDmaParms	parms;
		__u64		dword;
	} packed;

	packed.parms.xDirection		= direction;
	packed.parms.xLocalAddrType	= localAddressType;
	packed.parms.xRemoteAddrType	= remoteAddressType;
	packed.parms.xRsvd1		= 0;
	packed.parms.xRemoteLp		= remoteLp;
	packed.parms.xType		= type;
	packed.parms.xRsvd2		= 0;
	packed.parms.xLocalInstId	= localInstanceId;
	packed.parms.xRemoteInstId	= remoteInstanceId;

	retVal = (HvLpDma_Rc)HvCall4(HvCallEventDmaSingle,
			packed.dword, localAddrOrTce, remoteAddrOrTce,
			transferLength);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retVal;
}

static inline HvLpDma_Rc HvCallEvent_dmaToSp(void* local, __u32 remote,
		__u32 length, HvLpDma_Direction dir)
{
	__u64 abs_addr;
	HvLpDma_Rc retVal;

	abs_addr = virt_to_abs(local);
	retVal = (HvLpDma_Rc)HvCall4(HvCallEventDmaToSp, abs_addr, remote,
			length, dir);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retVal;
}


#endif /* _HVCALLEVENT_H */
