#include <stdio.h>
#include <String.h>
#include "IRControll.h"
#include "stm32f10x_conf.h"

#define int2str(dx,str) sprintf(str,"%d",dx)

#define ADDR	((uint32_t)0x08008000)
#define WRITE_END_ADDR		((uint32_t)0x0800C000)
#define FLASH_PAGE_SIZE   ((uint16_t)0x800)

/*Public Function*/
int Block=0;

void IR_Begin(void);
void IR_Record(int);
void IR_Play(int);



/*Private	Function*/
static inline void TIM2_Configuration(void);
static inline void TIM3_Configuration(void);
static inline void TIM2_IC_cfg(void);
static inline void TIM3_OC_cfg(int);
static inline void NVIC_Configuration(void);
static inline void SendString(USART_TypeDef*,char*);
static inline void delay_us(uint32_t);

/*Interruput Function*/
void TIM2_IRQHandler(void)									    //TIMER2���_�B�z���
{	
		static int flag=0;	//���L�Ĥ@�����_Ĳ�o���ȿ��J
		//static int flag2=0;	//�P�O�i�J�ĤG�����J
		static int data[400]={0};
		static int move = 0;
		

		
		if(TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
    {
		SendString(USART1,"1"); 			
			data[move]=TIM_GetCapture1(TIM2);
			//�M���Ĥ@����GetCapture�ȵL�k�ϥ�SetCounter�]��CCR�ȬO�i�J���_���e
			if(flag==0){data[move]=0;flag=1;}
			//data[move]=a;
			move++;
			TIM_SetCounter(TIM2,0);			
		}
		else if(TIM_GetITStatus(TIM2, TIM_IT_CC3) != RESET)
		{
		SendString(USART1,"3"); 
			if(flag==0){data[move]=0;flag=1;}
			data[move]=TIM_GetCapture3(TIM2);
			//data[move]=data[move]*-1;
			move++;
			TIM_SetCounter(TIM2,0);
			//TIM_ClearITPendingBit(TIM2,TIM_IT_CC2);
		}
		if(TIM_GetITStatus(TIM2, TIM_IT_Update)!=RESET && flag == 1)
		{
			SendString(USART1,"DONE\r\n");
			TIM_SetCounter(TIM2,0);
			flag = 0;			
			//�������_�V�q
			NVIC_InitTypeDef NVIC_InitStructure;
			NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
			NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
			NVIC_Init(&NVIC_InitStructure);	
			//�N����FLASH
			
			int i = 0;
			uint32_t ptr;
			ptr = ADDR;
			
			//�]�w�g�J����m�϶�
			ptr = ptr+(FLASH_PAGE_SIZE*Block);
			char* n;
			int2str(Block,n);
			SendString(USART1,"Rec to ->");
			SendString(USART1,n);
			//SendString(USART1,"\r\n");
			
			//�g�J��FLASH
			do
			{
			FLASH_ProgramWord((uint32_t)ptr,data[i]);
			i++;
			ptr = ptr + 4;
			}
			while(data[i]!=0);
			//�����T���������������_�I�GLED1
			//GPIO_SetBits(GPIOB,GPIO_Pin_0);
			//�M�����ޭȤ�
			move =0;
			for(i=0;i<400;i++){data[i]=0;}
			//SendString(USART1,"Done\r\n");
		}
		TIM_ClearITPendingBit(TIM2,TIM_IT_CC1);
		TIM_ClearITPendingBit(TIM2,TIM_IT_CC3);
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
}
void IR_Begin(void)
{
	//�Ұʨt�ή���
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2|RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM4,ENABLE);
	
	//PA6 TIM3_PWM_Modulation
	GPIO_InitTypeDef GPIO_InitStructure;	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//PA0,A2 TIM2_InputCapture_Rising/Falling 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
	//TIM2�Ұʳ]�m:�Ω�IR���J
	TIM2_Configuration();
	//TIM3�Ұʰt�m:�Ω�IR����
	TIM3_Configuration();
	//���J�H���\��t�m:CH1_Falling / CH3_Rising
	TIM2_IC_cfg();
	//��XPWM���ܰt�m	:CH1_PWM Duty=190/947=20%
	TIM3_OC_cfg(190);
	//NVIC�Ұʳ]�w
	//NVIC_Configuration();
}
void IR_Record(int in)
{
		//���ޫ��w�϶�
		Block = in;
		SendString(USART1,"Rec\n\r");
		//����òM��FLASH
		//FLASH_Status FLASHStatus = FLASH_COMPLETE;
		FLASH_Unlock();
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
		FLASH_ErasePage(ADDR + (FLASH_PAGE_SIZE * Block));
		SendString(USART1,"Erase done\n\r");
		//�}�ҿ�J�������_
		NVIC_Configuration();
}
void IR_Play(int input)
{
			Block = input;
			int i=0;
			int mode=0;
			char string[16];
			uint32_t ptr = ADDR+(FLASH_PAGE_SIZE*Block);
			//��ܯߪi�ɶ�����
			while(*(uint32_t*)ptr!=-1)
			{
			if(i%2==0){SendString(USART1,"\n\r");}				
			int2str(*(uint32_t*)ptr,string);
			SendString(USART1,string);
			SendString(USART1,"\t");
			
			//if(data[i]<0){data[i]=data[i]*(-1);}
			i++;
			ptr = ptr +4;
			}
			
			char ic[5];
			int2str(i,ic);
			SendString(USART1,"\n\r i=");
			SendString(USART1,ic);
			
			//����IR�T��
			i = 0;
			ptr = ADDR+FLASH_PAGE_SIZE*Block+4;//���L0
			
			while( *(uint32_t*)ptr != -1)
			{
				if(mode==0)
				{
				//GPIO_SetBits(GPIOB,GPIO_Pin_1);
				//TIM_ForcedOC1Config(TIM3,TIM_ForcedAction_Active);
				TIM3->CCER = 1;
				mode =1;
				}
				else if(mode==1)//else if�~�ॿ�`�ʧ@ 
				{
				//GPIO_ResetBits(GPIOB,GPIO_Pin_1);
				//TIM_ForcedOC1Config(TIM3,TIM_ForcedAction_InActive);
				TIM3->CCER = 0;
				mode =0;
				}
			delay_us(*(int*)ptr);
			ptr = ptr +4;
			}
			
			TIM3->CCER = 0;
			
}
static inline void TIM2_Configuration(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	//TIM2�Ұʳ]�w �Ω�H�����J
	TIM_DeInit(TIM2);  
	TIM_InternalClockConfig(TIM2);  
	
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;       //��������(�ļ��W�v=��l����*Div1/2/4)  
	TIM_TimeBaseStructure.TIM_Prescaler = 72;   	              //�w���W�v�� 72MHz/(18*1000)=4KHz(0x0000~0xffff[65535])
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   //�W�ƭp��
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;  
	TIM_TimeBaseStructure.TIM_Period = 65535	- 1;                  //(���X)�g�� 5000*4 ms=5s (0x0000~0xffff[65535])
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);  
	//�M�����줤�_�X��
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);  
	//�����w�����J�]�w
	TIM_ARRPreloadConfig(TIM2, DISABLE);
	//�}��TIM2���_
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC1|TIM_IT_CC3, ENABLE);
	//�P��TIM2
	TIM_Cmd(TIM2, ENABLE);	
}
static inline void TIM3_Configuration(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	//TIM3�Ұʳ]�w �Ω�38KHz���~�u����
	TIM_DeInit(TIM3);  
	TIM_InternalClockConfig(TIM3);  
	
	TIM_TimeBaseStructure.TIM_ClockDivision =	TIM_CKD_DIV1; ;         //�w�ɾ�����72MHz 1���W (DIV1/DIV2/DIV4)  
	TIM_TimeBaseStructure.TIM_Prescaler = 1;                     //�w���W�v�� 72MHz / 2 = 36MHz (0x0000~0xffff[65535])
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;     //�W�ƭp��
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;  
	TIM_TimeBaseStructure.TIM_Period = 947 - 1;                   //(���X)�g�� 36MHz/946 = 38.05KHz (0x0000~0xffff[65535])
	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);  
	//�M�����줤�_�X��
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);  
	//�����w�����J�]�w
	TIM_ARRPreloadConfig(TIM3, DISABLE);  
	//�}�ҤW�Ƥ��_
	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE); 
	//�P��TIM3
	TIM_Cmd(TIM3, ENABLE);	
}
static inline void TIM2_IC_cfg(void)
{
	TIM_ICInitTypeDef TIM_ICInitStructure;
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICPolarity= TIM_ICPolarity_Falling;
  TIM_ICInitStructure.TIM_ICSelection= TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler= TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter= 0x1; 
	TIM_ICInit(TIM2,&TIM_ICInitStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
  TIM_ICInitStructure.TIM_ICPolarity= TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection= TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler= TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter= 0x1; 
	TIM_ICInit(TIM2,&TIM_ICInitStructure);
	
}
static inline void TIM3_OC_cfg(int pulse)
{
	TIM_OCInitTypeDef TIM_OCInitStrucure;
	TIM_OCInitStrucure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStrucure.TIM_OutputState = TIM_OutputState_Disable;
	TIM_OCInitStrucure.TIM_Pulse = pulse ;
	TIM_OCInitStrucure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	TIM_OC1Init(TIM3,&TIM_OCInitStrucure);
}
static inline void NVIC_Configuration(void)
{
		NVIC_InitTypeDef NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);	
}
static inline void SendString(USART_TypeDef* USARTx,char* string)
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
static inline void delay_us(uint32_t time)
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
