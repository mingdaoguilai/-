#ifndef _ml_tim_h
#define _ml_tim_h
#include "headfile.h"

void tim_interrupt_ms_init(GPTIMER_Regs *timgn, uint32_t time_ms, uint8_t pri);

#endif

