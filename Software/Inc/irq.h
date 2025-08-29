/**
 ******************************************************************************
 * @file           : irq.h
 * @author         : Haoyi Chen
 * @date           : 2025-08-08
 * @brief          : Interrupt service routines header
 ******************************************************************************
 * @details
 * This file contains declarations for interrupt handlers and related functions
 * for various peripherals including DMA, Timer, ADC, GPIO, etc.
 *
 * This file is part of a bare-metal STM32F407VGT6 project.
 ******************************************************************************
 */

#ifndef IRQ_H
#define IRQ_H

#include "stm32f407xx.h"

/**
 * @brief DMA2 Stream 0 interrupt handler (ADC1 DMA)
 * 
 * This function is called when DMA completes transferring 50 ADC samples.
 * It automatically calculates the average and sets a ready flag.
 */
void DMA2_Stream0_IRQHandler(void);

/**
 * @brief SysTick interrupt handler
 * 
 * This interrupt is triggered every 1ms by the SysTick timer.
 * Calls the systick_irq_handler() to increment the system time counter.
 */
void SysTick_Handler(void);

#endif /* IRQ_H */
