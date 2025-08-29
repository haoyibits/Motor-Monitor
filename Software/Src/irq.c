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
    // Half-transfer complete interrupt
    if (DMA2->LISR & DMA_LISR_HTIF0) {
        // Clear half-transfer complete flag
        DMA2->LIFCR = DMA_LIFCR_CHTIF0;
        // No processing here - moved to main loop
    }
    
    // Transfer complete interrupt
    if (DMA2->LISR & DMA_LISR_TCIF0) {
        // Clear transfer complete flag
        DMA2->LIFCR = DMA_LIFCR_CTCIF0;
        
        // Set flag to notify main loop that new data is ready
        current_adcAverageReady = 1;
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
