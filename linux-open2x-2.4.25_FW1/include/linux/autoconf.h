/*
 * Automatically generated C config: don't edit
 */
#define AUTOCONF_INCLUDED
#define CONFIG_ARM 1
#undef  CONFIG_EISA
#undef  CONFIG_SBUS
#undef  CONFIG_MCA
#define CONFIG_UID16 1
#define CONFIG_RWSEM_GENERIC_SPINLOCK 1
#undef  CONFIG_RWSEM_XCHGADD_ALGORITHM
#undef  CONFIG_GENERIC_BUST_SPINLOCK
#undef  CONFIG_GENERIC_ISA_DMA
/*
 * Code maturity level options
 */
#define CONFIG_EXPERIMENTAL 1
#undef  CONFIG_OBSOLETE
/*
 * Loadable module support
 */
#define CONFIG_MODULES 1
#undef  CONFIG_MODVERSIONS
#define CONFIG_KMOD 1
/*
 * System Type
 */
#undef  CONFIG_ARCH_ANAKIN
#undef  CONFIG_ARCH_ARCA5K
#undef  CONFIG_ARCH_CLPS7500
#undef  CONFIG_ARCH_CLPS711X
#undef  CONFIG_ARCH_CO285
#undef  CONFIG_ARCH_EBSA110
#undef  CONFIG_ARCH_CAMELOT
#undef  CONFIG_ARCH_FOOTBRIDGE
#undef  CONFIG_ARCH_INTEGRATOR
#undef  CONFIG_ARCH_OMAHA
#undef  CONFIG_ARCH_L7200
#undef  CONFIG_ARCH_MX1ADS
#undef  CONFIG_ARCH_RPC
#undef  CONFIG_ARCH_RISCSTATION
#undef  CONFIG_ARCH_SA1100
#undef  CONFIG_ARCH_SHARK
#define CONFIG_ARCH_MMSP2 1
#undef  CONFIG_ARCH_AT91RM9200
/*
 * Archimedes/A5000 Implementations
 */
/*
 * Archimedes/A5000 Implementations (select only ONE)
 */
/*
 * Footbridge Implementations
 */
/*
 * SA11x0 Implementations
 */
#undef  CONFIG_SA1100_H3XXX
/*
 * AT91RM9200 Implementations
 */
/*
 * CLPS711X/EP721X Implementations
 */
#undef  CONFIG_ARCH_EP7211
#undef  CONFIG_ARCH_EP7212
/*
 * MMSP2 Implementation
 */
#define CONFIG_MMSP2_SHADOW_ENABLE 1
#define CONFIG_MACH_MMSP2_MDK 1
#undef  CONFIG_MACH_MMSP2_DTK3
#define CONFIG_MACH_MMSP2_DTK4 1
#define CONFIG_MACH_GP2X 1
#undef  CONFIG_MMSP2_USBDEV
#undef  CONFIG_ARCH_ACORN
#undef  CONFIG_PLD
#undef  CONFIG_FOOTBRIDGE
#undef  CONFIG_FOOTBRIDGE_HOST
#undef  CONFIG_FOOTBRIDGE_ADDIN
/*
 * Processor Type
 */
#define CONFIG_CPU_32 1
#undef  CONFIG_CPU_26
#undef  CONFIG_CPU_ARM610
#undef  CONFIG_CPU_ARM710
#undef  CONFIG_CPU_ARM720T
#define CONFIG_CPU_ARM920T 1
#undef  CONFIG_CPU_ARM922T
#undef  CONFIG_CPU_ARM926T
#undef  CONFIG_CPU_ARM1020
#undef  CONFIG_CPU_ARM1020E
#undef  CONFIG_CPU_ARM1022
#undef  CONFIG_CPU_ARM1026
#undef  CONFIG_CPU_SA110
#undef  CONFIG_CPU_SA1100
#undef  CONFIG_CPU_32v3
#define CONFIG_CPU_32v4 1
/*
 * Processor Features
 */
#undef  CONFIG_DISCONTIGMEM
/*
 * General setup
 */
#undef  CONFIG_PCI
#undef  CONFIG_ISA
#undef  CONFIG_ISA_DMA
#undef  CONFIG_ZBOOT_ROM
#define CONFIG_ZBOOT_ROM_TEXT 0x0
#define CONFIG_ZBOOT_ROM_BSS 0x0
#undef  CONFIG_HOTPLUG
#undef  CONFIG_PCMCIA
#undef  CONFIG_NET
#define CONFIG_SYSVIPC 1
#undef  CONFIG_BSD_PROCESS_ACCT
#define CONFIG_SYSCTL 1
/*
 * At least one math emulation must be selected
 */
#define CONFIG_FPE_NWFPE 1
#undef  CONFIG_FPE_NWFPE_XP
#undef  CONFIG_FPE_FASTFPE
#define CONFIG_KCORE_ELF 1
#undef  CONFIG_KCORE_AOUT
#undef  CONFIG_BINFMT_AOUT
#define CONFIG_BINFMT_ELF 1
#undef  CONFIG_BINFMT_MISC
#undef  CONFIG_PM
#undef  CONFIG_ARTHUR
#define CONFIG_CMDLINE "root=/dev/mtdblock3 rw"
#define CONFIG_ALIGNMENT_TRAP 1
/*
 * Parallel port support
 */
#undef  CONFIG_PARPORT
/*
 * Memory Technology Devices (MTD)
 */
#define CONFIG_MTD 1
#undef  CONFIG_MTD_DEBUG
#define CONFIG_MTD_PARTITIONS 1
#undef  CONFIG_MTD_CONCAT
#undef  CONFIG_MTD_REDBOOT_PARTS
#undef  CONFIG_MTD_CMDLINE_PARTS
#undef  CONFIG_MTD_AFS_PARTS
/*
 * User Modules And Translation Layers
 */
#define CONFIG_MTD_CHAR 1
#define CONFIG_MTD_BLOCK 1
#undef  CONFIG_FTL
#undef  CONFIG_NFTL
#undef  CONFIG_INFTL
/*
 * RAM/ROM/Flash chip drivers
 */
#undef  CONFIG_MTD_CFI
#undef  CONFIG_MTD_JEDECPROBE
#undef  CONFIG_MTD_GEN_PROBE
#undef  CONFIG_MTD_CFI_UTIL
#undef  CONFIG_MTD_RAM
#undef  CONFIG_MTD_ROM
#undef  CONFIG_MTD_ABSENT
#undef  CONFIG_MTD_OBSOLETE_CHIPS
/*
 * Mapping drivers for chip access
 */
#undef  CONFIG_MTD_COMPLEX_MAPPINGS
#undef  CONFIG_MTD_PHYSMAP
#undef  CONFIG_MTD_NOR_TOTO
/*
 * Self-contained MTD device drivers
 */
#undef  CONFIG_MTD_SLRAM
#undef  CONFIG_MTD_MTDRAM
#undef  CONFIG_MTD_BLKMTD
/*
 * Disk-On-Chip Device Drivers
 */
#undef  CONFIG_MTD_DOC2000
#undef  CONFIG_MTD_DOC2001
#undef  CONFIG_MTD_DOC2001PLUS
#undef  CONFIG_MTD_DOCPROBE
/*
 * NAND Flash Device Drivers
 */
#define CONFIG_MTD_NAND 1
#undef  CONFIG_MTD_NAND_VERIFY_WRITE
#undef  CONFIG_MTD_NAND_BONFS
#undef  CONFIG_MTD_NAND_TOTO
#undef  CONFIG_MTD_NAND_EDB7312
#define CONFIG_MTD_NAND_MP2520F 1
#undef  CONFIG_MTD_NAND_MP2520F_HWECC
#define CONFIG_MTD_NAND_IDS 1
/*
 * Plug and Play configuration
 */
#undef  CONFIG_PNP
/*
 * Block devices
 */
#undef  CONFIG_BLK_DEV_FD
#define CONFIG_BLK_DEV_LOOP 1
#undef  CONFIG_BLK_DEV_RAM
#undef  CONFIG_BLK_STATS
/*
 * Multi-device support (RAID and LVM)
 */
#undef  CONFIG_MD
/*
 * ATA/ATAPI/MFM/RLL support
 */
#undef  CONFIG_IDE
#undef  CONFIG_BLK_DEV_HD
/*
 * SCSI support
 */
#undef  CONFIG_SCSI
/*
 * I2O device support
 */
#undef  CONFIG_I2O
/*
 * ISDN subsystem
 */
#undef  CONFIG_ISDN
/*
 * Input core support
 */
#undef  CONFIG_INPUT
#define CONFIG_INPUT_MOUSEDEV_SCREEN_X (1024)
#define CONFIG_INPUT_MOUSEDEV_SCREEN_Y (768)
/*
 * Character devices
 */
#define CONFIG_VT 1
#undef  CONFIG_VT_CONSOLE
#undef  CONFIG_SERIAL
#undef  CONFIG_SERIAL_NONSTANDARD
#undef  CONFIG_IRDA_LIRC
/*
 * Serial drivers
 */
#define CONFIG_SERIAL_MMSP2 1
#define CONFIG_SERIAL_MMSP2_CONSOLE 1
#undef  CONFIG_SERIAL_8250
#define CONFIG_SERIAL_CORE 1
#define CONFIG_SERIAL_CORE_CONSOLE 1
#define CONFIG_UNIX98_PTYS 1
#define CONFIG_UNIX98_PTY_COUNT (256)
#undef  CONFIG_MMSP2_TPC
#define CONFIG_MMSP2_DUALCPU 1
#define CONFIG_MMSP2_VPP 1
#undef  CONFIG_MMSP2_ISP
#define CONFIG_MMSP2_COMM 1
#define CONFIG_MMSP2_KEY 1
#define CONFIG_MMSP_GPX_BATT 1
/*
 * I2C support
 */
#undef  CONFIG_I2C
/*
 * L3 serial bus support
 */
#undef  CONFIG_L3
/*
 * Other L3 adapters
 */
#undef  CONFIG_BIT_SA1100_GPIO
/*
 * Mice
 */
#undef  CONFIG_BUSMOUSE
#undef  CONFIG_MOUSE
/*
 * Joysticks
 */
#undef  CONFIG_INPUT_GAMEPORT
/*
 * Input core support is needed for gameports
 */
/*
 * Input core support is needed for joysticks
 */
#undef  CONFIG_QIC02_TAPE
#undef  CONFIG_IPMI_HANDLER
/*
 * Watchdog Cards
 */
#define CONFIG_WATCHDOG 1
#undef  CONFIG_WATCHDOG_NOWAYOUT
#undef  CONFIG_ACQUIRE_WDT
#undef  CONFIG_ADVANTECH_WDT
#undef  CONFIG_ALIM1535_WDT
#undef  CONFIG_ALIM7101_WDT
#undef  CONFIG_SC520_WDT
#undef  CONFIG_PCWATCHDOG
#undef  CONFIG_MMSP2_WATCHDOG
#undef  CONFIG_EUROTECH_WDT
#undef  CONFIG_IB700_WDT
#undef  CONFIG_WAFER_WDT
#undef  CONFIG_I810_TCO
#undef  CONFIG_MIXCOMWD
#undef  CONFIG_60XX_WDT
#undef  CONFIG_SC1200_WDT
#undef  CONFIG_SCx200_WDT
#undef  CONFIG_SOFT_WATCHDOG
#undef  CONFIG_W83877F_WDT
#undef  CONFIG_WDT
#undef  CONFIG_WDTPCI
#undef  CONFIG_MACHZ_WDT
#undef  CONFIG_AMD7XX_TCO
#undef  CONFIG_SCx200
#undef  CONFIG_NVRAM
#undef  CONFIG_RTC
#define CONFIG_MMSP2_RTC 1
#undef  CONFIG_DTLK
#undef  CONFIG_R3964
#undef  CONFIG_APPLICOM
/*
 * Ftape, the floppy tape device driver
 */
#undef  CONFIG_FTAPE
#undef  CONFIG_AGP
/*
 * Direct Rendering Manager (XFree86 DRI support)
 */
#undef  CONFIG_DRM
/*
 * Multimedia devices
 */
#define CONFIG_VIDEO_DEV 1
/*
 * Video For Linux
 */
#undef  CONFIG_VIDEO_PROC_FS
/*
 * Video Adapters
 */
#undef  CONFIG_VIDEO_PMS
#undef  CONFIG_VIDEO_OV9640
#define CONFIG_VIDEO_CX25874 1
#undef  CONFIG_VIDEO_SAA7113H
#undef  CONFIG_VIDEO_CPIA
/*
 * Radio Adapters
 */
#undef  CONFIG_RADIO_MAXIRADIO
#undef  CONFIG_RADIO_MAESTRO
/*
 * File systems
 */
#undef  CONFIG_QUOTA
#undef  CONFIG_AUTOFS_FS
#undef  CONFIG_AUTOFS4_FS
#undef  CONFIG_REISERFS_FS
#undef  CONFIG_ADFS_FS
#undef  CONFIG_AFFS_FS
#undef  CONFIG_HFS_FS
#undef  CONFIG_HFSPLUS_FS
#undef  CONFIG_BEFS_FS
#undef  CONFIG_BFS_FS
#define CONFIG_EXT3_FS 1
#define CONFIG_JBD 1
#define CONFIG_JBD_DEBUG 1
#define CONFIG_FAT_FS 1
#define CONFIG_MSDOS_FS 1
#undef  CONFIG_UMSDOS_FS
#define CONFIG_VFAT_FS 1
#undef  CONFIG_EFS_FS
#undef  CONFIG_JFFS_FS
#define CONFIG_JFFS2_FS 1
#define CONFIG_JFFS2_FS_DEBUG (0)
#define CONFIG_JFFS2_FS_NAND 1
#undef  CONFIG_CRAMFS
#define CONFIG_TMPFS 1
#define CONFIG_RAMFS 1
#undef  CONFIG_YAFFS
#undef  CONFIG_ISO9660_FS
#undef  CONFIG_JFS_FS
#undef  CONFIG_MINIX_FS
#undef  CONFIG_VXFS_FS
#undef  CONFIG_NTFS_FS
#undef  CONFIG_HPFS_FS
#define CONFIG_PROC_FS 1
#define CONFIG_DEVFS_FS 1
#define CONFIG_DEVFS_MOUNT 1
#undef  CONFIG_DEVFS_DEBUG
#define CONFIG_DEVPTS_FS 1
#undef  CONFIG_QNX4FS_FS
#undef  CONFIG_ROMFS_FS
#define CONFIG_EXT2_FS 1
#undef  CONFIG_SYSV_FS
#undef  CONFIG_UDF_FS
#undef  CONFIG_UFS_FS
#undef  CONFIG_XFS_FS
#undef  CONFIG_NCPFS_NLS
#undef  CONFIG_SMB_FS
#undef  CONFIG_ZISOFS_FS
/*
 * Partition Types
 */
#define CONFIG_PARTITION_ADVANCED 1
#undef  CONFIG_ACORN_PARTITION
#undef  CONFIG_OSF_PARTITION
#undef  CONFIG_AMIGA_PARTITION
#undef  CONFIG_ATARI_PARTITION
#undef  CONFIG_MAC_PARTITION
#define CONFIG_MSDOS_PARTITION 1
#undef  CONFIG_BSD_DISKLABEL
#undef  CONFIG_MINIX_SUBPARTITION
#undef  CONFIG_SOLARIS_X86_PARTITION
#undef  CONFIG_UNIXWARE_DISKLABEL
#undef  CONFIG_LDM_PARTITION
#undef  CONFIG_SGI_PARTITION
#undef  CONFIG_ULTRIX_PARTITION
#undef  CONFIG_SUN_PARTITION
#undef  CONFIG_EFI_PARTITION
#undef  CONFIG_SMB_NLS
#define CONFIG_NLS 1
/*
 * Native Language Support
 */
#define CONFIG_NLS_DEFAULT "CP949"
#undef  CONFIG_NLS_CODEPAGE_437
#undef  CONFIG_NLS_CODEPAGE_737
#undef  CONFIG_NLS_CODEPAGE_775
#undef  CONFIG_NLS_CODEPAGE_850
#undef  CONFIG_NLS_CODEPAGE_852
#undef  CONFIG_NLS_CODEPAGE_855
#undef  CONFIG_NLS_CODEPAGE_857
#undef  CONFIG_NLS_CODEPAGE_860
#undef  CONFIG_NLS_CODEPAGE_861
#undef  CONFIG_NLS_CODEPAGE_862
#undef  CONFIG_NLS_CODEPAGE_863
#undef  CONFIG_NLS_CODEPAGE_864
#undef  CONFIG_NLS_CODEPAGE_865
#undef  CONFIG_NLS_CODEPAGE_866
#undef  CONFIG_NLS_CODEPAGE_869
#undef  CONFIG_NLS_CODEPAGE_936
#undef  CONFIG_NLS_CODEPAGE_950
#undef  CONFIG_NLS_CODEPAGE_932
#define CONFIG_NLS_CODEPAGE_949 1
#undef  CONFIG_NLS_CODEPAGE_874
#undef  CONFIG_NLS_ISO8859_8
#undef  CONFIG_NLS_CODEPAGE_1250
#undef  CONFIG_NLS_CODEPAGE_1251
#undef  CONFIG_NLS_ISO8859_1
#undef  CONFIG_NLS_ISO8859_2
#undef  CONFIG_NLS_ISO8859_3
#undef  CONFIG_NLS_ISO8859_4
#undef  CONFIG_NLS_ISO8859_5
#undef  CONFIG_NLS_ISO8859_6
#undef  CONFIG_NLS_ISO8859_7
#undef  CONFIG_NLS_ISO8859_9
#undef  CONFIG_NLS_ISO8859_13
#undef  CONFIG_NLS_ISO8859_14
#undef  CONFIG_NLS_ISO8859_15
#undef  CONFIG_NLS_KOI8_R
#undef  CONFIG_NLS_KOI8_U
#define CONFIG_NLS_UTF8 1
/*
 * Console drivers
 */
#define CONFIG_PC_KEYMAP 1
#undef  CONFIG_VGA_CONSOLE
/*
 * Frame-buffer support
 */
#define CONFIG_FB 1
#define CONFIG_DUMMY_CONSOLE 1
#define CONFIG_FB_MMSP2 1
#undef  CONFIG_FB_VIRTUAL
#define CONFIG_FBCON_ADVANCED 1
#undef  CONFIG_FBCON_MFB
#undef  CONFIG_FBCON_CFB2
#undef  CONFIG_FBCON_CFB4
#undef  CONFIG_FBCON_CFB8
#define CONFIG_FBCON_CFB16 1
#undef  CONFIG_FBCON_CFB24
#undef  CONFIG_FBCON_CFB32
#undef  CONFIG_FBCON_AFB
#undef  CONFIG_FBCON_ILBM
#undef  CONFIG_FBCON_IPLAN2P2
#undef  CONFIG_FBCON_IPLAN2P4
#undef  CONFIG_FBCON_IPLAN2P8
#undef  CONFIG_FBCON_MAC
#undef  CONFIG_FBCON_VGA_PLANES
#undef  CONFIG_FBCON_VGA
#undef  CONFIG_FBCON_HGA
#define CONFIG_FBCON_FONTWIDTH8_ONLY 1
#define CONFIG_FBCON_FONTS 1
#define CONFIG_FONT_8x8 1
#undef  CONFIG_FONT_8x16
#undef  CONFIG_FONT_SUN8x16
#undef  CONFIG_FONT_PEARL_8x8
#undef  CONFIG_FONT_ACORN_8x8
/*
 * Sound
 */
#define CONFIG_SOUND 1
#undef  CONFIG_SOUND_MSNDCLAS
#undef  CONFIG_SOUND_MSNDPIN
#undef  CONFIG_SOUND_OSS
#define CONFIG_SOUND_MP2520F_AC97 1
#undef  CONFIG_SOUND_AD1980
#undef  CONFIG_SOUND_WM97XX
/*
 * Multimedia Capabilities Port drivers
 */
#undef  CONFIG_MCP
/*
 * USB support
 */
#define CONFIG_USB 1
#undef  CONFIG_USB_DEBUG
/*
 * Miscellaneous USB options
 */
#undef  CONFIG_USB_DEVICEFS
#undef  CONFIG_USB_BANDWIDTH
/*
 * USB Host Controller Drivers
 */
#undef  CONFIG_USB_EHCI_HCD
#undef  CONFIG_USB_UHCI
#undef  CONFIG_USB_UHCI_ALT
#undef  CONFIG_USB_OHCI
#undef  CONFIG_USB_OHCI_SA1111
#undef  CONFIG_USB_OHCI_MMSP2
#undef  CONFIG_USB_SL811HS_ALT
#undef  CONFIG_USB_SL811HS
/*
 * USB Device Class drivers
 */
#undef  CONFIG_USB_AUDIO
/*
 *   USB Bluetooth can only be used with disabled Bluetooth subsystem
 */
#undef  CONFIG_USB_MIDI
/*
 *   SCSI support is needed for USB Storage
 */
#undef  CONFIG_USB_ACM
#undef  CONFIG_USB_PRINTER
/*
 * USB Human Interface Devices (HID)
 */
#undef  CONFIG_USB_HID
/*
 *     Input core support is needed for USB HID input layer or HIDBP support
 */
/*
 * USB Imaging devices
 */
#undef  CONFIG_USB_DC2XX
#undef  CONFIG_USB_MDC800
#undef  CONFIG_USB_SCANNER
/*
 * USB Multimedia devices
 */
#undef  CONFIG_USB_IBMCAM
#undef  CONFIG_USB_KONICAWC
#undef  CONFIG_USB_OV511
#undef  CONFIG_USB_PWC
#undef  CONFIG_USB_SE401
#undef  CONFIG_USB_STV680
#undef  CONFIG_USB_VICAM
#undef  CONFIG_USB_DSBR
#undef  CONFIG_USB_DABUSB
/*
 * USB Network adaptors
 */
/*
 *   Networking support is needed for USB Networking device support
 */
/*
 * USB port drivers
 */
/*
 * USB Serial Converter support
 */
#undef  CONFIG_USB_SERIAL
/*
 * USB Miscellaneous drivers
 */
#undef  CONFIG_USB_RIO500
#undef  CONFIG_USB_AUERSWALD
#undef  CONFIG_USB_TIGL
#undef  CONFIG_USB_BRLVGER
#undef  CONFIG_USB_LCD
/*
 * Support for USB gadgets
 */
#undef  CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_MODULE 1
#define CONFIG_USB_GADGET_NET2272 1
#undef  CONFIG_USB_GADGET_NET2280
#undef  CONFIG_USB_GADGET_CONTROLLER
#undef  CONFIG_USB_NET2272
#define CONFIG_USB_NET2272_MODULE 1
#undef  CONFIG_USB_GADGET_CONTROLLER
#define CONFIG_USB_GADGET_CONTROLLER_MODULE 1
#define CONFIG_USB_GADGET_DUALSPEED 1
/*
 * USB Gadget Drivers...
 */
#undef  CONFIG_USB_ZERO
#define CONFIG_USB_ZERO_MODULE 1
#undef  CONFIG_USB_GADGETFS
#define CONFIG_USB_GADGETFS_MODULE 1
#undef  CONFIG_USB_FILE_STORAGE
#define CONFIG_USB_FILE_STORAGE_MODULE 1
#define CONFIG_USB_FILE_STORAGE_TEST 1
/*
 * MMC/SD Card support
 */
#define CONFIG_MMCSD 1
#undef  CONFIG_MMCSD_DEBUG
#define CONFIG_MMCSD_DISK 1
#define CONFIG_MMCSD_SLOT 1
/*
 * MMC/SD Controllers device drivers
 */
/*
 * Kernel hacking
 */
#define CONFIG_FRAME_POINTER 1
#define CONFIG_DEBUG_USER 1
#undef  CONFIG_DEBUG_INFO
#undef  CONFIG_DEBUG_KERNEL
/*
 * Library routines
 */
#undef  CONFIG_CRC32
#define CONFIG_ZLIB_INFLATE 1
#define CONFIG_ZLIB_DEFLATE 1
