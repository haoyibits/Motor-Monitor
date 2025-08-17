/**
 ******************************************************************************
 * @file           : systick.c
 * @author         : Haoyi Chen
 * @date           : 2025-08-10
 * @brief          : SysTick timer implementation for timing operations
 ******************************************************************************
 * @details
 * This file implements essential SysTick timer functions for embedded systems.
 * Provides system time keeping, delays, and simple software timers using the
 * ARM Cortex-M SysTick timer peripheral configured for 1ms interrupts.
 *
 * Key features:
 * - 1ms system tick resolution
 * - Overflow-safe time calculations
 * - Blocking and non-blocking delay functions
 * - Lightweight software timer implementation
 ******************************************************************************
 */

#include "systick.h"
#include "rcc.h"

/* Global variables */
volatile uint32_t system_tick_ms = 0;    /**< System time counter in milliseconds */

/**
 * @brief Initialize SysTick timer for 1ms interrupts
 * 
 * @details Configures the ARM Cortex-M SysTick timer to generate interrupts
 *          every 1ms using the processor clock. The SysTick timer is a 24-bit
 *          down-counter that reloads from a specified value.
 * 
 * @param system_clock_hz System clock frequency in Hz (typically 168MHz for STM32F407)
 * @return uint8_t 0 if successful, 1 if reload value exceeds 24-bit limit
 * 
 * @note SysTick uses processor clock (HCLK) as clock source
 * @warning Maximum reload value is 0xFFFFFF (24-bit), limiting max clock to ~16.7MHz
 */
uint8_t systick_init(uint32_t system_clock_hz)
{
    /* Calculate reload value for 1ms interrupts: clock_hz / 1000Hz */
    uint32_t reload_value = system_clock_hz / SYSTICK_FREQUENCY_HZ;
    
    /* Check if reload value fits in 24-bit register */
    if (reload_value > SysTick_LOAD_RELOAD_Msk) {
        return 1; // Error: reload value too large for 24-bit register
    }
    
    /* Configure SysTick timer */
    SysTick->LOAD = reload_value - 1;           /* Set reload value (minus 1 for 0-based counting) */
    SysTick->VAL = 0;                           /* Clear current value register */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | /* Use processor clock (HCLK) */
                    SysTick_CTRL_TICKINT_Msk |   /* Enable SysTick interrupt */
                    SysTick_CTRL_ENABLE_Msk;     /* Enable SysTick timer */
    
    return 0; // Success
}


/**
 * @brief Get current system time in milliseconds
 * 
 * @details Returns the current system time since initialization. The counter
 *          will overflow after approximately 49.7 days (2^32 ms = 4,294,967,296 ms).
 * 
 * @return uint32_t Current system time in milliseconds
 * 
 * @note Thread-safe: atomic read of volatile variable
 * @note Can be called from interrupt context
 */
uint32_t systick_get_ms(void)
{
    return system_tick_ms;
}

/**
 * @brief Calculate elapsed time since a reference point
 * 
 * @details Calculates time difference between current time and a reference time,
 *          correctly handling 32-bit counter overflow. This allows measuring
 *          intervals even when the counter rolls over.
 * 
 * @param start_time_ms Reference time obtained from systick_get_ms()
 * @return uint32_t Elapsed time in milliseconds
 * 
 * @note Handles counter overflow: works correctly across 32-bit boundary
 * @note Maximum measurable interval is ~49 days (full 32-bit range)
 */
uint32_t systick_elapsed_ms(uint32_t start_time_ms)
{
    uint32_t current_time = systick_get_ms();
    
    /* Handle counter overflow using unsigned arithmetic properties */
    if (current_time >= start_time_ms) {
        return current_time - start_time_ms;    /* Normal case: no overflow */
    } else {
        /* Overflow case: calculate elapsed time across 32-bit boundary */
        return (0xFFFFFFFF - start_time_ms) + current_time + 1;
    }
}

/**
 * @brief Blocking delay in milliseconds
 * 
 * @details Blocks program execution for the specified time period using
 *          active polling. Other interrupts can still execute during delay.
 * 
 * @param delay_ms Delay duration in milliseconds
 * 
 * @warning Blocking function: CPU is occupied during delay
 * @note Use systick_delay_elapsed() for non-blocking timing in main loops
 * @note Actual delay may be up to 1ms longer due to timing granularity
 */
void systick_delay_ms(uint32_t delay_ms)
{
    uint32_t start_time = systick_get_ms();    /* Record start time */
    
    /* Poll until desired time has elapsed */
    while (systick_elapsed_ms(start_time) < delay_ms) {
        __NOP();    /* No operation: prevents compiler optimization */
    }
}

/**
 * @brief Check if a delay period has elapsed (non-blocking)
 * 
 * @details Non-blocking function to check if a specified time period has
 *          passed since a reference time. Ideal for main loop timing without
 *          blocking execution.
 * 
 * @param start_time_ms Reference time from systick_get_ms()
 * @param delay_ms Desired delay period in milliseconds
 * @return uint8_t 1 if delay period has elapsed, 0 if still waiting
 * 
 * @note Preferred method for timing in event-driven applications
 * @note Does not block execution: allows other tasks to run
 * 
 * @code
 * // Example usage in main loop:
 * uint32_t last_time = systick_get_ms();
 * while(1) {
 *     if (systick_delay_elapsed(last_time, 1000)) {
 *         // Execute every 1000ms
 *         last_time = systick_get_ms();
 *     }
 *     // Other tasks can run here
 * }
 * @endcode
 */
uint8_t systick_delay_elapsed(uint32_t start_time_ms, uint32_t delay_ms)
{
    return (systick_elapsed_ms(start_time_ms) >= delay_ms) ? 1 : 0;
}

/**
 * @brief Initialize a software timer
 * 
 * @details Sets up a software timer structure with specified interval and mode.
 *          The timer is created in stopped state and must be started explicitly.
 * 
 * @param timer Pointer to timer structure to initialize
 * @param interval_ms Timer period in milliseconds
 * @param auto_reload 1 for continuous timer, 0 for one-shot timer
 * 
 * @note Timer is initialized in stopped state
 * @note Use systick_timer_start() to begin timing
 * 
 * @code
 * // Example: Create a 500ms auto-reload timer
 * SysTick_Timer_t led_timer;
 * systick_timer_init(&led_timer, 500, 1);
 * systick_timer_start(&led_timer);
 * @endcode
 */
void systick_timer_init(SysTick_Timer_t *timer, uint32_t interval_ms, uint8_t auto_reload)
{
    if (!timer) return;    /* Null pointer protection */
    
    timer->start_time = 0;          /* Initialize start time */
    timer->interval = interval_ms;   /* Set timer interval */
    timer->enabled = 0;             /* Timer starts disabled */
    timer->auto_reload = auto_reload; /* Set reload mode */
}

/**
 * @brief Start a software timer
 * 
 * @details Starts timer counting from the current system time. Can be used
 *          to start a new timer or restart an expired timer.
 * 
 * @param timer Pointer to initialized timer structure
 * 
 * @note Timer begins counting immediately from current system time
 * @note Can be called multiple times to restart timer
 */
void systick_timer_start(SysTick_Timer_t *timer)
{
    if (!timer) return;    /* Null pointer protection */
    
    timer->start_time = systick_get_ms();    /* Record current time */
    timer->enabled = 1;                      /* Enable timer */
}

/**
 * @brief Check if software timer has expired
 * 
 * @details Checks if the timer interval has elapsed. For auto-reload timers,
 *          automatically restarts the timer when expired. For one-shot timers,
 *          disables the timer when expired.
 * 
 * @param timer Pointer to timer structure
 * @return uint8_t 1 if timer expired, 0 if still counting
 * 
 * @note Call this function periodically (e.g., in main loop)
 * @note Auto-reload timers restart automatically
 * @note One-shot timers stop automatically after expiring
 * 
 * @code
 * // Example usage in main loop:
 * while(1) {
 *     if (systick_timer_expired(&led_timer)) {
 *         // Execute every timer interval
 *         toggle_led();
 *     }
 *     // Other tasks...
 * }
 * @endcode
 */
uint8_t systick_timer_expired(SysTick_Timer_t *timer)
{
    if (!timer || !timer->enabled) return 0;    /* Check valid and enabled timer */
    
    /* Check if timer interval has elapsed */
    if (systick_elapsed_ms(timer->start_time) >= timer->interval) {
        if (timer->auto_reload) {
            /* Auto-reload timer: restart automatically */
            timer->start_time = systick_get_ms();
        } else {
            /* One-shot timer: disable after expiring */
            timer->enabled = 0;
        }
        return 1;    /* Timer expired */
    }
    
    return 0;    /* Timer still counting */
}