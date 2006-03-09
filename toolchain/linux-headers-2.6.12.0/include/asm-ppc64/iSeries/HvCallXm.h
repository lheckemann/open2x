//============================================================================
//							 Header File Id
// Name______________:	HvCallXm.H
//
// Description_______:
//
//	This file contains the "hypervisor call" interface which is used to
//	drive the hypervisor from SLIC.
//
//============================================================================
#ifndef _HVCALLXM_H
#define _HVCALLXM_H

//-------------------------------------------------------------------
// Forward declarations 
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// Standard Includes
//-------------------------------------------------------------------
#include <asm/iSeries/HvCallSc.h>
#include <asm/iSeries/HvTypes.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

#define HvCallXmGetTceTableParms	HvCallXm +  0
#define HvCallXmTestBus			HvCallXm +  1
#define HvCallXmConnectBusUnit		HvCallXm +  2
#define HvCallXmLoadTod			HvCallXm +  8
#define HvCallXmTestBusUnit		HvCallXm +  9
#define HvCallXmSetTce			HvCallXm + 11
#define HvCallXmSetTces			HvCallXm + 13



//============================================================================
static inline void		HvCallXm_getTceTableParms(__u64 cb)
{
	HvCall1(HvCallXmGetTceTableParms, cb);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
}
//============================================================================
static inline __u64		HvCallXm_setTce(__u64 tceTableToken, __u64 tceOffset, __u64 tce)
{	
	__u64 retval = HvCall3(HvCallXmSetTce, tceTableToken, tceOffset, tce );
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retval;
}
//============================================================================
static inline __u64		HvCallXm_setTces(__u64 tceTableToken, __u64 tceOffset, __u64 numTces, __u64 tce1, __u64 tce2, __u64 tce3, __u64 tce4)
{	
	__u64 retval = HvCall7(HvCallXmSetTces, tceTableToken, tceOffset, numTces,
			     tce1, tce2, tce3, tce4 );
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retval;
}
//=============================================================================
static inline __u64	HvCallXm_testBus(__u16 busNumber)
{
	__u64 retVal = HvCall1(HvCallXmTestBus, busNumber);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retVal;
}
//=====================================================================================
static inline __u64	HvCallXm_testBusUnit(__u16 busNumber, __u8 subBusNumber, __u8 deviceId)
{
	__u64 busUnitNumber = (subBusNumber << 8) | deviceId;
	__u64 retVal = HvCall2(HvCallXmTestBusUnit, busNumber, busUnitNumber);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retVal;
}
//=====================================================================================
static inline __u64	HvCallXm_connectBusUnit(__u16 busNumber, __u8 subBusNumber, __u8 deviceId,
						__u64 interruptToken)
{
	__u64 busUnitNumber = (subBusNumber << 8) | deviceId;
	__u64 queueIndex = 0; // HvLpConfig::mapDsaToQueueIndex(HvLpDSA(busNumber, xBoard, xCard));  

	__u64 retVal = HvCall5(HvCallXmConnectBusUnit, busNumber, busUnitNumber,
			     interruptToken, 0, queueIndex);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retVal;
}
//=====================================================================================
static inline __u64	HvCallXm_loadTod(void)
{
	__u64 retVal = HvCall0(HvCallXmLoadTod);
	// getPaca()->adjustHmtForNoOfSpinLocksHeld();
	return retVal;
}
//=====================================================================================

#endif /* _HVCALLXM_H */
