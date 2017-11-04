#include <stdio.h>
#include <string.h>
#include <math.h>

#include "stm32f10x_gpio.h"

void 	DHT22_begin(GPIO_TypeDef*,uint16_t GPIO_Pin);
float	DHT22_readhumid(void);
float	DHT22_readtemp(void);

