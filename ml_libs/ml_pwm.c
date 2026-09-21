#include "ml_pwm.h"

//-------------------------------------------------------------------------------------------------------------------
// @brief		PWM初始化
// @param	  timgn	  选择定时器	
// @param	  ch      选择PWM通道 可选 DL_TIMER_CC_0_INDEX 或者 DL_TIMER_CC_1_INDEX
// @param	  fre     定义频率
// @return		void  
// Sample usage:		pwm_init(TIMG8, DL_TIMER_CC_0_INDEX, 1000);
//-------------------------------------------------------------------------------------------------------------------
void pwm_init(GPTIMER_Regs *timgn, DL_TIMER_CC_INDEX ch, uint16_t fre)
{
		uint8_t psc = 0;
		uint32_t arr = 0;
	
		pwm_pin_init(timgn, ch, fre, &psc, &arr);
	
		DL_TimerG_ClockConfig gPWM_0ClockConfig = {
				.clockSel = DL_TIMER_CLOCK_BUSCLK,
				.divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
				.prescale = psc
		};

		DL_TimerG_PWMConfig gPWM_0Config = {
				.pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
				.period = arr, // arr
				.startTimer = DL_TIMER_START,
		};
		
    timgn->GPRCM.RSTCTL = GPTIMER_RSTCTL_KEY_UNLOCK_W;
    DL_TimerG_enablePower(timgn);
	
    DL_TimerG_setClockConfig(
        timgn, (DL_TimerG_ClockConfig *) &gPWM_0ClockConfig);

    DL_TimerG_initPWMMode(
        timgn, (DL_TimerG_PWMConfig *) &gPWM_0Config);

    DL_TimerG_setCaptureCompareValue(timgn, 0, ch);
		
    DL_TimerG_setCaptureCompareOutCtl(timgn, DL_TIMER_CC_OCTL_INIT_VAL_LOW, DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL, ch);

    DL_TimerG_setCaptCompUpdateMethod(timgn, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, ch);

    DL_TimerG_enableClock(timgn);
    
		timgn->COMMONREGS.CCPD |= 1 << ch;
	
}

//-------------------------------------------------------------------------------------------------------------------
// @brief		PWM占空比更新
// @param	  timgn	  选择定时器	
// @param	  ch    选择PWM通道
// @param	  duty     定义功率(反映占空比)  duty取值范围0~MAX_DUTY 对应占空比=duty/MAX_DUTY*100%
// @return		void  
// Sample usage:		pwm_update(TIMG8,DL_TIMER_CC_1_INDEX,20000);		 
//-------------------------------------------------------------------------------------------------------------------
void pwm_update(GPTIMER_Regs *timgn, DL_TIMER_CC_INDEX ch, uint32_t duty)
{
	if(duty>=MAX_DUTY)
		duty=MAX_DUTY;
	uint16_t temp = timgn->COUNTERREGS.LOAD;   //读取ARR
	uint16_t ccr = duty*(temp+1)/MAX_DUTY; 
	DL_TimerG_setCaptureCompareValue(timgn, ccr, ch);
}


//-------------------------------------------------------------------------------------------------------------------
// @brief		PWM引脚初始化
// @param	  timn	  选择定时器	
// @param	  ccrn    选择PWM通道
// @return		void  
// Sample usage:		pwm_pin_init(TIMA_0,PWM_Channel_3);		 内部调用 无需关心
//-------------------------------------------------------------------------------------------------------------------
void pwm_pin_init(GPTIMER_Regs *timgn, DL_TIMER_CC_INDEX ch, uint16_t fre, uint8_t *psc, uint32_t *arr)
{
	if(timgn == TIMG0)
	{
		if(ch == DL_TIMER_CC_0_INDEX)
		{
			DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM34,IOMUX_PINCM34_PF_TIMG0_CCP0);
			DL_GPIO_enableOutput(GPIOA, DL_GPIO_PIN_12);
			*psc = 9;
			*arr = 500000 / fre - 1;
		}
		else if(ch == DL_TIMER_CC_1_INDEX)
		{
			DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM35,IOMUX_PINCM35_PF_TIMG0_CCP1);
			DL_GPIO_enableOutput(GPIOA, DL_GPIO_PIN_13);
			*psc = 9;
			*arr = 500000 / fre - 1;
		}
	}
	else if(timgn == TIMG6)
	{
		if(ch == DL_TIMER_CC_0_INDEX)
		{
			DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM23,IOMUX_PINCM23_PF_TIMG6_CCP0);
			DL_GPIO_enableOutput(GPIOB, DL_GPIO_PIN_6);
			*psc = 19;
			*arr = 500000 / fre - 1;
		}
		else if(ch == DL_TIMER_CC_1_INDEX)
		{
			DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM24,IOMUX_PINCM24_PF_TIMG6_CCP1);
			DL_GPIO_enableOutput(GPIOB, DL_GPIO_PIN_7);
			*psc = 19;
			*arr = 500000 / fre - 1;
		}
	}
	else if(timgn == TIMG7)
	{
		if(ch == DL_TIMER_CC_0_INDEX)
		{
			DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM39,IOMUX_PINCM39_PF_TIMG7_CCP0);
			DL_GPIO_enableOutput(GPIOA, DL_GPIO_PIN_17);
			*psc = 19;
			*arr = 500000 / fre - 1;
		}
		else if(ch == DL_TIMER_CC_1_INDEX)
		{
			DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM40,IOMUX_PINCM40_PF_TIMG7_CCP1);
			DL_GPIO_enableOutput(GPIOA, DL_GPIO_PIN_18);
			*psc = 19;
			*arr = 500000 / fre - 1;
		}
	}
	else if(timgn == TIMG8)
	{
		if(ch == DL_TIMER_CC_0_INDEX)
		{
			DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM59,IOMUX_PINCM59_PF_TIMG8_CCP0);
			DL_GPIO_enableOutput(GPIOA, DL_GPIO_PIN_26);
			*psc = 9;
			*arr = 500000 / fre - 1;
		}
		else if(ch == DL_TIMER_CC_1_INDEX)
		{
			DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM60,IOMUX_PINCM60_PF_TIMG8_CCP1);
			DL_GPIO_enableOutput(GPIOA, DL_GPIO_PIN_27);
			*psc = 9;
			*arr = 500000 / fre - 1;
		}
	}
	else if(timgn == TIMG12)
	{
		if(ch == DL_TIMER_CC_0_INDEX)
		{
			DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM36,IOMUX_PINCM36_PF_TIMG12_CCP0);
			DL_GPIO_enableOutput(GPIOA, DL_GPIO_PIN_14);
			*psc = 0;
			*arr = 10000000 / fre - 1;
		}
		else if(ch == DL_TIMER_CC_1_INDEX)
		{
			DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM52,IOMUX_PINCM52_PF_TIMG12_CCP1);
			DL_GPIO_enableOutput(GPIOA, DL_GPIO_PIN_24);
			*psc = 0;
			*arr = 10000000 / fre - 1;
		}
	}

}

