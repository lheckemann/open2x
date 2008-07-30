/*
 * linux/arch/arm/mach-mmsp2/proto_gpio.c
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/config.h>
#include <linux/module.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/arch/mmsp20.h>
#include <asm/arch/proto_gpio.h>

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(x...) printk(x)
#else
#  define ds_printk(fmt, args...)
#endif

MMSP20_GPIO * pGPIO_REG;
U32 GPIOALT;

static unsigned char  sdbitSize;
static unsigned char usbFlag=0;
static unsigned char sdMode=2;
static unsigned int gp2xInfoVarLsb;
static unsigned int gp2xInfoVarMsb;
static unsigned char usbHconFlag=0;

void GPIO_Initialize(void) {
        pGPIO_REG = MMSP20_GetBase_GPIO();
}

void GPIO_SetGPIOEAliveState (U16 Out, U16 Dir, U16 PullUp)
{
	pGPIO_REG->GPIOEOUT_ALIVE = Out;
	pGPIO_REG->GPIOEOUTENB_ALIVE = Dir;
	pGPIO_REG->GPIOEPUENB_ALIVE = PullUp;
}

void GPIO_SetGPIOFAliveState (U16 Out, U16 Dir, U16 PullUp)
{
	pGPIO_REG->GPIOFOUT_ALIVE = Out;
	pGPIO_REG->GPIOFOUTENB_ALIVE = Dir;
	pGPIO_REG->GPIOFPUENB_ALIVE = PullUp;
}

void GPIO_SetGPIOGAliveState (U16 Out, U16 Dir, U16 PullUp)
{
	pGPIO_REG->GPIOGOUT_ALIVE = Out;
	pGPIO_REG->GPIOGOUTENB_ALIVE = Dir;
	pGPIO_REG->GPIOGPUENB_ALIVE = PullUp;
}

void GPIO_SetGPIOIAliveState (U16 Out, U16 Dir, U16 PullUp)
{
	pGPIO_REG->GPIOIOUT_ALIVE = Out;
	pGPIO_REG->GPIOIOUTENB_ALIVE = Dir;
	pGPIO_REG->GPIOIPUENB_ALIVE = PullUp;
}

void GPIO_SetGPIOJAliveState (U16 Out, U16 Dir, U16 PullUp)
{
	pGPIO_REG->GPIOJOUT_ALIVE = Out;
	pGPIO_REG->GPIOJOUTENB_ALIVE = Dir;
	pGPIO_REG->GPIOJPUENB_ALIVE = PullUp;
}

/* buggy CPU */
const unsigned int _pins_of_gpio[] = {
	[GRP_NUM(GPIO_A0)] = 16,
	[GRP_NUM(GPIO_B0)] = 16,
	[GRP_NUM(GPIO_C0)] = 16,
	[GRP_NUM(GPIO_D0)] = 14,
	[GRP_NUM(GPIO_E0)] = 16,
	[GRP_NUM(GPIO_F0)] = 10,
	[GRP_NUM(GPIO_G0)] = 16,
	[GRP_NUM(GPIO_H0)] = 7,
	[GRP_NUM(GPIO_I0)] = 16,
	[GRP_NUM(GPIO_J0)] = 16,
	[GRP_NUM(GPIO_K0)] = 8,
	[GRP_NUM(GPIO_L0)] = 15,
	[GRP_NUM(GPIO_M0)] = 9,
	[GRP_NUM(GPIO_N0)] = 8,
	[GRP_NUM(GPIO_O0)] = 6,
};


/****************************************************************
** _get_mmsp2_gpio_func : GPIO alt func event read
** <Parameters>
** addr_l : GPIOx Alternate Function Low Register
**			or GPIOx Event Type Low Register
** addr_h : GPIOx Alternate Function High Register
**			of GPIOx Event Type High Register
** pins : GPIOx pin
** bit : GPIOx[n] n (0~15)
** <Retuen>
** func or event
****************************************************************/
unsigned int _get_mmsp2_gpio_func(unsigned int addr_l, unsigned int addr_h, unsigned int pins, unsigned int bit)
{
	unsigned short value_l = readw(addr_l);
	unsigned short value_h = readw(addr_h);
	unsigned int *addr_t = &addr_l;
	unsigned short *value_t = &value_l;
	unsigned int bitofs_t = bit * 2;

	value_l |= (value_h << pins);
	value_h >>= (16-pins);

	if (bit > 7) {
		bitofs_t -= (8*2);
		addr_t = &addr_h;
		value_t = &value_h;
	}

	ds_printk("%s(): read from 0x%08x is 0x%04x\n",
					__FUNCTION__, *addr_t, *value_t);

	return ((*value_t >> bitofs_t) & 0x03);
}

/****************************************************************
** _set_mmsp2_gpio_func : GPIO alt func event write
** <Parameters>
** addr_l : GPIOx Alternate Function Low Register
**			or GPIOx Event Type Low Register
** addr_h : GPIOx Alternate Function High Register
**			of GPIOx Event Type High Register
** pins : GPIOx pin
** bit : GPIOx[n] n (0~15)
** func : 00-GPIO In, 01-GPIO Out, 10-Alt func 1, 11-Alt func 2
** <Retuen>
** None
****************************************************************/
void _set_mmsp2_gpio_func(unsigned int addr_l, unsigned int addr_h, unsigned int pins, unsigned int bit, unsigned int func)
{
	unsigned short value_l = readw(addr_l);
	unsigned short value_h = readw(addr_h);
	unsigned int *addr_t = &addr_l;
	unsigned short *value_t = &value_l;
	unsigned int bitofs_t = bit * 2;

	value_l |= (value_h << pins);
	value_h >>= (16-pins);

	if (bit > 7) {
		bitofs_t -= (8*2);
		addr_t = &addr_h;
		value_t = &value_h;
	}

	*value_t &= ~(0x3 << bitofs_t);
	*value_t |= ((func & 0x03) << bitofs_t);

	ds_printk("%s(): try to set 0x%08x as 0x%04x\n",
					__FUNCTION__, *addr_t, *value_t);

	writew(*value_t, *addr_t);

	// verify
	BUG_ON(func != _get_mmsp2_gpio_func(addr_l, addr_h, pins, bit));
}

/****************************************************************
** set_gpio_ctrl : GPIO function & pullup
** <Parameters>
** num : GPIO number (0~239)
** func : 00-GPIO In, 01-GPIO Out, 10-Alt func 1, 11-Alt func 2
** pu : 0-no pullup, 1-pullup
** <Retuen>
** None
****************************************************************/
void set_gpio_ctrl(unsigned int num, unsigned int func, unsigned int pu)
{
	unsigned long reg_offset = GRP_NUM(num);		//((0xf0 & num) >> 3);
    unsigned long addr_l, addr_h;
	unsigned long bit_ofs = (0x7 & num) * 2;    // 0,1,2,.. --> 0,2,4,6,...
	unsigned short temp;

// GPIO function
	ds_printk("%s(): gpio = %04x(GPIO %c[%d])\n",
		 	__FUNCTION__, num, GRP_NUM(num)+'A', num & 0xf);

	GPIO_Initialize();

    addr_l = &(pGPIO_REG->GPIOALTFNLOW[reg_offset]);  //io_p2v(0xc0001020 + reg_offset);
	addr_h = &(pGPIO_REG->GPIOALTFNHI[reg_offset]);   //io_p2v(0xc0001040 + reg_offset);

	_set_mmsp2_gpio_func(addr_l, addr_h,
			_pins_of_gpio[GRP_NUM(num)], num & 0xf, func);

// GPIO Pullup
	addr_l = &(pGPIO_REG->GPIOPUENB[reg_offset]);		//io_p2v(0xc00010c0 + reg_offset);
	bit_ofs = num & 0xf;
	temp = readw(addr_l);   // PUENB

	ds_printk("%s(): PU: read from 0x%08x is 0x%04x\n",
					__FUNCTION__, addr_l, temp);

	if (pu == GPIOPU_NOSET)
		return;

	temp &= ~(1 << bit_ofs);
	temp |= (pu << bit_ofs);

	ds_printk("%s(): PU: try to set 0x%08x as 0x%04x\n",
					__FUNCTION__, addr_l, temp);

	writew(temp, addr_l);

	ds_printk("%s(): PU: read from 0x%08x is 0x%04x\n",
					__FUNCTION__, addr_l, readw(addr_l));
}

/**********************************************************************
** set_external_irq : gpio
** <parameters>
** edge : 00-rising edge, 01-falling edge, 10-high level, 11-low level
** pullup : 0-no pullup, 1-pullup
** <return>
** None
**********************************************************************/
void set_external_irq(int irq, unsigned int edge, unsigned int pullup)
{
	unsigned int gpio = IRQ_TO_GPIO(irq);
	unsigned long reg_offset = GRP_NUM(gpio);	//((0xf0 & gpio) >> 3);
	unsigned long addr_l, addr_h;

	ds_printk("%s(): irq = %d(%04x), gpio = %04x(GPIO %c[%d])\n",
		 	__FUNCTION__, irq, irq, gpio, GRP_NUM(gpio)+'A', gpio & 0xf);

	GPIO_Initialize();

	/* as input */
	set_gpio_ctrl(gpio, GPIOMD_IN, pullup);

	addr_l = &(pGPIO_REG->GPIOEVTTYPLOW[reg_offset]);	//  io_p2v(0xc0001080 + reg_offset);
	addr_h = &(pGPIO_REG->GPIOEVTTYPHI[reg_offset]);	//io_p2v(0xc00010a0 + reg_offset);

	_set_mmsp2_gpio_func(addr_l, addr_h,
			_pins_of_gpio[GRP_NUM(gpio)], gpio & 0xf, edge);
}

unsigned int inline read_gpio_bit(unsigned int num)
{
//	unsigned long addr;
	unsigned short value;
	unsigned long bit_ofs = num & 0xf;

	GPIO_Initialize();

	value = pGPIO_REG->GPIOPINLVL[GRP_NUM(num)];	//io_p2v(0xc0001180 + (0x2 * GRP_NUM(num));

	return ((value & (1<<bit_ofs)) >> bit_ofs);

//	return ((readw(addr) & (1 << bit_ofs)) >> bit_ofs);
}

void inline write_gpio_bit(unsigned int num, unsigned int val)
{
	unsigned short value;
	unsigned long bit_ofs = num & 0xf;

	GPIO_Initialize();

	value = pGPIO_REG->GPIOOUT[GRP_NUM(num)] & ~(1<<bit_ofs);
	value |= (val << bit_ofs);
	pGPIO_REG->GPIOOUT[GRP_NUM(num)] = value;
}

void inline gpio_pad_select(unsigned short pad, unsigned short on_off)
{
	GPIO_Initialize();

	if(on_off == 1)
		pGPIO_REG->GPIOPADSEL |= pad;
	else
		pGPIO_REG->GPIOPADSEL &= ~pad;
}


void mmsp_set_sdsize(unsigned char sdbit)
{
	sdbitSize = sdbit;
}

unsigned char mmsp_get_sdsize(void)
{
	return sdbitSize;
}

void mmsp_set_usbCONN(unsigned char uflag)
{
	usbFlag=uflag;
}

unsigned char mmsp_get_usbCONN(void)
{
	return usbFlag;
}

void mmsp_set_SDCLK(unsigned char sdclkMode)
{
	sdMode=sdclkMode;
}

unsigned char mmsp_get_SDCLK(void)
{
	return sdMode;
}

void SetGp2xInfo(unsigned int info,int infoFlag)
{
	if(infoFlag) gp2xInfoVarLsb=info;
	else gp2xInfoVarMsb=info;
}

unsigned int GetGp2xInfo(int infoFlag)
{
	if(infoFlag) return gp2xInfoVarLsb;
	else return gp2xInfoVarMsb;
}

void SetUsbHcon(unsigned char flag)
{
	usbHconFlag	= flag;
}

unsigned char GetUshHcon(void)
{
	return usbHconFlag;
}

EXPORT_SYMBOL(SetUsbHcon);
EXPORT_SYMBOL(GetUshHcon);
EXPORT_SYMBOL(GetGp2xInfo);
EXPORT_SYMBOL(SetGp2xInfo);
EXPORT_SYMBOL(mmsp_set_SDCLK);
EXPORT_SYMBOL(mmsp_get_SDCLK);
EXPORT_SYMBOL(mmsp_set_usbCONN);
EXPORT_SYMBOL(mmsp_get_sdsize);
EXPORT_SYMBOL(set_gpio_ctrl);
EXPORT_SYMBOL(set_external_irq);
EXPORT_SYMBOL(read_gpio_bit);
EXPORT_SYMBOL(write_gpio_bit);
EXPORT_SYMBOL(gpio_pad_select);
