/**
 * @file ssd1306.c
 * @author Haoyi Chen
 * @date 2025-08-08
 * @brief SSD1306 OLED display driver implementation
 *
 * @details This file contains the implementation of the SSD1306 OLED display driver,
 * based on I2C communication interface. It supports text display, drawing, and other
 * basic functions.
 */

#include "ssd1306.h"
#include <stdlib.h>
#include <string.h>

/* Default 5x8 font data */
static const uint8_t Font5x8[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, // Space (20)
    0x00, 0x00, 0x5F, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
    0x23, 0x13, 0x08, 0x64, 0x62, // %
    0x36, 0x49, 0x55, 0x22, 0x50, // &
    0x00, 0x05, 0x03, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, // )
    0x08, 0x2A, 0x1C, 0x2A, 0x08, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, // +
    0x00, 0x50, 0x30, 0x00, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, // -
    0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, // /
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    0x00, 0x36, 0x36, 0x00, 0x00, // :
    0x00, 0x56, 0x36, 0x00, 0x00, // ;
    0x00, 0x08, 0x14, 0x22, 0x41, // <
    0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x41, 0x22, 0x14, 0x08, 0x00, // >
    0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x32, 0x49, 0x79, 0x41, 0x3E, // @
    0x7E, 0x11, 0x11, 0x11, 0x7E, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x01, 0x01, // F
    0x3E, 0x41, 0x41, 0x51, 0x32, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x04, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x7F, 0x20, 0x18, 0x20, 0x7F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x03, 0x04, 0x78, 0x04, 0x03, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, // Z
    0x00, 0x00, 0x7F, 0x41, 0x41, // [
    0x02, 0x04, 0x08, 0x10, 0x20, // "\"
    0x41, 0x41, 0x7F, 0x00, 0x00, // ]
    0x04, 0x02, 0x01, 0x02, 0x04, // ^
    0x40, 0x40, 0x40, 0x40, 0x40, // _
    0x00, 0x01, 0x02, 0x04, 0x00, // `
    0x20, 0x54, 0x54, 0x54, 0x78, // a
    0x7F, 0x48, 0x44, 0x44, 0x38, // b
    0x38, 0x44, 0x44, 0x44, 0x20, // c
    0x38, 0x44, 0x44, 0x48, 0x7F, // d
    0x38, 0x54, 0x54, 0x54, 0x18, // e
    0x08, 0x7E, 0x09, 0x01, 0x02, // f
    0x08, 0x14, 0x54, 0x54, 0x3C, // g
    0x7F, 0x08, 0x04, 0x04, 0x78, // h
    0x00, 0x44, 0x7D, 0x40, 0x00, // i
    0x20, 0x40, 0x44, 0x3D, 0x00, // j
    0x00, 0x7F, 0x10, 0x28, 0x44, // k
    0x00, 0x41, 0x7F, 0x40, 0x00, // l
    0x7C, 0x04, 0x18, 0x04, 0x78, // m
    0x7C, 0x08, 0x04, 0x04, 0x78, // n
    0x38, 0x44, 0x44, 0x44, 0x38, // o
    0x7C, 0x14, 0x14, 0x14, 0x08, // p
    0x08, 0x14, 0x14, 0x18, 0x7C, // q
    0x7C, 0x08, 0x04, 0x04, 0x08, // r
    0x48, 0x54, 0x54, 0x54, 0x20, // s
    0x04, 0x3F, 0x44, 0x40, 0x20, // t
    0x3C, 0x40, 0x40, 0x20, 0x7C, // u
    0x1C, 0x20, 0x40, 0x20, 0x1C, // v
    0x3C, 0x40, 0x30, 0x40, 0x3C, // w
    0x44, 0x28, 0x10, 0x28, 0x44, // x
    0x0C, 0x50, 0x50, 0x50, 0x3C, // y
    0x44, 0x64, 0x54, 0x4C, 0x44, // z
    0x00, 0x08, 0x36, 0x41, 0x00, // {
    0x00, 0x00, 0x7F, 0x00, 0x00, // |
    0x00, 0x41, 0x36, 0x08, 0x00, // }
    0x08, 0x04, 0x08, 0x10, 0x08, // ~
    0x00, 0x00, 0x00, 0x00, 0x00
};

/* Display buffer */
static uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];

/* Current I2C instance in use */
static I2C_TypeDef *SSD1306_I2C;

/**
 * @brief Send a command to SSD1306
 * 
 * @param I2Cx I2C peripheral
 * @param cmd Command to be sent
 * @return uint8_t 0=success, 1=failure
 */
static uint8_t ssd1306_write_command(I2C_TypeDef *I2Cx, uint8_t cmd) {
    uint8_t data[2];
    data[0] = 0x00;  // Control byte: 0x00 indicates command
    data[1] = cmd;   // Command byte
    
    return i2c_master_transmit(I2Cx, SSD1306_I2C_ADDR, data, 2, 1000);
}

/**
 * @brief Send data to SSD1306
 * 
 * @param I2Cx I2C peripheral
 * @param data Data to be sent
 * @param size Data size
 * @return uint8_t 0=success, 1=failure
 */
static uint8_t ssd1306_write_data(I2C_TypeDef *I2Cx, uint8_t* data, uint16_t size) {
    uint8_t *buffer = (uint8_t*)malloc(size + 1);
    if (buffer == NULL) {
        return 1; // Memory allocation failed
    }
    
    buffer[0] = 0x40;  // Control byte: 0x40 indicates data
    memcpy(buffer + 1, data, size);
    
    uint8_t result = i2c_master_transmit(I2Cx, SSD1306_I2C_ADDR, buffer, size + 1, 1000);
    
    free(buffer);
    return result;
}

/**
 * @brief Initialize SSD1306 OLED display
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @return uint8_t 0=success, 1=failure
 */
uint8_t ssd1306_init(I2C_TypeDef *I2Cx) {
    uint8_t status;
    
    // Store I2C instance for other functions to use
    SSD1306_I2C = I2Cx;
    
    // Wait for the display to be ready
    if (i2c_is_device_ready(I2Cx, SSD1306_I2C_ADDR, 20, 1000) != 0) {
        return 1;  // Device not ready
    }
    
    // Initialize display
    status = ssd1306_write_command(I2Cx, SSD1306_CMD_DISPLAY_OFF);                 // 0xAE
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_DISPLAY_CLOCK_DIV);      // 0xD5
    status |= ssd1306_write_command(I2Cx, 0x80);                                   // Recommended value
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_MULTIPLEX_RATIO);        // 0xA8
    status |= ssd1306_write_command(I2Cx, SSD1306_HEIGHT - 1);
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_DISPLAY_OFFSET);         // 0xD3
    status |= ssd1306_write_command(I2Cx, 0x00);                                   // No offset
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_START_LINE | 0x00);      // 0x40 | Start line
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_CHARGE_PUMP);            // 0x8D
    status |= ssd1306_write_command(I2Cx, 0x14);                                   // Enable charge pump
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_MEMORY_ADDR_MODE);       // 0x20
    status |= ssd1306_write_command(I2Cx, 0x00);                                   // Horizontal addressing mode
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_SEGMENT_REMAP | 0x01);   // 0xA0 | 1
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_COM_SCAN_DIR | 0x8);     // 0xC0 | 8
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_COM_PINS);               // 0xDA
    status |= ssd1306_write_command(I2Cx, 0x12);
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_CONTRAST);               // 0x81
    status |= ssd1306_write_command(I2Cx, 0xCF);
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_PRECHARGE);              // 0xD9
    status |= ssd1306_write_command(I2Cx, 0xF1);
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_VCOM_DETECT);            // 0xDB
    status |= ssd1306_write_command(I2Cx, 0x40);
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_DISPLAY_RAM);                // 0xA4
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_DISPLAY_NORMAL);             // 0xA6
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_DISPLAY_ON);                 // 0xAF
    
    // Clear display
    ssd1306_clear(I2Cx);
    
    return status;
}

/**
 * @brief Clear display content
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @return uint8_t 0=success, 1=failure
 */
uint8_t ssd1306_clear(I2C_TypeDef *I2Cx) {
    // Clear the buffer
    memset(SSD1306_Buffer, 0, SSD1306_BUFFER_SIZE);
    
    // Update display
    return ssd1306_update_screen(I2Cx);
}

/**
 * @brief Update display content
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @return uint8_t 0=success, 1=failure
 */
uint8_t ssd1306_update_screen(I2C_TypeDef *I2Cx) {
    uint8_t status = 0;
    
    // Set column address range
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_COLUMN_ADDR);  // 0x21
    status |= ssd1306_write_command(I2Cx, 0);                            // Start column
    status |= ssd1306_write_command(I2Cx, SSD1306_WIDTH - 1);            // End column
    
    // Set page address range
    status |= ssd1306_write_command(I2Cx, SSD1306_CMD_SET_PAGE_ADDR);    // 0x22
    status |= ssd1306_write_command(I2Cx, 0);                            // Start page
    status |= ssd1306_write_command(I2Cx, SSD1306_HEIGHT / 8 - 1);       // End page
    
    // Write data
    status |= ssd1306_write_data(I2Cx, SSD1306_Buffer, SSD1306_BUFFER_SIZE);
    
    return status;
}

/**
 * @brief Write a character to the OLED display
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-7), indicating page (each page is 8 pixels high)
 * @param ch Character to be displayed
 * @param color Color
 * @return Character width
 */
uint8_t ssd1306_write_char(I2C_TypeDef *I2Cx, uint8_t x, uint8_t y, char ch, uint8_t color) {
    // Check parameters
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT / 8) {
        return 0;
    }
    
    // Get character index
    uint32_t i, b;
    uint8_t *ch_data;
    
    // Calculate the font data position of the ASCII character
    if (ch >= ' ' && ch <= '~') {
        ch_data = (uint8_t*)&Font5x8[(ch - ' ') * SSD1306_FONT_WIDTH];
    } else {
        ch_data = (uint8_t*)&Font5x8[0]; // Default to display space
    }
    
    // Check remaining space
    if (SSD1306_WIDTH <= (x + SSD1306_FONT_WIDTH)) {
        // Exceeds width range
        return 0;
    }
    
    // Draw character
    for (i = 0; i < SSD1306_FONT_WIDTH; i++) {
        uint8_t line;
        
        // Get font data
        line = ch_data[i];
        
        // Write to display buffer
        if (color == SSD1306_COLOR_WHITE) {
            SSD1306_Buffer[x + i + (y * SSD1306_WIDTH)] = line;
        } else {
            SSD1306_Buffer[x + i + (y * SSD1306_WIDTH)] = ~line;
        }
    }
    
    // Return the width of the drawn character
    return SSD1306_FONT_WIDTH;
}

/**
 * @brief Write a string to the OLED display
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-7), indicating page (each page is 8 pixels high)
 * @param str String to be displayed
 * @param color Color
 * @return String width
 */
uint8_t ssd1306_write_string(I2C_TypeDef *I2Cx, uint8_t x, uint8_t y, char *str, uint8_t color) {
    // Record starting position
    uint8_t start_x = x;
    
    // Process each character in a loop
    while (*str) {
        // Write a character
        uint8_t width = ssd1306_write_char(I2Cx, x, y, *str, color);
        
        // Increase x coordinate
        x += width;
        
        // Check if line feed is needed
        if (x >= SSD1306_WIDTH) {
            x = start_x; // Return to starting column
            y++; // Next line
            
            // Check if exceeds screen range
            if (y >= SSD1306_HEIGHT / 8) {
                break; // Exceeds screen range, stop drawing
            }
        }
        
        // Next character
        str++;
    }
    
    // Return total width
    return x - start_x;
}

/**
 * @brief Draw a pixel
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-63)
 * @param color Color
 */
void ssd1306_draw_pixel(I2C_TypeDef *I2Cx, uint8_t x, uint8_t y, uint8_t color) {
    // Check parameters
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return;
    }
    
    // Determine the page and bit where the pixel is located
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    uint16_t index = x + (page * SSD1306_WIDTH);
    
    // Set or clear the pixel
    if (color == SSD1306_COLOR_WHITE) {
        SSD1306_Buffer[index] |= (1 << bit);
    } else {
        SSD1306_Buffer[index] &= ~(1 << bit);
    }
}

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
void ssd1306_draw_line(I2C_TypeDef *I2Cx, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color) {
    int16_t dx, dy, sx, sy, err, e2;
    
    // Calculate absolute differences
    dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
    dy = (y0 < y1) ? (y1 - y0) : (y0 - y1);
    
    // Determine direction
    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;
    
    // Initialize error
    err = dx - dy;
    
    // Loop to draw the line
    while (1) {
        // Draw the current point
        ssd1306_draw_pixel(I2Cx, x0, y0, color);
        
        // Check endpoint
        if (x0 == x1 && y0 == y1) {
            break;
        }
        
        // Calculate the next point
        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

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
void ssd1306_draw_rectangle(I2C_TypeDef *I2Cx, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color) {
    // Draw four edges
    ssd1306_draw_line(I2Cx, x, y, x + width - 1, y, color);           // Top edge
    ssd1306_draw_line(I2Cx, x, y + height - 1, x + width - 1, y + height - 1, color); // Bottom edge
    ssd1306_draw_line(I2Cx, x, y, x, y + height - 1, color);          // Left edge
    ssd1306_draw_line(I2Cx, x + width - 1, y, x + width - 1, y + height - 1, color);  // Right edge
}

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
void ssd1306_fill_rectangle(I2C_TypeDef *I2Cx, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color) {
    // Draw horizontal lines to fill the rectangle
    for (uint8_t i = 0; i < height; i++) {
        ssd1306_draw_line(I2Cx, x, y + i, x + width - 1, y + i, color);
    }
}

/**
 * @brief Draw a circle
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @param x0 Center X coordinate
 * @param y0 Center Y coordinate
 * @param radius Radius
 * @param color Color
 */
void ssd1306_draw_circle(I2C_TypeDef *I2Cx, uint8_t x0, uint8_t y0, uint8_t radius, uint8_t color) {
    int16_t f = 1 - radius;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * radius;
    int16_t x = 0;
    int16_t y = radius;
    
    // Draw four basic points
    ssd1306_draw_pixel(I2Cx, x0, y0 + radius, color);
    ssd1306_draw_pixel(I2Cx, x0, y0 - radius, color);
    ssd1306_draw_pixel(I2Cx, x0 + radius, y0, color);
    ssd1306_draw_pixel(I2Cx, x0 - radius, y0, color);
    
    // Use Bresenham's algorithm to draw the circle
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        
        // Draw points in eight directions
        ssd1306_draw_pixel(I2Cx, x0 + x, y0 + y, color);
        ssd1306_draw_pixel(I2Cx, x0 - x, y0 + y, color);
        ssd1306_draw_pixel(I2Cx, x0 + x, y0 - y, color);
        ssd1306_draw_pixel(I2Cx, x0 - x, y0 - y, color);
        ssd1306_draw_pixel(I2Cx, x0 + y, y0 + x, color);
        ssd1306_draw_pixel(I2Cx, x0 - y, y0 + x, color);
        ssd1306_draw_pixel(I2Cx, x0 + y, y0 - x, color);
        ssd1306_draw_pixel(I2Cx, x0 - y, y0 - x, color);
    }
}

/**
 * @brief Fill a circle
 *
 * @param I2Cx I2C peripheral to be used (I2C1, I2C2, or I2C3)
 * @param x0 Center X coordinate
 * @param y0 Center Y coordinate
 * @param radius Radius
 * @param color Color
 */
void ssd1306_fill_circle(I2C_TypeDef *I2Cx, uint8_t x0, uint8_t y0, uint8_t radius, uint8_t color) {
    int16_t f = 1 - radius;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * radius;
    int16_t x = 0;
    int16_t y = radius;
    
    // Draw vertical centerline
    for (uint8_t i = y0 - radius; i <= y0 + radius; i++) {
        ssd1306_draw_pixel(I2Cx, x0, i, color);
    }
    
    // Use Bresenham's algorithm to fill the circle
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        
        // Draw horizontal lines for filling
        for (uint8_t i = x0 - x; i <= x0 + x; i++) {
            ssd1306_draw_pixel(I2Cx, i, y0 + y, color);
            ssd1306_draw_pixel(I2Cx, i, y0 - y, color);
        }
        for (uint8_t i = x0 - y; i <= x0 + y; i++) {
            ssd1306_draw_pixel(I2Cx, i, y0 + x, color);
            ssd1306_draw_pixel(I2Cx, i, y0 - x, color);
        }
    }
}
