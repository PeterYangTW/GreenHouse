/******************************************************************************
                            ���f�]�Ƥ��_�B�z���                           
  ��n�K�[���_�B�z��ƮɡA��������ƦW���Ұʤ��startup_stm32f10x_hd.s
                       �����_�V�q��Vector Table                               
******************************************************************************/

#include "stm32f10x_it.h"
#include "myARM.h"
#include <stdio.h>
#include <string.h>

void I2C1_EV_IRQHandler(void)
{
	
}
void SysTick_Handler(void)
{
	
}
void USART1_IRQHandler(void)
{
	char c='!';
	//GPIO_SetBits(GPIOB,GPIO_Pin_0);
		c = USART_ReceiveData(USART1);
		USART_SendData(USART2,c);
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
}

void USART2_IRQHandler(void)
{
	char c='!';
	//GPIO_SetBits(GPIOB,GPIO_Pin_1);
		c = USART_ReceiveData(USART2);
		USART_SendData(USART1,c);
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);
}
void TIM2_IRQHandler(void)									    //TIMER2���_�B�z���
{	
		if(TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
    { 		
			
		}
		//�M�����줤�_�X��
    TIM_ClearITPendingBit(TIM2,TIM_IT_CC1);
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);		
}
void TIM3_IRQHandler(void)									    //TIMER3���_�B�z���
{

}

void TIM4_IRQHandler(void)									    //TIMER4���_�B�z���
{
	
}

