#include "stm32f411xx_dwt_delay.h"

void Delay_Init(void) {
    /* Enable Debug Trace block to power up DWT peripheral */
    DEMCR &= ~DEMCR_TRCENA;
    DEMCR |= DEMCR_TRCENA;

    /* Enable 32-bit CPU cycle counter */
    DWT_CTRL &= ~DWT_CTRL_CYCCNTENA;
    DWT_CTRL |= DWT_CTRL_CYCCNTENA;
}
void Delay_Us(uint32_t us)
 {
    /* At 16 MHz HSI clock, 1 microsecond equals 16 CPU cycles */
    uint32_t ticks = us * 16;
    uint32_t start = DWT_CYCCNT;
    /* Unsigned 32-bit subtraction correctly handles counter overflow */
    while((DWT_CYCCNT - start) < ticks){}
 }