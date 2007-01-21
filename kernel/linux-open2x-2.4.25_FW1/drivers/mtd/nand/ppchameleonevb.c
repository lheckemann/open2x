/*
 *  drivers/mtd/nand/ppchameleonevb.c
 *
 *  Copyright (C) 2003 DAVE Srl (info@wawnet.biz)
 *
 *  Derived from drivers/mtd/nand/edb7312.c
 *
 *
 * $Id: ppchameleonevb.c,v 1.1.2.1 2004/04/27 08:15:38 bushi Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Overview:
 *   This is a device driver for the NAND flash device found on the
 *   PPChameleonEVB board which utilizes the Samsung K9F5608U0B part. This is
 *   a 256Mibit (32MiB x 8 bits) NAND flash device.
 */

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include <platforms/PPChameleonEVB.h>

/* handy sizes */
#define SZ_4M                           0x00400000

/* GPIO pins used to drive NAND chip */
#define NAND_nCE_GPIO_PIN (1 << 14)
#define NAND_CLE_GPIO_PIN (1 << 15)
#define NAND_ALE_GPIO_PIN (1 << 16)

/*
 * MTD structure for PPChameleonEVB board
 */
static struct mtd_info *ppchameleonevb_mtd = NULL;

/*
 * Module stuff
 */
static int ppchameleonevb_fio_pbase = CFG_NAND1_PADDR;
#ifdef MODULE
MODULE_PARM(ppchameleonevb_fio_pbase, "i");

__setup("ppchameleonevb_fio_pbase=",ppchameleonevb_fio_pbase);
#endif

#ifdef CONFIG_MTD_PARTITIONS
/*
 * Define static partitions for flash device
 */
static struct mtd_partition partition_info[] = {
	{ name: "PPChameleonEVB Nand Flash",
		  offset: 0,
		  size: 32*1024*1024 }
};
#define NUM_PARTITIONS 1

extern int parse_cmdline_partitions(struct mtd_info *master,
				    struct mtd_partition **pparts,
				    const char *mtd_id);
#endif


/*
 *	hardware specific access to control-lines
 */
static void ppchameleonevb_hwcontrol(struct mtd_info *mtdinfo, int cmd)
{
	switch(cmd) {

	case NAND_CTL_SETCLE:
        	MACRO_NAND_CTL_SETCLE(CFG_NAND1_PADDR);
		break;
	case NAND_CTL_CLRCLE:
        	MACRO_NAND_CTL_CLRCLE(CFG_NAND1_PADDR);
		break;
	case NAND_CTL_SETALE:
        	MACRO_NAND_CTL_SETALE(CFG_NAND1_PADDR);
		break;
	case NAND_CTL_CLRALE:
        	MACRO_NAND_CTL_CLRALE(CFG_NAND1_PADDR);
		break;
	case NAND_CTL_SETNCE:
        	MACRO_NAND_ENABLE_CE(CFG_NAND1_PADDR);
		break;
	case NAND_CTL_CLRNCE:
        	MACRO_NAND_DISABLE_CE(CFG_NAND1_PADDR);
		break;
	}
}

#if 0
/*
 *	read device ready pin
 */
static int ppchameleonevb_device_ready(struct mtd_info *minfo)
{
	return 1;
}
#endif

#ifdef CONFIG_MTD_PARTITIONS
const char *part_probes_evb[] = { "cmdlinepart", NULL };
#endif

/*
 * Main initialization routine
 */
static int __init ppchameleonevb_init (void)
{
	struct nand_chip *this;
	const char *part_type = 0;
	int mtd_parts_nb = 0;
	struct mtd_partition *mtd_parts = 0;
	int ppchameleonevb_fio_base;


	/* Allocate memory for MTD device structure and private data */
	ppchameleonevb_mtd = kmalloc(sizeof(struct mtd_info) +
			     sizeof(struct nand_chip),
			     GFP_KERNEL);
	if (!ppchameleonevb_mtd) {
		printk("Unable to allocate PPChameleonEVB NAND MTD device structure.\n");
		return -ENOMEM;
	}

	/* map physical address */
	ppchameleonevb_fio_base = (unsigned long)ioremap(ppchameleonevb_fio_pbase, SZ_4M);
	if(!ppchameleonevb_fio_base) {
		printk("ioremap PPChameleonEVB NAND flash failed\n");
		kfree(ppchameleonevb_mtd);
		return -EIO;
	}

	/* Get pointer to private data */
	this = (struct nand_chip *) (&ppchameleonevb_mtd[1]);

	/* Initialize structures */
	memset((char *) ppchameleonevb_mtd, 0, sizeof(struct mtd_info));
	memset((char *) this, 0, sizeof(struct nand_chip));

	/* Link the private data with the MTD structure */
	ppchameleonevb_mtd->priv = this;

        /* Initialize GPIOs */
	/* Pin mapping for NAND chip */
	/*
		CE	GPIO_14
		CLE	GPIO_15
		ALE	GPIO_16
		R/B	GPIO_31 (still not used)
	*/
	/* output select */
	out_be32(GPIO0_OSRH, in_be32(GPIO0_OSRH) & 0xFFFFFFF0);
	out_be32(GPIO0_OSRL, in_be32(GPIO0_OSRL) & 0x3FFFFFFF);
	/* three-state select */
	out_be32(GPIO0_TSRH, in_be32(GPIO0_TSRH) & 0xFFFFFFF0);
	out_be32(GPIO0_TSRL, in_be32(GPIO0_TSRL) & 0x3FFFFFFF);
	/* enable output driver */
	out_be32(GPIO0_TCR, in_be32(GPIO0_TCR) | NAND_nCE_GPIO_PIN | NAND_CLE_GPIO_PIN | NAND_ALE_GPIO_PIN);

	/* insert callbacks */
	this->IO_ADDR_R = ppchameleonevb_fio_base;
	this->IO_ADDR_W = ppchameleonevb_fio_base;
	this->hwcontrol = ppchameleonevb_hwcontrol;
	/*this->dev_ready = ppchameleonevb_device_ready;*/
        this->dev_ready = NULL;
	/* 12 us command delay time */
	this->chip_delay = 12;
	/* ECC mode */
	this->eccmode = NAND_ECC_SOFT;


	/* Scan to find existence of the device */
	if (nand_scan (ppchameleonevb_mtd, 1)) {
		iounmap((void *)ppchameleonevb_fio_base);
		kfree (ppchameleonevb_mtd);
		return -ENXIO;
	}



	/* Allocate memory for internal data buffer */
	this->data_buf = kmalloc (sizeof(u_char) * (ppchameleonevb_mtd->oobblock + ppchameleonevb_mtd->oobsize), GFP_KERNEL);
	if (!this->data_buf) {
		printk("Unable to allocate NAND data buffer for PPChameleonEVB.\n");
		iounmap((void *)ppchameleonevb_fio_base);
		kfree (ppchameleonevb_mtd);
		return -ENOMEM;

	}


#ifdef CONFIG_MTD_PARTITIONS
	ppchameleonevb_mtd->name = "ppchameleonevb-nand";
	mtd_parts_nb = parse_mtd_partitions(ppchameleonevb_mtd, part_probes_evb, &mtd_parts, 0);
	if (mtd_parts_nb > 0)
	  part_type = "command line";
	else
	  mtd_parts_nb = 0;
#endif
	if (mtd_parts_nb == 0)
	{
		mtd_parts = partition_info;
		mtd_parts_nb = NUM_PARTITIONS;
		part_type = "static";
	}



	/* Register the partitions */
	printk(KERN_NOTICE "Using %s partition definition\n", part_type);
	add_mtd_partitions(ppchameleonevb_mtd, mtd_parts, mtd_parts_nb);

	/* Return happy */
	return 0;
}
module_init(ppchameleonevb_init);

/*
 * Clean up routine
 */
static void __exit ppchameleonevb_cleanup (void)
{
	struct nand_chip *this = (struct nand_chip *) &ppchameleonevb_mtd[1];

	/* Unregister the device */
	del_mtd_device (ppchameleonevb_mtd);

	/* Free internal data buffer */
	kfree (this->data_buf);

	/* Free the MTD device structure */
	kfree (ppchameleonevb_mtd);
}
module_exit(ppchameleonevb_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("llandre <llandre@wawnet.biz>");
MODULE_DESCRIPTION("MTD map driver for DAVE Srl PPChameleonEVB board");
