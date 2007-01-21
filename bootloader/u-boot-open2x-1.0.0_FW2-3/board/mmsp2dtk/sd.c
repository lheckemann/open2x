//[*]----------------------------------------------------------------------------------------------------[*]
/*
 *      A     EEEE  SSSS   OOO  PPPP 
 *     A A    E     S     O   O P   P
 *    AAAAA   EEEE  SSSS  O   O PPPP
 *   A     A  E         S O   O P
 *   A     A  EEEE  SSSS   OOO  P
 *
 *  An Entertainment Solution On a Platform (AESOP) is a completely Open Source 
 *  based graphical user environment and suite of applications for PDAs and other 
 *  devices running Linux. It is included in various embedded Linux distributions 
 *  such as OpenZaurus - http://www.aesop-embedded.org 
 *
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
 *
 *	Title           : sd.c
 *	Author          : 
 *	Created date    : 2005. 06. 26. 23:05:59 KST
 *	Description     : 
 *
 *	$Revision: 1.2 $
 *	$Log: sd.c,v $
 *	Revision 1.2  2005/07/07 15:46:59  linux4go
 *	*** empty log message ***
 *	
 *	Revision 1.1.1.1  2005/06/27 17:03:53  linuxpark
 *	Initial import.
 *	
 *	
 *
 */ 
//[*]----------------------------------------------------------------------------------------------------[*]
#ident  "@(*) $Header: /cvsroot/aesop-embedded/u-boot-aesop/board/aesop2440/sd.c,v 1.2 2005/07/07 15:46:59 linux4go Exp $"
//[*]----------------------------------------------------------------------------------------------------[*]
#include <common.h>
#include <mmsp20.h>
#include <part.h>
//[*]----------------------------------------------------------------------------------------------------[*]
#define UData(Data)				((unsigned long) (Data))
#define Fld(Size, Shft)			(((Size) << 16) + (Shft))
#define FSize(Field)			((Field) >> 16) 
#define FShft(Field)			((Field) & 0x0000FFFF)
#define FMsk(Field)				(((UData (1) << FSize (Field)) - 1) << FShft (Field))
#define FAlnMsk(Field)			((UData (1) << FSize (Field)) - 1)
#define F1stBit(Field)			(UData (1) << FShft (Field))
#define FInsrt(Value, Field) 	(UData (Value) << FShft (Field))
#define FExtr(Data, Field)		((UData (Data) >> FShft (Field)) & FAlnMsk (Field))
//[*]----------------------------------------------------------------------------------------------------[*]
#define rSDICON			(*(volatile unsigned short*	)0xC0001500)		/* SDI Control Register */
#define rSDIPRE			(*(volatile unsigned short*	)0xC0001502)		/* SDI Prescaler Register */
#define rSDICARG		(*(volatile unsigned int*	)0xC0001504)	/* SDI Command Argument Register */
#define rSDICCON		(*(volatile unsigned short*	)0xC0001508)		/* SDI Command Control Register */
#define rSDICSTA		(*(volatile unsigned short*	)0xC000150A)		/* SDI Command Status Register */
#define rSDIRSP0		(*(volatile unsigned int*	)0xC000150C)	/* SDI Response Register 0 */
#define rSDIRSP1		(*(volatile unsigned int*	)0xC0001510)	/* SDI Response Register 1 */
#define rSDIRSP2		(*(volatile unsigned int*	)0xC0001514)	/* SDI Response Register 2 */
#define rSDIRSP3		(*(volatile unsigned int*	)0xC0001518)	/* SDI Response Register 3 */
#define rSDIBSIZE		(*(volatile unsigned short*	)0xC000151E)		/* SDI Block Size Register */
#define rSDIDCON		(*(volatile unsigned int*	)0xC0001520)	/* SDI Data Control Register */
#define rSDIDCNT		(*(volatile unsigned int*	)0xC0001524)	/* SDI Data Remain Counter Register */
#define rSDIDSTA		(*(volatile unsigned short*	)0xC0001528)		/* SDI Data Status Register */
#define rSDIFSTA		(*(volatile unsigned short*	)0xC000152A)		/* SDI FIFO Status Register */
#define rSDIDAT			(*(volatile unsigned char*	)0xC000152C)	/* SDI Data Register */
#define rSDIIMSK		(*(volatile unsigned int*	)0xC0001530)	/* SDI Interrupt Mask Register 0 */
#define rSDIDTIMERL		(*(volatile unsigned short*	)0xC0001536)		/* SDI Data Timer Register */
#define rSDIDTIMERH		(*(volatile unsigned short*	)0xC0001538)		/* SDI Data Timer Register */
#define rSDISFTL		(*(volatile unsigned short*	)0xC000153A)		/* SDI Shift Regisrt Low */
#define rSDISFTH		(*(volatile unsigned short*	)0xC000153C)		/* SDI Shift Regisrt Low */
//[*]----------------------------------------------------------------------------------------------------[*]
#define SDIDCON_DATA_SIZE_WORD 		(2 << 22)	/* SDIO Interrupt period is ... */
#define SDIDCON_PRD					(1 << 21)	/* SDIO Interrupt period is ... */
				/* when last data block is transferred. */
#define SDIDCON_PRD_2				(0 << 21)	/* 0: exact 2 cycle */
#define SDIDCON_PRD_N				(1 << 21)	/* 1: more cycle */
#define SDIDCON_TARSP				(1 << 20)	/* when data transmit start ... */
#define SDIDCON_TARSP_0				(0 << 20)	/* 0: directly after DatMode set */
#define SDIDCON_TARSP_1				(1 << 20)	/* 1: after response receive */
#define SDIDCON_RACMD				(1 << 19)	/* when data receive start ... */
#define SDIDCON_RACMD_0				(0 << 19)	/* 0: directly after DatMode set */
#define SDIDCON_RACMD_1				(1 << 19)	/* 1: after command sent */
#define SDIDCON_BACMD				(1 << 18)	/* when busy receive start ... */
#define SDIDCON_BACMD_0				(0 << 18)	/* 0: directly after DatMode set */
#define SDIDCON_BACMD_1				(1 << 18)	/* 1: after command sent */
#define SDIDCON_BLK					(1 << 17)	/* transfer mode. 0:stream, 1:block */
#define SDIDCON_WIDE_0				(0 << 16)	/* Standard bus mode[only SDIDAT[0]] */
#define SDIDCON_WIDE_1				(1 << 16)	/* Enable Wide bus mode [SDIDAT[0:3]] */
#define SDIDCON_DMA					(1 << 15)	/* Enable DMA */
#define SDIDCON_DATA_START			(1 << 14)	/* 24x0a data transfer start */
#define SDIDCON_DATA_MODE_Rx 		(2 << 12)	/* which direction of data transfer */
#define SDIDCON_DATA_MODE_Tx 		(3 << 12)	/* which direction of data transfer */
#define SDIDCON_DATA_STOP 			~(7 << 12)	/* which direction of data transfer */
#define fSDIDCON_DatMode 			Fld(2,12)	/* which direction of data transfer */
#define SDIDCON_READY				FInsrt(0x0, fSDIDCON_DatMode)	/* ready */
#define SDIDCON_BUSY				FInsrt(0x1, fSDIDCON_DatMode)	/* only busy check */
#define SDIDCON_RX					FInsrt(0x2, fSDIDCON_DatMode)	/* Data receive */
#define SDIDCON_TX					FInsrt(0x3, fSDIDCON_DatMode)	/* Data transmit */
#define SDIDCON_BNUM				FMsk(Fld(12,0))	/* Block Number(0~4095) */
//[*]----------------------------------------------------------------------------------------------------[*]
extern int fat_register_device(block_dev_desc_t *dev_desc, int part_no);
//[*]----------------------------------------------------------------------------------------------------[*]
static block_dev_desc_t mmc_dev;
//[*]----------------------------------------------------------------------------------------------------[*]
block_dev_desc_t * mmc_get_dev(int dev)
{
	return ((block_dev_desc_t *)&mmc_dev);
}
//[*]----------------------------------------------------------------------------------------------------[*]
int isMMC;	// 1:MMC, 0:SD
int RCA=0;	// Relative card address
unsigned int CardSize=0; // SD capacity in byte		 
//[*]----------------------------------------------------------------------------------------------------[*]
// CSD register, rwe == read/write/erase 
typedef struct 
{
	unsigned char csd;		/* CSD structure */
	unsigned char spec_vers;		/* Spec version, MMC only */
	struct 
	{
		unsigned char man;	/* time mantissa */
		unsigned char exp;	/* time exponent */
	} taac;			/* Data read access-time-1 */
	unsigned char nsac;		/* Data read access-time-2 in CLK cycle */
	struct 
	{
		unsigned char man;	/* rate mantissa */
		unsigned char exp;	/* rate exponent */
	} tran_speed;		/* Max. data transfer rate */
	unsigned short ccc;		/* Card command classes */
	unsigned char read_len;		/* Max. read data block length */
	unsigned char read_part;		/* Partial blocks for read allowed */
	unsigned char write_mis;		/* write block misalignment */
	unsigned char read_mis;		/* read block misalignment */
	unsigned char dsr;		/* DSR implemented */
	unsigned short  c_size;		/* Device size */
	unsigned char vcc_r_min;		/* Max. read current at Vcc min */
	unsigned char vcc_r_max;		/* Max. read current at Vcc max */
	unsigned char vcc_w_min;		/* Max. write current at Vcc min */
	unsigned char vcc_w_max;		/* Max. write current at Vcc max */
	unsigned char c_size_mult;	/* Device size multiplier */
	unsigned char er_size;		/* Erase sector size, MMC only */
	unsigned char er_grp_size;	/* Erase group size, MMC only */
	unsigned char wp_grp_size;	/* Write protect group size */
	unsigned char wp_grp_en;		/* Write protect group enable */
	unsigned char dflt_ecc;		/* Manufacturer default ECC, MMC only */
	unsigned char r2w_factor;	/* Write speed factor */
	unsigned char write_len;		/* Max. write data block length */
	unsigned char write_part;	/* Partial blocks for write allowed */
	unsigned char ffmt_grp;		/* File format group, rw */
	unsigned char copy;		/* Copy flag (OTP), rw */
	unsigned char perm_wp;		/* Permanent write protection, rw */
	unsigned char tmp_wp;		/* temporary write protection, rwe */
	unsigned char ffmt;		/* file format, rw */
	unsigned char ecc;		/* ECC, rwe, MMC only */

	/* SD only */
	unsigned char er_blk_en;		/* Erase single block enable, SD only */
	unsigned char er_sec_size;	/* Erase sector size, SD only */
} CSD_regs;
//[*]----------------------------------------------------------------------------------------------------[*]
int Chk_CMDend(int cmd, int be_resp)
{
	int volatile finish0;

	if(!be_resp)    // No response
	{
		finish0 = rSDICSTA;
		while((finish0&0x800) != 0x800)	// Check cmd end
		{
			finish0=rSDICSTA;
		}
		rSDICSTA = finish0;// Clear cmd end state

		return 1;
	}
	else	// With response
	{
		finish0 = rSDICSTA;
		while( !( ((finish0&0x200)==0x200) | ((finish0&0x400)==0x400) ))    // Check cmd/rsp end
			finish0=rSDICSTA;

		if((cmd==1) | (cmd==9) | (cmd==41))	// CRC no check, CMD9 is a long Resp. command.
		{
			if( (finish0&0xf00) != 0xa00 )  // Check error
			{
				rSDICSTA = finish0;   // Clear error state
				if(((finish0&0x400)==0x400))
					return 0;	// Timeout error
			}
			rSDICSTA = finish0;	// Clear cmd & rsp end state
		}
		else	// CRC check
		{
			if( (finish0&0x1f00) != 0xa00 )	// Check error
			{				
				//printf("CMD%d:rSDICSTA=0x%x,rSDIRSP0=0x%x\n",cmd, rSDICSTA, rSDIRSP0);
				rSDICSTA = finish0;   // Clear error state

				if(((finish0&0x400)==0x400))
					return 0;	// Timeout error
			}
			rSDICSTA=finish0;
		}
		return 1;
	}
}
//[*]----------------------------------------------------------------------------------------------------[*]
int Chk_DATend(void)
{
	int finish;

	finish = rSDIDSTA;
	while( !( ((finish&0x10)==0x10) | ((finish&0x20)==0x20) ))	
	{
		// Chek timeout or data end
		finish = rSDIDSTA;
		//printf("DATA:finish=0x%x\n", finish);
	}
	if( (finish&0xfc) != 0x10 )
	{
		printf("DATA:finish=0x%x\n", finish); // Something wrong !!
		rSDIDSTA = 0xec;  // Clear error state
		return 0;
	}
	return 1;
}
//[*]----------------------------------------------------------------------------------------------------[*]
int Chk_BUSYend(void)
{
	int finish;

	finish = rSDIDSTA;
	while( !( ((finish&0x08)==0x08) | ((finish&0x20)==0x20) ))
		finish = rSDIDSTA;

	if( (finish&0xfc) != 0x08 )
	{
		printf("BUSY:finish=0x%x\n", finish);
		rSDIDSTA = 0xf4;  //clear error state
		return 0;
	}
	return 1;
}
//[*]----------------------------------------------------------------------------------------------------[*]
void CMD0(void)
{
	//-- Make card idle state 
	rSDICARG = 0x0;	    // CMD0(stuff bit)
	rSDICCON = (1<<8) | 0x40;   // No_resp, start, CMD0

	//-- Check end of CMD0
	Chk_CMDend(0, 0);
//	rSDICSTA=0x800;	    // Clear cmd_end(no rsp)
}
//[*]----------------------------------------------------------------------------------------------------[*]
int CMD55(void)
{
	//--Make ACMD
	//rSDICARG=RCA<<16;			//CMD7(RCA,stuff bit)
	rSDICARG = (RCA<<16) & 0xffff0000;	// CMD7(RCA,stuff bit)
	rSDICCON=(0x1<<9)|(0x1<<8)|0x77;	//sht_resp, wait_resp, start, CMD55

	//-- Check end of CMD55
	if(!Chk_CMDend(55, 1)) 
		return 0;

	//rSDICSTA=0xa00;	// Clear cmd_end(with rsp)
	return 1;
}
//[*]----------------------------------------------------------------------------------------------------[*]
CSD_regs csd;
//[*]----------------------------------------------------------------------------------------------------[*]
int CMD9(void)//SEND_CSD
{
	rSDICARG = (RCA<<16) & 0xffff0000;	// CMD7(RCA,stuff bit)
	rSDICCON = (0x1<<10) | (0x1<<9) | (0x1<<8) | 0x49;	// long_resp, wait_resp, start, CMD9
	
    if(!Chk_CMDend(9, 1)) 
		return 0;

    return 1;
}
//[*]----------------------------------------------------------------------------------------------------[*]
int Chk_MMC_OCR(void)
{
	int i;
	
    //-- Negotiate operating condition for MMC, it makes card ready state
    for(i=0;i<15;i++)
    {
    	rSDICARG=0xffc000;	    	    //CMD1(OCR:2.6V~3.6V)
    	rSDICCON=(0x1<<9)|(0x1<<8)|0x41;    //sht_resp, wait_resp, start, CMD1

    	//-- Check end of CMD1
    	if(Chk_CMDend(1, 1) & (rSDIRSP0==0x80ffc000)) 
		{
	    	//rSDICSTA=0xa00;	// Clear cmd_end(with rsp)
	    	return 1;	// Success
		}	
    }
    //rSDICSTA=0xa00;	// Clear cmd_end(with rsp)
    return 0;		// Fail
}
//[*]----------------------------------------------------------------------------------------------------[*]
int Chk_SD_OCR(void)
{
    int i=0;

    //-- Negotiate operating condition for SD, it makes card ready state
//    for(i=0;i<15;i++)
    for(i=0;i<150;i++)
    {
//    	printf("SD i : %d\t", i);
    	CMD55();    // Make ACMD

    	rSDICARG = 0x00ff8000;					// ACMD41(OCR:2.7V~3.6V)
    	rSDICCON = (0x1<<9) | (0x1<<8) | 0x69;	// sht_resp, wait_resp, start, ACMD41

		//-- Check end of ACMD41
	    if(Chk_CMDend(41, 1) & (rSDIRSP0==0x80ff8000)) 
		{
		    //rSDICSTA=0xa00;	// Clear cmd_end(with rsp)
		    return 1;	// Success	    
		}
		udelay(20000); // Wait Card power up status
    }
    //rSDICSTA=0xa00;	// Clear cmd_end(with rsp)
    return 0;		// Fail
    
    
    /*
    	struct mmcsd_cmd  cmd;
retry_sd_ocr:
	// CMD55, make ACMD 
	slot->rca = 0;
	cmd.cmd = MMCSD_CMD55;
	cmd.arg = (slot->rca << 16) & 0xffff0000;
	cmd.res_type = MMCSD_RES_TYPE_R1;
	cmd.res_flag = 0;
	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
    slot->send_cmd(slot, &cmd);
	
	// ACMD41, SEND_OP_COND 
	cmd.cmd = MMCSD_ACMD41;
	slot->ocr = MMCSD_VDD_27_36;
	cmd.arg = slot->ocr;
	cmd.res_type = MMCSD_RES_TYPE_R1;
	cmd.res_flag = MMCSD_RES_FLAG_NOCRC;
	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
	slot->send_cmd(slot, &cmd);

	if(SDIRSP0 == 0x80ff8000) {
		mdelay(10);	// wait card power up status
		return 1;	//success
	} else
		goto retry_sd_ocr;
	return 0;		// fail
	*/
}
//[*]----------------------------------------------------------------------------------------------------[*]
int CardBusWidth = 0;
//[*]----------------------------------------------------------------------------------------------------[*]
void SetBus(void)
{
SET_BUS:
	CMD55();	// Make ACMD
	//-- CMD6 implement
	rSDICARG = CardBusWidth<<1;	    	//Width 0: 1bit, 1: 4bit
	rSDICCON=(0x1<<9)|(0x1<<8)|0x46;	//sht_resp, wait_resp, start, CMD55

	if(!Chk_CMDend(6, 1))   // ACMD6
		goto SET_BUS;
	//rSDICSTA=0xa00;	    // Clear cmd_end(with rsp)
}
//[*]----------------------------------------------------------------------------------------------------[*]
void Card_sel_desel(char sel_desel)
{
	//-- Card select or deselect
	if(sel_desel)
	{
RECMDS7:
		rSDICARG = (RCA<<16) & 0xffff0000;	// CMD7(RCA,stuff bit)
		rSDICCON= (0x1<<9)|(0x1<<8)|0x47;   // sht_resp, wait_resp, start, CMD7

		//-- Check end of CMD7
		if(!Chk_CMDend(7, 1))
			goto RECMDS7;
		//rSDICSTA=0xa00;	// Clear cmd_end(with rsp)

		//--State(transfer) check
		if( rSDIRSP0 & (0x1e00!=0x800) )
			goto RECMDS7;
	}
	else
	{
RECMDD7:
		rSDICARG=0<<16;		//CMD7(RCA,stuff bit)
		rSDICCON=(0x1<<8)|0x47;	//no_resp, start, CMD7

		//-- Check end of CMD7
		if(!Chk_CMDend(7, 0))
			goto RECMDD7;
		//rSDICSTA=0x800;	// Clear cmd_end(no rsp)
	}
}
//[*]----------------------------------------------------------------------------------------------------[*]
void SD_Read(unsigned char * pucdata, unsigned long ulLBA, unsigned long ulSectors);
void SD_Write(unsigned char * pucdata, unsigned long ulLBA, unsigned long ulSectors);
unsigned int SD_card_init(void);
//[*]----------------------------------------------------------------------------------------------------[*]
ulong mmc_bread(int dev_num, ulong blknr, ulong blkcnt, ulong *dst)
{
//	int mmc_block_size = MMC_BLOCK_SIZE;
//	ulong src = blknr * mmc_block_size + CFG_MMC_BASE;

	if(blkcnt==0)
	{
		printf("Why block_cnt == 0?? \n");
		return 0;
	}
//	mmc_read(src, (uchar *)dst, blkcnt*mmc_block_size);
//	printf("mmc_bread LBA=%d, Count=%d\n",blknr,blkcnt);
	SD_Read((uchar *)dst, blknr, blkcnt);
//	printf("mmc_bread end\n");	
	return blkcnt;
}
//[*]----------------------------------------------------------------------------------------------------[*]
int mmc_write(uchar *src, ulong dst, int size)
{
	printf("\nmmc_write should not be called !!!!\n");
	//SD_Write(
	return 0;
}
//[*]----------------------------------------------------------------------------------------------------[*]
int mmc_read(ulong src, uchar *dst, int size)
{
	printf("\nmmc_read should not be called !!!!\n");
	//SD_Write(
	return 0;
}
//[*]----------------------------------------------------------------------------------------------------[*]
#define INICLK	400000
#define SDCLK	25000000	
#define MMCCLK	15000000	
//[*]----------------------------------------------------------------------------------------------------[*]
int mmc_init_once = 0;
//[*]----------------------------------------------------------------------------------------------------[*]
int mmc_init(int verbose)
{
	SD_card_init();
	return 0;
}
//[*]----------------------------------------------------------------------------------------------------[*]
#define CFG_MMC_BASE 0
//[*]----------------------------------------------------------------------------------------------------[*]
int mmc2info(ulong addr)
{
	/* FIXME hard codes to 32 MB device */
	if (addr >= CFG_MMC_BASE && addr < CFG_MMC_BASE + (CardSize/512))
	{
		return 1;
	}
	return 0;
}
//[*]----------------------------------------------------------------------------------------------------[*]
#define MMCSD_PCLK			50000000	// 50 MHz
#define MMCSD_MMC_NORCLK	8000000		//  8 MHz
#define MMCSD_SD_NORCLK		2000000		//  2 MHz
#define MMCSD_INICLK		400000		// 400 KHz
//[*]----------------------------------------------------------------------------------------------------[*]
unsigned int SD_card_init(void)
{
	int i;
	unsigned long pclk;

	if(mmc_init_once == 1) return 1;	// already done !!
	
	rSDIDCON = 0;
	rSDIPRE = 0;
	rSDICARG = 0;
	rSDICCON = 0;
	rSDICSTA = 0xffff;
	rSDIBSIZE = 512;
	rSDICON = 0;
	rSDIDSTA = 0xffff;
	rSDIFSTA = 0xffff;
	rSDIDAT = 0;
	rSDIIMSK = 0;
	rSDIDTIMERL = 0xffff;
	rSDIDTIMERH = 0x001f;

	pclk = get_PCLK();
	//printf("\nSystem Frequency is %dHz\n",pclk);

	//rSDIPRE = pclk/(INICLK)-1;	// 400KHz
	rSDIPRE = MMCSD_PCLK/(2*MMCSD_INICLK) -1;
	//printf("\nInit. Frequency is %dHz\n",(pclk/(rSDIPRE+1)));

	rSDICON = (1<<4) | (1<<1) | 1;	// Type A, clk enable	(For Little Endian by Salamander)
	
	//rSDIFSTA   = rSDIFSTA|(1<<16);	// FIFO reset
	rSDICON = rSDICON | (1<<1);		// FIFO reset
	rSDIBSIZE  = 0x200;			// 512byte(128word)
	/* Set timeout count */
	rSDIDTIMERL = 0xffff;
	rSDIDTIMERH = 0x001f;

	for(i=0;i<0x1000;i++)	// Wait 74SDCLK for MMC card
		;  

	CMD0();  // Goto idel state

	//-- Check MMC card OCR
	if(Chk_MMC_OCR())
	{
		printf("\nMMC found.\n");
		isMMC=1;
		goto RECMD2;
	}

	//printf("MMC check end!!\n");
	
	//-- Check SD card OCR
	if(Chk_SD_OCR()) 
	{
		isMMC=0;
		printf("\nSD found : ");
	}
	else
	{
		printf("\nSD Initialize fail..\n\n");
		return 0;
	}

RECMD2:
	//-- Check attaced cards, it makes card identification state
	rSDICARG=0x0;   // CMD2(stuff bit)
	rSDICCON=(0x1<<10)|(0x1<<9)|(0x1<<8)|0x42; //lng_resp, wait_resp, start, CMD2

	//-- Check end of CMD2
	if(!Chk_CMDend(2, 1)) 
		goto RECMD2;
	//rSDICSTA=0xa00;	// Clear cmd_end(with rsp)

	//printf("\nEnd id\n");


RECMD3:
	//--Send RCA
	rSDICARG = 0;	   	// CMD3(MMC:Set RCA, SD:Ask RCA-->SBZ)
	//rSDICARG = isMMC<<16;	   	// CMD3(MMC:Set RCA, SD:Ask RCA-->SBZ)
	rSDICCON = (0x1<<9)|(0x1<<8)|0x43;	// sht_resp, wait_resp, start, CMD3

	//-- Check end of CMD3
	if(!Chk_CMDend(3, 1)) 
		goto RECMD3;
 
	//rSDICSTA = 0xa00;	// Clear cmd_end(with rsp)

	//--Publish RCA
	if(isMMC) 
	{
		RCA=1;
		rSDIPRE=(pclk/MMCCLK)-1;	
		printf("MMC Frequency is %dMHz\n", ((pclk/(rSDIPRE+1))/1000000));
	}
	else
	{
		RCA = ( rSDIRSP0 & 0xffff0000 )>>16;
		//rSDICARG = (RCA<<16) & 0xffff0000;	// CMD7(RCA,stuff bit)
		
		//printf("RCA=0x%x\n",RCA);
//		rSDIPRE=pclk/(SDCLK);	// Salamander pclk=66Mhz, prescaler = 2, SD_CLK = 22Mhz
		//rSDIPRE=3;  // test 16Mhz
		//printf("SD Frequency is %dMHz\n",(pclk/(rSDIPRE+1))/1000000);
		rSDIPRE = MMCSD_PCLK/(2*MMCSD_SD_NORCLK) -1;
		//rSDIPRE = pclk/(SDCLK+2);	// Salamander pclk=66Mhz, prescaler = 2, SD_CLK = 22Mhz
#if 0 /* org */
		printf("SD Frequency is %dMHz\n",(pclk/(rSDIPRE+1))/1000000);
#else /* shkim patch */
		printf("SD Frequency is %dMHz\n",(MMCSD_PCLK/(rSDIPRE+1))/(2*1000000));
#endif
	}	
	//--State(stand-by) check
	if( rSDIRSP0 & (0x1e00!=0x600) )  // CURRENT_STATE check
		goto RECMD3;

	//printf("\nIn stand-by\n");

	Card_sel_desel(1);	// Select

	if(!isMMC)
	{
		CardBusWidth=1;
		SetBus();	// For SD
	}
	else
	{
		CardBusWidth=0;
		SetBus();	// For MMC
	}

/*
	Card_sel_desel(0);	// DeSelect for IDLE

	if(!CMD9())
		printf("Get CSD fail!!!\n");

	Card_sel_desel(1);	// Select
*/

	{
		/* fill in device description */
		mmc_dev.if_type = IF_TYPE_MMC;
		mmc_dev.dev = 0;
		mmc_dev.lun = 0;
		mmc_dev.type = 0;
		/* FIXME fill in the correct size (is set to 32MByte) */
		mmc_dev.blksz = 512;
		//mmc_dev.lba = 0x10000;
		mmc_dev.lba = CardSize / 512;
		mmc_dev.part_type = PART_TYPE_DOS;
		/*
		sprintf(mmc_dev.vendor,"Man %02x%02x%02x Snr %02x%02x%02x",
				cid->id[0], cid->id[1], cid->id[2],
				cid->sn[0], cid->sn[1], cid->sn[2]);
		sprintf(mmc_dev.product,"%s",cid->name);
		sprintf(mmc_dev.revision,"%x %x",cid->hwrev, cid->fwrev);
		*/

		sprintf(mmc_dev.vendor,"Man aESOP1 Snr 123456");
		sprintf(mmc_dev.product,"%s","SDxxxMB");
		sprintf(mmc_dev.revision,"%x %x",0x123,0x456);

		mmc_dev.removable = 0;
		mmc_dev.block_read = mmc_bread;

		fat_register_device(&mmc_dev,1); /* partitions start counting with 1 */

	}
	mmc_init_once = 1;
	return 1;
}
//[*]----------------------------------------------------------------------------------------------------[*]
unsigned char *Rx_buffer1;	//128[word]*16[blk]=8192[byte]
//[*]----------------------------------------------------------------------------------------------------[*]
void SD_Read(unsigned char *pucdata, unsigned long ulLBA, unsigned long ulSectors)
{

	int status, state, rd_cnt, total_word, kk;
	
	Rx_buffer1 = (unsigned char*)(pucdata);

	rd_cnt=0;
	kk = 0;

	rSDICON = rSDICON | (1<<1);		// FIFO reset

//	rSDIDCON=(2<<22)|(1<<19)|(1<<17)|(CardBusWidth<<16)|(1<<14)|(2<<12)|(ulSectors<<0);	//Salamander
	rSDIDCON = (1<<19) | (1<<17) | (CardBusWidth<<16) | (2<<12) | (ulSectors<<0);


	//Word Rx, Rx after cmd, blk, 4bit bus, Rx start, blk num, data start, data transmit mode

	rSDICARG=ulLBA*512;	// CMD17/18(addr)
//	printf("rSDICARG :%x,ulLBA %x, ulLBA*512 %x\n", rSDICARG, ulLBA, ulLBA*512);
	//rSDICARG=0x0;	// CMD17/18(addr)

RERDCMD:
//	printf("RERCMD!!\n");
	if(ulSectors<2)	// SINGLE_READ
	{
		//printf("SINGLE\n");
		rSDICCON = (0x1<<9)|(0x1<<8)|0x51;    // sht_resp, wait_resp, dat, start, CMD17
		if(!Chk_CMDend(17, 1))	//-- Check end of CMD17
			goto RERDCMD;
	}
	else	// MULTI_READ
	{
		//printf("MULTI\n");
		rSDICCON = (0x1<<9)|(0x1<<8)|0x52;    // sht_resp, wait_resp, dat, start, CMD18
		if(!Chk_CMDend(18, 1))	//-- Check end of CMD18 
			goto RERDCMD;
	}
	//rSDICSTA=0xa00;			// Clear cmd_end(with rsp)

//	state = rSDICSTA;
//	rSDICSTA = state;	// Clear cmd_end(with rsp)	    
//	printf("\n Read CMD end\n");

	total_word = 128*ulSectors*4; 
	while(rd_cnt < total_word)
	{
//		for(kk=0; kk<1; kk++)	;	
		state = rSDIDSTA;
		if((state&0x20)==0x20) // Check timeout 
		{
			rSDIDSTA=(0x1<<0x5);  // Clear timeout flag
			//printf("\n[Time Out !!!] %d, state=%x  status=%x\n",rd_cnt,state,rSDIFSTA);
			//while(1);
			break;
		}
		status = rSDIFSTA;
		if( (status&0x1000) == 0x1000 )	// Is Rx data?
		{
			*Rx_buffer1++=rSDIDAT;
			//printf("%02x ",Rx_buffer1[rd_cnt]);
			rd_cnt++;
//			printf("%02x ", rd_cnt);
//			if(rd_cnt%8==0) printf("\n");			
		}
	}

	//printf("\n After Read loop %d %d\n",rd_cnt, kk);		

	//-- Check end of DATA
	//printf("\n[Start Chk_DATend !!!]\n");
	if(!Chk_DATend()) 
		printf("dat error\n");

    rSDIDSTA = 0x10;	// Clear data Tx/Rx end

	//printf("\n[End Chk_DATend !!!]\n");

	if(ulSectors>1)
	{
RERCMD12:
		//--Stop cmd(CMD12)
		rSDICARG=0x0;	    		//CMD12(stuff bit)
		rSDICCON=(0x1<<9)|(0x1<<8)|0x4c;	//sht_resp, wait_resp, start, CMD12

		//-- Check end of CMD12
		if(!Chk_CMDend(12, 1)) 
			goto RERCMD12;
		//rSDICSTA=0xa00;			// Clear cmd_end(with rsp)
	}
}
//[*]----------------------------------------------------------------------------------------------------[*]
void SD_Write(unsigned char * pucdata, unsigned long ulLBA, unsigned long ulSectors)
{
	int 			status, wt_cnt;
	unsigned int 	*Tx_buffer1;	//128[word]*16[blk]=8192[byte]

	Tx_buffer1 = (unsigned int *)(pucdata);

	wt_cnt=0;

	printf("### HERE WRITE\n");

	//rSDIFSTA   = rSDIFSTA|(1<<16);	// FIFO reset
	rSDICON = rSDICON | (1<<1);		// FIFO reset

	rSDIDCON = (2<<22)|(1<<20)|(1<<17)|(CardBusWidth<<16)|(1<<14)|(3<<12)|(ulSectors<<0);	
		//Word Tx, Tx after rsp, blk, 4bit bus, Tx start, blk num

	rSDICARG = ulLBA*512;	    // CMD24/25(addr)

REWTCMD:
	if(ulSectors<2)	// SINGLE_WRITE
	{
		rSDICCON=(0x1<<9)|(0x1<<8)|0x58;	//sht_resp, wait_resp, dat, start, CMD24
		if(!Chk_CMDend(24, 1))	//-- Check end of CMD24
			goto REWTCMD;
	}
	else	// MULTI_WRITE
	{
		rSDICCON = (0x1<<9)|(0x1<<8)|0x59;	//sht_resp, wait_resp, dat, start, CMD25
		if(!Chk_CMDend(25, 1))	//-- Check end of CMD25
			goto REWTCMD;
	}

	rSDICSTA = 0xa00;	// Clear cmd_end(with rsp)

	while(wt_cnt<128*ulSectors)
	{
		status = rSDIFSTA;
		if((status&0x2000)==0x2000)
		{
			rSDIDAT = *Tx_buffer1++;				
			wt_cnt++;
//				printf("Block No.=%d, wt_cnt=%d\n",block,wt_cnt);
		}
	}

	//-- Check end of DATA
	if(!Chk_DATend()) 
		printf("dat error\n");

	rSDIDCON = rSDIDCON & ~(7<<12);		//Clear Data Transfer mode => no operation, Cleata Data Transfer start
	rSDIDSTA = 0x10;	// Clear data Tx/Rx end

	if(ulSectors>1)
	{
		//--Stop cmd(CMD12)
REWCMD12:

		rSDIDCON = (1<<18)|(1<<17)|(0<<16)|(1<<14)|(1<<12)|(ulLBA<<0); 	

		rSDICARG = 0x0;	    		  //CMD12(stuff bit)
		rSDICCON = (0x1<<9)|(0x1<<8)|0x4c;    //sht_resp, wait_resp, start, CMD12

		//-- Check end of CMD12
		if(!Chk_CMDend(12, 1)) 
			goto REWCMD12;
		rSDICSTA = 0xa00;	// Clear cmd_end(with rsp)

		//-- Check end of DATA(with busy state)
		if(!Chk_BUSYend()) 
			printf("Write error!\n");
		rSDIDSTA = 0x08;	//! Should be cleared by writing '1'.
	}
}
//[*]----------------------------------------------------------------------------------------------------[*]
