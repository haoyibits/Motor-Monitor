/**
 ******************************************************************************
 * @file           : OLED_UI_Driver.h
 * @author         : Haoyi Chen
 * @date           : 2025-08-22
 * @brief          : OLED_UI_Core library hardware abstraction port header
 ******************************************************************************
 * @details
 * OLED_UI_Core Library Hardware Abstraction Layer - Hardware Driver Port
 * 
 * This file serves as the hardware abstraction layer port for integrating the
 * OLED_UI_Core library with bare-metal STM32F407VGT6 applications.
 * 
 * Original library: OLED_UI_Core (HAL-based, Timer interrupts)
 * Port target: STM32F407VGT6 bare-metal (register-based, SysTick)
 * 
 * Port responsibilities:
 * - Button input system integration with existing BSP
 * - Timer abstraction using SysTick-based timers
 * - Encoder interface placeholder (future implementation)
 * - Delay functions for UI timing requirements
 * - Hardware resource management integration
 * 
 * Integration approach:
 * - Maintains original OLED_UI_Core library interface compatibility
 * - Delegates hardware operations to project BSP layer
 * - Avoids duplicate initialization of shared hardware resources
 * - Provides clean abstraction between library and hardware
 ******************************************************************************
 */

#ifndef __OLED_UI_DRIVER_H
#define __OLED_UI_DRIVER_H

#include "stm32f407xx.h"
#include "button.h"
#include "systick.h"
//#include "misc.h"

/**
 * @brief External button handle declarations
 * @note Button handles are defined and initialized in BSP event system
 */
extern Button_HandleTypeDef button_up, button_down, button_enter, button_return;
extern SysTick_Timer_t oledui_timer;
/**
 * @brief Button status macros for UI integration
 * @note UI expects: 1=not pressed, 0=pressed
 * @note button_is_pressed() returns: 1=pressed, 0=not pressed (needs inversion)
 */
#define Key_GetEnterStatus()    (!button_is_pressed(&button_enter))
#define Key_GetBackStatus()     (!button_is_pressed(&button_return))  
#define Key_GetUpStatus()       (!button_is_pressed(&button_up))
#define Key_GetDownStatus()     (!button_is_pressed(&button_down))

/**
 * @brief Initialize UI timer system
 * @note Uses SysTick-based timer with 20ms interval for 50fps
 */
void Timer_Init(void);

/**
 * @brief Initialize button system
 * @note Empty implementation - buttons initialized in BSP
 */
void Key_Init(void);

/**
 * @brief Initialize encoder system
 * @note Not implemented - placeholder for future use
 */
void Encoder_Init(void);

/**
 * @brief Enable encoder
 * @note Not implemented - placeholder for future use
 */
void Encoder_Enable(void);

/**
 * @brief Disable encoder
 * @note Not implemented - placeholder for future use
 */
void Encoder_Disable(void);

/**
 * @brief Get encoder increment value
 * @return int16_t Encoder increment (always 0 - not implemented)
 */
int16_t Encoder_Get(void);

/**
 * @brief Millisecond delay function
 * @param xms Delay time in milliseconds
 */
void Delay_ms(uint32_t xms);

/**
 * @brief Second delay function
 * @param xs Delay time in seconds
 */
void Delay_s(uint32_t xs);

#endif
