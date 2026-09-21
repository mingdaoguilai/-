#include "ml_exti.h"

//-------------------------------------------------------------------------------------------------------------------
// @brief	  初始化外部中断 
// @param	  exti_gpio	  选择GPIO端口	所有PA引脚都可选用 定义在ml_exti.h文件里
// @param	  mode    选择模式 上升沿触发-RISING 下降沿触发-FALLING
// @param	  pri     中断优先级
// @return		void  
// Sample usage:		exti_init(EXTI_PA12, FALLING, 1);
//-------------------------------------------------------------------------------------------------------------------
void exti_init(EXTI_GPIO_enum	exti_gpio, EXTI_MODE_enum mode, uint8_t pri)
{
		uint8_t temp, gpion;
		exti_pin_init(exti_gpio, &gpion);
		if(exti_gpio >= 16)
		{
			temp = exti_gpio - 16;
			if(mode == RISING)
			{
				DL_GPIO_initDigitalInputFeatures(gpion, DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN, DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
				DL_GPIO_setUpperPinsPolarity(GPIOA, 1 << (2 * temp));
			}
			else if(mode == FALLING)
			{
				DL_GPIO_initDigitalInputFeatures(gpion, DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
				DL_GPIO_setUpperPinsPolarity(GPIOA, 1 << (2 * temp +1));
			}
		}
		else 
		{
			temp = exti_gpio;
			if(mode == RISING)
			{
				DL_GPIO_initDigitalInputFeatures(gpion, DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN, DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
				DL_GPIO_setLowerPinsPolarity(GPIOA, 1 << (2 * temp));
			}
			else if(mode == FALLING)
			{
				DL_GPIO_initDigitalInputFeatures(gpion, DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
				DL_GPIO_setLowerPinsPolarity(GPIOA, 1 << (2 * temp +1));
			}
		}

    
    DL_GPIO_clearInterruptStatus(GPIOA, 1 << exti_gpio );
    DL_GPIO_enableInterrupt(GPIOA, 1 << exti_gpio );
		
		NVIC_SetPriority(GPIOA_INT_IRQn, pri);
		NVIC_EnableIRQ(GPIOA_INT_IRQn);
		
}

void exti_pin_init(EXTI_GPIO_enum	exti_gpio, uint8_t *gpion)
{
	switch(exti_gpio)
	{
		case EXTI_PA0:{*gpion = PA0;break;}
		case EXTI_PA1:{*gpion = PA1;break;}
		case EXTI_PA2:{*gpion = PA2;break;}
		case EXTI_PA3:{*gpion = PA3;break;}
		case EXTI_PA4:{*gpion = PA4;break;}
		case EXTI_PA5:{*gpion = PA5;break;}
		case EXTI_PA6:{*gpion = PA6;break;}
		case EXTI_PA7:{*gpion = PA7;break;}
		case EXTI_PA8:{*gpion = PA8;break;}
		case EXTI_PA9:{*gpion = PA9;break;}
		case EXTI_PA10:{*gpion = PA10;break;}
		case EXTI_PA11:{*gpion = PA11;break;}
		case EXTI_PA12:{*gpion = PA12;break;}
		case EXTI_PA13:{*gpion = PA13;break;}
		case EXTI_PA14:{*gpion = PA14;break;}
		case EXTI_PA15:{*gpion = PA15;break;}
		case EXTI_PA16:{*gpion = PA16;break;}
		case EXTI_PA17:{*gpion = PA17;break;}
		case EXTI_PA18:{*gpion = PA18;break;}
		case EXTI_PA19:{*gpion = PA19;break;}
		case EXTI_PA20:{*gpion = PA20;break;}
		case EXTI_PA21:{*gpion = PA21;break;}
		case EXTI_PA22:{*gpion = PA22;break;}
		case EXTI_PA23:{*gpion = PA23;break;}
		case EXTI_PA24:{*gpion = PA24;break;}
		case EXTI_PA25:{*gpion = PA25;break;}
		case EXTI_PA26:{*gpion = PA26;break;}
		case EXTI_PA27:{*gpion = PA27;break;}
	}
}