#include "stm32f411xx_usart_driver.h"
#include "stm32f411xx.h"
#include <stdint.h>

static void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t BaudRate);

/******************************************************************************************
 *								APIs supported by this driver
 *		 For more information about the APIs check the function definitions
 ******************************************************************************************/

/*
 * Peripheral Clock setup
 */
void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t EnOrDi) {
    if (EnOrDi == ENABLE) {
        if (pUSARTx == USART1) {
            USART1_PCLK_EN();
        } else if (pUSARTx == USART2) {
            USART2_PCLK_EN();
        } else if (pUSARTx == USART6) {
            USART6_PCLK_EN();
        }
    } else {
        if (pUSARTx == USART1) {
            USART1_PCLK_DI();
        } else if (pUSARTx == USART2) {
            USART2_PCLK_DI();
        } else if (pUSARTx == USART6) {
            USART6_PCLK_DI();
        }
    }
}

/*
 * Init and De-init
 */
void USART_Init(USART_Handle_t *pUSARTHandle) {
    uint32_t tempreg = 0;
    /***************************Configuration CR1********************* */
    // mode
    if (pUSARTHandle->USART_Config.USART_Mode == USART_MODE_ONLY_RX) {
        tempreg |= (1 << USART_CR1_RE);
    } else if (pUSARTHandle->USART_Config.USART_Mode == USART_MODE_ONLY_TX) {
        tempreg |= (1 << USART_CR1_TE);
    } else if (pUSARTHandle->USART_Config.USART_Mode == USART_MODE_TXRX) {
        tempreg |= ((1 << USART_CR1_RE) | (1 << USART_CR1_TE));
    }

    tempreg |= pUSARTHandle->USART_Config.USART_WordLength << USART_CR1_M;

    if (pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_EN_EVEN) {
        tempreg |= (1 << USART_CR1_PCE);
    } else if (pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_EN_ODD) {
        tempreg |= (1 << USART_CR1_PCE) | (1 << USART_CR1_PS);
    }

    pUSARTHandle->pUSARTx->CR1 |= tempreg;

    /***************************Configuration CR2*********************/
    tempreg = 0;

    tempreg |= pUSARTHandle->USART_Config.USART_NoOfStopBits << USART_CR2_STOP;

    pUSARTHandle->pUSARTx->CR2 |= tempreg;

    /***************************Configuration CR3*********************/
    tempreg = 0;
    if (pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_CTS) {
        tempreg |= (1 << USART_CR3_CTSE);
    } else if (pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_RTS) {
        tempreg |= (1 << USART_CR3_RTSE);
    } else if (pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_CTS_RTS) {
        tempreg |= ((1 << USART_CR3_CTSE) | (1 << USART_CR3_RTSE));
    }
    pUSARTHandle->pUSARTx->CR3 = tempreg;

    /******************************** Configuration of BRR(Baudrate
     * register)******************************************/
    USART_SetBaudRate(pUSARTHandle->pUSARTx, pUSARTHandle->USART_Config.USART_Baud);
}
void USART_DeInit(USART_RegDef_t *pUSARTx) {
    if (pUSARTx == USART1) {
        USART1_REG_RESET();
    } else if (pUSARTx == USART2) {
        USART2_REG_RESET();
    } else if (pUSARTx == USART6) {
        USART6_REG_RESET();
    }
}

/*
 * Data Send and Receive
 */
void USART_SendData(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t Len) {
    uint16_t *pdata;

    // Loop over until "Len" number of bytes are transferred
    for (uint32_t i = 0; i < Len; i++) {
        // Implement the code to wait until TXE flag is set in the SR
        while (USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_FLAG_TXE) == FLAG_RESET)
            ;

        // Check the USART_WordLength item for 9BIT or 8BIT in a frame
        if (pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS) {
            // if 9BIT load the DR with 2bytes masking  the bits other than first 9 bits
            pdata = (uint16_t *)pTxBuffer;
            pUSARTHandle->pUSARTx->DR = (*pdata & 0x01FF);

            // Check for paruty control
            if (pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE) {
                // No parity is used in this transfer , so 9bits of user data will be sent
                // Implement the code to increment pTxBuffer twice
                pTxBuffer++;
                pTxBuffer++;
            } else {
                // Parity bit is used in this transfer . so 8bits of user data will be sent
                // The 9th bit will be replaced by parity bit by the hardware
                pTxBuffer++;
            }
        } else {
            pUSARTHandle->pUSARTx->DR = (*pTxBuffer & (uint8_t)0x0FF);
            pTxBuffer++;
        }
    }
    while (!USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_FLAG_TC))
        ;
}

void USART_ReceiveData(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Len) {
    // Loop over until "Len" number of bytes are transferred
    for (uint32_t i = 0; i < Len; i++) {
        // Implement the code to wait until RXNE flag is set in the SR
        while (USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_FLAG_RXNE) == FLAG_RESET)
            ;
        // Check the USART_WordLength to decide whether we are going to receive 9bit of data in a frame or 8 bit
        if (pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS) {
            // We are going to receive 9bit data in a frame
            // Now, check are we using USART_ParityControl control or not
            if (pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE) {
                *pRxBuffer = (pUSARTHandle->pUSARTx->DR & 0x01FF);

                pRxBuffer++;
                pRxBuffer++;
            } else {
                {
                    *pRxBuffer = (pUSARTHandle->pUSARTx->DR & 0x0FF);
                }
            }
        } else {
            // We are going to receive 8bit data in a frame

            // Now, check are we using USART_ParityControl control or not
            if (pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE) {
                // No parity is used , so all 8bits will be of user data

                // read 8 bits from DR
                *pRxBuffer = (pUSARTHandle->pUSARTx->DR & 0x0FF);
                pRxBuffer++;
            } else {
                {
                    // Parity is used, so , 7 bits will be of user data and 1 bit is parity

                    // read only 7 bits , hence mask the DR with 0X7F
                    *pRxBuffer = (pUSARTHandle->pUSARTx->DR & 0x07F);
                    pRxBuffer++;
                }
            }
        }
    }
}
uint8_t USART_SendDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pTxBuffer, uint32_t Len) {
    uint8_t txstate = pUSARTHandle->TxBusyState;

    if (txstate != USART_BUSY_IN_TX) {
        pUSARTHandle->TxLen = Len;
        pUSARTHandle->pTxBuffer = pTxBuffer;
        pUSARTHandle->TxBusyState = USART_BUSY_IN_TX;

        pUSARTHandle->pUSARTx->CR1 |= (1 << USART_CR1_TXEIE);
    }
    return txstate;
}
uint8_t USART_ReceiveDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Len) {
    uint8_t rxstate = pUSARTHandle->RxBusyState;
    if (rxstate != USART_BUSY_IN_RX) {
        pUSARTHandle->RxLen = Len;
        pUSARTHandle->pRxBuffer = pRxBuffer;
        pUSARTHandle->RxBusyState = USART_BUSY_IN_RX;

        pUSARTHandle->pUSARTx->CR1 |= (1 << USART_CR1_RXNEIE);
    }
    return rxstate;
}

/*
 * IRQ Configuration and ISR handling
 */
void USART_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi) {
    if (EnorDi == ENABLE) {
        if (IRQNumber <= 31) {
            *NVIC_ISER0 |= (1 << IRQNumber);
        } else if (IRQNumber >= 32 && IRQNumber < 64) {
            *NVIC_ISER1 |= (1 << (IRQNumber % 32));
        } else if (IRQNumber >= 64 && IRQNumber < 96) {
            *NVIC_ISER2 |= (1 << (IRQNumber % 32));
        }
    } else {
        {
            if (IRQNumber <= 31) {
                *NVIC_ICER0 |= (1 << IRQNumber);
            } else if (IRQNumber >= 32 && IRQNumber < 64) {
                *NVIC_ICER1 |= (1 << (IRQNumber % 32));
            } else if (IRQNumber >= 64 && IRQNumber < 96) {
                *NVIC_ICER2 |= (1 << (IRQNumber % 32));
            }
        }
    }
}
void USART_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority) {
    // 1. first lets find out the ipr register
    uint8_t iprx = IRQNumber / 4;         // find nuber of right register
    uint8_t iprx_section = IRQNumber % 4; // defines section

    uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);

    *(NVIC_PR_BASE_ADDR + iprx) &= ~(IRQPriority << shift_amount);
    *(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);
}
void USART_IRQHandling(USART_Handle_t *pUSARTHandle) {
    uint32_t temp1, temp2, temp3;

    uint16_t *pdata;

    /*************************Check for TC flag ********************************************/
    // Implement the code to check the state of TC bit in the SR
    temp1 = pUSARTHandle->pUSARTx->SR & (1 << USART_SR_TC);

    // Implement the code to check the state of TCEIE bit
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1 << USART_CR1_TCIE);

    if (temp1 && temp2) {
        // this interrupt is because of TC

        // close transmission and call application callback if TxLen is zero
        if (pUSARTHandle->TxBusyState == USART_BUSY_IN_TX) {
            // Check the TxLen . If it is zero then close the data transmission
            if (pUSARTHandle->TxLen == 0) {
                // Implement the code to clear the TC flag
                pUSARTHandle->pUSARTx->SR &= ~(1 << USART_SR_TC);
                // Implement the code to clear the TCIE control bit
                pUSARTHandle->pUSARTx->CR1 &= ~(1 << USART_CR1_TCIE);
                // Reset the application state
                pUSARTHandle->TxBusyState = USART_READY;
                // Reset Buffer address to NULL
                pUSARTHandle->pTxBuffer = NULL;
                // Reset the length to zero
                pUSARTHandle->TxLen = 0;
                // Call the application call back with event USART_EVENT_TX_CMPLT
                USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_TX_CMPLT);
            }
        }
    }

    /*************************Check for TXE flag ********************************************/

    temp1 = pUSARTHandle->pUSARTx->SR & (1 << USART_SR_TXE);

    temp2 = pUSARTHandle->pUSARTx->CR1 & (1 << USART_CR1_TXEIE);

    if (temp1 && temp2) {
        if (pUSARTHandle->TxBusyState == USART_BUSY_IN_TX) {
            if (pUSARTHandle->TxLen > 0) {
                if (pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS) {
                    pdata = (uint16_t*)pUSARTHandle->pTxBuffer;
                    pUSARTHandle->pUSARTx->DR = (*pdata & 0x01FF);

                    if (pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE) {
                        pUSARTHandle->pTxBuffer++;
                        pUSARTHandle->pTxBuffer++;
                        pUSARTHandle->TxLen -= 2;
                    } else {
                        pUSARTHandle->pTxBuffer++;
                        pUSARTHandle->TxLen--;
                    }
                } else {
                    // 8 bit transfer
                    pUSARTHandle->pUSARTx->DR = (*pUSARTHandle->pTxBuffer & 0x0FF);
                    pUSARTHandle->pTxBuffer++;
                    pUSARTHandle->TxLen--;
                }
            }
            if (pUSARTHandle->TxLen == 0) {
                pUSARTHandle->pUSARTx->CR1 &= ~(1 << USART_CR1_TXEIE);
            }
        }
    }

    /*************************Check for RXNE flag ********************************************/
    temp1 = pUSARTHandle->pUSARTx->SR & (1 << USART_SR_RXNE);
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1 << USART_CR1_RXNEIE);

    if (temp1 && temp2) {
        if (pUSARTHandle->RxBusyState == USART_BUSY_IN_RX) {
            if (pUSARTHandle->RxLen > 0) {
                if (pUSARTHandle->USART_Config.USART_WordLength == USART_WORDLEN_9BITS) {
                    if (pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE) {
                        *(pUSARTHandle->pRxBuffer) = (pUSARTHandle->pUSARTx->DR & 0x01FF);

                        pUSARTHandle->pRxBuffer += 2;
                        pUSARTHandle->RxLen -= 2;
                    } else {
                        *(pUSARTHandle->pRxBuffer) = (pUSARTHandle->pUSARTx->DR & 0x0FF);

                        pUSARTHandle->pRxBuffer++;
                        pUSARTHandle->RxLen--;
                    }
                } else {
                    // 8 bit receive
                    if (pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE) {
                        *pUSARTHandle->pRxBuffer = (pUSARTHandle->pUSARTx->DR & 0x0FF);
                    } else {
                        *pUSARTHandle->pRxBuffer = (pUSARTHandle->pUSARTx->DR & 0x07F);
                    }
                    pUSARTHandle->pRxBuffer++;
                    pUSARTHandle->RxLen--;
                }
            }
            if (pUSARTHandle->RxLen == 0) {
                pUSARTHandle->pUSARTx->CR1 &= ~(1 << USART_CR1_RXNEIE);
                pUSARTHandle->RxBusyState = USART_READY;
                USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_RX_CMPLT);
            }
        }
    }

    /*************************Check for CTS flag ********************************************/
    // Note : CTS feature is not applicable for UART4 and UART5

    // Implement the code to check the status of CTS bit in the SR
    temp1 = pUSARTHandle->pUSARTx->SR & (1 << USART_SR_CTS);

    temp2 = pUSARTHandle->pUSARTx->CR3 & (1 << USART_CR3_CTSE);
    temp3 = pUSARTHandle->pUSARTx->CR3 & (1 << USART_CR3_CTSIE);

    if (temp1 && temp2 && temp3) {
        pUSARTHandle->pUSARTx->SR &= ~(1 << USART_SR_CTS);
        USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_CTS);
    }

    /*************************Check for IDLE detection flag ********************************************/
    temp1 = pUSARTHandle->pUSARTx->SR & (1 << USART_SR_IDLE);
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1 << USART_CR1_IDLEIE);

    if (temp1 && temp2) {
        pUSARTHandle->pUSARTx->SR &= ~(1 << USART_SR_IDLE);
        USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_IDLE);
    }

    /*************************Check for Overrun detection flag ********************************************/
    temp1 = pUSARTHandle->pUSARTx->SR & (1 << USART_SR_ORE);
    temp2 = pUSARTHandle->pUSARTx->CR1 & (1 << USART_CR1_RXNEIE);

    if (temp1 && temp2) {
        pUSARTHandle->pUSARTx->SR &= ~(1 << USART_SR_ORE);
        USART_ApplicationEventCallback(pUSARTHandle, USART_ERR_ORE);
    }

    /*************************Check for Error Flag ********************************************/

    // Noise Flag, Overrun error and Framing Error in multibuffer communication
    // We dont discuss multibuffer communication in this course. please refer to the RM
    // The blow code will get executed in only if multibuffer mode is used.

    temp2 = pUSARTHandle->pUSARTx->CR3 & (1 << USART_CR3_EIE);

    if (temp2) {
        temp1 = pUSARTHandle->pUSARTx->SR;
        if (temp1 & (1 << USART_SR_FE)) {
            USART_ApplicationEventCallback(pUSARTHandle, USART_ERR_FE);
        }

        if (temp1 & (1 << USART_SR_NE)) {
            USART_ApplicationEventCallback(pUSARTHandle, USART_ERR_NE);
        }
        if (temp1 & (1 << USART_SR_ORE)) {
            USART_ApplicationEventCallback(pUSARTHandle, USART_ERR_ORE);
        }
    }
}

/*
 * Other Peripheral Control APIs
 */

uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSARTx, uint8_t StatusFlagName) {
    if (pUSARTx->SR & StatusFlagName) {
        return SET;
    }
    return RESET;
}

void USART_ClearFlag(USART_RegDef_t *pUSARTx, uint16_t StatusFlagName) {
    if (StatusFlagName & (USART_FLAG_PE | USART_FLAG_FE | USART_FLAG_NE | USART_FLAG_ORE | USART_FLAG_IDLE)) {
        (void)pUSARTx->SR;
        (void)pUSARTx->DR;
    }
    if (StatusFlagName & (USART_FLAG_TC | USART_FLAG_LBD | USART_FLAG_CTS)) {
        uint32_t clear_mask = StatusFlagName & (USART_FLAG_TC | USART_FLAG_LBD | USART_FLAG_CTS);
        pUSARTx->SR = (uint32_t)~clear_mask;
    }
}

void USART_PeripheralControl(USART_RegDef_t *pUSARTx, uint8_t EnOrDi) {
    if (EnOrDi == ENABLE) {
        pUSARTx->CR1 |= (1 << USART_CR1_UE);
    } else {
        {
            pUSARTx->CR1 &= ~(1 << USART_CR1_UE);
        }
    }
}

static void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t BaudRate) {
    // Variable to hold the APB clock
    uint32_t PCLKx;

    uint32_t usartdiv;

    // variables to hold Mantissa and Fraction values
    uint32_t M_part, F_part;

    uint32_t tempreg = 0;

    // Get the value of APB bus clock in to the variable PCLKx
    if ((pUSARTx == USART1) || (pUSARTx == USART6)) {
        // USART1 and USART6 are hanging on APB2 bus
        PCLKx = RCC_GetPCLK2Value();
    } else {
        PCLKx = RCC_GetPCLK1Value();
    }

    // Check for OVER8 configuration bit
    if (pUSARTx->CR1 & (1 << USART_CR1_OVER8)) {
        // OVER8 = 1 , over sampling by 8
        usartdiv = 25 * PCLKx / (2 * BaudRate);
    } else {
        // over sampling by 16
        usartdiv = 25 * PCLKx / (4 * BaudRate);
    }

    // Calculate the Mantissa part
    M_part = usartdiv / 100;

    // Place the Mantissa part in appropriate bit position . refer USART_BRR
    tempreg |= M_part << 4;

    // Extract the fraction part
    F_part = (usartdiv - (M_part * 100));

    // Calculate the final fractional
    if (pUSARTx->CR1 & (1 << USART_CR1_OVER8)) {
        // OVER8 = 1 , over sampling by 8
        F_part = (((F_part * 8) + 50) / 100) & 0x07;
    } else {
        {
            // over sampling by 16
            F_part = (((F_part * 16) + 50) / 100) & ((uint8_t)0x0F);
        }
    }
    // Place the fractional part in appropriate bit position . refer USART_BRR
    tempreg |= F_part;

    // copy the value of tempreg in to BRR register
    pUSARTx->BRR = tempreg;
}

/*
 * Application Callbacks
 */
void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle, uint8_t ApEv);