#ifndef _ml_gpio_h
#define _ml_gpio_h
#include "headfile.h"

typedef enum
{
	OUT,
	IN_UP,
	IN_DOWN,
}GPIO_Mode_enum;

typedef enum
{
	PA0,
	PA1,
	PA28,
	PA29,
	PA30,
	PA31,
	PA2,
	PA3,
	PA4,
	PA5,
	PA6,
	PB0,
	PB1,
	PA7,
	PB2,
	PB3,
	PB4,
	PB5,
	PA8,
	PA9,
	PA10,
	PA11,
	PB6,
	PB7,
	PB8,
	PB9,
	PB10,
	PB11,
	PB12,
	PB13,
	PB14,
	PB15,
	PB16,
	PA12,
	PA13,
	PA14,
	PA15,
	PA16,
	PA17,
	PA18,
	PA19,
	PA20,
	PB17,
	PB18,
	PB19,
	PA21,
	PA22,
	PB20,
	PB21,
	PB22,
	PB23,
	PB24,
	PA23,
	PA24,
	PA25,
	PB25,
	PB26,
	PB27,
	PA26,
	PA27,
}GPIOn_enum;


void gpio_init(GPIO_Regs* gpio, uint32_t pins, GPIOn_enum gpion, GPIO_Mode_enum mode);
void gpio_set(GPIO_Regs* gpio, uint32_t pins, uint8_t mode);
void gpio_toggle(GPIO_Regs* gpio, uint32_t pins);
uint8_t gpio_get(GPIO_Regs* gpio, uint32_t pins);

#endif