/**
 ******************************************************************************
 * @file           : event.h
 * @author         : Haoyi Chen
 * @date           : 2025-08-14
 * @brief          : User event handling header
 ******************************************************************************
 * @details
 * This file contains function declarations for the event handling system
 * including motor initialization, button system, and scanning functions.
 ******************************************************************************
 */

#ifndef EVENT_H
#define EVENT_H

#include "stm32f407xx.h"


/**
 * @brief Initialize button system
 * 
 * @details Configures all 4 buttons (UP, DOWN, ENTER, RETURN) and starts
 *          the button manager for efficient scanning.
 */
void button_system_init(void);

/**
 * @brief Initialize all system scanning timers
 * 
 * @details Initializes encoder timer, current monitoring timer, and button system.
 */
void scan_init(void);

/**
 * @brief Check all system timers and handle events
 * 
 * @details Checks encoder timer, current monitoring timer, and button states.
 *          Calls button_handler() when button events are detected.
 */
void scan_check(void);

/**
 * @brief Handle button events
 * 
 * @details Processes button press events for motor control including:
 *          - UP: Navigation/speed increase
 *          - DOWN: Navigation/speed decrease  
 *          - ENTER: Start/stop motor
 *          - RETURN: Emergency stop
 */
void button_handler(void);

/**
 * @brief Monitor motor current with startup-aware protection
 * 
 * @details Implements intelligent current monitoring with different thresholds
 *          for startup vs running states.
 */
void current_handler(void);

/**
 * @brief Process encoder feedback data and calculate speed
 * 
 * @details Monitors encoder position and calculates motor speed in RPM.
 */
void encoder_handler(void);

/**
 * @brief Early motor configuration loading during system startup
 * 
 * @details Loads motor configuration from SPI Flash early in the boot process.
 */
void motor_config_early_load(void);


#endif /* EVENT_H */
