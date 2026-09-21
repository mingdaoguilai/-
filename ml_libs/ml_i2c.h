#ifndef _i2c_h
#define _i2c_h
#include "headfile.h"


///* 
//   软件模拟I2C协议
//	 修改下面三个参数 
//	 自定义SCL和SDA引脚
//	 需将对应引脚配置成开漏输出	 
//*/
#define I2C_GPIO 	         GPIOB
#define I2C_SCL_GPIO_Pin   DL_GPIO_PIN_6
#define I2C_SDA_GPIO_Pin   DL_GPIO_PIN_7
#define I2C_SCL PB6   // 注意这里要改成对应的！
#define I2C_SDA PB7


void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_SendByte(uint8_t byte);
uint8_t I2C_ReceiveByte(void);
void I2C_SendAck();
void I2C_NotSendAck(void);
uint8_t I2C_WaitAck(void);

#endif

