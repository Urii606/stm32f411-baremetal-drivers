/*
 When button on the STM32 board (master) is pressed, master should send
data to the Arduino board (slave). The data received by the Arduino board
will be displayed on the serial monitor terminal of the Arduino IDE
*/

#include "stm32f411xx.h"
#include "stm32f411xx_gpio_driver.h"
#include "stm32f411xx_i2c_driver.h"
#include <stdint.h>

#define SLAVE_ADDR 0x68

void delay(void) {
    for (uint32_t i = 0; i < 500000; i++)
        ;
}

uint8_t *data = "chinazis";
int main(void) {
    GPIO_Handle_t GpioBtn;
    GpioBtn.pGPIOx = GPIOC;
    GpioBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    GpioBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_INPUT;
    GpioBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;

    GPIO_PeripheralClockControl(GpioBtn.pGPIOx, ENABLE);
    GPIO_Init(&GpioBtn);

    GPIO_Handle_t SCL, SDA;
    SCL.pGPIOx = GPIOB;
    SCL.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    SCL.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
    SCL.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    SCL.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
    SCL.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    SCL.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;

    SDA.pGPIOx = GPIOB;
    SDA.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    SDA.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
    SDA.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
    SDA.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    SDA.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    SDA.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;

    GPIO_PeripheralClockControl(SCL.pGPIOx, ENABLE);
    GPIO_PeripheralClockControl(SDA.pGPIOx, ENABLE);

    GPIO_Init(&SCL);
    GPIO_Init(&SDA);

    I2C_Handle_t I2CComun;
    I2CComun.pI2Cx = I2C1;
    I2CComun.I2C_Config.I2C_ACKControl = I2C_ACK_ENABLE;
    I2CComun.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;
    I2CComun.I2C_Config.I2C_DeviceAddress = 0x61;
    I2CComun.I2C_Config.I2C_FMDutyCycle = 0;

    I2C_PeripheralClockControl(I2CComun.pI2Cx, ENABLE);
    I2C_Init(&I2CComun);

    while (1) {
        if (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == 0) {
            delay();
            I2C_MastersendData(&I2CComun, data, sizeof(data), SLAVE_ADDR);
            while (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == 0)
                ;
            delay();
        }
    }

    return 0;
}
