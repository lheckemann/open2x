/*
 *  linux/arch/arm/mach-mmsp2/dma.h
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


/* controll registers */
#define REQ_ONCE_4BYTE		(0 << 14)
#define REQ_ONCE_16BYTE		(1 << 14)
#define REQ_ONCE_32BYTE		(2 << 14)
#define REQ_TWICE_16BYTE	(3 << 14)
#define SRC_ADDRINCEN		(1 << 13)
#define SRC_FLOW		(1 << 12)
#define SRC_MEMFMT_WORD		(0 << 10)
#define SRC_MEMFMT_LSHW		(1 << 10)
#define SRC_MEMFMT_MSHW		(2 << 10)
#define SRC_DW_BYTE		(0 << 8)
#define SRC_DW_HW		(1 << 8)
#define TRG_ADDRINCEN		(1 << 5)
#define TRG_FLOW		(1 << 4)
#define TRG_MEMFMT_WORD		(0 << 2)
#define TRG_MEMFMT_LSHW		(1 << 2)
#define TRG_MEMFMT_MSHW		(2 << 2)
#define TRG_DW_BYTE		(0 << 0)
#define TRG_DW_HW		(1 << 0)

#define SRCADDR_INC		(1 << 8)
#define SRCADDR_DEC		(-1 << 8)
#define SRCADDR_FIX		(0 << 8)
#define TRGADDR_INC		(1 << 0)
#define TRGADDR_DEC		(-1 << 0)
#define TRGADDR_FIX		(0 << 0)

#define DMA_IRQ_DONE_EN		(1 << 9)

#define ADDR_MEM		0xffffffff
#define ADDR_DEV_AC97		0xc0000e00
#define ADDR_DEV_AC97_FADDR	0x01a00000
#define ADDR_DEV_AC97_RADDR	0x01a20000
#define ADDR_DEV_AC97_CADDR	0x01a40000
#define ADDR_DEV_AC97_IADDR	0x01a60000
#define ADDR_DEV_AC97_MADDR	0x01a80000


#define MAX_DMA_BUF_SIZE	(64*1024)


/* DMA buffer struct */
typedef struct dma_buf_s {
	int size;		/* buffer size */
	dma_addr_t dma_start;	/* starting DMA address */
	int ref;		/* number of DMA reference */
	void *id;		/* to identify buffer from outside */
	struct dma_buf_s *next;	/* next buffer to process */
} dma_buf_t;

/* DMA control register structure */
typedef struct {
	volatile unsigned short COM0;		/* 0x00, command 0 */
	volatile unsigned short COM1;		/* 0x02, command 1 */
	volatile unsigned short COM2;		/* 0x04, command 2 */
	volatile unsigned short CONS;		/* 0x06, control/status */
	volatile unsigned short SRCLADDR;	/* 0x08, low address */
	volatile unsigned short SRCHADDR;	/* 0x0a, high address */
	volatile unsigned short TRGLADDR;	/* 0x0c, low address */
	volatile unsigned short TRGHADDR;	/* 0x0e, high address */
} dma_regs_t;

typedef struct {
	volatile unsigned short SRM;	/* source I/O request map */
	volatile unsigned short TRM;	/* target I/O request map */
} dma_iomap_t;

typedef struct {
	const char *name;
	int type;
	unsigned short com0;
	unsigned short com1;
	unsigned short con;
	unsigned long srcaddr;
	unsigned long trgaddr;
} dma_devinfo_t;


#define DMA_TYPE_READ		(0)
#define DMA_TYPE_WRITE		(1)

/* DMA channel structure */
typedef struct {
	dmach_t channel;
	unsigned int in_use;	/* Device is allocated */
	int device_id;		/* Device ID */
	dma_buf_t *head;	/* where to insert buffers */
	dma_buf_t *tail;	/* where to remove buffers */
	dma_buf_t *curr;	/* buffer currently DMA'ed */
	unsigned long queue_count;	/* number of buffers in the queue */
	int active;		/* 1 if DMA is actually processing data */
	dma_regs_t *regs;	/* points to appropriate DMA registers */
	dma_iomap_t *iomap;	/* points to appropriate I/O registers */
	int irq;		/* IRQ used by the channel */
	dma_devinfo_t *devinfo;
	dma_callback_t callback;
} mmsp2_dma_t;

