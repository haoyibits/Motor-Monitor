/**
 ******************************************************************************
 * @file           : systick.h
 * @author         : Haoyi Chen
 * @date           : 2025-08-10
 * @brief          : SysTick timer functions for timing and delay operations
 ******************************************************************************
 * @details
 * This file provides essential SysTick timer functions for embedded systems,
 * including time keeping, blocking/non-blocking delays, and simple timer
 * utilities. Designed for bare-metal STM32F407VGT6 applications.
 *
 * Common usage patterns:
 * - System time keeping with 1ms resolution
 * - Blocking delays for simple timing needs
 * - Non-blocking timing for main loop operations
 * - Simple software timers for periodic tasks
 ******************************************************************************
 */

#ifndef SYSTICK_H
#define SYSTICK_H

#include "stm32f407xx.h"
#include <stdint.h>

/* Global system tick counters (volatile for interrupt access) */
extern volatile uint32_t system_tick_ms;

/**
 * @brief Simple software timer structure
 * 
 * @details Lightweight timer for non-blocking periodic operations.
 *          Suitable for tasks like LED blinking, sensor reading, etc.
 */
typedef struct {
    uint32_t start_time;    /**< Timer start time in milliseconds */
    uint32_t interval;      /**< Timer interval in milliseconds */
    uint8_t enabled;        /**< Timer enabled flag: 1=active, 0=stopped */
    uint8_t auto_reload;    /**< Auto-reload mode: 1=continuous, 0=one-shot */
} SysTick_Timer_t;

/**
 * @name SysTick Configuration Constants
 * @{
 */
#define SYSTICK_FREQUENCY_HZ    1000    /**< SysTick frequency: 1kHz = 1ms interrupts */
/** @} */

/**
 * @name Core SysTick Functions
 * @{
 */

/**
 * @brief Initialize SysTick timer for 1ms interrupts
 * 
 * @details Configures SysTick to generate interrupts every 1ms using the
 *          processor clock. This is the most common configuration for
 *          system timing in embedded applications.
 * 
 * @param system_clock_hz System clock frequency in Hz (e.g., 168000000 for 168MHz)
 * @return uint8_t 0 if successful, 1 if reload value exceeds 24-bit limit
 * 
 * @note Maximum supported clock frequency is ~16.7MHz due to 24-bit reload register
 * @warning Call this function before enabling interrupts
 */
uint8_t systick_init(uint32_t system_clock_hz);



/**
 * @brief Get current system time in milliseconds
 * 
 * @details Returns the number of milliseconds since system initialization.
 *          Rolls over approximately every 49.7 days.
 * 
 * @return uint32_t Current system time in milliseconds
 * 
 * @note This function is interrupt-safe and can be called from any context
 */
uint32_t systick_get_ms(void);

/**
 * @brief Calculate elapsed time since a reference point
 * 
 * @details Calculates time difference handling 32-bit counter rollover.
 *          Useful for measuring time intervals up to ~49 days.
 * 
 * @param start_time_ms Reference time obtained from systick_get_ms()
 * @return uint32_t Elapsed time in milliseconds
 * 
 * @note Handles counter overflow correctly
 */
uint32_t systick_elapsed_ms(uint32_t start_time_ms);

/** @} */

/**
 * @name Delay Functions
 * @{
 */

/**
 * @brief Blocking delay in milliseconds
 * 
 * @details Blocks execution for the specified time period.
 *          Uses CPU polling, so other interrupts can still execute.
 * 
 * @param delay_ms Delay duration in milliseconds
 * 
 * @warning Blocking function - use sparingly in main application
 * @note Accuracy depends on SysTick interrupt timing (±1ms typical)
 */
void systick_delay_ms(uint32_t delay_ms);

/**
 * @brief Check if a delay period has elapsed (non-blocking)
 * 
 * @details Non-blocking function to check if a specified time has passed
 *          since a reference point. Ideal for main loop timing.
 * 
 * @param start_time_ms Reference time from systick_get_ms()
 * @param delay_ms Desired delay period in milliseconds
 * @return uint8_t 1 if delay period has elapsed, 0 otherwise
 * 
 * @note Preferred method for timing in main application loops
 */
uint8_t systick_delay_elapsed(uint32_t start_time_ms, uint32_t delay_ms);

/** @} */

/**
 * @name Simple Timer Utilities
 * @{
 */


/**
 * @brief Initialize a software timer
 * 
 * @details Sets up a timer structure with specified interval and mode.
 *          Timer is created in stopped state.
 * 
 * @param timer Pointer to timer structure
 * @param interval_ms Timer period in milliseconds
 * @param auto_reload 1 for continuous timer, 0 for one-shot
 * 
 * @note Timer must be started with systick_timer_start() to begin counting
 */
void systick_timer_init(SysTick_Timer_t *timer, uint32_t interval_ms, uint8_t auto_reload);

/**
 * @brief Start a software timer
 * 
 * @details Starts timer counting from current system time.
 *          Can be used to restart an expired timer.
 * 
 * @param timer Pointer to initialized timer structure
 */
void systick_timer_start(SysTick_Timer_t *timer);

/**
 * @brief Check if software timer has expired
 * 
 * @details Checks timer expiration and handles auto-reload if enabled.
 *          Call this function periodically in main loop.
 * 
 * @param timer Pointer to timer structure
 * @return uint8_t 1 if timer expired, 0 if still counting
 * 
 * @note Auto-reload timers restart automatically when expired
 * @note One-shot timers stop automatically when expired
 */
uint8_t systick_timer_expired(SysTick_Timer_t *timer);

/** @} */

#endif /* SYSTICK_H */

