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
void TIM2_IRQHandler(void)									    //TIMER2中斷處理函數
{	
		static int flag=0;	//跳過第一次中斷觸發的值錄入
		//static int flag2=0;	//判別進入第二次錄入
		static int data[400]={0};
		static int move = 0;
		

		
		if(TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
    {
		SendString(USART1,"1"); 			
			data[move]=TIM_GetCapture1(TIM2);
			//清除第一次的GetCapture值無法使用SetCounter因為CCR值是進入中斷之前
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
			//關閉中斷向量
			NVIC_InitTypeDef NVIC_InitStructure;
			NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
			NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
			NVIC_Init(&NVIC_InitStructure);	
			//燒錄至FLASH
			
			int i = 0;
			uint32_t ptr;
			ptr = ADDR;
			
			//設定寫入的位置區塊
			ptr = ptr+(FLASH_PAGE_SIZE*Block);
			char* n;
			int2str(Block,n);
			SendString(USART1,"Rec to ->");
			SendString(USART1,n);
			//SendString(USART1,"\r\n");
			
			//寫入至FLASH
			do
			{
			FLASH_ProgramWord((uint32_t)ptr,data[i]);
			i++;
			ptr = ptr + 4;
			}
			while(data[i]!=0);
			//接收訊號完成並關閉中斷點亮LED1
			//GPIO_SetBits(GPIOB,GPIO_Pin_0);
			//清除索引值及
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
	//啟動系統時鐘
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
	
	
	//TIM2啟動設置:用於IR錄入
	TIM2_Configuration();
	//TIM3啟動配置:用於IR調變
	TIM3_Configuration();
	//錄入信號功能配置:CH1_Falling / CH3_Rising
	TIM2_IC_cfg();
	//輸出PWM調變配置	:CH1_PWM Duty=190/947=20%
	TIM3_OC_cfg(190);
	//NVIC啟動設定
	//NVIC_Configuration();
}
void IR_Record(int in)
{
		//索引指定區塊
		Block = in;
		SendString(USART1,"Rec\n\r");
		//解鎖並清除FLASH
		//FLASH_Status FLASHStatus = FLASH_COMPLETE;
		FLASH_Unlock();
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
		FLASH_ErasePage(ADDR + (FLASH_PAGE_SIZE * Block));
		SendString(USART1,"Erase done\n\r");
		//開啟輸入捕捉中斷
		NVIC_Configuration();
}
void IR_Play(int input)
{
			Block = input;
			int i=0;
			int mode=0;
			char string[16];
			uint32_t ptr = ADDR+(FLASH_PAGE_SIZE*Block);
			//顯示脈波時間長度
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
			
			//播放IR訊號
			i = 0;
			ptr = ADDR+FLASH_PAGE_SIZE*Block+4;//跳過0
			
			while( *(uint32_t*)ptr != -1)
			{
				if(mode==0)
				{
				//GPIO_SetBits(GPIOB,GPIO_Pin_1);
				//TIM_ForcedOC1Config(TIM3,TIM_ForcedAction_Active);
				TIM3->CCER = 1;
				mode =1;
				}
				else if(mode==1)//else if才能正常動作 
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
	
	//TIM2啟動設定 用於信號錄入
	TIM_DeInit(TIM2);  
	TIM_InternalClockConfig(TIM2);  
	
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;       //時鐘分割(採樣頻率=原始時鐘*Div1/2/4)  
	TIM_TimeBaseStructure.TIM_Prescaler = 72;   	              //預分頻率為 72MHz/(18*1000)=4KHz(0x0000~0xffff[65535])
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   //上數計數
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;  
	TIM_TimeBaseStructure.TIM_Period = 65535	- 1;                  //(溢出)週期 5000*4 ms=5s (0x0000~0xffff[65535])
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);  
	//清除溢位中斷旗標
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);  
	//關閉預先載入設定
	TIM_ARRPreloadConfig(TIM2, DISABLE);
	//開啟TIM2中斷
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC1|TIM_IT_CC3, ENABLE);
	//致能TIM2
	TIM_Cmd(TIM2, ENABLE);	
}
static inline void TIM3_Configuration(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	//TIM3啟動設定 用於38KHz紅外線調變
	TIM_DeInit(TIM3);  
	TIM_InternalClockConfig(TIM3);  
	
	TIM_TimeBaseStructure.TIM_ClockDivision =	TIM_CKD_DIV1; ;         //定時器時鐘72MHz 1分頻 (DIV1/DIV2/DIV4)  
	TIM_TimeBaseStructure.TIM_Prescaler = 1;                     //預分頻率為 72MHz / 2 = 36MHz (0x0000~0xffff[65535])
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;     //上數計數
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;  
	TIM_TimeBaseStructure.TIM_Period = 947 - 1;                   //(溢出)週期 36MHz/946 = 38.05KHz (0x0000~0xffff[65535])
	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);  
	//清除溢位中斷旗標
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);  
	//關閉預先載入設定
	TIM_ARRPreloadConfig(TIM3, DISABLE);  
	//開啟上數中斷
	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE); 
	//致能TIM3
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
static inline void delay_us(uint32_t time)
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
