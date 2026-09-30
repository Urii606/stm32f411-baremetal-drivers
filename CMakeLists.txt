cmake_minimum_required(VERSION 3.20)

project(stm32_drivers C ASM)

# Створюємо статичну бібліотеку з правильними шляхами до джерел
add_library(stm32_drivers STATIC
    drivers/Src/stm32f411xx_gpio_driver.c
    drivers/Src/stm32f411xx_spi_driver.c
    drivers/Src/stm32f411xx_i2c_driver.c
    drivers/Src/stm32f411xx_usart_driver.c
    drivers/Src/stm32f411xx_rcc_driver.c
    drivers/Src/syscall.c
    drivers/Src/sysmem.c
)

# Експортуємо шляхи до заголовків (Inc)
target_include_directories(stm32_drivers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/Inc
)

# Визначення мікроконтролера
target_compile_definitions(stm32_drivers PRIVATE
    STM32F411xE
)