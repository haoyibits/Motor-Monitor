/**
 * @file i2c.h
 * @author Haoyi Chen
 * @date 2025-08-18
 * @brief Generic I2C driver for STM32F407VGT6
 *
 * @details This file contains I2C function declarations for register-based
 * I2C communication, providing a generic interface for all I2C devices.
 */

#ifndef I2C_H
#define I2C_H

#include "stm32f407xx.h"

/**
 * @brief I2C initialization structure
 */
typedef struct {
    uint32_t ClockSpeed;       /**< I2C clock frequency in Hz */
    uint8_t DutyCycle;         /**< Duty cycle in Fast mode */
} I2C_InitTypeDef;

/**
 * @name I2C Fast Mode duty cycle
 * @{
 */
#define I2C_DUTYCYCLE_2            0x00000000U  /**< Duty cycle 2 */
#define I2C_DUTYCYCLE_16_9         0x00004000U  /**< Duty cycle 16/9 */
/** @} */

/**
 * @name I2C Clock frequencies
 * @{
 */
#define I2C_CLOCKSPEED_100KHZ      100000       /**< Standard mode 100kHz */
#define I2C_CLOCKSPEED_400KHZ      400000       /**< Fast mode 400kHz */
#define I2C_CLOCKSPEED_800KHZ      800000       /**< Fast mode+ 800kHz (custom) */
#define I2C_CLOCKSPEED_1MHZ        1000000      /**< Fast mode+ 1MHz (custom) */
/** @} */

/**
 * @name I2C GPIO pin definitions for common configurations
 * @{
 */
#define I2C1_SCL_PIN_PB8           8            /**< I2C1 SCL on PB8 */
#define I2C1_SDA_PIN_PB9           9            /**< I2C1 SDA on PB9 */
#define I2C1_SCL_PIN_PB6           6            /**< I2C1 SCL on PB6 */
#define I2C1_SDA_PIN_PB7           7            /**< I2C1 SDA on PB7 */
/** @} */

/**
 * @name I2C Control byte definitions for OLED-style devices
 * @note These are commonly used by OLED controllers (SSD1306, SSD1309, etc.)
 * @{
 */
#define I2C_OLED_CMD_BYTE          0x00         /**< Co=0, D/C=0 (send command) */
#define I2C_OLED_DATA_BYTE         0x40         /**< Co=0, D/C=1 (send data) */
/** @} */

/**
 * @brief Initialize I2C peripheral
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param init I2C initialization parameters
 */
void i2c_init(I2C_TypeDef *I2Cx, I2C_InitTypeDef *init);

/**
 * @brief Initialize I2C GPIO pins
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param GPIOx GPIO port (GPIOA, GPIOB, etc.)
 * @param SCL_Pin SCL pin number (0-15)
 * @param SDA_Pin SDA pin number (0-15)
 */
void i2c_gpio_init(I2C_TypeDef *I2Cx, GPIO_TypeDef *GPIOx, uint8_t SCL_Pin, uint8_t SDA_Pin);

/**
 * @brief Write data to I2C device
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param DevAddress Device I2C address
 * @param data Data buffer pointer
 * @param size Data size
 * @return uint8_t 0=success, 1=failure
 */
uint8_t i2c_write(I2C_TypeDef *I2Cx, uint8_t DevAddress, uint8_t* data, uint16_t size);

/**
 * @brief Read data from I2C device
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param DevAddress Device I2C address
 * @param data Data buffer pointer
 * @param size Data size
 * @return uint8_t 0=success, 1=failure
 */
uint8_t i2c_read(I2C_TypeDef *I2Cx, uint8_t DevAddress, uint8_t* data, uint16_t size);

/**
 * @brief Check if I2C device is ready
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param DevAddress Device I2C address
 * @param Trials Number of attempts
 * @return uint8_t 0=ready, 1=not ready
 */
uint8_t i2c_is_ready(I2C_TypeDef *I2Cx, uint8_t DevAddress, uint8_t Trials);

#endif /* I2C_H */