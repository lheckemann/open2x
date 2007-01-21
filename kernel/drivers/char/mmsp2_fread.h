#ifndef _MMSP2_FILEREAD_H_
#define _MMSP2_FILEREAD_H_

#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/arch/memory.h>

#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <asm/processor.h>

#include <linux/mm.h> // for kmalloc
#include <linux/slab.h> // for kmalloc

#define SEEK_SET	0
#define SEEK_CUR 	1
#define SEEK_END 	2
#define EOF			(-1)


// function define
struct file *klib_fopen(const char *filename, int flags, int mode);
void         klib_fclose(struct file *filp);
int          klib_fread(char *buf, int len, struct file *filp);
long         klib_fsize(struct file *filp);

long         klib_fsize(struct file *filp);

#endif
