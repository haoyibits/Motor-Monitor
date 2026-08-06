/**
 * @file ssd1309.h
 * @author Haoyi Chen
 * @date 2025-08-18
 * @brief SSD1309 OLED display driver for STM32F407VGT6
 *
 * @details This file contains SSD1309 OLED display function declarations
 * built on top of the generic I2C driver.
 */

#ifndef SSD1309_H
#define SSD1309_H

#include "stm32f407xx.h"
#include "driver_status.h"

/**
 * @name SSD1309 I2C addresses
 * @{
 */
#define SSD1309_I2C_ADDR_DEFAULT    0x3C  /**< Default I2C address */
#define SSD1309_I2C_ADDR_ALT        0x3D  /**< Alternative I2C address */
/** @} */

/**
 * @name SSD1309 Display dimensions
 * @{
 */
#define SSD1309_WIDTH               128   /**< Display width in pixels */
#define SSD1309_HEIGHT              64    /**< Display height in pixels */
#define SSD1309_PAGES               8     /**< Number of pages (64/8) */
#define SSD1309_MAX_BRIGHTNESS      255   /**< Maximum brightness/contrast value */
/** @} */

/**
 * @name SSD1306/SSD1309 Command definitions
 * @note Compatible with both SSD1306 and SSD1309 controllers
 * @{
 */
/* Fundamental Commands */
#define SSD1309_DISPLAYOFF          0xAE  /**< Display OFF */
#define SSD1309_DISPLAYON           0xAF  /**< Display ON */
#define SSD1309_SETCONTRAST         0x81  /**< Set contrast */
#define SSD1309_DISPLAYALLON_RESUME 0xA4  /**< Resume to RAM content display */
#define SSD1309_DISPLAYALLON        0xA5  /**< Entire display ON */
#define SSD1309_NORMALDISPLAY       0xA6  /**< Normal display */
#define SSD1309_INVERTDISPLAY       0xA7  /**< Inverted display */


#define SSD1309_MEMORYMODE          0x20  /**< Set Memory Addressing Mode */
#define SSD1309_COLUMNADDR          0x21  /**< Set Column Address */
#define SSD1309_PAGEADDR            0x22  /**< Set Page Address */

/* Hardware Configuration Commands */
#define SSD1309_SETSTARTLINE        0x40  /**< Set start line address */
#define SSD1309_SEGREMAP            0xA0  /**< Set Segment Re-map */
#define SSD1309_SEGREMAP_REVERSE    0xA1  /**< Set Segment Re-map (reverse) */
#define SSD1309_SETMULTIPLEX        0xA8  /**< Set multiplex ratio */
#define SSD1309_COMSCANINC          0xC0  /**< Set COM output scan direction */
#define SSD1309_COMSCANDEC          0xC8  /**< Set COM output scan direction */
#define SSD1309_SETDISPLAYOFFSET    0xD3  /**< Set display offset */
#define SSD1309_SETCOMPINS          0xDA  /**< Set COM pins hardware config */

/* Timing & Driving Scheme Setting Commands */
#define SSD1309_SETDISPLAYCLOCKDIV  0xD5  /**< Set display clock divide ratio */
#define SSD1309_SETPRECHARGE        0xD9  /**< Set pre-charge period */
#define SSD1309_SETVCOMDETECT       0xDB  /**< Set VCOM deselect level */
#define SSD1309_CHARGEPUMP          0x8D  /**< Set DC-DC enable */

/** @} */

/**
 * @brief SSD1309 configuration structure
 */
typedef struct {
    I2C_TypeDef *I2Cx;         /**< I2C peripheral to use */
    uint8_t DevAddress;        /**< Device I2C address */
    uint8_t Contrast;          /**< Display contrast (0-255) */
    uint8_t InvertDisplay;     /**< Invert display (0=normal, 1=inverted) */
    uint32_t I2CTimeoutCycles; /**< Timeout used for each I2C wait */
} SSD1309_InitTypeDef;

/**
 * @brief Initialize SSD1309 OLED display
 * 
 * @param init SSD1309 initialization parameters
 * @return DriverStatus describing success or the failure reason
 */
DriverStatus ssd1309_init(SSD1309_InitTypeDef *init);

/**
 * @brief Send command to SSD1309 display
 * 
 * @param init SSD1309 configuration
 * @param cmd Command to send
 * @return DriverStatus describing success or the failure reason
 */
DriverStatus ssd1309_send_command(SSD1309_InitTypeDef *init, uint8_t cmd);

/**
 * @brief Send data to SSD1309 display
 * 
 * @param init SSD1309 configuration
 * @param data Data buffer pointer
 * @param size Data size
 * @return DriverStatus describing success or the failure reason
 */
DriverStatus ssd1309_send_data(SSD1309_InitTypeDef *init,
                               const uint8_t *data, uint16_t size);

/**
 * @brief Clear entire display
 * 
 * @param init SSD1309 configuration
 * @return DriverStatus describing success or the failure reason
 */
DriverStatus ssd1309_clear_display(SSD1309_InitTypeDef *init);

/**
 * @brief Set display contrast
 * 
 * @param init SSD1309 configuration
 * @param contrast Contrast value (0-255)
 * @return DriverStatus describing success or the failure reason
 */
DriverStatus ssd1309_set_contrast(SSD1309_InitTypeDef *init, uint8_t contrast);

/**
 * @brief Turn display on/off
 * 
 * @param init SSD1309 configuration
 * @param state Display state (0=off, 1=on)
 * @return DriverStatus describing success or the failure reason
 */
DriverStatus ssd1309_display_on_off(SSD1309_InitTypeDef *init, uint8_t state);

/**
 * @brief Invert display
 * 
 * @param init SSD1309 configuration
 * @param invert Invert state (0=normal, 1=inverted)
 * @return DriverStatus describing success or the failure reason
 */
DriverStatus ssd1309_invert_display(SSD1309_InitTypeDef *init, uint8_t invert);

/**
 * @brief Set cursor position
 * 
 * @param init SSD1309 configuration
 * @param column Column position (0-127)
 * @param page Page position (0-7)
 * @return DriverStatus describing success or the failure reason
 */
DriverStatus ssd1309_set_cursor(SSD1309_InitTypeDef *init,
                                uint8_t column, uint8_t page);

/**
 * @brief Check if SSD1309 device is ready
 * 
 * @param init SSD1309 configuration
 * @param trials Number of attempts
 * @return DriverStatus describing readiness or the failure reason
 */
DriverStatus ssd1309_is_ready(SSD1309_InitTypeDef *init, uint8_t trials);

#endif /* SSD1309_H */
