/**
 * @file ssd1306.h
 * @author Haoyi Chen
 * @date 2025-08-08
 * @brief SSD1306 OLED display driver header file
 *
 * @details This file contains function declarations, macro definitions, and font data
 * for the SSD1306 OLED display, suitable for 128x64 or 128x32 OLED displays
 * controlled via I2C interface.
 */

#ifndef SSD1306_H
#define SSD1306_H

#include "stm32f407xx.h"
#include "i2c_oled.h"

/* SSD1306 Display Parameters */
#define SSD1306_I2C_ADDR          0x3C  /**< SSD1306 I2C address (SA0=0: 0x3C, SA0=1: 0x3D) */
#define SSD1306_WIDTH             128   /**< SSD1306 display width (pixels) */
#define SSD1306_HEIGHT            64    /**< SSD1306 display height (pixels), 32 or 64 */
#define SSD1306_BUFFER_SIZE       (SSD1306_WIDTH * SSD1306_HEIGHT / 8)

/* SSD1306 Commands */
#define SSD1306_CMD_SET_CONTRAST           0x81  /**< Set contrast control */
#define SSD1306_CMD_DISPLAY_RAM            0xA4  /**< Display GDDRAM content */
#define SSD1306_CMD_DISPLAY_ALLON          0xA5  /**< Force all pixels on */
#define SSD1306_CMD_DISPLAY_NORMAL         0xA6  /**< Normal display */
#define SSD1306_CMD_DISPLAY_INVERTED       0xA7  /**< Inverted display */
#define SSD1306_CMD_DISPLAY_OFF            0xAE  /**< Turn display off */
#define SSD1306_CMD_DISPLAY_ON             0xAF  /**< Turn display on */

#define SSD1306_CMD_SET_MEMORY_ADDR_MODE   0x20  /**< Set memory addressing mode */
#define SSD1306_CMD_SET_COLUMN_ADDR        0x21  /**< Set column address range */
#define SSD1306_CMD_SET_PAGE_ADDR          0x22  /**< Set page address range */

#define SSD1306_CMD_SET_START_LINE         0x40  /**< Set start line */
#define SSD1306_CMD_SET_SEGMENT_REMAP      0xA0  /**< Segment remap, A0=column 0 to SEG0, A1=column 127 to SEG0 */
#define SSD1306_CMD_SET_MULTIPLEX_RATIO    0xA8  /**< Set multiplex ratio */
#define SSD1306_CMD_SET_COM_SCAN_DIR       0xC0  /**< COM scan direction, C0=normal, C8=reverse */
#define SSD1306_CMD_SET_DISPLAY_OFFSET     0xD3  /**< Set display offset */
#define SSD1306_CMD_SET_COM_PINS           0xDA  /**< Set COM pin configuration */

#define SSD1306_CMD_SET_DISPLAY_CLOCK_DIV  0xD5  /**< Set display clock divide ratio */
#define SSD1306_CMD_SET_PRECHARGE          0xD9  /**< Set pre-charge period */
#define SSD1306_CMD_SET_VCOM_DETECT        0xDB  /**< Set VCOMH voltage */

#define SSD1306_CMD_SET_CHARGE_PUMP        0x8D  /**< Set charge pump */

/* Text and Graphics Parameters */
#define SSD1306_COLOR_BLACK       0x00  /**< Black (no pixels) */
#define SSD1306_COLOR_WHITE       0x01  /**< White (pixels on) */

#define SSD1306_FONT_WIDTH        5     /**< Default font width */
#define SSD1306_FONT_HEIGHT       8     /**< Default font height */

/**
 * @brief Initialize SSD1306 OLED display
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @return uint8_t 0=success, 1=failure
 */
uint8_t ssd1306_init(I2C_TypeDef *I2Cx);

/**
 * @brief Clear display content
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @return uint8_t 0=success, 1=failure
 */
uint8_t ssd1306_clear(I2C_TypeDef *I2Cx);

/**
 * @brief Update display content
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @return uint8_t 0=success, 1=failure
 */
uint8_t ssd1306_update_screen(I2C_TypeDef *I2Cx);

/**
 * @brief Write a character to the OLED display
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-7), indicating page (each page is 8 pixels high)
 * @param ch Character to be displayed
 * @param font Font type
 * @param color Color
 * @return Character width
 */
uint8_t ssd1306_write_char(I2C_TypeDef *I2Cx, uint8_t x, uint8_t y, char ch, uint8_t color);

/**
 * @brief Write a string to the OLED display
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-7), indicating page (each page is 8 pixels high)
 * @param str String to be displayed
 * @param font Font type
 * @param color Color
 * @return String width
 */
uint8_t ssd1306_write_string(I2C_TypeDef *I2Cx, uint8_t x, uint8_t y, char *str, uint8_t color);

/**
 * @brief Draw a pixel
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-63)
 * @param color Color
 */
void ssd1306_draw_pixel(I2C_TypeDef *I2Cx, uint8_t x, uint8_t y, uint8_t color);

/**
 * @brief Draw a line
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @param x0 Starting X coordinate
 * @param y0 Starting Y coordinate
 * @param x1 Ending X coordinate
 * @param y1 Ending Y coordinate
 * @param color Color
 */
void ssd1306_draw_line(I2C_TypeDef *I2Cx, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color);

/**
 * @brief Draw a rectangle
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @param x Top-left X coordinate
 * @param y Top-left Y coordinate
 * @param width Width
 * @param height Height
 * @param color Color
 */
void ssd1306_draw_rectangle(I2C_TypeDef *I2Cx, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);

/**
 * @brief Fill a rectangle
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @param x Top-left X coordinate
 * @param y Top-left Y coordinate
 * @param width Width
 * @param height Height
 * @param color Color
 */
void ssd1306_fill_rectangle(I2C_TypeDef *I2Cx, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);

/**
 * @brief Draw a circle
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @param x0 Center X coordinate
 * @param y0 Center Y coordinate
 * @param radius Radius
 * @param color Color
 */
void ssd1306_draw_circle(I2C_TypeDef *I2Cx, uint8_t x0, uint8_t y0, uint8_t radius, uint8_t color);

/**
 * @brief Fill a circle
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @param x0 Center X coordinate
 * @param y0 Center Y coordinate
 * @param radius Radius
 * @param color Color
 */
void ssd1306_fill_circle(I2C_TypeDef *I2Cx, uint8_t x0, uint8_t y0, uint8_t radius, uint8_t color);

#endif /* SSD1306_H */
