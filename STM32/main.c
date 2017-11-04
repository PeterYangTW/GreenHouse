#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f10x_conf.h"
#include "myARM.h"
#include "ESP8266.h"
#include "DHT22.h"
#include "BH1750FVI.h"


/*****Pin & Peripheral Occupied*****
USART_TX			PA9
USART_RX			PA10
ESP8266_TX		PA2
ESP8266_RX		PA3
DHT22_DIO			PD10
BH1750FVI_SCL PA6
BH1750FVI_SDA PA7
IR_Trasmitter	
IR_Recorde_R	
IR_Recorde_F	
SG90_XY				
SG90_Z				
***********************************/
int main(void)
{
//±Ò°Ê¨ç¼Æ
	RCC_Configuration();
	GPIO_Configuration();
	NVIC_configuration();	
	UART1_Configuration();
	TIM2_Configuration();
	
	BH1750FVI_begin();
	DHT22_begin(GPIOD,GPIO_Pin_7);
	ESP8266_Begin(115200);
	
	SendString(USART1,"Initial complete\n\r");
	
	float humid,temp;
	char temp_s[16],humid_s[16];
	
	while(1)
	{
		delayms(2000);
		SendString(USART1,"\n\r");
		DHT22_begin(GPIOD,GPIO_Pin_7);	
		temp=DHT22_readtemp();
		humid=DHT22_readhumid();
		f2str(temp,temp_s);
		SendString(USART1,"temp(C)=");
		SendString(USART1,temp_s);
		f2str(humid,humid_s);
		SendString(USART1,"\n\r");
		SendString(USART1,"humid(%)=");
		SendString(USART1,humid_s);
		SendString(USART1,"\n\r");
		
		ESP8266_Send("AT\r\n");
		delayms(1000);
		ESP8266_Send("AT\r\n");
		delayms(1000);
		ESP8266_Send("AT+CIPSTART=\"TCP\",\"192.168.1.2\",4000\r\n");
		delayms(3000);
		ESP8266_Send("AT+CIPSEND=14\r\n");
		delayms(3000);
		ESP8266_Send(temp_s);
		ESP8266_Send("t");
		ESP8266_Send(humid_s);
		ESP8266_Send("h\r\n");
		//do nothing
	}
}
