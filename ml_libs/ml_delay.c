#include "ml_delay.h"

void SysTick_Wait(uint32_t delay)
{
  if(delay <= 1){
    // without this step:
    // if delay == 0, this function will wait 0x00FFFFFF cycles
    // if delay == 1, this function will never return (because COUNTFLAG is set on 1->0 transition)
    return;                   // do nothing; at least 1 cycle has already passed anyway
  }
  SysTick->LOAD = (delay - 1);// count down to zero
  SysTick->VAL = 0;          // any write to CVR clears it and COUNTFLAG in CSR
  while(( SysTick->CTRL&0x00010000) == 0){};//check COUNTFLAG
}

// Time delay using busy wait.
void delay_ms(uint32_t delay)
{
  uint32_t i;
  for(i=0; i<delay; i++){
    SysTick_Wait(80000);  
  }
}

void delay_us(uint32_t delay)
{
  uint32_t i;
  for(i=0; i<delay; i++){
    SysTick_Wait(80);  
  }
}
