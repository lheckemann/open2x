/*
 * IoHriMainStore.h
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

#ifndef _IOHRIMAINSTORE_H
#define _IOHRIMAINSTORE_H

/* Main Store Vpd for Condor,iStar,sStar */
struct IoHriMainStoreSegment4 {    
	__u8	msArea0Exists:1;
	__u8	msArea1Exists:1;
	__u8	msArea2Exists:1;
	__u8	msArea3Exists:1;
	__u8	reserved1:4;
	__u8	reserved2;

	__u8	msArea0Functional:1;
	__u8	msArea1Functional:1;
	__u8	msArea2Functional:1;
	__u8	msArea3Functional:1;
	__u8	reserved3:4;
	__u8	reserved4;

	__u32	totalMainStore;

	__u64	msArea0Ptr;
	__u64	msArea1Ptr;
	__u64	msArea2Ptr;
	__u64	msArea3Ptr;

	__u32	cardProductionLevel;

	__u32	msAdrHole;

	__u8	msArea0HasRiserVpd:1;
	__u8	msArea1HasRiserVpd:1;
	__u8	msArea2HasRiserVpd:1;
	__u8	msArea3HasRiserVpd:1;
	__u8	reserved5:4;	
	__u8	reserved6;
	__u16	reserved7;

	__u8	reserved8[28];

	__u64	nonInterleavedBlocksStartAdr;
	__u64	nonInterleavedBlocksEndAdr;
};

/* Main Store VPD for Power4 */
struct IoHriMainStoreChipInfo1 {
	__u32	chipMfgID	__attribute((packed));
	char	chipECLevel[4]	__attribute((packed));
};

struct IoHriMainStoreVpdIdData {
	char	typeNumber[4];
	char	modelNumber[4];
	char	partNumber[12];
	char	serialNumber[12];
};

struct IoHriMainStoreVpdFruData {
	char	fruLabel[8]	__attribute((packed));
	__u8	numberOfSlots	__attribute((packed));
	__u8	pluggingType	__attribute((packed));
	__u16	slotMapIndex	__attribute((packed));
};

struct IoHriMainStoreAdrRangeBlock {
	void *	blockStart      __attribute((packed));
	void *	blockEnd        __attribute((packed));
	__u32	blockProcChipId __attribute((packed));
};

#define MaxAreaAdrRangeBlocks 4

struct IoHriMainStoreArea4 {
	__u32	msVpdFormat			__attribute((packed));
	__u8	containedVpdType		__attribute((packed));
	__u8	reserved1			__attribute((packed));
	__u16	reserved2			__attribute((packed));

	__u64	msExists			__attribute((packed));
	__u64	msFunctional			__attribute((packed));

	__u32	memorySize			__attribute((packed));
	__u32	procNodeId			__attribute((packed));

	__u32	numAdrRangeBlocks		__attribute((packed));
	struct IoHriMainStoreAdrRangeBlock xAdrRangeBlock[MaxAreaAdrRangeBlocks] __attribute((packed));

	struct IoHriMainStoreChipInfo1	chipInfo0	__attribute((packed));
	struct IoHriMainStoreChipInfo1	chipInfo1	__attribute((packed));
	struct IoHriMainStoreChipInfo1	chipInfo2	__attribute((packed));
	struct IoHriMainStoreChipInfo1	chipInfo3	__attribute((packed));
	struct IoHriMainStoreChipInfo1	chipInfo4	__attribute((packed));
	struct IoHriMainStoreChipInfo1	chipInfo5	__attribute((packed));
	struct IoHriMainStoreChipInfo1	chipInfo6	__attribute((packed));
	struct IoHriMainStoreChipInfo1	chipInfo7	__attribute((packed));

	void *   msRamAreaArray			__attribute((packed));
	__u32	msRamAreaArrayNumEntries	__attribute((packed));
	__u32	msRamAreaArrayEntrySize		__attribute((packed));

	__u32	numaDimmExists			__attribute((packed));
	__u32	numaDimmFunctional		__attribute((packed));
	void *	numaDimmArray			__attribute((packed));
	__u32	numaDimmArrayNumEntries		__attribute((packed));
	__u32	numaDimmArrayEntrySize		__attribute((packed));

	struct IoHriMainStoreVpdIdData  idData	__attribute((packed));

	__u64	powerData			__attribute((packed));
	__u64	cardAssemblyPartNum		__attribute((packed));
	__u64	chipSerialNum			__attribute((packed));

	__u64	reserved3			__attribute((packed));
	char	reserved4[16]			__attribute((packed));

	struct IoHriMainStoreVpdFruData fruData	__attribute((packed));

	__u8	vpdPortNum			__attribute((packed));
	__u8	reserved5			__attribute((packed));
	__u8	frameId				__attribute((packed));
	__u8	rackUnit			__attribute((packed));
	char	asciiKeywordVpd[256]		__attribute((packed));
	__u32	reserved6			__attribute((packed));
};


struct IoHriMainStoreSegment5 {    
	__u16	reserved1;
	__u8	reserved2;
	__u8	msVpdFormat;

	__u32	totalMainStore;
	__u64	maxConfiguredMsAdr;

	struct IoHriMainStoreArea4*	msAreaArray;
	__u32	msAreaArrayNumEntries;
	__u32	msAreaArrayEntrySize;

	__u32	msAreaExists;    
	__u32	msAreaFunctional;

	__u64	reserved3;
};



#endif // _IOHRIMAINSTORE_H

