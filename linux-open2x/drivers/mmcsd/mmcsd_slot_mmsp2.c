/*
 * drivers/mmcsd/mmcsd_slot_mmsp2.c
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * Kim,SungHo < shkim@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * derived from mmcsd_slot_mmsp2.c
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

static void sleep(unsigned howlong)
{
	current->state = TASK_INTERRUPTIBLE;
	schedule_timeout(howlong);
}

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
	SDIPRE = 0;
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
	SDIDTIMERH = 0x001f;
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
#ifdef USE_POLLING
	int cnt = 0;
	u32 stat;
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
		if (!card_in) 
		{
			SDIDSTA = stat;
			return -ENODEV;
		}
	
	} while (!(stat & SDIDSTA_TOUT) && !(stat & SDIDSTA_DFIN)); /* time out , data transfer complete check */
	
	SDIDSTA = stat; /* clear */
	return 0;

#elif defined(USE_INTERRUPT)

#if 0 /* org */
	DPRINTK1("###[%s:%d]->%s: sleep\n", __FILE__, __LINE__, __FUNCTION__);
	rd_done = 0;
	SDIIMSK1 = SDIIMSK_TOUT | SDIIMSK_DFIN | SDIIMSK_RX_LAST |SDIIMSK_RX_FULL; //hbahn
	interruptible_sleep_on(&wq);

	while(1)
		if (rd_done == 1) break;

	if (!card_in) {
		return -ENODEV;
	}

	if (error) {
		return -EIO;
	}

	return 0;
#else /* shkim patch : 051219 */
	int cnt = 0;
	u32 stat;
	int ff_cnt, i;

	//SDIIMSK1 = SDIIMSK_TOUT | SDIIMSK_DFIN | SDIIMSK_RX_LAST |SDIIMSK_RX_FULL|SDIIMSK_RX_HALF; //hbahn
	//SDIIMSK1 = SDIIMSK_CARD; //hbahn
	//interruptible_sleep_on(&wq);

	while(cnt<len) // 512*block bytes
	{
		stat = SDIDSTA;
		if(SDIDSTA & SDIDSTA_TOUT) // Check timeout
		{
			SDIDSTA= SDIDSTA_TOUT;  // Clear timeout flag
			printk("###[%s:%d]->%s: SDIDSTA_TOUT\n", __FILE__, __LINE__, __FUNCTION__);
			break;
		}
	
		stat = SDIFSTA;
		if(stat & SDIFSTA_RX)     // Is Rx data?
		{	
			ff_cnt =  stat & SDIFSTA_CNT;
			for(i=0; i<ff_cnt; i++) { 
				buf[cnt]= (u8)SDIDAT;
				cnt++;
			}
		}
	}
	
	do {
		stat = SDIDSTA;
		if (!card_in) 
		{
			SDIDSTA = stat;
			//printk("###[%s:%d]->%s: !card_in\n", __FILE__, __LINE__, __FUNCTION__);
			return -ENODEV;
		}
	
	} while (!(stat & SDIDSTA_TOUT) && !(stat & SDIDSTA_DFIN)); /* time out , data transfer complete check */
	
	SDIDSTA = stat; /* clear */
	return 0;


#endif

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
//#ifdef USE_POLLING
#if 1
	int cnt = 0;
	u32 stat;

	DPRINTK2("###[%s:%d]->%s: len(%d)******\n", __FILE__, __LINE__, __FUNCTION__,len);
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
		if (!card_in) 
		{
			SDIDSTA = stat;
			return -ENODEV;
		}
	} while (!(stat & SDIDSTA_TOUT) && !(stat & SDIDSTA_DFIN)); /* time out , data transfer complete check */

	SDIDSTA = stat; /* clear */
//	wake_up_interruptible(&wq);
	DPRINTK2("###[%s:%d]->%s:******\n", __FILE__, __LINE__, __FUNCTION__);
//	printk("F\n");

	return 0;

#else
	DPRINTK2("###[%s:%d]->%s: sleep******\n", __FILE__, __LINE__, __FUNCTION__);
	if (!card_in) 
	{
		return -ENODEV;
	}

	if (error) 
	{
		return -EIO;
	}
	wr_done = 0;
	interruptible_sleep_on(&wq);
	SDIIMSK1 = SDIIMSK_TOUT | SDIIMSK_DFIN | SDIIMSK_TX_EMP;  //hbahn

	while(1)
		if (wr_done == 1) break;

	DPRINTK2("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);

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
#ifdef USE_INTERRUPT
static void sdi_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 stat;
	u32 cnt;

	INTMASK |= (1<<irq);         // interrupt mask
	SDIIMSK1 = 0x0 ;
	SDIIMSK0 = 0x0 ;
	SRCPEND = (1<<irq);          // source pending flag clear
	INTPEND = (1<<irq);          // interrupt pending flag clear

#if 0 /* org */
	if (reading) 
	{
		stat = SDIFSTA;
		if (stat & SDIFSTA_RX_LAST) 
		{
			cnt = (stat & SDIFSTA_CNT);
			while (cnt) 
			{
				buffer[bufcnt] = (u8)SDIDAT;
				bufcnt++;
				cnt--;
			}
			SDIFSTA = (SDIFSTA &  SDIFSTA_LAST_RX); 
		} 
		else if (stat & SDIFSTA_RX_FULL) /* FIFO_FULL */
		{
//		DPRINTK1("rd[s]\n");
			cnt = SDI_MAX_RX_FIFO;
			while (cnt && (bufcnt < datalen)) 
			{
				buffer[bufcnt] = (u8)SDIDAT;
				bufcnt++;
				cnt--;
			}
		SDIIMSK1 = SDIIMSK_TOUT | SDIIMSK_DFIN | SDIIMSK_RX_LAST |SDIIMSK_RX_FULL;
		}
		else {
		}

		if (SDIDSTA & SDIDSTA_TOUT) {
			printk("SDI time out!!\n");
		}
#if DATA_DUMP /* data display */
		{
		            int div=0;
			    int i;
		    		
		             for(i=bufcnt-SDI_MAX_RX_FIFO; i<bufcnt; i++) {
		                     if((buffer[i]>0x20) && (buffer[i]<0x78)) {
		                             printk(" %02x(%c)", buffer[i], buffer[i]);
		                     }
		                    else {
		                            printk(" %02x( )", buffer[i], buffer[i]);
		                    }
		                     div++;
		                     if(div == 16) {
		                             printk("\n");
		                             div =0;
		                     }
		             }
		    
		       }
#endif
	}
 
	else 
	{
		DPRINTK1("wr[s]");
		stat = SDIFSTA;
		if (stat & SDIFSTA_TX_EMP) 
		{
		DPRINTK1("[R]");
			cnt = SDI_MAX_TX_FIFO;
			while (cnt && bufcnt < datalen) 
			{
				(unsigned char)SDIDAT = buffer[bufcnt];
				bufcnt++;
				cnt--;
			}
			SDIIMSK1 = SDIIMSK_TOUT | SDIIMSK_DFIN | SDIIMSK_TX_EMP; 
		}
		DPRINTK1("wr[f]\n");
	}
	
	stat = SDIDSTA;
	if (stat & SDIDSTA_DFIN) 
	{
		rd_done = 1; wr_done = 1;
		SDIDCON=SDIDCON&SDIDCON_DATA_STOP; //data mode clear
        	SDIDSTA=0x10;  //reset finish condition
		DPRINTK1("###[%s:%d]->%s:wake_up\n", __FILE__, __LINE__, __FUNCTION__);
		wake_up_interruptible(&wq);
	} 
	else if (stat & SDIDSTA_TOUT) 
	{
		rd_done = 1; wr_done = 1;
		SDIDCON=SDIDCON&SDIDCON_DATA_STOP; //data mode clear
	        SDIDSTA=0x10;
		error = 1;
		DPRINTK1("###[%s:%d]->%s:wake_up\n", __FILE__, __LINE__, __FUNCTION__);
		printk("###[%s:%d]->%s:wake_up\n", __FILE__, __LINE__, __FUNCTION__);
		wake_up_interruptible(&wq);
	}
	else
	{
	}

#else /* shkim patch : 051219 */

	stat = SDIDSTA;
	if (stat & SDIDSTA_INT) 
	{
		printk("###[%s:%d]->%s: SDIDSTA_INT\n", __FILE__, __LINE__, __FUNCTION__);
		wake_up_interruptible(&wq);
	} 
	if (stat & SDIDSTA_TOUT) 
	{
		printk("###[%s:%d]->%s: SDIDSTA_TOUT\n", __FILE__, __LINE__, __FUNCTION__);
		wake_up_interruptible(&wq);
	}

	stat = SDIFSTA;
	if (stat & SDIFSTA_RX) 
	{
		//printk("###[%s:%d]->%s: SDIFSTA_RX\n", __FILE__, __LINE__, __FUNCTION__);
		wake_up_interruptible(&wq);
	} 

#endif


}
#endif /* USE_INTERRRUPT */

/*
 * SD Slot Interface funtions
 */

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
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	/* Set block size to 512 bytes */
	SDIBSIZE = 0x200;

	/* Set timeout count */
	SDIDTIMERL = 0xffff;
	SDIDTIMERH = 0x001f;

	/* FIFO reset, Big Endian, SD clock enable */
	SDICON |= (SDICON_FRESET | SDICON_BE | SDICON_ENCLK);

	/* FIFO Reset */
	//SDIFSTA |= SDIFSTA_FIFO_RESET;

	/* Wait for power-up :	74 clocks delay */
	mdelay(100);
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
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
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
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
#ifdef USE_INTERRUPT
		reading = 1;
		buffer = cmd->data;
		bufcnt = 0;
		datalen = cmd->data_len;
		error = 0;
#endif
	#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
		stat =SDIDCON_DATA_SIZE_WORD | SDIDCON_DATA_START;
	#endif
		/* Rx after cmd, blk, 4bit bus, Rx start, blk num */
		stat = SDIDCON_RACMD_1 | SDIDCON_BLK | SDIDCON_DATA_MODE_Rx | 1 << 0;
		
		/******* Set Data Bus *******/
		/* SD Card */
		if(slot->sd) {
			if (slot->bus_width) {
				stat |= SDIDCON_WIDE_1;		/* SDIDAT[0:3] */
			}
			else {
				stat |= SDIDCON_WIDE_0;		/* SDIDAT[0] */
			}
		}
		/* MMC Card */
		else {
			while(slot->cnt <= 1) {
				if (slot->bus_width)
					stat |= SDIDCON_WIDE_1;		/* SDIDAT[0:3] */
				else
					stat |= SDIDCON_WIDE_0;		/* SDIDAT[0] */
				slot->cnt = slot->cnt + 1;
			}
		}

		SDIBSIZE = cmd->data_len;

		/* FIFO Reset */
		SDICON |= SDICON_FRESET;

	#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
		SDIFSTA=SDIFSTA | SDIFSTA_FIFO_RESET;      // FIFO reset
	#endif
		SDIDCON = stat;
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	} /*MMCSD_RES_FLAG_RDATA */

	/**************** Write data *****************/
	 else if (cmd->res_flag & MMCSD_RES_FLAG_WDATA) {
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
#ifdef USE_INTERRUPT
		reading = 0;
		buffer = cmd->data;
		bufcnt = 0;
		datalen = cmd->data_len;
		error = 0;
#endif
	#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
		stat =SDIDCON_DATA_SIZE_WORD | SDIDCON_DATA_START;
	#endif
		stat = SDIDCON_TARSP_1 | SDIDCON_BLK |SDIDCON_DATA_MODE_Tx | 1 << 0;

		/******* Set Data Bus *******/
		/* SD Card */
		if(slot->sd) {
			if (slot->bus_width)
				stat |= SDIDCON_WIDE_1;		/* SDIDAT[0:3] */
			else
				stat |= SDIDCON_WIDE_0;		/* SDIDAT[0] */
		}
		/* MMC Card */
		else {
			while(slot->cnt <= 1) {
				if (slot->bus_width)
					stat |= SDIDCON_WIDE_1;		/* SDIDAT[0:3] */
				else
					stat |= SDIDCON_WIDE_0;		/* SDIDAT[0] */
				slot->cnt = slot->cnt + 1;
			}
		}

		SDIBSIZE = cmd->data_len;

		/* FIFO Reset */
		SDICON |= SDICON_FRESET;

	#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
		SDIFSTA=SDIFSTA | SDIFSTA_FIFO_RESET;      // FIFO reset
	#endif
		SDIDCON = stat;
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	} /*MMCSD_RES_FLAG_WDATA */

	/**************** Data line use *****************/
	 else if (cmd->res_flag & MMCSD_RES_FLAG_DATALINE) {
#ifdef USE_INTERRUPT
		reading = 1;
		buffer = cmd->res;
		bufcnt = 0;
		datalen = cmd->data_len;
		error = 0;
#endif
	#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
		stat =SDIDCON_DATA_SIZE_WORD | SDIDCON_DATA_START;
	#endif
		stat = SDIDCON_RACMD_1 | SDIDCON_BLK | SDIDCON_DATA_MODE_Rx | 1 << 0;

		/******* Set Data Bus *******/
		/* SD Card */
		if(slot->sd) {
			if (slot->bus_width)
				stat |= SDIDCON_WIDE_1;		/* SDIDAT[0:3] */
			else
				stat |= SDIDCON_WIDE_0;		/* SDIDAT[0] */
		}
		/* MMC Card */
		else {
			while(slot->cnt <= 1) {
				if (slot->bus_width)
					stat |= SDIDCON_WIDE_1;		/* SDIDAT[0:3] */
				else
					stat |= SDIDCON_WIDE_0;		/* SDIDAT[0] */
				slot->cnt = slot->cnt + 1;
			}
		}

		SDIBSIZE = cmd->data_len;

		/* FIFO Reset */
		SDICON |= SDICON_FRESET;

	#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
		SDIFSTA=SDIFSTA | SDIFSTA_FIFO_RESET;      // FIFO reset
	#endif
		SDIDCON = stat;
	} /* MMCSD_RES_FLAG_DATALINE */

	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	stat = SDICCON_START | cmd->cmd;
	if (cmd->res_type != MMCSD_RES_TYPE_NONE) {
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
		stat |=  SDICCON_WRSP;				/* Wait for Response */
		if (cmd->res_type == MMCSD_RES_TYPE_R2) {	// Response Type R2 : CID, CSD(CMD2, 9, 10)
			stat |= SDICCON_LRSP;			// Long Response
		}
	}
	
#if COMMAND_DUMP
{ /* for test */
	if (cmd->res_flag & MMCSD_RES_FLAG_RDATA)
		printk("###[%s:%d]->%s: SDICARG(0x%x) SDICCON(0x%x)\n", __FILE__, __LINE__, __FUNCTION__, cmd->arg, stat);
}
#endif
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	/* Send Command */
	SDICARG = cmd->arg;
	SDICCON = stat;
#if 1 /* shkim patch : 051219 */
	if (cmd->res_flag & MMCSD_RES_FLAG_RDATA) {
		SDIIMSK1 = SDIIMSK_TOUT | SDIIMSK_DFIN | SDIIMSK_RX_LAST |SDIIMSK_RX_FULL|SDIIMSK_RX_HALF; //hbahn
		interruptible_sleep_on(&wq);
	}
#endif

	//DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	/* Check Command Rspondse */	
	if (cmd->res_type != MMCSD_RES_TYPE_NONE){
		if (cmd->res_flag & MMCSD_RES_FLAG_DATALINE) {
			ret = read_data(slot, cmd->res, cmd->data_len);
			goto cmdexit;
		} else {
			while(1) {
				stat = SDICSTA;				/* Command Reponse State Check */
				if (!card_in) {
					SDICSTA = stat;
					return -ENODEV;
				}
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
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
				goto cmdexit;
			}

			if (cmd->res_flag & MMCSD_RES_FLAG_WDATA) {
	DPRINTK1("###[%s:%d]->%s:******\n", __FILE__, __LINE__, __FUNCTION__);
				ret = write_data(slot, cmd->data, cmd->data_len);
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
				goto cmdexit;
			}
		
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
			/* Command Response (SDIRSP0~3) save */
			if (mmcsd_get_res_len(cmd->res_type) == MMCSD_RES_LEN_LONG) {
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
				stat = SDIRSP0;
                                stat = cpu_to_be32(stat);
                                memcpy(cmd->res, &stat, sizeof(stat));
                                stat = SDIRSP1;
                                stat = cpu_to_be32(stat);
                                memcpy(cmd->res + (1 * sizeof(stat)),
                                       &stat, sizeof(stat));
                                stat = SDIRSP2;
                                stat = cpu_to_be32(stat);
                                memcpy(cmd->res + (2 * sizeof(stat)),
                                       &stat, sizeof(stat));
                                stat = SDIRSP3;
                                stat = cpu_to_be32(stat);
                                memcpy(cmd->res + (3 * sizeof(stat)),
                                       &stat, sizeof(stat));
			} 
			else {
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
				stat = SDIRSP0;
                                stat = cpu_to_be32(stat);
                                memcpy(cmd->res, &stat, sizeof(stat));
                                stat = SDIRSP1;
                                stat = cpu_to_be32(stat);
                                memcpy(cmd->res + (1 * sizeof(stat)),
                                       &stat, sizeof(stat));
			}
		}
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	} /* !MMCSD_RES_TYPE_NONE */
	
	else {
		do {
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
			stat = SDICSTA;
			if (!card_in) {
				SDICSTA = stat;
				return -ENODEV;
			}
		} while (!(stat & SDICSTA_SENT));
		SDICSTA = stat; /* clear bits */
	
	} /* MMCSD_RES_TYPE_NONE */
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	ret = 0;
 cmdexit:
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
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
	slot->card_in = card_in;
#if defined(CONFIG_MACH_MMSP2_DTK3)
	wp = 0;
#elif defined(CONFIG_MACH_MMSP2_DTK4) || defined(CONFIG_MACH_MMSP2_WDMS)
	wp = read_gpio_bit(GPIO_D2);
#endif

	slot->readonly = 0;

	add_mmcsd_device(slot);

#if 1
	sys_mount("/dev/mmcsd/disc0/part1", "/mnt/sd", "vfat", 0xC0ED0000|MS_NOATIME|MS_SYNCHRONOUS, "iocharset=utf8");
#endif

}


static void del_task_handler(void *data)
{
	struct mmcsd_slot *slot = (struct mmcsd_slot *)data;
	slot->card_in = card_in; 		/* 이걸 해야 하나 */
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
	if(!(CDINTPNR & SD_CDPN)) {
		return;
	}
	else {
		empty = 0;
		/* Card Detect Interrupt Pending Register */
		 CDINTPNR |= SD_CDPN_CL;	/* SD Card Detect Pending Clear */
	}
#endif
	if (!card_in && !empty) {
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
		/* card inserted */
		printk("MMC/SD Card Detected\n");
		card_in = 1;
		mdelay(100);
		card_detect_task.data = (void *)dev_id;
		card_detect_task.routine = add_task_handler;
		schedule_task(&card_detect_task);
	}
	else if (card_in && empty) {
		/* card ejected */
		printk("MMC/SD card ejected\n");
		card_in = 0;

#ifdef USE_INTERRUPT
		DPRINTK1("###[%s:%d]->%s:wake_up\n", __FILE__, __LINE__, __FUNCTION__);
		wake_up_interruptible(&wq);
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
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	/* Initialize status variables */
	card_in = 0;

	/* set GPIO to MMC/SD function */
	set_gpio_ctrl(GPIO_L0, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[0] */
	set_gpio_ctrl(GPIO_L1, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[1] */
	set_gpio_ctrl(GPIO_L2, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[2] */
	set_gpio_ctrl(GPIO_L3, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[3] */
	set_gpio_ctrl(GPIO_L4, GPIOMD_ALT1, GPIOPU_EN);		/* SDICMD */
	set_gpio_ctrl(GPIO_L5, GPIOMD_ALT1, GPIOPU_EN);		/* SDICLK */
	
#if defined(CONFIG_MACH_MMSP2_DTK4) || defined(CONFIG_MACH_MMSP2_WDMS)
	set_gpio_ctrl(GPIO_D2, GPIOMD_IN, GPIOPU_DIS);	/* SDIWP */
#endif
#ifdef CONFIG_MACH_MMSP2_DTK4
	set_gpio_ctrl(GPIO_I14, GPIOMD_IN, GPIOPU_EN);	/* SDICD */
#elif CONFIG_MACH_MMSP2_WDMS
	set_gpio_ctrl(GPIO_D3, GPIOMD_IN, GPIOPU_EN);	/* SDICD */
#endif
	/* Initialize MMC/SD Controller Register */
	init_mmcsd_reg();

#ifdef NO_CD_IRQ
	init_timer(&cd_timer);
	cd_timer.function = cd_timer_handler;
#else
	/* Card Detect Interrupt pin set */
	set_external_irq(IRQ_CD, EINT_HIGH_LEVEL, GPIOPU_NOSET);

	/* Card Detect Interrupt Enable Register*/
	//CDINTENR &= SD_CD_DIS;		/* SD 	  Card Detect Interrupt -> Disable   *
	CDINTENR |= SD_CD_EN;		/* SD 	  Card Detect Interrupt -> Enable   */
	CDINTENR &= PCMCIA_CD_DIS;	/* PCMCIA Card Detect Interrupt -> Disable  */
	CDINTENR &= CF_CD_DIS;		/* CF	  Card Detect Interrupt -> Disable  */

	/* Card Detect Interrupt Register */	
	CDINTMDR &= SD_CDMD_L;		/* SD Card Detect Interrupt -> Low Level */

	/* Card Detect Interrupt Pending Register */
	 CDINTPNR = SD_CDPN_CL | PCMCIA_CDPN_CL | CF_CDPN_CL;		/* All Pending Clear */
	
	/* Register IRQ handlers */
	ret = request_irq(IRQ_CD, card_detect_interrupt, SA_INTERRUPT, "CD", (void *)&slot);
	if (ret) {
		printk("MMC/SD Slot: request_irq(SD CD) failed\n");
		goto err1;
	}
#endif

#ifdef USE_INTERRUPT
	ret = request_irq(IRQ_SD, sdi_interrupt, SA_INTERRUPT, "SD", (void *)&slot);
	if (ret) {
		printk("MMC/SD Slot: request_irq(SD) failed\n");
		goto err2;
	}
#endif
	printk("MMC/SD Slot initialized\n");

#ifdef NO_CD_IRQ
	mod_timer(&cd_timer, jiffies + 10);
#else
	card_detect_interrupt(IRQ_CD, (void *)&slot, NULL);
#endif


	return 0;
#ifdef USE_INTERRUPT
 err2:
#ifndef NO_CD_IRQ
	free_irq(IRQ_CD, (void *)&slot);
#endif
#endif
 err1:
	return ret;
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
MODULE_DESCRIPTION("MMC/SD slot interfaces specific to MMSP2 MMC/SD Controller");
