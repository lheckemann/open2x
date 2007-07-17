
/*************************************************************************
  	(C) Copyright 2006

Project : GPX__BOOT__SOUND
Version : 1.0
Date    : 2006-3-15(hyun)
Author  : Hyun sog juk
Company : GamePark Holdings, Korea
Comments: ORIGNAL SOUCE => MAGIC EYES TEST SOUND
**************************************************************************/

#include <common.h>

#include "../sound/typedef.h"
#include "../sound/mes_const.h"
#include "../sound/mes_macro.h"
#include "../sound/mes_iomap.h"
#include "../sound/irq_manager.h"
#include "../sound/proto_ac97.h"
#include "../sound/proto_dma.h"
#include "../sound/proto_pwrman.h"
#include "../sound/AC97.h"

#define AC97_REAROUT	0
#define AC97_CENTEROUT	0
#define AC97_MICIN		0		// mic in to pcm out

#define AC97_USEIRQ		1		// use irq
#define PRINT_CODEC_REG	0

#define SOUND_CHIP_WM9711		1


#define SIZE_64KB		0xffff
#define SIZE_128KB		0x1ffff
#define	SIZE_OF_TEST_WAVEFILE		0x23fffff

#define AC97_TIMEOUT	6000

#define AC97_RADDR	0x01a20000		// rear data address
#define AC97_CADDR	0x01a40000		// center data address
#define AC97_IADDR	0x01a60000		// pcm in
#define AC97_MADDR	0x01a80000		// mic in data address

// intentional wrong value
U8	AC97_LINEIN = 0x07;
U8	AC97_PCMOUT = 0x07;
U8	AC97_WAVOUT = 0x07;

U32 WaveFileSize = 0;
U32 AC97_FADDR 	 = 0;
U8	SamplingRate = 0;

static U32 AC97_FDMA_CH = 0;		// front out dma channel
static U32 AC97_RDMA_CH = 1;		// rear out dma channel
static U32 AC97_CDMA_CH = 2;		// center out dma channel
static U32 AC97_IDMA_CH = 3;		// pcm in dma channel
static U32 AC97_MDMA_CH = 4;		// mic in dma channel

#define DMA_CHANNEL_COUNT	16

static TDMAC_ChannelData	DMAChannelData[DMA_CHANNEL_COUNT] = { 0, };


//extern CBOOL RegisterIRQHandler(U32 IRQNum, IRQSubhandler* Handler);

static void InitAC97Pwrman (void);
static void InitAC97Irq (void);
static void InitAC97Dma (U32 FrontDmaCh, U32 RearDmaCh, U32 CenterDmaCh,U32 InDmaCh, U32 MicDmaCh);
static void SetAC97Codec (int jackFlag);
void HandlerAC97Dma(void);
void RunAC97(int jackFlag);
void PMR_Initialize1 ( U32 Address );
void PMR_GetAudioStorageClkCtrl ( tAudStrClkCtrl *pAUDSTRCLKCTRL );
void PMR_SetAudioStorageClkCtrl ( tAudStrClkCtrl *pAUDSTRCLKCTRL );
void PMR_GetAC97I2SClk   ( tACLKInfo *pACLKINFO );
void PMR_SetAC97I2SClk     ( tACLKInfo *pACLKINFO );

struct WAVE_HEADER
{
 U8 riff_id[4];
 U32 riff_size;
 U8 wave_id[4];
 U8 format_id[4];
 U32 format_size;
 U16 format_type;
 U16 ChannelNo;
 U32 SamplesPerSec;
 U32 AvgBytesPerSec;
 U16 BytesPerSample;
 U16 BitsPerSample;
 U8 data_id[4];
 U32 data_size;
}waveheader;


void PlayerSoundLogo(int flag)
{
	S32		cmdcnt;
	U8 mode, samplerate;
	U32 filesize,memaddr;
	int ret;

	ret = nand_read_jffs2_func(PA_SOUND_DMA_BASE1, WAV_BASE, 0x40000);

	memcpy(&waveheader ,PA_SOUND_DMA_BASE1 ,sizeof(waveheader));

	if( (waveheader.data_size > 0x3c000) ||  (waveheader.ChannelNo > 2) || (waveheader.format_type != 0x01)
										 ||  (waveheader.BitsPerSample > 16) )
	{
		printf("WAV file format error\n");
		printf("format_type:0x%x\n", waveheader.format_type);
		printf("ChannelNo :%d\n",	 waveheader.ChannelNo);
		printf("SamplesPerSec:%d\n", waveheader.SamplesPerSec);
		printf("AvgBytesPerSec:%d\n", waveheader.AvgBytesPerSec);
		printf("BytesPerSample:%d\n", waveheader.BytesPerSample);
		printf("BitsPerSample:%d\n", waveheader.BitsPerSample);
		printf("data_size:%x\n", waveheader.data_size);
		return;
	}

	filesize=waveheader.data_size;
	memaddr=(U32) (PA_SOUND_DMA_BASE1 + sizeof(waveheader) );

	mode=1;											/* out */
	SamplingRate = 2;								/* 0=44khz 2=22khz*/
	WaveFileSize = filesize - 0x800;
	AC97_FADDR	= memaddr;

	if( mode==0 )
	{
		AC97_LINEIN = 1;
		AC97_PCMOUT = 1;
		AC97_WAVOUT = 0;
	}
	else if( mode==1 )
	{
		AC97_LINEIN = 0;
		AC97_PCMOUT = 0;
		AC97_WAVOUT = 1;
	}

	RunAC97(flag);

}


/*----------------------------------------------------------------------------
	initialize pwrman for ac97
----------------------------------------------------------------------------*/
static void InitAC97Pwrman(void)
{
	tAudStrClkCtrl AUDSTRCLKCTRL;
	tACLKInfo ACLKINFO;

	PMR_Initialize1( 0xc0000900 );
	// Enable AC97 pclk, Disable I2S clock, SPDIF Hold
	PMR_GetAudioStorageClkCtrl(&AUDSTRCLKCTRL);
	AUDSTRCLKCTRL.I2SCLK = 0; // Disable I2S Clock
	AUDSTRCLKCTRL.AC97CLK = 1; // Enable AC97 pclk
	PMR_SetAudioStorageClkCtrl(&AUDSTRCLKCTRL);

	PMR_GetAC97I2SClk(&ACLKINFO);
	ACLKINFO.ACLK_SOURCE = 0; 	// Source : ABIT Clock
	ACLKINFO.ACLK_DIVIDER = 0; 	// Divider : 1x
	PMR_SetAC97I2SClk(&ACLKINFO);
}

/*----------------------------------------------------------------------------
	initialize dma for ac97
------------------------------------------------------------------------------
< front/rear/center output dma channel setting >
	com0reg : burst=4word, increment src addr=1, flowsrc=0,
			  src data format=word, src peri width=don't care,
			  increment trg addr=0, flowtrg=1,
			  trg data format=word, trg peri width=half word
	com1reg : src addr step=1, trg addr step=0
	com2reg : 0xffff (64k bytes)
	srcaddr : external memory
	trgaddr : ac97 base address (0xc000_0e00)

< input dma channel setting >
	com0reg : burst=4word, increment src addr=0, flowsrc=1,
			  src data format=word, src peri width=half word,
			  increment trg addr=1, flowtrg=0,
			  trg data format=word, trg peri width=don't care
	com1reg : src addr step=0, trg addr step=1
	com2reg : 0xffff (64k bytes)
	srcaddr : ac97 base address (0xc000_0e00)
	trgaddr : external memory
----------------------------------------------------------------------------*/
static void InitAC97Dma (U32 FrontDmaCh, U32 RearDmaCh, U32 CenterDmaCh,U32 InDmaCh, U32 MicDmaCh)
{

	TDMAC_ChannelData *pChannelData;
	MES_DEVINFO devinfo;

	DMA_GetDeviceInfo( &devinfo );
	/* DMA ADDR를 레지스터에 기록하고 DMA 핸들러를 인터럽트 백터 핸들러에 기록한다 */
	DMA_Initialize( devinfo.IOBaseAddress );

	// setting front out channel DMA
	pChannelData = &(DMAChannelData[AC97_FDMA_CH]);
	pChannelData->iChannel = AC97_FDMA_CH;
	pChannelData->Source.dwType = DMA_MEMTYPE_MEM_WORD;
	pChannelData->Source.iStep = 1;

    if( AC97_LINEIN )
    	{pChannelData->Source.dwAddr = AC97_IADDR;}
	else
		{pChannelData->Source.dwAddr = AC97_FADDR;}

	pChannelData->Source.bFlyBy = CFALSE;

	pChannelData->Target.dwType = DMA_MEMTYPE_IO_HWORD;
	pChannelData->Target.iStep = 0;
	pChannelData->Target.dwAddr = AC97_BASE;	/*base dma address*/
	pChannelData->Target.bFlyBy = CFALSE;
	pChannelData->Target.IOIndex = DMA_PERIPHERAL_INDEX_AC97_PCMOUT;

	pChannelData->dwDataSize = (U16)SIZE_64KB;
	pChannelData->dwBurstType = DMA_BURSTTYPE_4WORD;
	pChannelData->dwIntType = DMA_INTTYPE_END;

	DMA_SetChannel(pChannelData);


#if AC97_REAROUT
	// setting rear out channel dma
	pChannelData = &(DMAChannelData[AC97_RDMA_CH]);
	pChannelData->iChannel = AC97_RDMA_CH;
	pChannelData->Source.dwType = DMA_MEMTYPE_MEM_WORD;
	pChannelData->Source.iStep = 1;

	if( AC97_LINEIN )
		{pChannelData->Source.dwAddr = AC97_IADDR;}
	else
		{pChannelData->Source.dwAddr = AC97_RADDR;}

	pChannelData->Source.bFlyBy = CFALSE;

	pChannelData->Target.dwType = DMA_MEMTYPE_IO_HWORD;
	pChannelData->Target.iStep = 0;
	pChannelData->Target.dwAddr = AC97_BASE;
	pChannelData->Target.bFlyBy = CFALSE;
	pChannelData->Target.IOIndex = DMA_PERIPHERAL_INDEX_AC97_SOUT;

	pChannelData->dwDataSize = (U16)SIZE_64KB;
	pChannelData->dwBurstType = DMA_BURSTTYPE_4WORD;
	pChannelData->dwIntType = DMA_INTTYPE_END;

	DMA_SetChannel(pChannelData);
#endif

#if AC97_CENTEROUT
	// setting center/lfe out channel dma
	pChannelData = &(DMAChannelData[AC97_CDMA_CH]);
	pChannelData->iChannel = AC97_CDMA_CH;
	pChannelData->Source.dwType = DMA_MEMTYPE_MEM_WORD;
	pChannelData->Source.iStep = 1;
	pChannelData->Source.dwAddr = AC97_CADDR;
	pChannelData->Source.bFlyBy = CFALSE;

	pChannelData->Target.dwType = DMA_MEMTYPE_IO_HWORD;
	pChannelData->Target.iStep = 0;
	pChannelData->Target.dwAddr = AC97_BASE;
	pChannelData->Target.bFlyBy = CFALSE;
	pChannelData->Target.IOIndex = DMA_PERIPHERAL_INDEX_AC97_CWOUT;

	pChannelData->dwDataSize = (U16)SIZE_64KB;
	pChannelData->dwBurstType = DMA_BURSTTYPE_4WORD;
	pChannelData->dwIntType = DMA_INTTYPE_END;

	DMA_SetChannel(pChannelData);
#endif

	// setting pcm in channel dma
	pChannelData = &(DMAChannelData[AC97_IDMA_CH]);
	pChannelData->iChannel = AC97_IDMA_CH;
	pChannelData->Source.dwType = DMA_MEMTYPE_IO_HWORD;
	pChannelData->Source.iStep = 0;
	pChannelData->Source.dwAddr = AC97_BASE;
	pChannelData->Source.bFlyBy = CFALSE;
	pChannelData->Source.IOIndex = DMA_PERIPHERAL_INDEX_AC97_PCMIN;

	pChannelData->Target.dwType = DMA_MEMTYPE_MEM_WORD;
	pChannelData->Target.iStep = 1;
	pChannelData->Target.dwAddr = AC97_IADDR;
	pChannelData->Target.bFlyBy = CFALSE;

	pChannelData->dwDataSize = (U16)SIZE_64KB;
	pChannelData->dwBurstType = DMA_BURSTTYPE_4WORD;
	pChannelData->dwIntType = DMA_INTTYPE_END;

	DMA_SetChannel(pChannelData);

	// setting mic in channel dma
	pChannelData = &(DMAChannelData[AC97_MDMA_CH]);
	pChannelData->iChannel = AC97_MDMA_CH;
	pChannelData->Source.dwType = DMA_MEMTYPE_IO_HWORD;
	pChannelData->Source.iStep = 0;
	pChannelData->Source.dwAddr = AC97_BASE;
	pChannelData->Source.bFlyBy = CFALSE;
	pChannelData->Source.IOIndex = DMA_PERIPHERAL_INDEX_AC97_MICIN;

	pChannelData->Target.dwType = DMA_MEMTYPE_MEM_WORD;
	pChannelData->Target.iStep = 1;
	pChannelData->Target.dwAddr = AC97_MADDR;
	pChannelData->Target.bFlyBy = CFALSE;

	pChannelData->dwDataSize = (U16)SIZE_64KB;
	pChannelData->dwBurstType = DMA_BURSTTYPE_4WORD;
	pChannelData->dwIntType = DMA_INTTYPE_END;

	DMA_SetChannel(pChannelData);

}

/*----------------------------------------------------------------------------
	initialize codec register
----------------------------------------------------------------------------*/
static void SetAC97Codec(int jackFlag)
{
	CBOOL CmdResult;
	U32 rdata;
	U16 wtimeout=600, rtimeout=1400;
	U16 offset;

	CmdResult = AC97_WriteCodecReg (0x00,0,wtimeout);		// codec reg reset

#ifdef SOUND_CHIP_WM9711
// set volume and tone
// 02 : LRout   04 : hp       06 : mono   08 : tone   0a : beep  0c : phone
// 0e : mic     10 : line in  12 : AUXDAC 14 : video  16 : aux   18 : DAC
	if(jackFlag)	/* JACK ON */
	{
		CmdResult = AC97_WriteCodecReg (0x02,0x8000,wtimeout); 		/* SPEAKER MUTE */
		CmdResult = AC97_WriteCodecReg (0x04,0x1212,wtimeout);		/* EAR VOLUME */
		CmdResult = AC97_WriteCodecReg (0x18,0x0606,wtimeout);		/* common volume */
	}
	else		/* NON JACK */
	{
		CmdResult = AC97_WriteCodecReg (0x04,0x8000,wtimeout);		/* EAR MUTE */
		//CmdResult = AC97_WriteCodecReg (0x02,0x0606,wtimeout); 	/* SPEAKER VOLUME */
		CmdResult = AC97_WriteCodecReg (0x02,0x0B0B,wtimeout); 		/* SPEAKER VOLUME */
		CmdResult = AC97_WriteCodecReg (0x18,0x0606,wtimeout);		/* common volume */
	}

#else
// set volume and tone
// 02 : LRout   04 : hp       06 : mono  08 : tone   0a : beep  0c : phone
// 0e : mic     10 : line in  12 : cd    14 : video  16 : aux   18 : pcm out
	for (offset=2; offset<=0x18; offset=offset+2)
	{
		CmdResult = AC97_WriteCodecReg (offset,0x0606,wtimeout);
	}
	CmdResult = AC97_WriteCodecReg (0x10,0x8808,wtimeout);	// line in mix mute

// record source select
#if AC97_MICIN
	CmdResult = AC97_WriteCodecReg (0x1a,0x0000,wtimeout);	// record mic
#else
	CmdResult = AC97_WriteCodecReg (0x1a,0x0404,wtimeout);	// record line in
#endif

	CmdResult = AC97_WriteCodecReg (0x1c,0x0000,wtimeout);	// rec gain
	CmdResult = AC97_WriteCodecReg (0x1e,0x000f,wtimeout);	// mic gain
#endif


#ifdef SOUND_CHIP_WM9711
	CmdResult =	AC97_WriteCodecReg (0x24,0x1C67,wtimeout);	//power down enable
	CmdResult = AC97_WriteCodecReg (0x26,0x8100,wtimeout);	// power
#else
	CmdResult = AC97_WriteCodecReg (0x26,0x8000,wtimeout);	// power
#endif

	// variable rate enable
	rdata = AC97_ReadCodecReg (0x2a,rtimeout);				// extended control
	rdata = rdata | 0x00000001;
	CmdResult = AC97_WriteCodecReg (0x2a,(U16) rdata,wtimeout);

	// variable sampling rate setting
	// modify dennis

	switch( SamplingRate )
	{
		case 0:
			for (offset=0x2c; offset<=0x34; offset=offset+2)
			{
				CmdResult = AC97_WriteCodecReg (offset,44100,wtimeout);
			}
			break;
		case 1:
			for (offset=0x2c; offset<=0x34; offset=offset+2)
			{
				CmdResult = AC97_WriteCodecReg (offset,32000,wtimeout);
			}
			break;
		case 2:
			for (offset=0x2c; offset<=0x34; offset=offset+2)
			{
				CmdResult = AC97_WriteCodecReg (offset,22050,wtimeout);
			}
			break;
		default:
			printf("Invalid sampling rate\n");
			break;
	}

#if AC97_REAROUT
	CmdResult = AC97_WriteCodecReg (0x38, 0x0808,wtimeout);
#endif
#if AC97_CENTEROUT
	CmdResult = AC97_WriteCodecReg (0x36, 0x0808,wtimeout);
#endif


// print codec register value
#if 0
	for (offset=0; offset<=0x38; offset=offset+2)
	{
		rdata = AC97_ReadCodecReg (offset, rtimeout);
	}
#endif

}

int kernel_load = 0;
/*----------------------------------------------------------------------------
	test main
----------------------------------------------------------------------------*/
void RunAC97(int jackFlag)
{
	tAudStrClkCtrl AUDSTRCLKCTRL;
	int i;
	// never get here
	if( (AC97_LINEIN>1) || (AC97_PCMOUT>1) || (AC97_WAVOUT>1) )
	{
		printf("Mode Setting Error\n");
		return;
	}

	InitAC97Pwrman();

	InitIRQ();

	InitAC97Dma(AC97_FDMA_CH, AC97_RDMA_CH, AC97_CDMA_CH,
				AC97_IDMA_CH, AC97_MDMA_CH);

	/* Get DMA HANDLER */
	InitAC97Irq();
	AC97_Initialize(0);		/* get base pointer */

	AC97_SetConfig (0, AC97_OUT16, AC97_IN16);

	if((AC97_Enable(AC97_TIMEOUT))==CFALSE)		// codec not ready
	{
		printf("\n ac97 codec is not ready \n");
		return;
	}
	SetAC97Codec(jackFlag);
	AC97_SetInterruptEnb(AC97_INT_FOUDFLOW | AC97_INT_MIOVFLOW);

	if( AC97_LINEIN )
		{DMA_RunChannel(AC97_IDMA_CH);}		// enable pcm in dma

#if AC97_MICIN
	DMA_RunChannel(AC97_MDMA_CH);			// enable mic in dma
#endif

	if( AC97_WAVOUT )
		{DMA_RunChannel(AC97_FDMA_CH);}		//dma_run

	if( AC97_PCMOUT )
	{
		DMA_RunChannel(AC97_FDMA_CH);
		DMA_RunChannel(AC97_RDMA_CH);
	}

	kernel_load=0;
	while ( DMA_GetSrcAddr(AC97_FDMA_CH) <= (AC97_FADDR + WaveFileSize) )
	{
		if(!kernel_load)
		{
			/* Read to kernel */
			nand_read_func(0x1000000, 0x80000,0xB0000);	//700kybte MAX
			kernel_load=1;
		}
	}

	DMA_StopChannel(AC97_FDMA_CH);
	AC97_Close();
	irq_free_handler (IRQ_DMAINT);
}

/*----------------------------------------------------------------------------
	initialize irq
----------------------------------------------------------------------------*/
static void InitAC97Irq (void)
{
#ifdef AC97_USEIRQ
	U32 ch;

#ifdef CONFIG_USE_IRQ
	MaskIRQ(IRQ_DMAINT);
	irq_install_handler(IRQ_DMAINT, (interrupt_handler_t *)HandlerAC97Dma, NULL);
	UnmaskIRQ(IRQ_DMAINT);	// unmask dma irq
#else

	MaskIRQ(IRQ_DMAINT);	// mask dma irq
	for (ch = AC97_FDMA_CH; ch <= AC97_MDMA_CH; ch++)
	{
		DMA_RegisterIRQHandler(ch, (DMAIRQSubHandlerFn*)HandlerAC97Dma);
	}
	UnmaskIRQ(IRQ_DMAINT);	// unmask dma irq
#endif

#endif

}

/*----------------------------------------------------------------------------
	dma handler
----------------------------------------------------------------------------*/
void HandlerAC97Dma (void)
{
	U32 ch;

	for (ch=AC97_FDMA_CH; ch<=AC97_MDMA_CH; ch++)
	{
		//Check whether there is END IRQ for this channel
		if ((DMA_GetChannelStatus(ch) & ENDINTR) != 0)
		{
			DMA_ClrInterruptPend(ch);
			DMA_SetDataSize(ch, (U32)SIZE_64KB);

			if( AC97_WAVOUT )
			{
				if (ch == AC97_FDMA_CH)
				{
					/* For repeatation */
#if 0
					if (DMA_GetSrcAddr(ch) >= SIZE_OF_TEST_WAVEFILE)
					{
						return;
					}
#endif
				}
			}
			else
			{
				switch (ch)
				{
					case 0 :
					if( AC97_LINEIN )
					{
						if (DMA_GetSrcAddr(ch) >= AC97_IADDR + SIZE_128KB + 1)
						{
							DMA_SetSrcAddr(ch, (U32) AC97_IADDR);
						}
						break;
					}
					else
					{
						if (DMA_GetSrcAddr(ch) >= AC97_FADDR + SIZE_128KB + 1)
						{
							DMA_SetSrcAddr(ch, (U32) AC97_FADDR);
						}
						break;
					}
					case 1 :
					if( AC97_LINEIN )
					{
						if (DMA_GetSrcAddr(ch) >= AC97_IADDR + SIZE_128KB + 1)
						{
							DMA_SetSrcAddr(ch, (U32) AC97_IADDR);
						}
						break;
					}
					else
					{
						if (DMA_GetSrcAddr(ch) >= AC97_RADDR + SIZE_128KB + 1)
						{
							DMA_SetSrcAddr(ch, (U32) AC97_RADDR);
						}
						break;
					}
					case 2 :
						if (DMA_GetSrcAddr(ch) >= AC97_CADDR + SIZE_128KB + 1)
						{
							DMA_SetSrcAddr(ch, (U32) AC97_CADDR);
						}
						break;
					case 3 :
						if (DMA_GetTrgAddr(ch) >= AC97_IADDR + SIZE_128KB + 1)
						{
							DMA_SetTrgAddr(ch, (U32) AC97_IADDR);
						}
						break;
					case 4 :
						if (DMA_GetTrgAddr(ch) >= AC97_MADDR + SIZE_128KB + 1)
						{
							DMA_SetTrgAddr(ch, (U32) AC97_MADDR);
						}
						break;
					default :
						printf("invalid dma channel \n");
						return;
				}
			}

			DMA_SetInterruptEnb(ch, DMA_INTTYPE_END);
			DMA_RunChannel(ch);
		}
	}
}

/*--------------------------------------------------------------------------------------------
********************* proto_ pwrman.c ********************************************************
----------------------------------------------------------------------------------------------*/
PMR_REG *pPMR_REG1;

void PMR_Initialize1 ( U32 Address )
{
	pPMR_REG1 = (PMR_REG *)(Address);
}


void PMR_GetAudioStorageClkCtrl ( tAudStrClkCtrl *pAUDSTRCLKCTRL )
{
	pAUDSTRCLKCTRL->PCLK_CDROM 		= (pPMR_REG1->ASCLKEN >> 13) & 0x1;
	pAUDSTRCLKCTRL->BCLK_IDEIF 		= (pPMR_REG1->ASCLKEN >> 12) & 0x1;
	pAUDSTRCLKCTRL->MSCLK 			= (pPMR_REG1->ASCLKEN >> 9 ) & 0x1;
	pAUDSTRCLKCTRL->PCLK_SD  		= (pPMR_REG1->ASCLKEN >> 8 ) & 0x1;
	pAUDSTRCLKCTRL->SPDIFOUTCLK 	= (pPMR_REG1->ASCLKEN >> 3 ) & 0x1;
    pAUDSTRCLKCTRL->SPDIFINCLK 		= (pPMR_REG1->ASCLKEN >> 2 ) & 0x1;
    pAUDSTRCLKCTRL->I2SCLK 		    = (pPMR_REG1->ASCLKEN >> 1 ) & 0x1;
    pAUDSTRCLKCTRL->AC97CLK 		= (pPMR_REG1->ASCLKEN >> 0 ) & 0x1;
}

void PMR_SetAudioStorageClkCtrl ( tAudStrClkCtrl *pAUDSTRCLKCTRL )
{
	pPMR_REG1->ASCLKEN = 	(pAUDSTRCLKCTRL->PCLK_CDROM 	<< 13) |
							(pAUDSTRCLKCTRL->BCLK_IDEIF 	<< 12) |
							(pAUDSTRCLKCTRL->MSCLK 			<< 9 ) |
							(pAUDSTRCLKCTRL->PCLK_SD 		<< 8 ) |
							(pAUDSTRCLKCTRL->SPDIFOUTCLK 	<< 3 ) |
							(pAUDSTRCLKCTRL->SPDIFINCLK 	<< 2 ) |
							(pAUDSTRCLKCTRL->I2SCLK 		<< 1 ) |
							(pAUDSTRCLKCTRL->AC97CLK 		<< 0 );
}


void PMR_SetAC97I2SClk     ( tACLKInfo *pACLKINFO )
{
	pPMR_REG1->AUDICSET = (pACLKINFO->ACLK_SOURCE << 6) | (pACLKINFO->ACLK_DIVIDER);
}

void PMR_GetAC97I2SClk   ( tACLKInfo *pACLKINFO )
{
	pACLKINFO->ACLK_SOURCE = (pPMR_REG1->AUDICSET >> 6) & 0x3;
	pACLKINFO->ACLK_DIVIDER = pPMR_REG1->AUDICSET  & 0x3f;
}

// end of sound_logo.c
