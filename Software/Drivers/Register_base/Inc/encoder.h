/**
 * @file encoder.h
 * @author Haoyi Chen
 * @date 2025-08-17
 * @brief STM32F4 Hardware Encoder interface driver header
 *
 * @details This file contains encoder function declarations, structures, and macro definitions
 * for STM32F407 series microcontrollers using timer hardware encoder mode.
 */

#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f407xx.h"
#include "tim.h"
#include "gpio.h"

/**
 * @brief Encoder configuration structure
 */
typedef struct {
    TIM_TypeDef *TIMx;           /**< Timer instance to use for encoder */
    uint16_t CountsPerRevolution; /**< Encoder counts per full revolution (CPR) */
    uint8_t IC1Polarity;         /**< Input capture 1 polarity */
    uint8_t IC2Polarity;         /**< Input capture 2 polarity */
    uint16_t MaxCount;           /**< Maximum count value (ARR) */
} Encoder_InitTypeDef;

/**
 * @brief Encoder data structure
 */
typedef struct {
    TIM_TypeDef *TIMx;           /**< Timer instance */
    uint16_t CountsPerRevolution; /**< CPR value */
    int32_t TotalCount;          /**< Total accumulated count */
    int32_t LastCount;           /**< Last total count for speed calculation */
    uint16_t LastHwCount;        /**< Last hardware count for overflow detection */
    int32_t Speed;               /**< Current speed in RPM */
    uint32_t LastTimeMs;         /**< Last time measurement in ms */
    uint8_t LastPhaseA;          /**< Last A phase state for software decoding */
    uint8_t LastPhaseB;          /**< Last B phase state for software decoding */
} Encoder_HandleTypeDef;

/**
 * @name Encoder Input Polarity
 * @{
 */
#define ENCODER_IC_POLARITY_RISING     0x00  /**< Rising edge polarity */
#define ENCODER_IC_POLARITY_FALLING    0x02  /**< Falling edge polarity */
/** @} */

/**
 * @name Encoder Mode
 * @{
 */
#define ENCODER_MODE_TI1         0x01  /**< Count on TI1 edges only */
#define ENCODER_MODE_TI2         0x02  /**< Count on TI2 edges only */
#define ENCODER_MODE_TI12        0x03  /**< Count on both TI1 and TI2 edges (4x resolution) */
/** @} */


/**
 * @brief Initialize encoder interface using timer hardware encoder mode
 * 
 * @details Configures the specified timer in hardware encoder mode with GPIO pins
 *          as encoder inputs. Sets up quadrature decoding using TI1 and TI2.
 * 
 * @param handle Pointer to encoder handle structure
 * @param init Pointer to encoder initialization structure
 * @return uint8_t 0 if successful, 1 if error
 * 
 * @note GPIO pins must be configured separately before calling this function
 */
uint8_t encoder_init(Encoder_HandleTypeDef *handle, Encoder_InitTypeDef *init);

/**
 * @brief Configure GPIO pins for encoder input
 * 
 * @details Sets up GPIO pins as alternate function for timer encoder inputs.
 *          Configures pull-up resistors and alternate function mapping.
 * 
 * @param TIMx Timer instance
 * @param ch1_port GPIO port for channel 1 (A phase)
 * @param ch1_pin GPIO pin for channel 1 (A phase)
 * @param ch2_port GPIO port for channel 2 (B phase)
 * @param ch2_pin GPIO pin for channel 2 (B phase)
 * @param af_selection Alternate function selection (1-3)
 * 
 * @note RCC clocks for GPIO ports must be enabled before calling this function
 */
void encoder_gpio_init(TIM_TypeDef *TIMx, GPIO_TypeDef *ch1_port, uint8_t ch1_pin,
                       GPIO_TypeDef *ch2_port, uint8_t ch2_pin, uint8_t af_selection);

/**
 * @brief Start encoder counting
 * 
 * @details Enables the timer counter to start counting encoder pulses.
 * 
 * @param handle Pointer to encoder handle structure
 */
void encoder_start(Encoder_HandleTypeDef *handle);

/**
 * @brief Stop encoder counting
 * 
 * @details Disables the timer counter to stop counting encoder pulses.
 * 
 * @param handle Pointer to encoder handle structure
 */
void encoder_stop(Encoder_HandleTypeDef *handle);

/**
 * @brief Get current encoder count
 * 
 * @details Reads the current timer counter value representing encoder position.
 * 
 * @param handle Pointer to encoder handle structure
 * @return uint16_t Current encoder count (0 to MaxCount)
 */
uint16_t encoder_get_count(Encoder_HandleTypeDef *handle);

/**
 * @brief Reset encoder count to zero
 * 
 * @details Resets both the timer counter and accumulated count to zero.
 * 
 * @param handle Pointer to encoder handle structure
 */
void encoder_reset_count(Encoder_HandleTypeDef *handle);

/**
 * @brief Get encoder direction
 * 
 * @details Determines rotation direction based on timer direction bit.
 *          In hardware encoder mode, the direction is automatically set by hardware.
 * 
 * @param handle Pointer to encoder handle structure
 * @return int8_t 1 = forward, -1 = reverse, 0 = stopped
 */
int8_t encoder_get_direction(Encoder_HandleTypeDef *handle);

/**
 * @brief Update encoder total count and detect direction
 * 
 * @details Updates accumulated count considering counter overflow/underflow.
 *          Should be called periodically or in timer interrupt.
 * 
 * @param handle Pointer to encoder handle structure
 */
void encoder_update(Encoder_HandleTypeDef *handle);

/**
 * @brief Calculate encoder speed in RPM
 * 
 * @details Calculates rotational speed based on count changes and time elapsed.
 * 
 * @param handle Pointer to encoder handle structure
 * @param current_time_ms Current system time in milliseconds
 * @return int32_t Speed in RPM (positive or negative)
 */
int32_t encoder_calculate_speed_rpm(Encoder_HandleTypeDef *handle, uint32_t current_time_ms);

/**
 * @brief Generic timer IRQ handler for encoder overflow/underflow
 * 
 * @details Handles overflow/underflow interrupts for extending count beyond 16-bit.
 *          Should be called from the appropriate timer IRQ handler.
 * 
 * @param handle Pointer to encoder handle structure
 */
void encoder_timer_irq_handler(Encoder_HandleTypeDef *handle);

#endif /* ENCODER_H */
