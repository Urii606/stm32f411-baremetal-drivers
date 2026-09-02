/*
 * stm32f411xx_spi_driver.h
 *
 *  Created on: 16 лип. 2026 р.
 *      Author: user
 */

#ifndef INC_STM32F411XX_SPI_DRIVER_H_
#define INC_STM32F411XX_SPI_DRIVER_H_

#include "stm32f411xx.h"


#define SPI_DEVICE_MODE_MASTER				1
#define SPI_DEVICE_MODE_SLAVE				0

#define SPI_BUS_CONFIG_FD					1
#define SPI_BUS_CONFIG_HD					2
#define SPI_BUS_CONFIG_SIMPLEX_RXONLY		3

#define SPI_SCLK_SPEED_DIV2					0
#define SPI_SCLK_SPEED_DIV4					1
#define SPI_SCLK_SPEED_DIV8					2
#define SPI_SCLK_SPEED_DIV16				3
#define SPI_SCLK_SPEED_DIV32				4
#define SPI_SCLK_SPEED_DIV64				5
#define SPI_SCLK_SPEED_DIV128				6
#define SPI_SCLK_SPEED_DIV256				7

#define SPI_DFF_8BITS						0
#define SPI_DFF_16BITS						1

#define SPI_SSM_EN							1
#define SPI_SSM_DI							0

#define SPI_CPOL_LOW						0
#define SPI_CPOL_HIGH						1

#define SPI_CPHA_LOW						0
#define SPI_CPHA_HIGH						1

/*
 * Bit position definitions SPI_CR1
 */
#define SPI_CR1_CPHA_Pos			0
#define SPI_CR1_CPOL_Pos			1
#define SPI_CR1_MSTR_Pos			2
#define SPI_CR1_BR_Pos				3
#define SPI_CR1_SPE_Pos				6
#define SPI_CR1_LSBFIRST_Pos		7
#define SPI_CR1_SSI_Pos				8
#define SPI_CR1_SSM_Pos				9
#define SPI_CR1_RXONLY_Pos			10
#define SPI_CR1_DFF_Pos				11
#define SPI_CR1_CRCNEXT_Pos			12
#define SPI_CR1_CRCEN_Pos			13
#define SPI_CR1_BIDIOE_Pos			14
#define SPI_CR1_BIDIMODE_Pos		15

/*
 * Bit position definitions SPI_CR2
 */
#define SPI_CR2_TXEIE				7
#define SPI_CR2_RXNEIE				6
#define SPI_CR2_ERRIE				5
#define SPI_CR2_FRF					4
#define SPI_CR2_SSOE				2
#define SPI_CR2_TXDMAEN				1
#define SPI_CR2_RXDMAEN				0

/*
 * Bit position definitions SPI_SR
 */
#define SPI_SR_FRE					8
#define SPI_SR_BSY					7
#define SPI_SR_OVR					6
#define SPI_SR_MODF					5
#define SPI_SR_CRCERR				4
#define SPI_SR_UDR					3
#define SPI_SR_CHSIDE				2
#define SPI_SR_TXE					1
#define SPI_SR_RXNE					0


#define SPI_TXE_FLAG				(1 << SPI_SR_TXE)
#define SPI_RXNE_FLAG				(1 << SPI_SR_RXNE)
#define SPI_CHSIDE_FLAG				(1 << SPI_SR_CHSIDE)
#define SPI_UDR_FLAG				(1 << SPI_SR_UDR)
#define SPI_CRCERR_FLAG				(1 << SPI_SR_CRCERR)
#define SPI_MODF_FLAG				(1 << SPI_SR_MODF)
#define SPI_OVR_FLAG				(1 << SPI_SR_OVR)
#define SPI_BSY_FLAG				(1 << SPI_SR_BSY)
#define SPI_FRE_FLAG				(1 << SPI_SR_FRE)


#define IRQ_NO_EXTI0 				6
#define IRQ_NO_EXTI1 				7
#define IRQ_NO_EXTI2 				8
#define IRQ_NO_EXTI3 				9
#define IRQ_NO_EXTI4 				10
#define IRQ_NO_EXTI9_5			 	23
#define IRQ_NO_EXTI15_10 			40

#define IRQ_NO_SPI2					36

/*
 * SPI application states
 */
#define SPI_READY					0
#define SPI_BUSY_IN_RX				1
#define SPI_BUSY_IN_TX				2

/*
 * Possible SPI Application events
 */
#define SPI_EVENT_TX_CMPLT			1
#define SPI_EVENT_RX_CMPLT			2
#define SPI_EVENT_OVR_ERR			3
#define SPI_EVENT_CRC_ERR			4



/*
 * Configuration structure for SPIx peripheral
 */
typedef struct
{
	uint8_t SPI_DeviceMode;
	uint8_t SPI_BusConfig;
	uint8_t SPI_SclkSpeed;
	uint8_t SPI_DFF;
	uint8_t SPI_CPOL;
	uint8_t SPI_CPHA;
	uint8_t SPI_SSM;
}SPI_Config_t;

/*
 * Handle structure for SPIx peripheral
 */

typedef struct
{
	SPI_RegDef_t *pSPIx;
	SPI_Config_t SPIConfig;
	uint8_t *pTxBuffer;
	uint8_t *pRxBuffer;
	uint32_t TxLen;
	uint32_t RxLen;
	uint8_t TxState;
	uint8_t RxState;
}SPI_Handle_t;



/*****************************************************************************
 * 								APIs supported by this driver
 *****************************************************************************/
/*
 * Peripheral Clock setup
 */
void SPI_PeripheralClockControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);

/*
 * Init and De-init
 */
//function(handle_structure)
void SPI_Init(SPI_Handle_t *pSPIHandle);
void SPI_DeInit(SPI_RegDef_t *pSPIx );

/*
 * Data Send and Receive
 */
void SPI_SendData(SPI_RegDef_t *pSPIx,uint8_t *pTxBuffer,uint32_t Len);
void SPI_ReceiveData(SPI_RegDef_t *pSPIx,uint8_t *pRxBuffer,uint32_t Len);

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle,uint8_t *pTxBuffer,uint32_t Len);
uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle,uint8_t *pRxBuffer,uint32_t Len);

/*
 * IRQ Configuration and ISR handling
 */
void SPI_IRQInterruptConfig(uint8_t IRQNumber ,uint8_t EnOrDi);
void SPI_IRQPriorityConfig(uint8_t IRQNumber,uint32_t IRQPriority);
void SPI_IRQHandling(SPI_Handle_t *pHandle);

/*
 * Other Peripheral Control APIs
 */
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx,uint8_t EnOrDi);
void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi);
uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx,uint32_t FlagName);

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx);
void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle);
void SPI_CloseReception(SPI_Handle_t *pSPIHandle);


/*
 * application callback
 */
void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle,uint8_t AppEv);

#endif /* INC_STM32F411XX_SPI_DRIVER_H_ */
