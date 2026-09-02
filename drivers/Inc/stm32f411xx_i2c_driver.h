#ifndef INC_STM32F411XX_I2C_DRIVER_H_
#define INC_STM32F411XX_I2C_DRIVER_H_

#include "stm32f411xx.h"
#include <stdint.h>

/*
 * Bit position definitions of I2C peripherals
 */

/*
 * Bit position definitions I2C_CR2
 */
#define I2C_CR1_PE 0
#define I2C_CR1_NOSCRETCH 7
#define I2C_CR1_START 8
#define I2C_CR1_STOP 9
#define I2C_CR1_ACK 10
#define I2C_CR1_SWRST 15

/*
 * Bit position definitions I2C_CR2
 */
#define I2C_CR2_FREQ 0     /* Bits 0-5: Peripheral clock frequency */
#define I2C_CR2_ITERREN 8  /* Error interrupt enable */
#define I2C_CR2_ITEVTEN 9  /* Event interrupt enable */
#define I2C_CR2_ITBUFEN 10 /* Buffer interrupt enable */
#define I2C_CR2_DMAEN 11   /* DMA requests enable */
#define I2C_CR2_LAST 12    /* DMA last transfer */

/*
 * Bit position definitions for I2C_SR1 register
 */
#define I2C_SR1_SB 0       /* Start bit (Master mode) */
#define I2C_SR1_ADDR 1     /* Address sent (Master) / Address matched (Slave) */
#define I2C_SR1_BTF 2      /* Byte transfer finished */
#define I2C_SR1_STOPF 4    /* Stop detection (Slave mode) */
#define I2C_SR1_RXNE 6     /* Data register not empty (Receivers) */
#define I2C_SR1_TXE 7      /* Data register empty (Transmitters) */
#define I2C_SR1_BERR 8     /* Bus error */
#define I2C_SR1_ARLO 9     /* Arbitration lost */
#define I2C_SR1_AF 10      /* Acknowledge failure */
#define I2C_SR1_OVR 11     /* Overrun/Underrun */
#define I2C_SR1_TIMEOUT 14 /* Timeout or Tlow error */

/*
 * Bit position definitions for I2C_SR2 register
 */
#define I2C_SR2_MSL 0     /* Master/Slave mode */
#define I2C_SR2_BUSY 1    /* Bus busy */
#define I2C_SR2_TRA 2     /* Transmitter/Receiver */
#define I2C_SR2_GENCALL 4 /* General call header (Slave mode) */
#define I2C_SR2_DUALF 7   /* Dual flag (Slave mode) */

/*
 * Bit position definitions for I2C_CCR register
 */
#define I2C_CCR_CCR 0 /* Bits 0-11: Clock control register in Master mode */
#define I2C_CCR_DUTY                                                           \
    14 /* Fast mode duty cycle (0: t_low/t_high = 2, 1: t_low/t_high = 16/9)   \
        */
#define I2C_CCR_FS                                                             \
    15 /* I2C master mode selection (0: Standard mode, 1: Fast mode) */

/*
Configuration structure for I2Cx peripheral
*/
typedef struct {
    uint32_t I2C_SCLSpeed;
    uint8_t I2C_DeviceAddress;
    uint8_t I2C_ACKControl;
    uint16_t I2C_FMDutyCycle;
} I2C_Config_t;

/*
Handle structure for I2Cx peripheral
*/
typedef struct {
    I2C_RegDef_t *pI2Cx;
    I2C_Config_t I2C_Config;
} I2C_Handle_t;

/*
    I2C_SCLSpeed
*/
#define I2C_SCL_SPEED_SM 100000
#define I2C_SCL_SPEED_FM4K 400000
#define I2C_SCL_SPEED_FM2K 200000

/*
    I2C_ACK_Control
*/
#define I2C_ACK_ENABLE 1
#define I2C_ACK_DISABLE 0

/*
    I2C_FMDutyCycle
*/
#define I2C_FM_DUTY_2 0
#define I2C_FM_DUTY_16_9 1

/*
    I2C status flag definitions
*/
#define I2C_FLAG_TXE (1 << I2C_SR1_TXE)
#define I2C_FLAG_RXNE (1 << I2C_SR1_RXNE)
#define I2C_FLAG_SB (1 << I2C_SR1_SB)
#define I2C_FLAG_ADDR (1 << I2C_SR1_ADDR)
#define I2C_FLAG_BTF (1 << I2C_SR1_BTF)
#define I2C_FLAG_STOPF (1 << I2C_SR1_STOPF)
#define I2C_FLAG_BERR (1 << I2C_SR1_BERR)
#define I2C_FLAG_ARLO (1 << I2C_SR1_ARLO)
#define I2C_FLAG_AF (1 << I2C_SR1_AF)
#define I2C_FLAG_OVR (1 << I2C_SR1_OVR)
#define I2C_FLAG_TIMEOUT (1 << I2C_SR1_TIMEOUT)

void I2C_PeripheralClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi);

/*
 * Init and De-init
 */
// function(handle_structure)
void I2C_Init(I2C_Handle_t *pI2CHandle);
void I2C_DeInit(I2C_RegDef_t *pI2Cx);

/*
 * Data Send and Receive
 */
void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer,
                        uint8_t Len, uint8_t SlaveAddr);

/*
 * IRQ Configuration and ISR handling
 */
void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi);
void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);

/*
 * Other Peripheral Control APIs
 */
void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi);
uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName);

/*
 * application callback
 */
void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEv);

#endif