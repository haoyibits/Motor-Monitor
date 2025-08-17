/**
 * @file encoder.c
 * @author Haoyi Chen
 * @date 2025-08-17
 * @brief Hardware encoder driver - using STM32 hardware encoder mode
 *
 * @details This implementation uses the STM32 timer's hardware encoder mode
 * for quadrature decoding, which is more efficient than input capture
 */

#include "../Inc/encoder.h"
#include "../Inc/rcc.h"

/**
 * @brief Initialize encoder using hardware encoder mode
 * 
 * @details Configures the timer in hardware encoder mode (SMS = 3)
 * for efficient quadrature decoding using TI1 and TI2 inputs
 */
uint8_t encoder_init(Encoder_HandleTypeDef *handle, Encoder_InitTypeDef *init)
{
    if (!handle || !init || !init->TIMx) {
        return 1; // Invalid parameters
    }
    
    // Initialize handle structure
    handle->TIMx = init->TIMx;
    handle->CountsPerRevolution = init->CountsPerRevolution;
    handle->TotalCount = 0;
    handle->LastCount = 0;
    handle->LastHwCount = 0;
    handle->Speed = 0;
    handle->LastTimeMs = 0;
    handle->LastPhaseA = 0;
    handle->LastPhaseB = 0;
    
    // Temporarily disable timer
    init->TIMx->CR1 &= ~TIM_CR1_CEN;
    
    // Configure Channel 1 (TI1) as encoder input
    init->TIMx->CCMR1 &= ~TIM_CCMR1_CC1S;
    init->TIMx->CCMR1 |= TIM_CCMR1_CC1S_0;  // CC1S = 01 (TI1)
    
    // Configure Channel 2 (TI2) as encoder input
    init->TIMx->CCMR1 &= ~TIM_CCMR1_CC2S;
    init->TIMx->CCMR1 |= TIM_CCMR1_CC2S_0;  // CC2S = 01 (TI2)
    
    // Set input filters for noise immunity
    init->TIMx->CCMR1 |= (0x3 << 4);   // IC1F = 0011 (moderate filtering)
    init->TIMx->CCMR1 |= (0x3 << 12);  // IC2F = 0011 (moderate filtering)
    
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
    init->TIMx->ARR = init->MaxCount - 1;
    
    // Reset counter
    init->TIMx->CNT = 0;
    
    // Enable overflow interrupt for counting beyond 16-bit
    init->TIMx->DIER |= TIM_DIER_UIE;
    
    // Enable the appropriate NVIC interrupt based on the timer
    if (init->TIMx == TIM1) {
        NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 1);
        NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
    } else if (init->TIMx == TIM2) {
        NVIC_SetPriority(TIM2_IRQn, 1);
        NVIC_EnableIRQ(TIM2_IRQn);
    } else if (init->TIMx == TIM3) {
        NVIC_SetPriority(TIM3_IRQn, 1);
        NVIC_EnableIRQ(TIM3_IRQn);
    } else if (init->TIMx == TIM4) {
        NVIC_SetPriority(TIM4_IRQn, 1);
        NVIC_EnableIRQ(TIM4_IRQn);
    } else if (init->TIMx == TIM5) {
        NVIC_SetPriority(TIM5_IRQn, 1);
        NVIC_EnableIRQ(TIM5_IRQn);
    }
    
    // Generate update event to load configuration
    init->TIMx->EGR = TIM_EGR_UG;
    
    return 0; // Success
}

/**
 * @brief Encoder GPIO initialization - using existing GPIO library
 */
void encoder_gpio_init(TIM_TypeDef *TIMx, GPIO_TypeDef *ch1_port, uint8_t ch1_pin,
                       GPIO_TypeDef *ch2_port, uint8_t ch2_pin, uint8_t af_selection)
{
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
    handle->LastHwCount = 0;
}

/**
 * @brief Get encoder direction from hardware
 */
int8_t encoder_get_direction(Encoder_HandleTypeDef *handle)
{
    if (!handle || !handle->TIMx) return 0;
    
    // Check timer direction bit (DIR in CR1 register)
    // In encoder mode, this bit is automatically set by hardware
    // based on the quadrature signals
    if (handle->TIMx->CR1 & TIM_CR1_DIR) {
        return -1; // Counting down (reverse)
    } else {
        return 1;  // Counting up (forward)
    }
}

/**
 * @brief Update encoder total count with overflow handling
 */
void encoder_update(Encoder_HandleTypeDef *handle)
{
    if (!handle || !handle->TIMx) return;
    
    uint16_t current_count = encoder_get_count(handle);
    int16_t count_diff = (int16_t)(current_count - handle->LastHwCount);
    
    // Handle counter overflow/underflow
    uint16_t max_count = handle->TIMx->ARR + 1;
    int16_t half_max = (int16_t)(max_count / 2);
    
    if (count_diff > half_max) {
        // Counter underflowed (wrapped from 0 to max)
        count_diff -= (int16_t)max_count;
    } else if (count_diff < -half_max) {
        // Counter overflowed (wrapped from max to 0)
        count_diff += (int16_t)max_count;
    }
    
    handle->TotalCount += count_diff;
    handle->LastHwCount = current_count;  // Update hardware count tracker
}

/**
 * @brief Calculate speed in RPM with time tracking
 */
int32_t encoder_calculate_speed_rpm(Encoder_HandleTypeDef *handle, uint32_t current_time_ms)
{
    if (!handle || handle->CountsPerRevolution == 0) return 0;
    
    // Update total count from hardware counter
    encoder_update(handle);
    
    // Initialize LastTimeMs on first call to avoid invalid time difference
    if (handle->LastTimeMs == 0) {
        handle->LastTimeMs = current_time_ms;
        handle->LastCount = handle->TotalCount;
        return 0; // Return 0 on first call since we need time difference
    }
    
    // Calculate time difference
    uint32_t time_diff_ms = current_time_ms - handle->LastTimeMs;
    
    if (time_diff_ms == 0) {
        return handle->Speed; // Return last calculated speed if no time passed
    }
    
    // Calculate count difference since last speed calculation
    int32_t count_diff = handle->TotalCount - handle->LastCount;
    
    // Calculate RPM: (count_diff / CPR) * (60000 ms/min / time_diff_ms)
    handle->Speed = (int32_t)((int64_t)count_diff * 60000L / 
                              ((int64_t)handle->CountsPerRevolution * time_diff_ms));
    
    // Update for next calculation
    handle->LastCount = handle->TotalCount;
    handle->LastTimeMs = current_time_ms;
    
    return handle->Speed;
}

/**
 * @brief Generic timer IRQ handler for encoder overflow/underflow
 * 
 * @details This function should be called from the appropriate timer IRQ handler
 * 
 * @param handle Pointer to encoder handle structure
 */
void encoder_timer_irq_handler(Encoder_HandleTypeDef *handle)
{
    if (!handle || !handle->TIMx) return;
    
    // Handle timer overflow/underflow interrupt
    if (handle->TIMx->SR & TIM_SR_UIF) {
        handle->TIMx->SR &= ~TIM_SR_UIF;  // Clear update interrupt flag
        
        // Determine direction and update total count
        if (handle->TIMx->CR1 & TIM_CR1_DIR) {
            // Counting down (underflow occurred)
            handle->TotalCount -= (int32_t)(handle->TIMx->ARR + 1);
        } else {
            // Counting up (overflow occurred)
            handle->TotalCount += (int32_t)(handle->TIMx->ARR + 1);
        }
    }
}
