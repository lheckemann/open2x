/*
 *  Copyright (c) Daniel Silsby
 *
 *  Based on the work of:
 *	Vojtech Pavlik and Andreas Pokorny
 */

/* This was adapted from gp2xjoy.c by Senor Quack for
 * the Open2X firmware.  We needed a joystick device
 * that, while valid in every way, reports no events. */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so by e-mail 
 * mail your message to <andreas.pokorny@gmail.com>
 */


#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/kernel.h>

/* This code is restricted to mmsp2 architectures:*/
#include <asm/hardware.h>
#include <asm/arch/mmsp20.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mmsp20_type.h>

MODULE_AUTHOR("Dan Silsby and Andreas Pokorny <andreas.pokorny@gmail.com>");
MODULE_LICENSE("GPL");

static unsigned int open_count = 0;

//void dummy_gamepad_timer_fun( unsigned long );

//static struct timer_list dummy_gamepad_timer;
static struct input_dev dummy_gamepad_dev = {0};

//void dummy_gamepad_timer_fun( unsigned long data ) 
//{
//  MMSP20_GPIO * base;
//	unsigned short buttons = 0, direction = 0, volumeStick = 0;
//  int i = 0;
//
//  base = MMSP20_GetBase_GPIO();
//
//  buttons = ~((base->GPIOPINLVL[GRP_NUM(GP2X_GROUP_1)]>>(GP2X_GROUP_1&0xf)) &0xFF);
//	volumeStick = ~(base->GPIOPINLVL[GRP_NUM(GP2X_VOLUME_DOWN)]);
//  direction = ~base->GPIOPINLVL[GRP_NUM(GP2X_GROUP_2)];

//  input_report_key(&gp2x_gamepad_dev,BTN_START,   buttons & 1);
//  input_report_key(&gp2x_gamepad_dev,BTN_SELECT, (buttons>>1) & 1);
//  input_report_key(&gp2x_gamepad_dev,BTN_TL,     (buttons>>2) & 1);
//  input_report_key(&gp2x_gamepad_dev,BTN_TR,     (buttons>>3) & 1);
//  input_report_key(&gp2x_gamepad_dev,BTN_A,      (buttons>>4) & 1);
//  input_report_key(&gp2x_gamepad_dev,BTN_B,      (buttons>>5) & 1);
//  input_report_key(&gp2x_gamepad_dev,BTN_X,      (buttons>>6) & 1);
//  input_report_key(&gp2x_gamepad_dev,BTN_Y,      (buttons>>7) & 1);
//  input_report_key(&gp2x_gamepad_dev, BTN_TL2,    (volumeStick >> (GP2X_VOLUME_DOWN &0xf))&1); /* volume down */
//  input_report_key(&gp2x_gamepad_dev, BTN_TR2,    (volumeStick >> (GP2X_VOLUME_UP   &0xf))&1); /* volume up */
//  input_report_key(&gp2x_gamepad_dev, BTN_TRIGGER,(volumeStick >> (GP2X_STICK_BUTTON&0xf))&1);
//
//
//  switch( direction&255 )
//  {
//    case 0: 
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,0);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,0);
//      break;
//    case 1: /* North*/
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,0);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,2);
//      break;
//    case 3: /* NorthNorthWest*/
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,-1);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,2);
//      break;
//    case 2: /* NorthWest*/
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,-2);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,2);
//      break;
//    case 5: /* NorthWestWest*/
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,-2);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,1);
//      break;
//    case 4: /* West*/
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,-2);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,0);
//      break;
//    case 12:/* SouthWestWest*/
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,-2);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,-1);
//      break;
//    case 8: /* SouthWest  */
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,-2);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,-2);
//      break;
//    case 24:/* SouthSouthWest */
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,-1);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,-2);
//      break;
//    case 16: /* South */
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,0);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,-2);
//      break;
//    case 48: /* SouthSouthEast */
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,1);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,-2);
//      break;
//    case 32: /* SouthEast */ 
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,2);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,-2);
//      break;
//    case 96: /* SouthEastEast   */
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,2);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,-1);
//      break;
//    case 64: /* East */
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,2);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,0);
//      break;
//    case 192: /* NorthEastEast   */
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,2);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,1);
//      break;
//    case 128: /* NorthEast       */
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,2);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,2);
//      break;
//    case 129: /* NorthNorthEast */
//      input_report_abs(&gp2x_gamepad_dev, ABS_X,1);
//      input_report_abs(&gp2x_gamepad_dev, ABS_Y,2);
//      break;
//  }
//
//  mod_timer( & gp2x_gamepad_timer, jiffies + 2 );
//}

int open_dummy_gamepad_dev(struct input_dev *dev)
{
  if( 0 == open_count )
  {
//    init_timer( & dummy_gamepad_timer );
//    gp2x_gamepad_timer.expires = jiffies + HZ;
//    gp2x_gamepad_timer.data = 0; 
//    gp2x_gamepad_timer.function = gp2x_gamepad_timer_fun;
//    add_timer( & gp2x_gamepad_timer );
  }

  ++open_count;
  return 0;
}


void close_dummy_gamepad_dev(struct input_dev *dev)
{
  if( open_count == 1 )
  {
//    del_timer( & dummy_gamepad_timer );
    --open_count;
  }
  else if ( open_count != 0 )
  {
    --open_count;
  }
}


int __init dummy_gamepad_init(void)
{
//  int i;
//
//  for( i=0; i<8; i++) 
//  {
//    set_gpio_ctrl( GPIO_C8 +i , GPIOMD_IN, GPIOPU_EN );
//    set_gpio_ctrl( GPIO_M0 +i , GPIOMD_IN, GPIOPU_EN );
//  }
//
//  set_gpio_ctrl(GP2X_VOLUME_UP,GPIOMD_IN,GPIOPU_EN);
//  set_gpio_ctrl(GP2X_VOLUME_DOWN,GPIOMD_IN,GPIOPU_EN);
//  set_gpio_ctrl(GP2X_STICK_BUTTON,GPIOMD_IN,GPIOPU_EN);
//
//
//  gp2x_gamepad_dev.evbit[0] = BIT(EV_ABS)|BIT(EV_KEY);
//  gp2x_gamepad_dev.keybit[LONG(KEY_VOLUMEDOWN)] = BIT(KEY_VOLUMEDOWN) | BIT(KEY_VOLUMEUP);
//  gp2x_gamepad_dev.keybit[LONG(BTN_JOYSTICK)] |= BIT(BTN_TRIGGER);
//  gp2x_gamepad_dev.keybit[LONG(BTN_GAMEPAD)] |= BIT(BTN_A) 
//    | BIT(BTN_B) 
//    | BIT(BTN_X) 
//    | BIT(BTN_Y) 
//    | BIT(BTN_TL) 
//    | BIT(BTN_TR) 
//    | BIT(BTN_TL2) 
//    | BIT(BTN_TR2) 
//    | BIT(BTN_SELECT) 
//    | BIT(BTN_START) 
//    ;
//	gp2x_gamepad_dev.absmax[ABS_X]=2;
//	gp2x_gamepad_dev.absmax[ABS_Y]=2;
//	gp2x_gamepad_dev.absmin[ABS_X]=-2;
//	gp2x_gamepad_dev.absmin[ABS_Y]=-2;
//	gp2x_gamepad_dev.absfuzz[ABS_X]=0;
//	gp2x_gamepad_dev.absfuzz[ABS_Y]=0;
//	gp2x_gamepad_dev.absflat[ABS_X]=0;
//	gp2x_gamepad_dev.absflat[ABS_Y]=0;
//
//
//  gp2x_gamepad_dev.absbit[LONG(ABS_X)] |= BIT(ABS_X) | BIT(ABS_Y); 
//  gp2x_gamepad_dev.name = "GP2X-F100 gamepad";
//  gp2x_gamepad_dev.open = open_gp2x_gamepad_dev;
//  gp2x_gamepad_dev.close = close_gp2x_gamepad_dev;
//  input_register_device(&dummy_gamepad_dev);
//  return 0;

  dummy_gamepad_dev.evbit[0] = BIT(EV_ABS)|BIT(EV_KEY);
  dummy_gamepad_dev.keybit[LONG(KEY_VOLUMEDOWN)] = BIT(KEY_VOLUMEDOWN) | BIT(KEY_VOLUMEUP);
  dummy_gamepad_dev.keybit[LONG(BTN_JOYSTICK)] |= BIT(BTN_TRIGGER);
  dummy_gamepad_dev.keybit[LONG(BTN_GAMEPAD)] |= BIT(BTN_A) 
    | BIT(BTN_B) 
    | BIT(BTN_X) 
    | BIT(BTN_Y) 
    | BIT(BTN_TL) 
    | BIT(BTN_TR) 
    | BIT(BTN_TL2) 
    | BIT(BTN_TR2) 
    | BIT(BTN_SELECT) 
    | BIT(BTN_START) 
    ;
	dummy_gamepad_dev.absmax[ABS_X]=2;
	dummy_gamepad_dev.absmax[ABS_Y]=2;
	dummy_gamepad_dev.absmin[ABS_X]=-2;
	dummy_gamepad_dev.absmin[ABS_Y]=-2;
	dummy_gamepad_dev.absfuzz[ABS_X]=0;
	dummy_gamepad_dev.absfuzz[ABS_Y]=0;
	dummy_gamepad_dev.absflat[ABS_X]=0;
	dummy_gamepad_dev.absflat[ABS_Y]=0;


  dummy_gamepad_dev.absbit[LONG(ABS_X)] |= BIT(ABS_X) | BIT(ABS_Y); 
  dummy_gamepad_dev.name = "Open2X dummy gamepad";
  dummy_gamepad_dev.open = open_dummy_gamepad_dev;
  dummy_gamepad_dev.close = close_dummy_gamepad_dev;
  input_register_device(&dummy_gamepad_dev);
  return 0;
}

void __exit dummy_gamepad_exit(void)
{
	input_unregister_device(&dummy_gamepad_dev);
}


module_init(dummy_gamepad_init)
module_exit(dummy_gamepad_exit)

