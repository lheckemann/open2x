/*
 *  linux/kernel/drivers/char/mmsp2_ts.c
 *
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 *  MMSP2 Touch panel controller driver
 *
 *  Based on s3c2410_ts.c
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/compiler.h>
#include <linux/interrupt.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/hardware.h>

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(x...) printk(x)
#else
#  define ds_printk(fmt, args...)
#endif

struct mmsp2_ts_event {
	unsigned short x;
	unsigned short y;
	unsigned short pressure;
	unsigned short pad;
};

enum pen_state {
	PEN_UP = 0,
	PEN_DOWN ,
	PEN_SAMPLE
};

static int count=0;
enum pen_state pen_data=PEN_UP;

#define CTRUE 1
#define TPC_MAN_READ_AZ		0
#define TPC_MAN_READ_X		1
#define TPC_MAN_READ_Y		2
#define TPC_MAN_READ_U1		3
#define TPC_MAN_READ_U2		4

#define TPC_BUSY		1
#define TPC_IDLE		0

static void
TPC_TimeDelay( unsigned long cnt )
{
	volatile unsigned long i, j, k;

	for( i=0, k=0 ; i<cnt ; i++ )
		for( j=0; j<cnt; j++ )
			k++;
}

static void
M2DTK_SetTouchPanelControlOut(void)
{
#ifdef CONFIG_MACH_MMSP2_DTK3
	set_gpio_ctrl(GPIO_C12, GPIOMD_OUT, GPIOPU_NOSET); // XMON
	set_gpio_ctrl(GPIO_C15, GPIOMD_OUT, GPIOPU_NOSET); // nYPON
	set_gpio_ctrl(GPIO_C14, GPIOMD_OUT, GPIOPU_NOSET); // nXPON
	set_gpio_ctrl(GPIO_C13, GPIOMD_OUT, GPIOPU_NOSET); // YMON
#elif defined(CONFIG_MACH_MMSP2_DTK4)
	set_gpio_ctrl(GPIO_F5, GPIOMD_OUT, GPIOPU_NOSET); // TSMX_EN
	set_gpio_ctrl(GPIO_F4, GPIOMD_OUT, GPIOPU_NOSET); // TSPY_EN
	set_gpio_ctrl(GPIO_F6, GPIOMD_OUT, GPIOPU_NOSET); // TSPX_EN
	set_gpio_ctrl(GPIO_F3, GPIOMD_OUT, GPIOPU_NOSET); // TSMY_EN
#endif

}

static void
PreSetXRead ( void )
{
#ifdef CONFIG_MACH_MMSP2_DTK3
	write_gpio_bit(GPIO_C12, TPC_GPIO_XM_ON);
	write_gpio_bit(GPIO_C15, TPC_GPIO_nYP_OFF);
	write_gpio_bit(GPIO_C14, TPC_GPIO_nXP_ON);
	write_gpio_bit(GPIO_C13, TPC_GPIO_YM_OFF);
#elif defined(CONFIG_MACH_MMSP2_DTK4)
	write_gpio_bit(GPIO_F5, TPC_GPIO_XM_ON);
	write_gpio_bit(GPIO_F4, TPC_GPIO_nYP_OFF);
	write_gpio_bit(GPIO_F6, TPC_GPIO_nXP_ON);
	write_gpio_bit(GPIO_F3, TPC_GPIO_YM_OFF);
#endif
}


static void
PreSetYRead ( void )
{
#ifdef CONFIG_MACH_MMSP2_DTK3
	write_gpio_bit(GPIO_C12, TPC_GPIO_XM_OFF);
	write_gpio_bit(GPIO_C15, TPC_GPIO_nYP_ON);
	write_gpio_bit(GPIO_C14, TPC_GPIO_nXP_OFF);
	write_gpio_bit(GPIO_C13, TPC_GPIO_YM_ON);
#elif defined(CONFIG_MACH_MMSP2_DTK4)
	write_gpio_bit(GPIO_F5, TPC_GPIO_XM_OFF);
	write_gpio_bit(GPIO_F4, TPC_GPIO_nYP_ON);
	write_gpio_bit(GPIO_F6, TPC_GPIO_nXP_OFF);
	write_gpio_bit(GPIO_F3, TPC_GPIO_YM_ON);
#endif
}


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
TPC_SetTouchSensitivity(unsigned long TPSNS)
{
//	TPC_CLK_CNTL = (unsigned short)( (TPC_CLK_CNTL & 0x00ffUL) | TPSNS << 8 ); // modify by hhsong
	TPC_CLK_CNTL = (unsigned short)( (TPC_CLK_CNTL & 0x00ffUL) | TPSNS ); 
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
TPC_SetInterruptEnable(unsigned short IntEn)
{
	TPC_INTR = 0;

	if( (IntEn & TPC_PEN_DWN_INT_EN) == TPC_PEN_DWN_INT_EN )
		TPC_INTR = (unsigned short)(TPC_INTR | TPC_PEN_DWN_INT_EN);

	if( (IntEn & TPC_PEN_CMP_INT_EN) == TPC_PEN_CMP_INT_EN )
		TPC_INTR = (unsigned short)(TPC_INTR | TPC_PEN_CMP_INT_EN);

	if( (IntEn & TPC_PEN_DAT_INT_EN) == TPC_PEN_DAT_INT_EN )
		TPC_INTR = (unsigned short)(TPC_INTR | TPC_PEN_DAT_INT_EN);

	if( (IntEn & TPC_U1_CMP_INT_EN ) == TPC_U1_CMP_INT_EN  )
		TPC_INTR = (unsigned short)(TPC_INTR | TPC_U1_CMP_INT_EN);

	if( (IntEn & TPC_U2_CMP_INT_EN ) == TPC_U2_CMP_INT_EN  )
		TPC_INTR = (unsigned short)(TPC_INTR | TPC_U2_CMP_INT_EN);
}

static void
TPC_SetInterruptDisable(unsigned short IntEn)
{
	if( (IntEn & TPC_PEN_DWN_INT_EN) == TPC_PEN_DWN_INT_EN )
		TPC_INTR = (unsigned short)(TPC_INTR & (~(TPC_PEN_DWN_INT_EN)));
	if( (IntEn & TPC_PEN_CMP_INT_EN) == TPC_PEN_CMP_INT_EN )
		TPC_INTR = (unsigned short)(TPC_INTR & (~(TPC_PEN_CMP_INT_EN)));
	if( (IntEn & TPC_PEN_DAT_INT_EN) == TPC_PEN_DAT_INT_EN )
		TPC_INTR = (unsigned short)(TPC_INTR & (~(TPC_PEN_DAT_INT_EN)));
	if( (IntEn & TPC_U1_CMP_INT_EN ) == TPC_U1_CMP_INT_EN  )
		TPC_COMP_U1 = (unsigned short)(TPC_COMP_U1 & (~(0x0800)));
	if( (IntEn & TPC_U2_CMP_INT_EN ) == TPC_U2_CMP_INT_EN)
		TPC_COMP_U2 = (unsigned short)(TPC_COMP_U2 & (~(0x0800)));
}

static void
TPC_ClrInterruptPend(unsigned long ClrInt)
{
	TPC_INTR = (unsigned short)((TPC_INTR & 0xffe0UL) | (ClrInt & 0x1fUL));
}

static unsigned short
TPC_GetInterruptPend(void)       
{
	return (unsigned short)( TPC_INTR & 0x1fUL );
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
	// CNTL[6] : '1': comparator power down
	TPC_CNTL = ~(1<<6);

	TPC_SetInterruptEnable( 0 );
	TPC_ClrInterruptPend( 0xffff );

	// off all switches
	TPC_SetDefaultSW(0);

	TPC_SetADCClock( 255 );
	TPC_SetADCParam ( 0,	// BandGapTime
			255,	// CaptureIdleTime
			255,	// SwitchSetupTime
			5,	// DecimationRatio
			4 );	// ADCConversionTime
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
	
	TPC_SetU1In ( TPC_ADC_CH_0, 50, CTRUE );

	TPC_SetDefaultSW ( TPC_PU_SW | TPC_YM_SW );
}

static unsigned short
TPC_ManRdData(unsigned long RdType)
{
	if(RdType == TPC_MAN_READ_X  )       return (unsigned short)(TPC_X_VALUE & 0x3ff);
	else if(RdType == TPC_MAN_READ_Y  )  return (unsigned short)(TPC_Y_VALUE & 0x3ff);
	else if(RdType == TPC_MAN_READ_AZ )  return (unsigned short)(TPC_AZ_VALUE & 0x3ff);
	else if(RdType == TPC_MAN_READ_U1 )  return (unsigned short)(TPC_U1_VALUE & 0x3ff);
	else if(RdType == TPC_MAN_READ_U2 )  return (unsigned short)(TPC_U2_VALUE & 0x3ff);
	else                                return 0xffff;
}

static unsigned short
TPC_ManReadChannel(unsigned long Channel)
{
	unsigned short ReadValue;

	if(Channel == TPC_MAN_READ_X  )
		TPC_SetChannelEnb(TPC_XY_CH_EN);
	else if(Channel == TPC_MAN_READ_Y  )
		TPC_SetChannelEnb(TPC_XY_CH_EN);
	else if(Channel == TPC_MAN_READ_AZ )
		TPC_SetChannelEnb(TPC_AZ_CH_EN);
	else if(Channel == TPC_MAN_READ_U1 )
		TPC_SetChannelEnb(TPC_U1_CH_EN);
	else if(Channel == TPC_MAN_READ_U2 )
		TPC_SetChannelEnb(TPC_U2_CH_EN);
	else
            return 0xffff;

	TPC_Run();

	do { } while( ( TPC_CNTL & 1 ) == (unsigned short)TPC_BUSY );

	ReadValue  = TPC_ManRdData(Channel);

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
	TPC_SetChannelEnb( TPC_U1_CH_EN );
	TPC_Run();

	while ( !TPC_IsTouchBusy() );
	while ( TPC_IsTouchBusy() );

	ADCOut = TPC_ManReadChannel( (unsigned char)TPC_MAN_READ_U1 );
	return ADCOut;
}

static void ts_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned long IrqPend = TPC_GetInterruptPend();
	unsigned short x, y;

    switch ( pen_data )
	{
	case PEN_UP:
		if( IrqPend & TPC_PEN_DWN_INT_FLAG )
		{
			pen_data = PEN_DOWN;
			TPC_SetInterruptEnable ( 0 );
			TPC_ClrInterruptPend ( 0xffff );
			TPC_Stop();
		}
                break;
	case PEN_DOWN:
		if( IrqPend & TPC_U1_CMP_INT_FLAG )
		{
			pen_data = PEN_UP;
			TPC_SetInterruptEnable ( TPC_PEN_DWN_INT_EN );
			TPC_ClrInterruptPend ( 0xffff );
		}

		if ( IrqPend & TPC_PEN_DAT_INT_FLAG )
		{
			TPC_ClrInterruptPend ( TPC_PEN_DAT_INT_FLAG );
			x = TPC_ManRdData( TPC_MAN_READ_X );
			y = TPC_ManRdData( TPC_MAN_READ_Y );
		}
		break;
        }
}

static ssize_t mmsp2_ts_read(struct file *filp, char *buf, size_t count, loff_t *l)
{
	struct mmsp2_ts_event ts;

	if(pen_data == PEN_DOWN) 
	{
		ts.pressure = 1;
		ts.pad = 0;

		// Read ADC	X
		udelay(10);
		TPC_SetDefaultSW( 0 );
		udelay(10);
		PreSetXRead();
		udelay(10);
		ts.x = TPC_GetStaticADCOut( 1 );
		
		// Read ADC	Y
		udelay(10);
		TPC_SetDefaultSW( 0 );
		udelay(10);
		PreSetYRead();
		udelay(10);
		ts.y = TPC_GetStaticADCOut( 0 );

		#ifdef DS_DEBUG
		ds_printk("--> X Position[%04d], Y Position[%04d]\n", ts.x, ts.y);	
		#endif
		
		TPC_SetDefaultSW ( TPC_PU_SW );
		udelay(30);
#ifdef CONFIG_MACH_MMSP2_DTK3
		write_gpio_bit(GPIO_C12, TPC_GPIO_XM_OFF);
		write_gpio_bit(GPIO_C15, TPC_GPIO_nYP_OFF);
		write_gpio_bit(GPIO_C14, TPC_GPIO_nXP_OFF);
		write_gpio_bit(GPIO_C13, TPC_GPIO_YM_ON);
#elif defined(CONFIG_MACH_MMSP2_DTK4)
		write_gpio_bit(GPIO_F5, TPC_GPIO_XM_OFF);
		write_gpio_bit(GPIO_F4, TPC_GPIO_nYP_OFF);
		write_gpio_bit(GPIO_F6, TPC_GPIO_nXP_OFF);
		write_gpio_bit(GPIO_F3, TPC_GPIO_YM_ON);
#endif
			
		pen_data = PEN_UP;
		TPC_ClrInterruptPend ( 0xffff );
		TPC_SetInterruptEnable ( TPC_PEN_DWN_INT_EN );
		TPC_Run();
	}
	else {
		ts.pressure = 0;
		ts.x = 0;
		ts.y = 0;
		ts.pad = 0;
	}

	if(copy_to_user(buf, &ts, sizeof(struct mmsp2_ts_event))) {
		return -EFAULT;
	}

	return sizeof(struct mmsp2_ts_event);
}

static int mmsp2_ts_open(struct inode *inode, struct file *filp)
{
	if(count==0) {
		count = 1;
		return 0;
	}
	else 
		return -EBUSY;
}

static int mmsp2_ts_release(struct inode *inode, struct file *filp)
{
	count = 0;
	return 0;
}

static struct file_operations mmsp2_ts_fops = {
	owner:		THIS_MODULE,
	read:		mmsp2_ts_read,
	open:		mmsp2_ts_open,
	release:	mmsp2_ts_release,
};

static struct miscdevice mmsp2_tpanel = {
	TPANEL_MINOR, "ts", &mmsp2_ts_fops
};

static int __init mmsp2_ts_init(void)
{
	int retval;

	TPC_SetDefaultSW(0);
	M2DTK_SetTouchPanelControlOut();

	InitTouch ();
	TPC_Run();

   	if((retval=request_irq(IRQ_ADC, ts_interrupt, SA_INTERRUPT, "mmsp2_ts", NULL))) {
		printk(KERN_WARNING "mmsp2_ts: failed to get IRQ\n");
		return retval;
	}

	TPC_SetDefaultSW (TPC_PU_SW);

#ifdef CONFIG_MACH_MMSP2_DTK3
	write_gpio_bit(GPIO_C12, TPC_GPIO_XM_OFF);
	write_gpio_bit(GPIO_C15, TPC_GPIO_nYP_OFF);
	write_gpio_bit(GPIO_C14, TPC_GPIO_nXP_OFF);
	write_gpio_bit(GPIO_C13, TPC_GPIO_YM_ON);
#elif defined(CONFIG_MACH_MMSP2_DTK4)
	write_gpio_bit(GPIO_F5, TPC_GPIO_XM_OFF);
	write_gpio_bit(GPIO_F4, TPC_GPIO_nYP_OFF);
	write_gpio_bit(GPIO_F6, TPC_GPIO_nXP_OFF);
	write_gpio_bit(GPIO_F3, TPC_GPIO_YM_ON);
#endif

	TPC_SetTouchSensitivity ( 10 );
	TPC_SetU1In ( TPC_ADC_CH_0, 50, CTRUE );

	pen_data = PEN_UP;
	misc_register(&mmsp2_tpanel);

	TPC_ClrInterruptPend( 0xffff );
	TPC_SetInterruptEnable(TPC_PEN_DWN_INT_EN);

	printk("MMSP2 Touch Panel Controller Driver\n");
	return 0;
}

static void __exit mmsp2_ts_exit(void)
{
	free_irq(IRQ_ADC,NULL);

	TPC_INTR = 0;
	TPC_CNTL = (1<<6);

	misc_deregister(&mmsp2_tpanel);
}

module_init(mmsp2_ts_init);
module_exit(mmsp2_ts_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
MODULE_DESCRIPTION("MMSP2 Touch Panel Controller Driver");
