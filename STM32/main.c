#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f10x_conf.h"
#include "myARM.h"
#include "ESP8266.h"
#include "DHT22.h"
#include "BH1750FVI.h"
#include "IRControll.h"

extern char cmd[5];

/*****Pin & Peripheral Occupied*****
USART_TX			PA9
USART_RX			PA10
ESP8266_TX		PD5
ESP8266_RX		PD6
DHT22_DIO			PD7
BH1750FVI_SCL PB12
BH1750FVI_SDA PB13
IR_Trasmitter	PA6
IR_Recorde_R	PA0	
IR_Recorde_F	PA2
SG90_XY				
SG90_Z				
***********************************/

void NVIC_off(void);
void NVIC_on(void);
void CMD_Execute(void);



int main(void)
{
//啟動函數
	RCC_Configuration();
	GPIO_Configuration();
	//NVIC_configuration();	
	UART1_Configuration();
	TIM2_Configuration();
	
	IR_Begin();
	BH1750FVI_begin();
	DHT22_begin(GPIOD,GPIO_Pin_7);
	ESP8266_Begin(115200);
	
	SendString(USART1,"Initial complete\n\r");
	//測試點燈
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	//手動模式
	while(0)
	{
	//do nothing
	}
	
	delayms(2000);
	ESP8266_Send("AT\r\n");
	delayms(1000);
	ESP8266_Send("AT+RST\r\n");
	delayms(5000);
	delayms(5000);
	ESP8266_Send("AT+CWJAP=\"b510174\",\"88888888\"\r\n");
	delayms(5000);
	delayms(5000);
	delayms(5000);
	ESP8266_Send("AT+CWMODE=3\r\n");
	delayms(1000);
	ESP8266_Send("AT+CIPMUX=1\r\n");
	delayms(1000);
	ESP8266_Send("AT+CIFSR\r\n");
	delayms(1000);
	ESP8266_Send("AT+CIPSERVER=1,8888\r\n");
	delayms(1000);
	
	while(1)
	{
		
		float humid,temp,lux;
		char temp_s[16],humid_s[16],lux_s[16],len_s[4];
		char len_cmd[16]="AT+CIPSEND=0,";
		
		//與感應器溝通 取得讀數
		NVIC_off();
		//delayms(2000);
		SendString(USART1,"\n\r");
		DHT22_begin(GPIOD,GPIO_Pin_7);
		delayms(500);
		temp=DHT22_readtemp();
		delayms(500);
		humid=DHT22_readhumid();
		delayms(500);
		lux=BH1750FVI_HRget();
		NVIC_on();
		
		//溫度轉換字串
		f2str(temp,temp_s);
		SendString(USART1,"temp(C)=");
		SendString(USART1,temp_s);
		SendString(USART1,"\n\r");		
		//濕度轉換字串
		f2str(humid,humid_s);
		SendString(USART1,"humid(%)=");
		SendString(USART1,humid_s);
		SendString(USART1,"\n\r");
		//光度轉換字串
		f2str(lux,lux_s);
		SendString(USART1,"lux(L)=");
		SendString(USART1,lux_s);
		SendString(USART1,"\n\r");
		
		/*單機測試*/ 		
		
		//建立連線
		//ESP8266_Send("AT\r\n");
		//delayms(1000);
		ESP8266_Send("AT+CIPSTART=0,\"TCP\",\"192.168.0.4\",4000\r\n");
		delayms(2000);
		
		//統計讀數字串長度
		int2str(strlen(temp_s)+strlen(humid_s)+strlen(lux_s)+3,len_s);		
		//SendString(USART1,"Length=");
		//SendString(USART1,len_s);
		//SendString(USART1,"\r\n");
		strcat(len_cmd,len_s);
		//發送字串給SERVER
		ESP8266_Send(len_cmd);
		ESP8266_Send("\r\n");		
		
		delayms(2000);
		
		ESP8266_Send(temp_s);
		ESP8266_Send("t");
		ESP8266_Send(humid_s);
		ESP8266_Send("h");
		ESP8266_Send(lux_s);
		ESP8266_Send("l\r\n");

		
		delayms(2000);

		
		if(cmd[0]!=0)
		{	
			SendString(USART1,"CMD=");
			SendString(USART1,cmd);
			SendString(USART1,"\r\n");
			CMD_Execute();
			cmd[0]=0;cmd[1]=0;cmd[2]=0;cmd[3]=0;cmd[4]=0;
		}
			
	}
}
void CMD_Execute(void)
{
	switch(cmd[2])
	{
		case 'I':
		{
			
			switch(cmd[4])
			{
				case '0':
				IR_Play(0);
				break;
				case '1':
				IR_Play(1);
				break;
				case '2':
				IR_Play(2);
				break;
				case '3':
				IR_Play(3);
				break;
			}
			
		}
		case 'F':
		{
			if(cmd[4]=='0'){GPIO_ResetBits(GPIOD,GPIO_Pin_12);;}
			if(cmd[4]=='1'){GPIO_SetBits(GPIOD,GPIO_Pin_12);;}
			break;
		}
		case 'L':
		{
			if(cmd[4]=='0'){GPIO_ResetBits(GPIOD,GPIO_Pin_13);;}
			if(cmd[4]=='1'){GPIO_SetBits(GPIOD,GPIO_Pin_13);;}
			break;			
		}
		case 'U':
		{
			if(cmd[4]=='0'){GPIO_ResetBits(GPIOD,GPIO_Pin_10);;}
			if(cmd[4]=='1'){GPIO_SetBits(GPIOD,GPIO_Pin_10);;}
			break;
		}
	}
	
	char str_cls[16]="AT+CIPCLOSE=";
	char tmp[2];tmp[0]=cmd[0];tmp[1]='\0';
	strcat(str_cls,tmp);
	ESP8266_Send(str_cls);
	ESP8266_Send("\r\n");
}
void NVIC_off(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
}
void NVIC_on(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
}

		/*
		//單機測試
		int2str(strlen(temp_s)+strlen(humid_s)+strlen(lux_s)+3,len_s);		
		SendString(USART1,"Length=");
		SendString(USART1,len_s);
		SendString(USART1,"\r\n");
		strcat(len_cmd,len_s);
		SendString(USART1,len_cmd);
		SendString(USART1,"\r\n");
		
		SendString(USART1,"AT\r\n");
		delayms(1000);
		
		SendString(USART1,temp_s);
		SendString(USART1,"t");
		SendString(USART1,humid_s);
		SendString(USART1,"h");
		SendString(USART1,lux_s);
		SendString(USART1,"l\r\n");
		*/
