#include "ml_oled_font.h"
#include "math.h"
#include "ml_oled.h"

// oled代码参考自江协科技 特此说明

/*引脚配置*/
#define OLED_W_SCL(x)		gpio_set(OLED_GPIO,OLED_SCL_Pin,x)
#define OLED_W_SDA(x)		gpio_set(OLED_GPIO,OLED_SDA_Pin,x)

#define OLED_I2C_HALF_PERIOD_US  4U
#define OLED_INIT_RETRY_COUNT     3U

static uint8_t oled_ready = 0;

/*
 * Open-drain software I2C:
 * - the output latch always contains 0;
 * - enabling output pulls the line low;
 * - disabling output releases the line to the pull-up resistor.
 */
static void OLED_SCL_Low(void)
{
	OLED_W_SCL(0);
	DL_GPIO_enableOutput(OLED_GPIO, OLED_SCL_Pin);
	delay_us(OLED_I2C_HALF_PERIOD_US);
}

static void OLED_SCL_Release(void)
{
	DL_GPIO_disableOutput(OLED_GPIO, OLED_SCL_Pin);
	delay_us(OLED_I2C_HALF_PERIOD_US);
}

static void OLED_SDA_Low(void)
{
	OLED_W_SDA(0);
	DL_GPIO_enableOutput(OLED_GPIO, OLED_SDA_Pin);
	delay_us(OLED_I2C_HALF_PERIOD_US);
}

static void OLED_SDA_Release(void)
{
	DL_GPIO_disableOutput(OLED_GPIO, OLED_SDA_Pin);
	delay_us(OLED_I2C_HALF_PERIOD_US);
}

static uint8_t OLED_SDA_Read(void)
{
	return gpio_get(OLED_GPIO, OLED_SDA_Pin);
}

/*引脚初始化*/
void OLED_I2C_Init(void)
{
	DL_GPIO_initDigitalInputFeatures(OLED_SCL,
		DL_GPIO_INVERSION_DISABLE,
		DL_GPIO_RESISTOR_PULL_UP,
		DL_GPIO_HYSTERESIS_ENABLE,
		DL_GPIO_WAKEUP_DISABLE);
	DL_GPIO_initDigitalInputFeatures(OLED_SDA,
		DL_GPIO_INVERSION_DISABLE,
		DL_GPIO_RESISTOR_PULL_UP,
		DL_GPIO_HYSTERESIS_ENABLE,
		DL_GPIO_WAKEUP_DISABLE);

	OLED_W_SCL(0);
	OLED_W_SDA(0);
	DL_GPIO_disableOutput(OLED_GPIO, OLED_SCL_Pin | OLED_SDA_Pin);
	delay_us(10);
}

static void OLED_I2C_Recover(void)
{
	uint8_t i;

	OLED_SDA_Release();
	for (i = 0; i < 9; i++)
	{
		OLED_SCL_Low();
		OLED_SCL_Release();
	}

	OLED_SDA_Low();
	OLED_SCL_Release();
	OLED_SDA_Release();
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
void OLED_I2C_Start(void)
{
	OLED_SDA_Release();
	OLED_SCL_Release();
	OLED_SDA_Low();
	OLED_SCL_Low();
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
void OLED_I2C_Stop(void)
{
	OLED_SDA_Low();
	OLED_SCL_Release();
	OLED_SDA_Release();
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
uint8_t OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	uint8_t ack;

	for (i = 0; i < 8; i++)
	{
		if ((Byte & (uint8_t)(0x80U >> i)) != 0U)
		{
			OLED_SDA_Release();
		}
		else
		{
			OLED_SDA_Low();
		}

		OLED_SCL_Release();
		OLED_SCL_Low();
	}

	OLED_SDA_Release();
	OLED_SCL_Release();
	ack = (uint8_t)(OLED_SDA_Read() == 0U);
	OLED_SCL_Low();

	return ack;
}

static uint8_t OLED_WriteCommandChecked(uint8_t Command)
{
	uint8_t ok;

	OLED_I2C_Start();
	ok  = OLED_I2C_SendByte(0x78);
	ok &= OLED_I2C_SendByte(0x00);
	ok &= OLED_I2C_SendByte(Command);
	OLED_I2C_Stop();

	return ok;
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
void OLED_WriteCommand(uint8_t Command)
{
	if (OLED_WriteCommandChecked(Command) == 0U)
	{
		oled_ready = 0;
	}
}

static uint8_t OLED_WriteDataChecked(uint8_t Data)
{
	uint8_t ok;

	OLED_I2C_Start();
	ok  = OLED_I2C_SendByte(0x78);
	ok &= OLED_I2C_SendByte(0x40);
	ok &= OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();

	return ok;
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
void OLED_WriteData(uint8_t Data)
{
	if (OLED_WriteDataChecked(Data) == 0U)
	{
		oled_ready = 0;
	}
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置低4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置高4位
}

/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}

/**
  * @brief  OLED显示一个字符
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内容
	}
}

/**
  * @brief  OLED显示字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/**
  * @brief  OLED显示数字（十进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十进制，带符号数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：-2147483648~2147483647
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十六进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
  * @param  Length 要显示数字的长度，范围：1~8
  * @retval 无
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}

/**
  * @brief  OLED显示数字（二进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  Length 要显示数字的长度，范围：1~16
  * @retval 无
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

/**
  * @brief  OLED显示浮点数
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  int_Length 要显示整数的长度，范围：1~16
  * @param  float_Length 要显示小数的长度，范围：1~16
  * @retval 无
  */

void OLED_ShowFloat(uint8_t Line, uint8_t Column, float Number, uint8_t int_Length, uint8_t float_Length)
{
	int temp1, temp2;
	temp1 = (int)Number;
	temp2 = (Number - temp1) * pow(10, float_Length);
	OLED_ShowSignedNum(Line, Column, temp1, int_Length);
  OLED_ShowString(Line, Column+int_Length+1, ".");
	OLED_ShowNum(Line, Column+int_Length+2, temp2, float_Length);
}


/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
static uint8_t OLED_SendInitSequence(void)
{
	static const uint8_t init_commands[] =
	{
		0xAE,
		0xD5, 0x80,
		0xA8, 0x3F,
		0xD3, 0x00,
		0x40,
		0x20, 0x02,
		0x2E,
		0xA1,
		0xC8,
		0xDA, 0x12,
		0x81, 0xCF,
		0xD9, 0xF1,
		0xDB, 0x30,
		0xA4,
		0xA6,
		0x8D, 0x14
	};
	uint8_t i;
	uint8_t ok = 1;

	for (i = 0; i < sizeof(init_commands); i++)
	{
		ok &= OLED_WriteCommandChecked(init_commands[i]);
	}

	return ok;
}

void OLED_Init(void)
{
	uint8_t attempt;

	oled_ready = 0;
	OLED_I2C_Init();

	for (attempt = 0; attempt < OLED_INIT_RETRY_COUNT; attempt++)
	{
		OLED_I2C_Recover();
		delay_ms(20);

		if (OLED_SendInitSequence() != 0U)
		{
			/*
			 * Four-wire modules have no external RESET pin.  Send the whole
			 * setup twice so a slow controller power-on cannot leave the
			 * multiplex ratio or addressing mode at an indeterminate value.
			 */
			delay_ms(30);
			if (OLED_SendInitSequence() != 0U)
			{
				oled_ready = 1;
				OLED_Clear();
				if ((oled_ready != 0U) &&
					(OLED_WriteCommandChecked(0xAF) != 0U))
				{
					delay_ms(20);
					return;
				}
			}
		}

		delay_ms(100);
	}
}

uint8_t OLED_IsReady(void)
{
	return oled_ready;
}



/***********功能描述：
显示显示BMP图片128×64起始点坐标(x,y),
x的范围0～127，
y为页的范围0～8
*****************/
 
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{ 	
 unsigned int j=0;
 unsigned char x,y;
  
  if(y1%8==0) y=y1/8;      
  else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		OLED_SetCursor(y,x0);
    for(x=x0;x<x1;x++)
	    {      
	    	OLED_WriteData(BMP[j++]);	    	
	    }
	}
} 
