/*
 * (C) Copyright 2004
 * Magic eye MMSP2 DTK board 
 * DIGNSYS Inc. < www.dignsys.com >
 * Kane Ahn < hbahn@dignsys.com >
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * If we are developing, we might want to start armboot from ram
 * so we MUST NOT initialize critical regs like mem-timing ...
 */
#define CONFIG_INIT_CRITICAL		/* undef for developing */

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM920T		1	/* This is an ARM920T Core	*/
#define	CONFIG_MMSP20		1	/* in a Magic eye MMSP20 SoC     */
#define CONFIG_MMSP2DTK		1	/* on a Magic eye MMSP2 DTK Board  */
#define CFG_SHADOW     		1 	/* Memory Bank Shadow enable */

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	7372800/* the MMSP2 DTK  has 7.3728MHz input clock */
#define USE_920T_MMU		1
//#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */
#define CONFIG_USE_IRQ

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)

/*
 * Hardware drivers
 */
#define	CONFIG_DRIVER_CS8900	1	/* we have a CS8900 on-board */
#define	CS8900_BASE		0x84000300 /* MMSP2 DTK use static bank 1 */
#define	CS8900_BUS16		1	/* the Linux driver does accesses as shorts */

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1          1	/* we use SERIAL 2 on MMSP20 */

/************************************************************
 * RTC
 ************************************************************/
#define	CONFIG_RTC_MMSP20	1

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200

#define CONFIG_MMC 1
#define CONFIG_DOS_PARTITION 1
#define CONFIG_SUPPORT_VFAT 1

/***********************************************************
 * Command definition
 ***********************************************************/
#define CONFIG_COMMANDS \
			(CONFIG_CMD_DFL  | \
			CFG_CMD_CACHE	 | \
			CFG_CMD_NAND	 | \
			CFG_CMD_LOADUB   | \
			/*CFG_CMD_EEPROM |*/ \
			/*CFG_CMD_I2C	 |*/ \
			/*CFG_CMD_USB	 |*/ \
			CFG_CMD_FAT      | \
			CFG_CMD_SD       | \
			CFG_CMD_MMC      | \
			CFG_CMD_REGINFO  | \
			CFG_CMD_DATE	 | \
			CFG_CMD_ELF)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define	CONFIG_BOOTDELAY	0
#define	CONFIG_BOOTARGS    	"root=ramfs devfs=mount console=ttyS0,115200"
#define	CONFIG_ETHADDR		00:11:22:33:44:55
#define	CONFIG_NETMASK		255.255.255.0
#define	CONFIG_IPADDR		192.168.1.225
#define	CONFIG_SERVERIP		192.168.1.230
#define	CONFIG_BOOTFILE		"bios.bin"
//#define	CONFIG_BOOTCOMMAND	"nand read 0x1000000 0x80000 0xE0000; bootm" 
#define	CONFIG_BOOTCOMMAND	"bootm" 
/* #define	CONFIG_BOOTCOMMAND	"nand read 0x1000000 0xD0000 0x130000; bootm" */	
/*#define	CONFIG_BOOTCOMMAND	"tftp; bootm" */

#if (CONFIG_COMMANDS & CFG_CMD_LOADUB)
#define CONFIG_DRIVER_USB_DEVICE
#define CONFIG_LOADUB_PATH  "/tftpboot/"    /* loadub default path */
#endif

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"GP2X# "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00100000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x03E00000	/* 64 MB in DRAM	*/

#undef  CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define	CFG_LOAD_ADDR		0x01000000	/* default load address	*/

/* the PWM TImer 4 uses a counter of 15625 for 10 ms, so we need */
/* it to wrap 100 times (total 1562500) to get 1 sec. */
#define	CFG_HZ			7372800

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x00100000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x03F00000 /* 63 MB */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */

#define CFG_FLASH_BASE		PHYS_FLASH_1 

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	0	/* max number of memory banks */

#define PHYS_FLASH_SIZE		0x00000000 /* 1MB */
#define CFG_MAX_FLASH_SECT	(1)	/* max number of sectors on one chip */
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x0F0000) /* addr of environment */

#ifdef CONFIG_AMD_LV800
#define PHYS_FLASH_SIZE		0x00100000 /* 1MB */
#define CFG_MAX_FLASH_SECT	(19)	/* max number of sectors on one chip */
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x0F0000) /* addr of environment */
#endif

#ifdef CONFIG_AMD_LV400
#define PHYS_FLASH_SIZE		0x00080000 /* 512KB */
#define CFG_MAX_FLASH_SECT	(11)	/* max number of sectors on one chip */
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x070000) /* addr of environment */
#endif 

/* timeout values are in ticks */

#define CFG_FLASH_ERASE_TOUT	(1*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(1*CFG_HZ) /* Timeout for Flash Write */


/*-----------------------------------------------------------------------
 * NAND FLASH and environment organization
 */

#define CONFIG_YAFFS                    /* use yaffs filesystem on nand flash */

#define CFG_MAX_NAND_DEVICE     1       /* Max number of NAND devices           */
#define SECTORSIZE 512

#define ADDR_COLUMN 1
#define ADDR_PAGE 2
#define ADDR_COLUMN_PAGE 3

#define NAND_ChipID_UNKNOWN     0x00
#define NAND_MAX_FLOORS 1
#define NAND_MAX_CHIPS 1

#define NAND_IO_BASE_ADDR	0xC0003A00
#ifdef CFG_SHADOW
    #define NAND_MEM_BASE_ADDR	0x9C000000
#else
    #define NAND_MEM_BASE_ADDR	0x1C000000
#endif /* CFG_SHADOW */

#define __NFDATA (*(volatile unsigned char *)0x9c000000)
#define __NFCMD  (*(volatile unsigned char *)0x9C000010)
#define __NFADDR (*(volatile unsigned char *)0x9C000018)

#define __MEMCFGW      (*(volatile unsigned short *)0xC0003A00)
#define __MEMNANDCTRLW (*(volatile unsigned short *)0xC0003A3A)
#define __MEMNANDTIMEW (*(volatile unsigned short *)0xC0003A3C)

#define NAND_INT_DETRnB		0x0080
#define NAND_PEND_DETRnB	0x8000
#define NAND_CLR_DETRnB		0x8000

#define NAND_ENABLE_CE(nand)  do {__MEMNANDCTRLW |= NAND_CLR_DETRnB; __MEMNANDCTRLW |= NAND_INT_DETRnB;} while(0)
#define NAND_DISABLE_CE(nand) do {__MEMNANDCTRLW &= (~NAND_INT_DETRnB); __MEMNANDCTRLW |= NAND_CLR_DETRnB;} while(0)
#define NAND_WAIT_READY(nand) while( ((__MEMNANDCTRLW & NAND_PEND_DETRnB) & NAND_PEND_DETRnB)?0:1 )

#define NAND_CTL_CLRALE(nandptr) do { ; } while(0)
#define NAND_CTL_SETALE(nandptr) do { ; } while(0)
#define NAND_CTL_CLRCLE(nandptr) do { ; } while(0)
#define NAND_CTL_SETCLE(nandptr) do { ; } while(0)

#define WRITE_NAND_COMMAND(d, adr) (__NFCMD  = (unsigned char)(d))
#define WRITE_NAND_ADDRESS(d, adr) (__NFADDR = (unsigned char)(d))
#define WRITE_NAND(d, adr) (__NFDATA = (__u8)d);
#define READ_NAND(adr) ((unsigned char)(__NFDATA))

/*-----------------------------------------------------------------------
 * environment organization
 */
#undef	CFG_ENV_IS_IN_FLASH	0
#define	CFG_ENV_IS_IN_NAND	1

#if 0
#undef CFG_ENV_ADDR
#define CFG_ENV_ADDR		0x20000		/* addr of environment in NAND */
#define CFG_ENV_SIZE		0x4000		/* Total Size of Environment Sector */
#define CFG_NAND_ENV_BUFFER	0x03F00000	/* The environment buffer in system memory */
#endif

#if 1
#undef CFG_ENV_ADDR
#define CFG_ENV_ADDR		0x70000		/* addr of environment in NAND */
#define CFG_ENV_SIZE		0x4000		/* Total Size of Environment Sector */
#define CFG_NAND_ENV_BUFFER	0x03F80000	/* The environment buffer in system memory */
#endif

#if 0
#undef CFG_ENV_ADDR
#define CFG_ENV_ADDR		0xB0000		/* addr of environment in NAND */
#define CFG_ENV_SIZE		0x4000		/* Total Size of Environment Sector */
#define CFG_NAND_ENV_BUFFER	0x03F80000	/* The environment buffer in system memory */
#endif


#endif	/* __CONFIG_H */
