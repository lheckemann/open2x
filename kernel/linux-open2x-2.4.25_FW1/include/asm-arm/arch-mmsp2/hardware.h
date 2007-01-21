/*
 * linux/include/asm-arm/arch-mmsp2/hardware.h
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <linux/config.h>
#include <asm/mach-types.h>

#define MP2520F_MEM_SIZE        (32*1024*1024)  
#define MEM_SIZE                MP2520F_MEM_SIZE

/* if CONFIG_BLK_DEV_RAM_SIZE not defined */
#define BLK_DEV_RAM_SIZE        (16*1024)

#ifdef CONFIG_MMSP2_SHADOW_ENABLE
	#define PA_SDRAM_BASE           0x00000000
	#define RAMDISK_DN_ADDR         0x00800000
	#define PA_ARM940_BASE          0x03000000
	#define PA_BUFFER_BASE          0x02000000
#else
	#define PA_SDRAM_BASE           0x80000000
	#define RAMDISK_DN_ADDR         0x80800000
	#define PA_ARM940_BASE          0x83000000
	#define PA_BUFFER_BASE          0x82000000
#endif

#define ZIP_RAMDISK_SIZE        (8*1024*1024)
#define VA_BUFFER_BASE          0xf7000000
#define BUFFER_MEM_SIZE         (16*1024*1024) 

#define VA_ARM940_BASE	        0xf4000000      // 0x03000000
#define ARM940_MEM_SIZE	       (16*1024*1024)  	// 16M

#define HEX_1M    0x00100000
#define HEX_512K  0x00080000
#define HEX_256K  0x00040000
	
#define VA_DUALCPU_BASE	        VA_ARM940_BASE 	// 0xf4000000     
#define PA_DUALCPU_BASE         PA_ARM940_BASE 	// 0x03000000
#define DUALCPU_MEM_SIZE	    (HEX_1M)  		// 1M

#define VA_FB0_BASE    (VA_ARM940_BASE+DUALCPU_MEM_SIZE)  	// 0xf400_0000+1M 2.5M(0x0028_0000)
#define PA_FB0_BASE    (PA_ARM940_BASE+DUALCPU_MEM_SIZE)  	// 0x0300_0000+1M 2.5M(0x0028_0000)
#define FB0_MEM_SIZE   (HEX_1M+HEX_1M+HEX_512K) 			// 2.5M

#define VA_FB1_BASE    (VA_FB0_BASE+FB0_MEM_SIZE)  // 0xf400_0000+1M+2.5M 2.5M(0x0028_0000)
#define PA_FB1_BASE    (PA_FB0_BASE+FB0_MEM_SIZE)  // 0x0300_0000+1M+2.5M 2.5M(0x0028_0000)
#define FB1_MEM_SIZE   (HEX_1M+HEX_1M+HEX_512K)    // 2.5M

#define VA_SOUND_DMA_BASE  (VA_FB1_BASE+FB1_MEM_SIZE) // 0xf400_0000+1M+2.5M+2.5M 0.5M(0x0008_0000)
#define PA_SOUND_DMA_BASE  (PA_FB1_BASE+FB1_MEM_SIZE) // 0x0300_0000+1M+2.5M+2.5M 0.5M(0x0008_0000)
#define SOUND_DMA_BUF_SIZE (HEX_512K)                 // 0.5M

#define VA_CAM_BASE  (VA_SOUND_DMA_BASE+SOUND_DMA_BUF_SIZE+HEX_512K)  // 0xf400_0000+1M+2.5M+2.5M+0.5M (0xf4700000)
#define PA_CAM_BASE  (PA_SOUND_DMA_BASE+SOUND_DMA_BUF_SIZE+HEX_512K)  // 0x0300_0000+1M+2.5M+2.5M+0.5M (0x03700000)
//#define CAM_MEM_SIZE (HEX_1M+HEX_1M+HEX_512K)    // 2.5M
#define CAM_MEM_SIZE (HEX_1M+HEX_1M+HEX_1M)    // 3M hhsong cam_addr used by scaler double-buffering..
	



#define PCIO_BASE		0

/*
 * MMSP2 internal I/O mappings
 *
 * We have the following mapping:
 *
 * 	phys		virt
 * 	c0000000	f0000000
 *
 * 	1c000000    fc000000
 * 	or
 * 	9c000000    fc000000
 *
 */
#define VIO_BASE	0xf0000000 /* virtual start of IO space */
#define PIO_BASE	0xc0000000 /* physical start of IO space */

#define VFIO_BASE   0xf6000000 /* virtual start of FIO space */
#define PFIO_BASE   0xe0000000 /* physical start of FIO space */

#define io_p2v(x)	((x) | 0x30000000)
#define io_v2p(x)	((x) & ~0x30000000)


#define NF_VIO_BASE		0xfc000000 /* virtual start of NAND flash IO space */
#define IDE_VIO_BASE	0xfd000000 /* virtual start of IDE IO space */
#define CF0_VIO_BASE	0xf1000000 /* virtual start of PCMCIA0 IO space */
#define CF0_VATTR_BASE	0xf5000000 /* virtual start of PCMCIA0 ATTR space */
#define CF0_VMEM_BASE	0xf9000000 /* virtual start of PCMCIA0 MEM space */
#define CF1_VIO_BASE	0xf2000000 /* virtual start of PCMCIA1 IO space */
#define CF1_VATTR_BASE	0xf6000000 /* virtual start of PCMCIA1 ATTR space */
#define CF1_VMEM_BASE	0xfa000000 /* virtual start of PCMCIA1 MEM space */
#define PERI_VREG_BASE  0xfe000000 /* virtual start of PCMCIA1 MEM space */
#define INTR_VREG_BASE  0xfe100000 /* virtual start of PCMCIA1 MEM space */

#ifdef CONFIG_MMSP2_SHADOW_ENABLE
#define NF_PIO_BASE		0x9c000000 /* physical start of NAND flash IO space */
#define IDE_PIO_BASE	0xbc000000 /* physical start of IDE IO space */
#define CF0_PIO_BASE	0xa0000000
#define CF0_PATTR_BASE	0xa4000000
#define CF0_PMEM_BASE	0xa8000000
#define CF1_PIO_BASE	0xb0000000
#define CF1_PATTR_BASE	0xb4000000
#define CF1_PMEM_BASE	0xb8000000
#define PERI_PREG_BASE  0x89400000
#define INTR_PREG_BASE  0x89800000

#define nf_p2v(x)	((x) | 0x60000000)
#define nf_v2p(x)	((x) & ~0x60000000)
#define ide_p2v(x)	((x) | 0x41000000)
#define ide_v2p(x)	((x) & ~0x41000000)
#define cf0_p2v(x)	((x) | 0x51000000)
#define cf0_v2p(x)	((x) & ~0x51000000)
#define cf1_p2v(x)	((x) | 0x42000000)
#define cf1_v2p(x)	((x) & ~0x42000000)
#else
#define NF_PIO_BASE		0x1c000000 /* physical start of NAND flash IO space */
#define IDE_PIO_BASE	0x3c000000 /* physical start of IDE IO space */
#define CF0_PIO_BASE	0x20000000
#define CF0_PATTR_BASE	0x24000000
#define CF0_PMEM_BASE	0x28000000
#define CF1_PIO_BASE	0x30000000
#define CF1_PATTR_BASE	0x34000000
#define CF1_PMEM_BASE	0x38000000
#define nf_p2v(x)	((x) | 0xe0000000)
#define nf_v2p(x)	((x) & ~0xe0000000)
#define ide_p2v(x)	((x) | 0xc1000000)
#define ide_v2p(x)	((x) & ~0xc1000000)
#define cf0_p2v(x)	((x) | 0xd1000000)
#define cf0_v2p(x)	((x) & ~0xd1000000)
#define cf1_p2v(x)	((x) | 0xc2000000)
#define cf1_v2p(x)	((x) & ~0xc2000000)
#endif /* CONFIG_MMSP2_SHADOW_ENABLE */


#ifndef __ASSEMBLY__

#if 0
#define __REG(x)	(*((volatile u32 *)io_p2v(x)))
#else
/*
 * This __REG() version gives the same results as the one above,  except
 * that we are fooling gcc somehow so it generates far better and smaller
 * assembly code for access to contigous registers.  It's a shame that gcc
 * doesn't guess this by itself
 */
#include <asm/types.h>
typedef struct { volatile u32 offset[4096]; } __regbase;
#define __REGP(x)	((__regbase *)((x)&~4095))->offset[((x)&4095)>>2]
#define __REG(x)	__REGP(io_p2v(x))
#endif

#define __REGL(x)	(*(volatile u32 *)io_p2v(x))
#define __REGW(x)	(*(volatile u16 *)io_p2v(x))
#define __REGB(x)	(*(volatile u8 *)io_p2v(x))	
#define __PREG(x)	(io_v2p((u32)&(x)))

#define __NF_REGL(x)	(*(volatile u32 *)nf_p2v(x))
#define __NF_REGW(x)	(*(volatile u16 *)nf_p2v(x))
#define __NF_REGB(x)	(*(volatile u8 *)nf_p2v(x))
#define __NF_PREG(x)	(nf_v2p((u32)&(x)))

#define __IDE_REGW(x)	(*(volatile u16 *)ide_p2v(x))
#define __IDE_REGB(x)	(*(volatile u8 *)ide_p2v(x))
#define __IDE_PREG(x)	(ide_v2p((u32)&(x)))

#define __CF0_REGW(x)	(*(volatile u16 *)cf0_p2v(x))
#define __CF0_REGB(x)	(*(volatile u8 *)cf0_p2v(x))
#define __CF0_PREG(x)	(cf0_v2p((u32)&(x)))

#define __CF1_REGW(x)	(*(volatile u16 *)cf1_p2v(x))
#define __CF1_REGB(x)	(*(volatile u8 *)cf1_p2v(x))
#define __CF1_PREG(x)	(cf1_v2p((u32)&(x)))

#else

#define __REG(x)	io_p2v(x)
#define __REGL(x)	io_p2v(x)
#define __REGW(x)	io_p2v(x)
#define __REGB(x)	io_p2v(x)
#define __PREG(x)	io_v2p(x)

#define __NF_REGW(x)	nf_p2v(x)
#define __NF_REGB(x)	nf_p2v(x)
#define __NF_PREG(x)	nf_v2p(x)

#define __IDE_REGW(x)	ide_p2v(x)
#define __IDE_REGB(x)	ide_p2v(x)
#define __IDE_PREG(x)	ide_v2p(x)

#define __CF0_REGW(x)	cf0_p2v(x)
#define __CF0_REGB(x)	cf0_p2v(x)
#define __CF0_PREG(x)	cf0_v2p(x)

#define __CF1_REGW(x)	cf1_p2v(x)
#define __CF1_REGB(x)	cf1_p2v(x)
#define __CF1_PREG(x)	cf1_v2p(x)

#endif /* __ASSEMBLY__ */

#include "mmsp2-regs.h"

#ifndef __ASSEMBLY__

#define GPIO_A0		(0x00000000)
#define GPIO_A1		(0x00000001)
#define GPIO_A2		(0x00000002)
#define GPIO_A3		(0x00000003)
#define GPIO_A4		(0x00000004)
#define GPIO_A5		(0x00000005)
#define GPIO_A6		(0x00000006)
#define GPIO_A7		(0x00000007)
#define GPIO_A8		(0x00000008)
#define GPIO_A9		(0x00000009)
#define GPIO_A10	(0x0000000a)
#define GPIO_A11	(0x0000000b)
#define GPIO_A12	(0x0000000c)
#define GPIO_A13	(0x0000000d)
#define GPIO_A14	(0x0000000e)
#define GPIO_A15	(0x0000000f)
#define GPIO_B0		(0x00000010)
#define GPIO_B1		(0x00000011)
#define GPIO_B2		(0x00000012)
#define GPIO_B3		(0x00000013)
#define GPIO_B4		(0x00000014)
#define GPIO_B5		(0x00000015)
#define GPIO_B6		(0x00000016)
#define GPIO_B7		(0x00000017)
#define GPIO_B8		(0x00000018)
#define GPIO_B9		(0x00000019)
#define GPIO_B10	(0x0000001a)
#define GPIO_B11	(0x0000001b)
#define GPIO_B12	(0x0000001c)
#define GPIO_B13	(0x0000001d)
#define GPIO_B14	(0x0000001e)
#define GPIO_B15	(0x0000001f)
#define GPIO_C0		(0x00000020)
#define GPIO_C1		(0x00000021)
#define GPIO_C2		(0x00000022)
#define GPIO_C3		(0x00000023)
#define GPIO_C4		(0x00000024)
#define GPIO_C5		(0x00000025)
#define GPIO_C6		(0x00000026)
#define GPIO_C7		(0x00000027)
#define GPIO_C8		(0x00000028)
#define GPIO_C9		(0x00000029)
#define GPIO_C10	(0x0000002a)
#define GPIO_C11	(0x0000002b)
#define GPIO_C12	(0x0000002c)
#define GPIO_C13	(0x0000002d)
#define GPIO_C14	(0x0000002e)
#define GPIO_C15	(0x0000002f)
#define GPIO_D0		(0x00000030)
#define GPIO_D1		(0x00000031)
#define GPIO_D2		(0x00000032)
#define GPIO_D3		(0x00000033)
#define GPIO_D4		(0x00000034)
#define GPIO_D5		(0x00000035)
#define GPIO_D6		(0x00000036)
#define GPIO_D7		(0x00000037)
#define GPIO_D8		(0x00000038)
#define GPIO_D9		(0x00000039)
#define GPIO_D10	(0x0000003a)
#define GPIO_D11	(0x0000003b)
#define GPIO_D12	(0x0000003c)
#define GPIO_D13	(0x0000003d)
#define GPIO_D14	(0x0000003e)
#define GPIO_D15	(0x0000003f)
#define GPIO_E0		(0x00000040)
#define GPIO_E1		(0x00000041)
#define GPIO_E2		(0x00000042)
#define GPIO_E3		(0x00000043)
#define GPIO_E4		(0x00000044)
#define GPIO_E5		(0x00000045)
#define GPIO_E6		(0x00000046)
#define GPIO_E7		(0x00000047)
#define GPIO_E8		(0x00000048)
#define GPIO_E9		(0x00000049)
#define GPIO_E10	(0x0000004a)
#define GPIO_E11	(0x0000004b)
#define GPIO_E12	(0x0000004c)
#define GPIO_E13	(0x0000004d)
#define GPIO_E14	(0x0000004e)
#define GPIO_E15	(0x0000004f)
#define GPIO_F0		(0x00000050)
#define GPIO_F1		(0x00000051)
#define GPIO_F2		(0x00000052)
#define GPIO_F3		(0x00000053)
#define GPIO_F4		(0x00000054)
#define GPIO_F5		(0x00000055)
#define GPIO_F6		(0x00000056)
#define GPIO_F7		(0x00000057)
#define GPIO_F8		(0x00000058)
#define GPIO_F9		(0x00000059)
#define GPIO_F10	(0x0000005a)
#define GPIO_F11	(0x0000005b)
#define GPIO_F12	(0x0000005c)
#define GPIO_F13	(0x0000005d)
#define GPIO_F14	(0x0000005e)
#define GPIO_F15	(0x0000005f)
#define GPIO_G0		(0x00000060)
#define GPIO_G1		(0x00000061)
#define GPIO_G2		(0x00000062)
#define GPIO_G3		(0x00000063)
#define GPIO_G4		(0x00000064)
#define GPIO_G5		(0x00000065)
#define GPIO_G6		(0x00000066)
#define GPIO_G7		(0x00000067)
#define GPIO_G8		(0x00000068)
#define GPIO_G9		(0x00000069)
#define GPIO_G10	(0x0000006a)
#define GPIO_G11	(0x0000006b)
#define GPIO_G12	(0x0000006c)
#define GPIO_G13	(0x0000006d)
#define GPIO_G14	(0x0000006e)
#define GPIO_G15	(0x0000006f)
#define GPIO_H0		(0x00000070)
#define GPIO_H1		(0x00000071)
#define GPIO_H2		(0x00000072)
#define GPIO_H3		(0x00000073)
#define GPIO_H4		(0x00000074)
#define GPIO_H5		(0x00000075)
#define GPIO_H6		(0x00000076)
#define GPIO_H7		(0x00000077)
#define GPIO_H8		(0x00000078)
#define GPIO_H9		(0x00000079)
#define GPIO_H10	(0x0000007a)
#define GPIO_H11	(0x0000007b)
#define GPIO_H12	(0x0000007c)
#define GPIO_H13	(0x0000007d)
#define GPIO_H14	(0x0000007e)
#define GPIO_H15	(0x0000007f)
#define GPIO_I0		(0x00000080)
#define GPIO_I1		(0x00000081)
#define GPIO_I2		(0x00000082)
#define GPIO_I3		(0x00000083)
#define GPIO_I4		(0x00000084)
#define GPIO_I5		(0x00000085)
#define GPIO_I6		(0x00000086)
#define GPIO_I7		(0x00000087)
#define GPIO_I8		(0x00000088)
#define GPIO_I9		(0x00000089)
#define GPIO_I10	(0x0000008a)
#define GPIO_I11	(0x0000008b)
#define GPIO_I12	(0x0000008c)
#define GPIO_I13	(0x0000008d)
#define GPIO_I14	(0x0000008e)
#define GPIO_I15	(0x0000008f)
#define GPIO_J0		(0x00000090)
#define GPIO_J1		(0x00000091)
#define GPIO_J2		(0x00000092)
#define GPIO_J3		(0x00000093)
#define GPIO_J4		(0x00000094)
#define GPIO_J5		(0x00000095)
#define GPIO_J6		(0x00000096)
#define GPIO_J7		(0x00000097)
#define GPIO_J8		(0x00000098)
#define GPIO_J9		(0x00000099)
#define GPIO_J10	(0x0000009a)
#define GPIO_J11	(0x0000009b)
#define GPIO_J12	(0x0000009c)
#define GPIO_J13	(0x0000009d)
#define GPIO_J14	(0x0000009e)
#define GPIO_J15	(0x0000009f)
#define GPIO_K0		(0x000000a0)
#define GPIO_K1		(0x000000a1)
#define GPIO_K2		(0x000000a2)
#define GPIO_K3		(0x000000a3)
#define GPIO_K4		(0x000000a4)
#define GPIO_K5		(0x000000a5)
#define GPIO_K6		(0x000000a6)
#define GPIO_K7		(0x000000a7)
#define GPIO_K8		(0x000000a8)
#define GPIO_K9		(0x000000a9)
#define GPIO_K10	(0x000000aa)
#define GPIO_K11	(0x000000ab)
#define GPIO_K12	(0x000000ac)
#define GPIO_K13	(0x000000ad)
#define GPIO_K14	(0x000000ae)
#define GPIO_K15	(0x000000af)
#define GPIO_L0		(0x000000b0)
#define GPIO_L1		(0x000000b1)
#define GPIO_L2		(0x000000b2)
#define GPIO_L3		(0x000000b3)
#define GPIO_L4		(0x000000b4)
#define GPIO_L5		(0x000000b5)
#define GPIO_L6		(0x000000b6)
#define GPIO_L7		(0x000000b7)
#define GPIO_L8		(0x000000b8)
#define GPIO_L9		(0x000000b9)
#define GPIO_L10	(0x000000ba)
#define GPIO_L11	(0x000000bb)
#define GPIO_L12	(0x000000bc)
#define GPIO_L13	(0x000000bd)
#define GPIO_L14	(0x000000be)
#define GPIO_L15	(0x000000bf)
#define GPIO_M0		(0x000000c0)
#define GPIO_M1		(0x000000c1)
#define GPIO_M2		(0x000000c2)
#define GPIO_M3		(0x000000c3)
#define GPIO_M4		(0x000000c4)
#define GPIO_M5		(0x000000c5)
#define GPIO_M6		(0x000000c6)
#define GPIO_M7		(0x000000c7)
#define GPIO_M8		(0x000000c8)
#define GPIO_M9		(0x000000c9)
#define GPIO_M10	(0x000000ca)
#define GPIO_M11	(0x000000cb)
#define GPIO_M12	(0x000000cc)
#define GPIO_M13	(0x000000cd)
#define GPIO_M14	(0x000000ce)
#define GPIO_M15	(0x000000cf)
#define GPIO_N0		(0x000000d0)
#define GPIO_N1		(0x000000d1)
#define GPIO_N2		(0x000000d2)
#define GPIO_N3		(0x000000d3)
#define GPIO_N4		(0x000000d4)
#define GPIO_N5		(0x000000d5)
#define GPIO_N6		(0x000000d6)
#define GPIO_N7		(0x000000d7)
#define GPIO_N8		(0x000000d8)
#define GPIO_N9		(0x000000d9)
#define GPIO_N10	(0x000000da)
#define GPIO_N11	(0x000000db)
#define GPIO_N12	(0x000000dc)
#define GPIO_N13	(0x000000dd)
#define GPIO_N14	(0x000000de)
#define GPIO_N15	(0x000000df)
#define GPIO_O0		(0x000000e0)
#define GPIO_O1		(0x000000e1)
#define GPIO_O2		(0x000000e2)
#define GPIO_O3		(0x000000e3)
#define GPIO_O4		(0x000000e4)
#define GPIO_O5		(0x000000e5)
#define GPIO_O6		(0x000000e6)
#define GPIO_O7		(0x000000e7)
#define GPIO_O8		(0x000000e8)
#define GPIO_O9		(0x000000e9)
#define GPIO_O10	(0x000000ea)
#define GPIO_O11	(0x000000eb)
#define GPIO_O12	(0x000000ec)
#define GPIO_O13	(0x000000ed)
#define GPIO_O14	(0x000000ee)
#define GPIO_O15	(0x000000ef)

#define GRP_OFS(x)	((x) >> 16)		
#define GRP_NUM(x)	((x) >> 4)		

#define GPIOMD_IN		(0)
#define GPIOMD_OUT		(1)
#define GPIOMD_ALT1		(2)
#define GPIOMD_ALT2		(3)
#define GPIOPU_DIS		(0)
#define GPIOPU_EN		(1)
#define GPIOPU_NOSET	(2)

#define EINT_RISING_EDGE	(0)
#define EINT_FALLING_EDGE	(1)
#define EINT_HIGH_LEVEL		(2)
#define EINT_LOW_LEVEL		(3)

#define PLL_MDIV	Fld(8, 8)	
#define PLL_PDIV	Fld(6, 2)
#define PLL_SDIV	Fld(2, 0)
#define PLL_DDIV	Fld(3, 6)

#define GET_MDIV(x)	FExtr(x, PLL_MDIV)
#define GET_PDIV(x)	FExtr(x, PLL_PDIV)
#define GET_SDIV(x)	FExtr(x, PLL_SDIV)
#define GET_DDIV(x) FExtr(x, PLL_DDIV)

extern unsigned long mmsp2_get_fclk(void);
extern unsigned long mmsp2_get_uclk(void);
extern unsigned long mmsp2_get_aclk(void);
extern unsigned long mmsp2_get_pclk(void);

extern void set_gpio_ctrl(unsigned int, unsigned int, unsigned int);
extern void set_external_irq(int irq, unsigned int edge, unsigned int pullup);
extern unsigned int inline read_gpio_bit(unsigned int);
extern void inline write_gpio_bit(unsigned int, unsigned int);

#endif /* __ASSEMBLY__ */

#if defined(CONFIG_MACH_MMSP2_MDK)
#include "mdk.h"
#else
#error not defined board
#endif

#endif /* __ASM_ARCH_HARDWARE_H */
