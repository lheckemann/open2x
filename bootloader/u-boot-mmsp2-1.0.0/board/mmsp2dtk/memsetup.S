/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
 *
 */


#include <config.h>
#include <version.h>


/* some parameters for the board */

/*
 * MMSP20
 */

#define MEMTIMEX0		0xC0003802  

/* MEMTIMEX0 */
#define tRCD			(0x4)
#define tRP			(0x4<<4)
#define tRFC			(0xF<<8)
#define	tMRD			(0x2<<12)

/* MEMTIMEX1 */
#define tWR			(0x2)
#define tRAS			(0x7<<4)
#define tRC			(0xA<<8)
#define	CAS2			(0x0<<12)
#define CAS3			(0x1<<12)
#define MODECOMP		(0x0<<15)
#define MODEBUSY		(0x0<<15)

/* MEMACTPWDX */
#define	ACTPERD			(0xFF)
#define ACTPWDEN		(0x0<<8)

/* MEMREFX */
#define REFPERD			(0x0100)

/**************************************/

_TEXT_BASE:
	.word	TEXT_BASE

.globl memsetup
memsetup:
	/* memory control configuration */
	/* make r0 relative the current location so that it */
	/* reads SMRDATA out of FLASH rather than memory ! */
	ldr     r0, =SMRDATA
	ldr	r1, _TEXT_BASE
	sub	r0, r0, r1
	sub 	r0, r0, #0x20
	ldr	r1, =MEMTIMEX0	/* Bus Width Status Controller */
	add     r2, r0, #4*2
0:
	ldr     r3, [r0], #2
	str     r3, [r1], #2
	cmp     r2, r0
	bne     0b

	/* everything is fine now */
	mov	pc, lr

	.ltorg
/* the literal pools origin */

SMRDATA:
    .hword  (MODECOMP+CAS3+tRC+tRAS+tWR)
    .hword  (tMRD+tRFC+tRP+tRCD)
    .hword  (REFPERD)
    .hword  (ACTPWDEN+ACTPERD)
