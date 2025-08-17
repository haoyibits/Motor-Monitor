/**
 * @file tim.c
 * @author Haoyi Chen
 * @date 2025-08-03
 * @brief STM32F4 Timer register-level driver
 *
 * @details This file provides basic timer initialization, PWM generation and interrupt
 * handling functions for STM32F407 series microcontrollers. All operations are
 * performed at the register level, without using HAL or LL libraries.
 *
 * Created for personal learning and embedded systems experimentation.
 */

#include "../Inc/tim.h"

/**
 * @brief Initialize a timer with basic parameters
 * 
 * @details Configures timer by setting PSC (prescaler), ARR (period), clock division,
 *          and counter mode. Generates update event to load new values.
 *
 * @param TIMx Timer instance (TIM1-TIM14)
 * @param init Pointer to timer initialization structure
 * 
 * @note Timer clock must be enabled separately via RCC registers before calling this function
 */
void tim_init(TIM_TypeDef *TIMx, TIM_InitTypeDef *init) {
    /* Set the prescaler value */
    TIMx->PSC = init->Prescaler;
    
    /* Set the auto-reload value */
    TIMx->ARR = init->Period;
    
    /* Set clock division */
    TIMx->CR1 &= ~TIM_CR1_CKD;
    TIMx->CR1 |= init->ClockDivision << 8;
    
    /* Set counter mode */
    TIMx->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS);
    if (init->CounterMode == TIM_COUNTERMODE_DOWN) {
        TIMx->CR1 |= TIM_CR1_DIR;  // Set direction bit for down-counting
    } else if (init->CounterMode >= TIM_COUNTERMODE_CENTERALIGNED1 && 
               init->CounterMode <= TIM_COUNTERMODE_CENTERALIGNED3) {
        TIMx->CR1 |= ((init->CounterMode - 1) << 5);  // Configure center-aligned mode
    }
    
    /* Generate an update event to reload the Prescaler and the ARR */
    TIMx->EGR = TIM_EGR_UG;
}

/**
 * @brief Configure timer channel for PWM output
 * 
 * @details Sets CCRx (pulse width), configures OCx mode, enables preload,
 *          sets polarity, and enables the channel output. Handles MOE for advanced timers.
 *          
 * @param TIMx Timer instance
 * @param config Pointer to PWM configuration structure
 * 
 * @note GPIO pins must be separately configured in alternate function mode
 */
void tim_pwm_config(TIM_TypeDef *TIMx, TIM_PWM_ConfigTypeDef *config) {
    uint32_t channel = config->Channel;
    uint32_t ccer_offset;
    
    /* Set the capture compare register value */
    switch (channel) {
        case TIM_CHANNEL_1:
            TIMx->CCR1 = config->Pulse;
            ccer_offset = 0;
            break;
        case TIM_CHANNEL_2:
            TIMx->CCR2 = config->Pulse;
            ccer_offset = 4;
            break;
        case TIM_CHANNEL_3:
            TIMx->CCR3 = config->Pulse;
            ccer_offset = 8;
            break;
        case TIM_CHANNEL_4:
            TIMx->CCR4 = config->Pulse;
            ccer_offset = 12;
            break;
        default:
            // Invalid channel, do nothing
            return;
    }
    
    /* Configure the OCx mode */
    if (channel <= TIM_CHANNEL_2) {
        // Channels 1-2 are in CCMR1 register
        TIMx->CCMR1 &= ~(0x7 << (4 + (channel * 8)));  // Clear OCxM bits
        TIMx->CCMR1 |= (config->OCMode << (4 + (channel * 8)));  // Set new mode
        
        /* Set the output compare preload enable bit (OCxPE) */
        TIMx->CCMR1 |= (1 << (3 + (channel * 8)));
    } else {
        // Channels 3-4 are in CCMR2 register
        TIMx->CCMR2 &= ~(0x7 << (4 + ((channel - 2) * 8)));  // Clear OCxM bits
        TIMx->CCMR2 |= (config->OCMode << (4 + ((channel - 2) * 8)));  // Set new mode
        
        /* Set the output compare preload enable bit (OCxPE) */
        TIMx->CCMR2 |= (1 << (3 + ((channel - 2) * 8)));
    }
    
    /* Set the output polarity */
    TIMx->CCER &= ~(0x1 << (1 + ccer_offset));  // Clear CCxP bit
    TIMx->CCER |= (config->OCPolarity << (1 + ccer_offset));  // Set polarity
    
    /* Enable the output */
    TIMx->CCER |= (0x1 << ccer_offset);  // Set CCxE bit
    
    /* For TIM1 and TIM8 (advanced timers), enable the main output */
    if (TIMx == TIM1 || TIMx == TIM8) {
        TIMx->BDTR |= TIM_BDTR_MOE;  // Required for advanced timers to output signal
    }
}

/**
 * @brief Enable timer counter
 * 
 * @details Sets CEN bit in CR1 register to start timer counting.
 * 
 * @param TIMx Timer instance
 */
void tim_enable(TIM_TypeDef *TIMx) {
    TIMx->CR1 |= TIM_CR1_CEN;
}

/**
 * @brief Disable timer counter
 * 
 * @details Clears CEN bit in CR1 register to stop timer.
 * 
 * @param TIMx Timer instance
 */
void tim_disable(TIM_TypeDef *TIMx) {
    TIMx->CR1 &= ~TIM_CR1_CEN;
}

/**
 * @brief Set timer PWM duty cycle
 * 
 * @details Updates CCRx register for specified channel to change duty cycle.
 *          Duty cycle = CCRx / ARR * 100%
 * 
 * @param TIMx Timer instance
 * @param channel Channel number (0-3 for CH1-CH4)
 * @param pulse PWM pulse value
 */
void tim_set_pwm_duty(TIM_TypeDef *TIMx, uint8_t channel, uint32_t pulse) {
    switch (channel) {
        case TIM_CHANNEL_1:
            TIMx->CCR1 = pulse;
            break;
        case TIM_CHANNEL_2:
            TIMx->CCR2 = pulse;
            break;
        case TIM_CHANNEL_3:
            TIMx->CCR3 = pulse;
            break;
        case TIM_CHANNEL_4:
            TIMx->CCR4 = pulse;
            break;
        default:
            // Invalid channel, do nothing
            break;
    }
}

/**
 * @brief Enable timer update interrupt
 * 
 * @details Sets UIE bit in DIER register to enable update interrupt generation.
 * 
 * @param TIMx Timer instance
 * 
 * @note NVIC must be configured separately to handle the interrupt
 */
void tim_enable_update_interrupt(TIM_TypeDef *TIMx) {
    TIMx->DIER |= TIM_DIER_UIE;
}

/**
 * @brief Disable timer update interrupt
 * 
 * @details Clears UIE bit in DIER register to disable update interrupt.
 * 
 * @param TIMx Timer instance
 */
void tim_disable_update_interrupt(TIM_TypeDef *TIMx) {
    TIMx->DIER &= ~TIM_DIER_UIE;
}

/**
 * @brief Check if update interrupt flag is set
 * 
 * @details Reads UIF bit in SR register to determine if an update event occurred.
 * 
 * @param TIMx Timer instance
 * @return uint8_t 1 if flag is set, 0 otherwise
 */
uint8_t tim_get_update_flag(TIM_TypeDef *TIMx) {
    return (TIMx->SR & TIM_SR_UIF) ? 1 : 0;
}

/**
 * @brief Clear update interrupt flag
 * 
 * @details Clears UIF bit in SR register by writing 0 to it.
 * 
 * @param TIMx Timer instance
 */
void tim_clear_update_flag(TIM_TypeDef *TIMx) {
    TIMx->SR &= ~TIM_SR_UIF;
}
