/*
 * LIRC SIR driver, (C) 2000 Milan Pikula <www@fornax.sk>
 *
 * lirc_sir - Device driver for use with SIR (serial infra red)
 * mode of IrDA on many notebooks.
 *
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

#if  !defined(CONFIG_SERIAL_MODULE)
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
#include <asm/hardware.h>

#include <linux/timer.h>

#include "lirc.h"

/* SECTION: Definitions */
#ifdef CONFIG_PM
static struct pm_dev *lirc_pmdev;
#endif

/******************************* PXA250 ********************************/

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


#define RBUF_LEN 1024
#define WBUF_LEN 1024

#define LIRC_DRIVER_NAME "lirc_sir"

#define PULSE '['

/* 9bit * 1s/115200bit in milli seconds = 78.125ms*/
#define TIME_CONST (9000000ul/115200ul)


/* timeout for sequences in jiffies (=5/100s) */
/* must be longer than TIME_CONST */
#define SIR_TIMEOUT	(HZ*5/100)

static int major = 61; // = LIRC_MAJOR;

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

static lirc_t tx_buf[WBUF_LEN];


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

static void send_space(unsigned long len);
static void send_pulse(unsigned long len);

static int init_hardware(void);
static void drop_hardware(void);
	/* Initialisation */
static int init_port(void);
static void drop_port(void);
int init_module(void);
void cleanup_module(void);

void inline on(void)
{
	GPSR(GPIO80_IRDA_EN) = GPIO_bit(GPIO80_IRDA_EN);
}
  
void inline off(void)
{
	GPCR(GPIO80_IRDA_EN) = GPIO_bit(GPIO80_IRDA_EN);
}

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

	/* STUART unit disable, Rx interrupt disable */
	STIER &= ~(IER_UUE | IER_RAVIE);

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

	off();
	udelay(1000); /* wait 1ms for IR diode to recover */

	/* FIFO reset */
	STFCR = FCR_RESETTF | FCR_RESETRF | FCR_ITL_1 ;

	/* STUART unit enable, Rx interrupt enable */
	STIER = IER_UUE | IER_RAVIE;

	return n;
}

static int lirc_ioctl(struct inode *node,struct file *filep,unsigned int cmd,
		unsigned long arg)
{
	int retval = 0;
	unsigned long value = 0;

	unsigned int ivalue;

	if (cmd == LIRC_GET_FEATURES)
		value = LIRC_CAN_SEND_PULSE |
			LIRC_CAN_SET_SEND_DUTY_CYCLE |
			LIRC_CAN_SET_SEND_CARRIER |
			LIRC_CAN_REC_MODE2;
	else if (cmd == LIRC_GET_SEND_MODE)
		value = LIRC_MODE_PULSE;
	else if (cmd == LIRC_GET_REC_MODE)
		value = LIRC_MODE_MODE2;

	switch (cmd) {
	case LIRC_GET_FEATURES:
	case LIRC_GET_SEND_MODE:
	case LIRC_GET_REC_MODE:
		put_user(value, (unsigned long *) arg);
		break;

	case LIRC_SET_SEND_MODE:
	case LIRC_SET_REC_MODE:
		retval = get_user(value, (unsigned long *) arg);
		break;

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


		if(pulse_width>=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY)
			pulse_width-=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY;
		if(space_width>=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY)
			space_width-=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY;

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

		if(pulse_width>=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY)
			pulse_width-=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY;
		if(space_width>=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY)
			space_width-=CONFIG_ARCH_PXA_TRANSMITTER_LATENCY;

		break;

	default:
		retval = -ENOIOCTLCMD;

	}
	
	if (retval)
		return retval;
	
	if (cmd == LIRC_SET_REC_MODE) {
		if (value != LIRC_MODE_MODE2)
			retval = -ENOSYS;
	} else if (cmd == LIRC_SET_SEND_MODE) {
		if (value != LIRC_MODE_PULSE)
			retval = -ENOSYS;
	}
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
	set_GPIO_mode(GPIO80_IRDA_EN | GPIO_IN);
}

static void inline lirc_pm_resume(void)
{
	set_GPIO_mode(GPIO80_IRDA_EN | GPIO_OUT);
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
	lirc_devfs = devfs_register (NULL, "lirc", DEVFS_FL_NONE,
			major, 0, S_IRUSR | S_IWUSR | S_IRGRP | S_IFCHR,
			&lirc_fops, NULL);
	devfs_register_chrdev(major, "lirc", &lirc_fops);

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

	devfs_unregister_chrdev(major, "lirc");
	if (lirc_devfs)
		devfs_unregister(lirc_devfs);
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
}

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

static int init_hardware(void)
{
	int flags;
	
	spin_lock_irqsave(&hardware_lock, flags);
	/* reset UART */
	GPCR(GPIO20_IRDA_SD) = GPIO_bit(GPIO20_IRDA_SD);
	GPCR(GPIO80_IRDA_EN) = GPIO_bit(GPIO80_IRDA_EN);
	set_GPIO_mode(GPIO20_IRDA_SD | GPIO_OUT);
	set_GPIO_mode(GPIO80_IRDA_EN | GPIO_OUT);
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

	
	spin_unlock_irqrestore(&hardware_lock, flags);
	return 0;
}

static void drop_hardware(void)
{
	int flags;

	spin_lock_irqsave(&hardware_lock, flags);

	GPCR(GPIO20_IRDA_SD) = GPIO_bit(GPIO20_IRDA_SD);
	GPCR(GPIO80_IRDA_EN) = GPIO_bit(GPIO80_IRDA_EN);

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

	spin_unlock_irqrestore(&hardware_lock, flags);
}

/* SECTION: Initialisation */

static int init_port(void)
{
	int retval;
	
	retval = request_irq(irq, sir_interrupt, SA_INTERRUPT,
			     LIRC_DRIVER_NAME, NULL);
	if (retval < 0) {
		printk(KERN_ERR LIRC_DRIVER_NAME
			": IRQ %d already in use.\n",
			irq);
		return retval;
	}

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
}


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
MODULE_AUTHOR("Milan Pikula");
MODULE_DESCRIPTION("Infrared receiver driver for SIR type serial ports");
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
