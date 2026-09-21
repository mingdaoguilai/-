#ifndef _ml_exti_h
#define _ml_exti_h
#include "headfile.h"

typedef enum
{
	RISING,       //上升沿触发
	FALLING,      //下降沿触发
}EXTI_MODE_enum;


typedef enum
{
	EXTI_PA0,
	EXTI_PA1,
	EXTI_PA2,
	EXTI_PA3,
	EXTI_PA4,
	EXTI_PA5,
	EXTI_PA6,
	EXTI_PA7,
	EXTI_PA8,
	EXTI_PA9,
	EXTI_PA10,
	EXTI_PA11,
	EXTI_PA12,
	EXTI_PA13,
	EXTI_PA14,
	EXTI_PA15,
	EXTI_PA16,
	EXTI_PA17,
	EXTI_PA18,
	EXTI_PA19,
	EXTI_PA20,
	EXTI_PA21,
	EXTI_PA22,
	EXTI_PA23,
	EXTI_PA24,
	EXTI_PA25,
	EXTI_PA26,
	EXTI_PA27,
}EXTI_GPIO_enum;

void exti_init(EXTI_GPIO_enum	exti_gpio, EXTI_MODE_enum mode, uint8_t pri);
void exti_pin_init(EXTI_GPIO_enum	exti_gpio, uint8_t *gpion);

#endif
