//
//	Copyright (C) 2003 MagicEyes Digital Co., All Rights Reserved
//	MagicEyes Digital Co. Proprietary & Confidential
//
//	MMSP 2.0 BASE
//	
//	- irq
//
//	Charlie Myung
//
//	history
//	2003/11/05	Charlie	IRQTopHandler에서 진입시 DisableIRQ를 호출하고
//					종료시 EnableIRQ 호출하는 부분 삭제(재진입문제-Tony)
//	2003/12/16	Charlie	UnRegisterIRQHandler 추가
//

#include "common.h"
#include "../sound/typedef.h"
#include "../sound/mes_const.h"
#include "../sound/mes_iomap.h"
#include "../sound/irq_manager.h"


static IRQSubhandler* IRQSubhandlerList[NUM_OF_IRQ] = {0,};
static IRQ_CONTROLLER_REG* IRQContReg;



void
IRQTopHandler(void)
{
	int irq_num = 0;
	
	// find irq number
	while( (IRQContReg->INTPEND & (U32)(1 << irq_num)) == 0 ) 
	{
		irq_num++;
	};

	if (irq_num >= NUM_OF_IRQ) 
	{
		EnableIRQ();
		return;
	}

	IRQSubhandlerList[irq_num]();

	IRQContReg->SRCPEND	|= (U32)(1 << irq_num);			// Clear SRCPENDREG
	IRQContReg->INTPEND	|= (U32)(1 << irq_num);			// Clear INTPENDREG
}


void
InitIRQ(void)
{
#ifdef CONFIG_USE_IRQ
	IRQContReg = (IRQ_CONTROLLER_REG*)(INTCON_BASE);
	IRQContReg->INTMODE	= 0x0;			// INTMODE Reg
	IRQContReg->INTMASK	= 0xffffffff;	// MASKREG
	IRQContReg->SRCPEND	= 0xffffffff;	// SET SRCPENDREG        
#else
#ifndef __TARGET_CPU_ARM940T
	IRQContReg = (IRQ_CONTROLLER_REG*)(INTCON_BASE);
#else
	IRQContReg = (IRQ_CONTROLLER_REG*)(A940T_INTCON_BASE);
#endif

	IRQContReg->INTMODE	= 0x0;			// INTMODE Reg
	IRQContReg->INTMASK	= 0xffffffff;	// MASKREG
	IRQContReg->SRCPEND	= 0xffffffff;	// SET SRCPENDREG        

	// insert IRQ vector
	/*  즉 현재위치에서 0x20 바이트 오프셋 뒤의 값을 어드레스로 브랜치 하라는 의미 */
	*(volatile int *)(0x18)	= 0xe59ff018; 
	*(volatile int *)(0x38)	= (int)IRQTopHandler;

	printf("vector:0x%x\n",*(volatile int *)(0x38));
	
	EnableIRQ();
#endif	
}


CBOOL
RegisterIRQHandler
(
	U32 IRQNum,
	IRQSubhandler* Handler
)
{
	if (IRQNum > NUM_OF_IRQ)
		return CFALSE;

	if (Handler == NULL)
		return CFALSE;

	if (IRQSubhandlerList[IRQNum]!=0)
		return CFALSE;

	// register handler
	IRQSubhandlerList[IRQNum] = Handler;

	return CTRUE;
}


CBOOL
UnRegisterIRQHandler
(
	U32 IRQNum
)
{
	if (IRQNum > NUM_OF_IRQ)
		return CFALSE;
 
	if (IRQSubhandlerList[IRQNum]!=0)
		IRQSubhandlerList[IRQNum] = (IRQSubhandler*)0;
 
	return CTRUE;
}


void
MaskIRQ
(
	U32 IRQNum
)
{
	IRQContReg->INTMASK = IRQContReg->INTMASK | (U32)(1 << IRQNum);
}


void
UnmaskIRQ
(
	U32 IRQNum
)
{
	IRQContReg->INTMASK = IRQContReg->INTMASK & (~(U32)(1 << IRQNum));
}
