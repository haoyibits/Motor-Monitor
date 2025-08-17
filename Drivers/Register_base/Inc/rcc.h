/**
 * @file rcc.h
 * @author Haoyi Chen
 * @date 2025-08-08
 * @brief STM32F4 RCC (Reset and Clock Control) register-level driver
 *
 * @details This file provides functions for configuring the system clock and
 * peripheral clocks for STM32F407 series microcontrollers. All operations are
 * performed at the register level, without using HAL or LL libraries.
 *
 * Created for personal learning and embedded systems experimentation.
 */

#ifndef __RCC_H
#define __RCC_H

#include <stdint.h>
#include <stm32f4xx.h>

extern uint32_t system_clock;
/**
 * @brief System clock sources
 */
typedef enum {
    RCC_CLOCK_HSI = 0,   /**< High-Speed Internal Clock (16MHz) */
    RCC_CLOCK_HSE = 1,   /**< High-Speed External Clock */
    RCC_CLOCK_PLL = 2    /**< Phase-Locked Loop */
} RCC_ClockSourceTypeDef;

/**
 * @brief AHB prescaler values
 */
typedef enum {
    RCC_AHB_DIV1 = 0,    /**< AHB not divided */
    RCC_AHB_DIV2 = 8,    /**< AHB divided by 2 */
    RCC_AHB_DIV4 = 9,    /**< AHB divided by 4 */
    RCC_AHB_DIV8 = 10,   /**< AHB divided by 8 */
    RCC_AHB_DIV16 = 11,  /**< AHB divided by 16 */
    RCC_AHB_DIV64 = 12,  /**< AHB divided by 64 */
    RCC_AHB_DIV128 = 13, /**< AHB divided by 128 */
    RCC_AHB_DIV256 = 14, /**< AHB divided by 256 */
    RCC_AHB_DIV512 = 15  /**< AHB divided by 512 */
} RCC_AHB_PrescalerTypeDef;

/**
 * @brief APB prescaler values
 */
typedef enum {
    RCC_APB_DIV1 = 0,    /**< APB not divided */
    RCC_APB_DIV2 = 4,    /**< APB divided by 2 */
    RCC_APB_DIV4 = 5,    /**< APB divided by 4 */
    RCC_APB_DIV8 = 6,    /**< APB divided by 8 */
    RCC_APB_DIV16 = 7    /**< APB divided by 16 */
} RCC_APB_PrescalerTypeDef;

/**
 * @brief System clock configuration structure
 */
typedef struct {
    RCC_ClockSourceTypeDef ClockSource;     /**< System clock source */
    uint8_t PLL_M;                          /**< PLL division factor for main PLL input clock */
    uint16_t PLL_N;                         /**< PLL multiplication factor for VCO */
    uint8_t PLL_P;                          /**< PLL division factor for main system clock */
    uint8_t PLL_Q;                          /**< PLL division factor for USB OTG FS, SDIO */
    RCC_AHB_PrescalerTypeDef AHB_Prescaler; /**< AHB prescaler */
    RCC_APB_PrescalerTypeDef APB1_Prescaler;/**< APB1 prescaler */
    RCC_APB_PrescalerTypeDef APB2_Prescaler;/**< APB2 prescaler */
    uint8_t Latency;                        /**< Flash latency (wait states) */
} RCC_ClockConfigTypeDef;

/**
 * @brief Configure the system clock
 * 
 * @param config Pointer to clock configuration structure
 * @return uint8_t 0 if successful, 1 if error occurred
 */
uint8_t rcc_system_clock_config(RCC_ClockConfigTypeDef *config);

/**
 * @brief Enable peripheral clock for a GPIO port
 * 
 * @param GPIOx Pointer to GPIO port (e.g. GPIOA, GPIOB)
 */
void rcc_enable_gpio_clock(GPIO_TypeDef *GPIOx);

/**
 * @brief Enable peripheral clock for a timer
 * 
 * @param TIMx Pointer to timer (e.g. TIM1, TIM2)
 */
void rcc_enable_tim_clock(TIM_TypeDef *TIMx);

/**
 * @brief Enable peripheral clock for ADC
 * 
 * @param ADCx Pointer to ADC (e.g. ADC1, ADC2)
 */
void rcc_enable_adc_clock(ADC_TypeDef *ADCx);

/**
 * @brief Enable peripheral clock for I2C
 * 
 * @param I2Cx Pointer to I2C (e.g. I2C1, I2C2)
 */
void rcc_enable_i2c_clock(I2C_TypeDef *I2Cx);

/**
 * @brief DMA Clock Enable Definitions
 *
 * @brief Enable DMA clocks
 * 
 * @param DMA_number Pointer to DMA instance (DMA1 or DMA2)
 */
void rcc_enable_dma_clock(DMA_TypeDef *DMA_number);

/**
 * @brief Enable peripheral clock for USART/UART
 * 
 * @param USARTx Pointer to USART (e.g. USART1, USART2, UART4)
 */
void rcc_enable_usart_clock(USART_TypeDef *USARTx);

/**
 * @brief Configure and set system to default 168MHz frequency using PLL
 *
 * @details This is a convenient function to set the system clock to 168MHz,
 *          which is the maximum frequency for STM32F407. It uses the HSI
 *          as clock source, and configures the PLL appropriately.
 * 
 * @param use_hse 0 to use internal oscillator (HSI), 1 to use external crystal (HSE)
 * @param hse_freq External crystal frequency in Hz (typically 8000000 for 8MHz)
 *                 This parameter is ignored when use_hse = 0
 * @return uint8_t 0 if successful, 1 if error occurred
 */
uint8_t rcc_config_max_frequency(uint8_t use_hse, uint32_t hse_freq);

/**
 * @brief Get APB1 peripheral clock frequency
 * 
 * @return uint32_t APB1 clock frequency in Hz
 */
uint32_t rcc_get_pclk1_freq(void);

/**
 * @brief Get APB2 peripheral clock frequency
 * 
 * @return uint32_t APB2 clock frequency in Hz
 */
uint32_t rcc_get_pclk2_freq(void);

#endif /* __RCC_H */
