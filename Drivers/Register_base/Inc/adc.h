// filepath: /Users/haoyi/Documents/Code/Electronics_for_embedded_systems/Motor_monitor/Software/Inc/adc.h
/**
 * @file adc.h
 * @author Haoyi Chen
 * @date 2025-08-08
 * @brief STM32F4 ADC register-level driver header
 *
 * @details This file contains ADC function declarations, structures, and macro definitions
 * for STM32F407 series microcontrollers.
 */

#ifndef ADC_H
#define ADC_H

#include "stm32f407xx.h"

/**
 * @brief ADC initialization configuration structure
 */
typedef struct {
    uint32_t Resolution;      /**< ADC resolution (12, 10, 8 or 6 bits) */
    uint32_t Align;           /**< Data alignment (right or left) */
    uint32_t ScanMode;        /**< Scan mode enable/disable */
    uint32_t ContMode;        /**< Continuous conversion mode enable/disable */
    uint32_t ExternalTrigger; /**< External trigger selection */
    uint32_t ExternalTrigConv; /**< External trigger enable and polarity */
    uint32_t DataManagement;  /**< Regular channel DMA configuration */
} ADC_InitTypeDef;

/**
 * @brief ADC channel configuration structure
 */
typedef struct {
    uint32_t Channel;         /**< ADC channel number (0-18) */
    uint32_t Rank;            /**< Rank in the regular sequence */
    uint32_t SamplingTime;    /**< Sample time selection */
} ADC_ChannelConfTypeDef;

/**
 * @name ADC Resolution
 * @{
 */
#define ADC_RESOLUTION_12BIT    0x00000000U /**< 12-bit resolution */
#define ADC_RESOLUTION_10BIT    0x00000100U /**< 10-bit resolution */
#define ADC_RESOLUTION_8BIT     0x00000200U /**< 8-bit resolution */
#define ADC_RESOLUTION_6BIT     0x00000300U /**< 6-bit resolution */
/** @} */

/**
 * @name ADC Data Alignment
 * @{
 */
#define ADC_DATAALIGN_RIGHT     0x00000000U /**< Right alignment */
#define ADC_DATAALIGN_LEFT      0x00000800U /**< Left alignment */
/** @} */

/**
 * @name ADC Scan Mode
 * @{
 */
#define ADC_SCAN_DISABLE        0x00000000U /**< Scan mode disabled */
#define ADC_SCAN_ENABLE         0x00000100U /**< Scan mode enabled */
/** @} */

/**
 * @name ADC Continuous Mode
 * @{
 */
#define ADC_CONTINUOUS_DISABLE  0x00000000U /**< Single conversion mode */
#define ADC_CONTINUOUS_ENABLE   0x00002000U /**< Continuous conversion mode */
/** @} */

/**
 * @name ADC External Trigger Sources for Regular Channels
 * @{
 */
#define ADC_EXTERNALTRIG_T1_CC1   0x00000000U /**< Timer 1 CC1 event  (EXTSEL=0000) */
#define ADC_EXTERNALTRIG_T1_CC2   0x00010000U /**< Timer 1 CC2 event  (EXTSEL=0001) */
#define ADC_EXTERNALTRIG_T1_CC3   0x00020000U /**< Timer 1 CC3 event  (EXTSEL=0010) */
#define ADC_EXTERNALTRIG_T2_CC2   0x00030000U /**< Timer 2 CC2 event  (EXTSEL=0011) */
#define ADC_EXTERNALTRIG_T2_CC3   0x00040000U /**< Timer 2 CC3 event  (EXTSEL=0100) */
#define ADC_EXTERNALTRIG_T2_CC4   0x00050000U /**< Timer 2 CC4 event  (EXTSEL=0101) */
#define ADC_EXTERNALTRIG_T2_TRGO  0x00060000U /**< Timer 2 TRGO event (EXTSEL=0110) */
#define ADC_EXTERNALTRIG_T3_CC1   0x00070000U /**< Timer 3 CC1 event  (EXTSEL=0111) */
#define ADC_EXTERNALTRIG_T3_TRGO  0x00080000U /**< Timer 3 TRGO event (EXTSEL=1000) */
#define ADC_EXTERNALTRIG_T4_CC4   0x00090000U /**< Timer 4 CC4 event  (EXTSEL=1001) */
#define ADC_EXTERNALTRIG_T5_CC1   0x000A0000U /**< Timer 5 CC1 event  (EXTSEL=1010) */
#define ADC_EXTERNALTRIG_T5_CC2   0x000B0000U /**< Timer  5 CC2 event (EXTSEL=1011) */
#define ADC_EXTERNALTRIG_T5_CC3   0x000C0000U /**< Timer 5 CC3 event  (EXTSEL=1100) */
#define ADC_EXTERNALTRIG_T8_CC1   0x000D0000U /**< Timer 8 CC1 event  (EXTSEL=1101) */
#define ADC_EXTERNALTRIG_T8_TRGO  0x000E0000U /**< Timer 8 TRGO event (EXTSEL=1110) */
#define ADC_EXTERNALTRIG_EXTI_11  0x000F0000U /**< EXTI line 11        (EXTSEL=1111) */
/** @} */

/**
 * @name ADC External Trigger Conversion Edge
 * @{
 */
#define ADC_EXTERNALTRIGCONV_NONE     0x00000000U /**< Trigger detection disabled */
#define ADC_EXTERNALTRIGCONV_RISING   0x10000000U /**< Trigger detection on rising edge */
#define ADC_EXTERNALTRIGCONV_FALLING  0x20000000U /**< Trigger detection on falling edge */
#define ADC_EXTERNALTRIGCONV_BOTH     0x30000000U /**< Trigger detection on both edges */
/** @} */

/**
 * @name ADC DMA Mode
 * @{
 */
#define ADC_DMA_DISABLE       0x00000000U /**< DMA disabled */
#define ADC_DMA_SINGLE        0x00000100U /**< DMA single mode */
#define ADC_DMA_CIRCULAR      0x00000500U /**< DMA circular mode */
/** @} */

/**
 * @name ADC Channels
 * @{
 */
#define ADC_CHANNEL_0           0U /**< ADC channel 0 */
#define ADC_CHANNEL_1           1U /**< ADC channel 1 */
#define ADC_CHANNEL_2           2U /**< ADC channel 2 */
#define ADC_CHANNEL_3           3U /**< ADC channel 3 */
#define ADC_CHANNEL_4           4U /**< ADC channel 4 */
#define ADC_CHANNEL_5           5U /**< ADC channel 5 */
#define ADC_CHANNEL_6           6U /**< ADC channel 6 */
#define ADC_CHANNEL_7           7U /**< ADC channel 7 */
#define ADC_CHANNEL_8           8U /**< ADC channel 8 */
#define ADC_CHANNEL_9           9U /**< ADC channel 9 */
#define ADC_CHANNEL_10          10U /**< ADC channel 10 */
#define ADC_CHANNEL_11          11U /**< ADC channel 11 */
#define ADC_CHANNEL_12          12U /**< ADC channel 12 */
#define ADC_CHANNEL_13          13U /**< ADC channel 13 */
#define ADC_CHANNEL_14          14U /**< ADC channel 14 */
#define ADC_CHANNEL_15          15U /**< ADC channel 15 */
#define ADC_CHANNEL_16          16U /**< ADC channel 16 - Temperature sensor */
#define ADC_CHANNEL_17          17U /**< ADC channel 17 - VREFINT */
#define ADC_CHANNEL_18          18U /**< ADC channel 18 - VBAT */
/** @} */

/**
 * @name ADC Channel Rank
 * @{
 */
#define ADC_REGULAR_RANK_1      1U /**< ADC regular sequence rank 1 */
#define ADC_REGULAR_RANK_2      2U /**< ADC regular sequence rank 2 */
#define ADC_REGULAR_RANK_3      3U /**< ADC regular sequence rank 3 */
#define ADC_REGULAR_RANK_4      4U /**< ADC regular sequence rank 4 */
#define ADC_REGULAR_RANK_5      5U /**< ADC regular sequence rank 5 */
#define ADC_REGULAR_RANK_6      6U /**< ADC regular sequence rank 6 */
#define ADC_REGULAR_RANK_7      7U /**< ADC regular sequence rank 7 */
#define ADC_REGULAR_RANK_8      8U /**< ADC regular sequence rank 8 */
#define ADC_REGULAR_RANK_9      9U /**< ADC regular sequence rank 9 */
#define ADC_REGULAR_RANK_10     10U /**< ADC regular sequence rank 10 */
#define ADC_REGULAR_RANK_11     11U /**< ADC regular sequence rank 11 */
#define ADC_REGULAR_RANK_12     12U /**< ADC regular sequence rank 12 */
#define ADC_REGULAR_RANK_13     13U /**< ADC regular sequence rank 13 */
#define ADC_REGULAR_RANK_14     14U /**< ADC regular sequence rank 14 */
#define ADC_REGULAR_RANK_15     15U /**< ADC regular sequence rank 15 */
#define ADC_REGULAR_RANK_16     16U /**< ADC regular sequence rank 16 */
/** @} */

/**
 * @name ADC Sampling Time
 * @{
 */
#define ADC_SAMPLETIME_3CYCLES    0x00000000U /**< 3 cycles sampling time */
#define ADC_SAMPLETIME_15CYCLES   0x00000001U /**< 15 cycles sampling time */
#define ADC_SAMPLETIME_28CYCLES   0x00000002U /**< 28 cycles sampling time */
#define ADC_SAMPLETIME_56CYCLES   0x00000003U /**< 56 cycles sampling time */
#define ADC_SAMPLETIME_84CYCLES   0x00000004U /**< 84 cycles sampling time */
#define ADC_SAMPLETIME_112CYCLES  0x00000005U /**< 112 cycles sampling time */
#define ADC_SAMPLETIME_144CYCLES  0x00000006U /**< 144 cycles sampling time */
#define ADC_SAMPLETIME_480CYCLES  0x00000007U /**< 480 cycles sampling time */
/** @} */

/**
 * @brief Initialize ADC with the specified parameters
 * 
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 * @param init Pointer to ADC initialization structure
 * 
 * @note ADC clock must be enabled separately via RCC registers before calling this function
 */
void adc_init(ADC_TypeDef *ADCx, ADC_InitTypeDef *init);

/**
 * @brief Configure an ADC channel
 * 
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 * @param config Pointer to channel configuration structure
 * 
 * @note Proper GPIO initialization and alternate function configuration must be done separately
 */
void adc_config_channel(ADC_TypeDef *ADCx, ADC_ChannelConfTypeDef *config);

/**
 * @brief Enable the ADC
 * 
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 */
void adc_enable(ADC_TypeDef *ADCx);

/**
 * @brief Disable the ADC
 * 
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 */
void adc_disable(ADC_TypeDef *ADCx);

/**
 * @brief Start ADC regular conversion
 * 
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 */
void adc_start_conversion(ADC_TypeDef *ADCx);

/**
 * @brief Check if ADC conversion is complete
 * 
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 * @return uint8_t 1 if conversion complete, 0 otherwise
 */
uint8_t adc_is_conversion_complete(ADC_TypeDef *ADCx);

/**
 * @brief Get the ADC conversion result
 * 
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 * @return uint16_t ADC conversion result
 */
uint16_t adc_get_conversion_value(ADC_TypeDef *ADCx);

/**
 * @brief Enable ADC DMA request
 * 
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 */
void adc_dma_enable(ADC_TypeDef *ADCx);

/**
 * @brief Disable ADC DMA request
 * 
 * @param ADCx ADC instance (ADC1, ADC2, or ADC3)
 */
void adc_dma_disable(ADC_TypeDef *ADCx);

/**
 * @brief Enable temperature sensor and Vrefint channel
 */
void adc_enable_temp_vref(void);

/**
 * @brief Disable temperature sensor and Vrefint channel
 */
void adc_disable_temp_vref(void);

/**
 * @brief Initialize GPIO pin for ADC input
 * 
 * @param GPIOx GPIO port (GPIOA, GPIOB, etc.)
 * @param pin Pin number (0-15)
 * 
 * @note GPIO clock must be enabled separately via RCC registers
 */
void adc_gpio_init(GPIO_TypeDef *GPIOx, uint8_t pin);

#endif /* ADC_H */
