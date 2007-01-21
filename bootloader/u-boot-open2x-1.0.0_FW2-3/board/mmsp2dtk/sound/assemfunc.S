/*-------------------------------------------------------------------------
;	Copyright (C) 2003 MagicEyes Digital Co., All Rights Reserved
;	MagicEyes Digital Co. Proprietary & Confidential
;
;	MMSP 2.0 BASE
;	
;	- assembly functions
;
;	Charlie Myung
;
;	history
;
;------------------------------------------------------------------------- */

/*	AREA	ASSEMFUNC, CODE, READONLY 

	EXPORT	EnableIRQ
	EXPORT	DisableIRQ
*/

/*-------------------------------------------------------------------------
; Program Status Register
;-------------------------------------------------------------------------*/

#define NO_INIT	0xC0

/*-------------------------------------------------------------------------
; enable irq
;-------------------------------------------------------------------------*/
.globl EnableIRQ
EnableIRQ:
	stmfd	sp!, {r0, lr}
	
	mrs		r0, cpsr
	bic		r0, r0, #0x80
	msr		cpsr_cxsf, r0
	
	ldmfd	sp!, {r0, pc}

/*-------------------------------------------------------------------------
; disable irq
;-------------------------------------------------------------------------*/
.globl DisableIRQ
DisableIRQ:
	stmfd	sp!, {r0, lr}
	
	mrs		r0, cpsr
	orr		r0, r0, #0xC0							/* no interrupt */
	msr		cpsr_cxsf, r0
	
	ldmfd	sp!, {r0, pc}

	