/* linux/include/asm-arm/arch-s3c2410/S3C2410-irq.h
 *
 * Copyright (c) 2003 Simtec Electronics <linux@simtec.co.uk>
 *                    http://www.simtec.co.uk/products/SWLINUX/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Changelog:
 *    19-06-2003     BJD     Created file
 */

#ifndef ASMARM_ARCH_S3C2410_IRQ_H
#define ASMARM_ARCH_S3C2410_IRQ_H

/* interrupt controller */

#define S3C2410_IRQREG(x)   ((x) + S3C2410_VA_IRQ)
#define S3C2410_EINTREG(x)  ((x) + S3C2410_VA_GPIO)

#define S3C2410_SRCPND         S3C2410_IRQREG(0x000)
#define S3C2410_INTMOD         S3C2410_IRQREG(0x004)
#define S3C2410_INTMSK         S3C2410_IRQREG(0x008)
#define S3C2410_PRIORITY       S3C2410_IRQREG(0x00C)
#define S3C2410_INTPND         S3C2410_IRQREG(0x010)
#define S3C2410_INTOFFSET      S3C2410_IRQREG(0x014)
#define S3C2410_SUBSRCPND      S3C2410_IRQREG(0x018)
#define S3C2410_INTSUBMSK      S3C2410_IRQREG(0x01C)

#define S3C2410_EINTMASK       S3C2410_EINTREG(0x0A4)
#define S3C2410_EINTPEND       S3C2410_EINTREG(0X0A8)

#endif /* ASMARM_ARCH_S3C2410_IRQ_H */
