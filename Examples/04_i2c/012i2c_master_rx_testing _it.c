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
#include <stdint.h>

//flag variable
uint8_t rxComplt = RESET;

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
#define OWN_ADDR 0x61

    // Module handles
    static I2C_Handle_t s_i2c_handle;

// recive buffer
uint8_t rcv_bfr[32];

// private functions prototype
static void delay_approx(uint32_t count);
static void button_init(void);
static void i2c_gpio_init(void);
static void i2c_module_init(void);
static uint8_t btn_is_pressed(void);

int main(void) {
    uint8_t commandcode, len;

    button_init();
    i2c_gpio_init();
    I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV, ENABLE);
    I2C_IRQInterruptConfig(IRQ_NO_I2C1_ER, ENABLE);

    i2c_module_init();

    I2C_ManageAcking(s_i2c_handle.pI2Cx, ENABLE);
    while (1) {

        if (btn_is_pressed() == 1) {
            while (GPIO_ReadFromInputPin(BTN_PORT, BTN_PIN) == 0)
                ;
            commandcode = 0x51;
            while (I2C_MasterSendDataIT(&s_i2c_handle, &commandcode, 1,
                                        SLAVE_ADDR, I2C_ENABLE_SR) != I2C_READY)
                ;
            while (I2C_MasterRecivedDataIT(&s_i2c_handle, &len, 1, SLAVE_ADDR,
                                           I2C_ENABLE_SR) != I2C_READY)
                ;

            commandcode = 0x52;
            while (I2C_MasterSendDataIT(&s_i2c_handle, &commandcode, 1,
                                        SLAVE_ADDR, I2C_ENABLE_SR))
                ;

            while (I2C_MasterRecivedDataIT(&s_i2c_handle, rcv_bfr, len,
                                           SLAVE_ADDR, I2C_DISABLE_SR))
                ;
                rxComplt = RESET;
                //wait till rx completes
                while(rxComplt != SET){}
rxComplt=RESET;
        }
    }
    return 0;
}

void delay_approx(uint32_t count) {
    for (int i = 0; i < count; i++) {
        __asm volatile("nop");
    }
}

void button_init(void) {
    GPIO_Handle_t btn;
    GPIO_PeripheralClockControl(BTN_PORT, ENABLE);

    btn.pGPIOx = BTN_PORT;
    btn.GPIO_PinConfig.GPIO_PinNumber = BTN_PIN;
    btn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
    btn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_INPUT;
    btn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;

    GPIO_Init(&btn);
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

static uint8_t btn_is_pressed(void) {
    if (GPIO_ReadFromInputPin(BTN_PORT, BTN_PIN) == 0) {
        delay_approx(250000);
        if (GPIO_ReadFromInputPin(BTN_PORT, BTN_PIN) == 0) {
            return 1;
        }
    }
    return 0;
}

void I2C1_ER_IRQHanler(void) { I2C_ER_IRQHandling(&s_i2c_handle); }
void I2C1_EV_IRQHanler(void) { I2C_EV_IRQHandling(&s_i2c_handle); }

void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEv)
{
    if(AppEv == I2C_EV_TX_CMPLT)
    {

    }else if(AppEv == I2C_EV_RX_CMPLT)
    {
        rxComplt=SET;
    }else if(AppEv==I2C_ERROR_AF)
    {
        I2C_CloseRecieveData(pI2CHandle);
        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

        while(1);
    }
}