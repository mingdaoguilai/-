#ifndef _ml_pwm_h
#define _ml_pwm_h
#include "headfile.h"

#define MAX_DUTY 10000

void pwm_init(GPTIMER_Regs *timgn, DL_TIMER_CC_INDEX ch, uint16_t fre);
void pwm_update(GPTIMER_Regs *timgn, DL_TIMER_CC_INDEX ch, uint32_t duty);
void pwm_pin_init(GPTIMER_Regs *timgn, DL_TIMER_CC_INDEX ch, uint16_t fre, uint8_t *psc, uint32_t *arr);

#endif
