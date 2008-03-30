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
#define rSDICARG		(*(volatile unsigned int*	)0xC0001504)		/* SDI Command Argument Register */
#define rSDICCON		(*(volatile unsigned short*	)0xC0001508)		/* SDI Command Control Register */
#define rSDICSTA		(*(volatile unsigned short*	)0xC000150A)		/* SDI Command Status Register */
#define rSDIRSP0		(*(volatile unsigned int*	)0xC000150C)		/* SDI Response Register 0 */
#define rSDIRSP1		(*(volatile unsigned int*	)0xC0001510)		/* SDI Response Register 1 */
#define rSDIRSP2		(*(volatile unsigned int*	)0xC0001514)		/* SDI Response Register 2 */
#define rSDIRSP3		(*(volatile unsigned int*	)0xC0001518)		/* SDI Response Register 3 */
#define rSDIBSIZE		(*(volatile unsigned short*	)0xC000151E)		/* SDI Block Size Register */
#define rSDIDCON		(*(volatile unsigned int*	)0xC0001520)		/* SDI Data Control Register */
#define rSDIDCNT		(*(volatile unsigned int*	)0xC0001524)		/* SDI Data Remain Counter Register */
#define rSDIDSTA		(*(volatile unsigned short*	)0xC0001528)		/* SDI Data Status Register */
#define rSDIFSTA		(*(volatile unsigned short*	)0xC000152A)		/* SDI FIFO Status Register */
#define rSDIDAT			(*(volatile unsigned char*	)0xC000152C)		/* SDI Data Register */
#define rSDIIMSK		(*(volatile unsigned int*	)0xC0001530)		/* SDI Interrupt Mask Register 0 */
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
#define SDIDCON_BNUM				FMsk(Fld(12,0))					/* Block Number(0~4095) */

#define RES_LEN_SHORT		6
#define RES_LEN_LONG		17


typedef struct {
	unsigned char  mid;			/* [127:120] Manufacturer ID */		
	unsigned short oid;			/* [119:104] OEM/Application ID */
	unsigned char  pnm[7];  	/* [103: 56] Product Name + '\0' (MMCSD) */
	unsigned char  pnm_sd[6];	/* [103: 64] Product Name + '\0' (SD) */
	unsigned char  prv;			/* [ 55: 48] Product Version (MMCSD) */
								/* [ 63: 56] Product Version (SD) */
	unsigned long psn;			/* [ 47: 16] Product Serial Number (MMCSD) */
								/* [ 55: 24] Product Serial Number (SD) */
	unsigned char  mdt;			/* [ 15:  8] Manufacturing date (MMCSD) */
	unsigned short mdt_sd;   	/* [ 19:  8] Manufacturing date (SD) */
								/* [  7:  1] CRC (rwe) */
} CID_regs;

typedef struct {
	unsigned char csd;			/* [127:126] CSD structure */
	unsigned char spec_vers;	/* [125:122] Spec version (MMCSD) */
	struct {
		unsigned char man;		/* [118:115] time mantissa */
		unsigned char exp;		/* [114:113] time exponent */
	} taac;						/* [119:112] Data read access-time-1 */
	unsigned char nsac;			/* [111:104] Data read access-time-2 in CLK cycle */
	struct {
		unsigned char man;		/* [103:100] rate mantissa */
		unsigned char exp;		/* [ 99: 97] rate exponent */
	} tran_speed;				/* [103: 96] Max. data transfer rate */
	unsigned short ccc;			/* [ 95: 84]  Card command classes */
	unsigned char read_len;		/* [ 83: 80] Max. read data block length */
	unsigned char read_part;	/* [ 79: 79] Partial blocks for read allowed */
	unsigned char write_mis;	/* [ 78: 78] write block misalignment */
	unsigned char read_mis;		/* [ 77: 77] read block misalignment */
	unsigned char dsr;			/* [ 76: 76] DSR implemented */
	unsigned long c_size;		/* [ 73: 62] Device size (SDHC: [ 69: 48] )*/

	unsigned char vcc_r_min;	/* [ 61: 59] Max. read current at Vcc min */
	unsigned char vcc_r_max;	/* [ 58: 56] Max. read current at Vcc max */
	unsigned char vcc_w_min;	/* [ 55: 53] Max. write current at Vcc min */
	unsigned char vcc_w_max;	/* [ 52: 50] Max. write current at Vcc max */
	unsigned char c_size_mult;	/* [ 49: 47] Device size multiplier */
	unsigned char er_blk_en;	/* [ 46: 46] Erase single block enable (SD) */
	unsigned char er_size;		/* [ 46: 42] Erase sector size (MMCSD) */
								/* [ 45: 39] Erase sector size (SD) */
	unsigned char er_grp_size;	/* [ 41: 37] Erase group size (MMCSD) */
	unsigned char wp_grp_size;	/* [ 36: 32] Write protect group size (MMCSD)*/
								/* [ 38: 32] Write Protect group size (SD) */	
	unsigned char wp_grp_en;	/* [ 31: 31] Write protect group enable */
	unsigned char dflt_ecc;		/* [ 30: 29] Manufacturer default ECC (MMCSD) */
	unsigned char r2w_factor;	/* [ 28: 26] Write speed factor */
	unsigned char write_len;	/* [ 25: 22] Max. write data block length */
	unsigned char write_part;	/* [ 21: 21] Partial blocks for write allowed */
								/* [ 20: 17] Reserved */
								/* [ 16: 16] Content protection application */
	unsigned char ffmt_grp;		/* [ 15: 15] File format group (rw) */
	unsigned char copy;			/* [ 14: 14] Copy flag (OTP) (rw) */
	unsigned char perm_wp;		/* [ 13: 13] Permanent write protection (rw) */
	unsigned char tmp_wp;		/* [ 12: 12] temporary write protection (rwe) */
	unsigned char ffmt;			/* [ 11: 10] file format (rw) */
	unsigned char ecc;			/* [  9:  8] ECC (MMCSD) (rwe) */
								/* [  7:  1] CRC (rwe) */
} CSD_regs;

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
unsigned char sdhc;
	 
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

		if((cmd==1) | (cmd==9) | (cmd==41) )	// CRC no check, CMD9 is a long Resp. command.
		{
			if( (finish0&0xf00) != 0xa00 )  // Check error
			{
				rSDICSTA = finish0;   		// Clear error state
				if(((finish0&0x400)==0x400))
					return 0;				// Timeout error
			}
			rSDICSTA = finish0;				// Clear cmd & rsp end state
		}
		else								// CRC check
		{
			
			if( (finish0&0x1f00) != 0xa00 )		// Check error
			{				
				rSDICSTA = finish0;   			// Clear error state
				if(((finish0&0x400)==0x400))
					return 0;					// Timeout error
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
		
	}
	if( (finish&0xfc) != 0x10 )
	{
		printf("DATA:finish=0x%x\n", finish); 	// Something wrong !!
		rSDIDSTA = 0xec;  						// Clear error state
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
unsigned char CMD8(void)
{
	int retry=10;

retry_CMD8:	
	rSDICARG = 0xAA | (1<<8);
	rSDICCON = (0x1<<9) | (0x1<<8) | 0x48;	// short_rsp, wait_resp, start, CMD8
		
	//-- Check end of CMD8
	if(!Chk_CMDend(8, 1)) 
	{
		if(retry--){
			CMD0();
			goto retry_CMD8;	
		}						
	}
	else
	{	
		if( (rSDIRSP0 & 0xff) == 0xAA ) 
		{
			printf("SD VER 2.0 \n");	
			return 1;
		}			
	}
	return 0;				/* none card & ver1.0 */	
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
int CMD9(void)//SEND_CSD
{
	int retry=10;
retry_CMD9:		
	if(isMMC) RCA=1;
	
	rSDICARG = (RCA<<16) & 0xffff0000;	// CMD7(RCA,stuff bit)
	rSDICCON = (0x1<<10) | (0x1<<9) | (0x1<<8) | 0x49;	// long_resp, wait_resp, start, CMD9

	if(!Chk_CMDend(9, 1))
    {
    	if(retry--) goto retry_CMD9;	 
		else return 0;
	}
    
  	udelay(20000);
    return 1;
}
//[*]----------------------------------------------------------------------------------------------------[*]
void SetResToBuf(unsigned char *p)
{
	unsigned long tmp;
	
	tmp = (rSDIRSP0 >> 24) | ( (rSDIRSP0 >> 8) & 0xff00 ) | ( (rSDIRSP0 & 0xff00) << 8 ) | ( (rSDIRSP0 & 0xff) << 24); 
	memcpy(p ,&tmp ,sizeof(tmp));
	tmp = (rSDIRSP1 >> 24) | ( (rSDIRSP1 >> 8) & 0xff00 ) | ( (rSDIRSP1 & 0xff00) << 8 ) | ( (rSDIRSP1 & 0xff) << 24); 
	memcpy(p + (1 * sizeof(tmp)) ,&tmp ,sizeof(tmp));
	tmp = (rSDIRSP2 >> 24) | ( (rSDIRSP2 >> 8) & 0xff00 ) | ( (rSDIRSP2 & 0xff00) << 8 ) | ( (rSDIRSP2 & 0xff) << 24); 
	memcpy(p + (2 * sizeof(tmp)) ,&tmp ,sizeof(tmp));
	tmp = (rSDIRSP3 >> 24) | ( (rSDIRSP3 >> 8) & 0xff00 ) | ( (rSDIRSP3 & 0xff00) << 8 ) | ( (rSDIRSP3 & 0xff) << 24); 
	memcpy(p + (3 * sizeof(tmp)) ,&tmp ,sizeof(tmp));
}
//[*]----------------------------------------------------------------------------------------------------[*]
void str2cid( CID_regs *regs, unsigned char *buff)
{
	int i;
	regs->mid = buff[0];
	regs->oid = (buff[1] << 8) | (buff[2]);
	
	if(!isMMC) {
		for(i=0; i < 5; i++)
			regs->pnm_sd[i] = buff[3+i];
		regs->pnm_sd[5] = '\0';
		regs->prv = buff[8];
		regs->psn = (buff[9] << 24) | (buff[10] << 16) |(buff[11] << 8) | buff[12];
		regs->mdt_sd = (buff[13] << 8) | buff[14];
	} /* SD Only*/
	else {
		for(i=0; i < 6; i++)
			regs->pnm[i] = buff[3+i];
		regs->pnm[6] = '\0';
		regs->prv = buff[9];
		regs->psn = (buff[10] << 24) | (buff[11] << 16) |(buff[12] << 8) | buff[13];
		regs->mdt = buff[14];
	} /* MMC Only*/		
}
//[*]----------------------------------------------------------------------------------------------------[*]
int str2csd( CSD_regs *regs, unsigned char  *buff, unsigned char  verSD)
{
	int ret;
	regs->csd		= (buff[0] & 0xc0) >> 6;
	
	if (regs->csd > 2)
		return -1;

	if(verSD)
	{
		if(regs->csd != 1) goto NOT_SDHC;
		/* CSD version 2.0; consists mostly of fixed values,
		 * which host must override and not bother parsing out.
		 * FIXME: we should parse CSD correctly for HC MMC cards */
		(regs->taac).man	= 1;
		(regs->taac).exp	= 6;
		 regs->nsac			= 0;
		(regs->tran_speed).man  = (buff[3] & 0x78) >> 3;
		(regs->tran_speed).exp  = (buff[3]) & 0x07;
		regs->ccc			= (buff[4] << 4) | ((buff[5] & 0xf0) >> 4);
		regs->read_len		= 9;
		regs->read_part		= 0;
		regs->write_mis		= 0;
		regs->read_mis		= 0;
		regs->dsr			= (buff[6] & 0x10) ? 1 : 0;
		regs->c_size		= ((buff[7] & 0x3f) << 16) | (buff[8] << 8) | buff[9];
		regs->vcc_r_min		= 7;
		regs->vcc_r_max		= 6;
		regs->vcc_w_min		= 7;
		regs->vcc_w_max		= 6;
		regs->c_size_mult 	= 10 - 2;
		regs->er_blk_en		= 1;
		regs->er_size		= 0x7f;
		regs->wp_grp_size	= 0;
		regs->wp_grp_en		= 0;
		regs->r2w_factor 	= 2;
		regs->write_len  	= 9;
		regs->write_part 	= 0;
		regs->ffmt_grp   	= 0;
		regs->copy			= (buff[14] & 0x40) ? 1 : 0;
		regs->perm_wp		= (buff[14] & 0x20) ? 1 : 0;
		regs->tmp_wp		= (buff[14] & 0x10) ? 1 : 0;
		regs->ffmt			= 0;
		
		ret					= 1;
	}
	else
	{
NOT_SDHC:
		if(isMMC) /* MMC */
			regs->spec_vers	= (buff[0] & 0x3c) >> 2;
		(regs->taac).man	= (buff[1] & 0x78) >> 3;
		(regs->taac).exp	= (buff[1]) & 0x07;
		regs->nsac			= buff[2];
		(regs->tran_speed).man  = (buff[3] & 0x78) >> 3;
		(regs->tran_speed).exp  = (buff[3]) & 0x07;
		regs->ccc			= (buff[4] << 4) | ((buff[5] & 0xf0) >> 4);
		regs->read_len		= (buff[5] & 0x0f);
		regs->read_part		= (buff[6] & 0x80) ? 1 : 0;
		regs->write_mis		= (buff[6] & 0x40) ? 1 : 0;
		regs->read_mis		= (buff[6] & 0x20) ? 1 : 0;
		regs->dsr			= (buff[6] & 0x10) ? 1 : 0;
		regs->c_size		= ((buff[6] & 0x03) << 10) | (buff[7] << 2) | ((buff[8] & 0xc0) >> 6);
		regs->vcc_r_min		= (buff[8] & 0x38) >> 3;
		regs->vcc_r_max		= (buff[8] & 0x07);
		regs->vcc_w_min		= (buff[9] & 0xe0) >> 5;
		regs->vcc_w_max		= (buff[9] & 0x1c) >> 2;
		regs->c_size_mult 	= ((buff[9] & 0x03) << 1) | ((buff[10] & 0x80) >> 7);
		if(!isMMC) {
			regs->er_blk_en	= (buff[10] & 0x40) ? 1: 0;
			regs->er_size	= ((buff[10] & 0x3f) | (buff[11] & 0x80)) >> 7;
			regs->wp_grp_size= (buff[11] & 0x7f);
		} /* SD */
		else {
			regs->er_size   = (buff[10] & 0x7c) >> 2;
			regs->er_grp_size= ((buff[10] & 0x03) << 3) |((buff[11] & 0xe0) >> 5);
			regs->wp_grp_size= (buff[11] & 0x1f);
		} /* MMC */
		regs->wp_grp_en		= (buff[12] & 0x80) ? 1 : 0;
		if(isMMC)	/* MMC */
			regs->dflt_ecc	= (buff[12] & 0x60) >> 5;
		regs->r2w_factor 	= (buff[12] & 0x1c) >> 2;
		regs->write_len  	= ((buff[12] & 0x03) << 2) | ((buff[13] & 0xc0) >> 6);
		regs->write_part 	= (buff[13] & 0x20) ? 1 : 0;
		regs->ffmt_grp   	= (buff[14] & 0x80) ? 1 : 0;
		regs->copy			= (buff[14] & 0x40) ? 1 : 0;
		regs->perm_wp		= (buff[14] & 0x20) ? 1 : 0;
		regs->tmp_wp		= (buff[14] & 0x10) ? 1 : 0;
		regs->ffmt			= (buff[14] & 0x0c) >> 2;
		if(isMMC)	/* MMC */
			regs->ecc		= (buff[14] & 0x03);
		
		ret=0;
	}
	
	return ret;		/* unknown CSD version */
}



//[*]----------------------------------------------------------------------------------------------------[*]
int Chk_MMC_OCR(void)
{
	int i;
	
    //-- Negotiate operating condition for MMC, it makes card ready state
    for(i=0;i<15;i++)
    {
    	rSDICARG=0xffc000;	    	    	//CMD1(OCR:2.6V~3.6V)
    	rSDICCON=(0x1<<9)|(0x1<<8)|0x41;    //sht_resp, wait_resp, start, CMD1

    	//-- Check end of CMD1
    	if(Chk_CMDend(1, 1) & (rSDIRSP0==0x80ffc000)) 
		{
	    	//rSDICSTA=0xa00;	// Clear cmd_end(with rsp)
	    	return 1;			// Success
		}	
    }
    //rSDICSTA=0xa00;			// Clear cmd_end(with rsp)
    return 0;					// Fail
}
//[*]----------------------------------------------------------------------------------------------------[*]
int Chk_SD_OCR(unsigned char* bVer)
{
    int i=0;
	int CntVer=0;
	unsigned long respChk;

   	if(*bVer) respChk = 0xc0ff8000;
    else respChk = 0x80ff8000;
    
    //-- Negotiate operating condition for SD, it makes card ready state
	for(i=0;i<150;i++)
	{
		CMD55();    // Make ACMD

 		if(*bVer) rSDICARG = 0xff8000 | (1<<30);	// ACMD41(OCR:2.7V~3.6V)
    	else rSDICARG = 0xff8000;
    	
    	rSDICCON = (0x1<<9) | (0x1<<8) | 0x69;	// sht_resp, wait_resp, start, ACMD41
		//-- Check end of ACMD41
	    Chk_CMDend(41, 1);
	   	if(rSDIRSP0==respChk) 
	    {
	    	udelay(20000);	 	// Wait Card power up status
	    	return 1;			// Success	    
		}
		else
		{
			if(*bVer)
			{
				if(rSDIRSP0 == 0x80ff8000) CntVer++;
				else CntVer=0;
			}
		
			if(CntVer==5)
			{
				*bVer = 0; 	
				udelay(20000);
				return 1;
			}
			udelay(20000); 
    	}
    }
    return 0;			// Fail
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

	if(!Chk_CMDend(6, 1))   			// ACMD6
		goto SET_BUS;
	//rSDICSTA=0xa00;	    			// Clear cmd_end(with rsp)
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
		rSDICARG=0<<16;				//CMD7(RCA,stuff bit)
		rSDICCON=(0x1<<8)|0x47;		//no_resp, start, CMD7

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
	if(blkcnt==0)
	{
		printf("Why block_cnt == 0?? \n");
		return 0;
	}
	
	
	SD_Read((uchar *)dst, blknr, blkcnt);
	return blkcnt;
}
//[*]----------------------------------------------------------------------------------------------------[*]
int mmc_write(uchar *src, ulong dst, int size)
{
	printf("\nmmc_write should not be called !!!!\n");
	return 0;
}
//[*]----------------------------------------------------------------------------------------------------[*]
int mmc_read(ulong src, uchar *dst, int size)
{
	printf("\nmmc_read should not be called !!!!\n");
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
	unsigned char bVerSD;
	unsigned long pclk,tmpRes,blkSizeCnt;
	unsigned char resVal[RES_LEN_LONG];
	CSD_regs csd;
	
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
	rSDIPRE = MMCSD_PCLK/(2*MMCSD_INICLK) -1;
	rSDICON = (1<<4) | (1<<1) | 1;		// Type A, clk enable	(For Little Endian by Salamander)
	rSDICON = rSDICON | (1<<1);			// FIFO reset
	rSDIBSIZE  = 0x200;					// 512byte(128word)
	/* Set timeout count */
	rSDIDTIMERL = 0xffff;
	rSDIDTIMERH = 0x001f;

	for(i=0;i<0x1000;i++)				// Wait 74SDCLK for MMC card
		;  
	CMD0();  							// Goto idel state(RESET)
	bVerSD=CMD8();						// check SD VERSION 
	if(Chk_SD_OCR(&bVerSD)) 
	{
		isMMC=0;
		printf("\nSD found : ");
	}
	else
	{
		printf("\nSD Initialize fail..\n\n");
		if(Chk_MMC_OCR())
		{
			printf("\nMMC found.\n");
			isMMC=1;
		}
		else
			return 0;
	}
	
RECMD2:
	/* ALL_SEND_CID */
	rSDICARG=0x0;   // CMD2(stuff bit)
	rSDICCON=(0x1<<10)|(0x1<<9)|(0x1<<8)|0x42; //lng_resp, wait_resp, start, CMD2
	if(!Chk_CMDend(2, 1)) 
		goto RECMD2;

RECMD3:
	//--Send RCA
	rSDICARG = 0;	   			
	rSDICCON = (0x1<<9)|(0x1<<8)|0x43;	
	Chk_CMDend(3, 1); 

	tmpRes = rSDIRSP0;
	if(!(tmpRes & 0x600)) 
	{
		printf("CMD3( Read CSD) error \n");
		goto RECMD3;
	} 	
	
	RCA = tmpRes >> 16;
	if(!CMD9()){		
		printf("CMD9( Read CSD) error \n");
		goto RECMD3;
	}	 

	SetResToBuf(resVal);
 	sdhc=str2csd(&csd,resVal,bVerSD);
 	if(sdhc < 0 ) {
 		printf("Not support SD VERSION !!!\n");
 		return sdhc; 
 	}	
	
	if(sdhc) blkSizeCnt = (1 + csd.c_size) * (0x01 << (csd.c_size_mult + 2));
	
	//--Publish RCA
	if(isMMC) 
	{
		RCA=1;
		rSDIPRE=(pclk/MMCCLK)-1;	
		printf("MMC Frequency is %dMHz\n", ((pclk/(rSDIPRE+1))/1000000));
	}
	else
	{
		rSDIPRE = MMCSD_PCLK/(2*MMCSD_SD_NORCLK) -1;
		printf("SD Frequency is %dMHz\n",(MMCSD_PCLK/(rSDIPRE+1))/(2*1000000));
	}	

	udelay(20000);

RECMD13:	
	/* check card-state. if card-state != StandBy, return BUSY */
	rSDICARG = (RCA << 16) & 0xffff0000;
	rSDICCON = (0x1<<9)|(0x1<<8)|0x4d;	
	if(!Chk_CMDend(13, 1)) 
	{	
		printf( "CMD13 failed \n");
		goto RECMD13;
	}else{
		tmpRes = rSDIRSP0;
		if (tmpRes & (0x100000 | 0x80000)) {
        	printf( "CMD13 failed ecc\n");
            goto RECMD13;
        }					
		if (!(tmpRes & 0x600)) {
        	printf( "CMD13 failed Busy\n");
        	goto RECMD13;    
       	}
		udelay(20000);
	}
	
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
	
	udelay(50000);
#if 0	
	if( (csd.read_len > 9) || (csd.write_len > 9) ) 
    {
RECMD16:
		rSDICARG = 512;	    
		rSDICCON = (0x1<<9) | (0x1<<8) | 0x50;	// short_rsp, wait_resp, start, CMD8
		
		if(!Chk_CMDend(16, 1)){
			printf("CMD16 failed\n");
			goto RECMD16;
		}
		
		if(sdhc) blkSizeCnt <<= csd.read_len - 9; 
		
		csd.read_len = 9;
		csd.write_len = 9;
	}	
#endif

	{
		/* fill in device description */
		mmc_dev.if_type = IF_TYPE_MMC;
		mmc_dev.dev = 0;
		mmc_dev.lun = 0;
		mmc_dev.type = 0;
		/* FIXME fill in the correct size (is set to 32MByte) */
		mmc_dev.blksz = 512;
		mmc_dev.lba = CardSize / 512;
		mmc_dev.part_type = PART_TYPE_DOS;

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
	rSDIDCON = (1<<19) | (1<<17) | (CardBusWidth<<16) | (2<<12) | (ulSectors<<0);

	if(!sdhc)
		rSDICARG=ulLBA*512;				// CMD17/18(addr)
	else
		rSDICARG=ulLBA;

RERDCMD:
	if(ulSectors<2)								// SINGLE_READ
	{
		rSDICCON = (0x1<<9)|(0x1<<8)|0x51;    	// sht_resp, wait_resp, dat, start, CMD17
		if(!Chk_CMDend(17, 1))					//-- Check end of CMD17
			goto RERDCMD;
	}
	else	
	{
		rSDICCON = (0x1<<9)|(0x1<<8)|0x52;    	// sht_resp, wait_resp, dat, start, CMD18
		if(!Chk_CMDend(18, 1))					//-- Check end of CMD18 
			goto RERDCMD;
	}
	//rSDICSTA=0xa00;							// Clear cmd_end(with rsp)

	total_word = 128*ulSectors*4; 
	while(rd_cnt < total_word)
	{
		state = rSDIDSTA;
		if((state&0x20)==0x20) 				// Check timeout 
		{
			rSDIDSTA=(0x1<<0x5);  			// Clear timeout flag
			break;
		}
		status = rSDIFSTA;
		if( (status&0x1000) == 0x1000 )		// Is Rx data?
		{
			*Rx_buffer1++=rSDIDAT;
			rd_cnt++;

		}
	}

	if(!Chk_DATend()) 
		printf("dat error\n");

    rSDIDSTA = 0x10;						// Clear data Tx/Rx end

	if(ulSectors>1)
	{
RERCMD12:
		//--Stop cmd(CMD12)
		rSDICARG=0x0;	    				//CMD12(stuff bit)
		rSDICCON=(0x1<<9)|(0x1<<8)|0x4c;	//sht_resp, wait_resp, start, CMD12

		//-- Check end of CMD12
		if(!Chk_CMDend(12, 1)) 
			goto RERCMD12;
		//rSDICSTA=0xa00;					// Clear cmd_end(with rsp)
	}
}
//[*]----------------------------------------------------------------------------------------------------[*]
void SD_Write(unsigned char * pucdata, unsigned long ulLBA, unsigned long ulSectors)
{
	int 			status, wt_cnt;
	unsigned int 	*Tx_buffer1;	//128[word]*16[blk]=8192[byte]

	Tx_buffer1 = (unsigned int *)(pucdata);
	wt_cnt=0;

	rSDICON = rSDICON | (1<<1);					// FIFO reset
	rSDIDCON = (2<<22)|(1<<20)|(1<<17)|(CardBusWidth<<16)|(1<<14)|(3<<12)|(ulSectors<<0);	
	rSDICARG = ulLBA*512;	    				// CMD24/25(addr)

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
