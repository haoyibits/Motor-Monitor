/**
 * @file i2c_oled.h
 * @author Haoyi Chen
 * @date 2025-08-08
 * @brief Streamlined I2C driver specifically for SSD1306 OLED
 *
 * @details This file contains I2C function declarations optimized for SSD1306 OLED displays,
 * removing unnecessary features to improve code efficiency.
 */

#ifndef I2C_OLED_H
#define I2C_OLED_H

#include "stm32f407xx.h"

/**
 * @brief I2C initialization structure
 */
typedef struct {
    uint32_t ClockSpeed;       /**< I2C clock frequency in Hz */
    uint8_t DutyCycle;         /**< Duty cycle in Fast mode */
} I2C_OLED_InitTypeDef;

/**
 * @name I2C Fast Mode duty cycle
 * @{
 */
#define I2C_DUTYCYCLE_2            0x00000000U  /**< Duty cycle 2 */
#define I2C_DUTYCYCLE_16_9         0x00004000U  /**< Duty cycle 16/9 */
/** @} */

/**
 * @brief Initialize I2C for OLED display
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param init I2C initialization parameters
 */
void i2c_oled_init(I2C_TypeDef *I2Cx, I2C_OLED_InitTypeDef *init);

/**
 * @brief Initialize I2C GPIO pins
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param GPIOx GPIO port (GPIOA, GPIOB, etc.)
 * @param SCL_Pin SCL pin number (0-15)
 * @param SDA_Pin SDA pin number (0-15)
 */
void i2c_oled_gpio_init(I2C_TypeDef *I2Cx, GPIO_TypeDef *GPIOx, uint8_t SCL_Pin, uint8_t SDA_Pin);

/**
 * @brief Send command to OLED display
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param DevAddress OLED I2C address
 * @param cmd Command to send
 * @return uint8_t 0=success, 1=failure
 */
uint8_t i2c_oled_send_command(I2C_TypeDef *I2Cx, uint8_t DevAddress, uint8_t cmd);

/**
 * @brief Send data to OLED display
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param DevAddress OLED I2C address
 * @param data Data buffer pointer
 * @param size Data size
 * @return uint8_t 0=success, 1=failure
 */
uint8_t i2c_oled_send_data(I2C_TypeDef *I2Cx, uint8_t DevAddress, uint8_t* data, uint16_t size);

/**
 * @brief Check if OLED device is ready
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param DevAddress OLED I2C address
 * @param Trials Number of attempts
 * @return uint8_t 0=ready, 1=not ready
 */
uint8_t i2c_oled_is_ready(I2C_TypeDef *I2Cx, uint8_t DevAddress, uint8_t Trials);

#endif /* I2C_OLED_H */
