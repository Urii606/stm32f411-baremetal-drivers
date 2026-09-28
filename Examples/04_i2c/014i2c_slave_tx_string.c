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
uint8_t tx_bfr[] = "Stm32 slave mode "
                   "testing.................................................."
                   "...........................................";

// private functions prototype
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
    static uint32_t cnt = 0;
    static uint8_t len_cnt = 0;

    if (AppEv == I2C_EV_DATA_REQ) {
        if (commandCode == 0x51) {
            uint32_t total_len = strlen((char *)tx_bfr);

            if (len_cnt < 4) {
                I2C_SlaveSendData(pI2CHandle->pI2Cx, (uint8_t)((total_len >> (len_cnt * 8)) & 0xFF));
                len_cnt++;
            } else {
                // DUMMY BYTE: Запобігає зависанню шини (Clock Stretching),
                // якщо залізо просить байт вже після закінчення нашої довжини
                I2C_SlaveSendData(pI2CHandle->pI2Cx, 0xFF);
            }
        } else if (commandCode == 0x52) {
            uint32_t total_len = strlen((char *)tx_bfr);

            if (cnt < total_len) {
                I2C_SlaveSendData(pI2CHandle->pI2Cx, tx_bfr[cnt++]);
            } else {
                // DUMMY BYTE: Згодовуємо пустишку, щоб STM32 відпустив лінію
                // SCL
                I2C_SlaveSendData(pI2CHandle->pI2Cx, 0xFF);
            }
        }
    } else if (AppEv == I2C_EV_DATA_RCV) {
        commandCode = I2C_SlaveRecivedData(pI2CHandle->pI2Cx);

        // Скидаємо лічильники лише на початку нової транзакції
        if (commandCode == 0x52) {
            cnt = 0;
        } else if (commandCode == 0x51) {
            len_cnt = 0;
        }
    } else if (AppEv == I2C_ERROR_AF) {
        // ЗАЛИШАЄМО ПОРОЖНІМ!
        // Дозволяємо Arduino читати довгий масив шматками по 32 байти.
    } else if (AppEv == I2C_EV_STOP) {
        // Master generated STOP.
    }
}