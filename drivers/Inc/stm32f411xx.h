/*
 * stm32f411xx.h
 *
 *  Created on: 4 черв. 2026 р.
 *      Author: user
 */

#ifndef INC_STM32F411XX_H_
#define INC_STM32F411XX_H_

#include <stdint.h>

#define __vo volatile
#define __weak __attribute__((weak))
// there are base adresses
// describws microcontroller

/*
 * ARM cortex Mx Processor NVIC ISERx register Addresses
 */
#define NVIC_ISER0 ((__vo uint32_t *)0xE000E100)
#define NVIC_ISER1 ((__vo uint32_t *)0XE000E104)
#define NVIC_ISER2 ((__vo uint32_t *)0xE000E108)
#define NVIC_ISER3 ((__vo uint32_t *)0XE000E10c)

/*
 * ARM cortex Mx Processor NVIC ICERx register Addresses
 */
#define NVIC_ICER0 ((__vo uint32_t *)0XE000E180)
#define NVIC_ICER1 ((__vo uint32_t *)0XE000E184)
#define NVIC_ICER2 ((__vo uint32_t *)0xE000E188)
#define NVIC_ICER3 ((__vo uint32_t *)0XE000E18c)

/*
 * ARM cortex Mx Processor Priority Register Address Calculation
 */
#define NVIC_PR_BASE_ADDR ((__vo uint32_t *)0xE000E400)

#define NO_PR_BITS_IMPLEMENTED 4
/*
 * base address of Flash and SRAM memories
 */

#define FLASH_BASEADDR 0x08000000U
#define SRAM1_BASEADDR 0x20000000U
#define ROM_BASEADDR 0x1FFF0000U
#define SRAM SRAM1_BASEADDR

/*
 * AHBx and APDx Bus Peripheral base addresses
 */

#define PERIPH_BASE 0X40000000U
#define APB1PERIPH_BASEADDR PERIPH_BASE
#define APB2PERIPH_BASEADDR 0x40010000U
#define AHB1PERIPH_BASEADDR 0x40020000U
#define AHB2PERIPH_BASEADDR 0X50000000U

/*
 * Base addresses of peripherals which are hanging on AHB1 bus
 * */

#define GPIOA_BASEADDR (AHB1PERIPH_BASEADDR + 0x0000)
#define GPIOB_BASEADDR (AHB1PERIPH_BASEADDR + 0x0400)
#define GPIOC_BASEADDR (AHB1PERIPH_BASEADDR + 0x0800)
#define GPIOD_BASEADDR (AHB1PERIPH_BASEADDR + 0x0C00)
#define GPIOE_BASEADDR (AHB1PERIPH_BASEADDR + 0x1000)
#define GPIOH_BASEADDR (AHB1PERIPH_BASEADDR + 0x1C00)
#define RCC_BASEADDR (AHB1PERIPH_BASEADDR + 0x3800)

/*
 * Base addresses of perephirals which are hanging on APB1 bus
 * */
#define I2C1_BASEADDR (APB1PERIPH_BASEADDR + 0x5400)
#define I2C2_BASEADDR (APB1PERIPH_BASEADDR + 0x5800)
#define I2C3_BASEADDR (APB1PERIPH_BASEADDR + 0x5C00)
#define SPI2_BASEADDR (APB1PERIPH_BASEADDR + 0x3800)
#define SPI3_BASEADDR (APB1PERIPH_BASEADDR + 0x3C00)
#define USART2_BASEADDR (APB1PERIPH_BASEADDR + 0x4400)

/*
 * Base addresses of perephirals which are hanging on APB2 bus
 * */
#define SPI1_BASEADDR (APB2PERIPH_BASEADDR + 0x3000)
#define USART1_BASEADDR (APB2PERIPH_BASEADDR + 0x1000)
#define USART6_BASEADDR (APB2PERIPH_BASEADDR + 0x1400)
#define EXTI_BASEADDR (APB2PERIPH_BASEADDR + 0x3C00)
#define SYSCFG_BASEADDR (APB2PERIPH_BASEADDR + 0x3800)

//

typedef struct {
    __vo uint32_t CR1;
    __vo uint32_t CR2;
    __vo uint32_t SR;
    __vo uint32_t DR;
    __vo uint32_t CRCPR;
    __vo uint32_t RXCRCR;
    __vo uint32_t TXCRCR;
    __vo uint32_t SPI_I2SCFGR;
    __vo uint32_t SPI_I2SPR;

} SPI_RegDef_t;

typedef struct {
    __vo uint32_t MODER;   // GPIO port mode register
    __vo uint32_t OTYPER;  /*GPIO port output type register*/
    __vo uint32_t OSPEEDR; /*GPIO port output speed register */
    __vo uint32_t PUPDR;   /*GPIO port pull-up/pull-down register*/
    __vo uint32_t IDR;     /*GPIO port input data register*/
    __vo uint32_t ODR;     /*GPIO port output data register*/
    __vo uint32_t BSRR;    /*GPIO port bit set/reset register */
    __vo uint32_t LCKR;    /*GPIO port configuration lock register */
    __vo uint32_t AFR[2];  /*AFR[0] : GPIO alternate function low register;
                              AFR[1] : GPIO alternate function high register */
} GPIO_RegDef_t;

// peripheral register definition structure for EXTI
typedef struct {
    __vo uint32_t IMR;   // Interrupt mask register
    __vo uint32_t EMR;   /*Event mask register */
    __vo uint32_t RTSR;  /*Rising trigger selection register */
    __vo uint32_t FTSR;  /*Falling trigger selection register*/
    __vo uint32_t SWEIR; /*Software interrupt event register*/
    __vo uint32_t PR;    /*Pending register*/
} EXTI_RegDef_t;

// peripheral register definition structure for SYSCFG
typedef struct {
    __vo uint32_t MEMRMP; // SYSCFG memory remap register
    __vo uint32_t PMC;    /*SYSCFG peripheral mode configuration register */
    __vo uint32_t
        EXTICR[4]; /*SYSCFG external interrupt configuration register 1/2/3/4 */
    uint32_t RESERVED[2];
    __vo uint32_t CMPCR; /*Compensation cell control register*/
} SYSCFG_RegDef_t;

// peripheral register definition structure for RCC
typedef struct {
    __vo uint32_t CR;       /*RCC clock control register*/
    __vo uint32_t PLLCFGR;  // RCC PLL configuration register
    __vo uint32_t CFGR;     // RCC clock configuration register
    __vo uint32_t CIR;      // RCC clock interrupt register
    __vo uint32_t AHB1RSTR; // RCC AHB1 peripheral reset register
    __vo uint32_t AHB2RSTR; // RCC AHB2 peripheral reset register

    uint32_t RESERVED0[2]; // Reserved 0x18-0x1C

    __vo uint32_t APB1RSTR; // RCC APB1 peripheral reset register
    __vo uint32_t APB2RSTR; // RCC APB2 peripheral reset register

    uint32_t RESERVED1[2]; // Reserved 0x28-0x2C

    __vo uint32_t AHB1ENR; // RCC AHB1 peripheral clock enable register
    __vo uint32_t AHB2ENR; // RCC AHB2 peripheral clock enable register

    uint32_t RESERVED2[2]; // Reserved 0x38-0x3C

    __vo uint32_t APB1ENR; // RCC APB1 peripheral clock enable register
    __vo uint32_t APB2ENR; // RCC APB2 peripheral clock enable register

    uint32_t RESERVED3[2]; // Reserved 0x48-0x4C

    __vo uint32_t AHB1LPENR; // RCC AHB1 peripheral clock enable in low power
                             // mode register
    __vo uint32_t AHB2LPENR; // RCC AHB2 peripheral clock enable in low power
                             // mode register

    uint32_t RESERVED4[2]; // Reserved 0x58-0x5C

    __vo uint32_t APB1LPENR; // RCC APB1 peripheral clock enable in low power
                             // mode register
    __vo uint32_t APB2LPENR; // RCC APB2 peripheral clock enable in low power
                             // mode register

    uint32_t RESERVED5[2]; // Reserved 0x68-0x6C

    __vo uint32_t BDCR; // RCC Backup domain control register
    __vo uint32_t CSR;  // RCC clock control & status register

    uint32_t RESERVED6[2]; // Reserved 0x78-0x7C

    __vo uint32_t SSCGR;      // RCC spread spectrum clock generation register
    __vo uint32_t PLLI2SCFGR; // RCC PLLI2S configuration register

    uint32_t RESERVED7; // Reserved 0x88

    __vo uint32_t DCKCFGR; // RCC Dedicated Clocks Configuration Register
} RCC_RegDef_t;

// peripheral register definition structure for I2C
typedef struct {
    __vo uint32_t CR1;
    __vo uint32_t CR2;
    __vo uint32_t OAR1;
    __vo uint32_t OAR2;
    __vo uint32_t DR;
    __vo uint32_t SR1;
    __vo uint32_t SR2;
    __vo uint32_t CCR;
    __vo uint32_t TRISE;
    __vo uint32_t FLTR;
} I2C_RegDef_t;

/*
 * peripheral definitions
 * */
#define GPIOA ((GPIO_RegDef_t *)GPIOA_BASEADDR)
#define GPIOB ((GPIO_RegDef_t *)GPIOB_BASEADDR)
#define GPIOC ((GPIO_RegDef_t *)GPIOC_BASEADDR)
#define GPIOD ((GPIO_RegDef_t *)GPIOD_BASEADDR)
#define GPIOE ((GPIO_RegDef_t *)GPIOE_BASEADDR)
#define GPIOH ((GPIO_RegDef_t *)GPIOH_BASEADDR)

#define RCC ((RCC_RegDef_t *)RCC_BASEADDR)
#define EXTI ((EXTI_RegDef_t *)EXTI_BASEADDR)
#define SYSCFG ((SYSCFG_RegDef_t *)SYSCFG_BASEADDR)

#define SPI1 ((SPI_RegDef_t *)SPI1_BASEADDR)
#define SPI2 ((SPI_RegDef_t *)SPI2_BASEADDR)
#define SPI3 ((SPI_RegDef_t *)SPI3_BASEADDR)

#define I2C1 ((I2C_RegDef_t *)I2C1_BASEADDR)
#define I2C2 ((I2C_RegDef_t *)I2C2_BASEADDR)
#define I2C3 ((I2C_RegDef_t *)I2C3_BASEADDR)

/*
 * Clock enable macros for GPIOx peripherals
 */
#define GPIOA_PCLK_EN() (RCC->AHB1ENR |= (1 << 0))
#define GPIOB_PCLK_EN() (RCC->AHB1ENR |= (1 << 1))
#define GPIOC_PCLK_EN() (RCC->AHB1ENR |= (1 << 2))
#define GPIOD_PCLK_EN() (RCC->AHB1ENR |= (1 << 3))
#define GPIOE_PCLK_EN() (RCC->AHB1ENR |= (1 << 4))
#define GPIOH_PCLK_EN() (RCC->AHB1ENR |= (1 << 7))

/*
 * Clock enable macros for I2Cx peripherals
 */
#define I2C1_PCLK_EN() (RCC->APB1ENR |= (1 << 21))
#define I2C2_PCLK_EN() (RCC->APB1ENR |= (1 << 22))
#define I2C3_PCLK_EN() (RCC->APB1ENR |= (1 << 23))

/*
 * Clock Enable MAcros for SPIx peripherals
 */
#define SPI1_PCLK_EN() (RCC->APB2ENR |= (1 << 12))
#define SPI2_PCLK_EN() (RCC->APB1ENR |= (1 << 14))
#define SPI3_PCLK_EN() (RCC->APB1ENR |= (1 << 15))
#define SPI4_PCLK_EN() (RCC->APB2ENR |= (1 << 13))
#define SPI5_PCLK_EN() (RCC->APB2ENR |= (1 << 20))

/*
 * Clock Enable MAcros for USARTx peripherals
 */
#define USART1_PCLK_EN() (RCC->APB2ENR |= (1 << 4))
#define USART2_PCLK_EN() (RCC->APB1ENR |= (1 << 17))
#define USART6_PCLK_EN() (RCC->APB2ENR |= (1 << 5))

/*
 * Clock Enable MAcros for SYSCFG peripherals
 */
#define SYSCFG_PCLK_EN() (RCC->APB2ENR |= (1 << 14))

/*
 * Clock Disable MAcros for GPIOx peripherals
 */
#define GPIOA_PCLK_DI() (RCC->AHB1ENR &= ~(1 << 0))
#define GPIOB_PCLK_DI() (RCC->AHB1ENR &= ~(1 << 1))
#define GPIOC_PCLK_DI() (RCC->AHB1ENR &= ~(1 << 2))
#define GPIOD_PCLK_DI() (RCC->AHB1ENR &= ~(1 << 3))
#define GPIOE_PCLK_DI() (RCC->AHB1ENR &= ~(1 << 4))
#define GPIOH_PCLK_DI() (RCC->AHB1ENR &= ~(1 << 7))

/*
 * Clock Disable macros for I2Cx peripherals
 */
#define I2C1_PCLK_DI() (RCC->APB1ENR &= ~(1 << 21))
#define I2C2_PCLK_DI() (RCC->APB1ENR &= ~(1 << 22))
#define I2C3_PCLK_DI() (RCC->APB1ENR &= ~(1 << 23))

/*
 * Clock Disable MAcros for SPIx peripherals
 */
#define SPI1_PCLK_DI() (RCC->APB2ENR &= ~(1 << 12))
#define SPI2_PCLK_DI() (RCC->APB1ENR &= ~(1 << 14))
#define SPI3_PCLK_DI() (RCC->APB1ENR &= ~(1 << 15))
#define SPI4_PCLK_DI() (RCC->APB2ENR &= ~(1 << 13))
#define SPI5_PCLK_DI() (RCC->APB2ENR &= ~(1 << 20))

/*
 * Clock Disable MAcros for USARTx peripherals
 */
#define USART1_PCLK_DI() (RCC->APB2ENR &= ~(1 << 4))
#define USART2_PCLK_DI() (RCC->APB1ENR &= ~(1 << 17))
#define USART6_PCLK_DI() (RCC->APB2ENR &= ~(1 << 5))

/*
 * Clock Disable MAcros for SYSCFG peripherals
 */
#define SYSCFG_PCLK_DI() (RCC->APB2ENR &= ~(1 << 14))

/*
 * Macros to reset GPIOx peripherals
 */
#define GPIOA_REG_RESET()                                                      \
    do {                                                                       \
        (RCC->AHB1RSTR |= (1 << 0));                                           \
        (RCC->AHB1RSTR &= ~(1 << 0));                                          \
    } while (0)
#define GPIOB_REG_RESET()                                                      \
    do {                                                                       \
        (RCC->AHB1RSTR |= (1 << 1));                                           \
        (RCC->AHB1RSTR &= ~(1 << 1));                                          \
    } while (0)
#define GPIOC_REG_RESET()                                                      \
    do {                                                                       \
        (RCC->AHB1RSTR |= (1 << 2));                                           \
        (RCC->AHB1RSTR &= ~(1 << 2));                                          \
    } while (0)
#define GPIOD_REG_RESET()                                                      \
    do {                                                                       \
        (RCC->AHB1RSTR |= (1 << 3));                                           \
        (RCC->AHB1RSTR &= ~(1 << 3));                                          \
    } while (0)
#define GPIOE_REG_RESET()                                                      \
    do {                                                                       \
        (RCC->AHB1RSTR |= (1 << 4));                                           \
        (RCC->AHB1RSTR &= ~(1 << 4));                                          \
    } while (0)
#define GPIOH_REG_RESET()                                                      \
    do {                                                                       \
        (RCC->AHB1RSTR |= (1 << 7));                                           \
        (RCC->AHB1RSTR &= ~(1 << 7));                                          \
    } while (0)

/*
 * Macros to reset SPIx peripherals
 */
#define SPI1_REG_RESET()                                                       \
    do {                                                                       \
        (RCC->APB2RSTR |= (1 << 12));                                          \
        (RCC->APB2RSTR &= ~(1 << 12));                                         \
    } while (0)
#define SPI2_REG_RESET()                                                       \
    do {                                                                       \
        (RCC->APB1RSTR |= (1 << 14));                                          \
        (RCC->APB1RSTR &= ~(1 << 14));                                         \
    } while (0)
#define SPI3_REG_RESET()                                                       \
    do {                                                                       \
        (RCC->APB1RSTR |= (1 << 15));                                          \
        (RCC->APB1RSTR &= ~(1 << 15));                                         \
    } while (0)

/*
 * Macros to reset I2Cx peripherals
 */
#define I2C1_REG_RESET()                                                       \
    do {                                                                       \
        (RCC->APB1RSTR |= (1 << 21));                                          \
        (RCC->APB1RSTR &= ~(1 << 21));                                         \
    } while (0)
#define I2C2_REG_RESET()                                                       \
    do {                                                                       \
        (RCC->APB1RSTR |= (1 << 22));                                          \
        (RCC->APB1RSTR &= ~(1 << 22));                                         \
    } while (0)
#define I2C3_REG_RESET()                                                       \
    do {                                                                       \
        (RCC->APB1RSTR |= (1 << 23));                                          \
        (RCC->APB1RSTR &= ~(1 << 23));                                         \
    } while (0)

#define GPIO_BASEADDR_TO_CODE(x)                                               \
    ((x == GPIOA)   ? 0                                                        \
     : (x == GPIOB) ? 1                                                        \
     : (x == GPIOC) ? 2                                                        \
     : (x == GPIOD) ? 3                                                        \
     : (x == GPIOE) ? 4                                                        \
     : (x == GPIOH) ? 7                                                        \
                    : 0)

// IRQ(Interrupt request) Numbers of STM32F411x MCU
#define IRQ_NO_EXTI0 6
#define IRQ_NO_EXTI1 7
#define IRQ_NO_EXTI2 8
#define IRQ_NO_EXTI3 9
#define IRQ_NO_EXTI4 10
#define IRQ_NO_EXTI9_5 23
#define IRQ_NO_EXTI15_10 40

// macros for all possible priority levels
#define NVIC_IRQ_PRIO0 0
#define NVIC_IRQ_PRIO1 1
#define NVIC_IRQ_PRIO2 2
#define NVIC_IRQ_PRIO3 3
#define NVIC_IRQ_PRIO4 4
#define NVIC_IRQ_PRIO5 5
#define NVIC_IRQ_PRIO6 6
#define NVIC_IRQ_PRIO7 7
#define NVIC_IRQ_PRIO8 8
#define NVIC_IRQ_PRIO9 9
#define NVIC_IRQ_PRIO10 10
#define NVIC_IRQ_PRIO11 11
#define NVIC_IRQ_PRIO12 12
#define NVIC_IRQ_PRIO13 13
#define NVIC_IRQ_PRIO14 14
#define NVIC_IRQ_PRIO15 15

// some generic macros
#define ENABLE 1
#define DISABLE 0
#define SET ENABLE
#define RESET DISABLE
#define GPIO_PIN_SET SET
#define GPIO_PIN_RESET RESET
#define FLAG_RESET RESET
#define FLAG_SET SET

#include "stm32f411xx_gpio_driver.h"
#include "stm32f411xx_i2c_driver.h"
#include "stm32f411xx_spi_driver.h"

#endif /* INC_STM32F411XX_H_ */
