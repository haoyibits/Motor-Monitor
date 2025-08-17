/**
 * @file adc.c
 * @author Haoyi Chen
 * @date 2025-08-08
 * @brief STM32F4 ADC register-level driver
 *
 * @details This file provides basic ADC initialization, configuration and data acquisition
 * functions for STM32F407 series microcontrollers. All operations are performed at the 
 * register level, without using HAL or LL libraries.
 *
 * Created for personal learning and embedded systems experimentation.
 */

#include "../Inc/adc.h"
#include "gpio.h"

/**
 * @brief Initialize ADC with the specified parameters
 * 
 * @details Configures ADC by setting resolution, data alignment, scan mode, 
 *          continuous conversion mode, and trigger settings.
 *
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 * @param init Pointer to ADC initialization structure
 * 
 * @note ADC clock must be enabled separately via RCC registers before calling this function
 */
void adc_init(ADC_TypeDef *ADCx, ADC_InitTypeDef *init) {
    /* Configure ADC clock prescaler to divide by 4 (PCLK2/4)
     * With PCLK2=84MHz, ADC clock will be 21MHz, optimal for ADC operation */
    ADC->CCR &= ~ADC_CCR_ADCPRE;         /* Clear ADCPRE bits */
    ADC->CCR |= ADC_CCR_ADCPRE_0;        /* Set ADCPRE to 01 (PCLK2/4) */
    
    /* Disable the ADC before configuration */
    ADCx->CR2 &= ~ADC_CR2_ADON;
    
    /* Wait for ADC to be fully disabled before configuration */
    uint32_t timeout = 10000;
    while((ADCx->CR2 & ADC_CR2_ADON) && (timeout--));
    
    /* 完全重置寄存器，然后重新配置 */
    ADCx->CR1 = 0;
    ADCx->CR2 = 0;
    
    /* Configure ADC resolution */
    ADCx->CR1 |= init->Resolution;
    
    /* Configure scan mode */
    ADCx->CR1 |= init->ScanMode;
    
    /* Configure data alignment */
    ADCx->CR2 |= init->Align;
    
    /* Configure continuous conversion mode */
    ADCx->CR2 |= init->ContMode;
    
    /* Configure external trigger source */
    ADCx->CR2 |= init->ExternalTrigger;
    
    /* Configure external trigger edge */
    ADCx->CR2 |= init->ExternalTrigConv;
    
    /* Configure DMA mode */
    if (init->DataManagement != ADC_DMA_DISABLE) {
        /* Enable DMA mode */
        ADCx->CR2 |= ADC_CR2_DMA;
        
        /* 修复: 在连续模式下始终设置DDS位，这是关键 */
        if (init->ContMode == ADC_CONTINUOUS_ENABLE) {
            ADCx->CR2 |= ADC_CR2_DDS;  /* 确保在连续模式下启用DDS位 */
        }
        else if (init->DataManagement == ADC_DMA_CIRCULAR) {
            ADCx->CR2 |= ADC_CR2_DDS;  /* 循环DMA模式下也需要启用DDS位 */
        }
    }
}

/**
 * @brief Configure an ADC channel
 * 
 * @details Sets the sampling time and channel sequence for the specified ADC channel.
 *
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 * @param config Pointer to channel configuration structure
 * 
 * @note Proper GPIO initialization and alternate function configuration must be done separately
 */
void adc_config_channel(ADC_TypeDef *ADCx, ADC_ChannelConfTypeDef *config) {
    uint32_t channel = config->Channel;
    uint32_t rank = config->Rank;
    
    /* Configure the channel sequence length */
    ADCx->SQR1 &= ~ADC_SQR1_L;
    ADCx->SQR1 |= ((rank - 1) << 20);
    
    /* Configure the channel sequence */
    if (rank <= 6) {
        uint32_t shift = 5 * (rank - 1);
        ADCx->SQR3 &= ~(0x1F << shift);
        ADCx->SQR3 |= (channel << shift);
    } else if (rank <= 12) {
        uint32_t shift = 5 * (rank - 7);
        ADCx->SQR2 &= ~(0x1F << shift);
        ADCx->SQR2 |= (channel << shift);
    } else {
        uint32_t shift = 5 * (rank - 13);
        ADCx->SQR1 &= ~(0x1F << shift);
        ADCx->SQR1 |= (channel << shift);
    }
    
    /* Configure the channel sampling time */
    if (channel <= 9) {
        uint32_t shift = 3 * channel;
        ADCx->SMPR2 &= ~(0x7 << shift);
        ADCx->SMPR2 |= (config->SamplingTime << shift);
    } else {
        uint32_t shift = 3 * (channel - 10);
        ADCx->SMPR1 &= ~(0x7 << shift);
        ADCx->SMPR1 |= (config->SamplingTime << shift);
    }
}

/**
 * @brief Enable the ADC
 * 
 * @details Sets the ADON bit in the CR2 register to enable the ADC.
 *
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 */
void adc_enable(ADC_TypeDef *ADCx) {
    /* Enable the ADC */
    ADCx->CR2 |= ADC_CR2_ADON;
    
    /* Wait a bit for stabilization (minimum 2 ADC clock cycles) */
    uint32_t i;
    for (i = 0; i < 10000; i++) {
        __NOP();
    }
}

/**
 * @brief Disable the ADC
 * 
 * @details Clears the ADON bit in the CR2 register to disable the ADC.
 *
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 */
void adc_disable(ADC_TypeDef *ADCx) {
    /* Disable the ADC */
    ADCx->CR2 &= ~ADC_CR2_ADON;
}

/**
 * @brief Start ADC regular conversion
 * 
 * @details Sets the SWSTART bit in the CR2 register to start regular conversion.
 *
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 */
void adc_start_conversion(ADC_TypeDef *ADCx) {
    /* Start conversion by setting SWSTART bit */
    ADCx->CR2 |= ADC_CR2_SWSTART;
}

/**
 * @brief Check if ADC conversion is complete
 * 
 * @details Reads the EOC flag in the SR register to determine if conversion is complete.
 *
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 * @return uint8_t 1 if conversion complete, 0 otherwise
 */
uint8_t adc_is_conversion_complete(ADC_TypeDef *ADCx) {
    return (ADCx->SR & ADC_SR_EOC) ? 1 : 0;
}

/**
 * @brief Get the ADC conversion result
 * 
 * @details Reads the converted value from the ADC data register.
 *
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 * @return uint16_t ADC conversion result
 */
uint16_t adc_get_conversion_value(ADC_TypeDef *ADCx) {
    return (uint16_t)ADCx->DR;
}

/**
 * @brief Enable ADC DMA request
 * 
 * @details Sets the DMA bit in the CR2 register to enable DMA requests.
 *
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 */
void adc_dma_enable(ADC_TypeDef *ADCx) {
    ADCx->CR2 |= ADC_CR2_DMA;
}

/**
 * @brief Disable ADC DMA request
 * 
 * @details Clears the DMA bit in the CR2 register to disable DMA requests.
 *
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 */
void adc_dma_disable(ADC_TypeDef *ADCx) {
    ADCx->CR2 &= ~ADC_CR2_DMA;
}

/**
 * @brief Enable temperature sensor and Vrefint channel
 * 
 * @details Sets the TSVREFE bit in the CCR register to enable the temperature sensor
 *          and internal reference voltage channels.
 */
void adc_enable_temp_vref(void) {
    /* Enable temperature sensor and internal reference voltage */
    ADC->CCR |= ADC_CCR_TSVREFE;
}

/**
 * @brief Disable temperature sensor and Vrefint channel
 * 
 * @details Clears the TSVREFE bit in the CCR register to disable the temperature sensor
 *          and internal reference voltage channels.
 */
void adc_disable_temp_vref(void) {
    /* Disable temperature sensor and internal reference voltage */
    ADC->CCR &= ~ADC_CCR_TSVREFE;
}

/**
 * @brief Initialize GPIO pin for ADC input
 * 
 * @details Configures a GPIO pin as an analog input for ADC.
 *
 * @param GPIOx GPIO port (GPIOA, GPIOB, etc.)
 * @param pin Pin number (0-15)
 * 
 * @note GPIO clock must be enabled separately via RCC registers
 */
void adc_gpio_init(GPIO_TypeDef *GPIOx, uint8_t pin) {
    /* Configure GPIO pin as analog mode (mode = 3) */
    gpio_init(GPIOx, pin, 0x03, 0, 0, 0);
}
