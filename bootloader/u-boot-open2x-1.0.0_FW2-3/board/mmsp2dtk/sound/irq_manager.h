//
//	Copyright (C) 2003 MagicEyes Digital Co., All Rights Reserved
//	MagicEyes Digital Co. Proprietary & Confidential
//
//	MMSP 2.0 BASE
//	
//	- IRQ Manager
//
//	Charlie Myung
//
//	history
//
//

#ifndef _IRQ_MANAGER_H
#define	_IRQ_MANAGER_H


//------------------------------------------------------------------------
// Interrupt Controller
//------------------------------------------------------------------------

typedef struct irqcont {
	volatile U32 SRCPEND;
	volatile U32 INTMODE;
	volatile U32 INTMASK;
	volatile U32 PRIORIT;
	volatile U32 INTPEND;
	volatile U32 INTOFST;
} IRQ_CONTROLLER_REG;


#define	SRCPENDREG			(*((volatile U32*)(INTCON_BASE+0x00)))	// source pending reg. (32-bit)
#define	SRCPNDLREG		    (*((volatile U16*)(INTCON_BASE+0x00)))	// 16-bit
#define	SRCPNDHREG			(*((volatile U16*)(INTCON_BASE+0x02)))	//
#define	INTMODEREG			(*((volatile U32*)(INTCON_BASE+0x04)))	// mode reg.
#define	INTMODLREG			(*((volatile U16*)(INTCON_BASE+0x04)))	// 16-bit
#define	INTMODHREG			(*((volatile U16*)(INTCON_BASE+0x06)))	//
#define	INTMASKREG			(*((volatile U32*)(INTCON_BASE+0x08)))	// mask reg.
#define	INTMSKLREG			(*((volatile U16*)(INTCON_BASE+0x08)))	// 16-bit
#define	INTMSKHREG			(*((volatile U16*)(INTCON_BASE+0x0A)))	//
#define	PRIORITREG			(*((volatile U32*)(INTCON_BASE+0x0C)))	// priority reg.
#define	PRIORTLREG			(*((volatile U16*)(INTCON_BASE+0x0C)))	// 16-bit
#define	PRIORTHREG			(*((volatile U16*)(INTCON_BASE+0x0E)))	//
#define	INTPENDREG			(*((volatile U32*)(INTCON_BASE+0x10)))	// interrupt pending reg. (32-bit)
#define	INTPNDLREG			(*((volatile U16*)(INTCON_BASE+0x10)))	// 16-bit
#define	INTPNDHREG			(*((volatile U16*)(INTCON_BASE+0x12)))	//
#define	INTOFSTREG			(*((volatile U16*)(INTCON_BASE+0x14)))	// 


//------------------------------------------------------------------------
// MMSP2 IRQ NUMBER
//------------------------------------------------------------------------
#define	IRQ_DISPINT				0
#define	IRQ_ISPV				1
#define	IRQ_ISPH				2
#define	IRQ_TIMERINT			5
#define	IRQ_MEMORYSTICK			6
#define	IRQ_SSP					7
#define	IRQ_PPM					8
#define	IRQ_DMAINT				9
#define	IRQ_UARTINT				10
#define	IRQ_2DG					11
#define	IRQ_SCALER				12
#define	IRQ_USBHINT				13
#define	IRQ_SDIINT				14
#define	IRQ_USBDINT				15        		
#define	IRQ_RTCINT				16
#define	IRQ_ADCINT				17
#define	IRQ_I2CINT				18
#define	IRQ_AC97INT				19
#define	IRQ_IRDAINT				20
#define	IRQ_GPIO				23
#define	IRQ_CDROM				24
#define	IRQ_OWM					25
#define	IRQ_DUALCPU				26
#define	IRQ_MCUC				27
#define	IRQ_VLD					28
#define	IRQ_MPEG				29
#define	IRQ_MPEGIF				30
#define	IRQ_I2S					31



//------------------------------------------------------------------------
// Type Definitions
//------------------------------------------------------------------------
typedef void (IRQSubhandler)(void);

//------------------------------------------------------------------------
// Prototypes
//------------------------------------------------------------------------
void IRQTopHandler(void);
void InitIRQ(void);
CBOOL RegisterIRQHandler(U32 IRQNum, IRQSubhandler* Handler);
CBOOL UnRegisterIRQHandler(U32 IRQNum);


void MaskIRQ(U32 IRQNum);
void UnmaskIRQ(U32 IRQNum);

//extern void EnableIRQ(void);
//extern void DisableIRQ(void);


#endif	// _IRQ_MANAGER_H
