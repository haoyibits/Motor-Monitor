/**
 ******************************************************************************
 * @file           : irq.c
 * @author         : Haoyi Chen
 * @date           : 2025-08-08
 * @brief          : Interrupt service routines implementation
 ******************************************************************************
 * @details
 * This file contains interrupt handlers for various peripherals including
 * DMA, Timer, ADC, GPIO, etc.
 ******************************************************************************
 */

#include "bsp.h"


/**
 * @brief DMA2 Stream0 interrupt handler
 * 
 * This interrupt is triggered when DMA completes transferring data.
 * Only sets flags and clears interrupt flags to minimize interrupt processing time.
 */
void DMA2_Stream0_IRQHandler(void)
{
    uint32_t flags = DMA2->LISR;

    if ((flags & (DMA_LISR_TEIF0 | DMA_LISR_DMEIF0 | DMA_LISR_FEIF0)) != 0U) {
        DMA2->LIFCR = DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0 |
                      DMA_LIFCR_CFEIF0 | DMA_LIFCR_CHTIF0 |
                      DMA_LIFCR_CTCIF0;
        current_adcDmaError = 1U;
        return;
    }

    // Half-transfer complete interrupt
    if ((flags & DMA_LISR_HTIF0) != 0U) {
        // Clear half-transfer complete flag
        DMA2->LIFCR = DMA_LIFCR_CHTIF0;
        uint32_t half_sum = 0U;
        for (uint16_t i = 0U; i < 100U; ++i) {
            half_sum += current_adcBuffer[i];
        }
        current_adcAverage = (uint16_t)(half_sum / 100U);
        current_adcAverageReady = 1U;
    }
    
    // Transfer complete interrupt
    if ((flags & DMA_LISR_TCIF0) != 0U) {
        // Clear transfer complete flag
        DMA2->LIFCR = DMA_LIFCR_CTCIF0;
        
        uint32_t half_sum = 0U;
        for (uint16_t i = 100U; i < 200U; ++i) {
            half_sum += current_adcBuffer[i];
        }
        current_adcAverage = (uint16_t)(half_sum / 100U);
        current_adcAverageReady = 1U;
    }
}

/**
 * @brief SysTick interrupt handler
 * 
 * This interrupt is triggered every 1ms by the SysTick timer.
 * Calls the systick_irq_handler() to increment the system time counter.
 */
void SysTick_Handler(void)
{
    system_tick_ms++;
    
}
