#ifndef _ml_delay_h
#define _ml_delay_h
#include "headfile.h"

void SysTick_Init(void);
void SysTick_Wait(uint32_t delay);
void delay_ms(uint32_t delay);
void delay_us(uint32_t delay);

#endif
