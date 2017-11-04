#include "stm32f10x_gpio.h"
#include "stm32f10x.h"


void 			BH1750FVI_begin(void);
uint16_t	BH1750FVI_HRget(void);

void		I2C_SendByte(uint8_t);
bool		I2C_Start(void);
bool		I2C_WaitAck(void);
void		I2C_Stop(void);
uint8_t I2C_ReceiveByte(void);
void 		I2C_Ack(void);
