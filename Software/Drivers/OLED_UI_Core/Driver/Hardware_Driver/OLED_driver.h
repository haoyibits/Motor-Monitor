/**
 ******************************************************************************
 * @file           : OLED_driver.h
 * @author         : Haoyi Chen
 * @date           : 2025-08-22
 * @brief          : OLED hardware driver port for OLED_UI_Core library
 ******************************************************************************
 * @details
 * OLED_UI_Core Library Hardware Abstraction Layer - OLED Driver Port
 * 
 * This file serves as a hardware abstraction port for integrating the
 * OLED_UI_Core library with bare-metal STM32F407VGT6 applications.
 * 
 * Original library: OLED_UI_Core (HAL-based, SPI communication)
 * Port target: STM32F407VGT6 bare-metal (register-based, I2C communication)
 * 
 * Port modifications:
 * - Replaced HAL SPI with register-based I2C communication
 * - Integrated with project's register-based driver architecture
 * - Maintained compatibility with original OLED_UI_Core library interface
 * - Added support for multiple OLED controllers via compile-time selection
 * 
 * Hardware configuration:
 * - I2C1: PB6 (SCL), PB7 (SDA) at 400kHz
 * - Reset: Not connected (software initialization only)
 * - I2C Address: 0x3C (SSD1309_I2C_ADDR_DEFAULT)
 * - Supported controllers: SSD1306, SH1106, SSD1309
 ******************************************************************************
 */

#ifndef __OLED_DRIVER_H
#define __OLED_DRIVER_H

#include "bsp.h"
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdarg.h>

/* Communication Protocol Configuration */
#define OLED_UI_USE_I2C  /* Use I2C communication instead of original SPI */

/* I2C Configuration */
#define OLED_I2C_ADDR           SSD1309_I2C_ADDR_DEFAULT  /* Use default address from ssd1309.h */
#define OLED_I2C_PERIPHERAL     I2C1                      /* Use I2C1 peripheral */

/* I2C Control Bytes (using generic definitions from i2c.h) */
#define OLED_I2C_CMD_BYTE       I2C_OLED_CMD_BYTE     /* Co=0, D/C=0 (send command) */
#define OLED_I2C_DATA_BYTE      I2C_OLED_DATA_BYTE    /* Co=0, D/C=1 (send data) */

/* Hardware Reset Pin Configuration */
/* Comment out the following line if RST pin is not connected */
// #define OLED_USE_RST_PIN  /* RST pin not connected */

#ifdef OLED_USE_RST_PIN
/* GPIO Pin Definitions */
#define OLED_RST_PORT           GPIOB
#define OLED_RST_PIN            5       /* Reset pin */

/* GPIO Control Macros */
#define OLED_RES_Clr()          gpio_write(OLED_RST_PORT, OLED_RST_PIN, 0)
#define OLED_RES_Set()          gpio_write(OLED_RST_PORT, OLED_RST_PIN, 1)
#else
/* No-op macros when RST pin is not used */
#define OLED_RES_Clr()          do {} while(0)
#define OLED_RES_Set()          do {} while(0)
#endif

/* Note: I2C pins configured in i2c_init: PB6(SCL), PB7(SDA) */

/**
 * @brief External display buffer declaration for UI layer access
 * @note This allows UI layer to directly access the framebuffer for advanced operations
 */
extern uint8_t OLED_DisplayBuf[SSD1309_PAGES][SSD1309_WIDTH];

/**
 * @brief Initialize OLED display hardware
 */
void OLED_Init(void);

/**
 * @brief Update entire display from framebuffer
 */
void OLED_Update(void);

/**
 * @brief Update specific area of display from framebuffer
 * @param X Left coordinate of update area (0-127)
 * @param Y Top coordinate of update area (0-63)  
 * @param Width Width of update area (0-128)
 * @param Height Height of update area (0-64)
 */
void OLED_UpdateArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);

/**
 * @brief Set OLED display color mode
 * @param colormode Color mode: true=normal mode, false=inverted mode
 */
void OLED_SetColorMode(bool colormode);

/**
 * @brief Set OLED display brightness/contrast  
 * @param Brightness Brightness level (0-255)
 */
void OLED_Brightness(int16_t Brightness);

#endif
