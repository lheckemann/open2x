/*
 * LIRC SIR driver, (C) 2000 Milan Pikula <www@fornax.sk>
 *
 * lirc_sir - Device driver for use with SIR (serial infra red)
 * mode of IrDA on many notebooks.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * 2000/09/16 Frank Przybylski <mail@frankprzybylski.de> :
 *  added timeout and relaxed pulse detection, removed gap bug
 *
 * 2000/12/15 Christoph Bartelmus <lirc@bartelmus.de> : 
 *   added support for Tekram Irmate 210 (sending does not work yet,
 *   kind of disappointing that nobody was able to implement that
 *   before),
 *   major clean-up
 *
 * 2001/02/27 Christoph Bartelmus <lirc@bartelmus.de> : 
 *   added support for StrongARM SA1100 embedded microprocessor
 *   parts cut'n'pasted from sa1100_ir.c (C) 2000 Russell King
 */


#include <linux/version.h>
#if LINUX_VERSION_CODE >= 0x020100
#define KERNEL_2_1
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)
#define KERNEL_2_3
#endif
#else
#define KERNEL_2_0
#endif

#include <linux/module.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
 
#include <linux/config.h>

#if !defined(CONFIG_ARCH_SA1100) && !defined(CONFIG_SERIAL_MODULE)
#warning "******************************************"
#warning " Your serial port driver is compiled into "
#warning " the kernel. You will have to release the "
#warning " port you want to use for LIRC with:      "
#warning "    setserial /dev/ttySx uart none        "
#warning "******************************************"
#endif

#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/serial_reg.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/pm.h>
#ifdef KERNEL_2_1
#include <linux/poll.h>
#endif
#include <asm/system.h>
#include <asm/segment.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/fcntl.h>
#if defined(CONFIG_ARCH_SA1100) || defined(CONFIG_ARCH_PXA)
#include <asm/hardware.h>
#ifdef CONFIG_SA1100_COLLIE
#include <asm/arch/tc35143.h>
#include <asm/ucb1200.h>
#endif
#endif

#include <linux/timer.h>

#include "lirc.h"

/* SECTION: Definitions */

/**************************** Tekram dongle ***************************/
#ifdef LIRC_SIR_TEKRAM
/* stolen from kernel source */
/* definitions for Tekram dongle */
#define TEKRAM_115200 0x00
#define TEKRAM_57600  0x01
#define TEKRAM_38400  0x02
#define TEKRAM_19200  0x03
#define TEKRAM_9600   0x04
#define TEKRAM_2400   0x08

#define TEKRAM_PW 0x10 /* Pulse select bit */

/* 10bit * 1s/115200bit in milli seconds = 87ms*/
#define TIME_CONST (10000000ul/115200ul)

#endif

#ifdef LIRC_SIR_ACTISYS_ACT200L
static void init_act200(void);
#endif

#ifdef CONFIG_PM
static struct pm_dev *lirc_pmdev;
#endif

/******************************* SA1100 ********************************/
#ifdef CONFIG_ARCH_SA1100
struct sa1100_ser2_registers
{
	/* HSSP control register */
	unsigned char hscr0;
	/* UART registers */
	unsigned char utcr0;
	unsigned char utcr1;
	unsigned char utcr2;
	unsigned char utcr3;
	unsigned char utcr4;
	unsigned char utdr;
	unsigned char utsr0;
	unsigned char utsr1;
} sr;

static int irq=IRQ_Ser2ICP;

#define CONFIG_ARCH_SA1100_TRANSMITTER_LATENCY 0

/* pulse/space ratio of 50/50 */
unsigned long pulse_width = (13-CONFIG_ARCH_SA1100_TRANSMITTER_LATENCY);
/* 1000000/freq-pulse_width */
unsigned long space_width = (13-CONFIG_ARCH_SA1100_TRANSMITTER_LATENCY);
unsigned int freq = 38000;      /* modulation frequency */
unsigned int duty_cycle = 50;   /* duty cycle of 50% */

#endif /* CONFIG_ARCH_SA1100 */

/******************************* PXA250 ********************************/
#ifdef CONFIG_ARCH_PXA
struct pxa250_ser2_registers
{
	/* ST-UART */
	unsigned char strbr;
	unsigned char stthr;
	unsigned char stier;
	unsigned char stiir;
	unsigned char stfcr;
	unsigned char stlcr;
	unsigned char stmcr;
	unsigned char stlsr;
	unsigned char stmsr;
	unsigned char stspr;
	unsigned char stisr;
	unsigned char stdll;
	unsigned char stdlh;

	/* FICP */
	unsigned char iccr0;
	
	/* CKEN */
	unsigned long cken;
} sr;

static int irq=IRQ_STUART;

#define CONFIG_ARCH_PXA_TRANSMITTER_LATENCY 0

/* pulse/space ratio of 50/50 */
unsigned long pulse_width = (13-CONFIG_ARCH_PXA_TRANSMITTER_LATENCY);
/* 1000000/freq-pulse_width */
unsigned long space_width = (13-CONFIG_ARCH_PXA_TRANSMITTER_LATENCY);
unsigned int freq = 38000;      /* modulation frequency */
unsigned int duty_cycle = 50;   /* duty cycle of 50% */
#endif /* CONFIG_ARCH_PXA */

#define RBUF_LEN 1024
#define WBUF_LEN 1024

#define LIRC_DRIVER_NAME "lirc_sir"

#ifndef LIRC_SIR_TEKRAM
#define PULSE '['

/* 9bit * 1s/115200bit in milli seconds = 78.125ms*/
#define TIME_CONST (9000000ul/115200ul)
#endif


/* timeout for sequences in jiffies (=5/100s) */
/* must be longer than TIME_CONST */
#define SIR_TIMEOUT	(HZ*5/100)

static int major = 61; // = LIRC_MAJOR;

#if !defined(CONFIG_ARCH_SA1100) && !defined(CONFIG_ARCH_PXA)
static int io = LIRC_PORT;
static int irq = LIRC_IRQ;
#endif

static spinlock_t timer_lock = SPIN_LOCK_UNLOCKED;
static struct timer_list timerlist;
/* time of last signal change detected */
static struct timeval last_tv = {0, 0};
/* time of last UART data ready interrupt */
static struct timeval last_intr_tv = {0, 0};
static int last_value = 0;

#ifdef KERNEL_2_3
static DECLARE_WAIT_QUEUE_HEAD(lirc_read_queue);
#else
static struct wait_queue * lirc_read_queue = NULL;
#endif

static spinlock_t hardware_lock = SPIN_LOCK_UNLOCKED;
static spinlock_t dev_lock = SPIN_LOCK_UNLOCKED;

static lirc_t rx_buf[RBUF_LEN]; unsigned int rx_tail = 0, rx_head = 0;
#ifndef LIRC_SIR_TEKRAM
static lirc_t tx_buf[WBUF_LEN];
#endif

/* SECTION: Prototypes */

/* Communication with user-space */
static int lirc_open(struct inode * inode, struct file * file);
#ifdef KERNEL_2_1
static int lirc_close(struct inode * inode, struct file *file);
static unsigned int lirc_poll(struct file * file, poll_table * wait);
#else
static void lirc_close(struct inode * inode, struct file *file);
static int lirc_select(struct inode * inode, struct file * file,
		int type, select_table * wait);
#endif
static ssize_t lirc_read(struct file * file, char * buf, size_t count,
		loff_t * ppos);
static ssize_t lirc_write(struct file * file, const char * buf, size_t n, loff_t * pos);
static int lirc_ioctl(struct inode *node,struct file *filep,unsigned int cmd,
		unsigned long arg);
static void add_read_queue(int flag, unsigned long val);
#ifdef MODULE
static int init_chrdev(void);
static void drop_chrdev(void);
#endif
	/* Hardware */
static void sir_interrupt(int irq, void * dev_id, struct pt_regs * regs);
#ifndef LIRC_SIR_TEKRAM
static void send_space(unsigned long len);
static void send_pulse(unsigned long len);
#endif
static int init_hardware(void);
static void drop_hardware(void);
	/* Initialisation */
static int init_port(void);
static void drop_port(void);
int init_module(void);
void cleanup_module(void);

#ifdef CONFIG_ARCH_SA1100
void inline on(void)
{
	PPSR|=PPC_TXD2;
}
  
void inline off(void)
{
	PPSR&=~PPC_TXD2;
}
#elif defined(CONFIG_ARCH_PXA)
void inline on(void)
{
#ifdef CONFIG_ARCH_I519
	GPSR(GPIO80_IRDA_EN) = GPIO_bit(GPIO80_IRDA_EN);
#else
	GPSR(GPIO47_STTXD) = GPIO_bit(GPIO47_STTXD);
#endif
}
  
void inline off(void)
{
#ifdef CONFIG_ARCH_I519
	GPCR(GPIO80_IRDA_EN) = GPIO_bit(GPIO80_IRDA_EN);
#else
	GPCR(GPIO47_STTXD) = GPIO_bit(GPIO47_STTXD);
#endif
}
#else
static inline unsigned int sinp(int offset)
{
	return inb(io + offset);
}

static inline void soutp(int offset, int value)
{
	outb(value, io + offset);
}
#endif

#ifndef MAX_UDELAY_MS
#define MAX_UDELAY_US 5000
#else
#define MAX_UDELAY_US (MAX_UDELAY_MS*1000)
#endif

static inline void safe_udelay(unsigned long usecs)
{
	while(usecs>MAX_UDELAY_US)
	{
		udelay(MAX_UDELAY_US);
		usecs-=MAX_UDELAY_US;
	}
	udelay(usecs);
}

/* SECTION: Communication with user-space */

static int lirc_open(struct inode * inode, struct file * file)
{
	spin_lock(&dev_lock);
	if (MOD_IN_USE) {
		spin_unlock(&dev_lock);
		return -EBUSY;
	}
	MOD_INC_USE_COUNT;
	spin_unlock(&dev_lock);
	return 0;
}

#ifdef KERNEL_2_1
static int lirc_close(struct inode * inode, struct file *file)
#else
static void lirc_close(struct inode * inode, struct file *file)
#endif
{
	MOD_DEC_USE_COUNT;
#ifdef KERNEL_2_1
	return 0;
#endif
}

#ifdef KERNEL_2_1
static unsigned int lirc_poll(struct file * file, poll_table * wait)
{
	poll_wait(file, &lirc_read_queue, wait);
	if (rx_head != rx_tail)
		return POLLIN | POLLRDNORM;
	return 0;
}
#else
static int lirc_select(struct inode * inode, struct file * file,
		int type, select_table * wait)
{
	if (type != SEL_IN)
		return 0;
	if (rx_head != rx_tail)
		return 1;
	select_wait(&lirc_read_queue, wait);
	return 0;
}
#endif

static ssize_t lirc_read(struct file * file, char * buf, size_t count,
		loff_t * ppos)
{
	int n=0;
	int retval = 0;
#ifdef KERNEL_2_3
	DECLARE_WAITQUEUE(wait,current);
#else
	struct wait_queue wait={current,NULL};
#endif
	
	if(n%sizeof(lirc_t)) return(-EINVAL);
	
	add_wait_queue(&lirc_read_queue,&wait);
	current->state=TASK_INTERRUPTIBLE;
	while(n<count)
	{
		if(rx_head!=rx_tail)
		{
			retval=verify_area(VERIFY_WRITE,
					   (void *) buf+n,sizeof(lirc_t));
			if (retval)
			{
				return retval;
			}
#ifdef KERNEL_2_1
			copy_to_user((void *) buf+n,(void *) (rx_buf+rx_head),
				     sizeof(lirc_t));
#else
			memcpy_tofs((void *) buf+n,(void *) (rx_buf+rx_head),
				    sizeof(lirc_t));
#endif
			rx_head=(rx_head+1)&(RBUF_LEN-1);
			n+=sizeof(lirc_t);
		}
		else
		{
			if(file->f_flags & O_NONBLOCK)
			{
				retval=-EAGAIN;
				break;
			}
#                       ifdef KERNEL_2_1
			if(signal_pending(current))
			{
				retval=-ERESTARTSYS;
				break;
			}
#                       else
			if(current->signal & ~current->blocked)
			{
				retval=-EINTR;
				break;
			}
#                       endif
			schedule();
			current->state=TASK_INTERRUPTIBLE;
		}
	}
	remove_wait_queue(&lirc_read_queue,&wait);
	current->state=TASK_RUNNING;
	return (n ? n : retval);
}
static ssize_t lirc_write(struct file * file, const char * buf, size_t n, loff_t * pos)
{
	unsigned long flags;
#ifdef LIRC_SIR_TEKRAM
	return(-EBADF);
#else
	int i;
	int retval;

        if(n%sizeof(lirc_t) || (n/sizeof(lirc_t)) > WBUF_LEN)
		return(-EINVAL);
	retval = verify_area(VERIFY_READ, buf, n);
	if (retval)
		return retval;
	copy_from_user(tx_buf, buf, n);
	i = 0;
	n/=sizeof(lirc_t);
#ifdef CONFIG_ARCH_SA1100
	/* disable receiver */
	Ser2UTCR3=0;
#elif defined(CONFIG_ARCH_PXA)
	/* STUART unit disable, Rx interrupt disable */
	STIER &= ~(IER_UUE | IER_RAVIE);
#endif
	save_flags(flags);cli();
	while (1) {
		if (i >= n)
			break;
		if (tx_buf[i])
			send_pulse(tx_buf[i]);
		i++;
		if (i >= n)
			break;
		if (tx_buf[i])
			send_space(tx_buf[i]);
		i++;
	}
	restore_flags(flags);
#ifdef CONFIG_ARCH_SA1100
	off();
	udelay(1000); /* wait 1ms for IR diode to recover */
	Ser2UTCR3=0;
	/* clear status register to prevent unwanted interrupts */
	Ser2UTSR0 &= (UTSR0_RID | UTSR0_RBB | UTSR0_REB);
	/* enable receiver */
	Ser2UTCR3=UTCR3_RXE|UTCR3_RIE;
#elif defined(CONFIG_ARCH_PXA)
	off();
	udelay(1000); /* wait 1ms for IR diode to recover */

	/* FIFO reset */
	STFCR = FCR_RESETTF | FCR_RESETRF | FCR_ITL_1 ;

	/* STUART unit enable, Rx interrupt enable */
	STIER = IER_UUE | IER_RAVIE;
#endif
	return n;
#endif
}

static int lirc_ioctl(struct inode *node,struct file *filep,unsigned int cmd,
		unsigned long arg)
{
	int retval = 0;
	unsigned long value = 0;
#if defined(CONFIG_ARCH_SA1100) || defined(CONFIG_ARCH_PXA)
	unsigned int ivalue;
#endif

#ifdef LIRC_SIR_TEKRAM
	if (cmd == LIRC_GET_FEATURES)
		value = LIRC_CAN_REC_MODE2;
	else if (cmd == LIRC_GET_SEND_MODE)
		value = 0;
	else if (cmd == LIRC_GET_REC_MODE)
		value = LIRC_MODE_MODE2;
#elif defined(CONFIG_ARCH_SA1100) || defined(CONFIG_ARCH_PXA)
	if (cmd == LIRC_GET_FEATURES)
		value = LIRC_CAN_SEND_PULSE |
			LIRC_CAN_SET_SEND_DUTY_CYCLE |
			LIRC_CAN_SET_SEND_CARRIER |
			LIRC_CAN_REC_MODE2;
	else if (cmd == LIRC_GET_SEND_MODE)
		value = LIRC_MODE_PULSE;
	else if (cmd == LIRC_GET_REC_MODE)
		value = LIRC_MODE_MODE2;
#else
	if (cmd == LIRC_GET_FEATURES)
		value = LIRC_CAN_SEND_PULSE | LIRC_CAN_REC_MODE2;
	else if (cmd == LIRC_GET_SEND_MODE)
		value = LIRC_MODE_PULSE;
	else if (cmd == LIRC_GET_REC_MODE)
		value = LIRC_MODE_MODE2;
#endif

	switch (cmd) {
	case LIRC_GET_FEATURES:
	case LIRC_GET_SEND_MODE:
	case LIRC_GET_REC_MODE:
#ifdef KERNEL_2_0
		retval = verify_area(VERIFY_WRITE, (unsigned long *) arg,
			sizeof(unsigned long));
		if (retval)
			break;
#else
		retval =
#endif
		put_user(value, (unsigned long *) arg);
		break;

	case LIRC_SET_SEND_MODE:
	case LIRC_SET_REC_MODE:
#ifdef KERNEL_2_0
		retval = verify_area(VERIFY_READ, (unsigned long *) arg,
			sizeof(unsigned long));
		if (retval)
			break;
		value = get_user((unsigned long *) arg);
#else
		retval = get_user(value, (unsigned long *) arg);
#endif
		break;
#if defined(CONFIG_ARCH_SA1100) || defined(CONFIG_ARCH_PXA)
	case LIRC_SET_SEND_DUTY_CYCLE:
#               ifdef KERNEL_2_1
		retval=get_user(ivalue,(unsigned int *) arg);
		if(retval) return(retval);
#               else
		retval=verify_area(VERIFY_READ,(unsigned int *) arg,
				   sizeof(unsigned int));
		if(result) return(result);
		ivalue=get_user((unsigned int *) arg);
#               endif
		if(ivalue<=0 || ivalue>100) return(-EINVAL);
		/* (ivalue/100)*(1000000/freq) */
		duty_cycle=ivalue;
		pulse_width=(unsigned long) duty_cycle*10000/freq;
		space_width=(unsigned long) 1000000L/freq-pulse_width;
#ifdef CONFIG_ARCH_SA1100
		if(pulse_width>=CONFIG_ARCH_SA1100_TRANSMITTER_LATENCY)
			pulse_width-=CONFIG_ARCH_SA1100_TRANSMITTER_LATENCY;
		if(space_width>=CONFIG_ARCH_SA1100_TRANSMITTER_LATENCY)
			space_width-=CONFIG_ARCH_SA1100_TRANSMITTER_LATENCY;
#endif 
#ifdef CONFIG_ARCH_PXA
		if(pulse_width>=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY)
			pulse_width-=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY;
		if(space_width>=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY)
			space_width-=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY;
#endif
		break;
	case LIRC_SET_SEND_CARRIER:
#               ifdef KERNEL_2_1
		retval=get_user(ivalue,(unsigned int *) arg);
		if(retval) return(retval);
#               else
		retval=verify_area(VERIFY_READ,(unsigned int *) arg,
				   sizeof(unsigned int));
		if(retval) return(retval);
		ivalue=get_user((unsigned int *) arg);
#               endif
		if(ivalue>500000 || ivalue<20000) return(-EINVAL);
		freq=ivalue;
		pulse_width=(unsigned long) duty_cycle*10000/freq;
		space_width=(unsigned long) 1000000L/freq-pulse_width;
#ifdef CONFIG_ARCH_SA1100
		if(pulse_width>=CONFIG_ARCH_SA1100_TRANSMITTER_LATENCY)
			pulse_width-=CONFIG_ARCH_SA1100_TRANSMITTER_LATENCY;
		if(space_width>=CONFIG_ARCH_SA1100_TRANSMITTER_LATENCY)
			space_width-=CONFIG_ARCH_SA1100_TRANSMITTER_LATENCY;
#endif
#ifdef CONFIG_ARCH_PXA
		if(pulse_width>=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY)
			pulse_width-=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY;
		if(space_width>=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY)
			space_width-=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY;
#endif
		break;
#endif
	default:
		retval = -ENOIOCTLCMD;

	}
	
	if (retval)
		return retval;
	
#ifdef LIRC_SIR_TEKRAM
	if (cmd == LIRC_SET_REC_MODE) {
		if (value != LIRC_MODE_MODE2)
			retval = -ENOSYS;
	} else if (cmd == LIRC_SET_SEND_MODE) {
		retval = -ENOSYS;
	}
#else
	if (cmd == LIRC_SET_REC_MODE) {
		if (value != LIRC_MODE_MODE2)
			retval = -ENOSYS;
	} else if (cmd == LIRC_SET_SEND_MODE) {
		if (value != LIRC_MODE_PULSE)
			retval = -ENOSYS;
	}
#endif
	return retval;
}

static void add_read_queue(int flag, unsigned long val)
{
	unsigned int new_rx_tail;
	lirc_t newval;

#ifdef DEBUG_SIGNAL
	printk(KERN_DEBUG LIRC_DRIVER_NAME
		": add flag %d with val %lu\n",
		flag,val);
#endif

	newval = val & PULSE_MASK;

	/* statistically pulses are ~TIME_CONST/2 too long: we could
	   maybe make this more exactly but this is good enough */
	if(flag) /* pulse */
	{
		if(newval>TIME_CONST/2)
		{
			newval-=TIME_CONST/2;
		}
		else /* should not ever happen */
		{
			newval=1;
		}
		newval|=PULSE_BIT;
	}
	else
	{
		newval+=TIME_CONST/2;
	}
	new_rx_tail = (rx_tail + 1) & (RBUF_LEN - 1);
	if (new_rx_tail == rx_head) {
#               ifdef DEBUG
		printk(KERN_WARNING LIRC_DRIVER_NAME ": Buffer overrun.\n");
#               endif
		return;
	}
	rx_buf[rx_tail] = newval;
	rx_tail = new_rx_tail;
	wake_up_interruptible(&lirc_read_queue);
}

static struct file_operations lirc_fops =
{
	read:    lirc_read,
	write:   lirc_write,
#ifdef KERNEL_2_1
	poll:    lirc_poll,
#else
	select:  lirc_select,
#endif
	ioctl:   lirc_ioctl,
	open:    lirc_open,
	release: lirc_close,
};

static void inline lirc_pm_suspend(void)
{
#ifdef CONFIG_ARCH_I519
	set_GPIO_mode(GPIO80_IRDA_EN | GPIO_IN);
#else
	set_GPIO_mode(GPIO47_STTXD | GPIO_IN);
#endif
}

static void inline lirc_pm_resume(void)
{
#ifdef CONFIG_ARCH_I519
	set_GPIO_mode(GPIO80_IRDA_EN | GPIO_OUT);
#else
	set_GPIO_mode(GPIO47_STTXD | GPIO_OUT);
#endif
}

#ifdef CONFIG_PM
static int lirc_irda_pmproc(struct pm_dev *dev, pm_request_t rq, void *data)
{
	if (!dev->data)
		return -EINVAL;

	switch (rq) {
		case PM_SUSPEND:
			lirc_pm_suspend();
		break;
		case PM_RESUME:
			lirc_pm_resume();
		break;
	}

	return 0;
}
#endif /* CONFIG_PM */

#ifdef MODULE
static devfs_handle_t lirc_devfs;
int init_chrdev(void)
{
	int retval;

#if 0
	retval = register_chrdev(major, LIRC_DRIVER_NAME, &lirc_fops);
	if (retval < 0) {
		printk(KERN_ERR LIRC_DRIVER_NAME ": init_chrdev() failed.\n");
		return retval;
	}
#else
	lirc_devfs = devfs_register (NULL, "lirc", DEVFS_FL_NONE,
			major, 0, S_IRUSR | S_IWUSR | S_IRGRP | S_IFCHR,
			&lirc_fops, NULL);
	devfs_register_chrdev(major, "lirc", &lirc_fops);
#endif

#ifdef CONFIG_PM
	/*
	 * Power-Management is optional.
	 */
	lirc_pmdev = pm_register(PM_SYS_DEV, PM_SYS_IRDA, lirc_irda_pmproc);
	if (lirc_pmdev)
		lirc_pmdev->data = (void*)&lirc_pmdev;
#endif /* CONFIG_PM */

	return 0;
}

static void drop_chrdev(void)
{
#ifdef CONFIG_PM
	if (lirc_pmdev)
		pm_unregister(lirc_pmdev);
	lirc_pmdev->data = 0;
	lirc_pmdev = NULL;
#endif /* CONFIG_PM */

#if 0
	unregister_chrdev(major, LIRC_DRIVER_NAME);
#else
	devfs_unregister_chrdev(major, "lirc");
	if (lirc_devfs)
		devfs_unregister(lirc_devfs);
#endif
}
#endif

/* SECTION: Hardware */
static long delta(struct timeval * tv1, struct timeval * tv2)
{
	unsigned long deltv;
	
	deltv = tv2->tv_sec - tv1->tv_sec;
	if (deltv > 15)
		deltv = 0xFFFFFF;
	else
		deltv = deltv*1000000 +
			tv2->tv_usec -
			tv1->tv_usec;
	return deltv;
}

static void sir_timeout(unsigned long data) 
{
	/* if last received signal was a pulse, but receiving stopped
	   within the 9 bit frame, we need to finish this pulse and
	   simulate a signal change to from pulse to space. Otherwise
	   upper layers will receive two sequences next time. */
	
	unsigned long flags;
	unsigned long pulse_end;
	
	/* avoid interference with interrupt */
 	spin_lock_irqsave(&timer_lock, flags);
	if (last_value)
	{
#if !defined(CONFIG_ARCH_SA1100) && !defined(CONFIG_ARCH_PXA)
		/* clear unread bits in UART and restart */
		outb(UART_FCR_CLEAR_RCVR, io + UART_FCR);
#endif
		/* determine 'virtual' pulse end: */
	 	pulse_end = delta(&last_tv, &last_intr_tv);
#ifdef DEBUG_SIGNAL
		printk(KERN_DEBUG LIRC_DRIVER_NAME
			": timeout add %d for %lu usec\n",last_value,pulse_end);
#endif
		add_read_queue(last_value,pulse_end);
		last_value = 0;
		last_tv=last_intr_tv;
	}
	spin_unlock_irqrestore(&timer_lock, flags);		
}

static void sir_interrupt(int irq, void * dev_id, struct pt_regs * regs)
{
	struct timeval curr_tv;
	static unsigned long deltv;
#ifdef CONFIG_ARCH_SA1100
	unsigned char data;
	int status;
	static int n=0;
	
	//printk("interrupt\n");
	status = Ser2UTSR0;
	/*
	 * Deal with any receive errors first.  The bytes in error may be
	 * the only bytes in the receive FIFO, so we do this first.
	 */
	while (status & UTSR0_EIF)
	{
		int bstat;
		
#ifdef DEBUG
		printk("EIF\n");
		bstat = Ser2UTSR1;
		
		if (bstat & UTSR1_FRE)
			printk("frame error\n");
		if (bstat & UTSR1_ROR)
			printk("receive fifo overrun\n");
		if(bstat&UTSR1_PRE)
			printk("parity error\n");
#endif
		
		bstat = Ser2UTDR;
		n++;
		status = Ser2UTSR0;
	}

	if (status & (UTSR0_RFS | UTSR0_RID))
	{
		do_gettimeofday(&curr_tv);
		deltv = delta(&last_tv, &curr_tv);
		do
		{
#ifdef DEBUG_SIGNAL
			printk(KERN_DEBUG LIRC_DRIVER_NAME": t %lu , d %d\n",
			       deltintrtv,(int)data);
#endif
			data=Ser2UTDR;
			//printk("data: %d\n",data);
			n++;
		}
		while(status&UTSR0_RID && /* do not empty fifo in
                                             order to get UTSR0_RID in
                                             any case */
		      Ser2UTSR1 & UTSR1_RNE); /* data ready */
		
		if(status&UTSR0_RID)
		{
			//printk("add\n");
			add_read_queue(0,deltv-n*TIME_CONST); /*space*/
			add_read_queue(1,n*TIME_CONST); /*pulse*/
			n=0;
			last_tv=curr_tv;
		}
	}

	if (status & UTSR0_TFS) {

		printk("transmit fifo not full, shouldn't ever happen\n");
	}

	/*
	 * We must clear certain bits.
	 */
	status &= (UTSR0_RID | UTSR0_RBB | UTSR0_REB);
	if (status)
		Ser2UTSR0 = status;

#elif defined(CONFIG_ARCH_PXA)
	unsigned char data;
	unsigned long deltintrtv;
	unsigned long flags;
	int iir, lsr;

	while ((iir = STIIR) & UART_IIR_ID) {
		switch (iir&UART_IIR_ID) { /* FIXME toto treba preriedit */
		case UART_IIR_MSI:
			(void) (STMSR);
			break;
		case UART_IIR_RLSI:
			(void) (UART_LSR);
			break;
		case UART_IIR_THRI:
			break;
		case UART_IIR_RDI:
			/* avoid interference with timer */
		 	spin_lock_irqsave(&timer_lock, flags);
			do
			{
				del_timer(&timerlist);
				data = STRBR;
				do_gettimeofday(&curr_tv);
				deltv = delta(&last_tv, &curr_tv);
				deltintrtv = delta(&last_intr_tv, &curr_tv);
#ifdef DEBUG_SIGNAL
				printk(KERN_DEBUG LIRC_DRIVER_NAME": t %lu , d %d\n",deltintrtv,(int)data);
#endif
				/* if nothing came in last 2 cycles,
				   it was gap */
				if (deltintrtv > TIME_CONST * 2) {
					if (last_value) {
#ifdef DEBUG_SIGNAL
						printk(KERN_DEBUG LIRC_DRIVER_NAME ": GAP\n");
#endif
						/* simulate signal change */
						add_read_queue(last_value,
							       deltv-
							       deltintrtv);
						last_value = 0;
						last_tv.tv_sec = last_intr_tv.tv_sec;
						last_tv.tv_usec = last_intr_tv.tv_usec;
						deltv = deltintrtv;
					}
				}
				data = 1;
				if (data ^ last_value) {
					/* deltintrtv > 2*TIME_CONST,
                                           remember ? */
					/* the other case is timeout */
					add_read_queue(last_value,
						       deltv-TIME_CONST);
					last_value = data;
					last_tv = curr_tv;
					if(last_tv.tv_usec>=TIME_CONST)
					{
						last_tv.tv_usec-=TIME_CONST;
					}
					else
					{
						last_tv.tv_sec--;
						last_tv.tv_usec+=1000000-
							TIME_CONST;
					}
				}
				last_intr_tv = curr_tv;
				if (data)
				{
					/* start timer for end of sequence detection */
					timerlist.expires = jiffies + SIR_TIMEOUT;
					add_timer(&timerlist);
				}
			}
			while ((lsr = STLSR) & UART_LSR_DR); /* data ready */
			spin_unlock_irqrestore(&timer_lock, flags);
			break;
		default:
			break;
		}
	}
#else
	unsigned char data;
	unsigned long deltintrtv;
	unsigned long flags;
	int iir, lsr;

	while ((iir = inb(io + UART_IIR) & UART_IIR_ID)) {
		switch (iir&UART_IIR_ID) { /* FIXME toto treba preriedit */
		case UART_IIR_MSI:
			(void) inb(io + UART_MSR);
			break;
		case UART_IIR_RLSI:
			(void) inb(io + UART_LSR);
			break;
		case UART_IIR_THRI:
#if 0
			if (lsr & UART_LSR_THRE) /* FIFO is empty */
				outb(data, io + UART_TX)
#endif
			break;
		case UART_IIR_RDI:
			/* avoid interference with timer */
		 	spin_lock_irqsave(&timer_lock, flags);
			do
			{
				del_timer(&timerlist);
				data = inb(io + UART_RX);
				do_gettimeofday(&curr_tv);
				deltv = delta(&last_tv, &curr_tv);
				deltintrtv = delta(&last_intr_tv, &curr_tv);
#ifdef DEBUG_SIGNAL
				printk(KERN_DEBUG LIRC_DRIVER_NAME": t %lu , d %d\n",deltintrtv,(int)data);
#endif
				/* if nothing came in last 2 cycles,
				   it was gap */
				if (deltintrtv > TIME_CONST * 2) {
					if (last_value) {
#ifdef DEBUG_SIGNAL
						printk(KERN_DEBUG LIRC_DRIVER_NAME ": GAP\n");
#endif
						/* simulate signal change */
						add_read_queue(last_value,
							       deltv-
							       deltintrtv);
						last_value = 0;
						last_tv.tv_sec = last_intr_tv.tv_sec;
						last_tv.tv_usec = last_intr_tv.tv_usec;
						deltv = deltintrtv;
					}
				}
				data = 1;
				if (data ^ last_value) {
					/* deltintrtv > 2*TIME_CONST,
                                           remember ? */
					/* the other case is timeout */
					add_read_queue(last_value,
						       deltv-TIME_CONST);
					last_value = data;
					last_tv = curr_tv;
					if(last_tv.tv_usec>=TIME_CONST)
					{
						last_tv.tv_usec-=TIME_CONST;
					}
					else
					{
						last_tv.tv_sec--;
						last_tv.tv_usec+=1000000-
							TIME_CONST;
					}
				}
				last_intr_tv = curr_tv;
				if (data)
				{
					/* start timer for end of sequence detection */
					timerlist.expires = jiffies + SIR_TIMEOUT;
					add_timer(&timerlist);
				}
			}
			while ((lsr = inb(io + UART_LSR))
				& UART_LSR_DR); /* data ready */
			spin_unlock_irqrestore(&timer_lock, flags);
			break;
		default:
			break;
		}
	}
#endif
}

#if defined(CONFIG_ARCH_SA1100) || defined(CONFIG_ARCH_PXA)
void send_pulse(unsigned long length)
{
	unsigned long k,delay;
	int flag;

	if(length==0) return;
	/* this won't give us the carrier frequency we really want
	   due to integer arithmetic, but we can accept this inaccuracy */

	for(k=flag=0;k<length;k+=delay,flag=!flag)
	{
		if(flag)
		{
			off();
			delay=space_width;
		}
		else
		{
			on();
			delay=pulse_width;
		}
		safe_udelay(delay);
	}
	off();
}

void send_space(unsigned long length)
{
	if(length==0) return;
	off();
	safe_udelay(length);
}
#elif defined(LIRC_SIR_TEKRAM)
#else
static void send_space(unsigned long len)
{
	safe_udelay(len);
}

static void send_pulse(unsigned long len)
{
	long bytes_out = len / TIME_CONST;
	long time_left;

	if (!bytes_out)
		bytes_out++;
	time_left = (long)len - (long)bytes_out * (long)TIME_CONST;
	while (--bytes_out) {
		outb(PULSE, io + UART_TX);
		/* FIXME treba seriozne cakanie z drivers/char/serial.c */
		while (!(inb(io + UART_LSR) & UART_LSR_THRE));
	}
#if 0
	if (time_left > 0)
		safe_udelay(time_left);
#endif
}
#endif

#ifdef CONFIG_SA1100_COLLIE
static inline int sa1100_irda_set_power_collie(int state)
{
	if (state) {
		/*
		 *  0 - off
		 *  1 - short range, lowest power
		 *  2 - medium range, medium power
		 *  3 - maximum range, high power
		 */
		ucb1200_set_io_direction(TC35143_GPIO_IR_ON,
					 TC35143_IODIR_OUTPUT);
		ucb1200_set_io(TC35143_GPIO_IR_ON, TC35143_IODAT_LOW);
		udelay(100);
	}
	else {
		/* OFF */
		ucb1200_set_io_direction(TC35143_GPIO_IR_ON,
					 TC35143_IODIR_OUTPUT);
		ucb1200_set_io(TC35143_GPIO_IR_ON, TC35143_IODAT_HIGH);
	}
	return 0;
}
#endif

static int init_hardware(void)
{
	int flags;
	
	spin_lock_irqsave(&hardware_lock, flags);
	/* reset UART */
#ifdef CONFIG_ARCH_SA1100
#ifdef CONFIG_SA1100_BITSY
	if (machine_is_bitsy()) {
		printk("Power on IR module\n");
		set_bitsy_egpio(EGPIO_BITSY_IR_ON);
	}
#endif
#ifdef CONFIG_SA1100_COLLIE
	sa1100_irda_set_power_collie(3);	/* power on */
#endif
#ifdef CONFIG_SA1100_WISMO
	clr_wismo_pcr(WISMO_PCR_IRDA_SHDN);
#endif
	sr.hscr0=Ser2HSCR0;

	sr.utcr0=Ser2UTCR0;
	sr.utcr1=Ser2UTCR1;
	sr.utcr2=Ser2UTCR2;
	sr.utcr3=Ser2UTCR3;
	sr.utcr4=Ser2UTCR4;

	sr.utdr=Ser2UTDR;
	sr.utsr0=Ser2UTSR0;
	sr.utsr1=Ser2UTSR1;

	/* configure GPIO */
	/* output */
	PPDR|=PPC_TXD2;
	PSDR|=PPC_TXD2;
	/* set output to 0 */
	off();
	
	/*
	 * Enable HP-SIR modulation, and ensure that the port is disabled.
	 */
	Ser2UTCR3=0;
	Ser2HSCR0=sr.hscr0 & (~HSCR0_HSSP);
	
	/* clear status register to prevent unwanted interrupts */
	Ser2UTSR0 &= (UTSR0_RID | UTSR0_RBB | UTSR0_REB);
	
	/* 7N1 */
	Ser2UTCR0=UTCR0_1StpBit|UTCR0_7BitData;
	/* 115200 */
	Ser2UTCR1=0;
	Ser2UTCR2=1;
	/* use HPSIR, 1.6 usec pulses */
	Ser2UTCR4=UTCR4_HPSIR|UTCR4_Z1_6us;
	
	/* enable receiver, receive fifo interrupt */
	Ser2UTCR3=UTCR3_RXE|UTCR3_RIE;
	
	/* clear status register to prevent unwanted interrupts */
	Ser2UTSR0 &= (UTSR0_RID | UTSR0_RBB | UTSR0_REB);

#elif defined(CONFIG_ARCH_PXA)

#ifdef CONFIG_ARCH_I519
	GPCR(GPIO20_IRDA_SD) = GPIO_bit(GPIO20_IRDA_SD);
//	GPSR(GPIO80_IRDA_EN) = GPIO_bit(GPIO80_IRDA_EN);
	GPCR(GPIO80_IRDA_EN) = GPIO_bit(GPIO80_IRDA_EN);
	set_GPIO_mode(GPIO20_IRDA_SD | GPIO_OUT);
	set_GPIO_mode(GPIO80_IRDA_EN | GPIO_OUT);
#endif	
	CKEN |= CKEN5_STUART;

	sr.strbr = STRBR;
	sr.stthr = STTHR;
	sr.stier = STIER;
	sr.stiir = STIIR;
	sr.stfcr = STFCR;
	sr.stlcr = STLCR;
	sr.stmcr = STMCR;
	sr.stlsr = STLSR;
	sr.stmsr = STMSR;
	sr.stspr = STSPR;
	sr.stisr = STISR;
	sr.stdll = STDLL;
	sr.stdlh = STDLH;
	sr.iccr0 = ICCR0;
	sr.cken = CKEN;

	/* GPIO out */
	GPCR(GPIO47_STTXD) = GPIO_bit(GPIO47_STTXD);
	set_GPIO_mode(GPIO47_STTXD | GPIO_OUT);
	GPCR(GPIO47_STTXD) = GPIO_bit(GPIO47_STTXD);

	/* ICP in */
	set_GPIO_mode(GPIO46_STRXD_MD);

	/* 115200 */
	STLCR |= LCR_DLAB;
	STDLH = ((14745600 / (16*115200)) >> 8) & 0xFF;
	STDLL = (14745600 / (16*115200)) & 0xFF;
	STLCR &= ~LCR_DLAB;

	/* connect STUART interrupt to CPU */
	STMCR = MCR_OUT2;

	/* enable STUART clock */
	CKEN |= CKEN5_STUART; 

	/* ??? */
	ICMR |= ( 1 << 20 );

	/* 7N1 */
	STLCR = LCR_WLS1;

	/* enable Rx, 1.6us pulse width */
	STISR = STISR_RCVEIR | STISR_XMODE;

	/* FIFO reset */
	STFCR = FCR_RESETTF | FCR_RESETRF | FCR_ITL_1 ;

	/* STUART unit enable, Rx interrupt enable */
	STIER = IER_UUE | IER_RAVIE;

	
#elif defined(LIRC_SIR_TEKRAM)
	/* disable FIFO */ 
	soutp(UART_FCR,
	      UART_FCR_CLEAR_RCVR|
	      UART_FCR_CLEAR_XMIT|
	      UART_FCR_TRIGGER_1);
	
	/* Set DLAB 0. */
	soutp(UART_LCR, sinp(UART_LCR) & (~UART_LCR_DLAB));
	
	/* First of all, disable all interrupts */
	soutp(UART_IER, sinp(UART_IER)&
	      (~(UART_IER_MSI|UART_IER_RLSI|UART_IER_THRI|UART_IER_RDI)));
	
	/* Set DLAB 1. */
	soutp(UART_LCR, sinp(UART_LCR) | UART_LCR_DLAB);
	
	/* Set divisor to 12 => 9600 Baud */
	soutp(UART_DLM,0);
	soutp(UART_DLL,12);
	
	/* Set DLAB 0. */
	soutp(UART_LCR, sinp(UART_LCR) & (~UART_LCR_DLAB));
	
	/* power supply */
	soutp(UART_MCR, UART_MCR_RTS|UART_MCR_DTR|UART_MCR_OUT2);
	safe_udelay(50*1000);
	
	/* -DTR low -> reset PIC */
	soutp(UART_MCR, UART_MCR_RTS|UART_MCR_OUT2);
	udelay(1*1000);
	
	soutp(UART_MCR, UART_MCR_RTS|UART_MCR_DTR|UART_MCR_OUT2);
	udelay(100);


        /* -RTS low -> send control byte */
	soutp(UART_MCR, UART_MCR_DTR|UART_MCR_OUT2);
	udelay(7);
	soutp(UART_TX, TEKRAM_115200|TEKRAM_PW);
	
	/* one byte takes ~1042 usec to transmit at 9600,8N1 */
	udelay(1500);
	
	/* back to normal operation */
	soutp(UART_MCR, UART_MCR_RTS|UART_MCR_DTR|UART_MCR_OUT2);
	udelay(50);

	udelay(1500);
	
	/* read previous control byte */
	printk(KERN_INFO LIRC_DRIVER_NAME
	       ": 0x%02x\n",sinp(UART_RX));
	
	/* Set DLAB 1. */
	soutp(UART_LCR, sinp(UART_LCR) | UART_LCR_DLAB);
	
	/* Set divisor to 1 => 115200 Baud */
	soutp(UART_DLM,0);
	soutp(UART_DLL,1);

	/* Set DLAB 0, 8 Bit */
	soutp(UART_LCR, UART_LCR_WLEN8);
	/* enable interrupts */
	soutp(UART_IER, sinp(UART_IER)|UART_IER_RDI);
#else
	outb(0, io + UART_MCR);
	outb(0, io + UART_IER);
	/* init UART */
		/* set DLAB, speed = 115200 */
	outb(UART_LCR_DLAB | UART_LCR_WLEN7, io + UART_LCR);
	outb(1, io + UART_DLL); outb(0, io + UART_DLM);
		/* 7N1+start = 9 bits at 115200 ~ 3 bits at 44000 */
	outb(UART_LCR_WLEN7, io + UART_LCR);
		/* FIFO operation */
	outb(UART_FCR_ENABLE_FIFO, io + UART_FCR);
		/* interrupts */
	// outb(UART_IER_RLSI|UART_IER_RDI|UART_IER_THRI, io + UART_IER);
	outb(UART_IER_RDI, io + UART_IER);	
	/* turn on UART */
	outb(UART_MCR_DTR|UART_MCR_RTS|UART_MCR_OUT2, io + UART_MCR);
#ifdef LIRC_SIR_ACTISYS_ACT200L
	init_act200();
#endif
#endif
	spin_unlock_irqrestore(&hardware_lock, flags);
	return 0;
}

static void drop_hardware(void)
{
	int flags;

	spin_lock_irqsave(&hardware_lock, flags);

#ifdef CONFIG_ARCH_SA1100
	Ser2UTCR3=0;
	
	Ser2UTCR0=sr.utcr0;
	Ser2UTCR1=sr.utcr1;
	Ser2UTCR2=sr.utcr2;
	Ser2UTCR4=sr.utcr4;
	Ser2UTCR3=sr.utcr3;
	
	Ser2HSCR0=sr.hscr0;
#ifdef CONFIG_SA1100_BITSY
	if (machine_is_bitsy()) {
		clr_bitsy_egpio(EGPIO_BITSY_IR_ON);
	}
#endif
#ifdef CONFIG_SA1100_COLLIE
	sa1100_irda_set_power_collie(0);	/* power off */
#endif
#ifdef CONFIG_SA1100_WISMO
	set_wismo_pcr(WISMO_PCR_IRDA_SHDN);
#endif

#elif defined(CONFIG_ARCH_PXA)

#ifdef CONFIG_ARCH_I519
	GPCR(GPIO20_IRDA_SD) = GPIO_bit(GPIO20_IRDA_SD);
	GPCR(GPIO80_IRDA_EN) = GPIO_bit(GPIO80_IRDA_EN);
#endif	

	CKEN &= ~CKEN5_STUART;

	STRBR = sr.strbr;
	STTHR = sr.stthr;
	STIER = sr.stier;
	STIIR = sr.stiir;
	STFCR = sr.stfcr;
	STLCR = sr.stlcr;
	STMCR = sr.stmcr;
	STLSR = sr.stlsr;
	STMSR = sr.stmsr;
	STSPR = sr.stspr;
	STISR = sr.stisr;
	STDLL = sr.stdll;
	STDLH = sr.stdlh;
	ICCR0 = sr.iccr0;
	CKEN  = sr.cken;  
#else
	/* turn off interrupts */
	outb(0, io + UART_IER);	
#endif
	spin_unlock_irqrestore(&hardware_lock, flags);
}

/* SECTION: Initialisation */

static int init_port(void)
{
	int retval;
	
#if !defined(CONFIG_ARCH_SA1100) && !defined(CONFIG_ARCH_PXA)
	/* get I/O port access and IRQ line */
	retval = check_region(io, 8);
	if (retval < 0) {
		printk(KERN_ERR LIRC_DRIVER_NAME
			": i/o port 0x%.4x already in use.\n",
			io);
		return retval;
	}
#endif
	retval = request_irq(irq, sir_interrupt, SA_INTERRUPT,
			     LIRC_DRIVER_NAME, NULL);
	if (retval < 0) {
		printk(KERN_ERR LIRC_DRIVER_NAME
			": IRQ %d already in use.\n",
			irq);
		return retval;
	}
#if !defined(CONFIG_ARCH_SA1100) && !defined(CONFIG_ARCH_PXA)
	request_region(io, 8, LIRC_DRIVER_NAME);
	printk(KERN_INFO LIRC_DRIVER_NAME
		": I/O port 0x%.4x, IRQ %d.\n",
		io, irq);
#endif

	init_timer(&timerlist);
	timerlist.function = sir_timeout;
	timerlist.data = 0xabadcafe;

	return 0;
}

static void drop_port(void)
{
	disable_irq(irq);
	free_irq(irq, NULL);
#ifdef KERNEL_2_3
	del_timer_sync(&timerlist);
#else
	start_bh_atomic();
	del_timer(&timerlist);
	end_bh_atomic();
#endif
#if !defined(CONFIG_ARCH_SA1100) && !defined(CONFIG_ARCH_PXA)
	release_region(io, 8);
#endif
}

#ifdef LIRC_SIR_ACTISYS_ACT200L
/******************************************************/
/* Crystal/Cirrus CS8130 IR transceiver, used in Actisys Act200L dongle */
/* some code borrowed from Linux IRDA driver */

/* Regsiter 0: Control register #1 */
#define ACT200L_REG0    0x00
#define ACT200L_TXEN    0x01 /* Enable transmitter */
#define ACT200L_RXEN    0x02 /* Enable receiver */
#define ACT200L_ECHO    0x08 /* Echo control chars */

/* Register 1: Control register #2 */
#define ACT200L_REG1    0x10
#define ACT200L_LODB    0x01 /* Load new baud rate count value */
#define ACT200L_WIDE    0x04 /* Expand the maximum allowable pulse */

/* Register 3: Transmit mode register #2 */
#define ACT200L_REG3    0x30
#define ACT200L_B0      0x01 /* DataBits, 0=6, 1=7, 2=8, 3=9(8P)  */
#define ACT200L_B1      0x02 /* DataBits, 0=6, 1=7, 2=8, 3=9(8P)  */
#define ACT200L_CHSY    0x04 /* StartBit Synced 0=bittime, 1=startbit */

/* Register 4: Output Power register */
#define ACT200L_REG4    0x40
#define ACT200L_OP0     0x01 /* Enable LED1C output */
#define ACT200L_OP1     0x02 /* Enable LED2C output */
#define ACT200L_BLKR    0x04

/* Register 5: Receive Mode register */
#define ACT200L_REG5    0x50
#define ACT200L_RWIDL   0x01 /* fixed 1.6us pulse mode */
    /*.. other various IRDA bit modes, and TV remote modes..*/

/* Register 6: Receive Sensitivity register #1 */
#define ACT200L_REG6    0x60
#define ACT200L_RS0     0x01 /* receive threshold bit 0 */
#define ACT200L_RS1     0x02 /* receive threshold bit 1 */

/* Register 7: Receive Sensitivity register #2 */
#define ACT200L_REG7    0x70
#define ACT200L_ENPOS   0x04 /* Ignore the falling edge */

/* Register 8,9: Baud Rate Dvider register #1,#2 */
#define ACT200L_REG8    0x80
#define ACT200L_REG9    0x90

#define ACT200L_2400    0x5f
#define ACT200L_9600    0x17
#define ACT200L_19200   0x0b
#define ACT200L_38400   0x05
#define ACT200L_57600   0x03
#define ACT200L_115200  0x01

/* Register 13: Control register #3 */
#define ACT200L_REG13   0xd0
#define ACT200L_SHDW    0x01 /* Enable access to shadow registers */

/* Register 15: Status register */
#define ACT200L_REG15   0xf0

/* Register 21: Control register #4 */
#define ACT200L_REG21   0x50
#define ACT200L_EXCK    0x02 /* Disable clock output driver */
#define ACT200L_OSCL    0x04 /* oscillator in low power, medium accuracy mode */

static void init_act200(void)
{
  int i;
	__u8 control[] = {
		ACT200L_REG15,
		ACT200L_REG13 | ACT200L_SHDW,
		ACT200L_REG21 | ACT200L_EXCK | ACT200L_OSCL,
		ACT200L_REG13,
		ACT200L_REG7  | ACT200L_ENPOS,
		ACT200L_REG6  | ACT200L_RS0  | ACT200L_RS1,
		ACT200L_REG5  | ACT200L_RWIDL,
		ACT200L_REG4  | ACT200L_OP0  | ACT200L_OP1 | ACT200L_BLKR,
		ACT200L_REG3  | ACT200L_B0,
		ACT200L_REG0  | ACT200L_TXEN | ACT200L_RXEN,
		ACT200L_REG8 |  (ACT200L_115200       & 0x0f),
		ACT200L_REG9 | ((ACT200L_115200 >> 4) & 0x0f),
		ACT200L_REG1 | ACT200L_LODB | ACT200L_WIDE
	};

	/* Set DLAB 1. */
	soutp(UART_LCR, UART_LCR_DLAB | UART_LCR_WLEN8);
	
	/* Set divisor to 12 => 9600 Baud */
	soutp(UART_DLM,0);
	soutp(UART_DLL,12);
	
	/* Set DLAB 0. */
	soutp(UART_LCR, UART_LCR_WLEN8);
	/* Set divisor to 12 => 9600 Baud */

	/* power supply */
	soutp(UART_MCR, UART_MCR_RTS|UART_MCR_DTR|UART_MCR_OUT2);
	for (i=0; i<50; i++) {
		safe_udelay(1000);
	}

		/* Reset the dongle : set RTS low for 25 ms */
	soutp(UART_MCR, UART_MCR_DTR|UART_MCR_OUT2);
	for (i=0; i<25; i++) {
		udelay(1000);
	}

	soutp(UART_MCR, UART_MCR_RTS|UART_MCR_DTR|UART_MCR_OUT2);
	udelay(100);

	/* Clear DTR and set RTS to enter command mode */
	soutp(UART_MCR, UART_MCR_RTS|UART_MCR_OUT2);
	udelay(7);

/* send out the control register settings for 115K 7N1 SIR operation */
	for (i=0; i<sizeof(control); i++) {
		soutp(UART_TX, control[i]);
		/* one byte takes ~1042 usec to transmit at 9600,8N1 */
		udelay(1500);
	}

	/* back to normal operation */
	soutp(UART_MCR, UART_MCR_RTS|UART_MCR_DTR|UART_MCR_OUT2);
	udelay(50);

	udelay(1500);
	soutp(UART_LCR, sinp(UART_LCR) | UART_LCR_DLAB);

	/* Set DLAB 1. */
	soutp(UART_LCR, UART_LCR_DLAB | UART_LCR_WLEN7);

	/* Set divisor to 1 => 115200 Baud */
	soutp(UART_DLM,0);
	soutp(UART_DLL,1);

	/* Set DLAB 0. */
	soutp(UART_LCR, sinp(UART_LCR) & (~UART_LCR_DLAB));

	/* Set DLAB 0, 7 Bit */
	soutp(UART_LCR, UART_LCR_WLEN7);

	/* enable interrupts */
	soutp(UART_IER, sinp(UART_IER)|UART_IER_RDI);
}
#endif

int init_lirc_sir(void)
{
	int retval;

#ifdef KERNEL_2_3
	init_waitqueue_head(&lirc_read_queue);
#endif
	retval = init_port();
	if (retval < 0)
		return retval;
	init_hardware();
	enable_irq(irq);
	printk(KERN_INFO LIRC_DRIVER_NAME
		": Installed.\n");
	return 0;
}

#ifdef MODULE

#ifdef KERNEL_2_1

#ifdef LIRC_SIR_TEKRAM
MODULE_AUTHOR("Christoph Bartelmus");
MODULE_DESCRIPTION("Infrared receiver driver for Tekram Irmate 210");
#elif defined(CONFIG_ARCH_SA1100)
MODULE_AUTHOR("Christoph Bartelmus");
MODULE_DESCRIPTION("LIRC driver for StrongARM SA1100 embedded microprocessor");
#elif defined(LIRC_SIR_ACTISYS_ACT200L)
MODULE_AUTHOR("Karl Bongers");
MODULE_DESCRIPTION("LIRC driver for Actisys Act200L");
#else
MODULE_AUTHOR("Milan Pikula");
MODULE_DESCRIPTION("Infrared receiver driver for SIR type serial ports");
#endif

#ifdef CONFIG_ARCH_SA1100
MODULE_PARM(irq, "i");
MODULE_PARM_DESC(irq, "Interrupt (16)");
#elif defined(CONFIG_ARCH_PXA)


#else
MODULE_PARM(io, "i");
MODULE_PARM_DESC(io, "I/O address base (0x3f8 or 0x2f8)");
MODULE_PARM(irq, "i");
MODULE_PARM_DESC(irq, "Interrupt (4 or 3)");
#endif

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

EXPORT_NO_SYMBOLS;
#endif

int init_module(void)
{
	int retval;
	
	retval=init_chrdev();
	if(retval < 0)
		return retval;
	retval = init_lirc_sir();
	if (retval) {
		drop_chrdev();
		return retval;
	}

	return 0;
}

void cleanup_module(void)
{
	drop_hardware();
	drop_chrdev();
	drop_port();
	printk(KERN_INFO LIRC_DRIVER_NAME ": Uninstalled.\n");
}
#endif