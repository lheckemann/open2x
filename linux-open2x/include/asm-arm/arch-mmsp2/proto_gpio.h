#ifndef __GPIO_H__
#define __GPIO_H__

#include <asm/arch/wincetype.h>

void GPIO_Initialize ( void );
void GPIO_SetGPIOEAliveState (U16 Out, U16 Dir, U16 PullUp);
void GPIO_SetGPIOFAliveState (U16 Out, U16 Dir, U16 PullUp);
void GPIO_SetGPIOGAliveState (U16 Out, U16 Dir, U16 PullUp);
void GPIO_SetGPIOIAliveState (U16 Out, U16 Dir, U16 PullUp);
void GPIO_SetGPIOJAliveState (U16 Out, U16 Dir, U16 PullUp);

void set_gpio_ctrl(unsigned int num, unsigned int func, unsigned int pu);
unsigned int inline read_gpio_bit(unsigned int num);
void inline write_gpio_bit(unsigned int num, unsigned int val);

#endif // __MMSP2GPIO_H__
