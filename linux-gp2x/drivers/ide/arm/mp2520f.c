/*
 *  IDE driver
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 * 	I/O base  : 0x3c000000[shadow == 0] or 0xbc000000[shadow == 1] 
 * 	IDE reg.s : 0x0 ~ 0x7,  0x8 ~ 0xf
 * 	IDE IRQ   : GPIO IRQ_GPIO_D13
 */

#include <linux/module.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/ide.h>
#include <linux/delay.h>

#ifdef CONFIG_ARCH_MMSP2 
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/hardware.h>
#include <asm/arch/mdk.h>
#endif
#include "mp2520f.h"

#define MAX_MMIO_NUM_HWIFS 1  /* ide0 */

#define MM_IDE_REG(x)		((x) - 0x1f0) /* 0x1f0 ~ 0x1f7 -> 0x00 ~ 0x07 */
#define MM_IDE_ALT_REG(x)	((x) - 0x3f0 + 0x8) /* 0x3f0 ~ 0x3f7 -> 0x08 ~ 0x0f */

static void wr_attr_mem(u32 attr_addr, u8 data);
static u8 rd_attr_mem(u32 attr_addr);
static void print_cis(struct cf_slot *slot);
void inline __dump_ide_offsets(hw_regs_t * ide);

struct cf_slot slot;
static hw_regs_t hw;

static int mmsp2ide_offsets[IDE_NR_PORTS] __initdata = {
	[ IDE_DATA_OFFSET    ] = MM_IDE_REG(HD_DATA), 
	[ IDE_ERROR_OFFSET   ] = MM_IDE_REG(HD_ERROR), 
	[ IDE_NSECTOR_OFFSET ] = MM_IDE_REG(HD_NSECTOR), 
	[ IDE_SECTOR_OFFSET  ] = MM_IDE_REG(HD_SECTOR),
	[ IDE_LCYL_OFFSET    ] = MM_IDE_REG(HD_LCYL), 
	[ IDE_HCYL_OFFSET    ] = MM_IDE_REG(HD_HCYL), 
	[ IDE_SELECT_OFFSET  ] = MM_IDE_REG(HD_CURRENT), 
	[ IDE_STATUS_OFFSET  ] = MM_IDE_REG(HD_STATUS),
	[ IDE_CONTROL_OFFSET ] = MM_IDE_ALT_REG(HD_ALTSTATUS),
	[ IDE_IRQ_OFFSET     ] = 0
};

/*
 *  Probe for a MMSP2 IDE interface
 */
int __init mmsp2_ide_init(void)
{	
	int index = -1;
	int cf_cd, cf_rdy, ide_rdy;


#ifdef CONFIG_MACH_MMSP2_DTK4
    /* Speed up the IDE Bus timing */
	MEMIDETIME = 0x002;		/* BANK C IDE Timing Control */	
	MEMEIDEWAIT = 0x001;		/* IDE Wait Timing */
#endif
	/* Bank C Configuration Register => IDE Buf Set */
	MEMCFG = (MEMCFG & 0xf0ff) | 0x0800;

	/* GPIO pin Set */ 
	set_gpio_ctrl(GPIO_J7, GPIOMD_ALT1, GPIOPU_NOSET);	// nPREG
	set_gpio_ctrl(GPIO_J9, GPIOMD_ALT1, GPIOPU_NOSET);	// nPIOW
	set_gpio_ctrl(GPIO_J10, GPIOMD_ALT1, GPIOPU_NOSET);	// nPIOR
	set_gpio_ctrl(GPIO_J11, GPIOMD_ALT1, GPIOPU_NOSET);	// nPWE
	set_gpio_ctrl(GPIO_J12, GPIOMD_ALT1, GPIOPU_NOSET);	// nPOE

#ifdef CONFIG_MACH_MMSP2_DTK4
	set_gpio_ctrl(GPIO_D9, GPIOMD_OUT, GPIOPU_EN);		// nRST
	set_gpio_ctrl(GPIO_F0, GPIOMD_OUT, GPIOPU_EN);		// GPIO_P1EN
	set_gpio_ctrl(GPIO_F1, GPIOMD_OUT, GPIOPU_EN);		// GPIO_P0EN
	set_gpio_ctrl(GPIO_O4, GPIOMD_OUT, GPIOPU_EN);		// nP0VCC3EN
	set_gpio_ctrl(GPIO_O5, GPIOMD_OUT, GPIOPU_EN);		// nP0VCC5EN
	set_gpio_ctrl(GPIO_D5, GPIOMD_IN, GPIOPU_EN);		// nCF1_CD
	set_gpio_ctrl(GPIO_D6, GPIOMD_IN, GPIOPU_EN);		// nCF1_S1_VS2
	set_gpio_ctrl(GPIO_D7, GPIOMD_IN, GPIOPU_EN);		// nCF1_S1_VS1

	/* Initialize GPIO */
	write_gpio_bit(GPIO_F0, 0);	// GPIO_P1EN (Disable)
	write_gpio_bit(GPIO_F1, 0);	// GPIO_P0EN (Disable)
	write_gpio_bit(GPIO_O4, 0);	// nP0VCC3EN (Disable)
	write_gpio_bit(GPIO_O5, 0);	// nP0VCC5EN (Disable)
	
	/* Reset Off */
	write_gpio_bit(GPIO_D9, 0);

	/* Check Card Detect */
	cf_cd =  read_gpio_bit(GPIO_D5);
	if(!cf_cd) {
		printk("CF Card is Detected\n");
		DPRINTK("Supply 3.3V\n");

		/* Power Enable */
		write_gpio_bit(GPIO_O5, 1);
		mdelay(10);

		/* Reset On */
		write_gpio_bit(GPIO_D9, 1);
		mdelay(10);

		/* Reset Off */
		write_gpio_bit(GPIO_D9, 0);
		mdelay(10);

		set_gpio_ctrl(GPIO_D8, GPIOMD_IN, GPIOPU_EN);	// nRDY
		cf_rdy = read_gpio_bit(GPIO_D8);
		mdelay(10);
		/* CF card Ready status check */
		while(cf_rdy);
		mdelay(10);
		DPRINTK(" --> Ready OK\n");

		/* Register IRQ */
		set_external_irq(IRQ_CF, EINT_LOW_LEVEL, GPIOPU_EN);

#elif defined(CONFIG_MACH_MMSP2_DTK3)
	if((S1_STSR & S1_nCD)==0) {
		/* Reset */
		S1_CONR |= S1_RESET_H;

		printk("CF Card is Detected\n");
		DPRINTK("Supply 3.3V\n");
		
		/* power on*/
		S1_CONR |= S1_VCC5EN_H;
	
		/* Reset On*/	    	
		S1_CONR &= S1_RESET_L;

		/* Reset Off*/	    	
		S1_CONR |= S1_RESET_H;

		/* CF card Ready status check */
		while( !(S1_STSR & S1_RDY) );
		DPRINTK(" --> Ready OK\n");

		/* Register IRQ */
		set_external_irq(IRQ_CF, EINT_HIGH_LEVEL, GPIOPU_EN);

		/* Interrupt Enable Register */
		NORINTENR &= PCMCIA_RDY_DIS;	/* PCMCIA Ready  Interrupt -> Disable */
		NORINTENR |= CF_RDY_EN;		/* CF     Ready  Interrupt -> Enable  */
		NORINTENR &= PCMCIA_BVD_DIS;	/* PCMCIA BVD    Interrupt -> Disable */
		NORINTENR &= CF_BVD_DIS;	/* CF     BVD    Interrupt -> Disable */
		NORINTENR &= EX_IRQ_DIS;	/* EX     IRQ    Interrupt -> Disable  */
	
		/* Interrupt Mode Register */
		NORINTMDR |= PCMCIA_RDYMOD_H;	/* PCMCIA Ready  Interrupt -> Low Level */
		//NORINTMDR &= PCMCIA_RDYMOD_L;	/* PCMCIA Ready  Interrupt -> Low Level */
		NORINTMDR &= CF_RDYMOD_L;	/* CF     Ready  Interrupt -> Low Level */
		NORINTMDR &= PCMCIA_BVDMOD_L;	/* PCMCIA BVD    Interrupt -> Low Level */
		NORINTMDR |= CF_BVDMOD_H;	/* CF     BVD    Interrupt -> Low Level */
		//NORINTMDR &= CF_BVDMOD_L;	/* CF     BVD    Interrupt -> Low Level */

		/* Interrupt Pending Register Clear */
		NORINTPNR = PCMCIA_RDYPN_CL | CF_RDYPN_CL | PCMCIA_BVDPN_CL | CF_BVDPN_CL;
#endif
		/* I/O Mapped Mode Set */
		wr_attr_mem(0x200, 0x45);

		/* Card Information Structure */
		//print_cis(&slot);

		memset(&hw, 0, sizeof(hw_regs_t));		

		/* do not use IDE_IRQ_REG */
//		mmsp2ide_offsets[IDE_IRQ_OFFSET] = 0;	
//		mmsp2ide_offsets[IDE_IRQ_OFFSET] -= CF1_VIO_BASE;
#ifdef CONFIG_MACH_MMSP2_DTK4
		ide_setup_ports(&hw, (ide_ioreg_t)CF0_VIO_BASE, mmsp2ide_offsets, 0, 0, NULL, IRQ_CF);
#elif defined(CONFIG_MACH_MMSP2_DTK3)
		ide_setup_ports(&hw, (ide_ioreg_t)CF1_VIO_BASE, mmsp2ide_offsets, 0, 0, NULL, IRQ_CF);
#endif
		index = ide_register_hw(&hw, NULL);

		// Enhance the transfer rate, brian, 04/11/22
		mmsp2_set_default_features(&hw);

		if (index != -1) 
			printk("ide%d: MMSP2 IDE-CF interface\n", index); // ok?
		__dump_ide_offsets(&hw);
	}
	
	else {
		set_gpio_ctrl(GPIO_IDE_nSWRST, GPIOMD_OUT, GPIOPU_NOSET); 
		write_gpio_bit(GPIO_IDE_nSWRST, 0);                       // see mdk.h

		set_gpio_ctrl(GPIO_J10, GPIOMD_ALT1, GPIOPU_NOSET); // nIOR
		set_gpio_ctrl(GPIO_J9, GPIOMD_ALT1, GPIOPU_NOSET); // nIOW

#ifdef CONFIG_MACH_MMSP2_DTK3
		set_gpio_ctrl(GPIO_J6, GPIOMD_ALT1, GPIOPU_NOSET); // nIOIS16
#endif

		set_gpio_ctrl(GPIO_I5, GPIOMD_ALT1, GPIOPU_NOSET); // nICS1
		set_gpio_ctrl(GPIO_I4, GPIOMD_ALT1, GPIOPU_NOSET); // nICS0

		set_gpio_ctrl(GPIO_G0, GPIOMD_ALT1, GPIOPU_NOSET); // ZA0
		set_gpio_ctrl(GPIO_G1, GPIOMD_ALT1, GPIOPU_NOSET); // ZA1
		set_gpio_ctrl(GPIO_G2, GPIOMD_ALT1, GPIOPU_NOSET); // ZA2

		set_gpio_ctrl(GPIO_E0, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[0]
		set_gpio_ctrl(GPIO_E1, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[1]
		set_gpio_ctrl(GPIO_E2, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[2]
		set_gpio_ctrl(GPIO_E3, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[3]
		set_gpio_ctrl(GPIO_E4, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[4]
		set_gpio_ctrl(GPIO_E5, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[5]
		set_gpio_ctrl(GPIO_E6, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[6]
		set_gpio_ctrl(GPIO_E7, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[7]
		set_gpio_ctrl(GPIO_E8, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[8]
		set_gpio_ctrl(GPIO_E9, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[9]
		set_gpio_ctrl(GPIO_E10, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[10]
		set_gpio_ctrl(GPIO_E11, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[11]
		set_gpio_ctrl(GPIO_E12, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[12]
		set_gpio_ctrl(GPIO_E13, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[13]
		set_gpio_ctrl(GPIO_E14, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[14]
		set_gpio_ctrl(GPIO_E15, GPIOMD_ALT1, GPIOPU_NOSET); // ZD[15]

		/* 
		 * taiwan kernel & mizi kernel: done at arch/arm/mach-mmsp2/mdk.c
		 */
		set_gpio_ctrl(GPIO_I13, GPIOMD_ALT1, GPIOPU_NOSET); // nWAIT
		//set_gpio_ctrl(GPIO_I4, GPIOMD_ALT1, GPIOPU_NOSET); // nBUFOE
		//set_gpio_ctrl(GPIO_I3, GPIOMD_ALT1, GPIOPU_NOSET); // nBUFENB
	
		write_gpio_bit(GPIO_IDE_nSWRST, 1); // software reset done

		memset(&hw, 0, sizeof(hw_regs_t));		

		set_external_irq(IRQ_HARD, EINT_HIGH_LEVEL, GPIOPU_EN);
	
		/* do not use IDE_IRQ_REG */
		mmsp2ide_offsets[IDE_IRQ_OFFSET] -= IDE_VIO_BASE;

		ide_setup_ports(&hw, (ide_ioreg_t)IDE_VIO_BASE, mmsp2ide_offsets, 0, 0, NULL, IRQ_HARD);
    
		index = ide_register_hw(&hw, NULL);

		// Enhance the transfer rate
		mmsp2_set_default_features(&hw);

	   	if(index != -1) 
			printk("ide%d: MMSP2 IDE interface\n", index); // ok?

		__dump_ide_offsets(&hw);
	}
	return 0;
}

int mmsp2_set_default_features(hw_regs_t *hw)
{
#define OUTB(offset,value)	(*(volatile u8 *)(hw->io_ports[offset]) = value)
#define INB(offset)	(*(volatile u8 *)(hw->io_ports[offset]))


	OUTB(IDE_FEATURE_OFFSET, SETFEATURES_XFER);
	// Set the IDE Disk to PIO #3 flow control mode
	OUTB(IDE_NSECTOR_OFFSET, 0x0B);
	OUTB(IDE_SELECT_OFFSET, 0xA0);
	OUTB(IDE_COMMAND_OFFSET, WIN_SETFEATURES);
	while ((INB(IDE_STATUS_OFFSET) & BUSY_STAT) != 0);

	OUTB(IDE_FEATURE_OFFSET, SETFEATURES_EN_RLA);
	OUTB(IDE_SELECT_OFFSET, 0xA0);
	OUTB(IDE_COMMAND_OFFSET, WIN_SETFEATURES);
	while ((INB(IDE_STATUS_OFFSET) & BUSY_STAT) != 0);

	OUTB(IDE_FEATURE_OFFSET, SETFEATURES_EN_WCACHE);
	OUTB(IDE_SELECT_OFFSET, 0xA0);
	OUTB(IDE_COMMAND_OFFSET, WIN_SETFEATURES);
	while ((INB(IDE_STATUS_OFFSET) & BUSY_STAT) != 0);

	return 1;
}

void inline __dump_ide_offsets(hw_regs_t * ide)
{
	int i;

	DPRINTK("-- ide --\n");
	for (i = 0; i < IDE_NR_PORTS; i++) {
		DPRINTK("0x%08lx\n", ide->io_ports[i]);
	}
	DPRINTK("IRQ : %d\n", ide->irq);
}

static void wr_attr_mem(u32 attr_addr, u8 data)
{
#ifdef CONFIG_MACH_MMSP2_DTK4
	__REGB(CF0_VATTR_BASE + attr_addr) = data;
#elif defined(CONFIG_MACH_MMSP2_DTK3)
	__REGB(CF1_VATTR_BASE + attr_addr) = data;
#endif
}

static u8 rd_attr_mem(u32 attr_addr)
{
#ifdef CONFIG_MACH_MMSP2_DTK4
	return __REGB(CF0_VATTR_BASE + attr_addr);
#elif defined(CONFIG_MACH_MMSP2_DTK3)
	return __REGB(CF1_VATTR_BASE + attr_addr);
#endif
}

static void print_cis(struct cf_slot *slot)
{
	char	data;
	u32	offset=0;
	u32	i, num_bytes;
	u32	conf_base;

	while(1) 
	{
		/* Read tuple ID */
		slot->cis.tuple_id = rd_attr_mem(offset);

  		if (slot->cis.tuple_id == 0xFF) // End of Chain 
  		{
  			DPRINTK("End of chain\n");
			break;		
		}
		offset+=2;

		/* Read offset Size */
		slot->cis.size = rd_attr_mem(offset);
		offset+=2;
		pData = buff;

        	/* Read data */
   		for (i=0; i<slot->cis.size; i++, offset+=2) 
		{
			data = rd_attr_mem(offset);

			if ((i%8) == 0) 
				DPRINTK ("\n[%04x] DATA = %x", offset, data);
			*pData++ = data;
		}
		pData = buff;

		switch (slot->cis.tuple_id) 
		{
		/* Product Information Tuple Case */
		case 0x15:
			/* Read 'Major version number' */
			if (*pData == 4)	slot->cis.major_vers = "for 2.01/2.1";
			else			slot->cis.major_vers = "for PCCard 95";
			pData++;
	
			/* Read 'Minorversion number' */
			if (*pData == 1)	slot->cis.minor_vers = "for 2.0/2.01/2.1";
			else			slot->cis.minor_vers = "for PCCard 95";
			pData++;
			
			/* Read 'Name of Manufacturer' */
			slot->cis.manu_id = pData;

			/* Read 'Name of Product' */
			pData += (strlen(pData) + 1);
			slot->cis.pro_nm = pData;

			/* Read 'Additional product information 1' */
			pData += (strlen(pData) + 1);    			
			slot->cis.pro_info1 = pData;

			/* Read 'Additional product information 2' */
			pData += (strlen(pData) + 1);
			slot->cis.pro_info2 = pData;

			DPRINTK("\n");
			DPRINTK("*********** CIS ***********\n");
			DPRINTK(" Tuple ID = 0x%x\n Major version = %s\n Minor version = %s\n Menufactuer ID = %s\n"
				" Product name = %s\n Product info 1 = %s\n Product info 2 = %s\n\n", 
				slot->cis.tuple_id, slot->cis.major_vers, slot->cis.minor_vers, slot->cis.manu_id, 
				slot->cis.pro_nm, slot->cis.pro_info1, slot->cis.pro_info2);
			DPRINTK("***************************\n");
 		
		/* Configuration Tuple Case */
		case 0x1a:
		 	/* Read 'Size of Configuration registers base address field' */
			num_bytes = ((*pData) & 0x3)+1;
   			
   			pData += 2;
   			conf_base = 0;
   			
   			/* Read 'Configuration register base address field' */
			for (i=num_bytes; i>0; i--) 
			{
				conf_base <<= 8;
				conf_base  |= *(pData + i - 1);
			}
			break;

		default:
			break;
		}
	}
}

void __exit mmsp2_ide_exit(void)
{
    printk("mmsp2_ide_exit call\n");
}

module_init(mmsp2_ide_init);
module_exit(mmsp2_ide_exit);

MODULE_AUTHOR("DIGNSYS Inc.(www.dignsys.com)");
