/*
 * 005spi_tx_testing.c
 *
 *  Created on: 18 лип. 2026 р.
 *      Author: user
 */
#include "stm32f411xx.h"
#include <string.h>
//download 002.... file to the arduino from github

#define COMMAND_LED_CTRL      		0x50
#define COMMAND_SENSOR_READ      	0x51
#define COMMAND_LED_READ      		0x52
#define COMMAND_PRINT      			0x53
#define COMMAND_ID_READ      		0x54


#define LED_ON		1
#define LED_OFF		0

//arduino analog pins
#define ANALOG_PIN0		0
#define ANALOG_PIN1		1
#define ANALOG_PIN2		2
#define ANALOG_PIN3		3
#define ANALOG_PIN4		4

//arduino led
#define LED_PIN 9

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
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
	GPIO_Init(&SPIPins);

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
		GPIO_Handle_t GpioBtn,GpioLed;

		GpioBtn.pGPIOx = GPIOA;
		GpioBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
		GpioBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_INPUT;
		GpioBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
		GpioBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;

		GPIO_Init(&GpioBtn);

		GpioLed.pGPIOx = GPIOC;
		GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
		GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
		GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
		GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
		GpioBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

		GPIO_Init(&GpioLed);
}

uint8_t SPI_VerifyResponse(uint8_t ackbyte)
{
	if(ackbyte == 0xF5)
	{
		//ack
		return 1;
	}else{
		//nack
		return 0;
	}
}

int main(void)
{

	uint8_t dummy_write = 0xff;
	uint8_t dummy_read;

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

		//cmd_led_ctrl <pin no(1)>  <value(1)>
		uint8_t commndcode = COMMAND_LED_CTRL;
		uint8_t ackbyte;
		uint8_t args[2];

		//send command
		SPI_SendData(SPI2,&commndcode,1);

		//do dummy read to clear off the RXNE
		SPI_SendData(SPI2,&dummy_read,1);

		//send some dummy bits to fetch the response from the slave
		SPI_SendData(SPI2,&dummy_write,1);

		//read the ack byte received
		SPI_ReceiveData(SPI2,&ackbyte,1);

		if(SPI_VerifyResponse(ackbyte))
		{

			args[0] = LED_PIN;
			args[1] = LED_ON;

			//send arguments
			SPI_SendData(SPI2,args,2);
		}

		//CMD_SENSOR_READ  	<analog pin number(1)>
		//wait till button is pressed
		while(!GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0));

		delay();

		commndcode = COMMAND_SENSOR_READ;
		//send command
		SPI_SendData(SPI2, &commndcode, 1);

		//do dummy read to clear off the RXNE
		SPI_SendData(SPI2,&dummy_read,1);

		//send some dummy bits to fetch the response from the slave
		SPI_SendData(SPI2,&dummy_write,1);

		//read the ack byte received
		SPI_ReceiveData(SPI2,&ackbyte,1);

		if(SPI_VerifyResponse(ackbyte))
		{

			args[0] = ANALOG_PIN0;

			//send arguments
			SPI_SendData(SPI2,args,1);

			//do dummy read to clear off the RXNE
			SPI_ReceiveData(SPI2,&dummy_read,1);

			delay();

			//send some dummy bits to fetch the response from the slave
			SPI_SendData(SPI2,&dummy_write,1);

			uint8_t analog_read;
			SPI_ReceiveData(SPI2, &analog_read, 1);
		}




		//3.  CMD_LED_READ 	 <pin no(1) >
		while(!GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0));

		delay();

		commndcode = COMMAND_LED_READ;
		//send command
		SPI_SendData(SPI2, &commndcode, 1);

		//do dummy read to clear off the RXNE
		SPI_SendData(SPI2,&dummy_read,1);

		//send some dummy bits to fetch the response from the slave
		SPI_SendData(SPI2,&dummy_write,1);

		//read the ack byte received
		SPI_ReceiveData(SPI2,&ackbyte,1);

		if(SPI_VerifyResponse(ackbyte))
		{

			args[0] = LED_PIN;

			//send arguments
			SPI_SendData(SPI2,args,1);

			//do dummy read to clear off the RXNE
			SPI_ReceiveData(SPI2,&dummy_read,1);

			delay();

			//send some dummy bits to fetch the response from the slave
			SPI_SendData(SPI2,&dummy_write,1);

			uint8_t led_status;
			SPI_ReceiveData(SPI2, &led_status, 1);
		}


		//4. CMD_PRINT 		<len(2)>  <message(len) >

		//wait till button is pressed
		while( ! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0) );

		//to avoid button de-bouncing related issues 200ms of delay
		delay();

		commndcode = COMMAND_PRINT;

		//send command
		SPI_SendData(SPI2, &commndcode, 1);

		//do dummy read to clear off the RXNE
				SPI_ReceiveData(SPI2,&dummy_read,1);

				//Send some dummy byte to fetch the response from the slave
				SPI_SendData(SPI2,&dummy_write,1);

				//read the ack byte received
				SPI_ReceiveData(SPI2,&ackbyte,1);

				uint8_t message[] = "Hello ! How are you ??";
				if( SPI_VerifyResponse(ackbyte))
				{
					args[0] = strlen((char*)message);

					//send arguments
					SPI_SendData(SPI2,args,1); //sending length

					//do dummy read to clear off the RXNE
					SPI_ReceiveData(SPI2,&dummy_read,1);

					delay();

					//send message
					for(int i = 0 ; i < args[0] ; i++){
						SPI_SendData(SPI2,&message[i],1);
						SPI_ReceiveData(SPI2,&dummy_read,1);
					}



				}

				//5. CMD_ID_READ
						//wait till button is pressed
						while( ! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0) );

						//to avoid button de-bouncing related issues 200ms of delay
						delay();

						commndcode = COMMAND_ID_READ;

						//send command
						SPI_SendData(SPI2,&commndcode,1);

						//do dummy read to clear off the RXNE
						SPI_ReceiveData(SPI2,&dummy_read,1);

						//Send some dummy byte to fetch the response from the slave
						SPI_SendData(SPI2,&dummy_write,1);

						//read the ack byte received
						SPI_ReceiveData(SPI2,&ackbyte,1);

						uint8_t id[11];
						uint32_t i=0;
						if( SPI_VerifyResponse(ackbyte))
						{
							//read 10 bytes id from the slave
							for(  i = 0 ; i < 10 ; i++)
							{
								//send dummy byte to fetch data from slave
								SPI_SendData(SPI2,&dummy_write,1);
								SPI_ReceiveData(SPI2,&id[i],1);
							}

							id[10] = '\0';



						}

						//lets confirm SPI is not busy
						while( SPI_GetFlagStatus(SPI2,SPI_BSY_FLAG) );

						//Disable the SPI2 peripheral
						SPI_PeripheralControl(SPI2,DISABLE);


					}
	return 0;
}

//whenever we pressed the button on stm32 then onle one transmissions should begin
