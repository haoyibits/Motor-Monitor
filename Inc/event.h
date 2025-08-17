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
 * @brief Initialize motor control system
 * 
 * @details Configures motor control GPIO pins, encoder interface, and starts
 *          encoder counting for position feedback.
 */
void motor_init(void);

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
void current_handler(void);
void encoder_handler(void);
#endif /* EVENT_H */
