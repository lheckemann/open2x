/*-----------------------------------------------------------------------------
	prototype header for ac97
	file name : proto_ac97.h
	by Nam Tae Woo
-----------------------------------------------------------------------------*/

#ifndef __PROTO_AC97_H__
#define __PROTO_AC97_H__

#include "../sound/typedef.h"

#define	AC_POWERDOWN			(*((volatile U16*)(0x0F26 +IO_BASE)))

// irq enable value
#define AC97_INT_CODECREADY		1<<7
#define AC97_INT_WRDONE			1<<6
#define AC97_INT_RDDONE			1<<5
#define AC97_INT_FOUDFLOW		1<<4
#define AC97_INT_ROUDFLOW		1<<3
#define AC97_INT_CLUDFLOW		1<<2
#define AC97_INT_PIOVFLOW		1<<1
#define AC97_INT_MIOVFLOW		1

// channel config value
#define AC97_CENTER				1<<6
#define AC97_REAR				1<<5
#define AC97_LFE				1<<4

// output width
#define AC97_OUT16				0x00
#define AC97_OUT18				0x08
#define AC97_OUT20				0x0c

// input width
#define AC97_IN16				0x00
#define AC97_IN18				0x02
#define AC97_IN20				0x03

// irq status
#define AC97_PEND_CODECREADY	1<<7
#define AC97_PEND_WRDONE		1<<6
#define AC97_PEND_RDDONE		1<<5
#define AC97_PEND_FOUDFLOW		1<<4
#define AC97_PEND_ROUDFLOW		1<<3
#define AC97_PEND_CLUDFLOW		1<<2
#define AC97_PEND_PIOVFLOW		1<<1
#define AC97_PEND_MIOVFLOW		1

// irq clear
#define AC97_CLR_CODECREADY		1<<7
#define AC97_CLR_WRDONE			1<<6
#define AC97_CLR_RDDONE			1<<5
#define AC97_CLR_FOUDFLOW		1<<4
#define AC97_CLR_ROUDFLOW		1<<3
#define AC97_CLR_CLUDFLOW		1<<2
#define AC97_CLR_PIOVFLOW		1<<1
#define AC97_CLR_MIOVFLOW		1

// control
#define AC97_COLDRESET			0
#define AC97_LINKOFF			5
#define AC97_LINKON				4
#define AC97_WARMRESET			6
#define AC97_PDOWNVAL			0x1000

// ac97 registers
typedef struct
{
	volatile U16 control;			// AC_CTRL_REG		0x0e00
	volatile U16 config;			// AC_CONFIG_REG	0x0e02
	volatile U16 irqena;			// AC_INT_ENA_REG	0x0e04
	volatile U16 irqpend;			// AC_STATUS_REG	0x0e06
	volatile U16 state;				// AC_STATE_MACHINE	0x0e08
	volatile U16 rsvrd1;			// reserved			0x0e0a
	volatile U16 codaddr;			// AC_CODEC_ADDR	0x0e0c
	volatile U16 coddata;			// AC_CODEC_DATA	0x0e0e
	volatile U16 codbusy;			// AC_CODEC_ACCESS	0x0e10
} AC97_REG;

// function
void AC97_GetDeviceInfo (MES_DEVINFO *pDevInfo);
void AC97_Initialize (U32 VirAddr);
void AC97_SetInterruptEnb (U16 IntEnb);
void AC97_SetConfig (U16 ExtChannel, U16 OutDatWidth, U16 InDatWidth);
CBOOL AC97_Enable (U16 TimeOut);
void AC97_Pause (void);
CBOOL AC97_Resume (U16 TimeOut);
void AC97_PowerDown (void);
CBOOL AC97_WakeUp (U16 TimeOut);
void AC97_Close (void);
U16 AC97_GetInterruptEnb (void);
U16 AC97_GetInterruptPend (void);
void AC97_ClrInterruptPend (U16 ClrPend);
CBOOL AC97_WriteCodecReg (U32 RegOffset, U16 WrData, U16 TimeOut);
U32 AC97_ReadCodecReg (U32 RegOffset, U16 TimeOut);
CBOOL AC97_IsBusy (void);

#endif
