/*
 * stm32f411xx_gpio.c
 *
 *  Created on: 13 черв. 2026 р.
 *      Author: user
 */

#include "stm32f411xx_gpio_driver.h"
#include "stm32f411xx.h"
#include <stdint.h>

/*
 *  Peripheral Clock Setup
 */

/************************************************************
 * @fn              - GPIO_PeripheralClockControl
 *
 * @brief           - Enables or disables the peripheral clock for the given GPIO port
 *
 * @param[in]       - Base address of the GPIO peripheral
 * @param[in]       - ENABLE or DISABLE macro
 *
 * @return          - None
 *
 * @Note            - None
 */
void GPIO_PeripheralClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnOrDi) {
    if (EnOrDi == ENABLE) {
        if (pGPIOx == GPIOA) {
            GPIOA_PCLK_EN();
        } else if (pGPIOx == GPIOB) {
            GPIOB_PCLK_EN();
        } else if (pGPIOx == GPIOC) {
            GPIOC_PCLK_EN();
        } else if (pGPIOx == GPIOD) {
            GPIOD_PCLK_EN();
        } else if (pGPIOx == GPIOE) {
            GPIOE_PCLK_EN();
        } else if (pGPIOx == GPIOH) {
            GPIOH_PCLK_EN();
        }
    } else {
        if (pGPIOx == GPIOA) {
            GPIOA_PCLK_DI();
        } else if (pGPIOx == GPIOB) {
            GPIOB_PCLK_DI();
        } else if (pGPIOx == GPIOC) {
            GPIOC_PCLK_DI();
        } else if (pGPIOx == GPIOD) {
            GPIOD_PCLK_DI();
        } else if (pGPIOx == GPIOE) {
            GPIOE_PCLK_DI();
        } else if (pGPIOx == GPIOH) {
            GPIOH_PCLK_DI();
        }
    }
}

/*
 * Init and De-init
 */

/************************************************************
 * @fn              - GPIO_Init
 *
 * @brief           - Initializes and configures the GPIO pin using settings in the GPIO handle structure
 *
 * @param[in]       - Pointer to the GPIO handle structure (contains port and pin configuration)
 *
 * @return          - None
 *
 * @Note            - None
 */
void GPIO_Init(GPIO_Handle_t *pGPIOHandle) {
    uint32_t temp = 0;
    // enable the peripheral clock
    GPIO_PeripheralClockControl(pGPIOHandle->pGPIOx, ENABLE);

    // 1. configure the mode of gpio pin
    if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <= GPIO_MODE_ANALOG) {
        // the non interrupt mode
        pGPIOHandle->pGPIOx->MODER &= ~(0x3 << ((2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber)));

        temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
        pGPIOHandle->pGPIOx->MODER |= temp;

    } else {
        // interrupt mode
        if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_FT) {
            // configure the ftsr
            EXTI->FTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
            EXTI->RTSR &= ~(1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
        } else if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RT) {
            // configure the ftsr
            EXTI->RTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
            EXTI->FTSR &= ~(1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
        } else if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RFT) {
            // configure the ftsr
            EXTI->FTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
            EXTI->RTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
        }

        // configure the gpio port selection in SYSCFG_EXTICR
        uint8_t temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 4;
        uint8_t temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 4;

        uint8_t portcode = GPIO_BASEADDR_TO_CODE(pGPIOHandle->pGPIOx);
        SYSCFG_PCLK_EN();
        SYSCFG->EXTICR[temp1] |= portcode << (temp2 * 4);

        // 3 . enable the exti interrupt delivery using IMR
        EXTI->IMR |= 1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber;
    }

    temp = 0;
    // 2. configure the speed
    pGPIOHandle->pGPIOx->OSPEEDR &= ~(0x3 << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinSpeed << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
    pGPIOHandle->pGPIOx->OSPEEDR |= temp;

    temp = 0;

    // 3. configure the pupd settings
    pGPIOHandle->pGPIOx->PUPDR &= ~(0x3 << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinPuPdControl << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
    pGPIOHandle->pGPIOx->PUPDR |= temp;

    temp = 0;
    // 4. configure the optype
    pGPIOHandle->pGPIOx->OTYPER &= ~(1 << (pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));

    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinOPType << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
    pGPIOHandle->pGPIOx->OTYPER |= temp;

    temp = 0;

    // 5. configure the alt funtionality
    if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN) {

        uint8_t temp1;
        uint8_t temp2;

        temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 8;
        temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 8;
        pGPIOHandle->pGPIOx->AFR[temp1] &= ~(0xF << (4 * temp2));
        pGPIOHandle->pGPIOx->AFR[temp1] |= (pGPIOHandle->GPIO_PinConfig.GPIO_PinAltFunMode << (4 * temp2));
    }
}

/************************************************************
 * @fn              - GPIO_DeInit
 *
 * @brief           - De-initializes the given GPIO port, resetting its registers to default values
 *
 * @param[in]       - Base address of the GPIO peripheral
 *
 * @return          - None
 *
 * @Note            - None
 */
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx) {
    if (pGPIOx == GPIOA) {
        GPIOA_REG_RESET();
    } else if (pGPIOx == GPIOB) {
        GPIOB_REG_RESET();
    } else if (pGPIOx == GPIOC) {
        GPIOC_REG_RESET();
    } else if (pGPIOx == GPIOD) {
        GPIOD_REG_RESET();
    } else if (pGPIOx == GPIOE) {
        GPIOE_REG_RESET();
    } else if (pGPIOx == GPIOH) {
        GPIOH_REG_RESET();
    }
}

/*
 * Data Read and Write
 */

/************************************************************
 * @fn              - GPIO_ReadFromInputPin
 *
 * @brief           - Reads data from a specific input pin
 *
 * @param[in]       - Base address of the GPIO peripheral
 * @param[in]       - Pin number to read from
 *
 * @return          - Data read from the pin (0 or 1)
 *
 * @Note            - None
 */
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber) {
    uint8_t value;
    value = (uint8_t)((pGPIOx->IDR >> PinNumber) & 0x00000001);
    return value;
}

/************************************************************
 * @fn              - GPIO_ReadFromInputPort
 *
 * @brief           - Reads data from an entire input port
 *
 * @param[in]       - Base address of the GPIO peripheral
 *
 * @return          - 16-bit data read from the port
 *
 * @Note            - None
 */
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx) {
    uint16_t value;
    value = (uint16_t)pGPIOx->IDR;
    return value;
}

/************************************************************
 * @fn              - GPIO_WriteToOutputPin
 *
 * @brief           - Writes data to a specific output pin
 *
 * @param[in]       - Base address of the GPIO peripheral
 * @param[in]       - Pin number to write to
 * @param[in]       - Value to write (SET or RESET)
 *
 * @return          - None
 *
 * @Note            - None
 */
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber, uint8_t Value) {
    if (Value == GPIO_PIN_SET) {
        // write 1 to the output data register at the bit field corresponding to the pinnumber
        pGPIOx->ODR |= (1 << PinNumber);
    } else {
        // write 0
        pGPIOx->ODR &= ~(1 << PinNumber);
    }
}

/************************************************************
 * @fn              - GPIO_WriteToOutputPort
 *
 * @brief           - Writes data to an entire output port
 *
 * @param[in]       - Base address of the GPIO peripheral
 * @param[in]       - 16-bit value to write to the port
 *
 * @return          - None
 *
 * @Note            - None
 */
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t Value) { pGPIOx->ODR = Value; }

/************************************************************
 * @fn              - GPIO_ToggleOutputPin
 *
 * @brief           - Toggles the state of a specific output pin
 *
 * @param[in]       - Base address of the GPIO peripheral
 * @param[in]       - Pin number to toggle
 *
 * @return          - None
 *
 * @Note            - None
 */
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber) { pGPIOx->ODR ^= (1 << PinNumber); }

/*
 * IRQ Configuration and ISR Handling
 */

/************************************************************
 * @fn              - GPIO_IRQConfig
 *
 * @brief           - Configures the interrupt for a specific GPIO pin
 *
 * @param[in]       - IRQ number
 * @param[in]       - IRQ priority level
 * @param[in]       - ENABLE or DISABLE macro
 *
 * @return          - None
 *
 * @Note            - None
 */
void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi) {
    if (EnOrDi == ENABLE) {
        if (IRQNumber <= 31) {
            // program ISER0 register
            *NVIC_ISER0 |= (1 << IRQNumber);
        } else if (IRQNumber > 31 && IRQNumber <= 64) // 31 to 64
        {
            // program ISER1 register
            *NVIC_ISER1 |= (1 << IRQNumber % 32);
        } else if (IRQNumber > 64 && IRQNumber < 96) // 64 to 95
        {
            // program ISER2 register
            *NVIC_ISER2 |= (1 << IRQNumber % 64);
        }
    } else {
        if (IRQNumber <= 31) {
            // program ICER0 register
            *NVIC_ICER0 |= (1 << IRQNumber);
        } else if (IRQNumber > 31 && IRQNumber <= 64) // 31 to 64
        {
            // program ICER1 register
            *NVIC_ICER1 |= (1 << IRQNumber % 32);
        } else if (IRQNumber > 64 && IRQNumber < 96) // 64 to 95
        {
            // program ICER2 register
            *NVIC_ICER2 |= (1 << IRQNumber % 64);
        }
    }
}

/************************************************************
 * @fn              - GPIO_IRQPriorityConfig
 *
 * @brief           -
 *
 * @param[in]       - IRQ priority number
 * @param[in]       - IRQ priority
 * @param[in]       -
 *
 * @return          - None
 *
 * @Note            - None
 */
void GPIO_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority) {
    uint8_t iprx = IRQNumber / 4;
    uint8_t iprx_section = IRQNumber % 4;

    uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);
    *(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);
}

/************************************************************
 * @fn              - GPIO_IRQHandling
 *
 * @brief           - Handles the interrupt service routine (ISR) for a triggered pin
 *
 * @param[in]       - Pin number that triggered the interrupt
 *
 * @return          - None
 *
 * @Note            - None
 */
void GPIO_IRQHandling(uint8_t PinNumber) {
    // clear the exti pr register corresponding to the pin number
    if (EXTI->PR & (1 << PinNumber)) {
        // clear
        EXTI->PR |= (1 << PinNumber);
    }
}
