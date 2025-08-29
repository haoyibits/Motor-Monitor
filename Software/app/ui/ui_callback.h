/**
 * @file ui_callback.h
 * @brief UI callback function declarations for motor monitor interface
 * 
 * Contains all UI-related business logic and callback function declarations.
 * This file handles the interaction between UI events and application logic.
 */

#ifndef UI_CALLBACK_H
#define UI_CALLBACK_H

#include "system_includes.h"

/**
 * @brief Main menu callback - Motor Settings
 * 
 * Handles navigation to motor settings submenu
 */
void ui_motor_settings_callback(void);

/**
 * @brief Main menu callback - Overcurrent Protection
 * 
 * Handles navigation to protection settings submenu
 */
void ui_overcurrent_protection_callback(void);

/**
 * @brief Main menu callback - Device Info
 * 
 * Handles navigation to device information submenu
 */
void ui_device_info_callback(void);

/**
 * @brief Motor settings callback - PWM Duty Cycle
 * 
 * Opens PWM duty cycle adjustment window
 */
void ui_pwm_duty_callback(void);

/**
 * @brief Protection settings callback - Current Limit
 * 
 * Opens current limit adjustment window
 */
void ui_current_limit_callback(void);

/**
 * @brief Device info callback - Display Brightness
 * 
 * Opens display brightness adjustment window
 */
void ui_brightness_callback(void);

/**
 * @brief UI initialization function
 * 
 * Initializes the UI system and loads the main menu
 */
void ui_init(void);

/**
 * @brief UI main loop function
 * 
 * Should be called periodically from main application loop
 */
void ui_update(void);

/**
 * @brief Button event handler for UI
 * 
 * @param button_id Button identifier (0-3 for Up/Down/Enter/Back)
 * @param event_type Event type (press/release/long_press)
 */
void ui_button_event(uint8_t button_id, uint8_t event_type);

/**
 * @brief Apply motor control changes from UI
 * 
 * Called when UI values change to update motor control system
 */
void ui_apply_motor_settings(void);

/**
 * @brief Update UI with current motor status
 * 
 * Called to refresh UI display with real-time motor data
 */
void ui_update_motor_status(void);

#endif /* UI_CALLBACK_H */