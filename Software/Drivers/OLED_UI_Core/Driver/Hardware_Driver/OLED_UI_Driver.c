/**
 ******************************************************************************
 * @file           : OLED_UI_Driver.c  
 * @author         : Haoyi Chen
 * @date           : 2025-08-22
 * @brief          : OLED_UI_Core library hardware abstraction port implementation
 ******************************************************************************
 * @details
 * OLED_UI_Core Library Hardware Abstraction Layer - Hardware Driver Port Implementation
 * 
 * This file contains the hardware abstraction layer port implementation for
 * integrating the OLED_UI_Core library with bare-metal STM32F407VGT6 applications.
 * 
 * Original library: OLED_UI_Core (HAL-based, Timer interrupts, HAL button handling)
 * Port target: STM32F407VGT6 bare-metal (register-based, SysTick, BSP button system)
 * 
 * Key port implementations:
 * - Button system: Integrated with existing BSP button management system
 * - Timer system: SysTick-based timers replacing HAL timer interrupts  
 * - Encoder system: Placeholder implementation (not currently used)
 * - Delay functions: Project systick delay functions
 * - Hardware resource delegation: All hardware access via BSP layer
 * 
 * Port design principles:
 * - Maintains original OLED_UI_Core library interface compatibility
 * - Delegates all hardware operations to project BSP layer
 * - Avoids duplicate initialization of shared hardware resources
 * - Provides seamless integration with existing project architecture
 ******************************************************************************
 */

#include "OLED_UI_Driver.h"
#include "bsp.h"

/**
 * @brief External button handle declarations
 * @note These handles are defined and initialized in BSP event system
 */
extern Button_HandleTypeDef button_up, button_down, button_enter, button_return;

/**
 * @brief UI timer handle for periodic updates
 * @note Uses SysTick-based timer with 20ms interval for 50fps
 */
SysTick_Timer_t oledui_timer;

/**
 * @brief Initialize UI timer system using SysTick
 * @note Uses existing SysTick infrastructure with 20ms interval for 50fps
 */
void Timer_Init(void)
{
    /* UI timer initialization: 20ms interval for 50fps, auto-reload */
    systick_timer_init(&oledui_timer, 20, 1);
    systick_timer_start(&oledui_timer);
}

/**
 * @brief Initialize button system (empty implementation)
 * @note Buttons already initialized in button_system_init(), no action needed
 * @details Actual button initialization completed in BSP/event.c button_system_init(),
 *          avoiding duplicate initialization of shared hardware resources
 */
void Key_Init(void)
{
    /* Button system already initialized in button_system_init() */
    /* No duplicate initialization needed here */
}

/**
 * @brief Initialize encoder system (placeholder)
 * @note Encoder functionality not currently implemented
 */
void Encoder_Init(void)
{
    /* Encoder functionality not implemented */
}

/**
 * @brief Enable encoder (placeholder)
 * @note Encoder functionality not currently implemented
 */
void Encoder_Enable(void)
{
    /* Encoder functionality not implemented */
}

/**
 * @brief Disable encoder (placeholder)
 * @note Encoder functionality not currently implemented
 */
void Encoder_Disable(void)
{
    /* Encoder functionality not implemented */
}

/**
 * @brief Get encoder increment value (placeholder)
 * @return int16_t Encoder increment value, always returns 0 (not implemented)
 * @note Encoder functionality not currently used, always returns 0
 */
int16_t Encoder_Get(void)
{
    return 0; /* Encoder functionality not implemented */
}


/**
 * @brief Millisecond delay function
 * @param xms Delay time in milliseconds, range: 0~4294967295
 */
void Delay_ms(uint32_t xms)
{
    systick_delay_ms(xms);
}

/**
 * @brief Second delay function  
 * @param xs Delay time in seconds, range: 0~4294967295
 */
void Delay_s(uint32_t xs)
{
    systick_delay_ms(xs * 1000);
}
