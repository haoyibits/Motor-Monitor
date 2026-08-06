/**
 * @file tim.h
 * @author Haoyi Chen
 * @date 2025-08-03
 * @brief STM32F4 Timer register-level driver header
 *
 * @details This file contains Timer function declarations, structures, and macro definitions
 * for STM32F407 series microcontrollers.
 */

#ifndef TIM_H
#define TIM_H

#include "stm32f407xx.h"

/**
 * @brief Timer initialization configuration structure
 */
typedef struct {
    uint32_t Prescaler;        /**< Prescaler value for timer clock */
    uint32_t Period;           /**< Auto-reload value for timer counter */
    uint8_t ClockDivision;     /**< Clock division */
    uint8_t CounterMode;       /**< Counter direction mode */
} TIM_InitTypeDef;

/**
 * @brief Timer PWM configuration structure
 */
typedef struct {
    uint32_t Channel;          /**< Channel number (1-4) */
    uint32_t Pulse;            /**< PWM pulse value (duty cycle) */
    uint8_t OCMode;            /**< Output compare mode */
    uint8_t OCPolarity;        /**< Output polarity */
} TIM_PWM_ConfigTypeDef;

/**
 * @name Timer Counter Modes
 * @{
 */
#define TIM_COUNTERMODE_UP             0x00  /**< Up-counting mode */
#define TIM_COUNTERMODE_DOWN           0x01  /**< Down-counting mode */
#define TIM_COUNTERMODE_CENTERALIGNED1 0x02  /**< Center-aligned mode 1 */
#define TIM_COUNTERMODE_CENTERALIGNED2 0x03  /**< Center-aligned mode 2 */
#define TIM_COUNTERMODE_CENTERALIGNED3 0x04  /**< Center-aligned mode 3 */
/** @} */

/**
 * @name Output Compare Modes
 * @{
 */
#define TIM_OCMODE_FROZEN           0x00  /**< Frozen mode */
#define TIM_OCMODE_ACTIVE           0x01  /**< Active mode */
#define TIM_OCMODE_INACTIVE         0x02  /**< Inactive mode */
#define TIM_OCMODE_TOGGLE           0x03  /**< Toggle mode */
#define TIM_OCMODE_PWM1             0x06  /**< PWM mode 1 */
#define TIM_OCMODE_PWM2             0x07  /**< PWM mode 2 */
/** @} */

/**
 * @name Output Compare Polarity
 * @{
 */
#define TIM_OCPOLARITY_HIGH         0x00  /**< Active high polarity */
#define TIM_OCPOLARITY_LOW          0x01  /**< Active low polarity */
/** @} */

/**
 * @name Clock Division
 * @{
 */
#define TIM_CLOCKDIVISION_DIV1      0x00  /**< Clock not divided */
#define TIM_CLOCKDIVISION_DIV2      0x01  /**< Clock divided by 2 */
#define TIM_CLOCKDIVISION_DIV4      0x02  /**< Clock divided by 4 */
/** @} */

/**
 * @name Timer Channels
 * @{
 */
#define TIM_CHANNEL_1               0x00  
#define TIM_CHANNEL_2               0x01  
#define TIM_CHANNEL_3               0x02  
#define TIM_CHANNEL_4               0x03  
/** @} */

/**
 * @brief Initialize a timer with basic parameters
 * 
 * @param TIMx Timer instance (TIM1-TIM14)
 * @param init Pointer to timer initialization structure
 * 
 * @note Timer clock must be enabled separately via RCC registers
 */
void tim_init(TIM_TypeDef *TIMx, TIM_InitTypeDef *init);

/**
 * @brief Configure timer channel for PWM output
 * 
 * @param TIMx Timer instance
 * @param config Pointer to PWM configuration structure
 * 
 * @note The corresponding GPIO pin must be configured separately as alternate function
 */
void tim_pwm_config(TIM_TypeDef *TIMx, TIM_PWM_ConfigTypeDef *config);

/**
 * @brief Enable timer counter
 * 
 * @param TIMx Timer instance
 */
void tim_enable(TIM_TypeDef *TIMx);

/**
 * @brief Disable timer counter
 * 
 * @param TIMx Timer instance
 */
void tim_disable(TIM_TypeDef *TIMx);

/**
 * @brief Set timer PWM duty cycle
 * 
 * @param TIMx Timer instance
 * @param channel Channel number (0-3 for CH1-CH4)
 * @param pulse PWM pulse value
 * 
 * @note PWM duty cycle = pulse / period * 100%
 */
void tim_set_pwm_duty(TIM_TypeDef *TIMx, uint8_t channel, uint32_t pulse);

/**
 * @brief Enable timer update interrupt
 * 
 * @param TIMx Timer instance
 * 
 * @note NVIC must be configured separately to handle the interrupt
 */
void tim_enable_update_interrupt(TIM_TypeDef *TIMx);

/**
 * @brief Disable timer update interrupt
 * 
 * @param TIMx Timer instance
 */
void tim_disable_update_interrupt(TIM_TypeDef *TIMx);

/**
 * @brief Check if update interrupt flag is set
 * 
 * @param TIMx Timer instance
 * @return uint8_t 1 if flag is set, 0 otherwise
 */
uint8_t tim_get_update_flag(TIM_TypeDef *TIMx);

/**
 * @brief Clear update interrupt flag
 * 
 * @param TIMx Timer instance
 */
void tim_clear_update_flag(TIM_TypeDef *TIMx);

#endif /* TIM_H */
