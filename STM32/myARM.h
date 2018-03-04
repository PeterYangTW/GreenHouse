#include <stdio.h>
#include <string.h>

#include "stm32f10x_conf.h"

#define int2str(dx,str) sprintf(str,"%d",dx)
#define f2str(fx,str) sprintf(str,"%.1f",fx)
#define ptr2str(px,str) sprintf(str,"%p",px)


void RCC_Configuration(void);
void GPIO_Configuration(void);
void NVIC_configuration(void);
void UART1_Configuration(void);
void TIM2_Configuration(void);
void TIM2_IC_cfg(void);
void TIM4_OC1_cfg(int pulse);


void	SendString(USART_TypeDef*,char*);
void	delayms(int);
void	delay_us(uint32_t);
float	map(float,float,float,float,float);

//void	int2str(int,char*);
//void	f2str(float,char*);
//void	ptr2str(void* ptr,char *s);



static void RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;
	/* RCC system reset(for debug purpose) */
	RCC_DeInit();												//時鐘控制寄存器全部恢復默認值
	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);								//外部高速時鐘源開啟（8M晶振）
	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();				//等待外部時鐘就緒
	if(HSEStartUpStatus == SUCCESS)	{						//如果時鐘啟動成功
	
		/* HCLK = SYSCLK */
		RCC_HCLKConfig(RCC_SYSCLK_Div1);						//定義AHB設備時鐘為系統時鐘1分頻
		/* PCLK2 = HCLK */
		RCC_PCLK2Config(RCC_HCLK_Div1);							//定義APB2設備時鐘為HCLK時鐘1分頻
		/* PCLK1 = HCLK/2 */
		RCC_PCLK1Config(RCC_HCLK_Div2);							//定義APB1設備時鐘為HCLK時鐘2分頻
		/* Flash 2 wait state */
		FLASH_SetLatency(FLASH_Latency_2);						//設定內部FLASH的的延時週期為2週期
		/* Enable Prefetch Buffer */
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);	//使能FLASH預存取緩衝區
		/* PLLCLK = 8MHz * 9 = 72 MHz */
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);	//配置PLL時鐘為外部高速時鐘的9倍頻，8MHz * 9 = 72 MHz
		/* Enable PLL */
		RCC_PLLCmd(ENABLE);										//使能PLL時鐘
		/* Wait till PLL is ready */
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)		//等待PLL時鐘設置完成準備就緒
		{
		}
		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);				//使用PLL時鐘作為系統時鐘源
		/* Wait till PLL is used as system clock source */
		while(RCC_GetSYSCLKSource() != 0x08)					//返回系統所用時鐘源確認為外部高速晶振，8M晶振。
		{
		}
	}
	/* 設備時鐘控制 */
	/* Enable I2C1 and I2C2 clock */
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 | RCC_APB1Periph_I2C2, ENABLE);	//使能由APB1時鐘控制的外設中的I2C1與I2C2時鐘
	/* Enable GPIOB、GPIOA clock */
	RCC_APB2PeriphClockCmd(/*RCC_APB2Periph_ADC1 |*/ RCC_APB2Periph_USART1 | /*RCC_APB2Periph_AFIO |*/
						   RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOE |
						   RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD, ENABLE);
				   
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_USART2 |
                         RCC_APB1Periph_TIM4/* |RCC_APB1Periph_CAN1*/, ENABLE);	
	

	 //RCC_ADCCLKConfig(RCC_PCLK2_Div6);
}

static void GPIO_Configuration(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	
	//板載LED1(PB0)與LED2(PB1)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//板載LED3(PA1)與LED4(PA4)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_4; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//板載S1(PA0)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//板載S2(PC13)與S3(PC4)與S4(PC5)
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_15; 
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
	//GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_12|GPIO_Pin_13; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_Init(GPIOD, &GPIO_InitStructure);

}
static void NVIC_configuration(void)
{
		NVIC_InitTypeDef NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);	
}
static void UART1_Configuration(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
	
		//將TX引出至PA9是為板載RS232的DB9接口TXD針腳
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		//將RX引出至PA10是為板載RS232的DB9接口TXD針腳
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}


static void TIM2_Configuration(void)
{
	//定時器時鐘72MHz  
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	TIM_DeInit(TIM2);  

	TIM_InternalClockConfig(TIM2);  
	  
	// Frequency  = (TIM_Prescaler + 1) * ( TIM_Period + 1 ) / TIMCLK
	
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;       //時鐘分割(採樣頻率=原始時鐘*Div1/2/4)  
	TIM_TimeBaseStructure.TIM_Prescaler = 18000;   	              //預分頻率為 72MHz/(18*1000)=4KHz(0x0000~0xffff[65535])
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   //上數計數
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;  
	TIM_TimeBaseStructure.TIM_Period = 5000*4	- 1;                  //(溢出)週期 5000*4 ms=5s (0x0000~0xffff[65535])
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);  
	//清除溢位中斷旗標
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);  
	//關閉預先載入設定
	TIM_ARRPreloadConfig(TIM2, DISABLE);
	//開啟TIM2中斷
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC1, ENABLE);
	//致能TIM2
	TIM_Cmd(TIM2, ENABLE);      
	
}
static void TIM2_IC_cfg(void)
{
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICPolarity= TIM_ICPolarity_Falling;
  TIM_ICInitStructure.TIM_ICSelection= TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler= TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter= 0x3; 
	TIM_ICInit(TIM2,&TIM_ICInitStructure);    
}
static void TIM4_OC1_cfg(int pulse)
{
	TIM_OCInitTypeDef TIM_OCInitStrucure;
	TIM_OCInitStrucure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStrucure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStrucure.TIM_Pulse = pulse ;
	TIM_OCInitStrucure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	TIM_OC1Init(TIM4,&TIM_OCInitStrucure);
	
}
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
			if(i>strlen(string)||string[i]==0)
			{
			break;
			}
		}	
}
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
static void delay_us(uint32_t time)
{
	//進出函式的遲延值大約是1us
	uint32_t temp;
	SysTick->LOAD = 9*time;	//重裝載定時器值
	SysTick->CTRL = 1;			//定時器致能:減到0無動作
	SysTick->VAL = 0;				//清零計數器
	
	do
	{
	temp=SysTick->CTRL;
	}
	while((temp&0x01)&&(!(temp&(1<<16))));
	
	SysTick->CTRL=0;	//關閉定時器
	SysTick->VAL=0;		//清空定時器
}

static float map(float x,float in_min,float in_max,float out_min, float out_max)
{
	x = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	return x;
}
/*
static void int2str(int i, char *s)
{
  sprintf(s,"%d",i);
}
static void f2str(float i, char *s)
{
  sprintf(s,"%f",i);
}
static void ptr2str(void* ptr,char *s)
{
	sprintf(s,"%p",ptr);
}
*/


