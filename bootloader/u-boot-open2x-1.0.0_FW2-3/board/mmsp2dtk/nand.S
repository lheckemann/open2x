/*
 * NAND Flash Handling on MMSP20
 * Copy boot loader when NAND boot procedure
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
 *
 */

#include <config.h>
#include <version.h>


/* some parameters for the board */
#define NFDATA		0x9c000000
#define NFCMD		0x9C000010
#define NFADDR		0x9C000018

#define MEMCFGW			0xC0003A00
#define MEMNANDCTRLW	0xC0003A3A
#define MEMNANDTIMEW	0xC0003A3C

#define BL_BASE		0x03F00000 	/* base address of the bootloader */
#define NAND_BASE	0x00000000	/* start address of image in NAND flash */
#define NAND_TOP	0x00080000	/* end address of image in NAND flash */

/**************************************/
.globl copy_boot_loader
copy_boot_loader:
	/* Initializng NAND flash controller */
	ldr	r1, =MEMNANDTIMEW	/* Set timing */
	and	r0, r0, #0x0
	add	r0, r0, #0x700
	add	r0, r0, #0xF0
	add	r0, r0, #0x8
	strh	r0, [r1]

	ldr	r1, =MEMCFGW

	/* Detecting type - 3 or 4 address type */

	ldr	r5, =BL_BASE	@ R5 contains current RAM address
	ldr	r6, =NAND_BASE	@ R6 contains current flash address
	ldr	r9, =NAND_TOP	@ R9 contains top of the flash address

/* Read loop */
read_command:
	ldr 	r1, =MEMNANDCTRLW
	ldrh	r0, [r1]
	orr	r0, r0, #0x80	@ Set interrupt enable (NAND_INT_DETRnB: bit 7)
	orr	r0, r0, #0x8000	@ Clear pending interrupt (NAND_CLR_DETRnB: bit 15)
	strh	R0, [R1]

/*	Set bank to 0 */
/*	Set read command (NAND_CMD_READ) */
	ldr	r1, =NFCMD
	ldr	r0, =0x0
	strb	r0, [r1]

	@ Set address 1
	ldr	r1, =NFADDR
	mov	r0, r6
	strb	r0, [r1]
	@ Set address 2
	mov	r0, r6, lsr #9
	strb	r0, [r1]
	@ Set address 3
	mov	r0, r6, lsr #17
	strb	r0, [r1]
	@ Set address 4 (if needed, according to the type)
	mov	r0, r6, lsr #25
	strb	r0, [r1]

/*	Wait ready */
wait_ready:
	ldr	r1, =MEMNANDCTRLW
	ldrh	r0, [r1]
	and	r0, r0, #0x8000
	cmp	r0, #0x8000
	bne	wait_ready
	and	r2, r2, #0x0
	sub	r2, r2, #0x80
	sub	r2, r2, #0x1	@ Get complement of 0x80
	and	r0, r0, r2
	strh	r0, [r1]

/*	Get data */
	and	r8, r8, #0x0	@ R8 works as word counter
get_page:
	ldr	r1, =NFDATA
	ldr	r0, [r1]
	str	r0, [r5]
	add	r8, r8, #0x4
	add	r5, r5, #0x4
	cmp	r8, #0x200
	bne	get_page

	add	r6, r6, #0x200
	cmp	r6, r9
	ble	read_command

/*	Repeat above cycle according to the number of pages */

	/* everything is fine now */
	mov	pc, lr
