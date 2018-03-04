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
	RCC_DeInit();												//��������H�s��������_�q�{��
	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);								//�~�����t�������}�ҡ]8M�����^
	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();				//���ݥ~�������N��
	if(HSEStartUpStatus == SUCCESS)	{						//�p�G�����Ұʦ��\
	
		/* HCLK = SYSCLK */
		RCC_HCLKConfig(RCC_SYSCLK_Div1);						//�w�qAHB�]�Ʈ������t�ή���1���W
		/* PCLK2 = HCLK */
		RCC_PCLK2Config(RCC_HCLK_Div1);							//�w�qAPB2�]�Ʈ�����HCLK����1���W
		/* PCLK1 = HCLK/2 */
		RCC_PCLK1Config(RCC_HCLK_Div2);							//�w�qAPB1�]�Ʈ�����HCLK����2���W
		/* Flash 2 wait state */
		FLASH_SetLatency(FLASH_Latency_2);						//�]�w����FLASH�������ɶg����2�g��
		/* Enable Prefetch Buffer */
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);	//�ϯ�FLASH�w�s���w�İ�
		/* PLLCLK = 8MHz * 9 = 72 MHz */
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);	//�t�mPLL�������~�����t������9���W�A8MHz * 9 = 72 MHz
		/* Enable PLL */
		RCC_PLLCmd(ENABLE);										//�ϯ�PLL����
		/* Wait till PLL is ready */
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)		//����PLL�����]�m�����ǳƴN��
		{
		}
		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);				//�ϥ�PLL�����@���t�ή�����
		/* Wait till PLL is used as system clock source */
		while(RCC_GetSYSCLKSource() != 0x08)					//��^�t�Ωҥή������T�{���~�����t�����A8M�����C
		{
		}
	}
	/* �]�Ʈ������� */
	/* Enable I2C1 and I2C2 clock */
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 | RCC_APB1Periph_I2C2, ENABLE);	//�ϯ��APB1��������~�]����I2C1�PI2C2����
	/* Enable GPIOB�BGPIOA clock */
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
	
	//�O��LED1(PB0)�PLED2(PB1)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//�O��LED3(PA1)�PLED4(PA4)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_4; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//�O��S1(PA0)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//�O��S2(PC13)�PS3(PC4)�PS4(PC5)
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
	
		//�NTX�ޥX��PA9�O���O��RS232��DB9���fTXD�w�}
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
		//�NRX�ޥX��PA10�O���O��RS232��DB9���fTXD�w�}
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
	//�w�ɾ�����72MHz  
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	TIM_DeInit(TIM2);  

	TIM_InternalClockConfig(TIM2);  
	  
	// Frequency  = (TIM_Prescaler + 1) * ( TIM_Period + 1 ) / TIMCLK
	
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;       //��������(�ļ��W�v=��l����*Div1/2/4)  
	TIM_TimeBaseStructure.TIM_Prescaler = 18000;   	              //�w���W�v�� 72MHz/(18*1000)=4KHz(0x0000~0xffff[65535])
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   //�W�ƭp��
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;  
	TIM_TimeBaseStructure.TIM_Period = 5000*4	- 1;                  //(���X)�g�� 5000*4 ms=5s (0x0000~0xffff[65535])
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);  
	//�M�����줤�_�X��
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);  
	//�����w�����J�]�w
	TIM_ARRPreloadConfig(TIM2, DISABLE);
	//�}��TIM2���_
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC1, ENABLE);
	//�P��TIM2
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
					//���ݼƾڶǰe����
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
	//�i�X�禡���𩵭Ȥj���O1us
	uint32_t temp;
	SysTick->LOAD = 9*time;	//���˸��w�ɾ���
	SysTick->CTRL = 1;			//�w�ɾ��P��:���0�L�ʧ@
	SysTick->VAL = 0;				//�M�s�p�ƾ�
	
	do
	{
	temp=SysTick->CTRL;
	}
	while((temp&0x01)&&(!(temp&(1<<16))));
	
	SysTick->CTRL=0;	//�����w�ɾ�
	SysTick->VAL=0;		//�M�ũw�ɾ�
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


