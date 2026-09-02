/*
 * stm32f411xx_spi_driver.c
 *
 *  Created on: 16 лип. 2026 р.
 *      Author: user
 */

#include "stm32f411xx_spi_driver.h"
#include <stddef.h>

//static f is as private helper function
static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle);

/*
 * Peripheral Clock setup
 */
/************************************************************
 * @fn              - SPI_PeripheralClockControl
 *
 * @brief           -
 *
 * @param[in]       -
 * @param[in]       -
 *
 * @return          -
 *
 * @Note            - None
 */
void SPI_PeripheralClockControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		if(pSPIx == SPI1)
		{
			SPI1_PCLK_EN();
		}else if(pSPIx == SPI2)
		{
			SPI2_PCLK_EN();
		}else if(pSPIx == SPI3)
		{
			SPI3_PCLK_EN();
		}

	}else{
		if(pSPIx == SPI1)
		{
			SPI1_PCLK_DI();
		}else if(pSPIx == SPI2)
		{
			SPI2_PCLK_DI();
		}else if(pSPIx == SPI3)
		{
			SPI3_PCLK_DI();
		}
	}
}

/*
 * Init and De-init
 */
//function(handle_structure)
void SPI_Init(SPI_Handle_t *pSPIHandle)
{
	//configure the SPI_CR! register
	uint32_t tempreg = 0;

	//peripheral clock enable
	SPI_PeripheralClockControl(pSPIHandle->pSPIx, ENABLE);

		//device mode master/slave
	tempreg |= (pSPIHandle->SPIConfig.SPI_DeviceMode << SPI_CR1_MSTR_Pos);

		// bus configuration full-duplex/half-duplex/simplex
	if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_HD)
	{	//half-duplex
		//bidi mode should be set
		tempreg |= (1 << SPI_CR1_BIDIMODE_Pos);
	}else if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_FD)
	{	//full-duplex
		//bidi mode should be cleared
		tempreg &= ~(1 << SPI_CR1_BIDIMODE_Pos);
	}else if(pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_SIMPLEX_RXONLY)
	{
		//simplex
		//bidi moode should be cleared
		//RXONLY bit must be set
		tempreg &= ~(1<<SPI_CR1_BIDIMODE_Pos);
		tempreg |= (1<<SPI_CR1_RXONLY_Pos);
	}

	tempreg |= (pSPIHandle->SPIConfig.SPI_SclkSpeed << SPI_CR1_BR_Pos);
	tempreg |= (pSPIHandle->SPIConfig.SPI_DFF << SPI_CR1_DFF_Pos);
	tempreg |= (pSPIHandle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL_Pos);
	tempreg |= (pSPIHandle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA_Pos);
	tempreg |= (pSPIHandle->SPIConfig.SPI_SSM << SPI_CR1_SSM_Pos);

	pSPIHandle->pSPIx->CR1 = tempreg;
}
void SPI_DeInit(SPI_RegDef_t *pSPIx )
{
	if(pSPIx == SPI1)
	{
		SPI1_REG_RESET();
	}else if(pSPIx == SPI2)
	{
		SPI2_REG_RESET();
	}else if(pSPIx == SPI3)
	{
		SPI3_REG_RESET();
	}
}

uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName)
{
	if(pSPIx->SR & FlagName)
	{
		return FLAG_SET;
	}
	return FLAG_RESET;
}

/*
 * Data Send and Receive
 */
/************************************************************
 * @fn              - SPI_SendData
 *
 * @brief           -
 *
 * @param[in]       -base adderss of the SPI
 * @param[in]       -pointer to the data
 * @param[in]       -data length
 *
 * @return          -
 *
 * @Note            - this is blocking call
 */
void SPI_SendData(SPI_RegDef_t *pSPIx,uint8_t *pTxBuffer,uint32_t Len)
{
	while(Len > 0)
	{
		//wait until TXE is set
		while(SPI_GetFlagStatus(pSPIx,SPI_TXE_FLAG) == FLAG_RESET);
		//check the DFF bit in CR1
		if(pSPIx->CR1 & (1 << SPI_CR1_DFF_Pos))
		{
			//16 bit DFF
			//1. load the data in to the DR
			pSPIx->DR = *((uint16_t*)pTxBuffer);
			Len--;
			Len--;
			(uint16_t*)pTxBuffer++;
		}else{
			//8 bit DFF
			pSPIx->DR = *pTxBuffer;
			Len--;
			pTxBuffer++;
		}
	}
}
void SPI_ReceiveData(SPI_RegDef_t *pSPIx,uint8_t *pRxBuffer,uint32_t Len)
{
	while(Len > 0)
	{
		while(SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET);

		if(pSPIx->CR1 & (1 << SPI_CR1_DFF_Pos))
		{
			*((uint16_t*)pRxBuffer) = pSPIx->DR;
			Len--;
			Len--;
			(uint16_t*)pRxBuffer++;
		}else{
			//8 bit DFF
			*pRxBuffer = pSPIx->DR;
			Len--;
			pRxBuffer++;
		}
	}
}

//send data with interrupt mode
uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle,uint8_t *pTxBuffer,uint32_t Len)
{
	uint8_t state = pSPIHandle->TxState;
	if(state != SPI_BUSY_IN_TX)
	{
		//1 . Save the Tx buffer address and Len information in some global variables
		pSPIHandle->pTxBuffer = pTxBuffer;
		pSPIHandle->TxLen = Len;

		//2. Mark the SPI state as busy in transmission so that
		//no other code can take over same SPI peripheral until transmission is over
		pSPIHandle->TxState = SPI_BUSY_IN_TX;

		//3. Enable the TXEIE control bit to get interrupt whenever TXE flag is set in SR
		pSPIHandle->pSPIx->CR2 |= (1<<SPI_CR2_TXEIE);


		}
	//4. Data Transmission will be handled by the ISR code ( will implement later)
	return state;
}


uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle,uint8_t *pRxBuffer,uint32_t Len)
{
	uint8_t state = pSPIHandle->RxState;
	if(state != SPI_BUSY_IN_RX)
	{
		//1 . Save the Rx buffer address and Len information in some global variables
		pSPIHandle->pRxBuffer = pRxBuffer;
		pSPIHandle->RxLen = Len;

		//2. Mark the SPI state as busy in transmission so that
		//no other code can take over same SPI peripheral until transmission is over
		pSPIHandle->RxState = SPI_BUSY_IN_RX;

		//3. Enable the  RXNEIE control bit to get interrupt whenever RXNE flag is set in SR
		pSPIHandle->pSPIx->CR2 |= (1<<SPI_CR2_RXNEIE);
		}
	//4. Data Transmission will be handled by the ISR code ( will implement later)
	return state;
}

/*
 * IRQ Configuration and ISR Handling
 */

/************************************************************
 * @fn              - SPI_IRQConfig
 *
 * @brief           - Configures the interrupt for a specific SPI pin
 *
 * @param[in]       - IRQ number
 * @param[in]       - IRQ priority level
 * @param[in]       - ENABLE or DISABLE macro
 *
 * @return          - None
 *
 * @Note            - None
 */
void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		if(IRQNumber <=31)
		{
			//program ISER0 register
			*NVIC_ISER0 |= (1<< IRQNumber);
		}else if(IRQNumber > 31 && IRQNumber <=64)//31 to 64
		{
			//program ISER1 register
			*NVIC_ISER1 |= (1<< IRQNumber % 32);
		}else if(IRQNumber > 64 && IRQNumber<96)//64 to 95
		{
			//program ISER2 register
			*NVIC_ISER2 |= (1<< IRQNumber % 64);
		}
	}else
	{
		if(IRQNumber <=31)
		{
			//program ICER0 register
			*NVIC_ICER0 |= (1<< IRQNumber);
		}else if(IRQNumber > 31 && IRQNumber <=64)//31 to 64
		{
			//program ICER1 register
			*NVIC_ICER1 |= (1<< IRQNumber % 32);
		}else if(IRQNumber > 64 && IRQNumber<96)//64 to 95
		{
			//program ICER2 register
			*NVIC_ICER2 |= (1<< IRQNumber % 64);
		}
	}
}


/************************************************************
 * @fn              - SPI_IRQPriorityConfig
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
void SPI_IRQPriorityConfig(uint8_t IRQNumber,uint32_t IRQPriority)
{
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;

	uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);
	*(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority <<shift_amount);
}

/************************************************************
 * @fn              - SPI_IRQHandling
 *
 * @brief           - Handles the interrupt service routine (ISR) for a triggered pin
 *
 * @param[in]       - Pin number that triggered the interrupt
 *
 * @return          - None
 *
 * @Note            - None
 */
void SPI_IRQHandling(SPI_Handle_t *pHandle)
{
	uint8_t temp1,temp2;
	//1. checks where the interrupt happened
	//first check for TXE
	temp1 = pHandle->pSPIx->SR & (1 << SPI_SR_TXE);
	temp2 = pHandle->pSPIx->CR2 & (1 << SPI_CR2_TXEIE);
	if(temp1 && temp2)
	{
		//handle TXE
		spi_txe_interrupt_handle(pHandle);
	}

	// check for RXNE
	temp1 = pHandle->pSPIx->SR & (1 << SPI_SR_RXNE);
	temp2 = pHandle->pSPIx->CR2 & (1 << SPI_CR2_RXNEIE);
	if(temp1 && temp2)
	{
		//handle RXNE
		spi_rxne_interrupt_handle(pHandle);
	}

	// check for ovr flag
	temp1 = pHandle->pSPIx->SR & (1 << SPI_SR_OVR);
	temp2 = pHandle->pSPIx->CR2 & (1 << SPI_CR2_ERRIE);
	if(temp1 && temp2)
	{
		//handle RXNE
		spi_ovr_err_interrupt_handle(pHandle);
	}
}



/************************************************************
 * @fn              - SPI_PeripheralControl
 *
 * @brief           -
 *
 * @param[in]       -base adderss of the SPI
 * @param[in]       -pointer to the data
 * @param[in]       -data length
 *
 * @return          -
 *
 * @Note            - this is blocking call
 */
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx,uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		pSPIx->CR1 |= (1 << SPI_CR1_SPE_Pos);
	}else
	{
		pSPIx->CR1 &= ~(1 << SPI_CR1_SPE_Pos);
	}

}

void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		pSPIx->CR1 |= (1 << SPI_CR1_SSI_Pos); // Встановлюємо 8-й біт (SSI) в 1
	}
	else
	{
		pSPIx->CR1 &= ~(1 << SPI_CR1_SSI_Pos); // Очищуємо 8-й біт в 0
	}
}

void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		pSPIx->CR2 |= (1 << SPI_CR2_SSOE); // Встановлюємо 8-й біт (SSI) в 1
	}
	else
	{
		pSPIx->CR2 &= ~(1 << SPI_CR2_SSOE); // Очищуємо 8-й біт в 0
	}
}


//some helper function implementations
static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	//check the DFF bit in CR1
	if(pSPIHandle->pSPIx->CR1 & (1 << SPI_CR1_DFF_Pos))
	{
		//16 bit DFF
		//1. load the data in to the DR
		pSPIHandle->pSPIx->DR = *((uint16_t*)pSPIHandle->pTxBuffer);
		pSPIHandle->TxLen--;
		pSPIHandle->TxLen--;
		(uint16_t*)pSPIHandle->pTxBuffer++;
	}else{
		//8 bit DFF
		pSPIHandle->pSPIx->DR = *pSPIHandle->pTxBuffer;
		pSPIHandle->TxLen--;
		pSPIHandle->pTxBuffer++;
	}
	if(! pSPIHandle->TxLen)
	{
		//TxLen is zero, so close the spi communication and inform the application that
		//TX is over
		//prevents interrupts from setting up of TXE flag
		SPI_CloseTransmission(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_TX_CMPLT);

	}
}

static void spi_rxne_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	if(pSPIHandle->pSPIx->CR1 & (1 << SPI_CR1_DFF_Pos))
	{
		*((uint16_t*)pSPIHandle->pRxBuffer) = pSPIHandle->pSPIx->DR;
		pSPIHandle->RxLen--;
		pSPIHandle->RxLen--;
		(uint16_t*)pSPIHandle->pRxBuffer++;
	}else{
		//8 bit DFF
		*pSPIHandle->pRxBuffer = pSPIHandle->pSPIx->DR;
		pSPIHandle->RxLen--;
		pSPIHandle->pRxBuffer++;
	}
	if(! pSPIHandle->RxLen)
	{
		SPI_CloseReception(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_RX_CMPLT);
	}
}

static void spi_ovr_err_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	uint8_t temp;
	//clear the ovr flag
	if(pSPIHandle->TxState != SPI_BUSY_IN_TX)
	{
		temp = pSPIHandle->pSPIx->DR;
		temp = pSPIHandle->pSPIx->SR;
	}
	(void)temp;
	//inform the application
	SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_OVR_ERR);
}

void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle)
{
	pSPIHandle->pSPIx->CR2 &= ~(1 << SPI_CR2_TXEIE);
	pSPIHandle->pTxBuffer = NULL;
	pSPIHandle->TxLen = 0;
	pSPIHandle->TxState = SPI_READY;
}

void SPI_CloseReception(SPI_Handle_t *pSPIHandle)
{
	pSPIHandle->pSPIx->CR2 &= ~(1<<SPI_CR2_RXNEIE);
	pSPIHandle->pRxBuffer = NULL;
	pSPIHandle->RxLen = 0;
	pSPIHandle->RxState = SPI_READY;
	SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_RX_CMPLT);
}

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx)
{
	uint8_t temp;
	temp = pSPIx->DR;
	temp = pSPIx->SR;
	(void)temp;

}

__weak void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle,uint8_t AppEv)
{
	//This is a weak implementation. The application may override this function
}
