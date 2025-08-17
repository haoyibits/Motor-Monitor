/**
 * @file button.c
 * @author Haoyi Chen
 * @date 2025-08-14
 * @brief STM32F4 Button driver implementation with optimized scanning
 *
 * @details This file contains optimized button function implementations using 
 * shared systick timer and shift register debouncing for efficient multi-button scanning.
 */

#include "button.h"
#include "stdio.h"
/**
 * @brief Initialize button with configuration
 * 
 * @param handle Pointer to button handle structure
 * @param init Pointer to button initialization structure
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t button_init(Button_HandleTypeDef *handle, Button_InitTypeDef *init)
{
    /* Validate input parameters */
    if (handle == NULL || init == NULL) {
        return 1;
    }
    
    if (init->GPIOx == NULL || init->pin > 15) {
        return 1;
    }
    
    /* Initialize GPIO for button input */
    button_gpio_init(init->GPIOx, init->pin, init->pullup_enable);
    
    /* Initialize button handle structure */
    handle->GPIOx = init->GPIOx;
    handle->pin = init->pin;
    handle->active_level = init->active_level;
    
    /* Initialize state variables */
    handle->current_state = 0;
    handle->last_state = 0;
    handle->press_event = 0;
    
    /* Initialize optimized debounce filter */
    handle->debounce_shift_reg = 0x00;  // Start with all zeros (released state)
    
    /* Set initialization flag */
    handle->initialized = 1;
    
    return 0;
}

/**
 * @brief Initialize button manager for efficient multi-button scanning
 * 
 * @param manager Pointer to button manager structure
 * @param buttons Array of button handle pointers
 * @param count Number of buttons (max BUTTON_MAX_BUTTONS)
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t button_manager_init(Button_Manager_t *manager, Button_HandleTypeDef **buttons, uint8_t count)
{
    /* Validate parameters */
    if (manager == NULL || buttons == NULL || count == 0 || count > BUTTON_MAX_BUTTONS) {
        return 1;
    }
    
    /* Validate all button handles */
    for (uint8_t i = 0; i < count; i++) {
        if (buttons[i] == NULL || !buttons[i]->initialized) {
            return 1;
        }
    }
    
    /* Initialize manager structure */
    manager->buttons = buttons;
    manager->button_count = count;
    manager->initialized = 1;
    
    return 0;
}

/**
 * @brief Configure GPIO pin for button input
 * 
 * @param GPIOx GPIO port
 * @param pin GPIO pin number
 * @param pullup_enable 1 to enable internal pull-up
 */
void button_gpio_init(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t pullup_enable)
{
    uint8_t pupd_config = GPIO_NOPULL;
    
    if (pullup_enable) {
        pupd_config = GPIO_PULLUP;
    }
    
    /* Configure GPIO pin as input with optional pull-up */
    gpio_init(GPIOx, pin, GPIO_MODE_INPUT, GPIO_OTYPE_PP, GPIO_SPEED_LOW, pupd_config);
}

/**
 * @brief Read raw button state from GPIO (optimized inline)
 * 
 * @param handle Pointer to button handle structure
 * @return uint8_t 1 if button is active, 0 if inactive
 */
uint8_t button_read_raw(Button_HandleTypeDef *handle)
{
    uint8_t pin_state = (handle->GPIOx->IDR >> handle->pin) & 0x01;
    
    /* Return 1 if button is active, 0 if inactive */
    return (handle->active_level == BUTTON_ACTIVE_HIGH) ? pin_state : !pin_state;
}

/**
 * @brief Optimized shift register debouncing
 * 
 * @details Uses 8-bit shift register for more efficient debouncing.
 *          Pattern 0x0F means 4 consecutive HIGH readings required.
 * 
 * @param handle Pointer to button handle structure
 * @param raw_reading Raw GPIO reading (0 or 1)
 */
void button_debounce_shift_register(Button_HandleTypeDef *handle, uint8_t raw_reading)
{
    /* Shift register left and add new reading */
    handle->debounce_shift_reg = (handle->debounce_shift_reg << 1) | raw_reading;
    
    /* Check for stable pressed state (4 consecutive 1s) */
    if ((handle->debounce_shift_reg & BUTTON_DEBOUNCE_PATTERN) == BUTTON_DEBOUNCE_PATTERN) {
        if (!handle->current_state) {
            /* Button just pressed */
            handle->last_state = handle->current_state;
            handle->current_state = 1;
            handle->press_event = 1;
        }
    }
    /* Check for stable released state (4 consecutive 0s) */
    else if ((handle->debounce_shift_reg & BUTTON_DEBOUNCE_PATTERN) == 0x00) {
        if (handle->current_state) {
            /* Button just released */
            handle->last_state = handle->current_state;
            handle->current_state = 0;
        }
    }
}


/**
 * @brief Check button state with optimized shift register debouncing
 * 
 * @param handle Pointer to button handle structure
 */
void button_check_optimized(Button_HandleTypeDef *handle)
{
    /* Validate parameters */
    if (handle == NULL || !handle->initialized) {
        return;
    }
    
    /* Read raw state and apply optimized debouncing */
    uint8_t raw_state = button_read_raw(handle);
    button_debounce_shift_register(handle, raw_state);
}

/**
 * @brief Check if button is currently pressed
 * 
 * @param handle Pointer to button handle structure
 * @return uint8_t 1 if pressed, 0 if released
 */
uint8_t button_is_pressed(Button_HandleTypeDef *handle)
{
    /* Validate parameters */
    if (handle == NULL || !handle->initialized) {
        return 0;
    }
    
    return handle->current_state;
}

/**
 * @brief Check for button press event
 * 
 * @param handle Pointer to button handle structure
 * @return uint8_t 1 if press event occurred, 0 otherwise
 */
uint8_t button_pressed(Button_HandleTypeDef *handle)
{
    /* Validate parameters */
    if (handle == NULL || !handle->initialized) {
        return 0;
    }
    
    if (handle->press_event) {
        handle->press_event = 0;  /* Clear event after reading */
        return 1;
    }
    
    return 0;
}
