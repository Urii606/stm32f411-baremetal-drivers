/*
12C Master(STM) and 12C Slave(Arduino) communication .

When button on the master is pressed , master should read and display data from
Arduino Slave connected. First master has to get the length of the data from the
slave to read subsequent data from the slave.
1. Use 12C SCL = 100KHz(Standard mode )
2. Use internal pull resistors for SDA and SCL lines
*/

#include "stm32f411xx.h"
#include "stm32f411xx_gpio_driver.h"
#include "stm32f411xx_i2c_driver.h"
#include <iso646.h>
#include <stdint.h>
#include <string.h>

// Button description
#define BTN_PORT GPIOA
#define BTN_PIN GPIO_PIN_NO_0
#define BTN_PRESSED 0

// I2C pins: SCL-PB6 SDA-PB7
#define I2C_GPIO_PORT GPIOB
#define I2C_SCL_PIN GPIO_PIN_NO_6
#define I2C_SDA_PIN GPIO_PIN_NO_7
#define I2C_AF_MODE 4

#define SLAVE_ADDR 0x68
#define OWN_ADDR SLAVE_ADDR

// Module handles
static I2C_Handle_t s_i2c_handle;

// transmit buffer
uint8_t tx_bfr[32] = "Stm32 slave mode testing";

// private functions prototype
static void delay_approx(uint32_t count);
static void button_init(void);
static void i2c_gpio_init(void);
static void i2c_module_init(void);

int main(void) {
    i2c_gpio_init();

    i2c_module_init();

    I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV, ENABLE);
    I2C_IRQInterruptConfig(IRQ_NO_I2C1_ER, ENABLE);

    I2C_SlaveEnableDisableCallbackEvents(I2C1, ENABLE);

    I2C_ManageAcking(s_i2c_handle.pI2Cx, ENABLE);
    while (1) {
    }
}

void i2c_gpio_init(void) {
    GPIO_Handle_t i2c_pins;
    GPIO_PeripheralClockControl(I2C_GPIO_PORT, ENABLE);

    i2c_pins.pGPIOx = I2C_GPIO_PORT;
    i2c_pins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    i2c_pins.GPIO_PinConfig.GPIO_PinAltFunMode = I2C_AF_MODE;
    i2c_pins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    i2c_pins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
    i2c_pins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;

    i2c_pins.GPIO_PinConfig.GPIO_PinNumber = I2C_SCL_PIN;
    GPIO_Init(&i2c_pins);

    i2c_pins.GPIO_PinConfig.GPIO_PinNumber = I2C_SDA_PIN;
    GPIO_Init(&i2c_pins);
}

void i2c_module_init(void) {
    s_i2c_handle.pI2Cx = I2C1;
    s_i2c_handle.I2C_Config.I2C_ACKControl = I2C_ACK_ENABLE;
    s_i2c_handle.I2C_Config.I2C_DeviceAddress = OWN_ADDR;
    s_i2c_handle.I2C_Config.I2C_FMDutyCycle = 0;
    s_i2c_handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;

    I2C_PeripheralClockControl(s_i2c_handle.pI2Cx, ENABLE);

    I2C_Init(&s_i2c_handle);

    I2C_PeripheralControl(s_i2c_handle.pI2Cx, ENABLE);
}

void I2C1_ER_IRQHandler(void) { I2C_ER_IRQHandling(&s_i2c_handle); }
void I2C1_EV_IRQHandler(void) { I2C_EV_IRQHandling(&s_i2c_handle); }

void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEv) {
    static uint8_t commandCode = 0;
    static uint8_t cnt = 0;
    if (AppEv == I2C_EV_DATA_REQ) {

        // master wants some data. slave has to send it
        if (commandCode == 0x51) {
            // send the length information to the master
            I2C_SlaveSendData(pI2CHandle->pI2Cx, strlen((char *)tx_bfr));
        } else if (commandCode == 0x52) {
            // send the data of tx-buf
            I2C_SlaveSendData(pI2CHandle->pI2Cx, tx_bfr[cnt++]);
        }
    } else if (AppEv == I2C_EV_DATA_RCV) {
        // data is waiting for the slave to read. slave has to read it
        commandCode = I2C_SlaveRecivedData(pI2CHandle->pI2Cx);

    } else if (AppEv == I2C_ERROR_AF) {
        // this happens only during slave transmission
        // master has sent the NACK. slave should understand that master doesnt
        // need more data
        commandCode = 0xff;
        cnt = 0;
    } else if (AppEv == I2C_EV_STOP) {
        // this happens only during slave reception
        // master has ended the I2C communication wit hthe slave
    }
}