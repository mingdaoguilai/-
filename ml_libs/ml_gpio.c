#include "ml_gpio.h"
//-------------------------------------------------------------------------------------------------------------------
// @brief		GPIO初始化
// @param	  gpio	  选择GPIO端口	
// @param	  pins    选择引脚号
// @param	  gpion   选择对应引脚 
//                  注意这里有所不同 其实就是多加一个参数 比如要初始化PA1引脚 那这个参数就是PA1
// @return		void  
// Sample usage:		gpio_init(GPIOA, DL_GPIO_PIN_1, PA1, OUT);		
//-------------------------------------------------------------------------------------------------------------------
void gpio_init(GPIO_Regs* gpio, uint32_t pins, GPIOn_enum gpion, GPIO_Mode_enum mode)
{
		if(mode == OUT)
		{
			DL_GPIO_initDigitalOutput(gpion);
      gpio_set(gpio, pins, 0);
      DL_GPIO_enableOutput(gpio, pins);
		}
		else if(mode == IN_DOWN)
		{
			DL_GPIO_initDigitalInputFeatures(gpion, DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN, DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
		}
		else if(mode == IN_UP)
		{
			DL_GPIO_initDigitalInputFeatures(gpion, DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
		}
}


//-------------------------------------------------------------------------------------------------------------------
// @brief	  设置GPIO电平
// @param	  gpio	  选择GPIO端口	
// @param	  pins    选择引脚号
// @param	  status   设置引脚电平(0/1)
// @return		void  
// Sample usage:		gpio_set(GPIOB, DL_GPIO_PIN_3, 1);
//-------------------------------------------------------------------------------------------------------------------
void gpio_set(GPIO_Regs* gpio, uint32_t pins, uint8_t status)
{
	if(status)
		gpio->DOUTSET31_0 = pins;
	else
		gpio->DOUTCLR31_0 = pins;
}

//-------------------------------------------------------------------------------------------------------------------
// @brief	  翻转GPIO电平
// @param	  gpiox	  选择GPIO端口	
// @param	  pinx    选择引脚号
// Sample usage:		gpio_toggle(GPIOB, DL_GPIO_PIN_3);
//-------------------------------------------------------------------------------------------------------------------
void gpio_toggle(GPIO_Regs* gpio, uint32_t pins)
{
	gpio->DOUTTGL31_0 = pins;
}


//-------------------------------------------------------------------------------------------------------------------
// @brief	  读取GPIO电平
// @param	  gpio	  选择GPIO端口	
// @param	  pins    选择引脚号
// @return	  引脚电平 
// Sample usage:		uint8_t state = gpio_get(GPIOB, DL_GPIO_PIN_3);
//-------------------------------------------------------------------------------------------------------------------
uint8_t gpio_get(GPIO_Regs* gpio, uint32_t pins)
{
	uint32_t temp;
	temp = gpio->DIN31_0 & pins;
	if(temp > 0)
		return 1;
	else 
		return 0;
}

