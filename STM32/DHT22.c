#include <stdio.h>
#include <string.h>
#include <math.h>

#include "stm32f10x_gpio.h"
#include "DHT22.h"

void	delayus(int);
//void	delayms(int);
//void	SendString(USART_TypeDef*,char*);

uint8_t 	arr[400]={0};								//輸入緩衝區
int 			humid_temp[8+8+8+8+1]={0};	//原始二進制資料  

void DHT22_begin(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{	
	//配置指定腳位輸出主機信號 
	GPIO_InitTypeDef GPIO_InitStructure;
	
	
	int i=0; 				//迴圈控制變數 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOx, &GPIO_InitStructure);
	

	
	GPIO_ResetBits(GPIOx,GPIO_Pin);
	delayus(800);
	GPIO_SetBits(GPIOx,GPIO_Pin);
	delayus(20);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOx, &GPIO_InitStructure);
	
	//讀取原始資料
	for(i=0;i<400;i++)
	{
		if(GPIO_ReadInputDataBit(GPIOx, GPIO_Pin))
		{
			arr[i]=1;
		}
		else
		{
			arr[i]=0;
		}
		delayus(1);
	}
	
	//印出原始資料
	int hi=0,lo=0,bit=0;
	for(i=0;i<400;i++)
	{

		if(arr[i]==1)
		{
			hi++;
			//SendString(USART1,"1");
		}
		else if(arr[i]==0)
		{
			lo++;
			//SendString(USART1,"0");			
		}
		
		if(arr[i]==1&&arr[i+1]==0)
		{
			if(hi>=4){humid_temp[bit]=1;bit++;}
			if(hi<=3){humid_temp[bit]=0;bit++;}
		hi=0;
		//SendString(USART1,"\n\r");			
		}
		if(arr[i]==0&&arr[i+1]==1)
		{
		lo=0;
		//SendString(USART1,"\n\r");			
		}		
	}
	
	//印出處理完的二進制值

	
	//SendString(USART1,"%data%\n\r");	
	for(i=0;i<40;i++)
	{
		if(i%8==0&&i!=0)
		//SendString(USART1,"\n\r");
		if(humid_temp[i]==1)
		{
			//SendString(USART1,"1");
		}
		if(humid_temp[i]==0)
		{
			//SendString(USART1,"0");			
		}
	}
	
}

float DHT22_readhumid(void)
{
	//變數v用以控制權值 
	float v=15,humid=0;
	int bit;
	for(bit=0;bit<32;bit++)
	{
		if(bit>=0&&bit<=15)
		{
			if(bit==0)
			{v=15;}
			humid+=(int)humid_temp[bit]*pow(2,v);
			v--;
		}
	}
	humid=humid/10.0;
	return humid;
}
float DHT22_readtemp(void)
{
	//變數v用以控制權值 
	float v=7,temp=0;
	int bit;
	if(humid_temp[16]==1)
	{
		for(bit=0;bit<32;bit++)
		{
			if(bit>=16&&bit<=31)
			{
			if(bit==16)
			{v=15;}
			temp+=(int)humid_temp[bit]*pow(2.0,v);
			v--;
			}
		}
		temp=temp*-1;
		temp=temp/10;
	}
	else
	{ 
		for(bit=0;bit<32;bit++)
		{
			if(bit>=16&&bit<=31)
			{
			if(bit==16)
			{v=15;}
			temp+=(int)humid_temp[bit]*pow(2.0,v);
			v--;
			}
		}
		temp=temp/10;
	} 
	
	return temp;	
}
/*
static void delayms(int time)
{
	unsigned int n;

	while (time>0)
	{
		n =10288;
		while(n>0) n--;
		time--;
	}
}
*/
static void delayus(int time)
{
	//time=10*us
	unsigned int n;

	while (time>0)
	{
		n =110;
		while(n>0) n--;
		time--;
	}
}

/*
static void SendString(USART_TypeDef* USARTx,char* string)
{
		int i=0;			
		while(1)
		{				
				USART_SendData(USARTx,string[i]);
				while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET)
				{
					//等待數據傳送完成
				}
				i++;
				if(i>strlen(string))
				{				
				break;				
				}
			if(i>strlen(string))
			{
			break;
			}
		}	
}
*/
