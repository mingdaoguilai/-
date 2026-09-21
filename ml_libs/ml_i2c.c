#include "ml_i2c.h"

#define delay_time (2)
#define SDA_SetOut gpio_init(I2C_GPIO, I2C_SDA_GPIO_Pin, I2C_SDA, OUT)
#define SDA_SetIn  gpio_init(I2C_GPIO, I2C_SDA_GPIO_Pin, I2C_SDA, IN_UP)

void SDA_Output(uint8_t x)  
{
	gpio_set(I2C_GPIO, I2C_SDA_GPIO_Pin, x);
	delay_us(delay_time);
}
void SCL_Output(uint8_t x)  
{
	gpio_set(I2C_GPIO, I2C_SCL_GPIO_Pin, x);
	delay_us(delay_time);
}

uint8_t SDA_Input() 
{
	uint8_t x = gpio_get(I2C_GPIO, I2C_SDA_GPIO_Pin);
	delay_us(delay_time);
	return x;
}

void I2C_Init()
{
	gpio_init(I2C_GPIO, I2C_SCL_GPIO_Pin, I2C_SCL, OUT);
	gpio_init(I2C_GPIO, I2C_SDA_GPIO_Pin, I2C_SDA, OUT);
	
	SDA_Output(1);
	SCL_Output(1);
}

// 起始信号
void I2C_Start()
{
	SDA_SetOut;
	SDA_Output(1);
	SCL_Output(1);
	SDA_Output(0);
	SCL_Output(0);
}

// 终止信号 
void I2C_Stop()
{
	SDA_SetOut;
	SDA_Output(0);
	SCL_Output(1);
	SDA_Output(1);
}

// 主机发送一个字节
void I2C_SendByte(uint8_t byte)
{
	SDA_SetOut;
	for(int i = 0; i < 8; i++)
	{
		if(byte & (0x80>>i))
			SDA_Output(1);
		else
			SDA_Output(0);
		SCL_Output(1);
		SCL_Output(0);
	}
}

// 主机接收一个字节
uint8_t I2C_ReceiveByte()
{
	uint8_t byte = 0;
	SDA_SetOut;
	SDA_Output(1);
	SDA_SetIn;
	for(uint8_t i = 0; i < 8; i++)
	{
		SCL_Output(1);
		if(SDA_Input())
			byte |= (0x80>>i);
		SCL_Output(0);
	}
	
	return byte;
}

// 主机应答 
void I2C_SendAck()
{
	SDA_SetOut;
	SDA_Output(0);
	SCL_Output(1);
	SCL_Output(0);
}

// 主机不应答 
void I2C_NotSendAck()
{
	SDA_SetOut;
	SDA_Output(1);
	SCL_Output(1);
	SCL_Output(0);
}

// 等待从机应答 
uint8_t I2C_WaitAck()
{
	uint8_t byte = 0;
	SDA_SetOut;
	SDA_Output(1);
	SCL_Output(1);
	SDA_SetIn;
	byte = SDA_Input();
	SCL_Output(0);
	
	return byte;
}


//void I2C_Init(void)
//{
//	DL_GPIO_initDigitalOutput(I2C_SDA_IOMUX);
//	DL_GPIO_initDigitalOutput(I2C_SCL_IOMUX);
//	  DL_GPIO_setPins(I2C_PORT, I2C_SDA_PIN);      
//    DL_GPIO_enableOutput(I2C_PORT, I2C_SDA_PIN);
//	
//	  DL_GPIO_setPins(I2C_PORT, I2C_SCL_PIN);      
//    DL_GPIO_enableOutput(I2C_PORT, I2C_SCL_PIN);
//}
///******************************************************************
// * 函 数 名 称：IIC_Start
// * 函 数 说 明：IIC起始信号
// * 函 数 形 参：无
// * 函 数 返 回：无
// * 作       者：LC
// * 备       注：无
//******************************************************************/
//void I2C_Start(void)
//{
//        SDA_OUT();

//        SCL(0);
//        SDA(1);
//        SCL(1);

//        delay_us(5);

//        SDA(0);
//        delay_us(5);
//        SCL(0);
//        delay_us(5);


//}

///******************************************************************
// * 函 数 名 称：IIC_Stop
// * 函 数 说 明：IIC停止信号
// * 函 数 形 参：无
// * 函 数 返 回：无
// * 作       者：LC
// * 备       注：无
//******************************************************************/
//void I2C_Stop(void)
//{
//        SDA_OUT();

//        SCL(0);
//        SDA(0);

//        SCL(1);
//        delay_us(5);
//        SDA(1);
//        delay_us(5);

//}

///******************************************************************
// * 函 数 名 称：IIC_Send_Ack
// * 函 数 说 明：主机发送应答
// * 函 数 形 参：0应答  1非应答
// * 函 数 返 回：无
// * 作       者：LC
// * 备       注：无
//******************************************************************/
//void I2C_SendAck(uint8_t ack)
//{
//        SDA_OUT();
//        SCL(0);
//        SDA(0);
//        delay_us(5);
//        if(!ack) SDA(0);
//        else         SDA(1);
//        SCL(1);
//        delay_us(5);
//        SCL(0);
//        SDA(1);
//}

///******************************************************************
// * 函 数 名 称：IIC_Wait_Ack
// * 函 数 说 明：等待从机应答
// * 函 数 形 参：无
// * 函 数 返 回：1=无应答   0=有应答
// * 作       者：LC
// * 备       注：无
//******************************************************************/
//uint8_t I2C_WaitAck(void)
//{
//        char ack = 0;
//        unsigned char ack_flag = 10;
//        SDA_IN();
//    SDA(1);
//        delay_us(5);
//        SCL(1);
//        delay_us(5);
//        while( (SDA_GET()==1) && ( ack_flag ) )
//        {
//                ack_flag--;
//                delay_us(5);
//        }

//        if( ack_flag <= 0 )
//        {
//                I2C_Stop();
//                return 1;
//        }
//        else
//        {
//                SCL(0);
//                SDA_OUT();
//        }
//        return ack;
//}
///******************************************************************
// * 函 数 名 称：IIC_Write
// * 函 数 说 明：IIC写一个字节
// * 函 数 形 参：dat写入的数据
// * 函 数 返 回：无
// * 作       者：LC
// * 备       注：无
//******************************************************************/
//void I2C_SendByte(uint8_t data)
//{
//        int i = 0;
//        SDA_OUT();
//        SCL(0);//拉低时钟开始数据传输

//        for( i = 0; i < 8; i++ )
//        {
//                SDA( (data & 0x80) >> 7 );
//                delay_us(2);
//        data<<=1;
//        delay_us(6);
//                SCL(1);
//                delay_us(4);
//                SCL(0);
//                delay_us(4);

//        }
//}

///******************************************************************
// * 函 数 名 称：IIC_Read
// * 函 数 说 明：IIC读1个字节
// * 函 数 形 参：无
// * 函 数 返 回：读出的1个字节数据
// * 作       者：LC
// * 备       注：无
//******************************************************************/
//uint8_t I2C_ReceiveByte(void)
//{
//        unsigned char i,receive=0;
//    SDA_IN();//SDA设置为输入
//    for(i=0;i<8;i++ )
//        {
//        SCL(0);
//        delay_us(5);
//        SCL(1);
//        delay_us(5);
//        receive<<=1;
//        if( SDA_GET() )
//        {
//            receive|=1;
//        }
//        delay_us(5);
//    }
//  return receive;
//}

