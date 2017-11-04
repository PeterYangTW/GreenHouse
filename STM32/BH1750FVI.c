#include <stdint.h>
#include <stdio.h>

#include "BH1750FVI.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "myARM.h"

#define SCL_H         GPIOA->BSRR = GPIO_Pin_6
#define SCL_L         GPIOA->BRR  = GPIO_Pin_6 
   
#define SDA_H         GPIOA->BSRR = GPIO_Pin_7
#define SDA_L         GPIOA->BRR  = GPIO_Pin_7 

#define SCL_read      GPIOA->IDR  & GPIO_Pin_6
#define SDA_read      GPIOA->IDR  & GPIO_Pin_7

void 			BH1750FVI_begin(void);
uint16_t	BH1750FVI_HRget(void);

void I2C_SendByte(uint8_t);
bool I2C_Start(void);
bool I2C_WaitAck(void);
void I2C_Stop(void);
uint8_t I2C_ReceiveByte(void);
void I2C_Ack(void);

void 	BH1750FVI_begin(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure; 
  /* Configure I2C1 pins: SCL and SDA */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	I2C_Start();
	I2C_SendByte(0x46);
	I2C_WaitAck();		
	I2C_SendByte(0x01);
	I2C_WaitAck();
	I2C_Stop();
}
uint16_t	BH1750FVI_HRget(void)
{
	uint16_t data1=0,data2=0,data=0;
	
	//送出量測指令
	I2C_Start();
	I2C_SendByte(0x46);
	I2C_WaitAck();		
	I2C_SendByte(0x20);
	I2C_WaitAck();
	I2C_Stop();
		
	//等待量測
	delayms(50);
		
	I2C_Start();
	I2C_SendByte(0x47);
	I2C_WaitAck();
	data1=I2C_ReceiveByte();
	I2C_Ack();
	data2=I2C_ReceiveByte();
	I2C_Ack();
	I2C_Stop();
	data1<<=8;
	data=data1+data2;
	
	return data;
}

void I2C_delay(void)
{	
   uint8_t i=30;
   while(i) 
   { 
     i--; 
   } 
}
bool I2C_Start(void)
{
	SDA_H;
	SCL_H;
	I2C_delay();
	//if(!SDA_read)return FALSE;
	SDA_L;
	I2C_delay();
	//if(SDA_read) return FALSE;
	SCL_L;
	I2C_delay();
	return TRUE;
}

void I2C_Stop(void)
{
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SDA_H;
	I2C_delay();
}

void I2C_Ack(void)
{	
	SCL_L;
	I2C_delay();
	I2C_delay();//
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	I2C_delay();//
	SCL_L;
	I2C_delay();
}

void I2C_NoAck(void)
{	
	SCL_L;
	I2C_delay();
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}

bool I2C_WaitAck(void) 	 //???:=1?ACK,=0?ACK
{
	SCL_L;
	I2C_delay();
	SDA_H;			
	I2C_delay();
	SCL_H;
	I2C_delay();
	I2C_delay();
	if(SDA_read)
	{
      SCL_L;
      return FALSE;
	}
	SCL_L;
	SDA_L;
	return TRUE;
}

void I2C_SendByte(uint8_t SendByte)
{
    uint8_t i=8;
    while(i--)
    {
      SCL_L;
      I2C_delay();
      if(SendByte&0x80)
			{
				SDA_H;
			}
      else
			{
        SDA_L;   
      } 
				SendByte<<=1;
        I2C_delay();
				SCL_H;
        I2C_delay();
				I2C_delay();//
    }
    SCL_L;
}
uint8_t I2C_ReceiveByte(void)
{ 
    uint8_t i=8;
    uint8_t ReceiveByte=0;

    SDA_H;				
    while(i--)
    {
      ReceiveByte<<=1;      
      SCL_L;
      I2C_delay();
			I2C_delay();//
			SCL_H;
      I2C_delay();
			I2C_delay();//
      if(SDA_read)
      {
        ReceiveByte|=0x01;
      }
    }
    SCL_L;
		I2C_delay();

    return ReceiveByte;
}
