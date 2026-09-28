#include "stm32f411xx.h"
#include "stm32f411xx_gpio_driver.h"
#include "stm32f411xx_usart_driver.h"
#include <string.h>

void delay_approx(uint32_t count);
void USART2_GPIOInit(void);
void USART2_Init(void);
void GPIO_ButtonInit(void);

char msg[1024] = "UART Tx testing...\n\r";

// 2pins (TX/RX)
// USART2_TX - pa2
// SART2_RX    pa3
// af = 7
USART_Handle_t usart2_handle;

int main(void) {
    GPIO_ButtonInit();
    USART2_GPIOInit();
    USART_PeriClockControl(USART2, ENABLE);
    USART2_Init();

    USART_PeripheralControl(USART2, ENABLE);
    while (1) {
        // wait till button is pressed
        while (GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0) != 0)
            ;

        // to avoid button de-bouncing related issues 200ms of delay
        delay_approx(200);

        USART_SendData(&usart2_handle, (uint8_t *)msg, strlen(msg));

        while (GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0) == 0)
            ;

        delay_approx(50);
    }
    return 0;
}

void USART2_GPIOInit(void) {
    GPIO_Handle_t usart_gpios;
    usart_gpios.pGPIOx = GPIOA;
    usart_gpios.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    usart_gpios.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    usart_gpios.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    usart_gpios.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    usart_gpios.GPIO_PinConfig.GPIO_PinAltFunMode = 7;

    // usart TX
    usart_gpios.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
    GPIO_Init(&usart_gpios);

    // usart RX
    usart_gpios.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
    GPIO_Init(&usart_gpios);
}

void USART2_Init(void) {
    usart2_handle.pUSARTx = USART2;
    usart2_handle.USART_Config.USART_Baud = USART_STD_BAUD_115200;
    usart2_handle.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;
    usart2_handle.USART_Config.USART_Mode = USART_MODE_ONLY_TX;
    usart2_handle.USART_Config.USART_NoOfStopBits = USART_STOPBITS_1;
    usart2_handle.USART_Config.USART_WordLength = USART_WORDLEN_8BITS;
    usart2_handle.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;

    USART_Init(&usart2_handle);
}

void GPIO_ButtonInit(void) {
    GPIO_Handle_t button, led;
    button.pGPIOx = GPIOA;
    button.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_INPUT;
    button.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
    button.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    button.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    GPIO_Init(&button);

    led.pGPIOx = GPIOC;
    led.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    led.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    led.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    led.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
    led.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    GPIO_Init(&led);
}

void delay_approx(uint32_t count) {
    for (uint32_t i = 0; i < (count * 2000); i++) {
        __asm volatile("nop");
    }
}
