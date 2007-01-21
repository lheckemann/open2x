/*
 * drivers/mmcsd/mmcsd_slot_mmsp2.c
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * MMC/SD slot interfaces specific to MMSP2 SD Controller
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/timer.h>

#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/arch/mdk.h>

#include "mmcsd.h"
#include "mmcsd_slot_mmsp2.h"

static struct tq_struct card_detect_task;
static volatile int card_in;

static DECLARE_WAIT_QUEUE_HEAD(wq);
static volatile int reading;
static volatile int rd_done,wr_done;
static u_char *buffer;
static int bufcnt;
static int datalen;
static volatile int error;

/************************************************************************
 * Initialization MMC/SD Control regisger
 *
 * Description: This function Initialize MMC/SD Control regisger
 *
 * Arguments  : none
 *
 * Returns    : none
 *
*************************************************************************/
static void init_mmcsd_reg(void)
{
	SDIDCON = 0;
#if defined(CONFIG_MACH_MMSP2_DTK4) || defined(CONFIG_MACH_MMSP2_WDMS)
	SDIPRE = MMCSD_PCLK/(MMCSD_INICLK) -1;
#endif
	SDICARG = 0;
	SDICCON = 0;
	SDICSTA = 0xffff;
	SDIBSIZE = 512;
	SDICON = 0;
	SDIDSTA = 0xffff;
	SDIFSTA = 0xffff;
	SDIDAT = 0;
	SDIIMSK1 = 0;
	SDIIMSK0 = 0;
	SDIDTIMERL = 0xffff;
	SDIDTIMERH = 0x0;
}

/************************************************************************
 * Read MMC/SD data
 *
 * Description: This function read MMC/SD data
 *
 * Arguments  : buf, len(data len)
 *
 * Returns    : none
 *
*************************************************************************/
static int read_data(struct mmcsd_slot *slot, u_char *buf, int len) 
{
	u32 stat;
#ifdef USE_POLLING
	int cnt = 0;
	while(cnt<len) // 512*block bytes
	{
		stat = SDIDSTA;
		if(SDIDSTA & SDIDSTA_TOUT) // Check timeout
		{
			SDIDSTA= SDIDSTA_TOUT;  // Clear timeout flag
			break;
		}
	
		stat = SDIFSTA;
		if(stat & SDIFSTA_RX)     // Is Rx data?
		{	
			buf[cnt]= (u8)SDIDAT;
			cnt++;
		}
	}
	
	do {
		stat = SDIDSTA;
	#if 0
		if (!card_in) 
		{
			SDIDSTA = stat;
			return -ENODEV;
		}
	#endif

	} while (!(stat & SDIDSTA_TOUT) && !(stat & SDIDSTA_DFIN)); /* time out , data transfer complete check */
	
	SDIDSTA = stat; /* clear */
	return 0;

#elif defined(USE_INTERRUPT)
	if (error)
		return -EIO;

	while(bufcnt<len) {
		stat = SDIFSTA;
		if(stat & (SDIFSTA_RX | SDIFSTA_RX_HALF | SDIFSTA_RX_LAST)) 
		{	
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
# if 1
		if(slot->bus_width) {
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			
		
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
			buf[bufcnt++]= (u8)SDIDAT;
		}
		}
	}
#endif
	rd_done = 0;
	SDIIMSK1 = SDIIMSK_TOUT | SDIIMSK_DFIN;
	while(1) {
		if (rd_done == 1) break;
	}

	return 0;
#endif /* USE_INTERRUPT */
}

/************************************************************************
 * Write MMC/SD data
 *
 * Description: This function write MMC/SD data
 *
 * Arguments  : buf, len(data len)
 *
 * Returns    : none
 *
*************************************************************************/
static int write_data(struct mmcsd_slot *slot, const u_char *buf, int len)
{
	u32 stat;
//#ifdef USE_POLLING
#if 1
	int cnt = 0;

	//interruptible_sleep_on(&wq);
	while(cnt<len)
	{
		stat=SDIFSTA;
		if(stat & SDIFSTA_TX)	/* fifo transfer status check */
		{
			(u8)SDIDAT=buf[cnt];
			cnt++;
		}
	}

	do {
		stat = SDIDSTA;
	#if 0
		if (!card_in) 
		{
			SDIDSTA = stat;
			return -ENODEV;
		}
	#endif
	} while (!(stat & SDIDSTA_TOUT) && !(stat & SDIDSTA_DFIN)); /* time out , data transfer complete check */

	SDIDSTA = stat; /* clear */
	//wake_up_interruptible(&wq);

	return 0;

#elif defined(USE_INTERRUPT)
	if (error) 
		return -EIO;

	while(bufcnt<len) {
		stat=SDIFSTA;
		//if(stat & SDIFSTA_TX_HALF)	/* fifo transfer status check */
		if(stat & SDIFSTA_TX_EMP)	/* fifo transfer status check */
		{
			(u8)SDIDAT=buf[bufcnt++];
			(u8)SDIDAT=buf[bufcnt++];
			(u8)SDIDAT=buf[bufcnt++];
			(u8)SDIDAT=buf[bufcnt++];
#if 0
		if (slot->bus_width) {
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];

			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];

			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];

			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];

			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];

			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];

			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
			SDIDAT=buf[bufcnt++];
		}
#endif
		}
	}
	
	wr_done = 0;
	SDIIMSK1 = SDIIMSK_TOUT | SDIIMSK_DFIN;
	while(1) {
		if (wr_done == 1) break;
	}

	return 0;	
#endif /* USE_INTERRUPT */
}

/************************************************************************
 * Interrupt Mode
 *
 * Description: This function data r/w in Interrupt mode
 *
 * Arguments  : irq_no, dev_id
 *
 * Returns    : none
 *
*************************************************************************/
static void sdi_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 stat;

	INTMASK |= (1<<irq);         // interrupt mask
	SDIIMSK1 = 0x0 ;
	SDIIMSK0 = 0x0 ;
	SRCPEND = (1<<irq);          // source pending flag clear
	INTPEND = (1<<irq);          // interrupt pending flag clear

	stat = SDIDSTA;
	if (stat & SDIDSTA_DFIN) {
		SDIDCON=SDIDCON&SDIDCON_DATA_STOP; //data mode clear
        	SDIDSTA=0x10;  //reset finish condition
		INTMASK |= (1<<irq);         // interrupt mask
		SDIIMSK1 = 0x0 ;
		SDIIMSK0 = 0x0 ;
		SRCPEND = (1<<irq);	// source pending flag clear
		INTPEND = (1<<irq);	// interrupt pending flag clear
		//wake_up_interruptible(&wq);
		if (wr_done == 0) 
			wr_done = 1;
		if (rd_done == 0) 
			rd_done = 1;
	} 
	else if (stat & SDIDSTA_TOUT) 
	{
		SDIDCON=SDIDCON&SDIDCON_DATA_STOP; //data mode clear
	        SDIDSTA=0x10;
		INTMASK |= (1<<irq);         // interrupt mask
		SDIIMSK1 = 0x0 ;
		SDIIMSK0 = 0x0 ;
		SRCPEND = (1<<irq);	// source pending flag clear
		INTPEND = (1<<irq);	// interrupt pending flag clear
		error = 1;
		if (wr_done == 0) 
			wr_done = 1;
		if (rd_done == 0) 
			rd_done = 1;
	}
	//SDIIMSK1 = SDIIMSK_TOUT | SDIIMSK_DFIN;
}

/************************************************************************
 * Power up slot
 *
 * Description: This function set register in power up
 *
 * Arguments  : 
 *
 * Returns    : none
 *
*************************************************************************/
static void power_up(struct mmcsd_slot *slot)
{
	/* Set block size to 512 bytes */
	SDIBSIZE = 0x200;

	/* Set timeout count */
	SDIDTIMERL = 0xffff;
	SDIDTIMERH = 0x007f;

	/* FIFO reset, Byte Order, SD clock enable */
	SDICON = SDICON_FRESET | SDICON_BE | SDICON_ENCLK;

	/* Wait for power-up :	74 clocks delay */
	mdelay(1000);
}

/************************************************************************
 * Power down slot
 *
 * Description: This function set register in power down
 *
 * Arguments  : 
 *
 * Returns    : none
 *
*************************************************************************/
static void power_down(struct mmcsd_slot *slot)
{
	/* Disable SD Controller */
	SDICON = 0x0;
	SDIIMSK1 = 0x0;
	SDIIMSK0 = 0x0;
	SDIDTIMERL = 0x0;
	SDIDTIMERH = 0x0;
	SDIBSIZE = 0x0;
}

/************************************************************************
 * Send MMC/SD command
 *
 * Description: This function Send MMC/SD command
 *
 * Arguments  : 
 *
 * Returns    : none
 *
*************************************************************************/
static int send_cmd(struct mmcsd_slot *slot,  struct mmcsd_cmd *cmd)
{
	int ret;
	u32 stat;

	/**************** Read data *****************/
	if (cmd->res_flag & MMCSD_RES_FLAG_RDATA) {
#ifdef USE_INTERRUPT
		reading = 1;
		buffer = cmd->data;
		bufcnt = 0;
		datalen = cmd->data_len;
		error = 0;
#endif
		/* FIFO Reset */
		SDICON |= SDICON_FRESET;

	#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
		stat =SDIDCON_DATA_SIZE_WORD | SDIDCON_DATA_START;
	#endif
		/* Rx after cmd, blk, 4bit bus, Rx start, blk num */
		stat = SDIDCON_RACMD_1 | SDIDCON_BLK | SDIDCON_DATA_MODE_Rx | 1 << 0;
		
		/******* Set Data Bus *******/
		if (slot->bus_width)
			stat |= SDIDCON_WIDE_1;		/* SDIDAT[0:3] */
		else
			stat |= SDIDCON_WIDE_0;		/* SDIDAT[0] */
		SDIDCON = stat;

		SDIBSIZE = cmd->data_len;

	#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
		SDIFSTA=SDIFSTA | SDIFSTA_FIFO_RESET;      // FIFO reset
	#endif
	} /*MMCSD_RES_FLAG_RDATA */

	/**************** Write data *****************/
	 else if (cmd->res_flag & MMCSD_RES_FLAG_WDATA) {
#ifdef USE_INTERRUPT
		reading = 0;
		buffer = cmd->data;
		bufcnt = 0;
		datalen = cmd->data_len;
		error = 0;
#endif
		/* FIFO Reset */
		SDICON |= SDICON_FRESET;

	#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
		stat =SDIDCON_DATA_SIZE_WORD | SDIDCON_DATA_START;
	#endif
		stat = SDIDCON_TARSP_1 | SDIDCON_BLK |SDIDCON_DATA_MODE_Tx | 1 << 0;

		/******* Set Data Bus *******/
		if (slot->bus_width)
			stat |= SDIDCON_WIDE_1;		/* SDIDAT[0:3] */
		else
			stat |= SDIDCON_WIDE_0;		/* SDIDAT[0] */
		SDIDCON = stat;

		SDIBSIZE = cmd->data_len;

	#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
		SDIFSTA=SDIFSTA | SDIFSTA_FIFO_RESET;      // FIFO reset
	#endif
	} /*MMCSD_RES_FLAG_WDATA */

	stat = SDICCON_START | cmd->cmd;
	if (cmd->res_type != MMCSD_RES_TYPE_NONE) {
		stat |=  SDICCON_WRSP;				/* Wait for Response */
		if (cmd->res_type == MMCSD_RES_TYPE_R2) {	// Response Type R2 : CID, CSD(CMD2, 9, 10)
			stat |= SDICCON_LRSP;			// Long Response
		}
	}
	
	/* Send Command */
	SDICARG = cmd->arg;
	SDICCON = stat;

	/* Check Command Rspondse */	
	if (cmd->res_type != MMCSD_RES_TYPE_NONE){
		if (cmd->res_flag & MMCSD_RES_FLAG_DATALINE) {
			ret = read_data(slot, cmd->res, cmd->data_len);
			goto cmdexit;
		} else {
			while(1) {
				stat = SDICSTA;				/* Command Reponse State Check */
			#if 0
				if (!card_in) {
					SDICSTA = stat;
					return -ENODEV;
				}
			#endif
				if (stat & SDICSTA_TOUT) {
					ret = -ETIMEDOUT;
					SDICSTA = stat; /* clear bits */
					DPRINTK("[%s] Timeout Error\n", __FUNCTION__);
					goto cmdexit;
				}
				if (stat & SDICSTA_RSP) break;		/* Command Response received */
			}

			if (!(cmd->res_flag & MMCSD_RES_FLAG_NOCRC)) {
				if ((stat & SDICSTA_ALLFLAG) != (SDICSTA_SENT | SDICSTA_RSP)) {
					ret = -EIO;
					SDICSTA = stat; /* clear bits */
					goto cmdexit;
				}
			}
			SDICSTA = stat; /* clear bits */

			if (cmd->res_flag & MMCSD_RES_FLAG_RDATA) {
				ret = read_data(slot, cmd->data, cmd->data_len);
				goto cmdexit;
			}

			if (cmd->res_flag & MMCSD_RES_FLAG_WDATA) {
				ret = write_data(slot, cmd->data, cmd->data_len);
				goto cmdexit;
			}
		
			/* Command Response (SDIRSP0~3) save */
			if (mmcsd_get_res_len(cmd->res_type) == MMCSD_RES_LEN_LONG) {
				stat = SDIRSP0;
				stat = cpu_to_be32(stat);
				memcpy(cmd->res, &stat, sizeof(stat));
				stat = SDIRSP1;
				stat = cpu_to_be32(stat);
				memcpy(cmd->res + (1 * sizeof(stat)), &stat, sizeof(stat));
				stat = SDIRSP2;
				stat = cpu_to_be32(stat);
				memcpy(cmd->res + (2 * sizeof(stat)), &stat, sizeof(stat));
				stat = SDIRSP3;
				stat = cpu_to_be32(stat);
				memcpy(cmd->res + (3 * sizeof(stat)), &stat, sizeof(stat));
			} 
			else {
				stat = SDIRSP0;
				stat = cpu_to_be32(stat);
				memcpy(cmd->res, &stat, sizeof(stat));
				stat = SDIRSP1;
				stat = cpu_to_be32(stat);
				memcpy(cmd->res + (1 * sizeof(stat)), &stat, sizeof(stat));
			}
		}
	} /* !MMCSD_RES_TYPE_NONE */
	
	else {
		do {
			stat = SDICSTA;
		#if 0
			if (!card_in) {
				SDICSTA = stat;
				return -ENODEV;
			}
		#endif
		} while (!(stat & SDICSTA_SENT));
		SDICSTA = stat; /* clear bits */
	
	} /* MMCSD_RES_TYPE_NONE */
	ret = 0;
 cmdexit:
	return ret;
}

static struct mmcsd_slot slot = {
	id:			0,
	narrow_bus:             1,			
	power_up:		power_up,
	power_down:		power_down,
	send_cmd:		send_cmd,
};

static void add_task_handler(void *data)
{
	struct mmcsd_slot *slot = (struct mmcsd_slot *)data;
	int wp;
	
#if defined(CONFIG_MACH_MMSP2_DTK3)
	wp = 0;
#elif defined(CONFIG_MACH_MMSP2_DTK4) || defined(CONFIG_MACH_MMSP2_WDMS)
	wp = read_gpio_bit(GPIO_D2);
#endif
	if (wp) {
		printk("write protected\n");
		slot->readonly = 1;
	}
	else 
		slot->readonly = 0;

#if 0
	add_mmcsd_device(slot);
#else	
	if( add_mmcsd_device(slot) < 0) printk("Re instart sdcard\n");
	else sys_mount("/dev/mmcsd/disc0/part1", "/mnt/sd", "vfat", 0xC0ED0000, "iocharset=utf8");	
#endif

}

static void del_task_handler(void *data)
{
	struct mmcsd_slot *slot = (struct mmcsd_slot *)data;

#if 1
	if(sys_umount("/mnt/sd") < 0) printk("umount error\n");
	else printk("umount ok\n");
#endif	
	
	del_mmcsd_device(slot);
	
}

/************************************************************************
 * Card detect intterupt
 *
 * Description: This function set card detect intterupt
 *
 * Arguments  : 
 *
 * Returns    : none
 *
*************************************************************************/
static void card_detect_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	int empty;

#ifdef NO_CD_IRQ
	#ifdef CONFIG_MACH_MMSP2_DTK3
		if(!(SD_MDR & GPIO_SDCD_H))
	#elif defined(CONFIG_MACH_MMSP2_DTK4)
		if(!read_gpio_bit(GPIO_I14))
	#elif defined(CONFIG_MACH_MMSP2_WDMS)
		if(!read_gpio_bit(GPIO_D3))
	#endif
			empty = 0;
		else
			empty = 1;
#else
	#ifdef CONFIG_MACH_MMSP2_DTK3
		if(!(CDINTPNR & SD_CDPN)) {
			return;
		}
		else {
			empty = 0;
			CDINTPNR |= SD_CDPN_CL;	/* SD Card Detect Pending Clear */
		}
	#elif defined(CONFIG_MACH_MMSP2_DTK4)
		empty = 0;
	#endif
#endif
	if (!card_in && !empty) {
		/* card inserted */
		card_in = 1;
		mdelay(100);
		card_detect_task.data = (void *)dev_id;
		card_detect_task.routine = add_task_handler;
		schedule_task(&card_detect_task);
	}
	else if (card_in && empty) {
		/* card ejected */
		card_in = 0;

#ifdef USE_INTERRUPT
#endif
		card_detect_task.data = (void *)dev_id;
		card_detect_task.routine = del_task_handler;
		schedule_task(&card_detect_task);
	}
}

#ifdef NO_CD_IRQ
struct timer_list cd_timer;
static void cd_timer_handler(unsigned long data)
{
	card_detect_interrupt(0, (void *)&slot, NULL);	
	mod_timer(&cd_timer, jiffies + 10);
}
#endif

/************************************************************************
 * Initialization MMC/SD slot
 *
 * Description: This function initialize mmc/sd slot
 *
 * Arguments  : 
 *
 * Returns    : none
 *
*************************************************************************/
static int __init init_mmcsd_slot(void)
{
	int ret;
	/* Initialize status variables */
	card_in = 0;

	/* Initialize MMC/SD Controller Register */
	init_mmcsd_reg();

	/* set GPIO to MMC/SD function */
	set_gpio_ctrl(GPIO_L0, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[0] */
	set_gpio_ctrl(GPIO_L1, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[1] */
	set_gpio_ctrl(GPIO_L2, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[2] */
	set_gpio_ctrl(GPIO_L3, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[3] */
	set_gpio_ctrl(GPIO_L4, GPIOMD_ALT1, GPIOPU_EN);		/* SDICMD */
	set_gpio_ctrl(GPIO_L5, GPIOMD_ALT1, GPIOPU_EN);		/* SDICLK */
#if defined(CONFIG_MACH_MMSP2_DTK4) || defined(CONFIG_MACH_MMSP2_WDMS)
	set_gpio_ctrl(GPIO_D2, GPIOMD_IN, GPIOPU_DIS);		/* SDIWP */
#endif

#ifdef NO_CD_IRQ
	#ifdef CONFIG_MACH_MMSP2_DTK4
		set_gpio_ctrl(GPIO_I14, GPIOMD_IN, GPIOPU_EN);		/* SDICD */
	#elif CONFIG_MACH_MMSP2_WDMS
		set_gpio_ctrl(GPIO_D3, GPIOMD_IN, GPIOPU_EN);		/* SDICD */
	#endif
	init_timer(&cd_timer);
	cd_timer.function = cd_timer_handler;
#else

#ifdef CONFIG_MACH_MMSP2_DTK3
	/* Card Detect Interrupt Enable Register*/
	//CDINTENR &= SD_CD_DIS;		/* SD 	  Card Detect Interrupt -> Disable   *
	CDINTENR |= SD_CD_EN;		/* SD 	  Card Detect Interrupt -> Enable   */
	CDINTENR &= PCMCIA_CD_DIS;	/* PCMCIA Card Detect Interrupt -> Disable  */
	CDINTENR &= CF_CD_DIS;		/* CF	  Card Detect Interrupt -> Disable  */

	/* Card Detect Interrupt Register */	
	CDINTMDR &= SD_CDMD_L;		/* SD Card Detect Interrupt -> Low Level */

	/* Card Detect Interrupt Pending Register */
	 CDINTPNR = SD_CDPN_CL | PCMCIA_CDPN_CL | CF_CDPN_CL;		/* All Pending Clear */
#elif defined(CONFIG_MACH_MMSP2_DTK4)
	/* Card Detect Interrupt pin set */
	set_external_irq(IRQ_CD, EINT_FALLING_EDGE, GPIOPU_NOSET);
	
	/* Register IRQ handlers */
	ret = request_irq(IRQ_CD, card_detect_interrupt, SA_INTERRUPT, "CD", (void *)&slot);
	if (ret) {
		printk("MMC/SD Slot: request_irq(SD CD) failed\n");
		return ret;
	}
#endif
#endif

#ifdef USE_INTERRUPT
	ret = request_irq(IRQ_SD, sdi_interrupt, SA_INTERRUPT, "SD", (void *)&slot);
	if (ret) {
		printk("MMC/SD Slot: request_irq(SD) failed\n");
	#ifndef NO_CD_IRQ
		free_irq(IRQ_CD, (void *)&slot);
	#endif
		return ret;
	}
#endif
	printk("MMC/SD Slot initialized\n");

#ifdef NO_CD_IRQ
	mod_timer(&cd_timer, jiffies + 10);
#else
	card_detect_interrupt(IRQ_CD, (void *)&slot, NULL);
#endif

}

static void __exit exit_mmcsd_slot(void)
{
#ifdef USE_INTERRUPT
	free_irq(IRQ_SD, (void *)&slot);
#endif
#ifndef NO_CD_IRQ
	free_irq(IRQ_CD, (void *)&slot);
#endif
	if (card_in) {
		card_in = 0;
		del_task_handler((void *)&slot);
	}
}

module_init(init_mmcsd_slot);
module_exit(exit_mmcsd_slot);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
MODULE_LICENSE("Not GPL, Proprietary License");
MODULE_DESCRIPTION("MMC/SD slot interfaces specific to MMSP2 MMC/SD Controller");
