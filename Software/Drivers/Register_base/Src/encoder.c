/**
 * @file encoder.c
 * @author Haoyi Chen
 * @date 2025-08-17
 * @brief Hardware encoder driver - using STM32 hardware encoder mode
 *
 * @details This implementation uses the STM32 timer's hardware encoder mode
 * for quadrature decoding, which is more efficient than input capture
 */

#include <stddef.h>

#include "encoder.h"


/**
 * @brief Initialize encoder using hardware encoder mode
 * 
 * @details Configures the timer in hardware encoder mode (SMS = 3)
 * for efficient quadrature decoding using TI1 and TI2 inputs
 */
DriverStatus encoder_init(Encoder_HandleTypeDef *handle,
                          const Encoder_InitTypeDef *init)
{
    if ((handle == NULL) || (init == NULL) || (init->TIMx == NULL) ||
        (init->CountsPerRevolution == 0U) || (init->MaxCount == 0U)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    
    // Initialize handle structure
    handle->TIMx = init->TIMx;
    handle->CountsPerRevolution = init->CountsPerRevolution;
    handle->TotalCount = 0;
    handle->LastCount = 0;
    handle->Speed = 0;
    handle->LastTimeMs = 0;
    handle->CounterPeriod = (uint32_t)init->MaxCount + 1U;
    handle->LastHardwareCount = 0U;
    handle->LastDirection = 0;
    
    // Temporarily disable timer
    init->TIMx->CR1 &= ~TIM_CR1_CEN;
    
    // Configure Channel 1 (TI1) as encoder input
    init->TIMx->CCMR1 &= ~TIM_CCMR1_CC1S;
    init->TIMx->CCMR1 |= TIM_CCMR1_CC1S_0;  // CC1S = 01 (TI1)
    
    // Configure Channel 2 (TI2) as encoder input
    init->TIMx->CCMR1 &= ~TIM_CCMR1_CC2S;
    init->TIMx->CCMR1 |= TIM_CCMR1_CC2S_0;  // CC2S = 01 (TI2)
    
    // Set input filters for noise immunity
    init->TIMx->CCMR1 &= ~(TIM_CCMR1_IC1F | TIM_CCMR1_IC2F);
    init->TIMx->CCMR1 |= (0x3U << TIM_CCMR1_IC1F_Pos);
    init->TIMx->CCMR1 |= (0x3U << TIM_CCMR1_IC2F_Pos);
    
    // Set polarities for input signals
    init->TIMx->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);
    if (init->IC1Polarity == ENCODER_IC_POLARITY_FALLING) {
        init->TIMx->CCER |= TIM_CCER_CC1P;
    }
    if (init->IC2Polarity == ENCODER_IC_POLARITY_FALLING) {
        init->TIMx->CCER |= TIM_CCER_CC2P;
    }
    
    // Enable both input captures
    init->TIMx->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
    
    // Configure encoder mode - count on both TI1 and TI2 edges (4x resolution)
    init->TIMx->SMCR &= ~TIM_SMCR_SMS;  // Clear SMS
    init->TIMx->SMCR |= TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0;  // SMS = 011 (Encoder mode 3)
    
    // Set auto-reload value
    init->TIMx->ARR = init->MaxCount;
    
    // Reset counter
    init->TIMx->CNT = 0;
    
    
    // Generate update event to load configuration
    init->TIMx->EGR = TIM_EGR_UG;
    
    return DRIVER_STATUS_OK;
}

/**
 * @brief Encoder GPIO initialization - using existing GPIO library
 */
void encoder_gpio_init(TIM_TypeDef *TIMx, GPIO_TypeDef *ch1_port, uint8_t ch1_pin,
                       GPIO_TypeDef *ch2_port, uint8_t ch2_pin, uint8_t af_selection)
{
    (void)TIMx;
    // Configure GPIO pins as alternate function with pull-up
    gpio_init(ch1_port, ch1_pin, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PULLUP);
    gpio_set_af(ch1_port, ch1_pin, af_selection);
    
    gpio_init(ch2_port, ch2_pin, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PULLUP);
    gpio_set_af(ch2_port, ch2_pin, af_selection);
}

/**
 * @brief Start encoder counting
 */
void encoder_start(Encoder_HandleTypeDef *handle)
{
    if (!handle || !handle->TIMx) return;
    
    // Enable timer counter
    handle->TIMx->CR1 |= TIM_CR1_CEN;
}

/**
 * @brief Stop encoder counting
 */
void encoder_stop(Encoder_HandleTypeDef *handle)
{
    if (!handle || !handle->TIMx) return;
    
    // Disable timer counter
    handle->TIMx->CR1 &= ~TIM_CR1_CEN;
}

/**
 * @brief Get current encoder count
 */
uint16_t encoder_get_count(Encoder_HandleTypeDef *handle)
{
    if (!handle || !handle->TIMx) return 0;
    
    return (uint16_t)handle->TIMx->CNT;
}

/**
 * @brief Reset encoder count
 */
void encoder_reset_count(Encoder_HandleTypeDef *handle)
{
    if (!handle || !handle->TIMx) return;
    
    handle->TIMx->CNT = 0;
    handle->TotalCount = 0;
    handle->LastCount = 0;
    handle->LastHardwareCount = 0U;
    handle->LastDirection = 0;
    handle->Speed = 0;
    handle->LastTimeMs = 0U;
}

/**
 * @brief Get encoder direction from hardware
 */
int8_t encoder_get_direction(Encoder_HandleTypeDef *handle)
{
    if (!handle || !handle->TIMx) return 0;
    
    return handle->LastDirection;
}


/**
 * @brief Calculate speed in RPM with direction-aware calculation
 */
int32_t encoder_calculate_speed_rpm(Encoder_HandleTypeDef *handle, uint32_t current_time_ms)
{
    if ((handle == NULL) || (handle->TIMx == NULL) ||
        (handle->CountsPerRevolution == 0U) ||
        (handle->CounterPeriod < 2U)) return 0;

    uint16_t hardware_count = (uint16_t)handle->TIMx->CNT;
    
    // Initialize LastTimeMs on first call to avoid invalid time difference
    if (handle->LastTimeMs == 0) {
        handle->LastTimeMs = current_time_ms;
        handle->LastHardwareCount = hardware_count;
        return 0; // Return 0 on first call since we need time difference
    }
    
    // Calculate time difference
    uint32_t time_diff_ms = current_time_ms - handle->LastTimeMs;
    
    if (time_diff_ms == 0) {
        return handle->Speed; // Return last calculated speed if no time passed
    }
    
    int32_t hardware_delta = (int32_t)hardware_count -
                             (int32_t)handle->LastHardwareCount;
    int32_t half_period = (int32_t)(handle->CounterPeriod / 2U);
    if (hardware_delta > half_period) {
        hardware_delta -= (int32_t)handle->CounterPeriod;
    } else if (hardware_delta < -half_period) {
        hardware_delta += (int32_t)handle->CounterPeriod;
    }

    /* Motor direction convention is the inverse of the timer count direction. */
    int32_t motor_delta = -hardware_delta;
    handle->TotalCount += motor_delta;
    handle->LastDirection = (motor_delta > 0) ? 1 :
                            ((motor_delta < 0) ? -1 : 0);
    handle->Speed = (int32_t)((int64_t)motor_delta * 60000L /
                              ((int64_t)handle->CountsPerRevolution *
                               time_diff_ms));
    
    // Update for next calculation
    handle->LastCount = handle->TotalCount;
    handle->LastHardwareCount = hardware_count;
    handle->LastTimeMs = current_time_ms;
    
    return handle->Speed;
}
