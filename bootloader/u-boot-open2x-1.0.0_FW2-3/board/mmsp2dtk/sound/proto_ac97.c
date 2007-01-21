/*-----------------------------------------------------------------------------
	prototype for ac97
	file name : proto_ac97.c
	by Nam Tae Woo 
-------------------------------------------------------------------------------
	2003/11/11		avoid undesired interrupt (codec ready, wr done, rd done)
	2003/11/03		receive address offset for codec register access
	2003/10/30		apply new type, add virtual address capability
	2003/10/16		modify for common environmemt
	2003/09/26		1st version
-----------------------------------------------------------------------------*/
#include <common.h>
#include "../sound/proto_ac97.h"
#include "../sound/mes_iomap.h"

#ifndef _WINDEF_
#include "../sound/irq_manager.h"
extern void MaskIRQ (U32 IRQNum);
extern void UnmaskIRQ (U32 IRQNUM);
#else
#include <irq_number.h>
#include "regmmsp.h"

typedef struct irqcont {
	volatile U32 SRCPEND;
	volatile U32 INTMODE;
	volatile U32 INTMASK;
	volatile U32 PRIORIT;
	volatile U32 INTPEND;
	volatile U32 INTOFST;
} IRQ_CONTROLLER_REG;

#endif



AC97_REG* ac97_reg;

/*---------------------------------------------------------------------------
	get ac97 device info
---------------------------------------------------------------------------*/
void AC97_GetDeviceInfo (MES_DEVINFO *pDevInfo)
{
	static char DevName[] = "MMSP2 AC97 Controller v0.5";

	pDevInfo->pDevName		= (const U8*) DevName;
	pDevInfo->IOBaseAddress	= 0xc0000e00;
	pDevInfo->IORegionSize	= 0x200;
	pDevInfo->Cap			= MES_DEV_INT | MES_DEV_NDMA;
}

/*---------------------------------------------------------------------------
	initialize ac97 
---------------------------------------------------------------------------*/
void AC97_Initialize (U32 VirAddr)
{
	U8 i;

// 2003.10.30, virtual address capability
	if (VirAddr==0) {			// use physical address
		ac97_reg = (AC97_REG *) AC97_BASE;
	}
	else {						// use virtual address
		ac97_reg = (AC97_REG *) VirAddr;
	}
	ac97_reg->control = AC97_LINKOFF;
	ac97_reg->control = AC97_COLDRESET;
	for (i=0; i<10; i++)		// hold reset status
	{
		// NULL
	}
	ac97_reg->control = AC97_LINKOFF;	// reset high & link off -> stand by
}

/*---------------------------------------------------------------------------
	set ac97 irq enable
---------------------------------------------------------------------------*/
void AC97_SetInterruptEnb (U16 IntEnb)	
{
	ac97_reg->irqena = IntEnb;
}

/*---------------------------------------------------------------------------
	set ac97 output channel, data width
---------------------------------------------------------------------------*/
void AC97_SetConfig (U16 ExtChannel, U16 OutDatWidth, U16 InDatWidth)
{
	ac97_reg->config = ExtChannel | OutDatWidth | InDatWidth;
}

/*---------------------------------------------------------------------------
	enable aclink & controller
---------------------------------------------------------------------------*/
CBOOL AC97_Enable (U16 TimeOut)
{
	U16 i;
	static IRQ_CONTROLLER_REG* IRQContReg;

	IRQContReg = (IRQ_CONTROLLER_REG*)(INTCON_BASE);
	MaskIRQ (IRQ_AC97INT);				// mask ac97 irq
	
	
#if 1
	ac97_reg->control = AC97_LINKON;	// aclink on
	for (i=0; i<TimeOut; i++);
#else
	ac97_reg->control = AC97_LINKOFF; 	/* ac-link off */
	ac97_reg->control =0;				/* cold reset */
	for (i=0; i<TimeOut; i++);	
	ac97_reg->control = AC97_LINKOFF; 	/* ac-link off */
	ac97_reg->control =	AC97_LINKON;
	for (i=0; i<TimeOut; i++);	
#endif

	ac97_reg->irqena |= (U16) AC97_INT_CODECREADY;

#if 0
	if ((ac97_reg->irqpend & AC97_INT_CODECREADY)!=0) 	// codec ready 
	{
		ac97_reg->irqena &= ~((U16)AC97_INT_CODECREADY);

		IRQContReg->SRCPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear SRCPENDREG
		IRQContReg->INTPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear INTPENDREG
		UnmaskIRQ (IRQ_AC97INT);
		return (CTRUE);
	}
	else 		// codec not ready & aclink is invalid
	{
		ac97_reg->irqena &= ~((U16)AC97_INT_CODECREADY);

		IRQContReg->SRCPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear SRCPENDREG
		IRQContReg->INTPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear INTPENDREG
		UnmaskIRQ (IRQ_AC97INT);
		return (CFALSE);
	}

#else
	
	for (i=0; i<TimeOut; i++)
	{
		if ((ac97_reg->irqpend & AC97_INT_CODECREADY)!=0) 	// codec ready 
		{
			ac97_reg->irqena &= ~((U16)AC97_INT_CODECREADY);
			IRQContReg->SRCPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear SRCPENDREG
			IRQContReg->INTPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear INTPENDREG
			UnmaskIRQ (IRQ_AC97INT);
			return (CTRUE);
		}	
	}	

	ac97_reg->irqena &= ~((U16)AC97_INT_CODECREADY);
	IRQContReg->SRCPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear SRCPENDREG
	IRQContReg->INTPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear INTPENDREG
	UnmaskIRQ (IRQ_AC97INT);
	
	return (CFALSE);
#endif

}

/*---------------------------------------------------------------------------
	disable aclink, not go to powerdown state
	enable aclink by AC97_Resume
---------------------------------------------------------------------------*/
void AC97_Pause (void)
{
	ac97_reg->control = AC97_LINKOFF;	// aclink off
}

/*---------------------------------------------------------------------------
	resume aclink from pause state
---------------------------------------------------------------------------*/
CBOOL AC97_Resume (U16 TimeOut)
{
	U16 i;
	static IRQ_CONTROLLER_REG* IRQContReg;
#ifndef __TARGET_CPU_ARM940T
	IRQContReg = (IRQ_CONTROLLER_REG*)(INTCON_BASE);
#else
	IRQContReg = (IRQ_CONTROLLER_REG*)(A940T_INTCON_BASE);
#endif

#ifndef _WINDEF_
	MaskIRQ (IRQ_AC97INT);				// mask ac97 irq
#else
	MMSP20_IOREG32(INTMSKREG) |= (U32) (_BIT(IRQ_AC97INT)); 
#endif
	ac97_reg->control = AC97_LINKON;	// aclink on
	for (i=0; i<TimeOut; i++)
	{
		// NULL
	}
	ac97_reg->irqena |= (U16) AC97_INT_CODECREADY;

	if ((ac97_reg->irqpend & AC97_INT_CODECREADY)!=0) 	// codec ready 
	{
		ac97_reg->irqena &= ~((U16)AC97_INT_CODECREADY);
#ifndef _WINDEF_
		IRQContReg->SRCPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear SRCPENDREG
		IRQContReg->INTPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear INTPENDREG
		UnmaskIRQ (IRQ_AC97INT);
#else
		MMSP20_IOREG32(INTMSKREG) &= ~((U32) (_BIT(IRQ_AC97INT))); 
#endif
		return (CTRUE);
	}
	else 		// codec not ready & aclink is invalid
	{
		ac97_reg->irqena &= ~((U16)AC97_INT_CODECREADY);
#ifndef _WINDEF_
		IRQContReg->SRCPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear SRCPENDREG
		IRQContReg->INTPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear INTPENDREG
		UnmaskIRQ (IRQ_AC97INT);
#else
		MMSP20_IOREG32(INTMSKREG) &= ~((U32) (_BIT(IRQ_AC97INT))); 
#endif
		return (CFALSE);
	}
}

/*---------------------------------------------------------------------------
	powerdown aclink (codec status to powerdown)
---------------------------------------------------------------------------*/
void AC97_PowerDown (void)
{
	AC_POWERDOWN = AC97_PDOWNVAL;	// power down reg write
}

/*---------------------------------------------------------------------------
	wake up aclink when powerdown state
---------------------------------------------------------------------------*/
CBOOL AC97_WakeUp (U16 TimeOut)
{
	U16 i;
	static IRQ_CONTROLLER_REG* IRQContReg;
#ifndef __TARGET_CPU_ARM940T
	IRQContReg = (IRQ_CONTROLLER_REG*)(INTCON_BASE);
#else
	IRQContReg = (IRQ_CONTROLLER_REG*)(A940T_INTCON_BASE);
#endif

	if (ac97_reg->state==3)			// powerdown state (desired state)
	{
		ac97_reg->control = AC97_WARMRESET;		// warm reset
	}
	else 		// not in powerdown state
	{
		if (ac97_reg->state==2) 	// active state
		{
			return (CTRUE);
		}
		else 	// other state
		{
			ac97_reg->control = AC97_LINKON;
		}
	}
#ifndef _WINDEF_
	MaskIRQ (IRQ_AC97INT);				// mask ac97 irq
#else
	MMSP20_IOREG32(INTMSKREG) |= (U32) (_BIT(IRQ_AC97INT)); 
#endif

	for (i=0; i<TimeOut; i++)
	{
		// NULL
	}

	ac97_reg->irqena |= (U16) AC97_INT_CODECREADY;

	if ((ac97_reg->irqpend & AC97_INT_CODECREADY)!=0) 	// codec ready
	{
		ac97_reg->irqena &= ~((U16) AC97_INT_CODECREADY);
#ifndef _WINDEF_
		IRQContReg->SRCPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear SRCPENDREG
		IRQContReg->INTPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear INTPENDREG
		UnmaskIRQ (IRQ_AC97INT);
#else
		MMSP20_IOREG32(INTMSKREG) &= ~((U32) (_BIT(IRQ_AC97INT))); 
#endif
		return (CTRUE);
	}
	else 		// codec not ready & aclink is invalid
	{
		ac97_reg->irqena &= ~((U16) AC97_INT_CODECREADY);
#ifndef _WINDEF_
		IRQContReg->SRCPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear SRCPENDREG
		IRQContReg->INTPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear INTPENDREG
		UnmaskIRQ (IRQ_AC97INT);
#else
		MMSP20_IOREG32(INTMSKREG) &= ~((U32) (_BIT(IRQ_AC97INT))); 
#endif
		return (CFALSE);
	}
}

/*---------------------------------------------------------------------------
	ac97 controller to idle state
---------------------------------------------------------------------------*/
void AC97_Close (void) 
{
	ac97_reg->control = AC97_LINKOFF;
	ac97_reg->control = AC97_COLDRESET;
}

/*---------------------------------------------------------------------------
	read ac97 status register
---------------------------------------------------------------------------*/
U16   AC97_GetInterruptPend (void) 
{
	U16 irqstatus;

	irqstatus = ac97_reg->irqpend;
	return (irqstatus);
}

/*---------------------------------------------------------------------------
	read ac97 irq enable register
---------------------------------------------------------------------------*/
U16   AC97_GetInterruptEnb (void) 
{
	U16 irqenable;

	irqenable = ac97_reg->irqena;
	return (irqenable);
}

/*---------------------------------------------------------------------------
	clear ac97 status register 
---------------------------------------------------------------------------*/
void AC97_ClrInterruptPend (U16 ClrPend) 
{
	ac97_reg->irqpend = ClrPend;
}

/*---------------------------------------------------------------------------
	write codec register by polling
---------------------------------------------------------------------------*/
CBOOL AC97_WriteCodecReg (U32 RegOffset, U16 WrData, U16 TimeOut) 
{
	U16 i;
	U32 RegAddr;		// 2003/11/03, offset to full address
	static IRQ_CONTROLLER_REG* IRQContReg;
#ifndef __TARGET_CPU_ARM940T
	IRQContReg = (IRQ_CONTROLLER_REG*)(INTCON_BASE);
#else
	IRQContReg = (IRQ_CONTROLLER_REG*)(A940T_INTCON_BASE);
#endif

	if ((ac97_reg->control)==AC97_LINKOFF) 	// if link is off, enable link
	{
		ac97_reg->control = AC97_LINKON;
	}
	else if (ac97_reg->state==3)		// powerdown state
	{
		return (CFALSE);
	}
#ifndef _WINDEF_
	MaskIRQ (IRQ_AC97INT);				// mask ac97 irq
#else
	MMSP20_IOREG32(INTMSKREG) |= (U32) (_BIT(IRQ_AC97INT)); 
#endif

	ac97_reg->irqena |= (U16) AC97_INT_WRDONE;

	RegAddr = (U32) (ac97_reg) + 0x0100 + RegOffset;
	ac97_reg->irqpend = (U16) AC97_CLR_WRDONE;
	*(volatile U16*) (RegAddr) = WrData;

	for (i=0; i<TimeOut; i++)
	{
		// NULL
	}

	if ((AC97_GetInterruptPend() & AC97_PEND_WRDONE)!=0)	// write done
	{
		ac97_reg->irqena &= ~((U16) AC97_INT_WRDONE);
#ifndef _WINDEF_
		IRQContReg->SRCPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear SRCPENDREG
		IRQContReg->INTPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear INTPENDREG
		UnmaskIRQ (IRQ_AC97INT);
#else
		MMSP20_IOREG32(INTMSKREG) &= ~((U32) (_BIT(IRQ_AC97INT))); 
#endif
		return (CTRUE);
	}
	else 
	{
		ac97_reg->irqena &= ~((U16) AC97_INT_WRDONE);
#ifndef _WINDEF_
		IRQContReg->SRCPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear SRCPENDREG
		IRQContReg->INTPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear INTPENDREG
		UnmaskIRQ (IRQ_AC97INT);
#else
		MMSP20_IOREG32(INTMSKREG) &= ~((U32) (_BIT(IRQ_AC97INT))); 
#endif
		return (CFALSE);
	}
}

/*---------------------------------------------------------------------------
	read codec regiser by polling
---------------------------------------------------------------------------*/
U32  AC97_ReadCodecReg (U32 RegOffset, U16 TimeOut) 
{
	U32 returnval;
	U16 rdval;
	U16 i;
	U32 RegAddr;		// 2003/11/03, offset to full address
	static IRQ_CONTROLLER_REG* IRQContReg;
#ifndef __TARGET_CPU_ARM940T
	IRQContReg = (IRQ_CONTROLLER_REG*)(INTCON_BASE);
#else
	IRQContReg = (IRQ_CONTROLLER_REG*)(A940T_INTCON_BASE);
#endif

	if ((ac97_reg->control)==AC97_LINKOFF) 	// if link is off, enable link
	{
		ac97_reg->control = AC97_LINKON;
	}
	else if (ac97_reg->state==3)		// powerdown state
	{
		return (CFALSE);
	}
#ifndef _WINDEF_
	MaskIRQ (IRQ_AC97INT);				// mask ac97 irq
#else
	MMSP20_IOREG32(INTMSKREG) |= (U32) (_BIT(IRQ_AC97INT)); 
#endif

	ac97_reg->irqena |= (U16) AC97_INT_RDDONE;

	RegAddr = (U32) (ac97_reg) + 0x0100 + RegOffset;
	ac97_reg->irqpend = (U16) (AC97_CLR_WRDONE | AC97_CLR_RDDONE);
	rdval = *(volatile U16*) (RegAddr);
	for (i=0; i<TimeOut; i++)
	{
		// NULL
	}
	if ((AC97_GetInterruptPend() & AC97_PEND_RDDONE)!=0) {	// read done
		ac97_reg->irqena &= ~((U16) AC97_INT_RDDONE);
#ifndef _WINDEF_
		IRQContReg->SRCPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear SRCPENDREG
		IRQContReg->INTPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear INTPENDREG
		UnmaskIRQ (IRQ_AC97INT);
#else
		MMSP20_IOREG32(INTMSKREG) &= ~((U32) (_BIT(IRQ_AC97INT))); 
#endif
		rdval = ac97_reg->coddata;
		returnval = (U32) rdval;
		return (returnval);
	}
	else {	// read error
		ac97_reg->irqena &= ~((U16) AC97_INT_RDDONE);
#ifndef _WINDEF_
		IRQContReg->SRCPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear SRCPENDREG
		IRQContReg->INTPEND	|= (U32)(1 << IRQ_AC97INT);		// Clear INTPENDREG
		UnmaskIRQ (IRQ_AC97INT);
#else
		MMSP20_IOREG32(INTMSKREG) &= ~((U32) (_BIT(IRQ_AC97INT))); 
#endif
		return (0xffffffff);
	}
}

/*---------------------------------------------------------------------------
	check aclink command is busy or not
---------------------------------------------------------------------------*/
CBOOL AC97_IsBusy (void) 
{
	if ((AC97_GetInterruptPend() & AC97_PEND_WRDONE)==0) 	// is busy
	{
		return (CTRUE);
	}
	else 	// is not busy
	{
		return (CFALSE);
	}
}

// end of proto_ac97.c
