#include <stdint.h>
#include <stdio.h>

#include "ESP8266.h"
#include "stm32f10x_gpio.h"
#include "myARM.h"

void ESP8266_Begin(int);
void ESP8266_Send(char* String);

void ESP8266_Begin(int BaudRate)
{
	
	//�Ұ�UART1,UART2,GPIOA���Ҳ�
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA,ENABLE);				   
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	//�NTX�ޥX��PA2�O��UART2���w�}
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	//�NRX�ޥX��PA3�O��UART2���w�}
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/*
	//���M�g�P��
	GPIO_PinRemapConfig(GPIO_Remap_USART2,ENABLE);
	//�ϯ�_�ή�����GPIO�Ҳծ���
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOD, ENABLE); 
	
	//�NTX�ޥX��PD5�O��UART2���M�gRemap���w�}
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
	//�NRX�ޥX��PD6�O��UART2���M�gRemap���w�}
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	*/
	
	USART_InitTypeDef USART_InitStructure;
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART1, &USART_InitStructure);
	//�}��UART���_RXĲ�o
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);

	//UART�P��
   USART_Cmd(USART1, ENABLE);
	
	//�]�wUART2
  USART_InitStructure.USART_BaudRate = BaudRate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	//���JUART��l�Ƴ]�w
  USART_Init(USART2, &USART_InitStructure);
	
	//�}��UART���_RXĲ�o
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);

	//UART�P��
	USART_Cmd(USART2, ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	//�}�ҹ��������_�V�q�޲z
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);	
		
	//�}�ҹ��������_�V�q�޲z
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	

}
void ESP8266_Send(char* String)
{
	SendString(USART2,String);
}

