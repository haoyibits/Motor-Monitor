/**
 * @file gpio.h
 * @author Haoyi Chen
 * @date 2025-08-03
 * @brief STM32F4 GPIO register-level driver header
 *
 * @details This file contains GPIO function declarations and constant definitions
 * for STM32F407 series microcontrollers.
 */

#ifndef GPIO_H
#define GPIO_H

#include "stm32f407xx.h"

/**
 * @name GPIO Pin Mode Definitions
 * @{
 */
#define GPIO_MODE_INPUT     0x0  /**< Input mode */
#define GPIO_MODE_OUTPUT    0x1  /**< Output mode */
#define GPIO_MODE_AF        0x2  /**< Alternate function mode */
#define GPIO_MODE_ANALOG    0x3  /**< Analog mode */
/** @} */

/**
 * @name GPIO Output Type Definitions
 * @{
 */
#define GPIO_OTYPE_PP       0x0  /**< Push-pull output */
#define GPIO_OTYPE_OD       0x1  /**< Open-drain output */
/** @} */
    
/**
 * @name GPIO Speed Definitions
 * @{
 */
#define GPIO_SPEED_LOW      0x0  /**< Low speed */
#define GPIO_SPEED_MED      0x1  /**< Medium speed */
#define GPIO_SPEED_HIGH     0x2  /**< High speed */
#define GPIO_SPEED_VHIGH    0x3  /**< Very high speed */
/** @} */
    
/**
 * @name GPIO Pull-up/Pull-down Definitions
 * @{
 */
#define GPIO_NOPULL         0x0  /**< No pull-up, pull-down */
#define GPIO_PULLUP         0x1  /**< Pull-up */
#define GPIO_PULLDOWN       0x2  /**< Pull-down */
#define GPIO_RESERVED       0x3  /**< Reserved */
/** @} */

/**
 * @name GPIO Interrupt Trigger Definitions
 * @{
 */
#define GPIO_INT_RISING     0x1  /**< Rising edge trigger */
#define GPIO_INT_FALLING    0x2  /**< Falling edge trigger */
#define GPIO_INT_BOTH       0x3  /**< Both edges trigger */
/** @} */

/**
 * @name GPIO Alternate Function Values for STM32F407,
 * @note Reference from dm00037051.pdf Alternate function mapping
 * @{
 */
/** @name System Functions */
#define GPIO_AF_SYSTEM      0x00  /**< System function */
#define GPIO_AF_MCO         0x00  /**< MCO (MCO1 and MCO2) */
#define GPIO_AF_SWJ         0x00  /**< SWJ (SWD and JTAG) */
#define GPIO_AF_TRACE       0x00  /**< TRACE */

/** @name Timer Functions */
#define GPIO_AF_TIM1        0x01  
#define GPIO_AF_TIM2        0x01  
#define GPIO_AF_TIM3        0x02  
#define GPIO_AF_TIM4        0x02  
#define GPIO_AF_TIM5        0x02  
#define GPIO_AF_TIM8        0x03  
#define GPIO_AF_TIM9        0x03  
#define GPIO_AF_TIM10       0x03  
#define GPIO_AF_TIM11       0x03  
#define GPIO_AF_TIM12       0x09  
#define GPIO_AF_TIM13       0x09  
#define GPIO_AF_TIM14       0x09  

/** @name Serial Communication */
#define GPIO_AF_USART1      0x07  
#define GPIO_AF_USART2      0x07  
#define GPIO_AF_USART3      0x07  
#define GPIO_AF_UART4       0x08  
#define GPIO_AF_UART5       0x08  
#define GPIO_AF_USART6      0x08  

/** @name I2C Functions */
#define GPIO_AF_I2C1        0x04  
#define GPIO_AF_I2C2        0x04  
#define GPIO_AF_I2C3        0x04  

/** @name SPI Functions */
#define GPIO_AF_SPI1        0x05  
#define GPIO_AF_SPI2        0x05  
#define GPIO_AF_I2S2        0x05  
#define GPIO_AF_SPI3        0x06  
#define GPIO_AF_I2S3        0x06  
#define GPIO_AF_SPI4        0x05  
#define GPIO_AF_SPI5        0x05  
#define GPIO_AF_SPI6        0x05  

/** @name CAN & USB Functions */
#define GPIO_AF_CAN1        0x09  
#define GPIO_AF_CAN2        0x09  
#define GPIO_AF_OTG_FS      0x0A  
#define GPIO_AF_OTG_HS      0x0A  

/** @name Ethernet Functions */
#define GPIO_AF_ETH         0x0B  /**< Ethernet */

/** @name FSMC/SDIO Functions */
#define GPIO_AF_FSMC        0x0C  /**< FSMC */
#define GPIO_AF_SDIO        0x0C  /**< SDIO */

/** @name DCMI Functions */
#define GPIO_AF_DCMI        0x0D  /**< DCMI */

/** @name Event Output */
#define GPIO_AF_EVENTOUT    0x0F  /**< EVENTOUT */
/** @} */

/**
 * @brief Initialize a GPIO pin
 * 
 * @param GPIOx Pointer to GPIO port (GPIOA, GPIOB, etc.)
 * @param pin Pin number (0-15)
 * @param mode Pin mode (GPIO_MODE_xxx)
 * @param otype Output type (GPIO_OTYPE_xxx)
 * @param speed GPIO speed (GPIO_SPEED_xxx)
 * @param pupd Pull-up/pull-down (GPIO_xxx)
 * 
 * @note GPIO clock must be enabled separately via RCC_AHB1ENR
 */
void gpio_init(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t mode, uint8_t otype, uint8_t speed, uint8_t pupd);

/**
 * @brief Configure GPIO pin as alternate function
 * 
 * @param GPIOx Pointer to GPIO port
 * @param pin Pin number (0-15)
 * @param alternate Alternate function number (GPIO_AF_xxx)
 * 
 * @note This function should be called after gpio_init() with GPIO_MODE_AF
 */
void gpio_set_af(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t alternate);

/**
 * @brief Set GPIO pin level
 * 
 * @param GPIOx Pointer to GPIO port
 * @param pin Pin number (0-15)
 * @param value Pin value (1=high, 0=low)
 */
void gpio_write(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t value);

/**
 * @brief Read GPIO pin level
 * 
 * @param GPIOx Pointer to GPIO port
 * @param pin Pin number (0-15)
 * @return uint8_t Pin state (1=high, 0=low)
 */
uint8_t gpio_read(GPIO_TypeDef *GPIOx, uint8_t pin);

/**
 * @brief Toggle GPIO pin level
 * 
 * @param GPIOx Pointer to GPIO port
 * @param pin Pin number (0-15)
 */
void gpio_toggle(GPIO_TypeDef *GPIOx, uint8_t pin);

/**
 * @brief Configure GPIO pin for external interrupt
 * 
 * @param GPIOx Pointer to GPIO port
 * @param pin Pin number (0-15)
 * @param trigger_type Interrupt trigger type (GPIO_INT_RISING, GPIO_INT_FALLING, or GPIO_INT_BOTH)
 * @param priority Interrupt priority (0-15, 0 is highest)
 * 
 * @note SYSCFG clock must be enabled via RCC_APB2ENR
 */
void gpio_configure_interrupt(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t trigger_type, uint8_t priority);

/**
 * @brief Initialize a GPIO pin as input with external interrupt
 * 
 * @param GPIOx Pointer to GPIO port
 * @param pin Pin number (0-15)
 * @param pupd Pull-up/pull-down (GPIO_NOPULL, GPIO_PULLUP, GPIO_PULLDOWN)
 * @param trigger_type Interrupt trigger type (GPIO_INT_RISING, GPIO_INT_FALLING, or GPIO_INT_BOTH)
 * @param priority Interrupt priority (0-15, 0 is highest)
 * 
 * @note This function combines gpio_init() and gpio_configure_interrupt()
 */
void gpio_init_input_with_interrupt(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t pupd, 
                                   uint8_t trigger_type, uint8_t priority);

#endif /* GPIO_H */
