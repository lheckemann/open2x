/*
 * drivers/mmcsd/mmcsd_slot.h
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * MMC/SD slot interfaces specific to MMSP2 SD Controller
 *
 */

#ifndef MMCSD_SLOT_H
#define MMCSD_SLOT_H

#include <asm/hardware.h>
#include "mmcsd.h"

#ifdef CONFIG_MMSP2_MDK  
#define bSDI(Nb)	__REGW(0xC0001500 + (Nb))
#define bSDI32(Nb)	__REG(0xC0001500 + (Nb))
#else /* S3C2440 */
#define bSDI(Nb)	__REG(0x5A000000 + (Nb))
#endif

#define SDICON		bSDI(0x00)	/* SDI Control Register */
#define SDIPRE		bSDI(0x02)	/* SDI Prescaler Register */
#define SDICARG		bSDI32(0x04)	/* SDI Command Argument Register */
#define SDICCON		bSDI(0x08)	/* SDI Command Control Register */
#define SDICSTA		bSDI(0x0A)	/* SDI Command Status Register */
#define SDIRSP0		bSDI32(0x0C)	/* SDI Response Register 0 */
#define SDIRSP1		bSDI32(0x10)	/* SDI Response Register 1 */
#define SDIRSP2		bSDI32(0x14)	/* SDI Response Register 2 */
#define SDIRSP3		bSDI32(0x18)	/* SDI Response Register 3 */
#define SDIBSIZE	bSDI(0x1E)	/* SDI Block Size Register */
#define SDIDCON		bSDI32(0x20)	/* SDI Data Control Register */
#define SDIDCNT		bSDI32(0x24)	/* SDI Data Remain Counter Register */
#define SDIDSTA		bSDI(0x28)	/* SDI Data Status Register */
#define SDIFSTA		bSDI(0x2A)	/* SDI FIFO Status Register */
#define SDIDAT		bSDI32(0x2C)	/* SDI Data Register */
//#define SDIIMSK		bSDI32(0x30)	/* SDI Interrupt Mask Register 0 */
#define SDIIMSK0	bSDI32(0x30)	/* SDI Interrupt Mask Register 0 */
#define SDIIMSK1	bSDI32(0x32)	/* SDI Interrupt Mask Register 0 */
#define SDIDTIMERL	bSDI(0x36)	/* SDI Data Timer Register */
#define SDIDTIMERH	bSDI(0x38)	/* SDI Data Timer Register */
#define SDISFTL		bSDI(0x3A)	/* SDI Shift Regisrt Low */
#define SDISFTH		bSDI(0x3C)	/* SDI Shift Regisrt High */

#define SDICON_BYTE	(1 << 4)	/* Byte Order Type */
#define SDICON_LE	(0 << 4)	/* D[7:0],D[15:8],D[23:16],D[31:24] */
#define SDICON_BE	(1 << 4)	/* D[31:24],D[23:16],D[15:8],D[7:0] */
#define SDICON_INT	(1 << 3)	/* Receive SDIO INterrupt from card */
#define SDICON_RWE	(1 << 2)	/* Read Wait Enable */
#ifdef  CONFIG_MMSP2_MDK 		
#define SDICON_FRESET	(1 << 1)	/* FIFO Reset */
#endif
#define SDICON_ENCLK	(1 << 0)	/* Clock Out Enable */
#define SDICON_DISCLK	(0 << 0)	/* Clock Out Disable */

#define SDIPRE_MSK	(0xff)		/* Baud rate = PCLK/2/(value + 1) */

#define SDICCON_ABORT	(1 << 12)	/* Command type: Abort(CMD12,CMD52) */
#define SDICCON_DATA	(1 << 11)	/* Command type: with Data */
#define SDICCON_LRSP	(1 << 10)	/* Response is 136-bit long */
#define SDICCON_WRSP	(1 << 9)	/* Wait for Response */
#define SDICCON_START	(1 << 8)	/* 0: cmd ready, 1: cmd start */
#define SDICCON_CMD_MSK	(0xff)		/* with start 2bit */

#define SDICSTA_CRC	(1 << 12)	/* CRC error */
#define SDICSTA_SENT	(1 << 11)	/* Command sent */
#define SDICSTA_TOUT	(1 << 10)	/* Command Timeout */
#define SDICSTA_RSP	(1 << 9)	/* Command Response received */
#define SDICSTA_BUSY	(1 << 8)	/* Command transfer in progress */
#define SDICSTA_ALLFLAG	(0x1f00)	/* All flags mask */
#define SDICSTA_RSP_MSK	(0xff)		/* with start 2bit */
#define SDICSTA_ERR	(0x1400)	/* CRC, TOUT */

#define SDIRSP1_CRC7	FExtr(SDIRSP1, Fld(8,24))

#define SDIDCON_DATA_SIZE_WORD (2 << 22)	/* SDIO Interrupt period is ... */

#define SDIDCON_PRD	(1 << 21)	/* SDIO Interrupt period is ... */
				/* when last data block is transferred. */
#define SDIDCON_PRD_2	(0 << 21)	/* 0: exact 2 cycle */
#define SDIDCON_PRD_N	(1 << 21)	/* 1: more cycle */
#define SDIDCON_TARSP	(1 << 20)	/* when data transmit start ... */
#define SDIDCON_TARSP_0	(0 << 20)	/* 0: directly after DatMode set */
#define SDIDCON_TARSP_1	(1 << 20)	/* 1: after response receive */
#define SDIDCON_RACMD	(1 << 19)	/* when data receive start ... */
#define SDIDCON_RACMD_0	(0 << 19)	/* 0: directly after DatMode set */
#define SDIDCON_RACMD_1	(1 << 19)	/* 1: after command sent */
#define SDIDCON_BACMD	(1 << 18)	/* when busy receive start ... */
#define SDIDCON_BACMD_0	(0 << 18)	/* 0: directly after DatMode set */
#define SDIDCON_BACMD_1	(1 << 18)	/* 1: after command sent */
#define SDIDCON_BLK	(1 << 17)	/* transfer mode. 0:stream, 1:block */
#define SDIDCON_WIDE_1	(1 << 16)	/* Enable Wide bus mode */
#define SDIDCON_WIDE_0	(0 << 16)	/* Enable Wide bus mode */
#define SDIDCON_DMA	(1 << 15)	/* Enable DMA */
#define SDIDCON_DATA_START	(1 << 14)	/* 24x0a data transfer start */
#define SDIDCON_DATA_MODE_Rx (2 << 12)	/* which direction of data transfer */
#define SDIDCON_DATA_MODE_Tx (3 << 12)	/* which direction of data transfer */
#define SDIDCON_DATA_STOP ~(7 << 12)	/* which direction of data transfer */
#define fSDIDCON_DatMode Fld(2,12)	/* which direction of data transfer */
#define SDIDCON_READY	FInsrt(0x0, fSDIDCON_DatMode)	/* ready */
#define SDIDCON_BUSY	FInsrt(0x1, fSDIDCON_DatMode)	/* only busy check */
#define SDIDCON_RX	FInsrt(0x2, fSDIDCON_DatMode)	/* Data receive */
#define SDIDCON_TX	FInsrt(0x3, fSDIDCON_DatMode)	/* Data transmit */
#define SDIDCON_BNUM	FMsk(Fld(12,0))	/* Block Number(0~4095) */

#define SDIDCNT_CNT(x)	FExtr((x), Fld(12,12))	/* Remaining Blk No. */
#define SDIDCNT_SIZE(x)	FExtr((x), Fld(12,0))	/* Remaining byte of 1 block */

#define SDIDSTA_RWQ	(1 << 10)	/* Read wait request occur */
#define SDIDSTA_INT	(1 << 9)	/* SDIO interrupt detect */
#ifdef  CONFIG_S3C2410X_SMDK 		/* reserved in 2440A and 24A0A */
#define SDIDSTA_EFIFO	(1 << 8)	/* FIFO fail */
#endif
#define SDIDSTA_ETCRC	(1 << 7)	/* Block sent(CRC status error) */
#define SDIDSTA_ERCRC	(1 << 6)	/* Block received(CRC error) */
#define SDIDSTA_TOUT	(1 << 5)	/* Data/Busy receive timeout */
#define SDIDSTA_DFIN	(1 << 4)	/* Data transfer complete */
#define SDIDSTA_BFIN	(1 << 3)	/* Only busy check is finished */
#ifdef  CONFIG_S3C2410X_SMDK 		/* reserved in 2440A and 24A0A */
#define SDIDSTA_SBIT	(1 << 2)	/* Start bit is not detected */
#endif
#define SDIDSTA_TX	(1 << 1)	/* data Tx in progress */
#define SDIDSTA_RX	(1 << 0)	/* data Rx in progress */
#ifdef  CONFIG_S3C2410X_SMDK 		/* reserved in 2440A and 24A0A */
#define SDIDSTA_ERR	(0x1e4)		/* EFIFO, ETCRC, ERCRC, TOUT, SBIT */
#else
#define SDIDSTA_ERR	(0xe)		/* ETCRC, ERCRC, TOUT */
#endif
#define fSDIDSTA_ALL	Fld(11, 0)
#define SDIDSTA_ALL	FMsk(fSDIDSTA_ALL)

#define SDIFSTA_TX	(1 << 13)	/* 0: FIFO is full, 1: FIFO ready */
#define SDIFSTA_RX	(1 << 12)	/* 0: FIFO empty, 1: Data in FIFO */
#define SDIFSTA_TX_HALF	(1 << 11)	/* Data in FIFO are 0:33~64, 1:0~32 */
#define SDIFSTA_TX_EMP	(1 << 10)	/* FIFO is empty */
#define SDIFSTA_RX_LAST	(1 << 9)	/* Last Data ready in FIFO */
#define SDIFSTA_RX_FULL	(1 << 8)	/* FIFO is full */
#if defined(CONFIG_S3C2440A_SMDK) || defined(CONFIG_S3C24A0A_SMDK) || defined(CONFIG_MMSP2_MDK) /* only for 24X0A version*/
#define SDIFSTA_FIFO_RESET (1 << 16)	/* FIFO is full */
//#define SDIFSTA_FIFO_RESET (0 << 16)	/* FIFO is full */
#endif
#define SDIFSTA_RX_HALF	(1 << 7)	/* Data in FIFO are 0:0~31, 1:32~64 */
#define SDIFSTA_CNT	(0x7f)		/* Number of data in FIFO */
#if defined(CONFIG_S3C2440A_SMDK) || defined(CONFIG_S3C24A0A_SMDK) || defined(CONFIG_MMSP2_MDK) /* only for 24X0A version*/
#define SDIFSTA_LAST_RX	0x200  		/* 24x0a rx last clear */
#endif

#if defined(CONFIG_S3C2440A_SMDK) || defined(CONFIG_S3C24A0A_SMDK) || defined(CONFIG_MMSP2_MDK) /* only for 24X0A version*/
#define SDIIMSK_NOBUSY  (1 << 18)       /* No busy Interrupt */
#endif
#define SDIIMSK_RCRC	(1 << 17)	/* Response CRC error */
#define SDIIMSK_CSENT	(1 << 16)	/* Command Sent */
#define SDIIMSK_CTOUT	(1 << 15)	/* Command Response Timeout */
#define SDIIMSK_CRSP	(1 << 14)	/* Command Response received */
#define SDIIMSK_RWAIT	(1 << 13)	/* Read Wait Request */
#define SDIIMSK_CARD	(1 << 12)	/* SDIO Interrupt from card */
#define SDIIMSK_EFIFO	(1 << 11)	/* FIFO fail error */
#define SDIIMSK_ETCRC	(1 << 10)	/* CRC status error */
#define SDIIMSK_ERCRC	(1 << 9)	/* Data CRC fail */
#define SDIIMSK_TOUT	(1 << 8)	/* Data Timeout */
#define SDIIMSK_DFIN	(1 << 7)	/* Data transfer complete (cnt = 0) */
#define SDIIMSK_BFIN	(1 << 6)	/* Busy check complete */
#define SDIIMSK_SBIT	(1 << 5)	/* Start bit Error */
#define SDIIMSK_TX_HALF	(1 << 4)	/* Tx FIFO half */
#define SDIIMSK_TX_EMP	(1 << 3)	/* Tx FIFO empty */
#define SDIIMSK_RX_LAST	(1 << 2)	/* Rx FIFO has last data */
#define SDIIMSK_RX_FULL	(1 << 1)	/* Rx FIFO full */
#define SDIIMSK_RX_HALF	(1 << 0)	/* Rx FIFO half */
#define SDIIMSK_ALL	(0x3ffff)

#define SDI_MAX_TX_FIFO		64
#define SDI_MAX_RX_FIFO		64

#define EXT_BOTH_EDGES		6
#define MMCSD_RETRIES_MAX	100
#define MMCSD_RETRIES_MIN	40
#define MMCSD_RETRIES_CHECK	2

#define MMCSD_OP_COND_DELAY	set_current_state(TASK_INTERRUPTIBLE); \
				schedule_timeout(HZ/10)

/* MMC/SD Clock */
#define MMCSD_PCLK	50000000	/* 50 MHz */
#if 0
#define MMC_NORCLK	7500000		/* 7.5 MHz */
#define SD_NORCLK	12000000	/* 12 MHz */
#else
#define MMC_NORCLK	8000000		/* 8 MHz */
#define SD_NORCLK	2000000		/* 2 MHz */
#endif
#define MMCSD_INICLK	400000		/* 400 KHz */

/* Board-specific definitions */
#if 0
#define USE_POLLING		
#else
#define USE_INTERRUPT		
#endif

/* card detect */
#define NO_CD_IRQ		// no card detect interrupt

#define DATA_DUMP	0
#define COMMAND_DUMP	0

#ifdef USE_INTERRUPT
static DECLARE_WAIT_QUEUE_HEAD(wq);
#endif

#endif /* MMCSD_MMSP2_H */

