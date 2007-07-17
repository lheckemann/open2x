//[*]----------------------------------------------------------------------------------------------------[*]
#ifndef __GP2X_LCD_H_
#define __GP2X_LCD_H_
//[*]----------------------------------------------------------------------------------------------------[*]
//[*]----------------------------------------------------------------------------------------------------[*]
//[*]----------------------------------------------------------------------------------------------------[*]
#include "hardware.h"

#define le16_to_cpu __le16_to_cpu
#define __le16_to_cpu(x) ((__u16)(x))

#define cpu_to_le16 __cpu_to_le16
#define __cpu_to_le16(x) ((__u16)(x))

#define __mem_pci(a)		((unsigned long)(a))
#define __arch_getw(a)		(*(volatile unsigned short *)(a))
#define __arch_putw(v,a)	(*(volatile unsigned short *)(a) = (v))

#define __raw_writew(v,a)		__arch_putw(v,a)
#define __raw_readw(a)			__arch_getw(a)

#define readw(c) ({ unsigned int __v = le16_to_cpu(__raw_readw(__mem_pci(c))); __v; })

#define writew(v,c)		__raw_writew(cpu_to_le16(v),__mem_pci(c))
#define PAGE_SHIFT		12
#define PAGE_SIZE       (1UL << PAGE_SHIFT)

#define HEX_1M    0x00100000
#define PA_ARM940_BASE          0x03000000
#define DUALCPU_MEM_SIZE	    (HEX_1M)  		// 1M


extern const unsigned int _pins_of_gpio[];

void show_logo(void);

#endif
//[*]----------------------------------------------------------------------------------------------------[*]
