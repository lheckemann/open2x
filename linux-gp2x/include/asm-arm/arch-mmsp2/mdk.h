#ifndef __MMSP2_MDK_H
#define __MMSP2_MDK_H

#ifndef __ASM_ARCH_HARDWARE_H
#error "include <asm/hardware.h> instead"
#endif
#include <linux/config.h>

/* GPIO out, USB host #2 power enable, negative */

/* GPIO alt. PWM#1, LCD backlight control */
#define GPIO_LCD_BR_CON		GPIO_L11

/* CS8900 */
#ifdef CONFIG_MACH_MMSP2_DTK3
	#define IRQ_ETHER		IRQ_GPIO_D(12)
#elif defined(CONFIG_MACH_MMSP2_DTK4)
	#define IRQ_ETHER		IRQ_GPIO_N(4)
#endif

/* IDE */
#ifdef CONFIG_MACH_MMSP2_DTK3
	#define IRQ_HARD		IRQ_GPIO_D(13)
	#define GPIO_IDE_nSWRST	GPIO_M4  /* GPIO out, negative */
#elif defined(CONFIG_MACH_MMSP2_DTK4)
	#define IRQ_HARD		IRQ_GPIO_N(3)
	#define GPIO_IDE_nSWRST		GPIO_N7  /* GPIO out, negative */
#endif

/* GPIO I2C */
#ifdef CONFIG_MACH_MMSP2_DTK3
	#define GPIO_SCL		GPIO_J13
	#define GPIO_SDA		GPIO_J14
#elif defined(CONFIG_MACH_MMSP2_DTK4)
	#define GPIO_SCL		GPIO_B1
	#define GPIO_SDA		GPIO_B0	
#endif

/* CMOS Image Sensor */
#define CIS_CLKO			GPIO_H1
#define CIS_VSYNC			GPIO_H2
#define CIS_HSYNC			GPIO_H3
#define CIS_CLKI			GPIO_H4
#ifdef CONFIG_MACH_MMSP2_DTK3
	#define CIS_PWDN		GPIO_C10
	#define CIS_RESET		GPIO_C11
#elif defined(CONFIG_MACH_MMSP2_DTK4)
	#define CIS_PWDN		GPIO_O2
	#define CIS_RESET		GPIO_O3
#endif

/* CF */
#ifdef CONFIG_MACH_MMSP2_DTK3
	#define IRQ_CF          IRQ_GPIO_L(12)
#elif defined(CONFIG_MACH_MMSP2_DTK4)
	#define nCF1_CD			GPIO_D5
	#define IRQ_CF          IRQ_GPIO_D(8)
#endif

/* board specific device IO address */
#define MDK_ETH_VIO_BASE	0xf3000000UL /* CS8900 */

#ifdef CONFIG_MMSP2_SHADOW_ENABLE
   	#define MDK_ETH_PIO_BASE 0x84000000 /* static bank #1 */
#else
	#define MDK_ETH_PIO_BASE 0x04000000 /* static bank #1 */
#endif

#endif /* __MMSP2_MDK_H */
