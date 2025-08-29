/**
 ******************************************************************************
 * @file           : motor.h
 * @author         : Haoyi Chen
 * @date           : 2025-08-23
 * @brief          : Motor control and configuration management header
 ******************************************************************************
 * @details
 * This file contains declarations for motor control functions, configuration
 * management, and SPI Flash persistent storage functionality.
 ******************************************************************************
 */

#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f407xx.h"

/**
 * @brief Motor parameter configuration structure for SPI Flash storage
 * 
 * @details This structure holds all motor parameters that need to be persisted
 *          across system restarts including PWM settings, output source, and protection settings
 */
typedef struct {
    uint16_t magic_number;           // Magic number for data validation (0xA5B6)
    float pwm_frequency;             // PWM frequency in Hz
    float pwm_duty_cycle;            // PWM duty cycle (0-100%)
    uint8_t output_source;           // Output source: 0=disabled, 1=STM32, 2=FPGA
    uint8_t overcurrent_protection;  // Overcurrent protection enable: 0=disabled, 1=enabled
    uint8_t motor_direction;         // Motor direction: 0=CW, 1=CCW
    uint8_t auto_restart_enable;     // Auto restart after overcurrent: 0=disabled, 1=enabled
    uint16_t auto_restart_delay_ms;  // Auto restart delay in milliseconds
    uint8_t max_restart_attempts;    // Maximum restart attempts before fault
    uint8_t motor_control_mode;      // Motor control mode: 0=PWM, 1=PID speed control
    float pid_target_speed_percent;  // PID target speed as percentage of max RPM (0-100%)
    float motor_max_rpm;             // Maximum motor RPM for percentage calculation
    uint16_t checksum;               // Simple checksum for data integrity
} Motor_Config_t;

/**
 * @brief Motor protection and restart state management
 */
typedef struct {
    uint8_t protection_fault;        // Current protection fault status: 0=OK, 1=fault
    uint8_t restart_attempts;        // Current restart attempt count
    uint32_t last_fault_time;        // Timestamp of last fault occurrence
    uint32_t restart_scheduled_time; // Timestamp when restart should occur
    uint8_t fault_display_pending;   // Flag to indicate UI should show fault
    uint8_t waiting_user_decision;   // Flag: waiting for user decision after max attempts
    uint8_t user_decision_retry;     // User decision: 1=retry, 0=cancel
} Motor_Protection_State_t;

/**
 * @brief UART communication packet for FPGA
 */
typedef struct {
    uint8_t header[2];               // Packet header: 0xAA, 0x55
    float pwm_frequency;             // PWM frequency to send to FPGA
    float pwm_duty_cycle;            // PWM duty cycle to send to FPGA
    uint8_t motor_direction;         // Motor direction
    uint8_t checksum;                // Simple checksum
} FPGA_Command_Packet_t;

/* Global motor configuration instance */
extern Motor_Config_t motor_config;

/* Global motor protection state */
extern Motor_Protection_State_t motor_protection_state;

/* Flash storage configuration */
#define MOTOR_CONFIG_FLASH_ADDRESS    0x000000  // Store config at beginning of flash
#define MOTOR_CONFIG_MAGIC_NUMBER     0xA5B6    // Magic number for data validation

/* FPGA communication configuration */
#define FPGA_PACKET_HEADER_1         0xAA      // First byte of packet header
#define FPGA_PACKET_HEADER_2         0x55      // Second byte of packet header

/**
 * @brief Initialize motor configuration system and load settings from SPI Flash
 * 
 * @details Initializes SPI Flash and loads motor parameters from storage.
 *          If no valid data is found, default values are used and saved.
 */
void motor_config_init(void);

/**
 * @brief Save current motor configuration to SPI Flash
 * 
 * @return uint8_t 1 if successful, 0 if failed
 */
uint8_t motor_config_save_to_flash(void);

/**
 * @brief Load motor configuration from SPI Flash
 * 
 * @return uint8_t 1 if config loaded from flash, 0 if defaults were used
 */
uint8_t motor_config_load_from_flash(void);

/**
 * @brief Initialize motor control system
 * 
 * @details Configures motor control GPIO pins, encoder interface, and starts
 *          encoder counting for position feedback.
 */
void motor_init(void);

/**
 * @brief Start motor with startup-aware current protection
 * 
 * @details Starts the motor with specified direction and initializes startup protection.
 * 
 * @param direction Motor direction (1 = forward, 0 = reverse)
 */
/**
 * @brief Enable motor with startup current protection
 * 
 * @details Enables motor operation and sets up startup current monitoring.
 *          Should be called after PWM configuration and direction setting.
 */
void motor_start(void);

/**
 * @brief Set motor direction
 * 
 * @param direction Motor direction (0=clockwise, 1=counter-clockwise)
 * 
 * @details Controls motor direction via GPIO pins P/M regardless of PWM source.
 *          This function works with both STM32 and FPGA PWM configurations.
 */
void motor_set_direction(uint8_t direction);

/**
 * @brief Perform emergency motor shutdown and reset monitoring state
 * 
 * @details Safely shuts down the motor and resets all control signals.
 */
void motor_emergency_shutdown(void);

/**
 * @brief Update motor state and current threshold based on operational conditions
 * 
 * @details Manages motor state transitions between STOPPED, STARTING, and RUNNING.
 */
void motor_current_state_update(void);

/**
 * @brief Send motor parameters to FPGA via UART
 * 
 * @details Transmits PWM frequency and duty cycle to FPGA via UART.
 *          Motor direction is controlled separately by STM32 GPIO pins.
 */
void motor_send_fpga_command(float frequency, float duty_cycle);

/**
 * @brief Update motor protection state and handle auto-restart logic
 * 
 * @details Manages overcurrent protection, restart attempts, and fault displays.
 */
void motor_protection_update(void);

/**
 * @brief Apply current motor configuration
 * 
 * @details Configures PWM output source and sends parameters to FPGA if needed.
 */
void motor_apply_config(void);

/**
 * @brief Initialize motor protection system
 * 
 * @details Initializes protection state and sets default values.
 */
void motor_protection_init(void);

/**
 * @brief Configure PWM output based on current motor configuration
 * 
 * @param frequency PWM frequency in Hz
 * @param duty_cycle PWM duty cycle in percentage (0-100)
 * @param output_source Output source selection (0=disabled, 1=STM32, 2=FPGA)
 * 
 * @details Handles PWM configuration based on output source:
 *          - STM32: Configures TIM3_CH1 on PB4 for PWM output
 *          - FPGA: Sends PWM parameters via UART
 *          - Disabled: Stops PWM output
 */
void motor_pwm_config(float frequency, float duty_cycle, uint8_t output_source);

/**
 * @brief Handle user decision for motor restart after max attempts
 * 
 * @param retry 1 to retry motor start, 0 to cancel and disable motor
 */
void motor_handle_user_decision(uint8_t retry);

#endif /* MOTOR_H */