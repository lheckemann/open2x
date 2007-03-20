/*
 * drivers/mmcsd/mmcsd_cs.c
 *
 * Copyright (C) 2007 The Open2x Project (www.open2x.org)
 * John Willis <djwillis@open2x.org>
 *
 * SDHC Support - John Willis, help from Notaz, Squidge and RobBrown
 *
 * Copyright (C) 2005,2006 Gamepark Holdings, Inc. (www.gp2x.com)
 * Hyun <hyun3678@gp2x.com>
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * Kim,SungHo < shkim@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * MMC/SD core service routines
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/errno.h>

#include "mmcsd.h"

/*====================================================================*/
/* Data Conversion utilities */

/************************************************************************
 * MMC/SD CID register
 *
 * Description: This function Print MMC/SD CID register
 *
 * Arguments  : CID Struct, id(MMC/SD flag)
 *
 * Returns    : none
 *
*************************************************************************/
void mmcsd_str2cid( CID_regs *regs, __u8 *buff, __u8 id)
{
	int i;
	regs->mid = buff[0];
	regs->oid = (buff[1] << 8) | (buff[2]);

	if(id) {
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

/************************************************************************
 * MMC/SD CSD register
 *
 * Description: This function Print MMC/SD CSD register
 *
 * Arguments  : CSD struct, id(MMC/SD flag)
 *
 * Returns    : none
 *
*************************************************************************/
int mmcsd_str2csd( CSD_regs *regs, __u8 *buff, __u8 id)
{
	regs->csd		= (buff[0] & 0xc0) >> 6;
	if (regs->csd > 2)
		return -ENODEV;		/* unknown CSD version */

	if (regs->csd == 1)
	{
		/* CSD version 2.0; consists mostly of fixed values,
		 * which host must override and not bother parsing out.
		 * FIXME: we should parse CSD correctly for HC MMC cards */

		(regs->taac).man	= 1;
		(regs->taac).exp	= 6;
		regs->nsac		= 0;
		(regs->tran_speed).man  = (buff[3] & 0x78) >> 3;
		(regs->tran_speed).exp  = (buff[3]) & 0x07;
		regs->ccc		= (buff[4] << 4) | ((buff[5] & 0xf0) >> 4);
		regs->read_len		= 9;
		regs->read_part		= 0;
		regs->write_mis		= 0;
		regs->read_mis		= 0;
		regs->dsr		= (buff[6] & 0x10) ? 1 : 0;
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
		regs->copy		= (buff[14] & 0x40) ? 1 : 0;
		regs->perm_wp		= (buff[14] & 0x20) ? 1 : 0;
		regs->tmp_wp		= (buff[14] & 0x10) ? 1 : 0;
		regs->ffmt		= 0;
	}
	else
	{
		if(!id) /* MMC */
			regs->spec_vers	= (buff[0] & 0x3c) >> 2;
		(regs->taac).man	= (buff[1] & 0x78) >> 3;
		(regs->taac).exp	= (buff[1]) & 0x07;
		regs->nsac		= buff[2];
		(regs->tran_speed).man  = (buff[3] & 0x78) >> 3;
		(regs->tran_speed).exp  = (buff[3]) & 0x07;
		regs->ccc		= (buff[4] << 4) | ((buff[5] & 0xf0) >> 4);
		regs->read_len		= (buff[5] & 0x0f);
		regs->read_part		= (buff[6] & 0x80) ? 1 : 0;
		regs->write_mis		= (buff[6] & 0x40) ? 1 : 0;
		regs->read_mis		= (buff[6] & 0x20) ? 1 : 0;
		regs->dsr		= (buff[6] & 0x10) ? 1 : 0;
		regs->c_size		= ((buff[6] & 0x03) << 10) | (buff[7] << 2) | ((buff[8] & 0xc0) >> 6);
		regs->vcc_r_min		= (buff[8] & 0x38) >> 3;
		regs->vcc_r_max		= (buff[8] & 0x07);
		regs->vcc_w_min		= (buff[9] & 0xe0) >> 5;
		regs->vcc_w_max		= (buff[9] & 0x1c) >> 2;
		regs->c_size_mult 	= ((buff[9] & 0x03) << 1) | ((buff[10] & 0x80) >> 7);
		if(id) {
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
		if(!id)	/* MMC */
			regs->dflt_ecc	= (buff[12] & 0x60) >> 5;
		regs->r2w_factor 	= (buff[12] & 0x1c) >> 2;
		regs->write_len  	= ((buff[12] & 0x03) << 2) | ((buff[13] & 0xc0) >> 6);
		regs->write_part 	= (buff[13] & 0x20) ? 1 : 0;
		regs->ffmt_grp   	= (buff[14] & 0x80) ? 1 : 0;
		regs->copy		= (buff[14] & 0x40) ? 1 : 0;
		regs->perm_wp		= (buff[14] & 0x20) ? 1 : 0;
		regs->tmp_wp		= (buff[14] & 0x10) ? 1 : 0;
		regs->ffmt		= (buff[14] & 0x0c) >> 2;
		if(!id)	/* MMC */
			regs->ecc	= (buff[14] & 0x03);
	}

	return 0;
}

/*====================================================================*/
/* Information collecting functions */

/************************************************************************
 * MMC/SD CSD Information
 *
 * Description: This function get CSD Infomation (lenth, size)
 *
 * Arguments  : CSD struct
 *
 * Returns    : none
 *
*************************************************************************/
void mmcsd_get_CSD_info(struct mmcsd_slot *slot, CSD_regs *csd)
{
	/* read/write block size */
	slot->read_len	= 0x0001 << (csd->read_len);
	slot->write_len	= 0x0001 << (csd->write_len);

	if (slot->read_len != slot->write_len) {
		DPRINTK("read_len(%u) and write_len(%u) are not equal\n",
		       slot->read_len, slot->write_len);
	}

	/* partial block read/write I/O support */
	if (csd->read_part) slot->stat |= MMCSD_READ_PART;
	if (csd->write_part) slot->stat |= MMCSD_WRITE_PART;

	if (csd->wp_grp_en) slot->stat |= MMCSD_WP_GRP_EN;
	if (csd->perm_wp) slot->stat |= MMCSD_PERM_WP;
	if (csd->tmp_wp) slot->stat |= MMCSD_TMP_WP;

	/* calculate total card size in blocks */
	slot->block_cnt = (1 + csd->c_size) * (0x01 << (csd->c_size_mult + 2));

	/* If card's block size is not 512, force it to 512 and adjust block count.
	 * Card will be notified about this when mmcsd_do_transfer_Multblock is called the first time */
	if (slot->read_len > 512)
	{
		slot->block_cnt <<= csd->read_len - 9;
		slot->read_len  = 512;
		slot->write_len = 512;
	}
}
