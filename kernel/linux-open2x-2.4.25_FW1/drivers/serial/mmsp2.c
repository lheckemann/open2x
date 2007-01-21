/*
 * device for MMSP2 Development Board
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/serial.h>
#include <linux/serial_reg.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/timer.h>

#include <asm/irq.h>
#include <asm/hardware.h>

#if defined(CONFIG_SERIAL_MMSP2_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/serial_core.h>

#define SERIAL_MMSP2_MAJOR	204
#define CALLOUT_MMSP2_MAJOR	205
#define MINOR_START		5

#define UART_NR			3
#define UART_ULCON(sport)	__REGW((sport)->port.mapbase + 0x00)
#define UART_UCON(sport)	__REGW((sport)->port.mapbase + 0x02)
#define UART_UFCON(sport)	__REGW((sport)->port.mapbase + 0x04)
#define UART_UMCON(sport)	__REGW((sport)->port.mapbase + 0x06)
#define UART_UTRSTAT(sport)	__REGW((sport)->port.mapbase + 0x08)
#define UART_UERSTAT(sport)	__REGW((sport)->port.mapbase + 0x0a)
#define UART_UFSTAT(sport)	__REGW((sport)->port.mapbase + 0x0c)
#define UART_UMSTAT(sport)	__REGW((sport)->port.mapbase + 0x0e)
#define UART_UTXH(sport)	__REGW((sport)->port.mapbase + 0x10)
#define UART_URXH(sport)	__REGW((sport)->port.mapbase + 0x12)
#define UART_UBRDIV(sport)	__REGW((sport)->port.mapbase + 0x14)

#define NUM_TX 	0
#define NUM_RX	1
#define NUM_ERR	2
#define TX_IRQ(sport)		((sport)->port.irq + NUM_TX)
#define RX_IRQ(sport)		((sport)->port.irq + NUM_RX)
#define ERR_IRQ(sport)		((sport)->port.irq + NUM_ERR)

#define IRQ_BIT(sport)	((sport)->port.unused[0])

static struct tty_driver normal, callout;
static struct tty_struct *mmsp2_table[UART_NR];
static struct termios *mmsp2_termios[UART_NR];
static struct termios *mmsp2_termios_locked[UART_NR];

#ifdef SUPPORT_SYSRQ
static struct console mmsp2_cons;
#endif

#define MCTRL_TIMEOUT	(250*HZ/1000)

struct mmsp2_port {
	struct uart_port port;
	struct timer_list timer;
	unsigned int old_status;
};

static void
mmsp2uart_mctrl_check(struct mmsp2_port *sport)
{
	unsigned int status, changed;

	status = sport->port.ops->get_mctrl(&sport->port);
	changed = status ^ sport->old_status;

	if (changed == 0)
		return;

	sport->old_status = status;

	if (changed & TIOCM_RI)
		sport->port.icount.rng++;
	if (changed & TIOCM_DSR)
		sport->port.icount.dsr++;
	if (changed & TIOCM_CAR)
		uart_handle_dcd_change(&sport->port, status & TIOCM_CAR);
	if (changed & TIOCM_CTS)
		uart_handle_cts_change(&sport->port, status & TIOCM_CTS);

	wake_up_interruptible(&sport->port.info->delta_msr_wait);
}

static void
mmsp2uart_timeout(unsigned long data)
{
	struct mmsp2_port *sport = (struct mmsp2_port *) data;
	unsigned long flags;

	if (sport->port.info) {
		spin_lock_irqsave(&sport->port.lock, flags);
		mmsp2uart_mctrl_check(sport);
		spin_unlock_irqrestore(&sport->port.lock, flags);

		mod_timer(&sport->timer, jiffies + MCTRL_TIMEOUT);
	}
}

static int eirq = 0, dcnt = 0, ecnt = 0;

static void
mmsp2uart_stop_tx(struct uart_port *port, unsigned int tty_stop)
{
	struct mmsp2_port *sport = (struct mmsp2_port *) port;
        #if 1 
		if (test_and_clear_bit(NUM_TX, &IRQ_BIT(sport)))
        {
			disable_irq(TX_IRQ(sport));
        }
        #else
        if( eirq == 1 )
        {
            disable_irq(TX_IRQ(sport));
            eirq = 0;
        }
        #endif
}

static void
mmsp2uart_start_tx(struct uart_port *port, unsigned int tty_start)
{
	struct mmsp2_port *sport = (struct mmsp2_port *) port;

		#if 1 
		if (!test_and_set_bit(NUM_TX, &IRQ_BIT(sport)))
        {
        	ecnt++;
			enable_irq(TX_IRQ(sport));
        }
       	#else
        if( eirq == 0 )
        {
            enable_irq(TX_IRQ(sport));
            eirq = 1;
        }
        #endif
}

static void
mmsp2uart_stop_rx(struct uart_port *port)
{
	struct mmsp2_port *sport = (struct mmsp2_port *) port;
	disable_irq(RX_IRQ(sport));
}

/*
 * Set the modem control timer to fire immediately
 */
static void
mmsp2uart_enable_ms(struct uart_port *port)
{
	struct mmsp2_port *sport = (struct mmsp2_port *) port;
	mod_timer(&sport->timer, jiffies);
}

static unsigned int
mmsp2uart_tx_empty(struct uart_port *port)
{
	struct mmsp2_port *sport = (struct mmsp2_port *) port;
	return (UART_UTRSTAT(sport) & UTRSTAT_TX_EMP ? TIOCSER_TEMT:0);
}

static unsigned int
mmsp2uart_get_mctrl(struct uart_port *port)
{
	struct mmsp2_port *sport = (struct mmsp2_port *) port;
	unsigned int result = 0;
	unsigned short status;

	if(port->line==1) {
		status = UART_UMSTAT(sport);
		if(status & (UART_MSR_RI<<4))
			result |= TIOCM_RNG;
		if(status & (UART_MSR_DCD<<4))
			result |= TIOCM_CAR;
		if(status & (UART_MSR_DSR<<4))
			result |= TIOCM_DSR;
		if(status & (UART_MSR_CTS<<4))
			result |= TIOCM_CTS;
		return result;
	}
	else
	return (TIOCM_CTS | TIOCM_DSR | TIOCM_CAR);
}

static void
mmsp2uart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	struct mmsp2_port *sport = (struct mmsp2_port *) port;
	unsigned short ctrl = 0x00;

	if(port->line==1) {
		if(mctrl & TIOCM_RTS)
			ctrl |= 0x01;
		else
			ctrl &= ~0x01;

		if(mctrl & TIOCM_DTR)
			ctrl |= 0x02;
		else
			ctrl &= ~0x02;

		UART_UMCON(sport) = ctrl;
	}
}

static void
mmsp2uart_break_ctl(struct uart_port *port, int break_state)
{
	struct mmsp2_port *sport = (struct mmsp2_port *) port;
	unsigned long flags;
	unsigned int ucon;

	spin_lock_irqsave(&sport->port.lock, flags);
	ucon = UART_UCON(sport);

	if (break_state == -1)
		ucon |= UCON_BRK_SIG;
	else
		ucon &= ~UCON_BRK_SIG;

	UART_UCON(sport) = ucon;
	spin_unlock_irqrestore(&sport->port.lock, flags);
}

static void
mmsp2uart_rx_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned int status, ch, flg, max_count = 256, ignored = 0;
	unsigned char err;
	struct mmsp2_port *sport = dev_id;
	struct tty_struct *tty = sport->port.info->tty;

	status = UART_UTRSTAT(sport);
	while ((status & UTRSTAT_RX_RDY) && max_count--) {
		if (tty->flip.count >= TTY_FLIPBUF_SIZE) {
			tty->flip.tqueue.routine((void *) tty);
			if (tty->flip.count >= TTY_FLIPBUF_SIZE) {
				printk(KERN_WARNING "TTY_DONT_FLIP set\n");
				return;
			}
		}

		err = UART_UERSTAT(sport) & UART_ERR_MASK;
		ch = UART_URXH(sport);

#ifdef SUPPORT_SYSRQ
		if (err & UERSTAT_BRK) {
			sport->port.icount.brk++;
			if (sport->port.line == mmsp2_cons.index
			    && !sport->port.sysrq) {
				sport->port.sysrq = jiffies + HZ * 5;
			}
			goto out;
		}
#endif

		sport->port.icount.rx++;

		flg = TTY_NORMAL;

		if (err & (UERSTAT_FRAME | UERSTAT_PARITY | UERSTAT_OVERRUN))
			goto handle_error;

#ifdef SUPPORT_SYSRQ
		if (uart_handle_sysrq_char(&sport->port, ch, regs))
			goto ignore_char;
#endif

 error_return:
		*tty->flip.flag_buf_ptr++ = flg;
		*tty->flip.char_buf_ptr++ = ch;
		tty->flip.count++;

 ignore_char:
		status = UART_UTRSTAT(sport);
	}

 out:
	tty_flip_buffer_push(tty);
	return;

 handle_error:
	if (err & UERSTAT_FRAME)
		sport->port.icount.frame++;
	if (err & UERSTAT_PARITY)
		sport->port.icount.parity++;
	if (err & UERSTAT_OVERRUN)
		sport->port.icount.overrun++;

	if (status & sport->port.ignore_status_mask) {
		if (++ignored > 100)
			goto out;
		goto ignore_char;
	}

	err &= sport->port.read_status_mask;

	if (err & UERSTAT_PARITY)
		flg = TTY_PARITY;
	else if (err & UERSTAT_FRAME)
		flg = TTY_FRAME;

	if (err & UERSTAT_OVERRUN) {
		*tty->flip.flag_buf_ptr++ = flg;
		*tty->flip.char_buf_ptr++ = ch;
		tty->flip.count++;
		if (tty->flip.count >= TTY_FLIPBUF_SIZE)
			goto ignore_char;
		ch = 0;
		flg = TTY_OVERRUN;
	}
#ifdef SUPPORT_SYSRQ
	sport->port.sysrq = 0;
#endif
	goto error_return;
}

static void
mmsp2uart_tx_interrupt(int irq, void *dev_id, struct pt_regs *reg)
{
	struct mmsp2_port *sport = dev_id;
	struct circ_buf *xmit = &sport->port.info->xmit;
	int count;

	if (sport->port.x_char) {
		UART_UTXH(sport) = sport->port.x_char;
		sport->port.icount.tx++;
		sport->port.x_char = 0;
		return;
	}

	/*
	 * Check the modem control lines before transmitting anything
	 */
	mmsp2uart_mctrl_check(sport);

	if (uart_circ_empty(xmit) || uart_tx_stopped(&sport->port)) {
		mmsp2uart_stop_tx(&sport->port, 0);
		return;
	}

	count = sport->port.fifosize >> 1;
	do {
		UART_UTXH(sport) = xmit->buf[xmit->tail];
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		sport->port.icount.tx++;
		if (uart_circ_empty(xmit))
			break;
	} while (--count > 0);

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&sport->port);

	if (uart_circ_empty(xmit))
		mmsp2uart_stop_tx(&sport->port, 0);
}

static int
mmsp2uart_startup(struct uart_port *port)
{
	int ret, flags;
	u_int ucon;
	struct mmsp2_port *sport = (struct mmsp2_port *) port;

	ret = request_irq(RX_IRQ(sport), mmsp2uart_rx_interrupt, 0 /*SA_INTERRUPT*/,
			  "serial_mmsp2_rx", sport);
	if (ret) {
		printk("%s: Rx: request_irq(%d) failed. ret = %d\n",
				__FILE__, RX_IRQ(sport), ret);
		goto rx_failed;
	}
	ret = request_irq(TX_IRQ(sport), mmsp2uart_tx_interrupt, 0 /*SA_INTERRUPT*/,
			  "serial_mmsp2_tx", sport);
	if (ret) {
		printk("%s: Tx: request_irq(%d) failed. ret = %d\n",
				__FILE__, TX_IRQ(sport), ret);
		goto tx_failed;
	}

    #if 1 // fifo interrupt level to zero
		ucon = (UCON_TX_INT_LVL | UCON_RX_INT_LVL |
		UCON_TX_INT | UCON_RX_INT | UCON_RX_TIMEOUT);
    #else
		ucon = (UCON_TX_INT | UCON_RX_INT | UCON_RX_TIMEOUT);
    #endif

	save_flags(flags);
	cli();

	UART_UCON(sport) = ucon;

	sti();
	restore_flags(flags);

	/* reset RX fifo */
	UART_UFCON(sport) |= UFCON_RX_CLR;

	return 0;

 tx_failed:
	free_irq(RX_IRQ(sport), sport);

 rx_failed:
	return ret;
}

static void
mmsp2uart_shutdown(struct uart_port *port)
{
	struct mmsp2_port *sport = (struct mmsp2_port *) port;
	del_timer_sync(&sport->timer);
	free_irq(RX_IRQ(sport), sport);
	free_irq(TX_IRQ(sport), sport);
}

static void
mmsp2uart_change_speed(struct uart_port *port, u_int cflag, u_int iflag,
			 u_int quot)
{
	struct mmsp2_port *sport = (struct mmsp2_port *) port;
	u_int ulcon, ufcon;
	int flags;

	ufcon = UART_UFCON(sport);

	switch (cflag & CSIZE) {
	case CS5:
		ulcon = ULCON_WL5;
		break;
	case CS6:
		ulcon = ULCON_WL6;
		break;
	case CS7:
		ulcon = ULCON_WL7;
		break;
	default:
		ulcon = ULCON_WL8;
		break;
	}

	if (cflag & CSTOPB)
		ulcon |= ULCON_STOP;
	if (cflag & PARENB) {
		if (cflag & PARODD)
			ulcon |= ULCON_PAR_ODD;
		else
			ulcon |= ULCON_PAR_EVEN;
	} else
		ulcon |= ULCON_PAR_NONE;

	if (sport->port.fifosize > 1)
		ufcon |= UFCON_FIFO_EN;

	sport->port.read_status_mask = UERSTAT_OVERRUN;
	if (iflag & INPCK)
		sport->port.read_status_mask |= UERSTAT_PARITY | UERSTAT_FRAME;

	sport->port.ignore_status_mask = 0;
	if (iflag & IGNPAR)
		sport->port.ignore_status_mask |=
		    UERSTAT_FRAME | UERSTAT_PARITY;
	if (iflag & IGNBRK) {
		if (iflag & IGNPAR)
			sport->port.ignore_status_mask |= UERSTAT_OVERRUN;
	}

	quot -= 1;

	spin_lock_irqsave(&sport->port.lock, flags);

	UART_UFCON(sport) = ufcon;
#if defined(CONFIG_MACH_SMDK2410TK) || defined(CONFIG_MACH_SMDK2410AJ)
	UART_ULCON(sport) =
	    (UART_ULCON(sport) & ~(ULCON_PAR | ULCON_STOP | ULCON_WL)) | ulcon;
#else
	UART_ULCON(sport) = ulcon;
#endif
	UART_UBRDIV(sport) = quot;

	spin_unlock_irqrestore(&sport->port.lock, flags);
}

static const char *
mmsp2uart_type(struct uart_port *port)
{
	struct mmsp2_port *sport = (struct mmsp2_port *) port;
	return sport->port.type == PORT_MMSP2 ? "MMSP2" : NULL;
}

static void
mmsp2uart_config_port(struct uart_port *port, int flags)
{
	struct mmsp2_port *sport = (struct mmsp2_port *) port;

	if (flags & UART_CONFIG_TYPE)
		sport->port.type = PORT_MMSP2;
}

static void
mmsp2uart_release_port(struct uart_port *port)
{
}

static int
mmsp2uart_request_port(struct uart_port *port)
{
	return 0;
}

static struct uart_ops mmsp2_pops = {
	.tx_empty	= mmsp2uart_tx_empty,
	.set_mctrl	= mmsp2uart_set_mctrl,
	.get_mctrl	= mmsp2uart_get_mctrl,
	.stop_tx	= mmsp2uart_stop_tx,
	.start_tx	= mmsp2uart_start_tx,
	.stop_rx	= mmsp2uart_stop_rx,
	.enable_ms	= mmsp2uart_enable_ms,
	.break_ctl	= mmsp2uart_break_ctl,
	.startup	= mmsp2uart_startup,
	.shutdown	= mmsp2uart_shutdown,
	.change_speed	= mmsp2uart_change_speed,
	.type		= mmsp2uart_type,
	.config_port	= mmsp2uart_config_port,
	.release_port	= mmsp2uart_release_port,
	.request_port	= mmsp2uart_request_port,
};

static struct mmsp2_port mmsp2_ports[UART_NR] = {
	{
	.port = {
		.membase	= (void*)io_p2v(0xc0001200),
		.mapbase	= 0xc0001200,
		.iotype		= SERIAL_IO_MEM,
		.irq		= IRQ_TXD0,
		.line		= 0,
		.uartclk	= 14745600, //130252800,
		.fifosize	= 16,
		.ops		= &mmsp2_pops,
		.type		= PORT_MMSP2,
		.flags		= ASYNC_BOOT_AUTOCONF,
		},
	}, 
	{
	.port = {
		.membase	= (void*)io_p2v(0xc0001220),
		.mapbase	= 0xc0001220,
		.iotype		= SERIAL_IO_MEM,
		.irq		= IRQ_TXD1,
		.line		= 1,
		.uartclk	= 14745600, //130252800,
		.fifosize	= 16,
		.ops		= &mmsp2_pops,
		.type		= PORT_MMSP2,
		.flags		= ASYNC_BOOT_AUTOCONF,
		},
	}, 
	{
	.port = {
		.membase	= (void*)io_p2v(0xc0001240),
		.mapbase	= 0xc0001240,
		.iotype		= SERIAL_IO_MEM,
		.irq		= IRQ_TXD2,
		.line		= 2,
		.uartclk	= 14745600, //130252800,
		.fifosize	= 16,
		.ops		= &mmsp2_pops,
		.type		= PORT_MMSP2,
		.flags		= ASYNC_BOOT_AUTOCONF,
		},
	}
};

void __init
mmsp2_register_uart(int idx, int port)
{
	struct uart_port *p_port;
	
	if (idx >= UART_NR) {
		printk(KERN_ERR "%s: bad index number %d\n", __FUNCTION__, idx);
		return;
	}
	p_port = &mmsp2_ports[idx].port;

	p_port->mapbase = 0xc0001200 + 0x20 * port;
	p_port->membase = (void*)io_p2v(p_port->mapbase);
	p_port->irq = IRQ_TXD0 + 4 * port; 
	p_port->uartclk = mmsp2_get_aclk()/10;
	p_port->line = idx;
	p_port->unused[0] = 0;
}

#ifdef CONFIG_SERIAL_MMSP2_CONSOLE

static void
mmsp2_console_write(struct console *co, const char *s, u_int count)
{
	int i;
	struct mmsp2_port *sport = mmsp2_ports + co->index;

	for (i = 0; i < count; i++) {
		while (!(UART_UTRSTAT(sport) & UTRSTAT_TX_EMP)) ;
		UART_UTXH(sport) = s[i];
		if (s[i] == '\n') {
			while (!(UART_UTRSTAT(sport) & UTRSTAT_TX_EMP)) ;
			UART_UTXH(sport) = '\r';
		}
	}
}

static kdev_t
mmsp2_console_device(struct console *co)
{
	return MKDEV(SERIAL_MMSP2_MAJOR, MINOR_START + co->index);
}

static int __init
mmsp2_console_setup(struct console *co, char *options)
{
	struct mmsp2_port *sport;
	int baud = 115200;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	if (co->index == -1 || co->index >= UART_NR)
		co->index = 0;
	sport = &mmsp2_ports[co->index];

	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);

	return uart_set_options(&sport->port, co, baud, parity, bits, flow);
}

static struct console mmsp2_cons = {
	.name	= "ttyS",
	.write	= mmsp2_console_write,
	.device	= mmsp2_console_device,
	.setup	= mmsp2_console_setup,
	.flags	= CON_PRINTBUFFER,
	.index	= -1,
};

void __init
mmsp2_console_init(void)
{
	register_console(&mmsp2_cons);
}

#define MMSP2_CONSOLE		&mmsp2_cons
#else				/* CONFIG_SERIAL_MMSP2_CONSOLE */
#define MMSP2_CONSOLE		NULL
#endif				/* CONFIG_SERIAL_MMSP2_CONSOLE */

static struct uart_driver mmsp2_reg = {
	.owner		= THIS_MODULE,
	.normal_major	= SERIAL_MMSP2_MAJOR,
#ifdef CONFIG_DEVFS_FS
	.normal_name	= "ttyS%d",
	.callout_name	= "cua%d",
#else
	.normal_name	= "ttyS",
	.callout_name	= "cua",
#endif
	.normal_driver	= &normal,
	.callout_major	= CALLOUT_MMSP2_MAJOR,
	.callout_driver	= &callout,
	.table		= mmsp2_table,
	.termios	= mmsp2_termios,
	.termios_locked	= mmsp2_termios_locked,
	.minor		= MINOR_START,
	.nr		= UART_NR,
	.cons		= MMSP2_CONSOLE,
};

static int __init
mmsp2uart_init(void)
{
	int i, ret;

	ret = uart_register_driver(&mmsp2_reg);
	if (ret)
		return ret;

	for (i = 0; i < UART_NR; i++) {
		init_timer(&mmsp2_ports[i].timer);
		mmsp2_ports[i].timer.function = mmsp2uart_timeout;
		mmsp2_ports[i].timer.data = (unsigned long) &mmsp2_ports[i];
		uart_add_one_port(&mmsp2_reg, &mmsp2_ports[i].port);
	}

	return 0;
}

static void __exit
mmsp2uart_exit(void)
{
	int i;

	for (i = 0; i < UART_NR; i++) {
		del_timer_sync(&mmsp2_ports[i].timer);
		uart_remove_one_port(&mmsp2_reg, &mmsp2_ports[i].port);
	}

	uart_unregister_driver(&mmsp2_reg);
}

module_init(mmsp2uart_init);
module_exit(mmsp2uart_exit);

EXPORT_NO_SYMBOLS;

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
MODULE_DESCRIPTION("MMSP2 generic serial port driver");
