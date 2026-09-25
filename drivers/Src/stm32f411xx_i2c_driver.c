#include "stm32f411xx_i2c_driver.h"
#include "stm32f411xx.h"
#include <stdint.h>

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx);
static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ClearAddressFlag(I2C_Handle_t *pI2CHandle);
static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle);
static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle);


uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName) {
    if (pI2Cx->SR1 & FlagName) {
        return FLAG_SET;
    }
    return FLAG_RESET;
}

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx) { pI2Cx->CR1 |= (1 << I2C_CR1_START); }

static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr) {
    SlaveAddr = (SlaveAddr << 1);
    SlaveAddr &= ~(1 << 0); // r/nw bit = 0 (Write)
    pI2Cx->DR = SlaveAddr;
}

static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr) {
    SlaveAddr = (SlaveAddr << 1);
    SlaveAddr |= 1; // r/nw bit = 0 (Write)
    pI2Cx->DR = SlaveAddr;
}

static void I2C_ClearAddressFlag(I2C_Handle_t *pI2CHandle) {
    uint32_t dummy_read;
    // check for device mode
    if (pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_MSL)) {
        // master mode
        if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX) {
            if (pI2CHandle->RxSize == 1) {
                // disable ack
                I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);

                // clear the Addr flag(read SR1,SR2)
                dummy_read = pI2CHandle->pI2Cx->SR1;
                dummy_read = pI2CHandle->pI2Cx->SR2;
                (void)dummy_read;
            }
        } else {
            {
                // clear the Addr flag(read SR1,SR2)
                dummy_read = pI2CHandle->pI2Cx->SR1;
                dummy_read = pI2CHandle->pI2Cx->SR2;
                (void)dummy_read;
            }
        }
    } else {
        // slave mode
        // clear the Addr flag(read SR1,SR2)
        dummy_read = pI2CHandle->pI2Cx->SR1;
        dummy_read = pI2CHandle->pI2Cx->SR2;
        (void)dummy_read;
    }
}

void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx) { pI2Cx->CR1 |= (1 << I2C_CR1_STOP); }

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
        ccr_value = (RCC_GetPCLK1Value() / (2 * pI2CHandle->I2C_Config.I2C_SCLSpeed));
        tempreg |= (ccr_value & 0xFFF);
    } else {
        // Fast Mode (FM)
        tempreg |= (1 << 15);
        tempreg |= (pI2CHandle->I2C_Config.I2C_FMDutyCycle << 14);

        if (pI2CHandle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_2) {
            ccr_value = (RCC_GetPCLK1Value() / (3 * pI2CHandle->I2C_Config.I2C_SCLSpeed));
        } else {
            ccr_value = (RCC_GetPCLK1Value() / (25 * pI2CHandle->I2C_Config.I2C_SCLSpeed));
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

void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t Sr) {
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
    I2C_ClearAddressFlag(pI2CHandle);

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

void I2C_MasterRecivedData(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t Sr) {
    // 1. Generate the START condition
    I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

    // 2. confirm that start generation is completed by checking the SB flag
    // in the SR1 Note: Until SB is cleared SCL will be stretched (pulled to
    // LOW)
    while (!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB)) {
    }

    // 3. Send the address of the slave with r/nw bit set to R(1) (total 8
    // bits)
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
        I2C_ClearAddressFlag(pI2CHandle);

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
        I2C_ClearAddressFlag(pI2CHandle);

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

uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t Sr) {
    uint8_t busystate = pI2CHandle->TxRxState;

    if ((busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX)) {
        pI2CHandle->pTxBuffer = pTxBuffer;
        pI2CHandle->TxLen = Len;
        pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
        pI2CHandle->DevAddr = SlaveAddr;
        pI2CHandle->Sr = Sr;

        // Generate start condition
        I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

        // enable ITBUFFEN Control Bit
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITBUFEN);

        // enable ITEVTEN control Bit
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);

        // enable ITERREN control bit
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);
    }
    return busystate;
}
uint8_t I2C_MasterRecivedDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t Sr) {
    uint8_t busystate = pI2CHandle->TxRxState;

    if ((busystate != I2C_BUSY_IN_RX) && (busystate != I2C_BUSY_IN_TX)) {
        pI2CHandle->pRxBuffer = pRxBuffer;
        pI2CHandle->RxLen = Len;
        pI2CHandle->RxSize = Len;
        pI2CHandle->TxRxState = I2C_BUSY_IN_RX;
        pI2CHandle->DevAddr = SlaveAddr;
        pI2CHandle->Sr = Sr;

        // Generate start condition
        I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

        // enable ITBUFFEN Control Bit
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITBUFEN);

        // enable ITEVTEN control Bit
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);

        // enable ITERREN control bit
        pI2CHandle->pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);
    }
    return busystate;
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

void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi) {
    if (EnOrDi == ENABLE) {
        if (IRQNumber <= 31) {
            *NVIC_ISER0 |= (1 << IRQNumber);
        } else if (IRQNumber > 31 && IRQNumber < 64) {
            *NVIC_ISER1 |= (1 << (IRQNumber % 32));
        } else if (IRQNumber >= 64 && IRQNumber < 96) {
            *NVIC_ISER2 |= (1 << (IRQNumber % 64));
        }
    } else {
        if (IRQNumber <= 31) {
            *NVIC_ICER0 |= (1 << IRQNumber);
        } else if (IRQNumber > 31 && IRQNumber < 64) {
            *NVIC_ICER1 |= (1 << (IRQNumber % 32));
        } else if (IRQNumber >= 64 && IRQNumber < 96) {
            *NVIC_ICER2 |= (1 << (IRQNumber % 64));
        }
    }
}

void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority) {}

void I2C_CloseRecieveData(I2C_Handle_t *pI2CHandle) {
    // disable ITBUFEN control bit
    pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);

    // disable ITEVTEN control bit
    pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);

    pI2CHandle->TxRxState = I2C_READY;
    pI2CHandle->pRxBuffer = NULL;
    pI2CHandle->RxLen = 0;
    pI2CHandle->RxSize = 0;

    if (pI2CHandle->I2C_Config.I2C_ACKControl == I2C_ACK_ENABLE) {
        I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
    }
}

void I2C_CloseSendData(I2C_Handle_t *pI2CHandle) {
    // disable ITBUFEN control bit
    pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);

    // disable ITEVTEN control bit
    pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);

    pI2CHandle->TxRxState = I2C_READY;
    pI2CHandle->pTxBuffer = NULL;
    pI2CHandle->TxLen = 0;
}

static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle) {
    if (pI2CHandle->TxLen > 0) {
        // load data to the DR
        pI2CHandle->pI2Cx->DR = *(pI2CHandle->pTxBuffer);
        // decrement the TxLen
        pI2CHandle->TxLen--;
        // increment the buffer address
        pI2CHandle->pTxBuffer++;
    }
}

static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle) {
    // data reception
    if (pI2CHandle->RxSize == 1) {
        *pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->DR;
        pI2CHandle->RxLen--;
    }
    if (pI2CHandle->RxSize > 1) {
        if (pI2CHandle->RxLen == 2) {
            // disable ack
            I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);
        }

        // read DR
        *pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->DR;
        pI2CHandle->pRxBuffer++;
        pI2CHandle->RxLen--;
    }

    if (pI2CHandle->RxLen == 0) {
        // close the i2c reception and notify the application

        // generate the stop condition
        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

        // close the I2C RX
        I2C_CloseRecieveData(pI2CHandle);
        // notif ythe application
        I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_RX_CMPLT);
    }
}

void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle) {

    // Interrupt handling for both master and slave mode of a device

    uint32_t temp1, temp2, temp3;

    temp1 = pI2CHandle->pI2Cx->CR2 & (1 << I2C_CR2_ITEVTEN);
    temp2 = pI2CHandle->pI2Cx->CR2 & (1 << I2C_CR2_ITBUFEN);

    temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_SB);

    // 1. Handle For interrupt generated by SB event
    //  Note : SB flag is only applicable in Master mode

    if (temp1 && temp3) {
        // SB event
        // will not be executed in slave mode because for slave SB is always
        // zero adress phase
        if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX) {
            I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
        } else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX) {
            I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
        }
    }

    // 2. Handle For interrupt generated by ADDR event
    // Note : When master mode : Address is sent
    // When Slave mode : Address matched with own address
    temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_ADDR);
    if (temp1 && temp3) {
        // ADDR flag is set
        // adress phase
        I2C_ClearAddressFlag(pI2CHandle);
    }

    // 3. Handle For interrupt generated by BTF(Byte Transfer Finished)
    // event
    temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_BTF);
    if (temp1 && temp3) {
        // BTF flag is set
        if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX) {
            // make sure that TXE is also set
            if (pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_TXE)) {
                // TxE,BTF =1
                if (pI2CHandle->TxLen == 0) {
                    // generate stop condition
                    if (pI2CHandle->Sr == I2C_DISABLE_SR) {
                        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
                    }

                    // reset all member elements of the handle structure
                    I2C_CloseSendData(pI2CHandle);
                    // notify the application about transmission comlete
                    I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMPLT);
                }
            }

        } else if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX) {
            ;
        }
    }

    // 4. Handle For interrupt generated by STOPF event
    //  Note : Stop detection flag is applicable only slave mode . For
    //  master this flag wi
    temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_STOPF);
    if (temp1 && temp3) {
        // STOPF flag is set
        // clear the stopf (read SR1 and write in the CR1)
        pI2CHandle->pI2Cx->CR1 |= 0x0000;
        // notify the application that STOP is detected
        I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_STOP);
    }

    // 5. Handle For interrupt generated by TXE event
    temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_TXE);
    if (temp1 && temp2 && temp3) {
        // check for device mode
        if (pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_MSL)) {
            // TXE flag is set
            if (pI2CHandle->TxRxState == I2C_BUSY_IN_TX) {
                I2C_MasterHandleTXEInterrupt(pI2CHandle);
            }
        } else {
            // slave
            // make sure that the transmitter is really in transmitter mode
            if (pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_TRA)) {
                I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_REQ);
            }
        }
    }

    // 6. Handle For interrupt generated by RXNE event
    temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_RXNE);
    if (temp1 && temp2 && temp3) {
        // check for device mode
        if (pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_MSL)) {
            // master mode

            // RXNE flag is set
            if (pI2CHandle->TxRxState == I2C_BUSY_IN_RX) {
                I2C_MasterHandleRXNEInterrupt(pI2CHandle);
            }
        } else {
            // slave mode
            if (pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_TRA)) {
                I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_RCV);
            }
        }
    }
}

void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle) {
    uint32_t temp1, temp2;

    // status of the ITERREN ctrl bit in the CR2
    temp2 = pI2CHandle->pI2Cx->CR2 & (1 << I2C_CR2_ITERREN);

    // bus error
    temp1 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_BERR);
    if (temp1 && temp2) {
        // bus error
        // clear the bus error flag
        pI2CHandle->pI2Cx->SR1 &= ~(1 << I2C_SR1_BERR);
        // notify the application about the error
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_BERR);
    }

    // arbitration lost error
    temp1 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_ARLO);
    if (temp1 && temp2) {
        // arbitration lost error
        // clear the arbitration lost error flag
        pI2CHandle->pI2Cx->SR1 &= ~(1 << I2C_SR1_ARLO);
        // notify the application about the error
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_ARLO);
    }

    // Acknowledge failure error
    temp1 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_AF);
    if (temp1 && temp2) {
        // Acknowledge failure lost error
        // clear the Acknowledge failure lost error flag
        pI2CHandle->pI2Cx->SR1 &= ~(1 << I2C_SR1_AF);
        // notify the application about the error
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_AF);
    }

    // Overrun/Underrun error
    temp1 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_OVR);
    if (temp1 && temp2) {
        // Overrun/Underrun error
        // clear the Overrun/Underrun error flag
        pI2CHandle->pI2Cx->SR1 &= ~(1 << I2C_SR1_OVR);
        // notify the application about the error
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_OVR);
    }

    // Timeout or Tlow error
    temp1 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_TIMEOUT);
    if (temp1 && temp2) {
        // Timeout or Tlow error

        // clear the Timeout or Tlow error flag
        pI2CHandle->pI2Cx->SR1 &= ~(1 << I2C_SR1_TIMEOUT);
        // notify the application about the error
        I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_TIMEOUT);
    }
}

void I2C_SlaveSendData(I2C_RegDef_t *pI2C, uint8_t data) { pI2C->DR = data; }
uint8_t I2C_SlaveRecivedData(I2C_RegDef_t *pI2C) { return (uint8_t)pI2C->DR; }

void I2C_SlaveEnableDisableCallbackEvents(I2C_RegDef_t *pI2Cx, uint8_t EnorDI) {
    if (EnorDI == ENABLE) {
        pI2Cx->CR2 |= (1 << I2C_CR2_ITBUFEN);
        pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);
        pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);
    } else {
        pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);
        pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);
        pI2Cx->CR2 &= ~(1 << I2C_CR2_ITERREN);
    }
}