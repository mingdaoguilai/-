#include "ml_uart.h"


#if !defined(__MICROLIB)
//不使用微库的话就需要添加下面的函数
#if (__ARMCLIB_VERSION <= 6000000)
//如果编译器是AC5  就定义下面这个结构体
struct __FILE
{
        int handle;
};
#endif
FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
        x = x;
}
#endif

int fputc(int ch, FILE *stream)
{
        while( DL_UART_isBusy(UART0) == true );     
			  DL_UART_Main_transmitData(UART0, ch);

        return ch;
}


//-------------------------------------------------------------------------------------------------------------------
// @brief		串口初始化
// @param	  uart	  选择串口	  可用串口UART0、UART1、UART2、UART3
// @param	  baud	  选择波特率 (可选波特率:9600/115200)
// @param	  pri	  中断优先级
// @return		void  
// Sample usage:		uart_init(UART0,115200,1);  
//-------------------------------------------------------------------------------------------------------------------
void uart_init(UART_Regs *uart, uint32_t baud, uint32_t pri)
{
	int baud1,baud2;
	if(baud == 9600)
	{
		baud1 = 26;
		baud2 = 3;
	}
	if(baud == 115200)
	{
		baud1 = 2;
		baud2 = 11;
	}
static const DL_UART_Main_ClockConfig gUART_1ClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_MFCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_1Config = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};


	if(uart == UART0)
	{
		DL_UART_Main_reset(UART0);
		DL_UART_Main_enablePower(UART0);
    DL_GPIO_initPeripheralOutputFunction(
        IOMUX_PINCM21, IOMUX_PINCM21_PF_UART0_TX);
    DL_GPIO_initPeripheralInputFunction(
        IOMUX_PINCM22, IOMUX_PINCM22_PF_UART0_RX);
    DL_UART_Main_setClockConfig(UART0, (DL_UART_Main_ClockConfig *) &gUART_1ClockConfig);

    DL_UART_Main_init(UART0, (DL_UART_Main_Config *) &gUART_1Config);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 9600
     *  Actual baud rate: 9598.08
     */
    DL_UART_Main_setOversampling(UART0, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART0, baud1, baud2);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART0,
                                 DL_UART_MAIN_INTERRUPT_RX);


    DL_UART_Main_enable(UART0);
    NVIC_ClearPendingIRQ(UART0_INT_IRQn);
		NVIC_SetPriority(UART0_INT_IRQn, pri);
    //使能串口中断
    NVIC_EnableIRQ(UART0_INT_IRQn);
	}
	else if(uart == UART1)
	{
		DL_UART_Main_reset(UART1);
		DL_UART_Main_enablePower(UART1);
    DL_GPIO_initPeripheralOutputFunction(
        IOMUX_PINCM19, IOMUX_PINCM19_PF_UART1_TX);
    DL_GPIO_initPeripheralInputFunction(
        IOMUX_PINCM20, IOMUX_PINCM20_PF_UART1_RX);
		
    DL_UART_Main_setClockConfig(UART1, (DL_UART_Main_ClockConfig *) &gUART_1ClockConfig);

    DL_UART_Main_init(UART1, (DL_UART_Main_Config *) &gUART_1Config);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 9600
     *  Actual baud rate: 9598.08
     */
    DL_UART_Main_setOversampling(UART1, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART1, baud1, baud2);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART1,
                                 DL_UART_MAIN_INTERRUPT_RX);


    DL_UART_Main_enable(UART1);
    NVIC_ClearPendingIRQ(UART1_INT_IRQn);
		NVIC_SetPriority(UART1_INT_IRQn, pri);
    //使能串口中断
    NVIC_EnableIRQ(UART1_INT_IRQn);
	}
	else if(uart == UART2)
	{
		DL_UART_Main_reset(UART2);
		DL_UART_Main_enablePower(UART2);
    DL_GPIO_initPeripheralOutputFunction(
        IOMUX_PINCM46, IOMUX_PINCM46_PF_UART2_TX);
    DL_GPIO_initPeripheralInputFunction(
        IOMUX_PINCM47, IOMUX_PINCM47_PF_UART2_RX);
		
    DL_UART_Main_setClockConfig(UART2, (DL_UART_Main_ClockConfig *) &gUART_1ClockConfig);

    DL_UART_Main_init(UART2, (DL_UART_Main_Config *) &gUART_1Config);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 9600
     *  Actual baud rate: 9598.08
     */
    DL_UART_Main_setOversampling(UART2, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART2, baud1, baud2);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART2,
                                 DL_UART_MAIN_INTERRUPT_RX);


    DL_UART_Main_enable(UART2);
    NVIC_ClearPendingIRQ(UART2_INT_IRQn);
		NVIC_SetPriority(UART2_INT_IRQn, pri);
    //使能串口中断
    NVIC_EnableIRQ(UART2_INT_IRQn);
	}
	else if(uart == UART3)
	{
		DL_UART_Main_reset(UART3);
		DL_UART_Main_enablePower(UART3);
    DL_GPIO_initPeripheralOutputFunction(
        IOMUX_PINCM15, IOMUX_PINCM15_PF_UART3_TX);
    DL_GPIO_initPeripheralInputFunction(
        IOMUX_PINCM16, IOMUX_PINCM16_PF_UART3_RX);
    DL_UART_Main_setClockConfig(UART3, (DL_UART_Main_ClockConfig *) &gUART_1ClockConfig);

    DL_UART_Main_init(UART3, (DL_UART_Main_Config *) &gUART_1Config);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 9600
     *  Actual baud rate: 9598.08
     */
    DL_UART_Main_setOversampling(UART3, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART3, baud1, baud2);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART3,
                                 DL_UART_MAIN_INTERRUPT_RX);


    DL_UART_Main_enable(UART3);
    NVIC_ClearPendingIRQ(UART3_INT_IRQn);
		NVIC_SetPriority(UART3_INT_IRQn, pri);
    //使能串口中断
    NVIC_EnableIRQ(UART3_INT_IRQn);
	}
	
		
		
}

//-------------------------------------------------------------------------------------------------------------------
// @brief		串口发送字节
// @param	  uart	  选择串口	
// @param	  byte	  发送字节
// @return		void  
// Sample usage:		uart_sendbyte(UART0,0x01);  
//-------------------------------------------------------------------------------------------------------------------
void uart_sendbyte(UART_Regs *uart, uint8_t ch)
{

    while( DL_UART_isBusy(uart) == true );
    DL_UART_Main_transmitData(uart, ch);
}
//-------------------------------------------------------------------------------------------------------------------
// @brief		串口接收字节
// @param	  uart	  选择串口	
// @return		uint8_t  
// Sample usage:		uint8_t data = uart_getbyte(UART0);  
//-------------------------------------------------------------------------------------------------------------------
uint8_t uart_getbyte(UART_Regs *uart)
{
	return DL_UART_Main_receiveData(uart);;
}