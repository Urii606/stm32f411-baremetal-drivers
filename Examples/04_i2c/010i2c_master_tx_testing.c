#include "stm32f411xx.h"
#include "stm32f411xx_gpio_driver.h"
#include "stm32f411xx_i2c_driver.h"

#include <stdint.h>
#include <string.h>

/******************* */
/*Hardware definitions*/
/***********************/
#define SLAVE_ADDR 0x68
#define MY_ADDR 0x61

/*Button (Black pill KEY: PA0 tied to GND,active-LOW)*/
#define BTN_PORT GPIOA
#define BTN_PIN GPIO_PIN_NO_0
#define BTN_PRESSED 0
#define BTN_RELEASED 1

/*I2C1 Pins (PB6:SCL,PB7:SDA)*/
#define I2C_GPIO_PORT GPIOB
#define I2C_SCL_PIN GPIO_PIN_NO_6
#define I2C_SDA_PIN GPIO_PIN_NO_7
#define I2C_AF_MODE 4

/*Module handles*/
static I2C_Handle_t s_i2c_handle;
static const uint8_t s_tx_payload[] = "testdata";

/*Private functions prototypes*/
static void delay_approx(uint32_t count);
static void button_init(void);
static void i2c_gpio_init(void);
static void i2c_module_init(void);
static uint8_t btn_is_pressed(void);

/*main routine*/
int main(void) {
    button_init();
    i2c_gpio_init();
    i2c_module_init();

    while (1) {
        if (btn_is_pressed()) {
            I2C_MasterSendData(&s_i2c_handle, (uint8_t *)s_tx_payload,
                               strlen((char *)s_tx_payload), SLAVE_ADDR);

            while (GPIO_ReadFromInputPin(BTN_PORT, BTN_PIN) == BTN_PRESSED) {
                delay_approx(250000);
            }
        }
    }

    return 0;
}

static void delay_approx(uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        __asm volatile("nop");  
    }
}
static void button_init(void) {
    GPIO_Handle_t btn;
    btn.pGPIOx = BTN_PORT;
    btn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_INPUT;
    btn.GPIO_PinConfig.GPIO_PinNumber = BTN_PIN;
    btn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    btn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;

    GPIO_Init(&btn);
    GPIO_PeripheralClockControl(btn.pGPIOx, ENABLE);
}

static void i2c_gpio_init(void) {
    GPIO_Handle_t i2c_pins;
    i2c_pins.pGPIOx = I2C_GPIO_PORT;
    i2c_pins.GPIO_PinConfig.GPIO_PinAltFunMode = I2C_AF_MODE;
    i2c_pins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    i2c_pins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    i2c_pins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_OP_TYPE_OD;

    GPIO_PeripheralClockControl(i2c_pins.pGPIOx, ENABLE);

    // SDA
    i2c_pins.GPIO_PinConfig.GPIO_PinNumber = I2C_SDA_PIN;
    GPIO_Init(&i2c_pins);

    // SCL
    i2c_pins.GPIO_PinConfig.GPIO_PinNumber = I2C_SCL_PIN;
    GPIO_Init(&i2c_pins);
}

static void i2c_module_init(void) {
    s_i2c_handle.pI2Cx = I2C1;
    s_i2c_handle.I2C_Config.I2C_ACKControl = I2C_ACK_ENABLE;
    s_i2c_handle.I2C_Config.I2C_DeviceAddress = MY_ADDR;
    s_i2c_handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;
    s_i2c_handle.I2C_Config.I2C_FMDutyCycle = 0;

    I2C_PeripheralClockControl(s_i2c_handle.pI2Cx, ENABLE);
    I2C_Init(&s_i2c_handle);
    I2C_PeripheralControl(s_i2c_handle.pI2Cx, ENABLE);
}

static uint8_t btn_is_pressed(void) {
    if (GPIO_ReadFromInputPin(BTN_PORT, BTN_PIN) == BTN_PRESSED) {
        delay_approx(250000);
        if (GPIO_ReadFromInputPin(BTN_PORT, BTN_PIN) == BTN_PRESSED) {
            return 1;
        }
    }
    return 0;
}