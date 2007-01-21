//
//	Copyright (C) 2003 MagicEyes Digital Co., All Rights Reserved
//	MagicEyes Digital Co. Proprietary & Confidential
//
//	MMSP 2.0 BASE
//	
//	- Macro definition
//
//	Charlie Myung, 
//
//	history
//		2003.10.02 : Add Bit Operation Macros by Goofy
//
//	

#ifndef	_MMSP2MACRO_H
#define	_MMSP2MACRO_H


// get low half word or high half word
#define LHWORD(v)		(U16)( (U32)v & 0x0000FFFF )
#define HHWORD(v)		(U16)( ((U32)v >> 16) & 0x0000FFFF )


/*************************************************************************
							Bit Operation
*************************************************************************/
#define _BIT( num )		(1U<<(num))

#define BIT0    ((U32)1<<0)
#define BIT1    ((U32)1<<1)
#define BIT2    ((U32)1<<2)
#define BIT3    ((U32)1<<3)
#define BIT4    ((U32)1<<4)
#define BIT5    ((U32)1<<5)
#define BIT6    ((U32)1<<6)
#define BIT7    ((U32)1<<7)
#define BIT8    ((U32)1<<8)
#define BIT9    ((U32)1<<9)
#define BIT10   ((U32)1<<10)
#define BIT11   ((U32)1<<11)
#define BIT12   ((U32)1<<12)
#define BIT13   ((U32)1<<13)
#define BIT14   ((U32)1<<14)
#define BIT15   ((U32)1<<15)
#define BIT16   ((U32)1<<16)
#define BIT17   ((U32)1<<17)
#define BIT18   ((U32)1<<18)
#define BIT19   ((U32)1<<19)
#define BIT20   ((U32)1<<20)
#define BIT21   ((U32)1<<21)
#define BIT22   ((U32)1<<22)
#define BIT23   ((U32)1<<23)
#define BIT24   ((U32)1<<24)
#define BIT25   ((U32)1<<25)
#define BIT26   ((U32)1<<26)
#define BIT27   ((U32)1<<27)
#define BIT28   ((U32)1<<28)
#define BIT29   ((U32)1<<29)
#define BIT30   ((U32)1<<30)
#define BIT31   ((U32)1<<31)

/* create a bitmask with a bitfield width and an amount to shift left */
#define BIT_MASK(__bf) (((1U << (bw ## __bf)) - 1) << (bs ## __bf))

/* create a bitmask with a bitfield width and an amount to shift left */
#define BIT_MAX(__bf)  ((1U << (bw ## __bf)) - 1)

/* set value only, use | operator to set many field simultaneous */
#define SET_VALUE(__bf, __val) ((((__val) << (bs ## __bf)) & (BIT_MASK(__bf))))

/* get bits in the width/shift specified, but don't shift back down */
#define GET_BOOL(__src, __bf) (((__src) & (BIT_MASK(__bf))) != 0)
#define GET_BOOL2OR(__src, __bf1, __bf2)          (((__src) & (BIT_MASK(__bf1) | BIT_MASK(__bf2))) != 0)
#define GET_BOOL3OR(__src, __bf1, __bf2, __bf3)   (((__src) & (BIT_MASK(__bf1) | BIT_MASK(__bf2) | BIT_MASK(__bf3))) != 0)
#define GET_BOOL2AND(__src, __bf1, __bf2)         (((__src) & (BIT_MASK(__bf1) | BIT_MASK(__bf2))) == (BIT_MASK(__bf1) | BIT_MASK(__bf2)))
#define GET_BOOL3AND(__src, __bf1, __bf2, __bf3)  (((__src) & (BIT_MASK(__bf1) | BIT_MASK(__bf2) | BIT_MASK(__bf3))) == (BIT_MASK(__bf1) | BIT_MASK(__bf2) | BIT_MASK(__bf3)))

/* set bits in the width/shift specified; the value is cleaned of other bits by the macro */
#define SET_BITS(__dst, __bf, __val) ((__dst) = ((__dst) & ~(BIT_MASK(__bf))) | (((__val) << (bs ## __bf)) & (BIT_MASK(__bf))))

/* set bits in the width/shift specified; the value is cleaned of other bits by the macro */
#define SET_2BITS(__dst, __bf1, __val1, __bf2, __val2) ((__dst) = ((__dst) & ~((BIT_MASK(__bf1) | BIT_MASK(__bf2)))) | SET_VALUE(__bf1, __val1) | SET_VALUE(__bf2, __val2))

/* set bits without reading first - for sensitive interrupt type registers */
#define SET_BITS_ONLY(__dst, __bf, __val) ((__dst) = (((__val) << (bs ## __bf)) & (BIT_MASK(__bf))))

/* and bits in the width/shift specified; the value is cleaned of other bits by the macro */
#define AND_BITS(__dst, __bf, __val) ((__dst) &= (((__val) << (bs ## __bf)) & (BIT_MASK(__bf))))

/* get bits in the width/shift specified, and shift back down to the lowest bit */
#define GET_BITS(__src, __bf) (((__src) & (BIT_MASK(__bf))) >> (bs ## __bf))

/* get bits in the width/shift specified, and shift back down to the lowest bit */
#define GET_BITS_DIRECT(__src, __bs, __bw)        (((__src) & (((1U << __bw) - 1) << __bs)) >> (__bs))


#endif