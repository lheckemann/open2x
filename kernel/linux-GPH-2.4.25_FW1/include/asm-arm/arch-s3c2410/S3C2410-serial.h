/*
 *  linux/include/asm-arm/arch-s3c2410/S3C2410-serial.h
 *
 *  Internal header file for Samsung S3C2410 serial ports (UART0-2)
 *
 *  Copyright (C) 2002 Shane Nay (shane@minirl.com)
 *
 *  Additional defines, (c) 2003 Simtec Electronics (linux@simtec.co.uk)
 *
 *  Adapted from:
 *
 *  Internal header file for MX1ADS serial ports (UART1 & 2)
 *
 *  Copyright (C) 2002 Shane Nay (shane@minirl.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ASMARM_ARCH_S3C2410_SERIAL_H
#define ASMARM_ARCH_S3C2410_SERIAL_H

#define S3C2410_UARTRXH0_OFF      (0x24)
#define S3C2410_UARTTXH0_OFF      (0x20)
#define S3C2410_UARTLCON_OFF      (0x00)
#define S3C2410_UARTCON_OFF       (0x04)
#define S3C2410_UARTFCON_OFF      (0x08)
#define S3C2410_UARTMCON_OFF      (0x0C)
#define S3C2410_UARTBRDIV_OFF     (0x28)
#define S3C2410_UARTTRSTAT_OFF    (0x10)
#define S3C2410_UARTERSTAT_OFF    (0x14)
#define S3C2410_UARTFSTAT_OFF     (0x18)
#define S3C2410_UARTMSTAT_OFF     (0x1C)


#define S3C2410_UART1_OFF         (0x4000)
#define S3C2410_UART2_OFF         (0x8000)

#define S3C2410_LCON_CFGMASK      ((0xF<<3)|(0x3))

#define S3C2410_LCON_CS5          (0x0)
#define S3C2410_LCON_CS6          (0x1)
#define S3C2410_LCON_CS7          (0x2)
#define S3C2410_LCON_CS8          (0x3)

#define S3C2410_LCON_PNONE        (0x0)
#define S3C2410_LCON_PEVEN        ((0x5)<<3)
#define S3C2410_LCON_PODD         ((0x4)<<3)

#define S3C2410_UMCON_AFC         (0x10)
#define S3C2410_UMCON_RTS         (0x1)

#define S3C2410_UMSTAT_CTS        (0x1)

#define S3C2410_UCON_SBREAK       (1<<4)

#define S3C2410_UCON_TXILEVEL     (1<<9)
#define S3C2410_UCON_RXILEVEL     (1<<8)
#define S3C2410_UCON_TXIRQMODE    (1<<2)
#define S3C2410_UCON_RXIRQMODE    (1<<0)
#define S3C2410_UCON_RXFIFO_TOI   (1<<7)

#define S3C2410_UCON_DEFAULT      (S3C2410_UCON_TXILEVEL | S3C2410_UCON_RXILEVEL \
                                   | S3C2410_UCON_TXIRQMODE | S3C2410_UCON_RXIRQMODE \
				   | S3C2410_UCON_RXFIFO_TOI)

#define S3C2410_UFCON_FIFOMODE    (1<<0)
#define S3C2410_UFCON_TXTRIG0     (0<<6)
#define S3C2410_UFCON_RXTRIG8     (1<<4)
#define S3C2410_UFCON_RXTRIG12    (2<<4)

#define S3C2410_UFCON_RESETBOTH   (3<<1)

#define S3C2410_UFCON_DEFAULT     (S3C2410_UFCON_FIFOMODE | S3C2410_UFCON_TXTRIG0 \
                                  | S3C2410_UFCON_RXTRIG8 )

#define S3C2410_UFSTAT_TXFULL     (1<<9)
#define S3C2410_UFSTAT_RXFULL     (1<<8)
#define S3C2410_UFSTAT_TXMASK     (15<<4)
#define S3C2410_UFSTAT_TXSHIFT    (4)
#define S3C2410_UFSTAT_RXMASK     (15<<0)
#define S3C2410_UFSTAT_RXSHIFT    (0)

#define S3C2410_UTRSTAT_TXFE      (1<<1)
#define S3C2410_UTRSTAT_RXDR      (1<<0)

#endif /* ASMARM_ARCH_S3C2410_SERIAL_H */
