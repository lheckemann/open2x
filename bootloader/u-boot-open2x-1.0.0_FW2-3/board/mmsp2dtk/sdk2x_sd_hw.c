/* **************************************************************************
 *    Copyright (C) 2005 by Rob Brown                                       *
 *    rob@cobbleware.com                                                    *
 *                                                                          *
 *    C Implementation: sdk2x_sd_hw                                         *
 *                                                                          *
 *    This program is free software; you can redistribute it and/or modify  *
 *    it under the terms of version 2 (and only version 2) of the GNU       *
 *    General Public License as published by the Free Software Foundation.  *
 *                                                                          *
 *    This program is distributed in the hope that it will be useful,       *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *    GNU General Public License for more details.                          *
 *                                                                          *
 *    You should have received a copy of the GNU General Public License     *
 *    along with this program; if not, write to the                         *
 *    Free Software Foundation, Inc.,                                       *
 *    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 * **************************************************************************/

/* The SDHC stuff might be useless because the whole API uses 32bit ints.
 * Well at least we can try.. */

#include <mmsp2dtk/sdk2x_priv.h>
#include <mmsp2dtk/sdk2x_sd_priv.h>
#include <mmsp2dtk/sdk2x_sd_hw.h>
#include <mmsp2dtk/mmsp2.h>
#include <mmsp20.h>
#include <../board/mmsp2dtk/video/mmsp2-regs.h>

#include <common.h>

#define TRANSFER_TIMEOUT				(1000000)
#define TRANSFER_TIMEOUT_MS			(10)
#define MAX_RETRIES						(15)
#define INIT_MAX_ITERATIONS			(40)
#define COMMAND_RESPONSE_TIMEOUT_MS	(10)

/* This is all debug stuff. The CID and CSD information isn't useful, although it would possibly be good
 * to store the card's serial number and check it sometimes, to make sure the card hasn't been changed from
 * under us. Then again, if the card *is* changed, it won't respond to data read/write commands because it
 * won't have been initialised, so that check probably has no value. */
#include <common.h>

int nInitPass;

int mmc_init_once = 0;

typedef struct _tagSdk2x_CID
{
	unsigned char ManufacturerID;
	unsigned short ApplicationID;
	char Name[6];
	int Revision;
	unsigned int SerialNumber;
	int Year;
	int Month;
} sdk2x_CID_t;

typedef struct _tagSdk2x_CsdFluff
{
	unsigned short CommandClasses;
	unsigned char ReadBlkPartial;
	unsigned char WriteBlkMisalign;
	unsigned char ReadBlkMisalign;
	unsigned char DsrImp;
	unsigned char VddRCurrMin;
	unsigned char VddRCurrMax;
	unsigned char VddWCurrMin;
	unsigned char VddWCurrMax;
	unsigned char EraseBlkEnable;
	unsigned char WpGrpEnable;
	unsigned char FileFormatGrp;
	unsigned char Copy;
	unsigned char FileFormat;
} sdk2x_CsdFluff_t;

static sdk2x_CID_t CID;
static sdk2x_CsdFluff_t CsdFluff;
#if (SDK2X_SD_DEBUG > 0)

void sdk2x_UartSendString(int a, char * text)
{
   printf(text);
}

#endif

sdk2x_SdCardInfo_t sdk2x_SdCardInfo;

/* Structure containing the important bits of the CSD, which are needed for further operation of the card. */
typedef struct _tagSdk2x_CSD
{
	unsigned char CsdStructure;
	unsigned char TAAC;
	unsigned char NSAC;
	unsigned char R2WFactor;
	unsigned char TranSpeed;
	unsigned char ReadBlkLength;
	unsigned int  CSize;
	unsigned char CSizeMult;
	unsigned char EraseBlkSize;
	unsigned char WpGrpSize;
	unsigned char WriteBlkLength;
	unsigned char PermWriteProtect;
	unsigned char TempWriteProtect;
	unsigned char WriteBlkPartial;
} sdk2x_CSD_t;

static sdk2x_CSD_t CSD;

/* Constants for decoding the transfer speed */
static const int TranSpeedRate[8]={ 10000, 100000, 1000000, 10000000, 0, 0, 0, 0 };
static const int TranSpeedMult[16]={ 0,10,12,13,15,20,25,30,35,40,45,50,55,60,70,80 };

/* Constants for decoding the access times */
static const int R2WFactorValues[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
static const int TaacValue[16] = { 0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80 };
static const int TaacExponent[8] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000 };

/* Calculated access times */
static int nAccessTimeRd;
static int nAccessTimeWr;

/* Prescaler used for low-speed (200kHz) comms. This is used until the card's transfer speed has
 * been determined. */
static unsigned char psLowSpeed;
static unsigned char psHighSpeed;
/* Frequency of the input clock to the CPU's SD/MMC peripheral. Used when calculating the prescaler
 * for the high-speed clock when the card's transfer speed has been determined. */
static unsigned int fMMC;
/* Hold the error returned from the card when the code returns a data transfer error. Probably only
 * useful for debugging, but it could be made externally available if necessary */
unsigned short nCardError;
/* Only put the card in 4-bit mode once and leave it there. For SD cards which don't support 4-bit mode
 * (apparently these exist, although I've never seen one), the code will keep trying to enable it.
 * Not a huge performance hit. */
static int n4BitMode;
/* Variables for providing a bit of down-time between transactions on the bus, which is apparently needed
 * (testing indicates that when the transactions are too close together, cards fail some transations) */
static int nInterTransactionTime;
static int nLastTransactionTime;
static int nFastTransactionDelay;
static block_dev_desc_t mmc_dev;

block_dev_desc_t * mmc_get_dev(int dev)
{
	if (0 != sdk2x_SdHwInit())
		return NULL;
	else
		return ((block_dev_desc_t *)&mmc_dev);
}

ulong
/****************************************************/
mmc_bread(int dev_num, ulong blknr, ulong blkcnt, ulong *dst)
/****************************************************/
{
	int ret = sdk2x_SdReadPages (blknr * mmc_dev.blksz, dst, blkcnt);
	return ret ? 0 : blkcnt;
}

#if (SDK2X_SD_DEBUG > 0)
void DumpSector (void *p)
{
	/* Simply dumps 512 bytes to the UART, 16 bytes per line. The line starts with offset from 0x0000 to 0x01F0,
	 * then 16 bytes in ascii format 00 01 02 03 etc, then 16 ASCII characters with unprintable characters
	 * replaced by '.'
	 * In other words, bog-standard memory dump format.
	 */
	int i, j;
	char temp[8];
	char c;
	unsigned char *pC = (unsigned char *)p;
	char DumpLine[71];

	for (i=0;i<70;i++)
		DumpLine[i]=' ';

	DumpLine[70]='\0';

	for (i=0;i<512;i+=16)
	{
		sprintf (temp,"%04X",i);
		for (j=0;j<4;j++)
			DumpLine[j]=temp[j];

		for (j=0;j<16;j++)
		{
			c = pC[i+j];

			sprintf (temp,"%02X",c);
			DumpLine[(j*3) + 5] = temp[0];
			DumpLine[(j*3) + 6] = temp[1];

			if (c<32)
				c='.';
			DumpLine[54 + j] = c;
		}
		sdk2x_UartSendString(0,DumpLine);
		sdk2x_UartSendString(0,"\r\n");
	}
	sdk2x_UartSendString(0,"\r\n");
}
#endif

static void SetPrescalerAndAccessTimes (unsigned char nPrescaler)
{
	int fClock;
	int nTemp;

	MSP_SDIPRE = nPrescaler;

	/* Find the time for 100 clocks at the given speed */
	fClock = fMMC / (((int)nPrescaler) + 1);
	/* Get clock in kHz */
	fClock /= 1000;

	if (!fClock)
		fClock = 1;

	nTemp = sdk2x_SdCardInfo.AccessClocks / fClock;

	if (!nTemp)
		nTemp = 1;

	/* Got the time in ms for the clock-related access time. Now add the asynchronous part */
	nAccessTimeRd = (nTemp + sdk2x_SdCardInfo.AccessTime) * 7373;
	nAccessTimeWr = nAccessTimeRd * R2WFactorValues[CSD.R2WFactor];

#if (SDK2X_SD_DEBUG > 0)
	sprintf (sdk2x_private_string,"Access: Clock %dms - Async %dms - Read %d ticks - Write %d ticks\r\n",
				nTemp,sdk2x_SdCardInfo.AccessTime,nAccessTimeRd,nAccessTimeWr );
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif
}

static void SetupSFRs (void)
{
	unsigned int fAPLL;
	unsigned short psAPLL;
	unsigned int nTemp;

	/* Set the GPIO port L lines to alt function 1 */
	MSP_GPIOLALTFNLOW = ((MSP_GPIOLALTFNLOW & 0xf000) | 0x0AAA);

	set_gpio_ctrl(GPIO_I14, GPIOMD_IN, GPIOPU_EN);		/* SDICD */

	/* Initially set the MMC/SD clock to less than 400kHz */
	/* APLL is the clock source used by the SD/MMC peripheral... well it is in this code anyway */
	fAPLL = sdk2x_GetAPLL();

#if (SDK2X_SD_DEBUG > 1)
	sprintf (sdk2x_private_string,"APLL frequency = %d Hz\r\n",fAPLL);
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif

	/* Start with a base clock of something less than 75MHz, find the lowest prescaler that fits.
	 *	Remember that the max prescaler allowed is 0x3f, although usually it will be 6 or so
	*/
	psAPLL = fAPLL / 75000000;
	if (psAPLL > 0x3f)
		psAPLL = 0x3f;
	/* Find the input frequency to the SD/MMC peripheral */
	fMMC = fAPLL / (psAPLL + 1);

#if (SDK2X_SD_DEBUG > 1)
	sprintf (sdk2x_private_string,"APLL prescaler = %3d, MMC frequency   = %d Hz\r\n",psAPLL,fMMC);
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif

	/* Find internal prescaler for 200kHz operation */
	nTemp = fMMC / 200000;
	if (nTemp > 255)
		nTemp = 255;

	psLowSpeed = (unsigned char)nTemp;

	/* Set inter-transaction time to 1250us, to give 250 clock pulses or so */
	nInterTransactionTime = 1250;

#if (SDK2X_SD_DEBUG > 1)
	sprintf (sdk2x_private_string,"Low speed MMC prescaler = %3d, Card frequency  = %d Hz\r\n",psLowSpeed,fMMC / (psLowSpeed + 1));
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif

	/* Set internal prescaler to maximum, so that we don't cause an overclocking situation when the external
		prescaler gets set */
	MSP_SDIPRE = 0xff;

	/* Set master clock register to supply the SD/MMC peripheral from APLL, divided by the prescaler calculated above. */
	MSP_UIRMCSETREG = ((MSP_UIRMCSETREG & 0xff) | 0xC000 | (psAPLL <<8));

	/* Set internal prescaler for low-speed operation */
	SetPrescalerAndAccessTimes (psLowSpeed);

	/* Enable the MMC/SD module clock */
	MSP_ASCLKENREG |= 0x100;

	/* Disable the MMC/SD module interrupt sources */
	MSP_SDIINTMSK0 = 0;
	MSP_SDIINTMSK1 = 0;

	/* Set byte order and enable the SD/MMC clock output. Also reset the FIFOs just for good measure. */
	MSP_SDICON = 0x13;
}

/* This function extracts arbitrary-length (up to 32bits) fields from the CID or CSD registers. */
static void GetBits (int start, int length, int longresponse, void *target)
{
	int i;
	unsigned char tmask = 1;
	unsigned short smask = 1 << (start & 15);
	unsigned char c = 0;
	unsigned char *pC = (unsigned char *)target;
	const unsigned short *pSTA = (unsigned short *)nMSP_SDIRSP0;
	int index;
	unsigned short source;

	/* Some sanity checking and find the index of the first bit in the registers */
	if (longresponse)
	{
		if ((start < 0) || (length < 0) || ((start + length)>128))
			return;
		index = 7 - (start >> 4);
	}
	else
	{
		if ((start < 0) || (length < 0) || ((start + length)>32))
			return;
		index = 1 - (start >> 4);
	}

	/* The order of the response registers is strange - pairs of SDIRSP* registers are seemingly
	 * reversed. So in ascending order of memory hword locations, the response is arranged as
	 * bits [111:96] [127:112] [79:61] [95:80] [47:32] [63:48] [15:0] [31:16]. This can all be
	 * handled by inverting bit 0 of the index at appropriate times. */
	index ^= 1;

	source = pSTA[index];

	/* Copy bits, remembering to invert bit 0 of the index before moving it along */
	for (i=0;i<length;i++)
	{
		if (source & smask)
			c|=tmask;

		smask <<= 1;
		if (!smask)
		{
			smask = 1;
			index ^= 1;
			index--;
			index ^= 1;
			source = pSTA[index];
		}

		tmask <<= 1;
		if (!tmask)
		{
			/* Got 8 bits of response - store it and start a new byte */
			*pC = c;
			pC++;
			c=0;
			tmask = 1;
		}
	}

	/* If there is a non-empty byte that hasn't been stored, store it. */
	if (tmask!=1)
		*pC=c;
}

static inline void ResetSdPeripheral(void)
{
	unsigned short temp;

	/* Reset the FIFOs */
	MSP_SDICON |= 2;
	while (MSP_SDICON & 2);

	/* Reset Command Status flags */
	temp = MSP_SDICMDSTA & 0x1e00;
	if (temp)
		MSP_SDICMDSTA = temp;

	/* Reset Data Status flags */
	temp = MSP_SDIDATSTA & 0x7FC;
	if (temp)
		MSP_SDIDATSTA = temp;

	/* Make sure the next transaction doesn't start for a few clock cycles */
	nLastTransactionTime = sdk2x_GetCurrentTime();
}

static int SdSendCommandBlocking (unsigned char command, unsigned int argument, int longresponse)
{
	unsigned short cmd = (unsigned short)command;
	int r = SDK2X_ERR_SDHW_OK;
	int timeout = sdk2x_GetCurrentTime();

	/* Give the card some clock cycles to sort itself out after the last transaction */
	sdk2x_TimerWait(nLastTransactionTime,nInterTransactionTime,2000);

	/* Send command immediately, wait for response */
	cmd |= 0x340;
	if (longresponse)
		cmd |= 0x400;

	/* Clear the error flags in MSP_SDICMDSTA */
	MSP_SDICMDSTA = 0x1e00;

	/* Copy argument */
	MSP_SDICMDARGL = (unsigned short)(argument & 0xffff);
	MSP_SDICMDARGH = (unsigned short)(argument >> 16);

#if (SDK2X_SD_DEBUG > 2)
	sprintf (sdk2x_private_string,"Command = 0x%04X (%i), Argument = %08X\r\n",cmd,command,argument);
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif

	/* Send the command */
	MSP_SDICMDCON = cmd;

	/* Wait for response */
	while ((!(MSP_SDICMDSTA & 0x200)) & (!r))
	{
		if (MSP_SDICMDSTA & 0x400) /* Timeout in response */
		{
			MSP_SDICMDSTA |= 0x400;
			r = SDK2X_ERR_SDHW_TIMEOUT;
		}
		if (sdk2x_GetElapsedMSecs(timeout) > COMMAND_RESPONSE_TIMEOUT_MS)
		{
			//sdk2x_UartSendString(0,"CMD: Timeout");
			return SDK2X_ERR_SDHW_NO_RESPONSE;
		}
	}

	if (!r)
	{
		MSP_SDICMDSTA |= 0x200;

		if (MSP_SDICMDSTA & 0x1000) /* CRC error in response */
		{
			MSP_SDICMDSTA |= 0x1000;
			r = SDK2X_ERR_SDHW_CRC;
		}
	}

#if (SDK2X_SD_DEBUG > 2)
	if (!r)
	{
		if (!longresponse)
			sprintf (sdk2x_private_string,"Command successful, response = %04X %04X\r\n",MSP_SDIRSP1,MSP_SDIRSP0);
		else
			sprintf (sdk2x_private_string,"Command successful, response = %04X %04X %04X %04X %04X %04X %04X %04X\r\n",
						MSP_SDIRSP1,MSP_SDIRSP0,MSP_SDIRSP3,MSP_SDIRSP2,MSP_SDIRSP5,MSP_SDIRSP4,MSP_SDIRSP7,MSP_SDIRSP6);
	}
	else
		sprintf (sdk2x_private_string,"Command failed, code = %d\r\n",r);

	sdk2x_UartSendString(0,sdk2x_private_string);
#endif

	/* Remember the time, so that the next transaction doesn't happen immediately */
	nLastTransactionTime = sdk2x_GetCurrentTime();

	return r;
}


#if (SDK2X_SD_DEBUG > 0)
static const int VddCurrMin[8] = { 1, 1, 5, 10, 25, 35, 60, 100 };
static const int VddCurrMax[8] = { 1, 5, 10, 25, 35, 45, 80, 200 };
static char NoString[] = "No";
static char YesString[] = "Yes";
#endif

static int SdHwTrySd (int retries)
{
	int i,r;
	int iterations = 0;
	int sd20 = 0;			/* SD 2.0 mode */
	int loops;
#if (SDK2X_SD_DEBUG > 0)
	unsigned char temp;
#endif

	/* Now send CMD0 to reset the card. Don't worry about a response. */
	MSP_SDICMDARGL = 0;
	MSP_SDICMDARGH = 0;
	MSP_SDICMDCON = 0x140;

	/* Wait for 74 SPICLK cycles, which is 370us at 200kHz. SD Data sheet also says add 1ms.
	* Just wait for 50ms. */
	sdk2x_MSleep(50);

	/* Wait for card to be initialised. This means sending CMD55 / ACMD41 until card indicates it is ready. */
	for (loops=0; loops < 2;)
	{
		/* Now we need to send CMD8 to detect SD2.0 cards. Other cards will harmlessly fail here */
		r=SdSendCommandBlocking (8,0x000001aa,0); /* param: [11:8] voltage flags; [7:0] check pattern */
		if (!r) sd20 = 1;

		/* Because ACMD41 is app specific command, we must send APP_CMD before it. */
		for (; iterations < retries; iterations++)
		{
			r=SdSendCommandBlocking (55,0,0);
			if (((MSP_SDIRSP0 & 0x120)==0x120) && (!r)) break;
		}

		if (iterations >= retries)
			return SDK2X_ERR_SDHW_TOO_MANY_ITERATIONS;

		/* Send ACMD41. First time, send it with 0 parameter, to get the card to respond with its allowed
		* operating voltages. Second and subsequent times, send it with the "3.3V" bit set. If the card
		* doesn't work at 3.3V (there won't be many of those), it will go into inactive state and look like
		* there's no card at all.
		* Additionally, set bit30 (HCS, host capacity support) bit if we detected SD2.0 card.
		*/
		if (!loops)
			r=SdSendCommandBlocking (41,0x00000000,0);
		else
			r=SdSendCommandBlocking (41,0x00200000|(sd20<<30),0);

		loops++;

		if (r)
		{
			iterations++;
			loops = 0;
		}
		else
		{
			if ((MSP_SDIRSP1 >> 15)&1)
			{
#if (SDK2X_SD_DEBUG > 1)
				sdk2x_UartSendString(0,"Card Ready.\r\n");
#endif
				break;
			}
			else
			{
				/* Only do this a limited amount of times. It has to be done at least twice, but if the card responds
				* "Card busy", go and do it again
				*/
				iterations++;
				if (loops == 2) loops = 0;
			}
		}
	}

	/* OK, card is initialised. Now send ALL_SEND_CID to get the card ID. */
	r = 1;

	for (i=0;(i<INIT_MAX_ITERATIONS) && (r);i++)
		r=SdSendCommandBlocking (2,0,1);

	if (r)
		return r;

#if (SDK2X_SD_DEBUG > 0)
	memset(&CID, 0, sizeof(CID));
	GetBits (120,8,1,&CID.ManufacturerID);
	GetBits (104,16,1,&CID.ApplicationID);
	GetBits (96,8,1,&CID.Name[0]);
	GetBits (88,8,1,&CID.Name[1]);
	GetBits (80,8,1,&CID.Name[2]);
	GetBits (72,8,1,&CID.Name[3]);
	GetBits (64,8,1,&CID.Name[4]);
	GetBits (60,4,1,&temp);
	CID.Revision = (int)(10*temp);
	GetBits (56,4,1,&temp);
	CID.Revision += (int)temp;
	GetBits (24,32,1,&CID.SerialNumber);
	GetBits (16,4,1,&temp);
	CID.Year = 2000 + (int)(10*temp);
	GetBits (12,4,1,&temp);
	CID.Year += (int)temp;
	GetBits (8,4,1,&CID.Month);

	if (CID.Name[0]<32) CID.Name[0]='.';
	if (CID.Name[1]<32) CID.Name[1]='.';
	if (CID.Name[2]<32) CID.Name[2]='.';
	if (CID.Name[3]<32) CID.Name[3]='.';
	if (CID.Name[4]<32) CID.Name[4]='.';
	CID.Name[5]='\0';

	sdk2x_UartSendString(0,"\r\nCard CID Contents:\r\n\r\n");

	sprintf (sdk2x_private_string,"Manufacturer ID   = %02X\r\n",CID.ManufacturerID);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string,"Application ID    = %04X\r\n",CID.ApplicationID);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string,"Product name      = %c%c%c%c%c\r\n",CID.Name[0],CID.Name[1],CID.Name[2],CID.Name[3],CID.Name[4]);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string,"Product revision  = %02X\r\n",CID.Revision);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string,"Serial Number     = %08X\r\n",CID.SerialNumber);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string,"Manufacture year  = %d\r\n",CID.Year);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string,"Manufacture month = %d\r\n",CID.Month);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string, "\r\nIterations taken  = %d\r\n",iterations);
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif

	return SDK2X_ERR_SDHW_OK;
}

static int SdHwTryMmc (int retries)
{
	int i,r;
	int iterations = 0;
	int CardReady = 0;
	int loops;
#if (SDK2X_SD_DEBUG > 0)
	unsigned char temp;
#endif

	/* Now send CMD0 to reset the card. Don't worry about a response. */
	MSP_SDICMDARGL = 0;
	MSP_SDICMDARGH = 0;
	MSP_SDICMDCON = 0x140;

	/* Wait for 74 SPICLK cycles, which is 370us at 200kHz. SD Data sheet also says add 1ms.
	* Just wait for 20ms. */
	sdk2x_MSleep(20);

	/* Wait for card to be initialised. This means sending CMD1 until card indicates it is ready. */
	for (loops=0;loops<2;loops++)
	{
		if (iterations >= retries)
			return SDK2X_ERR_SDHW_TOO_MANY_ITERATIONS;

		/* Send CMD1 with the "3.3V" bit set. If the card doesn't work at 3.3V (there won't be many of those),
		 * it will go into inactive state and look like there's no card at all.
		 */
		r=SdSendCommandBlocking (1,0x00200000,0);

		if (r)
		{
			iterations++;
			loops = 0;
		}
		else
		{
			if ((MSP_SDIRSP1 >> 15)&1)
			{
				CardReady = 1;
#if (SDK2X_SD_DEBUG > 1)
				sdk2x_UartSendString(0,"Card Ready.\r\n");
#endif
			}
			else
			{
				/* Only do this a limited amount of times. It has to be done at least twice, but if the card responds
				* "Card busy", go and do it again
				*/
				iterations++;
				if (!CardReady)
					loops=0;
			}
		}
	}

	/* OK, card is initialised. Now send ALL_SEND_CID to get the card ID. */
	r = 1;

	for (i=0;(i<INIT_MAX_ITERATIONS) && (r);i++)
		r=SdSendCommandBlocking (2,0,1);

	if (r)
		return r;

#if (SDK2X_SD_DEBUG > 0)
	GetBits (120,8,1,&CID.ManufacturerID);
	GetBits (104,16,1,&CID.ApplicationID);
	GetBits (96,8,1,&CID.Name[0]);
	GetBits (88,8,1,&CID.Name[1]);
	GetBits (80,8,1,&CID.Name[2]);
	GetBits (72,8,1,&CID.Name[3]);
	GetBits (64,8,1,&CID.Name[4]);
	GetBits (56,8,1,&CID.Name[5]);
	GetBits (52,4,1,&temp);
	CID.Revision = (int)(10*temp);
	GetBits (48,4,1,&temp);
	CID.Revision += (int)temp;
	GetBits (16,32,1,&CID.SerialNumber);
	GetBits (8,4,1,&temp);
	CID.Year = 1997 + (int)temp;
	GetBits (12,4,1,&CID.Month);

	if (CID.Name[0]<32) CID.Name[0]='.';
	if (CID.Name[1]<32) CID.Name[1]='.';
	if (CID.Name[2]<32) CID.Name[2]='.';
	if (CID.Name[3]<32) CID.Name[3]='.';
	if (CID.Name[4]<32) CID.Name[4]='.';
	if (CID.Name[5]<32) CID.Name[5]='.';

	sdk2x_UartSendString(0,"\r\nCard CID Contents:\r\n\r\n");

	sprintf (sdk2x_private_string,"Manufacturer ID   = %02X\r\n",CID.ManufacturerID);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string,"Application ID    = %04X\r\n",CID.ApplicationID);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string,"Product name      = %c%c%c%c%c%c\r\n",CID.Name[0],CID.Name[1],CID.Name[2],CID.Name[3],CID.Name[4],CID.Name[5]);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string,"Product revision  = %02X\r\n",CID.Revision);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string,"Serial Number     = %08X\r\n",CID.SerialNumber);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string,"Manufacture year  = %d\r\n",CID.Year);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string,"Manufacture month = %d\r\n",CID.Month);
	sdk2x_UartSendString(0,sdk2x_private_string);
	sprintf (sdk2x_private_string, "\r\nIterations taken  = %d\r\n",iterations);
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif

	return SDK2X_ERR_SDHW_OK;
}

int sdk2x_SdHwInit (void)
{
	int i,r;
#if (SDK2X_SD_DEBUG > 0)
	unsigned int size;
#endif
	int retries;
	unsigned int nTemp;

	sdk2x_SysTimerEnable();

	SetupSFRs();

	/* Check to see if the card is inserted */
	if (MSP_GPIOIPINLVL & 0x4000)
	{
	#if (SDK2X_SD_DEBUG > 0)
		sprintf (sdk2x_private_string,"NO MMC / SD Detected\r\n");
		sdk2x_UartSendString(0,sdk2x_private_string);
	#endif
		return SDK2X_ERR_SDHW_NO_RESPONSE;
	}

	#if (SDK2X_SD_DEBUG > 0)
		sprintf (sdk2x_private_string,"MMC / SD Inserted\r\n");
		sdk2x_UartSendString(0,sdk2x_private_string);
	#endif

	/* Only try to initialise the SD/MMC once */
	if (!mmc_init_once)
	{
		nLastTransactionTime = 0;

		/* Do the initial wait, but also reset the card and do the wait again. This will account for
		* a system which has just been powered up as well as one that's been warm-started. */

		/* Wait for 74 SPICLK cycles, which is 370us at 200kHz. SD Data sheet also says add 1ms.
		* Just wait for 20ms. */
		sdk2x_MSleep(20);

		sdk2x_SdCardInfo.Type = 1;
		r = 1;
		retries = 10;

		/* Alternately try the SD and MMC initialisations until the card shows some life. */
		for (i=0;(i<10) && (r);i++)
		{
			r=SdHwTryMmc(retries);
			if (r)
				r = SdHwTrySd(retries);
			else
				sdk2x_SdCardInfo.Type=0;

			if (r)
			{
				ResetSdPeripheral();
				// retries <<= 1;
				retries += retries/2;
			}
	#if (SDK2X_SD_DEBUG > 0)
			else
				nInitPass = i;
	#endif
		}

		if (r)
			return r;

	#if (SDK2X_SD_DEBUG > 0)
		sprintf (sdk2x_private_string,"\r\nType of card is %s\r\n",sdk2x_SdCardInfo.Type ? "SD":"MMC");
		sdk2x_UartSendString(0,sdk2x_private_string);
	#endif

		r = 1;

		for (i=0;(i<INIT_MAX_ITERATIONS) && (r);i++)
			r=SdSendCommandBlocking (3,0,0);			/* Get ident from the card */

		if (r)
			return SDK2X_ERR_SDHW_GET_ID;

		GetBits (16,16,0,&sdk2x_SdCardInfo.RelativeCardAddress);

	#if (SDK2X_SD_DEBUG > 0)
		sprintf (sdk2x_private_string,"Relative Card Address = 0x%04X\r\n",sdk2x_SdCardInfo.RelativeCardAddress);
		sdk2x_UartSendString(0,sdk2x_private_string);
	#endif

		/* Get CSD from the card */
		r=SdSendCommandBlocking (9,((unsigned int)sdk2x_SdCardInfo.RelativeCardAddress)<<16,1);

		if (r)
			return SDK2X_ERR_SDHW_GET_CSD;

		memset(&CSD, 0, sizeof(CSD));
		memset(&CsdFluff, 0, sizeof(CsdFluff));

		/* common stuff */
		GetBits (126,2,1,&CSD.CsdStructure);
		GetBits (96,8,1,&CSD.TranSpeed);
		GetBits (13,1,1,&CSD.PermWriteProtect);
		GetBits (12,1,1,&CSD.TempWriteProtect);

		if (CSD.CsdStructure > 1)
			return SDK2X_ERR_SDHW_UNSUPPORTED;

		if (CSD.CsdStructure == 1)
		{
			/* SDHC card */
	#if (SDK2X_SD_DEBUG > 1)
			sprintf (sdk2x_private_string,"SDHC card detected.\r\n");
			sdk2x_UartSendString(0,sdk2x_private_string);
	#endif

			GetBits (48,22,1,&CSD.CSize);

			/* SDHCs use mostly fixed values. Here we fill only fields we will need later */
			CSD.R2WFactor = 2;
			CSD.TAAC = 0xE;
			CSD.NSAC = 0;
			CSD.ReadBlkLength = 9;
			CSD.WriteBlkLength = 9;

			sdk2x_SdCardInfo.BlockSize = 1 << 9;
			sdk2x_SdCardInfo.NumBlocks = (CSD.CSize + 1) << 10;
			sdk2x_SdCardInfo.EraseBlocks = 0;			/* unused */
			sdk2x_SdCardInfo.WPBlocks = 0;				/* unused */

	#if (SDK2X_SD_DEBUG > 1)
			sprintf (sdk2x_private_string,"                      = %u kB\r\n",sdk2x_SdCardInfo.NumBlocks>>1);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"                      = %u MB\r\n",sdk2x_SdCardInfo.NumBlocks>>11);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"                      = %u GB\r\n\r\n",sdk2x_SdCardInfo.NumBlocks>>21);
			sdk2x_UartSendString(0,sdk2x_private_string);
	#endif
		}
		else
		{
			GetBits (112,8,1,&CSD.TAAC);
			GetBits (104,8,1,&CSD.NSAC);
			GetBits (80,4,1,&CSD.ReadBlkLength);
			GetBits (62,12,1,&CSD.CSize);
			GetBits (47,3,1,&CSD.CSizeMult);
			GetBits (39,7,1,&CSD.EraseBlkSize);
			GetBits (32,7,1,&CSD.WpGrpSize);
			GetBits (26,3,1,&CSD.R2WFactor);
			GetBits (22,4,1,&CSD.WriteBlkLength);
			GetBits (21,1,1,&CSD.WriteBlkPartial);

			sdk2x_SdCardInfo.BlockSize = 1 << ((unsigned int)CSD.ReadBlkLength);
			if (sdk2x_SdCardInfo.BlockSize != (1 << ((unsigned int)CSD.WriteBlkLength)))
			{
	#if (SDK2X_SD_DEBUG > 1)
				sprintf (sdk2x_private_string,"Read and Write Block sizes don't match! R = %d, W = %d\r\n",
					sdk2x_SdCardInfo.BlockSize,(1 << ((unsigned int)CSD.WriteBlkLength)));
				sdk2x_UartSendString(0,sdk2x_private_string);
	#endif
				return SDK2X_ERR_SDHW_RW_MISMATCH;
			}

			sdk2x_SdCardInfo.NumBlocks = ((unsigned int)CSD.CSize + 1) * (1 << ((unsigned int)(CSD.CSizeMult + 2)));
			sdk2x_SdCardInfo.EraseBlocks = ((unsigned int)(CSD.EraseBlkSize + 1));
			sdk2x_SdCardInfo.WPBlocks = (unsigned int)(CSD.WpGrpSize + 1);


	#if (SDK2X_SD_DEBUG > 0)
			GetBits (84,12,1,&CsdFluff.CommandClasses);
			GetBits (79,1,1,&CsdFluff.ReadBlkPartial);
			GetBits (78,1,1,&CsdFluff.WriteBlkMisalign);
			GetBits (77,1,1,&CsdFluff.ReadBlkMisalign);
			GetBits (76,1,1,&CsdFluff.DsrImp);
			GetBits (59,3,1,&CsdFluff.VddRCurrMin);
			GetBits (56,3,1,&CsdFluff.VddRCurrMax);
			GetBits (53,3,1,&CsdFluff.VddWCurrMin);
			GetBits (50,3,1,&CsdFluff.VddWCurrMax);
			GetBits (46,1,1,&CsdFluff.EraseBlkEnable);
			GetBits (31,1,1,&CsdFluff.WpGrpEnable);
			GetBits (15,1,1,&CsdFluff.FileFormatGrp);
			GetBits (14,1,1,&CsdFluff.Copy);
			GetBits (10,2,1,&CsdFluff.FileFormat);

			sdk2x_UartSendString(0,"\r\nCard CSD Contents:\r\n\r\n");

			sprintf (sdk2x_private_string,"CSD Structure Version = %d\r\n",CSD.CsdStructure);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Command Classes       = 0x%04X\r\n",CsdFluff.CommandClasses);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Read Block Partial    = %s\r\n",CsdFluff.ReadBlkPartial ? YesString : NoString);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Write Block Misalign  = %s\r\n",CsdFluff.WriteBlkMisalign ? YesString : NoString);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Read Block Misalign   = %s\r\n",CsdFluff.ReadBlkMisalign ? YesString : NoString);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"DSR Implemented       = %s\r\n",CsdFluff.DsrImp ? YesString : NoString);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Vdd Read Current Min  = %d mA\r\n",VddCurrMin[CsdFluff.VddRCurrMin]);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Vdd Read Current Min  = %d mA\r\n",VddCurrMax[CsdFluff.VddRCurrMax]);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Vdd Write Current Min = %d mA\r\n",VddCurrMin[CsdFluff.VddWCurrMin]);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Vdd Write Current Min = %d mA\r\n",VddCurrMax[CsdFluff.VddWCurrMax]);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Erase Block Enable    = %s\r\n",CsdFluff.EraseBlkEnable ? YesString : NoString);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"W-Protect Grp Enable  = %s\r\n",CsdFluff.WpGrpEnable ? YesString : NoString);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Write Block Partial   = %s\r\n",CSD.WriteBlkPartial ? YesString : NoString);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"File Format Group     = %d\r\n",CsdFluff.FileFormatGrp);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Copy                  = %s\r\n",CsdFluff.Copy ? YesString : NoString);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"File Format           = %d\r\n",CsdFluff.FileFormat);
			sdk2x_UartSendString(0,sdk2x_private_string);

			sdk2x_UartSendString(0,"\r\nMore Important Stuff:\r\n\r\n");

			sprintf (sdk2x_private_string,"Block Size            = %d bytes\r\n",sdk2x_SdCardInfo.BlockSize);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Number of Blocks      = %d\r\n",sdk2x_SdCardInfo.NumBlocks);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Erase Block Size      = %d\r\n",sdk2x_SdCardInfo.EraseBlocks);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Transfer Speed        = %d Hz\r\n",sdk2x_SdCardInfo.TransferSpeed);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Write Prot Block Size = %d\r\n",sdk2x_SdCardInfo.WPBlocks);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"Write Protect         = %s\r\n",
				(CSD.PermWriteProtect || CSD.TempWriteProtect) ? YesString : NoString);
			sdk2x_UartSendString(0,sdk2x_private_string);

			size = sdk2x_SdCardInfo.EraseBlocks * sdk2x_SdCardInfo.BlockSize;
			sprintf (sdk2x_private_string,"\r\nErase Sector Size     = %u bytes\r\n",size);
			sdk2x_UartSendString(0,sdk2x_private_string);

			size = sdk2x_SdCardInfo.EraseBlocks * sdk2x_SdCardInfo.BlockSize * sdk2x_SdCardInfo.WPBlocks;
			sprintf (sdk2x_private_string,"Write Prot Chunk Size = %u bytes\r\n",size);
			sdk2x_UartSendString(0,sdk2x_private_string);

			size = (unsigned long long)sdk2x_SdCardInfo.BlockSize * sdk2x_SdCardInfo.NumBlocks;
			sprintf (sdk2x_private_string,"\r\nCard Capacity         = %u bytes\r\n",size);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"                      = %u kB\r\n",size>>10);
			sdk2x_UartSendString(0,sdk2x_private_string);
			sprintf (sdk2x_private_string,"                      = %u MB\r\n\r\n",size>>20);
			sdk2x_UartSendString(0,sdk2x_private_string);
	#endif
		}

		sdk2x_SdCardInfo.TransferSpeed = TranSpeedRate[CSD.TranSpeed & 7] * TranSpeedMult[(CSD.TranSpeed>>3)&0x0f];
		sdk2x_SdCardInfo.AccessTime = ((TaacValue[(CSD.TAAC >> 3) & 0x0f] * TaacExponent[CSD.TAAC & 0x07]) / 10000000) + 1;
		sdk2x_SdCardInfo.AccessClocks = ((int)CSD.NSAC) * 100;
		sdk2x_SdCardInfo.WriteProtect = (CSD.PermWriteProtect || CSD.TempWriteProtect) ? 1 : 0;

		if (MSP_GPIODPINLVL & 0x04)
			sdk2x_SdCardInfo.WriteProtect = 1;

	#if (SDK2X_SD_DEBUG > 0)
		if (sdk2x_SdCardInfo.WriteProtect)
			sdk2x_UartSendString(0,"CARD IS WRITE PROTECTED\r\n");
	#endif

		sdk2x_SdCardInfo.IsSelected = 0;

		/* The CPU data sheet mentions maxima for MMC and SD of 10MHz and 25MHz respectively. */
		if (sdk2x_SdCardInfo.Type)
		{
			if (sdk2x_SdCardInfo.TransferSpeed > 25000000)
				sdk2x_SdCardInfo.TransferSpeed = 25000000;
		}
		else
		{
			if (sdk2x_SdCardInfo.TransferSpeed > 10000000)
				sdk2x_SdCardInfo.TransferSpeed = 10000000;
		}

		if ( sdk2x_SdCardInfo.TransferSpeed )
		{
			nTemp = fMMC / sdk2x_SdCardInfo.TransferSpeed;
			if (nTemp > 255)  /* This is really daft, never going to happen, but paranoia etc etc */
				nTemp = 255;
			psHighSpeed = (unsigned char)nTemp;
			/* Give an inter-transaction time of 100 clocks */
			nInterTransactionTime = 100000000 / sdk2x_SdCardInfo.TransferSpeed;
			nFastTransactionDelay = nInterTransactionTime;
		}
		else
		{
			nFastTransactionDelay = 1250;
			psHighSpeed = psLowSpeed;
		}

		SetPrescalerAndAccessTimes (psHighSpeed);

	#if (SDK2X_SD_DEBUG > 0)
		sprintf (sdk2x_private_string,"Card frequency  = %d Hz (prescaler = %d)\r\n",fMMC / (psHighSpeed + 1), psHighSpeed);
		sdk2x_UartSendString(0,sdk2x_private_string);
	#endif

		/* Fill in u-boot stuff */
		mmc_dev.if_type = IF_TYPE_MMC;
		mmc_dev.part_type = PART_TYPE_DOS;
		mmc_dev.dev = 0;
		mmc_dev.lun = 0;
		mmc_dev.type = 0;
		/* FIXME fill in the correct size (is set to 32MByte) */
		/* FIXME 2: FAT code is hardcoded to use 512B block size, so we pretend we use that */
		mmc_dev.blksz = 512; /* sdk2x_SdCardInfo.BlockSize; */
		mmc_dev.lba = sdk2x_SdCardInfo.NumBlocks << (CSD.ReadBlkLength - 9);

		sprintf(mmc_dev.vendor,"Man %02x App %04x Snr %08x", CID.ManufacturerID, CID.ApplicationID, CID.SerialNumber);
		sprintf(mmc_dev.product,"%s",CID.Name);
		sprintf(mmc_dev.revision,"%08x",CID.Revision);
		mmc_dev.removable = 0;
		mmc_dev.block_read = mmc_bread;
#if 1
		if (sdk2x_SdCardInfo.BlockSize != 512)
		{
			/* Since most u-boot FAT code assumes block size of 512, and there is no
			 * conversion code anywhere, and other than 512B block reads fail with CRC errors,
			 * we just force the block size to 512 here.
			 * Also from Linux kernel sources:
			 *   Both SD and MMC specifications state (although a bit
			 *   unclearly in the MMC case) that a block size of 512
			 *   bytes must always be supported by the card.
			 */
			printf("Warning: forcing MMC/SD block size from %i to 512\n", sdk2x_SdCardInfo.BlockSize);
			sdk2x_SdCardInfo.BlockSize = 512;
			CSD.ReadBlkLength = CSD.WriteBlkLength = 9;
		}
#endif
		MSP_SDIBSIZE = sdk2x_SdCardInfo.BlockSize;

		mmc_init_once = 1;
	}
	return SDK2X_ERR_SDHW_OK;
}

static int WaitForReady (void)
{
	int i,r;

	for (i=0;i<1000;i++)
	{
#if (SDK2X_SD_DEBUG > 1)
		sdk2x_UartSendString(0," .WFR.");
#endif

		r=SdSendCommandBlocking (13,((unsigned int)sdk2x_SdCardInfo.RelativeCardAddress)<<16,0);

		if (r)
			return r;

		if (MSP_SDIRSP0 & 0x100)
		{
#if (SDK2X_SD_DEBUG > 1)
			sprintf (sdk2x_private_string,"Card ready after %d poll(s)\r\n",i+1);
			sdk2x_UartSendString(0,sdk2x_private_string);
#endif
			return 0;
		}
	}

	return SDK2X_ERR_SDHW_NOT_READY;
}

/* TODO: I haven't managed to get DMA working, as the DMA transfer refuses to start.
 * Register setup so far:
 * MSP_SRC0LADDR = (unsigned short)(nMSP_SDIDAT&0xffff);
 *	MSP_SRC0HADDR = (unsigned short)(nMSP_SDIDAT>>16);
 *	MSP_TRG0LADDR = 0;
 *	MSP_TRG0HADDR = 0x0200;
 *
 *	MSP_DCH0SRM = 0x40 | 20;
 *	MSP_DCH0TRM = 0;
 *
 *	MSP_DMA0CONS = 8;
 *	MSP_DMA0COM2 = 511;
 *	MSP_DMA0COM1 = 1;
 *	MSP_DMA0COM0 = 0x1020;
 *	MSP_DMA0CONS = 0x408;
 */

static int SetupForTransfer (int isWrite, int blocks)
{
	int r;

	/* Reset the FIFOs */
	MSP_SDICON |= 2;
	while (MSP_SDICON & 2);

	MSP_SDIBSIZE = sdk2x_SdCardInfo.BlockSize;
	MSP_SDIDATSTA = 0x7fc;
	MSP_SDIDTIMERL = 0xffff;
	MSP_SDIDTIMERH = 0x001f;

	if (!sdk2x_SdCardInfo.IsSelected)
	{
		/* Select the card */
		r=SdSendCommandBlocking (7,((unsigned int)sdk2x_SdCardInfo.RelativeCardAddress)<<16,0);
		if (r)
			return SDK2X_ERR_SDHW_CARD_SELECT;

		if (CSD.CsdStructure != 1) /* We don't need to SET_BLOCKLEN for SDHCs, as it is fixed to 512 */
		{
			r=SdSendCommandBlocking (16,sdk2x_SdCardInfo.BlockSize,0);
			if (r)
				return SDK2X_ERR_SDHW_SET_BLOCK_SIZE;
		}

		sdk2x_SdCardInfo.IsSelected = 1;
	}

	/* Set up 4-bit comms mode and pre-erase function, only for SD cards */
	if (sdk2x_SdCardInfo.Type)
	{
		if (isWrite)
		{
			/* Don't give an error for this, as it's not essential */
			r=SdSendCommandBlocking (55,((unsigned int)sdk2x_SdCardInfo.RelativeCardAddress)<<16,0);

			if (!r)
				SdSendCommandBlocking (23,blocks,0);
		}

		if (!n4BitMode)
		{
			/* Again, don't give an error for this as it's not essential */
			r=SdSendCommandBlocking (55,((unsigned int)sdk2x_SdCardInfo.RelativeCardAddress)<<16,0);

			if (!r)
				r = SdSendCommandBlocking (6,2,0);

			if (!r)
				n4BitMode = 1;
		}
	}

	/* 1-bit mode for MMC, 4-bit mode for SD */
#if (SDK2X_SD_DEBUG > 2)
	sprintf (sdk2x_private_string,"Setting up data transfer in %d-bit mode\r\n",n4BitMode ? 4 : 1 );
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif

	MSP_SDIDATCONH = n4BitMode ? 0x000b : 0x000a;

	if (isWrite)
		MSP_SDIDATCONL = 0x3000 | blocks;			/* Data direction is "write" */
	else
		MSP_SDIDATCONL = 0x2000 | blocks;			/* Data direction is "read" */

	return SDK2X_ERR_SDHW_OK;
}

static int sdk2x_SdReadPagesInner (unsigned int address, void *dest, unsigned short numblocks)
{
	int r,r2;
	unsigned int i;
	unsigned char *pD;
	int timeout;
	int elapsed = 0;
	unsigned int bytestotransfer;

	if ((numblocks < 1))// || (numblocks > 64))
		return SDK2X_ERR_SDHW_PARAMETER;

	pD = (unsigned char *)dest;

#if (SDK2X_SD_DEBUG > 1)
	sprintf (sdk2x_private_string, "Reading %d blocks from card address 0x%08X to memory address %p\r\n",
				numblocks,address,pD);
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif

	if (CSD.CsdStructure == 1)
	{
		/* We do block addressing with SDHC cards */
		if (address & 0x1ff)
			return SDK2X_ERR_SDHW_UNSUPPORTED;
		address >>= 9;
	}

	r = SetupForTransfer(0,numblocks);
	if (r)
		return r;

	r=SdSendCommandBlocking (18,address,0);

	if (r)
	{
#if (SDK2X_SD_DEBUG > 0)
		sprintf (sdk2x_private_string, "Read multiple command rejected - %d\r\n",r);
		sdk2x_UartSendString(0,sdk2x_private_string);
#endif
		return SDK2X_ERR_SDHW_READ_MULTIPLE;
	}

	n4BitMode = 0;
	bytestotransfer = (numblocks * sdk2x_SdCardInfo.BlockSize);
	timeout = (int)MSP_TCOUNT;

	for (i=0;(i<bytestotransfer) && (!r);i++)
	{
		while (!(MSP_SDIFSTA & 0x1000))
		{
			nCardError = MSP_SDIDATSTA;

			if (nCardError & 0x400)
				MSP_SDIDATSTA |= 0x400;

			if (nCardError & 0x3e4)
			{
				r = SDK2X_ERR_SDHW_TRANSFER_ERROR;
				break;
			}

			elapsed = ((int)MSP_TCOUNT) - timeout;
#if (SDK2X_SD_DEBUG > 0)
			if (elapsed < 0)
			{
				sprintf(sdk2x_private_string,"Elapsed is negative! 1 %d\r\n",elapsed);
				sdk2x_UartSendString(0,sdk2x_private_string);
			}
#endif
			if (elapsed > nAccessTimeRd)
			{
				r = SDK2X_ERR_SDHW_FIFO_TIMEOUT;
				break;
			}
		}

		pD[i] = MSP_SDIDAT;
		timeout = (int)MSP_TCOUNT;
	}

	if (!r)
	{
		while (!(MSP_SDIDATSTA & 0x10))
		{
			elapsed = ((int)MSP_TCOUNT) - timeout;
#if (SDK2X_SD_DEBUG > 0)
			if (elapsed < 0)
			{
				sprintf(sdk2x_private_string,"Elapsed is negative! 2 %d\r\n",elapsed);
				sdk2x_UartSendString(0,sdk2x_private_string);
			}
#endif
			if (elapsed > nAccessTimeRd)
			{
				r = SDK2X_ERR_SDHW_FIFO_TIMEOUT_AT_END;
#if (SDK2X_SD_DEBUG > 0)
				sprintf (sdk2x_private_string,"FIFO Timeout on read, data remaining = %04X\r\n",MSP_SDIDATCNTL);
				sdk2x_UartSendString(0,sdk2x_private_string);
#endif
				break;
			}
		}
	}

	if (r)
	{
		nCardError = MSP_SDIDATSTA;
		MSP_SDIDATSTA = nCardError;
#if (SDK2X_SD_DEBUG > 0)
		sprintf (sdk2x_private_string,"Data read error - code = 0x%04X, card error = %04X  ************************\r\n",r,nCardError);
		sdk2x_UartSendString(0,sdk2x_private_string);
#endif
	}

	r2 = r;

	MSP_SDIDATCONL |= 0x4000;

	r=SdSendCommandBlocking (12,0,0);

#if (SDK2X_SD_DEBUG > 1)
	sprintf (sdk2x_private_string,"Data status = 0x%04X\r\n",MSP_SDIDATSTA);
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif

	nCardError = MSP_SDIDATSTA & 0x3e4;
	if (nCardError)
		return SDK2X_ERR_SDHW_TRANSFER_ERROR;

	MSP_SDIDATSTA = 0x7fc;
	MSP_SDIDATCONL = 0;
	MSP_SDIDATCONH = 0;

	if (r)
		return SDK2X_ERR_SDHW_READ_MULTIPLE_STOP;
#if (SDK2X_SD_DEBUG > 4)
	DumpSector(dest);
#endif

	return r2;
}

static int sdk2x_SdWritePagesInner (unsigned int address, const void *source, unsigned short numblocks)
{
	int r,r2;
	unsigned int i;
	const unsigned char *pS;
	int timeout;
	int elapsed = 0;
	unsigned int bytestotransfer;

	if ((numblocks < 1) || (numblocks > 64))
		return SDK2X_ERR_SDHW_PARAMETER;

	pS = (const unsigned char *)source;

#if (SDK2X_SD_DEBUG > 1)
	sprintf (sdk2x_private_string, "Writing %d blocks from memory address %p to card address 0x%08X\r\n",
				numblocks,pS,address);
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif

	if (CSD.CsdStructure == 1)
	{
		/* We do block addressing with SDHC cards */
		if (address & 0x1ff)
			return SDK2X_ERR_SDHW_UNSUPPORTED;
		address >>= 9;
	}

	r = SetupForTransfer(1,numblocks);
	if (r)
		return r;

	r=SdSendCommandBlocking (25,address,0);

	if (r)
		return SDK2X_ERR_SDHW_WRITE_MULTIPLE;

	timeout = (int)MSP_TCOUNT;
	bytestotransfer = numblocks * sdk2x_SdCardInfo.BlockSize;
	n4BitMode = 0;

	for (i=0;(i<bytestotransfer) && (!r);i++)
	{
		while (!(MSP_SDIFSTA & 0x2000))
		{
			nCardError = MSP_SDIDATSTA;

			if (nCardError & 0x400)
				MSP_SDIDATSTA |= 0x400;

			if (nCardError & 0x3e4)
			{
				r = SDK2X_ERR_SDHW_TRANSFER_ERROR;
				break;
			}

			elapsed = ((int)MSP_TCOUNT) - timeout;
#if (SDK2X_SD_DEBUG > 0)
			if (elapsed < 0)
			{
				sprintf(sdk2x_private_string,"Elapsed is negative! 3 %d\r\n",elapsed);
				sdk2x_UartSendString(0,sdk2x_private_string);
			}
#endif
			if (elapsed > nAccessTimeWr)
			{
				r = SDK2X_ERR_SDHW_FIFO_TIMEOUT;
				break;
			}
		}

		MSP_SDIDAT = pS[i];
		timeout = (int)MSP_TCOUNT;
	}

	if (!r)
	{
		while (!(MSP_SDIFSTA & 0x400))
		{
			elapsed = ((int)MSP_TCOUNT) - timeout;
#if (SDK2X_SD_DEBUG > 0)
			if (elapsed < 0)
			{
				sprintf(sdk2x_private_string,"Elapsed is negative! 4 %d\r\n",elapsed);
				sdk2x_UartSendString(0,sdk2x_private_string);
			}
#endif
			if (elapsed > nAccessTimeWr)
			{
				r = SDK2X_ERR_SDHW_FIFO_TIMEOUT_AT_END;
#if (SDK2X_SD_DEBUG > 0)
				sprintf (sdk2x_private_string,"FIFO Timeout on write, data remaining = %04X\r\n",MSP_SDIDATCNTL);
				sdk2x_UartSendString(0,sdk2x_private_string);
#endif
				break;
			}
		}
	}

#if (SDK2X_SD_DEBUG > 0)
	if (r)
	{
		sprintf (sdk2x_private_string,"Data write error - code = 0x%04X, card error = %04X ************************\r\n",r,nCardError);
		sdk2x_UartSendString(0,sdk2x_private_string);
	}
#endif

	r2=r;

	/* Wait for 100 clocks, to allow the data that's been sent to arrive at the card */
	sdk2x_USleep(nInterTransactionTime);

	MSP_SDIDATCONL |= 0x4000;

	r=SdSendCommandBlocking (12,0,0);

#if (SDK2X_SD_DEBUG > 1)
	sprintf (sdk2x_private_string,"Data status = 0x%04X\r\n",MSP_SDIDATSTA);
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif

	nCardError = MSP_SDIDATSTA & 0x3e4;
	if (nCardError)
		return SDK2X_ERR_SDHW_TRANSFER_ERROR_AT_STOP;

	MSP_SDIDATSTA = 0x7fc;
	MSP_SDIDATCONL = 0;
	MSP_SDIDATCONH = 0;

	if (r)
		return SDK2X_ERR_SDHW_WRITE_MULTIPLE_STOP;

	r = WaitForReady();

	if (r)
		return r;

	r = MSP_SDIRSP1 & 0xFFF9;
	if (r)
		return r;

	return r2;
}

int sdk2x_SdReadPages (unsigned int address, void *dest, unsigned short numblocks)
{
	int z;
	int r = 1;
	int slow = 0;
	/* Check to see if the card is inserted */
	if (MSP_GPIOIPINLVL & 0x4000)
	{
	#if (SDK2X_SD_DEBUG > 0)
		sprintf (sdk2x_private_string,"NO MMC / SD Detected\r\n");
		sdk2x_UartSendString(0,sdk2x_private_string);
	#endif
		return SDK2X_ERR_SDHW_NO_RESPONSE;
	}

#if (SDK2X_SD_DEBUG > 2)
	sprintf (sdk2x_private_string,"Reading %d pages from address %08X\r\n",numblocks,address);
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif
	// ART103
	ResetSdPeripheral();

	for (z=0; (z<MAX_RETRIES) && (r); z++)
	{
		r = sdk2x_SdReadPagesInner (address, dest, numblocks);
		if (r)
		{
			if (slow)
				sdk2x_MSleep(200);
			else
			{
#if (SDK2X_SD_DEBUG > 0)
				sprintf (sdk2x_private_string,"Read pages failed (%d:0x%04X)... falling back to slow data rate\r\n",r,nCardError);
				sdk2x_UartSendString(0,sdk2x_private_string);
#endif
				SetPrescalerAndAccessTimes (psLowSpeed);
				nInterTransactionTime = 1250;
				slow = 1;
			}
			WaitForReady();
			ResetSdPeripheral();
		}
	}

	if (slow)
	{
		SetPrescalerAndAccessTimes(psHighSpeed);
		nInterTransactionTime = nFastTransactionDelay;
	}

	return r;
}

int sdk2x_SdWritePages (unsigned int address, const void *source, unsigned short numblocks)
{
	int z;
	int r = 1;
	int slow = 0;

	/* Check to see if the card is inserted */
	if (MSP_GPIOIPINLVL & 0x4000)
	{
	#if (SDK2X_SD_DEBUG > 0)
		sprintf (sdk2x_private_string,"NO MMC / SD Detected\r\n");
		sdk2x_UartSendString(0,sdk2x_private_string);
	#endif
		return SDK2X_ERR_SDHW_NO_RESPONSE;
	}

#if (SDK2X_SD_DEBUG > 2)
	sprintf (sdk2x_private_string,"Writing %d pages to address %08X\r\n",numblocks,address);
	sdk2x_UartSendString(0,sdk2x_private_string);
#endif

	if (sdk2x_SdCardInfo.WriteProtect)
		return SDK2X_ERR_SDHW_WRITE_PROTECT;

	for (z=0; (z<MAX_RETRIES) && (r); z++)
	{
		r = sdk2x_SdWritePagesInner (address, source, numblocks);
		if (r)
		{
			if (slow)
				sdk2x_MSleep(200);
			else
			{
#if (SDK2X_SD_DEBUG > 0)
				sprintf (sdk2x_private_string,"Write pages failed (%d:0x%04X)... falling back to slow data rate\r\n",r,nCardError);
				sdk2x_UartSendString(0,sdk2x_private_string);
#endif
				SetPrescalerAndAccessTimes(psLowSpeed);
				nInterTransactionTime = 1250;
				slow = 1;
			}
			ResetSdPeripheral();
			WaitForReady();
		}
	}

	if (slow)
	{
		SetPrescalerAndAccessTimes(psHighSpeed);
		nInterTransactionTime = nFastTransactionDelay;
	}

	return r;
}


/* U-Boot MMc API implementation */
int mmc_init(int verbose)
{
	return sdk2x_SdHwInit();
}


int mmc_read(ulong src, uchar *dst, int size)
{
	return sdk2x_SdReadPages (src, dst, size);
}


int mmc_write(uchar *src, ulong dst, int size)
{
	return sdk2x_SdWritePages (dst, src, size);
}


int mmc2info(ulong addr)
{
return 1;
}

