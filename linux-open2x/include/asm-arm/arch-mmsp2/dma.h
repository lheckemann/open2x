/*
 * linux/include/asm-arm/arch-mmsp2/dma.h
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H

#include "hardware.h"

#define MAX_DMA_ADDRESS		0xffffffff

/*
 * It means that this architecture doesn't use the generic DMA interface
 * provided by kernel.
 */
#define MAX_DMA_CHANNELS		0

#define MAX_MMSP2_DMA_CHANNELS	16

#define DMA_UART0_RX			(0)
#define DMA_UART0_TX			(1)
#define DMA_UART1_RX			(2)
#define DMA_UART1_TX			(3)
#define DMA_UART2_RX			(4)
#define DMA_UART2_TX			(5)
#define DMA_UART3_RX			(6)
#define DMA_UART3_TX			(7)
#define DMA_IRDA_RX				(10)
#define DMA_IRDA_TX				(11)
#define DMA_USB_EP1				(12)
#define DMA_USB_EP2				(13)
#define DMA_USB_EP3				(14)
#define DMA_USB_EP4				(15)
#define DMA_DECSS_RX			(18)
#define DMA_DECSS_TX			(19)
#define DMA_MMCSD_RX			(20)
#define DMA_MMCSD_TX			(21)
#define DMA_MSTICK				(22)
#define DMA_AC97_LRPCM_OUT		(24)
#define DMA_AC97_SLRPCM_OUT		(25)
#define DMA_AC97_CWPCM_OUT		(26)
#define DMA_AC97_MIC_IN			(27)
#define DMA_AC97_LRPCM_IN		(28)
#define DMA_TOUCH				(29)
#define DMA_I2S_RX				(30)
#define DMA_I2S_TX				(31)
#define DMA_SPDIF_RX			(36)
#define DMA_SPDIF_TX			(37)
#define DMA_SSP_RX				(38)
#define DMA_SSP_TX				(39)
#define DMA_MPEG				(54)
#define DMA_SP					(55)
#define DMA_HUFFMAN				(60)
#define DMA_RVLD				(61)

typedef void (*dma_callback_t)(void *buf_id, int size);

extern int mmsp2_request_dma(int device_id, dma_callback_t cb);
extern int mmsp2_dma_queue_buffer(dmach_t channel, void *buf_id, dma_addr_t data, int size);
extern int mmsp2_dma_flush_all(dmach_t channel);
extern void mmsp2_free_dma(dmach_t channel);
extern int mmsp2_dma_get_send_size(dmach_t channel);

#endif /* __ASM_ARCH_DMA_H */
