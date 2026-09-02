/*
 * 001led_toggle.c
 *
 *  Created on: 17 черв. 2026 р.
 *      Author: user
 */
#include "stm32f411xx.h"

void delay(void)
{
	for(uint32_t i = 0;i<50000;i++);
}

int main(void)
{
	GPIO_Handle_t GpioLed;

	GpioLed.pGPIOx = GPIOC;
	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;

	GPIO_PeripheralClockControl(GPIOC, ENABLE);

	GPIO_Init(&GpioLed);

	while(1)
	{
		 GPIO_ToggleOutputPin(GPIOC, GPIO_PIN_NO_13);
		delay();
	}
	return 0;
}


