#include "stm32f411xx_i2c_driver.h"
#include "stm32f411xx.h"
#include <stdint.h>

uint16_t AHB_PreSceler[] = {2, 4, 8, 16, 32, 64, 128, 256, 512};
uint16_t APB1_PreSceler[4] = {2, 4, 8, 16};

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx);
static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx,
                                         uint8_t SlaveAddr);
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ClearAddressFlag(I2C_RegDef_t *pI2Cx);
static void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx);

uint32_t RCC_GetPLLOutputClock(void) { return 16000000U; }

uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName) {
    if (pI2Cx->SR1 & FlagName) {
        return FLAG_SET;
    }
    return FLAG_RESET;
}

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx) {
    pI2Cx->CR1 |= (1 << I2C_CR1_START);
}

static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx,
                                         uint8_t SlaveAddr) {
    SlaveAddr = (SlaveAddr << 1);
    SlaveAddr &= ~(1 << 0); // r/nw bit = 0 (Write)
    pI2Cx->DR = SlaveAddr;
}

static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx,
                                        uint8_t SlaveAddr) {
    SlaveAddr = (SlaveAddr << 1);
    SlaveAddr |= 1; // r/nw bit = 0 (Write)
    pI2Cx->DR = SlaveAddr;
}

static void I2C_ClearAddressFlag(I2C_RegDef_t *pI2Cx) {
    uint32_t dummyRead;
    dummyRead = pI2Cx->SR1;
    dummyRead = pI2Cx->SR2;
    (void)dummyRead;
}

static void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx) {
    pI2Cx->CR1 |= (1 << I2C_CR1_STOP);
}

uint32_t RCC_GetPCLK1Value(void) {
    uint32_t pclk1 = 0, systemClk = 0;
    uint8_t clksrc, temp, ahbp, apb1;

    clksrc = ((RCC->CFGR >> 2) & 0x3);
    if (clksrc == 0) {
        systemClk = 16000000U; // HSI = 16 MHz
    } else if (clksrc == 1) {
        systemClk = 8000000U; // HSE = 8 MHz
    } else if (clksrc == 2) {
        systemClk = RCC_GetPLLOutputClock();
    }

    // AHB Prescaler
    temp = ((RCC->CFGR >> 4) & 0xF);
    if (temp < 8) {
        ahbp = 1;
    } else {
        ahbp = AHB_PreSceler[temp - 8];
    }

    // APB1 Prescaler
    temp = ((RCC->CFGR >> 10) & 0x7);
    if (temp < 4) {
        apb1 = 1;
    } else {
        apb1 = APB1_PreSceler[temp - 4];
    }

    pclk1 = (systemClk / ahbp) / apb1;
    return pclk1;
}

void I2C_PeripheralClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi) {
    if (EnOrDi == ENABLE) {
        if (pI2Cx == I2C1) {
            I2C1_PCLK_EN();
        } else if (pI2Cx == I2C2) {
            I2C2_PCLK_EN();
        } else if (pI2Cx == I2C3) {
            I2C3_PCLK_EN();
        }
    } else {
        if (pI2Cx == I2C1) {
            I2C1_PCLK_DI();
        } else if (pI2Cx == I2C2) {
            I2C2_PCLK_DI();
        } else if (pI2Cx == I2C3) {
            I2C3_PCLK_DI();
        }
    }
}

void I2C_Init(I2C_Handle_t *pI2CHandle) {
    uint32_t tempreg = 0;

    // 1. Налаштування біта ACK у CR1
    tempreg |= (pI2CHandle->I2C_Config.I2C_ACKControl << 10);
    pI2CHandle->pI2Cx->CR1 = tempreg;

    // 2. Налаштування поля FREQ у CR2
    tempreg = 0;
    tempreg = RCC_GetPCLK1Value() / 1000000U;
    pI2CHandle->pI2Cx->CR2 = (tempreg & 0x3F);

    // 3. Власна адреса пристрою у OAR1
    tempreg = 0;
    tempreg |= (pI2CHandle->I2C_Config.I2C_DeviceAddress << 1);
    tempreg |= (1 << 14); // 14-й біт має завжди залишатися 1 за стандартом RM
    pI2CHandle->pI2Cx->OAR1 = tempreg;

    // 4. Розрахунок CCR
    uint16_t ccr_value = 0;
    tempreg = 0;

    if (pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM) {
        // Standard Mode (SM <= 100 kHz)
        ccr_value =
            (RCC_GetPCLK1Value() / (2 * pI2CHandle->I2C_Config.I2C_SCLSpeed));
        tempreg |= (ccr_value & 0xFFF);
    } else {
        // Fast Mode (FM)
        tempreg |= (1 << 15);
        tempreg |= (pI2CHandle->I2C_Config.I2C_FMDutyCycle << 14);

        if (pI2CHandle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_2) {
            ccr_value = (RCC_GetPCLK1Value() /
                         (3 * pI2CHandle->I2C_Config.I2C_SCLSpeed));
        } else {
            ccr_value = (RCC_GetPCLK1Value() /
                         (25 * pI2CHandle->I2C_Config.I2C_SCLSpeed));
        }
        tempreg |= (ccr_value & 0xFFF);
    }
    pI2CHandle->pI2Cx->CCR = tempreg;

    // 5. Конфігурація TRISE
    if (pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM) {
        // SM mode: Trise(max) = 1000ns -> (1000ns * PCLK1_MHz) + 1
        tempreg = (RCC_GetPCLK1Value() / 1000000U) + 1;
    } else {
        // FM mode: Trise(max) = 300ns -> ((300 * PCLK1_MHz) / 1000) + 1
        tempreg = ((RCC_GetPCLK1Value() * 300) / 1000000000U) + 1;
    }
    pI2CHandle->pI2Cx->TRISE = (tempreg & 0x3F);
}

void I2C_DeInit(I2C_RegDef_t *pI2Cx) {
    if (pI2Cx == I2C1) {
        I2C1_REG_RESET();
    } else if (pI2Cx == I2C2) {
        I2C2_REG_RESET();
    } else if (pI2Cx == I2C3) {
        I2C3_REG_RESET();
    }
}

void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer,
                        uint8_t Len, uint8_t SlaveAddr, uint8_t Sr) {
    // 1. Генерація START
    I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

    // 2. Очікування прапорця SB (Start Bit)
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB))
        ;

    // 3. Відправка адреси слейва
    I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx, SlaveAddr);

    // 4. Очікування прапорця ADDR
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR))
        ;

    // 5. Очищення прапорця ADDR
    I2C_ClearAddressFlag(pI2CHandle->pI2Cx);

    // 6. Передача даних
    while (Len > 0) {
        while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE))
            ;
        pI2CHandle->pI2Cx->DR = *pTxBuffer;
        pTxBuffer++;
        Len--;
    }

    // 7. Очікування спустошення буферів (TXE=1 та BTF=1)
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE))
        ;
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_BTF))
        ;

    // 8. Генерація STOP
    if (Sr == I2C_DISABLE_SR) {
        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
    }
}

void I2C_MasterRecivedData(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer,
                           uint8_t Len, uint8_t SlaveAddr, uint8_t Sr) {
    // 1. Generate the START condition
    I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

    // 2. confirm that start generation is completed by checking the SB flag in
    // the SR1 Note: Until SB is cleared SCL will be stretched (pulled to LOW)
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB)) {
    }

    // 3. Send the address of the slave with r/nw bit set to R(1) (total 8 bits)
    I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, SlaveAddr);

    // 4. wait until address phase is completed by checking the ADDR flag in
    // teh SR1
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR))
        ;

    // procedure to read only 1 byte from slave
    if (Len == 1) {
        // Disable Acking
        I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);

        // clear the ADDR flag
        I2C_ClearAddressFlag(pI2CHandle->pI2Cx);

        // generate STOP condition
        if (Sr == I2C_DISABLE_SR) {
            I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
        }

        // wait until RXNE becomes 1
        while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE))
            ;

        // read data in to buffer
        *pRxBuffer = pI2CHandle->pI2Cx->DR;
    }

    // procedure to read data from slave when Len > 1
    if (Len > 1) {
        // clear the ADDR flag
        I2C_ClearAddressFlag(pI2CHandle->pI2Cx);

        // read the data until Len becomes zero
        while (Len > 0) {
            // wait until RXNE becomes 1
            while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE))
                ;

            if (Len == 2) // if last 2 bytes are remaining
            {
                // disable Acking
                I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);

                // generate stop condition
                if (Sr == I2C_DISABLE_SR) {
                    I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
                }
            }

            // read the data from data register in to buffer
            *pRxBuffer = pI2CHandle->pI2Cx->DR;

            // increment the buffer address
            pRxBuffer++;
            Len--;
        }
    }
    // re-enable ACKing
    if (pI2CHandle->I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE) {
        I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_ENABLE);
    }
}

void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi) {
    if (EnOrDi == ENABLE) {
        pI2Cx->CR1 |= (1 << I2C_CR1_PE);
    } else {
        pI2Cx->CR1 &= ~(1 << I2C_CR1_PE);
    }
}

void I2C_ManageAcking(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi) {
    if (EnOrDi == I2C_ACK_ENABLE) {
        // enable the ack
        pI2Cx->CR1 |= (I2C_ACK_ENABLE << I2C_CR1_ACK);
    } else {
        // disable the ack
        pI2Cx->CR1 &= ~(I2C_ACK_ENABLE << I2C_CR1_ACK);
    }
}