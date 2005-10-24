/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * (C) Copyright 2004
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
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
 *
 */

#include <common.h>
/*
#if defined(CONFIG_S3C2400) || defined(CONFIG_TRAB)
# include <s3c2400.h>
#elif defined(CONFIG_S3C2410)
# include <s3c2410.h>
#elif defined(CONFIG_MMSP20)
*/
# include <mmsp20.h>
/*
#endif
*/
/*
#ifdef CONFIG_SERIAL1
# if defined(CONFIG_MMSP20)
#  error "MMSP20 support CONFIG_SERIAL2"
# define UART_NR	S3C24X0_UART0
# endif
*/
/*
#elif CONFIG_SERIAL2
# if defined(CONFIG_TRAB)
#  error "TRAB supports only CONFIG_SERIAL1"
# elif defined(CONFIG_MMSP20)
*/
#  define UART_NR 	MMSP20_UART0
#  define UART_CLK	ACLK
/*
# else
#  define UART_NR	S3C24X0_UART1
# endif
*/
/*
#elif CONFIG_SERIAL3
# if defined(CONFIG_TRAB) || defined(CONFIG_MMSP20)
#  error "TRAB supports only CONFIG_SERIAL1\nMMSP20 supports only CONFIG_SERIAL2"
#define UART_NR	S3C24X0_UART2

#else
#error "Bad: you didn't configure serial ..."
#endif
*/
#if defined(CONFIG_MMSP20)

void serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	MMSP20_UART * const uart = MMSP20_GetBase_UART();
	int i;
	unsigned int reg = 0;
	
//	gd->baudrate=CONFIG_BAUDRATE;
	gd->baudrate= 115200;

/*	reg = get_UART1_CLK() / (16 * gd->baudrate) - 1; */
/*	reg = 0x2f; 19200*/
	reg = ((14745600/(gd->baudrate*16))-1);

	uart->MMSP20_UART_NR[UART_NR].LCON = 
		( uart->MMSP20_UART_NR[UART_NR].LCON & ~0x007C) | 0x0003;
	uart->MMSP20_UART_NR[UART_NR].UCON =
		( uart->MMSP20_UART_NR[UART_NR].UCON & ~0x03FF) | 0x00C5;
	uart->MMSP20_UART_NR[UART_NR].FCON =
		( uart->MMSP20_UART_NR[UART_NR].FCON & ~0x00FF) | 0x0001;
	uart->MMSP20_UART_NR[UART_NR].MCON = 
		( uart->MMSP20_UART_NR[UART_NR].MCON & ~0x00FF);
	uart->MMSP20_UART_NR[UART_NR].BRD = reg;
	
	for(i = 0 ; i < 100 ; i++);
}


#else
void serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	S3C24X0_UART * const uart = S3C24X0_GetBase_UART(UART_NR);
	int i;
	unsigned int reg = 0;

	/* value is calculated so : (int)(PCLK/16./baudrate) -1 */
	reg = get_PCLK() / (16 * gd->baudrate) - 1;

	/* FIFO enable, Tx/Rx FIFO clear */
	uart->UFCON = 0x07;
	uart->UMCON = 0x0;
	/* Normal,No parity,1 stop,8 bit */
	uart->ULCON = 0x3;
	/*
	 * tx=level,rx=edge,disable timeout int.,enable rx error int.,
	 * normal,interrupt or polling
	 */
	uart->UCON = 0x245;
	uart->UBRDIV = reg;

#ifdef CONFIG_HWFLOW
	uart->UMCON = 0x1; /* RTS up */
#endif
	for (i = 0; i < 100; i++);
}
#endif
/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
int serial_init (void)
{
	serial_setbrg ();
	serial_putc ('\n');
	return (0);
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int serial_getc (void)
{
#if defined(CONFIG_MMSP20)
	MMSP20_UART * const uart = MMSP20_GetBase_UART();
	
	while (!(uart->MMSP20_UART_NR[UART_NR].TRSTATUS & 0x1));
	
	return uart->MMSP20_UART_NR[UART_NR].RHB & 0xFF;	
#else
	S3C24X0_UART * const uart = S3C24X0_GetBase_UART(UART_NR);

	/* wait for character to arrive */
	while (!(uart->UTRSTAT & 0x1));

	return uart->URXH & 0xff;
#endif
}

#ifdef CONFIG_HWFLOW
static int hwflow = 0; /* turned off by default */
int hwflow_onoff(int on)
{
	switch(on) {
	case 0:
	default:
		break; /* return current */
	case 1:
		hwflow = 1; /* turn on */
		break;
	case -1:
		hwflow = 0; /* turn off */
		break;
	}
	return hwflow;
}
#endif

#ifdef CONFIG_MODEM_SUPPORT
static int be_quiet = 0;
void disable_putc(void)
{
	be_quiet = 1;
}

void enable_putc(void)
{
	be_quiet = 0;
}
#endif


/*
 * Output a single byte to the serial port.
 */

#if defined (CONFIG_MMSP20)
void serial_putc (const char c)
{
	MMSP20_UART * const uart = MMSP20_GetBase_UART();

	while(!(uart->MMSP20_UART_NR[UART_NR].TRSTATUS & 0x02));

	uart->MMSP20_UART_NR[UART_NR].THB = c;

	if (c == '\n')
		serial_putc('\r');
}
#else
void serial_putc (const char c)
{
	S3C24X0_UART * const uart = S3C24X0_GetBase_UART(UART_NR);
#ifdef CONFIG_MODEM_SUPPORT
	if (be_quiet)
		return;
#endif

	/* wait for room in the tx FIFO */
	while (!(uart->UTRSTAT & 0x2));

#ifdef CONFIG_HWFLOW
	/* Wait for CTS up */
	while(hwflow && !(uart->UMSTAT & 0x1))
		;
#endif

	uart->UTXH = c;

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc ('\r');
}
#endif
/*
 * Test whether a character is in the RX buffer
 */
int serial_tstc (void)
{
#if defined (CONFIG_MMSP20)
	MMSP20_UART * const uart = MMSP20_GetBase_UART();

	return uart->MMSP20_UART_NR[UART_NR].TRSTATUS & 0x1;
#else
	S3C24X0_UART * const uart = S3C24X0_GetBase_UART(UART_NR);

	return uart->UTRSTAT & 0x1;
#endif
}

void
serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}
