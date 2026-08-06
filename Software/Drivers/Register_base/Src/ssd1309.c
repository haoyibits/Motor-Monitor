/**
 * @file ssd1309.c
 * @author Haoyi Chen
 * @date 2025-08-18
 * @brief Implementation of SSD1309 OLED display driver
 *
 * @details This file contains the implementation of SSD1309 OLED display functions
 * built on top of the generic I2C driver.
 */

#include <stddef.h>

#include "i2c.h"
#include "ssd1309.h"


/**
 * @brief Initialize SSD1309 OLED display
 * 
 * @param init SSD1309 initialization parameters
 * @return uint8_t 0=success, 1=failure
 */
DriverStatus ssd1309_init(SSD1309_InitTypeDef *init) {
    if ((init == NULL) || (init->I2Cx == NULL)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    /* Check if device is ready */
    if (ssd1309_is_ready(init, 3) != 0) {
        return 1; // Device not ready
    }
    
    /* Display OFF */
    if (ssd1309_send_command(init, SSD1309_DISPLAYOFF) != 0) return 1;
    
    /* Set display clock divide ratio */
    if (ssd1309_send_command(init, SSD1309_SETDISPLAYCLOCKDIV) != 0) return 1;
    if (ssd1309_send_command(init, 0x80) != 0) return 1; // Default divide ratio
    
    /* Set multiplex ratio */
    if (ssd1309_send_command(init, SSD1309_SETMULTIPLEX) != 0) return 1;
    if (ssd1309_send_command(init, SSD1309_HEIGHT - 1) != 0) return 1; // 64-1 = 63
    
    /* Set display offset */
    if (ssd1309_send_command(init, SSD1309_SETDISPLAYOFFSET) != 0) return 1;
    if (ssd1309_send_command(init, 0x00) != 0) return 1; // No offset
    
    /* Set start line address */
    if (ssd1309_send_command(init, SSD1309_SETSTARTLINE | 0x00) != 0) return 1;
    
    /* Enable charge pump */
    if (ssd1309_send_command(init, SSD1309_CHARGEPUMP) != 0) return 1;
    if (ssd1309_send_command(init, 0x14) != 0) return 1; // Enable
    
    /* Set memory addressing mode */
    if (ssd1309_send_command(init, SSD1309_MEMORYMODE) != 0) return 1;
    if (ssd1309_send_command(init, 0x00) != 0) return 1; // Horizontal addressing mode
    
    /* Set segment re-map */
    if (ssd1309_send_command(init, SSD1309_SEGREMAP | 0x01) != 0) return 1; // Column address 127 mapped to SEG0
    
    /* Set COM output scan direction */
    if (ssd1309_send_command(init, SSD1309_COMSCANDEC) != 0) return 1; // Scan from COM[N] to COM0
    
    /* Set COM pins hardware configuration */
    if (ssd1309_send_command(init, SSD1309_SETCOMPINS) != 0) return 1;
    if (ssd1309_send_command(init, 0x12) != 0) return 1; // Alternative COM pin config, disable left/right remap
    
    /* Set contrast */
    if (ssd1309_set_contrast(init, init->Contrast) != 0) return 1;
    
    /* Set pre-charge period */
    if (ssd1309_send_command(init, SSD1309_SETPRECHARGE) != 0) return 1;
    if (ssd1309_send_command(init, 0xF1) != 0) return 1; // Phase 1: 1 DCLK, Phase 2: 15 DCLK
    
    /* Set VCOM deselect level */
    if (ssd1309_send_command(init, SSD1309_SETVCOMDETECT) != 0) return 1;
    if (ssd1309_send_command(init, 0x40) != 0) return 1; // 0.77*VCC
    
    /* Display entire RAM contents */
    if (ssd1309_send_command(init, SSD1309_DISPLAYALLON_RESUME) != 0) return 1;
    
    /* Set normal/inverted display */
    if (ssd1309_invert_display(init, init->InvertDisplay) != 0) return 1;
    
    /* Clear display */
    if (ssd1309_clear_display(init) != 0) return 1;
    
    /* Display ON */
    if (ssd1309_send_command(init, SSD1309_DISPLAYON) != 0) return 1;
    
    return 0; // Success
}

/**
 * @brief Send command to SSD1309 display
 * 
 * @param init SSD1309 configuration
 * @param cmd Command to send
 * @return uint8_t 0=success, 1=failure
 */
DriverStatus ssd1309_send_command(SSD1309_InitTypeDef *init, uint8_t cmd) {
    if ((init == NULL) || (init->I2Cx == NULL)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    uint8_t data[2];
    data[0] = 0x00; // Control byte: Co=0, D/C=0 (command)
    data[1] = cmd;
    
    return i2c_write(init->I2Cx, init->DevAddress, data, 2,
                     init->I2CTimeoutCycles);
}

/**
 * @brief Send data to SSD1309 display
 * 
 * @param init SSD1309 configuration
 * @param data Data buffer pointer
 * @param size Data size
 * @return uint8_t 0=success, 1=failure
 */
DriverStatus ssd1309_send_data(SSD1309_InitTypeDef *init,
                               const uint8_t *data, uint16_t size) {
    /* Check parameters */
    if ((init == NULL) || (init->I2Cx == NULL) || (data == NULL) ||
        (size == 0U) || (size > SSD1309_WIDTH)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    
    /* Prepare data with control byte */
    uint8_t buffer[SSD1309_WIDTH + 1U];
    buffer[0] = 0x40; // Control byte: Co=0, D/C=1 (data)
    
    for (uint16_t i = 0; i < size; i++) {
        buffer[i + 1] = data[i];
    }
    
    return i2c_write(init->I2Cx, init->DevAddress, buffer, size + 1U,
                     init->I2CTimeoutCycles);
}

/**
 * @brief Clear entire display
 * 
 * @param init SSD1309 configuration
 * @return uint8_t 0=success, 1=failure
 */
DriverStatus ssd1309_clear_display(SSD1309_InitTypeDef *init) {
    /* Set column address range */
    if (ssd1309_send_command(init, SSD1309_COLUMNADDR) != 0) return 1;
    if (ssd1309_send_command(init, 0) != 0) return 1;     // Start column
    if (ssd1309_send_command(init, SSD1309_WIDTH - 1) != 0) return 1; // End column
    
    /* Set page address range */
    if (ssd1309_send_command(init, SSD1309_PAGEADDR) != 0) return 1;
    if (ssd1309_send_command(init, 0) != 0) return 1;     // Start page
    if (ssd1309_send_command(init, SSD1309_PAGES - 1) != 0) return 1; // End page
    
    /* Clear display data in chunks to avoid large stack allocation */
    uint8_t clear_data[64]; // Clear 64 bytes at a time
    for (uint16_t i = 0; i < 64; i++) {
        clear_data[i] = 0x00;
    }
    
    /* Send clear data (128*64/8 = 1024 bytes total) */
    for (uint16_t chunk = 0; chunk < (SSD1309_WIDTH * SSD1309_HEIGHT / 8) / 64; chunk++) {
        if (ssd1309_send_data(init, clear_data, 64) != 0) {
            return 1;
        }
    }
    
    return 0; // Success
}

/**
 * @brief Set display contrast
 * 
 * @param init SSD1309 configuration
 * @param contrast Contrast value (0-255)
 * @return uint8_t 0=success, 1=failure
 */
DriverStatus ssd1309_set_contrast(SSD1309_InitTypeDef *init, uint8_t contrast) {
    if (ssd1309_send_command(init, SSD1309_SETCONTRAST) != 0) return 1;
    if (ssd1309_send_command(init, contrast) != 0) return 1;
    
    /* Update stored contrast value */
    init->Contrast = contrast;
    
    return 0; // Success
}

/**
 * @brief Turn display on/off
 * 
 * @param init SSD1309 configuration
 * @param state Display state (0=off, 1=on)
 * @return uint8_t 0=success, 1=failure
 */
DriverStatus ssd1309_display_on_off(SSD1309_InitTypeDef *init, uint8_t state) {
    if (state) {
        return ssd1309_send_command(init, SSD1309_DISPLAYON);
    } else {
        return ssd1309_send_command(init, SSD1309_DISPLAYOFF);
    }
}

/**
 * @brief Invert display
 * 
 * @param init SSD1309 configuration
 * @param invert Invert state (0=normal, 1=inverted)
 * @return uint8_t 0=success, 1=failure
 */
DriverStatus ssd1309_invert_display(SSD1309_InitTypeDef *init, uint8_t invert) {
    DriverStatus result;
    
    if (invert) {
        result = ssd1309_send_command(init, SSD1309_INVERTDISPLAY);
    } else {
        result = ssd1309_send_command(init, SSD1309_NORMALDISPLAY);
    }
    
    if (result == 0) {
        /* Update stored invert state */
        init->InvertDisplay = invert;
    }
    
    return result;
}

/**
 * @brief Set cursor position
 * 
 * @param init SSD1309 configuration
 * @param column Column position (0-127)
 * @param page Page position (0-7)
 * @return uint8_t 0=success, 1=failure
 */
DriverStatus ssd1309_set_cursor(SSD1309_InitTypeDef *init, uint8_t column, uint8_t page) {
    /* Check bounds */
    if (column >= SSD1309_WIDTH || page >= SSD1309_PAGES) {
        return 1;
    }
    
    /* Set column address */
    if (ssd1309_send_command(init, SSD1309_COLUMNADDR) != 0) return 1;
    if (ssd1309_send_command(init, column) != 0) return 1;       // Start column
    if (ssd1309_send_command(init, SSD1309_WIDTH - 1) != 0) return 1; // End column
    
    /* Set page address */
    if (ssd1309_send_command(init, SSD1309_PAGEADDR) != 0) return 1;
    if (ssd1309_send_command(init, page) != 0) return 1;         // Start page
    if (ssd1309_send_command(init, SSD1309_PAGES - 1) != 0) return 1; // End page
    
    return 0; // Success
}

/**
 * @brief Check if SSD1309 device is ready
 * 
 * @param init SSD1309 configuration
 * @param trials Number of attempts
 * @return uint8_t 0=ready, 1=not ready
 */
DriverStatus ssd1309_is_ready(SSD1309_InitTypeDef *init, uint8_t trials) {
    if ((init == NULL) || (init->I2Cx == NULL)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    return i2c_is_ready(init->I2Cx, init->DevAddress, trials,
                        init->I2CTimeoutCycles);
}
