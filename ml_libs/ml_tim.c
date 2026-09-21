#include "ml_tim.h"

//-------------------------------------------------------------------------------------------------------------------
// @brief	  初始化定时器中断 
// @param	  timgn	  选择定时器	  可选定时器 TIMG0、TIMG6、TIMG7、TIMG8、TIMG12
// @param	  time_ms   中断时间间隔(单位ms 不要超过1300ms 如果是定时器TIMG12不能超过6ms)
// @param	  pri       中断优先级
// @return		void  
// Sample usage:		tim_interrupt_ms_init(TIMG0, 10, 1);
//-------------------------------------------------------------------------------------------------------------------
void tim_interrupt_ms_init(GPTIMER_Regs *timgn, uint32_t time_ms, uint8_t pri)
{
		uint8_t psc, irq;
		uint16_t arr;
		if(timgn == TIMG0)
		{
			psc = 99;
			arr = 50 * time_ms - 1;
			irq = TIMG0_INT_IRQn;
		}
		else if(timgn == TIMG6)
		{
			psc = 199;
			arr = 50 * time_ms - 1;
			irq = TIMG6_INT_IRQn;
		}
		else if(timgn == TIMG7)
		{
			psc = 199;
			arr = 50 * time_ms - 1;
			irq = TIMG7_INT_IRQn;
		}
		else if(timgn == TIMG8)
		{
			psc = 99;
			arr = 50 * time_ms - 1;
			irq = TIMG8_INT_IRQn;
		}
		else if(timgn == TIMG12)
		{
			psc = 0;
			arr = 10000 * time_ms - 1;
			irq = TIMG12_INT_IRQn;
		}
		
		DL_TimerG_ClockConfig gTIMERClockConfig = {
			.clockSel    = DL_TIMER_CLOCK_BUSCLK,
			.divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
			.prescale    = psc,
		};

		DL_TimerG_TimerConfig gTIMERTimerConfig = {
			.period     = arr,
			.timerMode  = DL_TIMER_TIMER_MODE_PERIODIC_UP,
			.startTimer = DL_TIMER_START,
		};
	
    DL_TimerG_reset(timgn);
	  DL_TimerG_enablePower(timgn);

    DL_TimerG_setClockConfig(timgn,
        (DL_TimerG_ClockConfig *) &gTIMERClockConfig);
		
		DL_TimerG_initTimerMode(timgn,
				(DL_TimerG_TimerConfig *) &gTIMERTimerConfig);
		
    DL_TimerG_enableInterrupt(timgn , DL_TIMERG_INTERRUPT_LOAD_EVENT);
    DL_TimerG_enableClock(timgn);
		
	  NVIC_ClearPendingIRQ(irq);
		NVIC_SetPriority(irq, pri);
    //使能定时器中断
    NVIC_EnableIRQ(irq);
}






