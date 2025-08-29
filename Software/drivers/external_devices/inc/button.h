/**
 * @file button.h
 * @author Haoyi Chen
 * @date 2025-08-14
 * @brief STM32F4 Button driver with optimized scanning
 *
 * @details This file contains optimized button functions for STM32F407 series
 * using shared systick timer for efficient scanning and shift register debouncing.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include "stm32f407xx.h"
#include "gpio.h"
#include "systick.h"

/**
 * @brief Button configuration structure
 */
typedef struct {
    GPIO_TypeDef *GPIOx;         /**< GPIO port for button */
    uint8_t pin;                 /**< GPIO pin number (0-15) */
    uint8_t active_level;        /**< Button active level: 0=active low, 1=active high */
    uint8_t pullup_enable;       /**< Enable internal pull-up: 1=enable, 0=disable */
} Button_InitTypeDef;

/**
 * @brief Button handle structure - optimized with shift register debouncing
 */
typedef struct {
    GPIO_TypeDef *GPIOx;         /**< GPIO port */
    uint8_t pin;                 /**< GPIO pin number */
    uint8_t active_level;        /**< Active level configuration */
    
    /* Optimized state variables */
    uint8_t current_state;       /**< Current button state: 0=released, 1=pressed */
    uint8_t last_state;          /**< Previous button state */
    uint8_t press_event;         /**< Press event flag */
    
    /* Optimized debounce filter using shift register */
    uint8_t debounce_shift_reg;  /**< 8-bit shift register for debouncing */
    
    /* Status flags */
    uint8_t initialized;         /**< Initialization flag */
} Button_HandleTypeDef;

/**
 * @brief Button manager structure for efficient multi-button handling
 */
typedef struct {
    Button_HandleTypeDef **buttons;     /**< Array of button pointers */
    uint8_t button_count;               /**< Number of buttons */
    SysTick_Timer_t scan_timer;         /**< Shared timer for all buttons */
    uint8_t initialized;                /**< Manager initialization flag */
} Button_Manager_t;

/**
 * @name Button Configuration Constants
 * @{
 */
#define BUTTON_ACTIVE_LOW           0    /**< Button is active when GPIO is low */
#define BUTTON_ACTIVE_HIGH          1    /**< Button is active when GPIO is high */
#define BUTTON_SCAN_INTERVAL_MS     5    /**< Button scan interval in ms */
#define BUTTON_DEBOUNCE_PATTERN     0x0F /**< Debounce pattern: 4 consecutive readings */
#define BUTTON_MAX_BUTTONS          8    /**< Maximum buttons in manager */
/** @} */

/**
 * @brief Initialize button with configuration
 * 
 * @param handle Pointer to button handle structure
 * @param init Pointer to button initialization structure
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t button_init(Button_HandleTypeDef *handle, Button_InitTypeDef *init);

/**
 * @brief Initialize button manager for efficient multi-button scanning
 * 
 * @param manager Pointer to button manager structure
 * @param buttons Array of button handle pointers
 * @param count Number of buttons (max BUTTON_MAX_BUTTONS)
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t button_manager_init(Button_Manager_t *manager, Button_HandleTypeDef **buttons, uint8_t count);

void button_gpio_init(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t pullup_enable);

/**
 * @brief Check button state with optimized shift register debouncing
 * 
 * @param handle Pointer to button handle structure
 */
void button_check_optimized(Button_HandleTypeDef *handle);

/**
 * @brief Check if button is currently pressed
 * 
 * @param handle Pointer to button handle structure
 * @return uint8_t 1 if pressed, 0 if released
 */
uint8_t button_is_pressed(Button_HandleTypeDef *handle);

/**
 * @brief Check for button press event (edge detection)
 * 
 * @param handle Pointer to button handle structure
 * @return uint8_t 1 if press event occurred, 0 otherwise
 */
uint8_t button_pressed(Button_HandleTypeDef *handle);

/* Internal helper functions */
uint8_t button_read_raw(Button_HandleTypeDef *handle);
void button_debounce_shift_register(Button_HandleTypeDef *handle, uint8_t raw_reading);


#endif /* BUTTON_H */
