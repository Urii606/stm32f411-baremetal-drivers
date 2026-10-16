#pragma once

#include <stdint.h> 

/* ARM Cortex-M4 Core Debug and DWT hardware register addresses */
#define DEMCR         (*((volatile uint32_t*)0xE000EDFC))
#define DWT_CTRL      (*((volatile uint32_t *)0xE0001000U))
#define DWT_CYCCNT    (*((volatile uint32_t *)0xE0001004U))

/* Enable bit for Trace and Debug blocks (required to supply clock to DWT) */
#define DEMCR_TRCENA       (1U << 24)

/* Enable bit to start the CYCCNT cycle counter */
#define DWT_CTRL_CYCCNTENA (1U << 0)

#define ENABLE 1U
#define DISABLE 0U

/**
 * @brief Initializes and enables ARM Cortex-M4 DWT cycle counter.
 * 
 */
void Delay_Init(void);

/**
 * @brief Blocking microsecond delay based on CPU cycle counting.
 * 
 * @param us Duration in microseconds (calculated for 16 MHz CPU frequency).
 */
void Delay_Us(uint32_t us);