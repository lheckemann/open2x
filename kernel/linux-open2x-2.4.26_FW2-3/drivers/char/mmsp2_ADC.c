/*
 * drivers/char/mmsp2_ADC.c
 *
 * Copyright (C) 2007 Gamepark Holdings, Inc. (www.gp2x.com)
 * Hyun <hyun3678@gp2x.com>
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/arch/proto_gpio.h>
#include <asm/arch/mmsp20.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <asm/ioctl.h>
#include <asm/unistd.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/ioport.h>
#include <linux/slab.h>     // kmalloc()
#include <linux/poll.h>     // poll
#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/miscdevice.h>

#define TPC_MAN_READ_U1		3
#define TPC_BUSY			1
#define TPC_IDLE			0
#define ADCCLK				(1<<13)

/******    IOCTL COMMAND   ************/
/* Select ADC */
#define BATT_MODE			0
#define REMOCON_MODE		1
#define BATT_REMOCON_MODE	2

/* Gp2x running mode*/
#define MENU_MODE			10
#define MP3_MODE			11
#define TEXT_VIEW_MODE		12
#define IMAGE_VIEW_MODE 	13
#define MP3_TEXT_MODE		14
#define MP3_IMAGE_MODE		15
#define MOVIE_MODE			16
#define GAME_MODE			17
#define CALL_REMOCON		20

/* ********** ISOUND EARPHONE REMOCON    */
#define NONE_REMOCON	0
#define EQ				1
#define VOLUME_UP		2
#define VOLUME_DOWN		3
#define PLAY_PAUSE		4
#define REW				5
#define FF				6
#define NEXT_SKIP		7
#define REMOVE_REMOCON	8


#define DECT_RECOMON_MIN	0x315			/* 2.6V_min	  */
#define DECT_RECOMON_MAX	0x350			/* 2.6V_max	  */

#define ZONE_LIMIT				0x2EA
#define VOLUME_UP_LIMIT			0x2C5//0x2BA			/* 0x2CA~ 0x2d9       */
#define VOLUME_UP_MAX_LIMIT		0x2DC //0x2E3			/* 0x2CA~ 0x2d9       */

#define VOLUME_DOWN_LIMIT		0x267			/* 0x26E ~ 0x27F     */
#define VOLUME_DOWN_MAX_LIMIT	0x27B			/* 0x26E ~ 0x27F     */

#define EQ_LIMIT				0x1FA			/* 0x1fA ~ 0x20A     */
#define EQ_MAX_LIMIT			0x20D			/* 0x1fA ~ 0x20A     */

#define FF_LIMIT				0x193			/* 0x195 ~ 0x1a0	 */
#define FF_MAX_LIMIT			0x1A6			/* 0x195 ~ 0x1a0	 */

#define PLAY_PAUSE_LIMIT		0x130			/* 0x130 ~ 0x140     */
#define PLAY_PAUSE_MAX_LIMIT	0x140			/* 0x130 ~ 0x140     */

#define REW_LIMIT				0xd5  			//0xCC			/* 0xd5 ~  0xC5		 */
#define REW_MAX_LIMIT			0xE5 			//0xEA			/* 0xd5 ~  0xC5		 */

#define NEXT_SKIP_LIMIT				0x6A			/* 0x75 ~  0x85		*/
#define NEXT_SKIP_MAX_LIMIT			0x8A			/* 0x75 ~  0x85		*/

#define CHK_REMCOCON_LOW_LIMIT		0x60
#define CHK_REMCOCON_HIGH_LIMIT		0x350

/* **** POWER SUPPLY 3V ~ 2V RANGE menu mode  *****
	input 3V:0x3b5   ~~~ 0x3bf
	input 2.8V:0x365 ~~~ 0x36f
	input 2.4V:0x31b ~~~ 0x31E
	input 2V:2B0     ~~~ 0x2af
************************************************** */
#define BATT_HIGH_LIMIT		0x340
#define BATT_MID_LIMIT		0x2D0	//0x2B0
#define BATT_LOW_LIMIT		0x260	//0x260

#define BATT_LEVEL_HIGH				0
#define BATT_LEVEL_MID				1
#define BATT_LEVEL_LOW				2
#define BATT_LEVEL_EMPTY			3
#define BATT_LEVEL_OLD_STATUS		4

static unsigned char PosAdc=0;
static unsigned char inc_val=0;
static unsigned char calRemoconFlag;

typedef struct {
	unsigned short batt;
	unsigned short remocon;
} MMSP2ADC;

static unsigned short
TPC_SwitchConvert(unsigned long SW)
{
	unsigned short value;
	value = 0;

	if( (SW & TPC_XP_SW) == TPC_XP_SW ) value = (unsigned char)(value & ~(TPC_XP_SW));
	else				value = (unsigned char)(TPC_XP_SW);

	if( (SW & TPC_YP_SW) == TPC_YP_SW ) value = (unsigned char)(value & ~(TPC_YP_SW));
	else				value = (unsigned char)(value | TPC_YP_SW);

	if( (SW & TPC_XM_SW) == TPC_XM_SW ) value = (unsigned char)(value | TPC_XM_SW);
	else				value = (unsigned char)(value & ~(TPC_XM_SW));

	if( (SW & TPC_YM_SW) == TPC_YM_SW ) value = (unsigned char)(value | TPC_YM_SW);
	else				value = (unsigned char)(value & ~(TPC_YM_SW));

	if( (SW & TPC_PU_SW) == TPC_PU_SW ) value = (unsigned char)(value & ~(TPC_PU_SW));
	else				value = (unsigned char)(value | TPC_PU_SW);

	return value;
}

static void
TPC_SetDefaultSW(unsigned long SW)
{
	TPC_U1_VALUE = (unsigned short)(TPC_SwitchConvert(SW) << 11);
}

static void
TPC_Run(void)
{
	unsigned short Mask = (1<<15) | (1<<14) | (1<<4);
	TPC_CNTL = (unsigned short)( TPC_CNTL | Mask );
}

static void
TPC_Stop(void)
{
	TPC_CNTL = 0 ;
}

static int
TPC_IsTouchBusy(void)
{
	int IsBusy;
	if( TPC_CNTL & 1 ) IsBusy = 1;
	else		IsBusy = 0;
	return IsBusy;
}

static void
TPC_SetChannelEnb(unsigned long ChannelEnb)
{
	TPC_CNTL = (unsigned short)(((unsigned long)TPC_CNTL & ~(0x1E00)) | ChannelEnb);
}

static void
TPC_SetADCClock(unsigned long APSV)
{
	TPC_CLK_CNTL = (unsigned short)( (TPC_CLK_CNTL & 0xff00UL) | APSV );
}

static void
TPC_SetADCParam
(
	unsigned long BandGapTime,
	unsigned long CaptureIdleTime,
	unsigned long SwitchSetupTime,
	unsigned long DecimationRatio,
	unsigned long ADCConversionTime
)
{
	TPC_TIME_PARM1 = (unsigned short)((DecimationRatio<<8) | SwitchSetupTime);
	TPC_TIME_PARM2 = (unsigned short)((BandGapTime<<8) | CaptureIdleTime);
	TPC_TIME_PARM3 = (unsigned short)(ADCConversionTime);
}

static void
TPC_InitHardware (void)
{
	// All Power Down
	TPC_CNTL = ~(1<<6);
	// off all switches
	TPC_SetDefaultSW(0); 		/* u1 buffer switch all off */
	TPC_SetADCClock( 255 );
	TPC_SetADCParam ( 0,		// BandGapTime
			127,		// CaptureIdleTime
			127,		// SwitchSetupTime
			5,		// DecimationRatio
			4 );		// ADCConversionTime
}

void
TPC_SetU1In
(
	unsigned long U1Channel,
	unsigned long Thres,
	int IsGreat
)
{
	TPC_CH_SEL = (unsigned long)(( TPC_CH_SEL & ~(0x0007UL<<9) ) | (U1Channel<<9));
	TPC_COMP_U1 = (unsigned long)(((unsigned long)IsGreat<<10) | Thres);
}

void
TPC_SetU1ADC(unsigned long ADCChannelNum)
{
	unsigned long ChSel;
	ChSel = TPC_CH_SEL;
	ChSel = (unsigned short)(ChSel & ~(0x7 << 9));
	TPC_CH_SEL = (unsigned short)(ChSel | (ADCChannelNum << 9));
}

static void
InitTouch (void)
{
	TPC_InitHardware();

	TPC_SetADCClock( 255 );

	TPC_SetADCParam ( 0,	// BandGapTime
			255,	// CaptureIdleTime
			255,	// SwitchSetupTime
			5,	// DecimationRatio
			4 );	// ADCConversionTime

	TPC_SetDefaultSW ( TPC_PU_SW | TPC_YM_SW );
}


static unsigned short
TPC_ManReadChannel(void)
{
	unsigned short ReadValue;

	TPC_Run();

	do { } while( ( TPC_CNTL & 1 ) == (unsigned short)TPC_BUSY );

	ReadValue  = (unsigned short)(TPC_U1_VALUE & 0x3ff);

	TPC_SetChannelEnb(0);
	TPC_Stop();

	return ReadValue;
}


static unsigned short
TPC_GetStaticADCOut(unsigned long ChannelNum)
{
	unsigned short ADCOut;
	TPC_Stop();
	TPC_SetU1ADC ( ChannelNum );
	udelay(5);
	TPC_SetChannelEnb( TPC_U1_CH_EN );
	udelay(5);
	TPC_Run();

	while ( !TPC_IsTouchBusy() );
	while ( TPC_IsTouchBusy() );

	ADCOut = TPC_ManReadChannel();

	return ADCOut;
}

int mmsp2ADC_open(struct inode *inode, struct file *filp)
{
	MOD_INC_USE_COUNT;
	calRemoconFlag = 0;
	return (0);          /* success */
}

int mmsp2ADC_release(struct inode *inode, struct file *filp)
{
	MOD_DEC_USE_COUNT;
	return (0);
}

static ssize_t mmsp2ADC_read(struct file *filp, char *Putbuf, size_t length, loff_t *f_pos)
{
	MMSP2ADC adcVal;
	unsigned short tempval;
	unsigned char cmode,cmode_old;
	int cnt=0,ccnt=0;

	TPC_SetDefaultSW( 0 );
	udelay(20);  //5

	switch(PosAdc)
	{
		case BATT_MODE:
		case BATT_REMOCON_MODE:
			tempval = TPC_GetStaticADCOut( 0 ) + inc_val;

			if( tempval > BATT_HIGH_LIMIT) cmode_old = BATT_LEVEL_HIGH;
			else if( tempval > BATT_MID_LIMIT) cmode_old = BATT_LEVEL_MID;
			else if( tempval > BATT_LOW_LIMIT) cmode_old = BATT_LEVEL_LOW;
			else cmode_old = BATT_LEVEL_EMPTY;

			while(cnt++ < 10)
			{
				TPC_SetDefaultSW( 0 );
				udelay(20);
				tempval = TPC_GetStaticADCOut( 0 ) + inc_val;

				if( tempval > BATT_HIGH_LIMIT) cmode = BATT_LEVEL_HIGH;
				else if( tempval > BATT_MID_LIMIT) cmode = BATT_LEVEL_MID;
				else if( tempval > BATT_LOW_LIMIT) cmode = BATT_LEVEL_LOW;
				else cmode = BATT_LEVEL_EMPTY;

				if(ccnt == 4){
					adcVal.batt = cmode;
					break;
				}
				else
				{
					if(cmode == cmode_old) ccnt++;
					else ccnt=0;
				}

				adcVal.batt = BATT_LEVEL_OLD_STATUS;
				cmode_old = cmode;
			}
			if (PosAdc == BATT_MODE) break;
		case REMOCON_MODE:
#ifdef CONFIG_MACH_GP2XF200
			if(calRemoconFlag)
			{
				tempval = TPC_GetStaticADCOut( 1 );
				if( (tempval < CHK_REMCOCON_LOW_LIMIT) || (tempval > CHK_REMCOCON_HIGH_LIMIT) )
				{
					printk("Remove remocon\n");
					calRemoconFlag=0;
					adcVal.remocon = REMOVE_REMOCON;
				}
				else if(tempval > ZONE_LIMIT )  adcVal.remocon = NONE_REMOCON;
				else if( (tempval > VOLUME_UP_LIMIT) && (tempval < VOLUME_UP_MAX_LIMIT) )  adcVal.remocon = VOLUME_UP;
				else if( (tempval > VOLUME_DOWN_LIMIT)&&(tempval < VOLUME_DOWN_MAX_LIMIT) ) adcVal.remocon = VOLUME_DOWN;
				else if( (tempval > EQ_LIMIT) && (tempval < EQ_MAX_LIMIT) ) adcVal.remocon = EQ;
				else if( (tempval > FF_LIMIT) && (tempval < FF_MAX_LIMIT) ) adcVal.remocon = FF;
				else if( (tempval > PLAY_PAUSE_LIMIT) && (tempval < PLAY_PAUSE_MAX_LIMIT) ) adcVal.remocon = PLAY_PAUSE;
				else if( (tempval > REW_LIMIT) && (tempval < REW_MAX_LIMIT) ) adcVal.remocon = REW;
				else if( (tempval > NEXT_SKIP_LIMIT) && (tempval < NEXT_SKIP_MAX_LIMIT) ) adcVal.remocon = NEXT_SKIP;
				else adcVal.remocon = NONE_REMOCON;
			} else
#endif
				adcVal.remocon = REMOVE_REMOCON;;
			break;
	}

	TPC_SetDefaultSW ( TPC_PU_SW );

	if(copy_to_user(Putbuf, &adcVal, sizeof(MMSP2ADC)))
	{
		return -EFAULT;
	}

	return sizeof(MMSP2ADC);
}

int mmsp2ADC_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,unsigned long arg)
{
	unsigned short tempval;
	int calRemoconCnt=10;
	int chkcnt=0,noIncnt=0;


	switch(arg)
	{
		case BATT_MODE:
			PosAdc=BATT_MODE;
			break;
		case REMOCON_MODE:
			PosAdc=REMOCON_MODE;
			break;
		case BATT_REMOCON_MODE:
			PosAdc=BATT_REMOCON_MODE;
			break;
		case MOVIE_MODE:
			inc_val=0x22;
			break;
		case GAME_MODE:
			inc_val=0x10;
			break;
		case CALL_REMOCON:
#ifndef CONFIG_MACH_GP2XF200
			return 0;   /* Not support is f100 */
#endif
			if(!calRemoconFlag){   			/* Only Mp3 mode */
				while(calRemoconCnt--){
					TPC_SetDefaultSW(0);
					udelay(20);
					tempval = TPC_GetStaticADCOut( 1 );
					if( (tempval > DECT_RECOMON_MIN) && (tempval < DECT_RECOMON_MAX) ){
						if(chkcnt==5){
							printk("insert ear remocon\n");
							calRemoconFlag=1;
							return 1;
						}
						chkcnt++;
					}
					else chkcnt=0;

					udelay(20);
				}

			}else{
				return 1;
			}
	}
	return 0;
}

static struct file_operations mmsp2ADC_fops = {
	owner:      THIS_MODULE,
	open:       mmsp2ADC_open,
	read:       mmsp2ADC_read,
	ioctl:	    mmsp2ADC_ioctl,
	release:    mmsp2ADC_release,
};

static struct miscdevice mmsp2ADC = {
    MMSP2ADC_MINOR, "mmsp2adc", &mmsp2ADC_fops
};

static int __init mmsp2ADC_init(void)
{
	int i;
	PosAdc = BATT_MODE;
	TPC_SetDefaultSW(0);
	InitTouch();
	TPC_Run();
	TPC_SetDefaultSW(TPC_PU_SW);
	TPC_SetU1In ( TPC_ADC_CH_0, 0x2A0, 0 );

	misc_register(& mmsp2ADC);
	return 0;
}

static void __exit mmsp2ADC_exit(void)
{
	TPC_INTR = 0;
	TPC_CNTL = (1<<6);
	misc_deregister(&mmsp2ADC);
}

module_init(mmsp2ADC_init);
module_exit(mmsp2ADC_exit);
