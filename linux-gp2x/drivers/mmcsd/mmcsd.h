/*
 * drivers/mmcsdsd/mmcsdsd.h
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 */

#ifndef MMCSD_H
#define MMCSD_H

#include <linux/config.h>
#include <linux/types.h>
#include <asm/semaphore.h>

#undef CONFIG_S3C2440
#undef	CONFIG_S3C2410
/*
 * MMC/SD Legends
 */
/*
  PP		PushPull, output driver type with
  		low impedance driver capability for 0 and 1
  OD		OpenDrain, output driver type with
  		low impedance driver capability for 0 and
		high impedance driver capability for 1

  bc		broadcast commands
  bcr		broadcast commands with response
  ac		addressed (point-to-point) commands (with resp.)
  adtc		addressed (point-to-point) data transfer commands (with resp.)

  CSD		CardSpecific data, MultiMediaCard register
  		to store operating parameters
  CID		Card IDentification Data, MultiMediaCard register
		for ther card initialization procedure
  RCA		Ralative Card Address, MultiMediaCard register
		which contains the current card address of
		an initialized MultiMediaCard
  OCR		Operation Condition Register, MultiMediaCard register
		which contains the voltage window
		witch is supported by the MultiMediaCard
  DSR		Driver Stage Register, control register
  		for the programmable driver tge driver (PDS)
  PDS		Porgrammable Driver Stage driver, is a tristate output driver
  		which has is programmable to adapt
		the driver capabilities to the bus design
  SCR		SD CARD Configuration Register, SD only
  		SCR provides information on SD Memory Card's special features

  CMD<n>	send command (48bit)
  		47	0		start bit
		46	1		host
		45:40	bit5   .. bit0	command
		39:8	bit31  .. bit0	argument
		7:1	bit6   .. bit0	CRC7
		0	1		end bit
  R1		reponse command (48bit)
  		47	0		start bit
		46	0		card
		45:40	bit5   .. bit0	command
		39:8	bit31  .. bit0	status
		7:1	bit6   .. bit0	CRC7
		0	1		end bit
  R1b		identical to R1 with the additional busy signaling via the data
  R2		CID, CSD register (136bit)
  		135	0		start bit
		134	0		card
		133:128	bit5   .. bit0	reserved
		127:1	bit127 .. bit1	CID or CSD register
					including internal CRC
		0	1		end bit
  R3		OCR register (48bit)
  		47	0		start bit
		46	0		card
		45:40	bit5   .. bit0	reserved
		39:8	bit31  .. bit0	OCR
		7:1	bit6   .. bit0	reserved
		0	1		end bit
  R6		RCA response (48bit), SD only
  		47	0		start bit
		46	0		card
		45:40	bit5   .. bit0	command index
		39:24	bit31  .. bit16	new published RCA[31:16] of the card
		23:8    bit15  .. bit0	card status bits:23,22,19,12:0
		7:1	bit6   .. bit0	CRC7
		0	1		end bit

  dadr		Data Address
  wadr		Write protect data Address

  7-bit CRC	CRC7 (warning!!! it's a polynomical arithmetic mod 2)
		G(x) = x^7 + x^3 + 1
  		M(x) = (start bit)*x^39 + (host bit)*x^38 + ... +
		       (last bit before CRC)*x^0
		CRC[6..0] = Remainder[M(x)*x^7 / G(x)]
*/

/*
 * MMCSD Commands
 */

/* class 1, basic commands */
#define	MMCSD_CMD0	0x40	/* bc, , , GO_IDLE_STATE */
#define MMCSD_CMD1	0x41	/* bcr, 31:0 OCR, R3, SEND_OP_COND */
#define MMCSD_CMD2	0x42	/* bcr, , R2, ALL_SEND_CID */
#define MMCSD_CMD3	0x43	/* ac, 31:16 RCA, R1, SET_RELATIVE_ADDR */
				/* bcr, , R6, SEBD_RELATIVE_ADDR, SD only */
#define MMCSD_CMD4	0x44	/* bc, 31:16 RCA, , SET_DSR */
#define MMCSD_CMD7	0x47	/* ac, 31:16 RCA, R1, SELECT/DESELECT CARD */
#define MMCSD_CMD9	0x49	/* ac, 31:16 RCA, R2, SEND_CSD */
#define MMCSD_CMD10	0x4a	/* ac, 31:16 RCA, R2, SEND_CID */
#define MMCSD_CMD11	0x4b	/* adtc, 31:0 dadr, R1, READ_DAT_UNTIL_STOP */
				/* reserved, SD only */
#define MMCSD_CMD12	0x4c	/* ac, , R1b, STOP_TRANSMISSION */
#define MMCSD_CMD13	0x4d	/* ac, 31:16 RCA, R1, SEND_STATUS */
#define MMCSD_CMD15	0x4f	/* ac, 31:16 RCA, , GO_INACTIVE_SATE */

/* class 2, block oriented read commands */
#define MMCSD_CMD16	0x50	/* ac, 31:0 blk len, R1, SET_BLOCKLEN */
#define MMCSD_CMD17	0x51	/* adtc, 31:0 dadr, R1, READ_SINGLE_BLOCK */
#define MMCSD_CMD18	0x52	/* adtc, 31:0 dadr, R1, READ_MULTIPLE_BLOCK */

/* class 3 */
#define MMCSD_CMD20	0x54	/* adtc, 31:0 dadr, R1, WRITE_DAT_UNTIL_STOP */
				/* reserved, SD only */

/* class 4, block oriented write commands */
#define MMCSD_CMD24	0x58	/* adtc, 31:0 dadr, R1, WRITE_BLOCK */
#define MMCSD_CMD25	0x59	/* adtc, 31:0 dadr, R1, WRITE_MULTIPLE_BLOCK */
#define MMCSD_CMD26	0x5a	/* adtc, , R1, PROGRAM_CID */
#define MMCSD_CMD27	0x5b	/* adtc, , R1, PROGRAM_CSD */

/* class 6, block oriented write protection commands */
#define MMCSD_CMD28	0x5c	/* ac, 31:0 dadr, R1b, SET_WRITE_PROT */
#define MMCSD_CMD29	0x5d	/* ac, 31:0 dadr, R1b, CLR_WRITE_PROT */
#define MMCSD_CMD30	0x5e	/* adtc, 31:0 wadr, R1, SEND_WRITE_PROT */

/* class 5, erase commands */
#define MMCSD_CMD32	0x60	/* ac, 31:0 dadr, R1, TAG_SECTOR_START */
				/* ac, 31:0 dadr, R1, ERASE_WR_BLK_START, SD only */
#define MMCSD_CMD33	0x61	/* ac, 31:0 dadr, R1, TAG_SECTOR_END */
				/* ac, 31:0 dadr, R1, ERASE_WR_BLK_END, SD only */
#define MMCSD_CMD34	0x62	/* ac, 31:0 dadr, R1, UNTAG_SECTOR */
#define MMCSD_CMD35	0x63	/* ac, 31:0 dadr, R1, TAG_ERASE_GROUP_START */
#define MMCSD_CMD36	0x64	/* ac, 31:0 dadr, R1, TAG_ERASE_GROUP_END */
#define MMCSD_CMD37	0x65	/* ac, 31:0 dadr, R1, UNTAG_ERASE_GROUP */
				/* MMCSD_CMD34~MMCSD_CMD37 reserved, SD only */
#define MMCSD_CMD38	0x66	/* ac, , R1b, ERASE */

/* class 7, lock card */
#define MMCSD_CMD42	0x6a	/* adtc, , R1b, LOCK_UNLOCK */

/* class 8, applicatin specific commands */
#define MMCSD_CMD55	0x77	/* ac, 31:16 RCA, R1, APP_CMD */
#define MMCSD_CMD56	0x78	/* adtc, 0 RD/WR, R1, GEN_CMD  */

/* application specific commands used/reserved for SD Memory Card */
#define MMCSD_ACMD6	0x46	/* ac, 1:0 bus width, R1, SET_BUS_WIDTH */
#define MMCSD_ACMD13	0x4d	/* adtc, , R1, SD_STATUS  */
#define MMCSD_ACMD22	0x56	/* adtc, , R1, SEND_NUM_WR_BLOCKS */
#define MMCSD_ACMD23	0x57	/* ac, 22:0 blks, R1, SET_WR_BLK_ERASE_COUNT */
#define MMCSD_ACMD41	0x69	/* bcr, 31:0 OCR, R3, MMCSD_SEND_OP_COND */
#define MMCSD_ACMD42	0x6a	/* ac, 0 set_cd, R1, SET_CLR_CARD_DETECT */
#define MMCSD_ACMD51	0x73	/* adtc, , R1, SEND_SCR */

/* Command size */
#define MMCSD_CMD_SIZE	6

/* Command timings */
#define	MMCSD_TIME_NCR_MIN	2	/* min. of Number of cycles
					   between command and response */
#define MMCSD_TIME_NCR_MAX	64	/* max. of Number of cycles
					   between command and response */
#define MMCSD_TIME_NID_MIN	5	/* min. of Number of cycles
					   between card identification or
					   card operation conditions command
					   and the corresponding response */
#define MMCSD_TIME_NID_MAX	10	/* max. of Number of cycles
					   between card identification or
					   card operation conditions command
					   and the corresponding response */
#define MMCSD_TIME_NAC_MIN	2	/* min. of Number of cycles
					   between command and 
					   the start of a related data block */
#define MMCSD_TIME_NRC_MIN	8	/* min. of Number of cycles
					   between the last reponse and
					   a new command */
#define MMCSD_TIME_NCC_MIN	8	/* min. of Number of cycles
					   between two commands, if no reponse
					   will be send after the first command
					   (e.g. broadcast) */
#define MMCSD_TIME_NWR_MIN	2	/* min. of Number of cycles
					   between a write command and
					   the start of a related data block */

/* 
 * CID(Card IDentification) Register 
 */
typedef struct {
	__u8  mid;	/* [127:120] Manufacturer ID */		
	__u16 oid;	/* [119:104] OEM/Application ID */
	__u8  pnm[7];  	/* [103: 56] Product Name + '\0' (MMCSD) */
	__u8  pnm_sd[6];/* [103: 64] Product Name + '\0' (SD) */
	__u8  prv;	/* [ 55: 48] Product Version (MMCSD) */
			/* [ 63: 56] Product Version (SD) */
	__u32 psn;	/* [ 47: 16] Product Serial Number (MMCSD) */
			/* [ 55: 24] Product Serial Number (SD) */
	__u8  mdt;	/* [ 15:  8] Manufacturing date (MMCSD) */
	__u16 mdt_sd;   /* [ 19:  8] Manufacturing date (SD) */
			/* [  7:  1] CRC (rwe) */
} CID_regs;

#define MMCSD_CID_SIZE	16

/* 
 * OCR (Operation Condition Register)
 */
typedef __u32 OCR_regs;

#define MMCSD_OCR_SIZE	4

#define MMCSD_VDD_20_36	0x00ffff00	/* VDD voltage 2.0 ~ 3.6 */
#define MMCSD_VDD_26_36	0x00ffc000	/* VDD voltage 2.6 ~ 3.6 */
#define MMCSD_VDD_27_36	0x00ff8000	/* VDD voltage 2.7 ~ 3.6 */
#define MMCSD_VDD_20_21	0x00000100	/* VDD voltage 2.0 ~ 2.1 */
#define MMCSD_VDD_21_22	0x00000200	/* VDD voltage 2.1 ~ 2.2 */
#define MMCSD_VDD_22_23	0x00000400	/* VDD voltage 2.2 ~ 2.3 */
#define MMCSD_VDD_23_24	0x00000800	/* VDD voltage 2.3 ~ 2.4 */
#define MMCSD_VDD_24_25	0x00001000	/* VDD voltage 2.4 ~ 2.5 */
#define MMCSD_VDD_25_26	0x00002000	/* VDD voltage 2.5 ~ 2.6 */
#define MMCSD_VDD_26_27	0x00004000	/* VDD voltage 2.6 ~ 2.7 */
#define MMCSD_VDD_27_28	0x00008000	/* VDD voltage 2.7 ~ 2.8 */
#define MMCSD_VDD_28_29	0x00010000	/* VDD voltage 2.8 ~ 2.9 */
#define MMCSD_VDD_29_30	0x00020000	/* VDD voltage 2.9 ~ 3.0 */
#define MMCSD_VDD_30_31	0x00040000	/* VDD voltage 3.0 ~ 3.1 */
#define MMCSD_VDD_31_32	0x00080000	/* VDD voltage 3.1 ~ 3.2 */
#define MMCSD_VDD_32_33	0x00100000	/* VDD voltage 3.2 ~ 3.3 */
#define MMCSD_VDD_33_34	0x00200000	/* VDD voltage 3.3 ~ 3.4 */
#define MMCSD_VDD_34_35	0x00400000	/* VDD voltage 3.4 ~ 3.5 */
#define MMCSD_VDD_35_36	0x00800000	/* VDD voltage 3.5 ~ 3.6 */
#define MMCSD_nCARD_BUSY	0x80000000	/* Card Power up status bit */

/* 
 * Relative Card Address 
 */
typedef __u16 RCA_regs;

#define MMCSD_RCA_SIZE	2


/* 
 * CSD register, rwe == read/write/erase 
 */
typedef struct {
	__u8 csd;		/* [127:126] CSD structure */
	__u8 spec_vers;		/* [125:122] Spec version (MMCSD) */
	struct {
		__u8 man;	/* [118:115] time mantissa */
		__u8 exp;	/* [114:113] time exponent */
	} taac;			/* [119:112] Data read access-time-1 */
	__u8 nsac;		/* [111:104] Data read access-time-2 in CLK cycle */
	struct {
		__u8 man;	/* [103:100] rate mantissa */
		__u8 exp;	/* [ 99: 97] rate exponent */
	} tran_speed;		/* [103: 96] Max. data transfer rate */
	__u16 ccc;		/* [ 95: 84]  Card command classes */
	__u8 read_len;		/* [ 83: 80] Max. read data block length */
	__u8 read_part;		/* [ 79: 79] Partial blocks for read allowed */
	__u8 write_mis;		/* [ 78: 78] write block misalignment */
	__u8 read_mis;		/* [ 77: 77] read block misalignment */
	__u8 dsr;		/* [ 76: 76] DSR implemented */
				/* [ 75: 74] Reserved */
	__u16 c_size;		/* [ 73: 62] Device size */
	__u8 vcc_r_min;		/* [ 61: 59] Max. read current at Vcc min */
	__u8 vcc_r_max;		/* [ 58: 56] Max. read current at Vcc max */
	__u8 vcc_w_min;		/* [ 55: 53] Max. write current at Vcc min */
	__u8 vcc_w_max;		/* [ 52: 50] Max. write current at Vcc max */
	__u8 c_size_mult;	/* [ 49: 47] Device size multiplier */
	__u8 er_blk_en;		/* [ 46: 46] Erase single block enable (SD) */
	__u8 er_size;		/* [ 46: 42] Erase sector size (MMCSD) */
				/* [ 45: 39] Erase sector size (SD) */
	__u8 er_grp_size;	/* [ 41: 37] Erase group size (MMCSD) */
	__u8 wp_grp_size;	/* [ 36: 32] Write protect group size (MMCSD)*/
				/* [ 38: 32] Write Protect group size (SD) */	
	__u8 wp_grp_en;		/* [ 31: 31] Write protect group enable */
	__u8 dflt_ecc;		/* [ 30: 29] Manufacturer default ECC (MMCSD) */
	__u8 r2w_factor;	/* [ 28: 26] Write speed factor */
	__u8 write_len;		/* [ 25: 22] Max. write data block length */
	__u8 write_part;	/* [ 21: 21] Partial blocks for write allowed */
				/* [ 20: 17] Reserved */
				/* [ 16: 16] Content protection application */
	__u8 ffmt_grp;		/* [ 15: 15] File format group (rw) */
	__u8 copy;		/* [ 14: 14] Copy flag (OTP) (rw) */
	__u8 perm_wp;		/* [ 13: 13] Permanent write protection (rw) */
	__u8 tmp_wp;		/* [ 12: 12] temporary write protection (rwe) */
	__u8 ffmt;		/* [ 11: 10] file format (rw) */
	__u8 ecc;		/* [  9:  8] ECC (MMCSD) (rwe) */
				/* [  7:  1] CRC (rwe) */
} CSD_regs;

#define MMCSD_CSD_SIZE	16

#define CSD_VERSION_10	0
#define CSD_VERSION_11	1

#define MMCSD_PROT_10	0	/* MMCSD protocol version 1.0 - 1.2 */
#define MMCSD_PROT_14	1	/* MMCSD protocol version 1.4 */

#define TAAC_EXP_1NS	0	/* 1ns */
#define TAAC_EXP_10NS	1	/* 10ns */
#define TAAC_EXP_100NS	2	/* 100ns */
#define TAAC_EXP_1UMS	3	/* 1 u-ms */
#define TAAC_EXP_10UMS	4	/* 10 u-ms */
#define TAAC_EXP_100UMS	5	/* 100 u-ms */
#define TAAC_EXP_1MS	6	/* 1ms */
#define TAAC_EXP_10MS	7	/* 10ms */

#define TIME_MAN_NONE	0	/* reserved */
#define TIME_MAN_10	1	/* 1.0 */
#define TIME_MAN_12	2	/* 1.2 */
#define TIME_MAN_13	3	/* 1.3 */
#define TIME_MAN_15	4	/* 1.5 */
#define TIME_MAN_20	5	/* 2.0 */
#define TIME_MAN_25	6	/* 2.5 */
#define TIME_MAN_30	7	/* 3.0 */
#define TIME_MAN_35	8	/* 3.5 */
#define TIME_MAN_40	9	/* 4.0 */
#define TIME_MAN_45	A	/* 4.5 */
#define TIME_MAN_50	B	/* 5.0 */
#define TIME_MAN_55	C	/* 5.5 */
#define TIME_MAN_60	D	/* 6.0 */
#define TIME_MAN_70	E	/* 7.0 */
#define TIME_MAN_80	F	/* 8.0 */

#define TRAN_EXP_100K	0	/* 100kbit/s */
#define TRAN_EXP_1M	1	/* 1Mbit/s */
#define TRAN_EXP_10M	2	/* 10Mbit/s */
#define TRAN_EXP_100M	3	/* 100Mbit/s */

#define CCC_CLASS_0	0x001	/* Card Command Class 0 */
#define CCC_CLASS_1	0x002	/* Card Command Class 1 */
#define CCC_CLASS_2	0x004	/* Card Command Class 2 */
#define CCC_CLASS_3	0x008	/* Card Command Class 3 */
#define CCC_CLASS_4	0x010	/* Card Command Class 4 */
#define CCC_CLASS_5	0x020	/* Card Command Class 5 */
#define CCC_CLASS_6	0x040	/* Card Command Class 6 */
#define CCC_CLASS_7	0x080	/* Card Command Class 7 */
#define CCC_CLASS_8	0x100	/* Card Command Class 8 */
#define CCC_CLASS_9	0x200	/* Card Command Class 9 */
#define CCC_CLASS_10	0x400	/* Card Command Class 10 */
#define CCC_CLASS_11	0x800	/* Card Command Class 11 */

#define BLK_LEN_1	0	/* 2^0 = 1 byte */
#define BLK_LEN_2	1	/* 2^1 = 2 bytes */
#define BLK_LEN_4	2	/* 2^2 = 4 bytes */
#define BLK_LEN_8	3	/* 2^3 = 8 bytes */
#define BLK_LEN_16	4	/* 2^4 = 16 bytes */
#define BLK_LEN_32	5	/* 2^5 = 32 bytes */
#define BLK_LEN_64	6	/* 2^6 = 64 bytes */
#define BLK_LEN_128	7	/* 2^7 = 128 bytes */
#define BLK_LEN_256	8	/* 2^8 = 256 bytes */
#define BLK_LEN_512	9	/* 2^9 = 512 bytes */
#define BLK_LEN_1024	10	/* 2^10 = 1024 bytes */
#define BLK_LEN_2048	11	/* 2^11 = 2048 bytes */
#define MAX_MMCSD_BLK_LEN	2048

/*
	Memory Capacity = BLOCKNR * BLOCK_LEN
	where
	BLOCKNR = (C_SIZE + 1) * MULT
	MULT = 2^(c_size_mult+2) ( c_size_mult < 8)
	BLOCK_LEN = 2^read_len	( read_len < 12)
 */

#define VCC_MIN_05	0	/* 0.5mA */
#define VCC_MIN_1	1	/* 1mA */
#define VCC_MIN_5	2	/* 5mA */
#define VCC_MIN_10	3	/* 10mA */
#define VCC_MIN_25	4	/* 25mA */
#define VCC_MIN_35	5	/* 35mA */
#define VCC_MIN_60	6	/* 60mA */
#define VCC_MIN_100	7	/* 100mA */

#define VCC_MAX_1	0	/* 1mA */
#define VCC_MAX_5	1	/* 5mA */
#define VCC_MAX_10	2	/* 10mA */
#define VCC_MAX_25	3	/* 25mA */
#define VCC_MAX_35	4	/* 35mA */
#define VCC_MAX_45	5	/* 45mA */
#define VCC_MAX_80	6	/* 80mA */
#define VCC_MAX_200	7	/* 200mA */

				/* the typical block program time is */
#define R2W_FACTOR_1	0	/*   1 Multiples of Read Access Time */
#define R2W_FACTOR_2	1	/*   2 Multiples of Read Access Time */
#define R2W_FACTOR_4	2	/*   4 Multiples of Read Access Time */
#define R2W_FACTOR_8	3	/*   8 Multiples of Read Access Time */
#define R2W_FACTOR_16	4	/*  16 Multiples of Read Access Time */
#define R2W_FACTOR_32	5	/*  32 Multiples of Read Access Time */

#define ECC_NONE	0	/* none. Max.No.ofCorrectable bit/block=0 */
#define ECC_BCH		1	/* 542,512. Max.No.ofCorrectable bit/block=3 */

/* SD only */
/* 
 * SCR(SD CARD Configuration Register)
 */
typedef struct{
	__u8 scr_version;	/* SCR version */
	__u8 sd_spec_version;	/* physical layer spec version */
	__u8 data_status;	/* data status after erase */
	__u8 sd_security;	/* security protocol */
	__u8 sd_bus_widths;	/* bus width */  
} SCR_regs;

#define MMCSD_SCR_SIZE	8

#define SCR_VERSION_10		0	/* SCR version 1.0 */
#define SPEC_VERSION_10		0	/* physical layer spec version 1.0 */
#define SECURITY_PROT_NONE	0	/* no security */
#define SECURITY_PROT_1		1	/* security protocol 1.0 */
#define SECURITY_PROT_2		2	/* security protocol 2.0 */
#define BUS_WIDTH_1		0	/* 1 bit (DAT0) */
#define BUS_WIDTH_4		1	/* 4 bits (DAT0-3) */

/*
 * R1 status: card status
 * Type
 *	e : error bit
 *	s : status bit
 *	r : detected and set for the actual command response
 *	x : detected and set during command execution. the host must poll
 *	    the card by sending status command in order to read these bits.
 * Clear condition
 *	a : according to the card state
 *	b : always related to the previous command. Reception of
 *	    a valid command will clear it (with a delay of one command)
 *	c : clear by read
 */
typedef __u32 R1_status;
#define R1_out_of_range		0x80000000	/* er, c */
#define R1_address_err		0x40000000	/* erx, c */
#define R1_block_len_err	0x20000000	/* er, c */
#define R1_erase_seq_err	0x10000000	/* er, c */
#define R1_erase_param		0x08000000	/* ex, c */
#define R1_wp_violation		0x04000000	/* erx, c */
#define R1_card_is_locked	0x02000000	/* sx, a */
#define R1_lock_unlock_fail	0x01000000	/* erx, c */
#define R1_com_crc_err		0x00800000	/* er, b */
#define R1_illegal_command	0x00400000	/* er, b */
#define R1_ecc_fail		0x00200000	/* ex, c */
#define R1_cc_err		0x00100000	/* erx, c */
#define R1_err			0x00080000	/* erx, c */
#define R1_underrun		0x00040000	/* ex, c */
#define R1_overrun		0x00020000	/* ex, c */
#define R1_overwrite		0x00010000	/* erx, c, CID/CSD overwrite */
#define R1_wp_erase_skip	0x00008000	/* sx, c */
#define R1_ecc_disable		0x00004000	/* sx, a */
#define R1_erase_reset		0x00002000	/* sr, c */
#define R1_state		0x00001E00	/* sx, b */
#define R1_buf_empty		0x00000100	/* sx, a */
#define R1_app_cmd		0x00000020	/* sr, c */
#define R1_ERR			0xFDFF0000	/* R1 error mask */

/* SD only */
#define R1_ake_seq_err		0x00000008	/* sr, c */

/* card state flags of R1 */
#define STATE_IDLE	0x00000000	/* 0 */
#define STATE_READY	0x00000200	/* 1 */
#define STATE_IDENT	0x00000400	/* 2 */
#define STATE_STBY	0x00000600	/* 3 */
#define STATE_TRAN	0x00000800	/* 4 */
#define STATE_DATA	0x00000A00	/* 5 */
#define STATE_RCV	0x00000C00	/* 6 */
#define STATE_PRG	0x00000E00	/* 7 */
#define STATE_DIS	0x00001000	/* 8 */

/* SD only */
/* 
 * R6 status: RCA and a part of card status
 */
typedef struct{
	__u16 rca;             /* RCA */
	__u16 card_status;     /* card status bits[23,22,19,12:0] */
} R6_status;


/* flags for stat field of the mmcsd_slot structure */
#define MMCSD_WP_GRP_EN		0x00000010
#define MMCSD_PERM_WP		0x00000020
#define MMCSD_TMP_WP		0x00000040
#define MMCSD_READ_PART		0x00000100
#define MMCSD_WRITE_PART		0x00000200

/*
 * MMC/SD Clock rates
 */
#define MMCSD_MMC_CLOCK_HIGH		20000000
#define MMCSD_MMC_CLOCK_LOW		400000
#define MMCSD_SD_CLOCK_HIGH		25000000
#define MMCSD_SD_CLOCK_LOW		MMCSD_MMC_CLOCK_LOW

/*
 * MMCSD command/response structure
 */

/* MMCSD Response type */
enum {
	MMCSD_RES_TYPE_NONE = 0,
	MMCSD_RES_TYPE_R1,
	MMCSD_RES_TYPE_R1B,
	MMCSD_RES_TYPE_R2,
	MMCSD_RES_TYPE_R3,
	MMCSD_RES_TYPE_R4,
	MMCSD_RES_TYPE_R5,
	MMCSD_RES_TYPE_R6,
};

/* MMCSD Response length */
#define MMCSD_RES_LEN_SHORT		6
#define MMCSD_RES_LEN_LONG		17

/* MMCSD Response flag */
#define MMCSD_RES_FLAG_DATALINE		0x01 /* transferred on dataline */
#define MMCSD_RES_FLAG_NOCRC		0x02 /* no crc check */
#define MMCSD_RES_FLAG_RDATA		0x04 /* data read */
#define MMCSD_RES_FLAG_WDATA		0x08 /* data write */

/* MMCSD Command request type */
struct mmcsd_cmd {
	__u8 cmd; /* command */
	__u32 arg; /* command argument */
	__u8 res_type; /* response type */
	__u8 res_flag; /* response flag */
	__u8 res[MMCSD_RES_LEN_LONG]; /* response buffer */
	__u8 *data; /* pointer to data buffer */
	__u32 data_len; /* data length */
	__u32 t_res; /* timing between command and response */
	__u32 t_fin; /* timing after response */
};

/*
 * MMC/SD slot interface
 */

//#define MAX_MMCSD_SLOTS		2
#define MAX_MMCSD_SLOTS		1

struct mmcsd_slot {
	/* power up */
	void (*power_up)(struct mmcsd_slot *slot);

	/* power down */
	void (*power_down)(struct mmcsd_slot *slot);

	/* wait for reset */
	void (*wait_for_reset)(struct mmcsd_slot *slot);

	/* set clock rate */
	void (*set_clock)(struct mmcsd_slot *slot, int rate);

	/* send command and receive response */
	int (*send_cmd)(struct mmcsd_slot *slot, struct mmcsd_cmd *cmd);

	/* transfer 1 block for Memory Card */
	int (*transfer1b)(struct mmcsd_slot *slot, int rd, u_long from, u_char *buf);


	struct semaphore mutex; /* for exclusive I/O */

	__u8 id;	/* slot id(begins at 0) assigned by slot driver */
	__u8 sd;	/* if true, it's a SD card */
	__u8 bus_width;	/* bus width for SD card(0x0: 1 bit, 0x2: 4 bits) */
	__u8 narrow_bus; /* if true, support only narrow bus */
	__u8 cnt;
	__u8 *type;

	/* MMC/SD card information */
	unsigned int read_len;	/* read block length */
	unsigned int write_len;	/* write block length */
	u_long size;	/* total size of card in bytes */
	u_long stat;	/* card status */
	__u8 readonly;	/* If true, it's readonly */

	/* Temporary registers */
	RCA_regs rca;
	CID_regs cid;
	OCR_regs ocr;
	CSD_regs csd;
	R1_status r1;
	SCR_regs scr;
	R6_status r6;

	void *priv;
};

/*
 * MMC/SD device notifier structure
 */
struct mmcsd_notifier
{
	void (*add)(struct mmcsd_slot *slot);
        void (*remove)(struct mmcsd_slot *slot);
        struct mmcsd_notifier *next;
};

/*
 * Function prototypes
 */

/* Data Conversion functions */
static inline void mmcsd_str2r1(R1_status *r1, __u8 *buff)
{
	*r1 = (buff[0] << 24) | (buff[1] << 16) | (buff[2] << 8) | buff[3];
}

static inline void mmcsd_str2ocr(OCR_regs *regs, __u8 *buff)
{
	*regs = (buff[0] << 24) | (buff[1] << 16) | (buff[2] << 8) | buff[3];
}

static inline void mmcsd_str2rca(RCA_regs *regs, __u8 *buff)
{
	*regs = (buff[0] << 8) | buff[1];
}

/* SD only */
static inline void mmcsd_str2r6(R6_status * r6, __u8 *buff)
{
	r6->rca = (buff[0] << 8) | buff[1];
	r6->card_status = (buff[2] << 8) | buff[3];
}
extern void mmcsd_str2cid( CID_regs *regs, __u8 *buff, __u8 id);
extern void mmcsd_str2csd( CSD_regs *regs, __u8 *buff, __u8 id);

/* Information Collecting functions */
extern void mmcsd_get_CSD_info(struct mmcsd_slot *slot, CSD_regs *csd);

#if defined(CONFIG_MACH_MMSP2_MDK)
static const int block=1;
extern const int mmcsd_res_len[];
static inline int mmcsd_get_res_len(int res_type)
{
	return mmcsd_res_len[res_type];
}
static DECLARE_MUTEX(mmcsd_slots_mutex);
#endif

/* Slot and User functions */
extern int add_mmcsd_device(struct mmcsd_slot *slot);
extern int del_mmcsd_device(struct mmcsd_slot *slot);
extern int reidentify_mmcsd_device(struct mmcsd_slot *slot);
extern void register_mmcsd_user(struct mmcsd_notifier *new);
extern int unregister_mmcsd_user(struct mmcsd_notifier *old);
extern void mmcsd_init(void);


/*DEBUG*/
//#define MMCSD_DEBUG
#ifdef MMCSD_DEBUG
	#define DPRINTK(args...)        printk(args)
#else
	#define DPRINTK(args...)
#endif

#define SD_DEBUG	0
#if	SD_DEBUG
	#define DPRINTK1(args...)        printk(args)
#else
	#define DPRINTK1(args...)
#endif

//DPRINTK("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);

#endif /* ! MMCSD_H */
