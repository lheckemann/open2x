/*
 * drivers/mmcsd/mmcsd_bus.c
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * Kim,SungHo < shkim@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * MMC/SD Bus protocol interfaces (supports MMC/SD mode)
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/sched.h>
#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#endif

#include <asm/hardware.h>

#include "mmcsd.h"
#include "mmcsd_slot_mmsp2.h"

#define MMCSD_RETRIES_MAX		100
#define MMCSD_RETRIES_MIN		10
#define MMCSD_RETRIES_CHECK		2

#define MMCSD_OP_COND_DELAY	set_current_state(TASK_INTERRUPTIBLE); \
				schedule_timeout(HZ/10)
volatile int RCA;

static struct mmcsd_slot *mmcsd_slots[MAX_MMCSD_SLOTS];
static struct mmcsd_notifier *mmcsd_notifiers;
#if defined(CONFIG_MACH_MMSP2_MDK)
const int mmcsd_res_len[] = {
	0,
	MMCSD_RES_LEN_SHORT,
	MMCSD_RES_LEN_SHORT,
	MMCSD_RES_LEN_LONG,
	MMCSD_RES_LEN_SHORT,
	MMCSD_RES_LEN_SHORT,
	MMCSD_RES_LEN_SHORT,
	MMCSD_RES_LEN_SHORT,
};

#define MMCSD_RESP_TIME_SHORT		50
#define MMCSD_RESP_TIME_LONG		150


EXPORT_SYMBOL(mmcsd_res_len);
#endif

/************************************************************************
 * Set Data Bus width (ACMD6)
 *
 * Description: This function Set Data Bus width (MMC : 1bit, SD : 4bits)
 *
 * Arguments  : wide(0,1), mmcsd_slot slot
 *
 * Returns    : none
 *
*************************************************************************/

inline static int set_data_bus(int wide, struct mmcsd_slot *slot)
{	
	int ret;
	struct mmcsd_cmd  cmd;

#if 1 /* shkim patch : 051215 */
	int retry = MMCSD_RESP_TIME_LONG;		//org int retry = 50;
#endif


	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
retry_set_bus:
	cmd.cmd = MMCSD_CMD55;
	slot->rca = RCA;
	cmd.arg = (slot->rca << 16) & 0xffff0000;
	cmd.res_type = MMCSD_RES_TYPE_R1;
	cmd.res_flag = 0;
	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
	slot->send_cmd(slot, &cmd);
	
	/* CMD6 implement */
	cmd.cmd = MMCSD_ACMD6;
	cmd.arg = wide<<1;
	cmd.res_type = MMCSD_RES_TYPE_NONE;
	cmd.res_flag = 0;
 	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
	slot->bus_width = wide;
        ret = slot->send_cmd(slot, &cmd);
	if(ret < 0) {
#if 1 /* shkim patch : 051215 */
		if(retry-- == 0) {
			printk("###[%s:%d]->%s: ACMD6 fail!!!\n", __FILE__, __LINE__, __FUNCTION__);
			return -1;
		}
		mdelay(10);
#endif
		goto retry_set_bus;
	}
	return 0;
}

/************************************************************************
 * Transfer 1block ( read/write block)
 *
 * Description: This function transfer lblock
 *
 * Arguments  : rd(r/w), from(sector positin), buf
 *
 * Returns    : none
 *
*************************************************************************/
static int mmcsd_do_transfer_1blk(struct mmcsd_slot *slot, int rd, u_long from, u_char *buf)
{
	int ret;
	struct mmcsd_cmd cmd;

	ret = down_interruptible(&slot->mutex);
	if (ret < 0) {
		return ret;
	}
	
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	if(slot->cnt < 1) {
	/* check card-state. if card-state != StandBy, return BUSY */
	if(slot->sd) /* SD */
		slot->rca = RCA;
	else		/* MMC */
		slot->rca = 1;
	cmd.cmd = MMCSD_CMD13;
	cmd.arg = (slot->rca << 16) & 0xffff0000;
	cmd.res_type = MMCSD_RES_TYPE_R1;
	cmd.res_flag = 0;
	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
	ret = slot->send_cmd(slot, &cmd);
	if (ret < 0) {
		printk( "CMD13 failed, ret = \n");
		goto err;
	}
	else {
	}
	mmcsd_str2r1(&(slot->r1), cmd.res);
	if (slot->r1 & (R1_cc_err | R1_err)) {
		printk( "CMD13 failed, R1\n");
		ret = -EIO;
		goto err;
	}
	if (!(slot->r1 & STATE_STBY)) {
		printk( "CMD13 failed, R1: Busy\n");
		ret = -EBUSY;
		goto err;
	}
	mdelay(10);
	DPRINTK("sent CMD13\n");
//	}
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);

	/* SELECT CARD & set card state from Stand-by to Transfer */
	cmd.cmd = MMCSD_CMD7;
	cmd.arg = (slot->rca << 16) & 0xffff0000;
	cmd.res_type = MMCSD_RES_TYPE_R1;
	cmd.res_flag = 0;
	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
	ret = slot->send_cmd(slot, &cmd);
	if (ret < 0) {
		printk("CMD7 failed, ret = %d\n", ret);
		goto err;
	}
	mmcsd_str2r1(&(slot->r1), cmd.res);
	if (slot->r1 & (R1_cc_err | R1_err)) {
		printk("CMD7 failed, R1(0x%08x)\n", slot->r1);
		ret = -EIO;
		goto err;
	}
	DPRINTK("sent CMD7\n");

	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
//	if(slot->cnt < 1) {
		if(slot->sd){
			/* cmd42 */
			cmd42(slot);

	 		/* SD card Set 4bit data bus */
		#if 1
			set_data_bus(BUS_WIDTH_4, slot);
		#else
			set_data_bus(BUS_WIDTH_1, slot);
		#endif
		}
		else
			set_data_bus(BUS_WIDTH_1, slot);
		(slot->cnt)++;
		mdelay(10);

#if 1 /* shkim patch : support 1024 bytes per block */
		DPRINTK("###[%s:%d]->%s: slot->read_len=0x%x\n", __FILE__, __LINE__, __FUNCTION__, slot->read_len);
		DPRINTK("###[%s:%d]->%s: slot->write_len=0x%x\n", __FILE__, __LINE__, __FUNCTION__, slot->write_len);
		if( (slot->read_len > 512) || (slot->write_len > 512) ) {
			/* change block length 512 bytes */
			cmd.cmd = MMCSD_CMD16;
			cmd.arg = 512;
			cmd.res_type = MMCSD_RES_TYPE_R1;
			cmd.res_flag = 0;
			cmd.t_res = MMCSD_TIME_NCR_MAX;
			cmd.t_fin = MMCSD_TIME_NRC_MIN;
			ret = slot->send_cmd(slot, &cmd);
			if (ret < 0) {
				printk("CMD16 failed, ret = %d\n", ret);
				goto err;
			}
			DPRINTK("sent CMD16\n");

			/* change block length infomation */
			slot->read_len = 512;
			slot->write_len = 512;
		}
#endif
	}

	DPRINTK2("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	if (rd) {
		DPRINTK1("read[%d]\n", slot->cnt);
		/* adtc, 31:0 dadr, R1, READ_SINGLE_BLOCK */
		/* set card state from Transfer to Sending-data */
		cmd.cmd = MMCSD_CMD17;
		cmd.arg = from;
		cmd.res_type = MMCSD_RES_TYPE_R1;
		cmd.res_flag = MMCSD_RES_FLAG_RDATA;
		cmd.data = buf;
		cmd.data_len = slot->read_len;
		cmd.t_res = MMCSD_TIME_NCR_MAX;
		cmd.t_fin = MMCSD_TIME_NAC_MIN;
		ret = slot->send_cmd(slot, &cmd);
		if (ret < 0) {
			printk("CMD17 failed, ret = %d\n", ret);
		}
		DPRINTK1("read[f]\n");
		DPRINTK("sent CMD17\n");

	} else {
		if(slot->card_in) {
		DPRINTK2("write[%d]\n", slot->cnt);
		/* adtc, 31:0 dadr, R1, WRITE_SINGLE_BLOCK */
		/* set card state from Transfer to Receive-data */
		cmd.cmd = MMCSD_CMD24;
		cmd.arg = from;
		cmd.res_type = MMCSD_RES_TYPE_R1;
		cmd.res_flag = MMCSD_RES_FLAG_WDATA;
		cmd.data = buf;
		cmd.data_len = slot->write_len;
		cmd.t_res = MMCSD_TIME_NCR_MAX;
		cmd.t_fin = MMCSD_TIME_NWR_MIN;
		ret = slot->send_cmd(slot, &cmd);
		if (ret < 0) {
			printk("CMD24 failed, ret = %d\n", ret);
		}
		DPRINTK1("write[f]\n");
		DPRINTK("sent CMD24\n");
		}
	}

#if 0
	/* DESELECT CARD */
	cmd.cmd = MMCSD_CMD7;
	cmd.arg = 0;
	cmd.res_type = MMCSD_RES_TYPE_NONE;
	cmd.res_flag = 0;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
	slot->send_cmd(slot, &cmd);
	DPRINTK("sent CMD7(DESELECT)\n");
#endif
 err:
	up(&slot->mutex);
	return ret;
}

/************************************************************************
 * MMC/SD Reset
 *
 * Description: This function Reset MMC/SD (CMD0)
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
inline static int mmcsd_reset(struct mmcsd_slot *slot)
{
	int ret;
	struct mmcsd_cmd cmd;

	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	/* go IDLE state */
	cmd.cmd = MMCSD_CMD0;
	cmd.arg = 0;
	cmd.res_type = MMCSD_RES_TYPE_NONE;
	cmd.res_flag = 0;
	cmd.t_fin = MMCSD_TIME_NCC_MIN;
	ret = slot->send_cmd(slot, &cmd);
	if (ret < 0) {
		DPRINTK("CMD0 failed, ret = %d\n", ret);
		return ret;
	}
	DPRINTK("sent CMD0\n");
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	mdelay(10);	/* wait for reset */
	return 0;
}


/************************************************************************
 * Check MMC OCR regisgter
 *
 * Description: This function check mmc ocr
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
int check_mmc_ocr(struct mmcsd_slot *slot)
{	
	int ret;
	int retries = MMCSD_RETRIES_MAX;
	struct mmcsd_cmd  cmd;

retry_send_op_cond:
	/* SEND_OP_COND */
	cmd.cmd = MMCSD_CMD1;
	slot->ocr = MMCSD_VDD_26_36;
	cmd.arg = slot->ocr;
	cmd.res_type = MMCSD_RES_TYPE_R3;
	cmd.res_flag = MMCSD_RES_FLAG_NOCRC;	
	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
	ret=slot->send_cmd(slot, &cmd);
	if (ret < 0) {
		if (ret != -ENODEV && retries--) {
			ret = mmcsd_reset(slot);
			if (ret < 0) {
				return ret;
			}
			MMCSD_OP_COND_DELAY;
			goto retry_send_op_cond;
		}
		return -ENODEV;
	}

	mmcsd_str2ocr(&(slot->ocr), cmd.res);
	if (!(slot->ocr & MMCSD_nCARD_BUSY)) {
		if (retries--) {
			MMCSD_OP_COND_DELAY;
			goto retry_send_op_cond;
		}
		return -ENODEV;
	}
	return ret;
}

/************************************************************************
 * Check SD OCR regisgter
 *
 * Description: This function check sd ocr
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
int check_sd_ocr(struct mmcsd_slot *slot)
{	
	struct mmcsd_cmd  cmd;

#if 1 /* shkim patch : 051215 */
	int retry = MMCSD_RESP_TIME_LONG;  //org int retry = 50;
#endif



retry_sd_ocr:
	/* CMD55, make ACMD */
	slot->rca = 0;
	cmd.cmd = MMCSD_CMD55;
       	cmd.arg = (slot->rca << 16) & 0xffff0000;
        cmd.res_type = MMCSD_RES_TYPE_R1;
	cmd.res_flag = 0;
	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
       	slot->send_cmd(slot, &cmd);
	
	/* ACMD41, SEND_OP_COND */
	cmd.cmd = MMCSD_ACMD41;
	slot->ocr = MMCSD_VDD_27_36;
	cmd.arg = slot->ocr;
	cmd.res_type = MMCSD_RES_TYPE_R1;
	cmd.res_flag = MMCSD_RES_FLAG_NOCRC;
	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
	slot->send_cmd(slot, &cmd);

#if 0 /* org */
	if(SDIRSP0 == 0x80ff8000) {
		mdelay(10);     // wait card power up status
		return 1;       //success
	} else
		goto retry_sd_ocr;
		return 0;               // fail
#else /* shkim patch : 051215 */
	if(SDIRSP0 == 0x80ff8000) {
		mdelay(10);     // wait card power up status
		return 1;       //success
	} else {
		if(retry-- == 0) {
			printk("###[%s:%d]->%s: ACMD41 fail!!!\n", __FILE__, __LINE__, __FUNCTION__);
			return 0;
		}
		else {
			mdelay(10);     // wait card power up status
			goto retry_sd_ocr;
		}
	}
	return 0;               // fail
#endif


}

/************************************************************************
 * CMD42 fuction
 *
 * Description: This function CMD42 (pull-up)
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
int cmd42(struct mmcsd_slot *slot)
{	
	int i, ret;
	struct mmcsd_cmd  cmd;

#if 1 /* shkim patch : 051215 */
	int retry = MMCSD_RESP_TIME_LONG;  //org int retry = 50
#endif



retry_cmd42:
	/* CMD55, make ACMD */
	slot->rca = RCA;
	cmd.cmd = MMCSD_CMD55;
       	cmd.arg = (slot->rca << 16) & 0xffff0000;
        cmd.res_type = MMCSD_RES_TYPE_R1;
	cmd.res_flag = 0;
	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
       	slot->send_cmd(slot, &cmd);
	
	/* ACMD42, SEND_OP_COND */
	cmd.cmd = MMCSD_CMD42;
	cmd.arg = 1;
	cmd.res_type = MMCSD_RES_TYPE_R1B;
	cmd.res_flag = 0;
	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
	ret=slot->send_cmd(slot, &cmd);
	if(ret<0) {
#if 1 /* shkim patch : 051215 */
		if(retry-- == 0) {
			printk("###[%s:%d]->%s: ACMD42 fail!!!\n", __FILE__, __LINE__, __FUNCTION__);
			return -1;
		}
		mdelay(10);
#endif
		goto retry_cmd42;
	}
	return 0;
}

/************************************************************************
 * MMC/SD identification
 *
 * Description: This function identify
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
inline static int mmcsd_identify(struct mmcsd_slot *slot)
{
	int ret;
	struct mmcsd_cmd cmd;

#if 1 /* shkim patch : 051215 */
	int retry = MMCSD_RESP_TIME_LONG;   //org  int retry = 50;
#endif

	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	if(slot->sd) {
		if(!check_sd_ocr(slot)) {	/* ACMD41 */
			DPRINTK("In SD Ready\n");
			return 0;
		}
	} /* SD */

	else {
		if(!check_mmc_ocr(slot)) /* CMD1 */
			DPRINTK("In MMC Ready\n");
		else {
			DPRINTK("Initialize Fail\n");
			return 0;
		}
	} /* MMC */
	
	/* ALL_SEND_CID */
	cmd.cmd = MMCSD_CMD2;
	cmd.arg = 0;
	cmd.res_type = MMCSD_RES_TYPE_R2;
	cmd.res_flag = 0;
	cmd.t_res = MMCSD_TIME_NID_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
	ret = slot->send_cmd(slot, &cmd);

	if (ret < 0) {
		DPRINTK("CMD2 failed, ret = %d\n", ret);
		return ret;
	}
	DPRINTK("sent CMD2\n");
		
	mmcsd_str2cid( &(slot->cid), cmd.res, slot->sd);
	DPRINTK("*** CID Register ***\n");
	if(slot->sd) {
		DPRINTK(" Manufactuer ID : 0x%02x\n OEM ID : 0x%04x\n Product Name : %s\n"
			" Product Revision : %d\n Product S/N : 0x%08x\n Manufacturing Data : %04d/%02d\n",
	       		slot->cid.mid, slot->cid.oid, slot->cid.pnm_sd, slot->cid.prv, slot->cid.psn,
			2000 + ((slot->cid.mdt_sd >> 4) & 0xff), slot->cid.mdt_sd & 0xf);
	} /* SD */

	else {
		DPRINTK(" Manufactuer ID : 0x%02x\n OEM ID : 0x%04x\n Product Name : %s\n"
			" Product Revision : %d\n Product S/N : 0x%08x\n Manufacturing Data : %04d/%02d\n",
	       		slot->cid.mid, slot->cid.oid, slot->cid.pnm, slot->cid.prv, slot->cid.psn,
			1997 + ((slot->cid.mdt_sd >> 4) & 0xff), slot->cid.mdt_sd & 0xf);
	} /* MMC */

	/* SET_RELATIVE_ADDR, Stand-by State */
	if(slot->sd) /* SD */ {
		slot->rca = 0;
	}
	else /* MMC */
		slot->rca = 1 + slot->id; /* slot id begins at 0 */
	cmd.cmd = MMCSD_CMD3;
	cmd.arg = (slot->rca << 16) & 0xffff0000;
	cmd.res_type = MMCSD_RES_TYPE_R1;
	cmd.res_flag = 0;
	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
	ret = slot->send_cmd(slot, &cmd);
	if (ret < 0) {
		DPRINTK("CMD3 failed, ret = %d\n", ret);
	return ret;
	}
	DPRINTK("sent CMD3\n");

	mmcsd_str2r1(&(slot->r1), cmd.res);
//	if (!(slot->r1 & STATE_IDENT)) {
	if (!(slot->r1 & STATE_STBY)) {
		printk("CMD3 failed, r1 = 0x%08x\n", slot->r1);
		return -ENODEV;
	}
	RCA = slot->r1 >> 16;
	DPRINTK("R1 = %08x\n", slot->r1);
send_csd:
	/* SEND_CSD, Stand-by State */
	cmd.cmd = MMCSD_CMD9;
	if(slot->sd) /* SD */
		slot->rca = RCA;
	else /* MMC */
		slot->rca = 1;
	cmd.arg = (slot->rca << 16) & 0xffff0000;
	cmd.res_type = MMCSD_RES_TYPE_R2;
	cmd.res_flag = 0;
	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
	ret = slot->send_cmd(slot, &cmd);
	if (ret < 0) {
#if 0 /* org */
		DPRINTK("CMD9 failed, ret = %d\n", ret);
		goto send_csd;
		return ret;
#else /* shkim patch : 051215 */
		if(retry-- == 0) {
			printk("###[%s:%d]->%s: CMD9 fail!!!\n", __FILE__, __LINE__, __FUNCTION__);
			return ret;
		}
		mdelay(10);
		goto send_csd;
#endif
	}


	DPRINTK("sent CMD9\n");

	mmcsd_str2csd( &(slot->csd), cmd.res, slot->sd);
if(slot->sd) /* SD */
	DPRINTK("csd(%d) spec_vers(%d) taac.man(%d) taac.exp(%d) nsac(%d)\n"
	       "tran.man(%d) tran.exp(%d) ccc(%03x) read_len(%x)\n"//read_len(%d)\n"
	       "read_part(%d) write_mis(%d) read_mis(%d) dsr(%d) c_size(%d)\n"
	       "vcc_r_min(%d) vcc_r_max(%d) vcc_w_min(%d) vcc_w_max(%d)\n"
	       "c_size_mult(%d) er_blk_en(%d) er_size(%d) wp_grp_size(%d)\n"
	       "wp_grp_en(%d) r2w_factor(%d) write_len(%x) write_part(%d)\n"
	       //"wp_grp_en(%d) r2w_factor(%d) write_len(%d) write_part(%d)\n"
	       "ffmt_grp(%d) copy(%d) perm_wp(%d) tmp_wp(%d) ffmt(%d)\n",
	       slot->csd.csd, slot->csd.spec_vers, slot->csd.taac.man, slot->csd.taac.exp, slot->csd.nsac,
	       slot->csd.tran_speed.man, slot->csd.tran_speed.exp, slot->csd.ccc,
	       slot->csd.read_len, slot->csd.read_part, slot->csd.write_mis,
               slot->csd.read_mis, slot->csd.dsr, slot->csd.c_size, slot->csd.vcc_r_min, 
	       slot->csd.vcc_r_max, slot->csd.vcc_w_min, slot->csd.vcc_w_max,
	       slot->csd.c_size_mult, slot->csd.er_blk_en, slot->csd.er_size, 
	       slot->csd.wp_grp_size, slot->csd.wp_grp_en, slot->csd.r2w_factor, 
	       slot->csd.write_len, slot->csd.write_part, slot->csd.ffmt_grp,
	       slot->csd.copy, slot->csd.perm_wp, slot->csd.tmp_wp, slot->csd.ffmt);

else /* MMC */
	DPRINTK("csd(%d) spec(%d) taac.man(%d) taac.exp(%d) nsac(%d)\n"
	       "tran.man(%d) tran.exp(%d) ccc(%03x) read_len(%d)\n"
	       "read_part(%d) write_mis(%d) read_mis(%d) dsr(%d) c_size(%d)\n"
	       "vcc_r_min(%d) vcc_r_max(%d) vcc_w_min(%d) vcc_w_max(%d)\n"
	       "c_size_mult(%d) er_size(%d) er_grp_size(%d) wp_grp_size(%d)\n"
	       "wp_grp_en(%d) dflt_ecc(%d) r2w_factor(%d) write_len(%d)\n"
	       "write_part(%d) ffmt_grp(%d) copy(%d) perm_wp(%d) tmp_wp(%d)\n"
	       "ffmt(%d) ecc(%d)\n",
	       slot->csd.csd, slot->csd.spec_vers, slot->csd.taac.man, 
	       slot->csd.taac.exp, slot->csd.nsac, slot->csd.tran_speed.man, 
	       slot->csd.tran_speed.exp, slot->csd.ccc, slot->csd.read_len, 
	       slot->csd.read_part, slot->csd.write_mis, slot->csd.read_mis,
	       slot->csd.dsr, slot->csd.c_size, slot->csd.vcc_r_min, 
	       slot->csd.vcc_r_max, slot->csd.vcc_w_min, slot->csd.vcc_w_max,
	       slot->csd.c_size_mult, slot->csd.er_size, 
	       slot->csd.er_grp_size, slot->csd.wp_grp_size, 
	       slot->csd.wp_grp_en, slot->csd.dflt_ecc, slot->csd.r2w_factor, 
	       slot->csd.write_len, slot->csd.write_part, slot->csd.ffmt_grp,
	       slot->csd.copy, slot->csd.perm_wp, slot->csd.tmp_wp,
	       slot->csd.ffmt, slot->csd.ecc);

	/* Get CSD value */
	mmcsd_get_CSD_info(slot, &(slot->csd));

/* for test */
#if 0
{
	unsigned long Pclk;
	Pclk = mmsp2_get_pclk();
	printk("##[%s:%d]->%s: Pclk(%d)\n", __FILE__, __LINE__, __FUNCTION__, Pclk);
}
#endif
	/* Set high clock rate for the normal data transfer */
	if(slot->sd)	/* SD */
		SDIPRE = MMCSD_PCLK/(2*MMCSD_SD_NORCLK) -1 ;		// normal clock
	else 		/* MMC */
		SDIPRE = MMCSD_PCLK/(2*MMCSD_MMC_NORCLK) -1 ;		// normal clock
	return 0;
}


/************************************************************************
 * Card identification
 *
 * Description: This function identify card (mmc/sd)
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
static int card_identify(struct mmcsd_slot *slot)
{
	int ret;
	int retry = MMCSD_RETRIES_CHECK;
	struct mmcsd_cmd cmd;

	/* Initialize some slot variables */
	slot->bus_width = 0;
	slot->rca = 0;
	slot->cnt = 0;

	/* Set low clock rate for the indentification */
	SDIPRE = MMCSD_PCLK/(2*MMCSD_INICLK) -1;	//Initialize clock

	/* Power up */
	if (slot->power_up) {
		slot->power_up(slot);
	}

 retry_card_check:
	/* go IDLE state */
	ret = mmcsd_reset(slot);
	if (ret < 0) {
		return ret;
	}

	/* check whether it's a MMC or a SD card. */
	/* APP_CMD */
	cmd.cmd = MMCSD_CMD55;
	cmd.arg = (slot->rca << 16) & 0xffff0000;
	cmd.res_type = MMCSD_RES_TYPE_R1;
	cmd.res_flag = 0;
	cmd.t_res = MMCSD_TIME_NCR_MAX;
	cmd.t_fin = MMCSD_TIME_NRC_MIN;
	ret = slot->send_cmd(slot, &cmd);
	if (ret < 0) {
		if (ret == -ENODEV) {
			return ret;
		}
		if (retry--) {
			DPRINTK("retry to check MMC or SD\n");
			goto retry_card_check;
		}
		DPRINTK("identify MultiMediaCard\n");
		slot->sd = 0;	/* MMC */
	} else {
		DPRINTK("identify SD Memory Card\n");

		slot->sd = 1;	/* SD */
	}
	DPRINTK("sent CMD55\n");
	ret = mmcsd_identify(slot);
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	return ret;
}

#ifdef CONFIG_PROC_FS
/************************************************************************
 * Read proc file
 *
 * Description: This function read proc file
 *
 * Arguments  :
 *
 * Returns    : len
 *
*************************************************************************/
static int mmcsd_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	char *p = page;
	int len;
	struct mmcsd_slot *slot = (struct mmcsd_slot *)data;

	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	p += sprintf(p, "read-only\t: %s\n"
			"card type\t: %s\n"
			"product name\t: %s\n"
			"card size\t: %ldMB\n",
			slot->readonly ? "yes":"no", slot->sd ? "SD":"MMC",
			slot->sd ? slot->cid.pnm_sd : slot->cid.pnm, 
			slot->size/(1024 * 1024));

	len = (p - page) - off;
	if (len < 0) len = 0;

	*eof = (len <= count) ? 1 : 0;
	*start = page + off;

	return len;
}
#endif

/************************************************************************
 * Add MMC/SD device
 *
 * Description: This function add mmcsd device
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
int add_mmcsd_device(struct mmcsd_slot *slot)
{
	int i;
	
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	down(&mmcsd_slots_mutex);
	slot->stat = 0;
	i = card_identify(slot);
	if (i) {
		up(&mmcsd_slots_mutex);
		return i;
	}

	init_MUTEX(&slot->mutex);
	slot->transfer1b = mmcsd_do_transfer_1blk;
	slot->stat = 1;

	for (i = 0; i < MAX_MMCSD_SLOTS; i++) {
		if (!mmcsd_slots[i]) {
			struct mmcsd_notifier *not=mmcsd_notifiers;
			//mmcsd_slots[i] = slot;	/* 요기가 막어져 있네 뭐지 */
			while (not) {
				(*(not->add))(slot);
				not = not->next;
                        }
			up(&mmcsd_slots_mutex);
			MOD_INC_USE_COUNT;
#ifdef CONFIG_PROC_FS
			create_proc_read_entry ("driver/mmcsd", 0, NULL, mmcsd_read_proc, (void *)slot);
			//sys_mount("/dev/mmcsd/disc0/part1", "/mnt/sd", "vfat", 0xC0ED0000, "iocharset=utf8"); /* 요기 */
#endif
			return 0;
		}
	}
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	up(&mmcsd_slots_mutex);
	return -ENOSPC;
}

/************************************************************************
 * Delete MMC/SD device
 *
 * Description: This function delete MMC/SD device
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
int del_mmcsd_device(struct mmcsd_slot *slot)
{
	int i;
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	down(&mmcsd_slots_mutex);

	/* Power down */
	if (slot->power_down) {
		slot->power_down(slot);
	}
	
	sys_umount("/mnt/sd", 0xC0ED0000);
	//sys_sync();
	
	for (i = 0; i < MAX_MMCSD_SLOTS; i++) {
		if (mmcsd_slots[i] == slot) {
			struct mmcsd_notifier *not=mmcsd_notifiers;
#ifdef CONFIG_PROC_FS
			remove_proc_entry ("driver/mmcsd", NULL);
#endif
			while (not) {
				(*(not->remove))(slot);
				not = not->next;
			}
			mmcsd_slots[i] = NULL;
			up(&mmcsd_slots_mutex);
			MOD_DEC_USE_COUNT;
			return 0;
		}
	}
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	up(&mmcsd_slots_mutex);
	return -EINVAL;
}

/************************************************************************
 * ReIdentification Set Data Bus width
 *
 * Description: This function ReIdentify MMC/SD device
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
int reidentify_mmcsd_device(struct mmcsd_slot *slot)
{
	int ret;

	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	down(&mmcsd_slots_mutex);
	ret = card_identify(slot);
	up(&mmcsd_slots_mutex);
	return ret;
}

/************************************************************************
 * Register MMC/SD
 *
 * Description: This function register MMC/SD
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
void register_mmcsd_user(struct mmcsd_notifier *new)
{
	int i;

	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	down(&mmcsd_slots_mutex);
	new->next = mmcsd_notifiers;
	mmcsd_notifiers = new;

	MOD_INC_USE_COUNT;

	for (i=0; i< MAX_MMCSD_SLOTS; i++)
		if (mmcsd_slots[i])
			new->add(mmcsd_slots[i]);
	up(&mmcsd_slots_mutex);
}

/************************************************************************
 * UnRegister MMC/SD
 *
 * Description: This function unregister mmc/sd
 *
 * Arguments  :
 *
 * Returns    : 1
 *
*************************************************************************/
int unregister_mmcsd_user(struct mmcsd_notifier *old)
{
	struct mmcsd_notifier **prev = &mmcsd_notifiers;
	struct mmcsd_notifier *cur;
	int i;

	down(&mmcsd_slots_mutex);
	while ((cur = *prev)) {
		if (cur == old) {
			*prev = cur->next;

			MOD_DEC_USE_COUNT;

			for (i=0; i< MAX_MMCSD_SLOTS; i++)
				if (mmcsd_slots[i])
					old->remove(mmcsd_slots[i]);

			up(&mmcsd_slots_mutex);
			return 0;
		}
		prev = &cur->next;
	}
	up(&mmcsd_slots_mutex);
	return 1;
}

/************************************************************************
 * Initialization MMC/SD Bus
 *
 * Description: This function initialize MMC/SD Bus
 *
 * Arguments  :
 *
 * Returns    : none
 *
*************************************************************************/
static int __init init_mmcsd_bus(void)
{
	int i;
	for (i = 0; i < MAX_MMCSD_SLOTS; i++) {
		mmcsd_slots[i] = NULL;
	}
	mmcsd_notifiers = NULL;
	DPRINTK1("###[%s:%d]->%s:\n", __FILE__, __LINE__, __FUNCTION__);
	return 0;
}

static void __exit exit_mmcsd_bus(void)
{
}

module_init(init_mmcsd_bus);
module_exit(exit_mmcsd_bus);

EXPORT_SYMBOL(add_mmcsd_device);
EXPORT_SYMBOL(del_mmcsd_device);
EXPORT_SYMBOL(reidentify_mmcsd_device);
EXPORT_SYMBOL(register_mmcsd_user);
EXPORT_SYMBOL(unregister_mmcsd_user);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
MODULE_LICENSE("Not GPL, Proprietary License");
MODULE_DESCRIPTION("MMC/SD bus protocol interfaces");
