/*
 * 005spi_tx_testing.c
 *
 *  Created on: 18 лип. 2026 р.
 *      Author: user
 */
#include "stm32f411xx.h"
#include <string.h>

//PB15 --> SPI2_MOSI
//PB14 --> SPI2_MISO
//PB13 --> SPI2_SClK
//PB12 --> SPI2_NSS
// ALT function mode : 5

void SPI2_GPIOInits(void)
{
	GPIO_Handle_t SPIPins;
	SPIPins.pGPIOx = GPIOB;
	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
	SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

	//sclk
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GPIO_Init(&SPIPins);

	//mosi
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
	GPIO_Init(&SPIPins);

	//miso
	//SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
	//GPIO_Init(&SPIPins);

	//nss
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	GPIO_Init(&SPIPins);

}

void SPI2_Inits(void)
{
	SPI_Handle_t SPI2Handle;
	SPI2Handle.pSPIx = SPI2;
	SPI2Handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	SPI2Handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
	SPI2Handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV8;//2 MHz
	SPI2Handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
	SPI2Handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
	SPI2Handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
	SPI2Handle.SPIConfig.SPI_SSM = SPI_SSM_DI; //hardware slave managment enabled for NSS pin

	SPI_Init(&SPI2Handle);
}



void delay(void)
{
    for(uint32_t i = 0; i < 500000; i++);
}

void GPIO_ButtonInit(void)
{
		GPIO_Handle_t GpioBtn;

		GpioBtn.pGPIOx = GPIOA;
		GpioBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
		GpioBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_INPUT;
		GpioBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
		GpioBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;

		GPIO_Init(&GpioBtn);
}

int main(void)
{
	char user_data[] = "Hello world";

	GPIO_ButtonInit();

	//initialize the GPIO pins to behave as SPI2 pins
	SPI2_GPIOInits();

	//initialize the SPI2 peripheral parameters
	SPI2_Inits();

	SPI_SSOEConfig(SPI2,ENABLE);
	while(1)
	{
		while(!GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0));

		delay();
		//enable the SPI2 peripheral
		SPI_PeripheralControl(SPI2, ENABLE);


		uint8_t dataLen = strlen(user_data);
		SPI_SendData(SPI2,&dataLen,1);


		SPI_SendData(SPI2, (uint8_t*)user_data, strlen(user_data));

		while(SPI_GetFlagStatus(SPI2,SPI_BSY_FLAG));
		SPI_PeripheralControl(SPI2, DISABLE );
	}
	return 0;
}

//whenever we pressed the button on stm32 then onle one transmissions should begin
