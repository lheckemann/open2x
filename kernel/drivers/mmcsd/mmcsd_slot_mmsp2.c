/*
 * drivers/mmcsd/mmcsd_slot_mmsp2.c
 *
 * Copyright (C) 2005,2006 Gamepark Holdings, Inc. (www.gp2x.com)
 * Hyun <hyun3678@gp2x.com>
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

#include <linux/blkdev.h>

#include "mmcsd.h"
#include "mmcsd_slot_mmsp2.h"

#ifndef NO_CD_IRQ
#define IRQ_SDCD	IRQ_GPIO_I(14)
static int card_detect_level, boottime = 1;
#endif

static int errflag=0;

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
static int read_data(struct mmcsd_slot *slot, u_char *buf, int len,u_long nSectors)
{
#ifdef USE_POLLING
	int cnt = 0;
	long tSize;
	u32 stat;
	int ff_cnt, i;

	tSize=len * nSectors;
	while(cnt < tSize )
	{
		stat = SDIDSTA;
		if(SDIDSTA & SDIDSTA_TOUT) 		// Check timeout
		{
			SDIDSTA= SDIDSTA_TOUT;  	// Clear timeout flag
			printk("Read Timeout\n");
			break;
		}

		stat = SDIFSTA;
		if(stat & SDIFSTA_RX)     // Is Rx data?
		{
			buf[cnt]= (u8)SDIDAT;
			cnt++;
		}
	}

	/* Chk_DATend */
	do
	{
		stat = SDIDSTA;
		if (!card_in)
		{
			SDIDSTA = stat;
			return -ENODEV;
		}

	} while (!(stat & SDIDSTA_TOUT) && !(stat & SDIDSTA_DFIN)); 		/* time out , data transfer complete check */

	SDIDSTA = stat; /* clear */
	return 0;

#elif defined(USE_INTERRUPT)

	int cnt = 0;
	u32 stat;
	long tSize;
	int ff_cnt, i;


	tSize=len * nSectors;
	while(cnt < tSize )
	{
		stat = SDIDSTA;
		if(SDIDSTA & SDIDSTA_TOUT) // Check timeout
		{
			SDIDSTA= SDIDSTA_TOUT;  // Clear timeout flag
			printk("Read Time out :%d\n",cnt);
			break;
		}

		stat = SDIFSTA;
		if(stat & SDIFSTA_RX)     // Is Rx data?
		{
			ff_cnt =  stat & SDIFSTA_CNT;
			for(i=0; i<ff_cnt; i++)
			{
				buf[cnt]= (u8)SDIDAT;
				cnt++;
			}
		}
	}

	do
	{
		stat = SDIDSTA;
		if (!card_in)
		{
			SDIDSTA = stat;
			return -ENODEV;
		}

	} while (!(stat & SDIDSTA_TOUT) && !(stat & SDIDSTA_DFIN)); /* time out , data transfer complete check */

	SDIDSTA = stat; /* clear */
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
static int write_data(struct mmcsd_slot *slot, const u_char *buf, int len,u_long nSectors)
{
	int cnt = 0;
	long tSize;
	u32 stat;
	int ff_cnt,i;
	unsigned long timeCnt=0;

	tSize=len * nSectors;
	while(cnt< tSize)
	{
		stat=SDIFSTA;
		if(stat & 0x800)
		{
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
		/********************************************/
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];

			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];

			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];

			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];

			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];

			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];

			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
			(u8)SDIDAT=buf[cnt++];
		}
	}

	/* Check of end data */
	do {
		stat = SDIDSTA;
		if (!card_in)
		{
			SDIDSTA = stat;
			return -ENODEV;
		}
		if (stat & SDIDSTA_TOUT)
		{
			SDIDSTA = stat; /* clear bits */
			return -ETIMEDOUT;
		}
	} while (!(stat & SDIDSTA_DFIN)); 	/* data transfer complete check */

	SDIDSTA = stat; /* clear */

	return 0;
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
	unsigned long timeCnt=0;


	/**************** Read data *************************************************************************/
	if (cmd->res_flag & MMCSD_RES_FLAG_RDATA)
	{

#ifdef USE_INTERRUPT
		reading = 1;
		buffer = cmd->data;							/* mmcsd_slot_mmsp2.h */
		bufcnt = 0;
		datalen = cmd->data_len;
		error = 0;
#endif
#if defined(CONFIG_S3C2410) || defined(CONFIG_S3C2440)
		stat =SDIDCON_DATA_SIZE_WORD | SDIDCON_DATA_START;
#endif

		stat = SDIDCON_RACMD_1 | SDIDCON_BLK | SDIDCON_DATA_MODE_Rx | cmd->nSectors << 0;

		/******* Set Data Bus *******/
		/* SD Card */
		if(slot->sd)
		{
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

	} /*MMCSD_RES_FLAG_RDATA */

	/**************** Write data **********************************************************************/
	 else if (cmd->res_flag & MMCSD_RES_FLAG_WDATA)
	 {
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

		stat = SDIDCON_TARSP_1 | SDIDCON_BLK |SDIDCON_DATA_MODE_Tx | cmd->nSectors << 0;

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
	} /*MMCSD_RES_FLAG_WDATA */

	/**************** Data line use *******************************************/
	 else if (cmd->res_flag & MMCSD_RES_FLAG_DATALINE)
	 {
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

	stat = SDICCON_START | cmd->cmd;
	if (cmd->res_type != MMCSD_RES_TYPE_NONE)
	{
		stat |=  SDICCON_WRSP;						/* Wait for Response */
		if (cmd->res_type == MMCSD_RES_TYPE_R2) 	// Response Type R2 : CID, CSD(CMD2, 9, 10)
		{
			stat |= SDICCON_LRSP;					// Long Response
		}
	}

	SDICARG = cmd->arg;
	SDICCON = stat;

#ifdef USE_INTERRUPT 	/* shkim patch : 051219 */
	if (cmd->res_flag & MMCSD_RES_FLAG_RDATA)
	{

		SDIIMSK1 = SDIIMSK_TOUT | SDIIMSK_DFIN | SDIIMSK_RX_LAST |SDIIMSK_RX_FULL|SDIIMSK_RX_HALF; //hbahn
		//interruptible_sleep_on(&wq);
		interruptible_sleep_on_timeout(&wq, 1);
	}
#endif

	/* Check Command Rspondse */
	if (cmd->res_type != MMCSD_RES_TYPE_NONE)
	{
		if (cmd->res_flag & MMCSD_RES_FLAG_DATALINE)
		{
			cmd->nSectors=1;
			ret = read_data(slot, cmd->res, cmd->data_len, cmd->nSectors);

			goto cmdexit;
		}
		else
		{
			while(1)
			{
				stat = SDICSTA;				/* Command Reponse State Check */
				if (!card_in)
				{
					SDICSTA = stat;
					return -ENODEV;
				}
				if (stat & SDICSTA_TOUT) 	/* Time out error */
				{
					ret = -ETIMEDOUT;
					SDICSTA = stat; /* clear bits */
					goto cmdexit;
				}
				if (stat & SDICSTA_RSP) break;		/* Command Response received */
			}

			if (!(cmd->res_flag & MMCSD_RES_FLAG_NOCRC))
			{
				if ((stat & SDICSTA_ALLFLAG) != (SDICSTA_SENT | SDICSTA_RSP))
				{
					ret = -EIO;
					SDICSTA = stat; /* clear bits */
					goto cmdexit;
				}
			}
			SDICSTA = stat; /* clear bits */

			if (cmd->res_flag & MMCSD_RES_FLAG_RDATA)
			{
				ret = read_data(slot, cmd->data, cmd->data_len, cmd->nSectors);

				if(ret)
				{
					printk("Read error => cmd->nSectors:%d\n",cmd->nSectors);
					return ret;	/* Read  ERROR */
				}

				if( cmd->nSectors >1)
				{
					/* MULT BLOCK STOP COMMAND */
					SDIDCON = SDIDCON_BACMD | SDIDCON_BLK | SDIDCON_DATA_MODE_BUSY  | (cmd->nSectors << 0);
					SDICARG = 0x0;
					SDICCON = SDICCON_WRSP | SDICCON_START | MMCSD_CMD12;

					while(1)
					{
						stat = SDICSTA;							/* Command Reponse State Check */
						if (!card_in)
						{
							SDICSTA = stat;
							return -ENODEV;
						}
						if (stat & SDICSTA_TOUT)
						{
							ret = -ETIMEDOUT;
							SDICSTA = stat; /* clear bits */
							goto cmdexit;
						}
						if (stat & SDICSTA_RSP) break;			/* Command Response received */
					}

					if (!(cmd->res_flag & MMCSD_RES_FLAG_NOCRC))
					{
							/* CRC no check */
						if ((stat & SDICSTA_ALLFLAG) != (SDICSTA_SENT | SDICSTA_RSP))
						{
							ret = -EIO;
							SDICSTA = stat; /* clear bits */
							goto cmdexit;
						}
					}
					SDICSTA = stat; /* clear bits */


					do
					{
						stat = SDIDSTA;

						if (!card_in)
						{
							SDIDSTA = stat;
							return -ENODEV;
						}
						if (stat & SDIDSTA_TOUT)
						{
							ret = -ETIMEDOUT;
							SDIDSTA = stat;
							printk("BUSY TIME_OUT\n");
							goto cmdexit;
						}

					}while (!(stat & SDIDSTA_BFIN));

					SDIDSTA = stat;


				}
				return 0;
			}

			if (cmd->res_flag & MMCSD_RES_FLAG_WDATA)
			{
				ret = write_data(slot, cmd->data, cmd->data_len, cmd->nSectors);
				if(ret) return ret;	/* WRITE ERROR */

				if ( cmd->nSectors >1)
				{	/* MULT BLOCK STOP COMMAND */
					SDIDCON = SDIDCON_BACMD | SDIDCON_BLK | SDIDCON_DATA_MODE_BUSY  | (cmd->nSectors << 0);
					SDICARG = 0x0;
					SDICCON = SDICCON_WRSP | SDICCON_START | MMCSD_CMD12;

					while(1)
					{
						stat = SDICSTA;								/* Command Reponse State Check */
						if (!card_in)
						{
							SDICSTA = stat;
							return -ENODEV;
						}
						if (stat & SDICSTA_TOUT)
						{
							ret = -ETIMEDOUT;
							SDICSTA = stat; /* clear bits */
							goto cmdexit;
						}
						if (stat & SDICSTA_RSP) break;			/* Command Response received */
					}

					SDICSTA = stat; /* clear bits  */

					/* CRC CHECK */
					if (!(cmd->res_flag & MMCSD_RES_FLAG_NOCRC))
					{
						if ((stat & SDICSTA_ALLFLAG) != (SDICSTA_SENT | SDICSTA_RSP))
						{
							ret = -EIO;
							SDICSTA = stat; /* clear bits */
							goto cmdexit;
						}
					}


#if 0				/************************************/
					timeCnt=0;
					do
					{
						stat = SDIDSTA;
						if (!card_in)
						{
							SDIDSTA = stat;
							return -ENODEV;
						}

						if (stat & SDIDSTA_TOUT)
						{
							ret = -ETIMEDOUT;
							SDIDSTA = stat;
							goto cmdexit;
						}
						timeCnt++;
					}while (!(stat & SDIDSTA_BFIN));

					SDIDSTA = stat;


#endif
				}

				return 0;
			}


			/* Command Response (SDIRSP0~3) save */
			if (mmcsd_get_res_len(cmd->res_type) == MMCSD_RES_LEN_LONG)
			{
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
			else
			{
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
	else
	{
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
	slot->card_in = card_in;
#if defined(CONFIG_MACH_MMSP2_DTK3)
	wp = 0;
#elif defined(CONFIG_MACH_MMSP2_DTK4) || defined(CONFIG_MACH_MMSP2_WDMS)
	wp = read_gpio_bit(GPIO_D2);
#endif

	slot->readonly = 0;

	if(errflag)
	{
		printk("error is again\n");
		errflag=0;
		set_gpio_ctrl(GPIO_L0, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[0] */
		set_gpio_ctrl(GPIO_L1, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[1] */
		set_gpio_ctrl(GPIO_L2, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[2] */
		set_gpio_ctrl(GPIO_L3, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[3] */
		set_gpio_ctrl(GPIO_L4, GPIOMD_ALT1, GPIOPU_EN);		/* SDICMD */
		set_gpio_ctrl(GPIO_L5, GPIOMD_ALT1, GPIOPU_EN);		/* SDICLK */

		init_mmcsd_reg();
	}

	add_mmcsd_device(slot);

	printk("mount...1 \n");
#if 1
	sys_mount("/dev/mmcsd/disc0/part1", "/mnt/sd", "vfat", 0xC0ED0000|MS_NOATIME|MS_SYNCHRONOUS, "iocharset=utf8");
#endif
	printk("mount...2: \n");


#if 1
	if(read_gpio_bit(GPIO_I14))
	{
		printk("mount... error\n");
		set_external_irq(IRQ_SDCD, EINT_LOW_LEVEL, GPIOPU_NOSET);
		card_detect_level = 0;
		card_in = 0;
		errflag=1;

	}
#endif

}


static void del_task_handler(void *data)
{
	struct mmcsd_slot *slot = (struct mmcsd_slot *)data;
	slot->card_in = card_in; 		/* 이걸 해야 하나 */
	printk("del...1\n");
	del_mmcsd_device(slot);
	printk("del...2\n");
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
#ifdef NO_CD_IRQ
static void card_detect_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	int empty;

	if(!read_gpio_bit(GPIO_I14))
		empty = 0;
	else
		empty = 1;

	if (!card_in && !empty)
	{
		/* card inserted */
		printk("MMC/SD Card Detected\n");
		card_in = 1;
		mdelay(100);
		card_detect_task.data = (void *)dev_id;
		card_detect_task.routine = add_task_handler;
		schedule_task(&card_detect_task);
	}
	else if (card_in && empty)
	{
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
#else

static void card_detect_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	int empty;

	if(!read_gpio_bit(GPIO_I14))
		empty = 0;
	else
		empty = 1;


    if (!card_in && !empty)
    {
#if 0
 		card_detect_level = 1;
    	set_external_irq(IRQ_SDCD, EINT_HIGH_LEVEL, GPIOPU_NOSET);
       	printk("MMC/SD Card Detected\n");
        card_in = 1;
        mdelay(100);
        card_detect_task.data = (void *) dev_id;
        card_detect_task.routine = add_task_handler;
        schedule_task(&card_detect_task);
#else
		mdelay(500);
		if(read_gpio_bit(GPIO_I14)) return;   //Exit is error
		else{
			card_detect_level = 1;
    		set_external_irq(IRQ_SDCD, EINT_HIGH_LEVEL, GPIOPU_NOSET);
       		printk("MMC/SD Card Detected\n");
        	card_in = 1;
        	mdelay(20);
        	card_detect_task.data = (void *) dev_id;
        	card_detect_task.routine = add_task_handler;
        	schedule_task(&card_detect_task);
		}

#endif

    }
    else if (card_in && empty) 		// card_in == 1, empty == 1
    {
       	card_detect_level = 0;
    	set_external_irq(IRQ_SDCD, EINT_LOW_LEVEL, GPIOPU_NOSET);
        printk("MMC/SD card ejected\n");
        card_in = 0;

        DPRINTK1("###[%s:%d]->%s:wake_up\n", __FILE__, __LINE__, __FUNCTION__);
        wake_up_interruptible(&wq);
        card_detect_task.data = (void *) dev_id;
        card_detect_task.routine = del_task_handler;
        schedule_task(&card_detect_task);
    }

}
#endif



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

	/* set GPIO to MMC/SD function */
	set_gpio_ctrl(GPIO_L0, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[0] */
	set_gpio_ctrl(GPIO_L1, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[1] */
	set_gpio_ctrl(GPIO_L2, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[2] */
	set_gpio_ctrl(GPIO_L3, GPIOMD_ALT1, GPIOPU_EN);		/* SDIDATA[3] */
	set_gpio_ctrl(GPIO_L4, GPIOMD_ALT1, GPIOPU_EN);		/* SDICMD */
	set_gpio_ctrl(GPIO_L5, GPIOMD_ALT1, GPIOPU_EN);		/* SDICLK */

	/* Initialize MMC/SD Controller Register */
	init_mmcsd_reg();

#ifdef NO_CD_IRQ
	set_gpio_ctrl(GPIO_I14, GPIOMD_IN, GPIOPU_EN);		/* SDICD */

	/*  NO_CD_IRQ */
	init_timer(&cd_timer);
	cd_timer.function = cd_timer_handler;
#else
	card_detect_level = 0; // first skip low
    set_external_irq(IRQ_SDCD, EINT_LOW_LEVEL, GPIOPU_NOSET);

#endif

#ifdef USE_INTERRUPT
	ret = request_irq(IRQ_SD, sdi_interrupt, SA_INTERRUPT, "SD", (void *)&slot);
	if (ret) {
		printk("MMC/SD Slot: request_irq(SD) failed\n");
		return ret;
	}
#endif


#ifdef NO_CD_IRQ
	mod_timer(&cd_timer, jiffies + 10);
#else
	ret = request_irq(IRQ_SDCD, card_detect_interrupt, SA_INTERRUPT, "CD",
                    (void *) &slot);
#endif


    if (ret)
    {
        printk("MMC/SD Slot: request_irq(SD CD) failed\n");
#ifdef USE_INTERRUPT
		free_irq(IRQ_SD, (void *)&slot);
#endif
		return ret;
    }


	printk("MMC/SD Slot initialized\n");

	return ret;
}

static void __exit exit_mmcsd_slot(void)
{
#ifdef USE_INTERRUPT
	free_irq(IRQ_SD, (void *)&slot);
#endif
#ifndef NO_CD_IRQ
	free_irq(IRQ_SDCD, (void *)&slot);
#endif

	if (card_in)
	{
		card_in = 0;
		del_task_handler((void *)&slot);
	}
}

module_init(init_mmcsd_slot);
module_exit(exit_mmcsd_slot);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
MODULE_DESCRIPTION("MMC/SD slot interfaces specific to MMSP2 MMC/SD Controller");
