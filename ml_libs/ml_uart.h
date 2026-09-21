#ifndef _ml_uart_h
#define _ml_uart_h
#include "headfile.h"

void uart_init(UART_Regs *uart, uint32_t baud, uint32_t pri);
void uart_sendbyte(UART_Regs *uart, uint8_t ch);
uint8_t uart_getbyte(UART_Regs *uart);

#endif