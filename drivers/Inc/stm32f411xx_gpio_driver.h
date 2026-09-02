/*
 * stm32f411xx_gpio_driver.h
 *
 *  Created on: 14 черв. 2026 р.
 *      Author: user
 */

#ifndef INC_STM32F411XX_GPIO_DRIVER_H_
#define INC_STM32F411XX_GPIO_DRIVER_H_

#include "stm32f411xx.h"
/*
 * a Configuration structure for a GPIO pin
 * */
typedef struct
{										//uint8_t because PIN numbers can range from 0 to 15
	uint8_t GPIO_PinNumber;				 /* possible values from @GPIO_PIN_NUMBERS */
	uint8_t GPIO_PinMode;				 /* possible values from @GPIO_PIN_MODES */
	uint8_t GPIO_PinSpeed;				 /* possible values from @GPIO_PIN_SPEED */
	uint8_t GPIO_PinPuPdControl;		 /* possible values from @GPIO_PIN_PuPdControl */
	uint8_t GPIO_PinOPType;				 /* possible values from @GPIO_PIN_OPType */
	uint8_t GPIO_PinAltFunMode;			 /* possible values from @GPIO_PIN_AltFunMode */
}GPIO_PinConfig_t;

/*
 * A handle structure for a GPIO pin
 */
typedef struct
{
	// pointer to hold the base address of the GPIO peripheral
	GPIO_RegDef_t *pGPIOx; 				//This hold the base address of the GPIO port to which the pin belongs
	GPIO_PinConfig_t GPIO_PinConfig; 	// holds GPIO pin configuration settings
}GPIO_Handle_t;


/*
 * @GPIO_PIN_NUMBERS
 * GPIO pin numbers
 */
#define GPIO_PIN_NO_0   0
#define GPIO_PIN_NO_1   1
#define GPIO_PIN_NO_2   2
#define GPIO_PIN_NO_3   3
#define GPIO_PIN_NO_4   4
#define GPIO_PIN_NO_5   5
#define GPIO_PIN_NO_6   6
#define GPIO_PIN_NO_7   7
#define GPIO_PIN_NO_8   8
#define GPIO_PIN_NO_9   9
#define GPIO_PIN_NO_10  10
#define GPIO_PIN_NO_11  11
#define GPIO_PIN_NO_12  12
#define GPIO_PIN_NO_13  13
#define GPIO_PIN_NO_14  14
#define GPIO_PIN_NO_15  15

/*
 * @GPIO_PIN_MODES
 * GPIO pin possible modes
 */
#define GPIO_MODE_INPUT 0
#define GPIO_MODE_OUT 1
#define GPIO_MODE_ALTFN 2
#define GPIO_MODE_ANALOG 3
#define GPIO_MODE_IT_FT 4 			//interrupt trigger - falling edge(when a button does from H to L)
#define GPIO_MODE_IT_RT 5			//Interrupt trigger - rising edge (from L to H)
#define GPIO_MODE_IT_RFT 6			//Interrupt Trigger - rising/falling edge(triggers on both)

/*
 * @GPIO_PIN_OPType
 * GPIO pin possible output types
 */
#define GPIO_OP_TYPE_PP 0
#define GPIO_OP_TYPE_OD 1

/*
 * @GPIO_PIN_SPEED
 * GPIO pin possible output speeds
 */
#define GPIO_SPEED_LOW      0
#define GPIO_SPEED_MEDIUM   1
#define GPIO_SPEED_FAST     2
#define GPIO_SPEED_HIGH     3


/*
 * @GPIO_PIN_PuPdControl
 * GPIO pin pull-up and pull-down configuration macros
 */
#define GPIO_NO_PUPD 0
#define GPIO_PIN_PU 1
#define GPIO_PIN_PD 2



/**********************************************
 * APIs supported by this driver
 **********************************************/
/*Peripheral Clock setup
 * */
//void GPIO_PeripheralClockControl(base_addr, en/di clock);
void GPIO_PeripheralClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnOrDi);

/*
 * Init and De-init
 */
//function(handle_structure)
void GPIO_Init(GPIO_Handle_t *pGPIOHandle);
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx );

/*
 * Data read and write
 * */
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx,uint8_t PinNumber);
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx);
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx,uint8_t PinNumber,uint8_t Value);
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx,uint16_t Value);
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx,uint8_t PinNumber);

/*
 * IRQ Configuration and ISR handling
 */
void GPIO_IRQInterruptConfig(uint8_t IRQNumber ,uint8_t EnOrDi);
void GPIO_IRQPriorityConfig(uint8_t IRQNumber,uint32_t IRQPriority);
void GPIO_IRQHandling(uint8_t PinNumber);


















#endif /* INC_STM32F411XX_GPIO_DRIVER_H_ */
