/**
 * @file i2c_oled.c
 * @author Haoyi Chen
 * @date 2025-08-08
 * @brief Implementation of streamlined I2C driver for SSD1306 OLED
 *
 * @details This file contains the implementation of I2C functions optimized for SSD1306 OLED displays,
 * removing unnecessary features to improve code efficiency and readability.
 */

#include <stdio.h>
#include "../Inc/i2c_oled.h"
#include "../Inc/gpio.h"

/**
 * @brief Initialize I2C for OLED display
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param init I2C initialization parameters
 */
void i2c_oled_init(I2C_TypeDef *I2Cx, I2C_OLED_InitTypeDef *init) {
    uint32_t pclk1 = 42000000; // 42 MHz (modify according to actual project)
    uint32_t freqrange = (uint32_t)(pclk1 / 1000000); // MHz
    
    /* Disable I2C */
    I2Cx->CR1 &= ~I2C_CR1_PE;
    
    /* Set I2C clock */
    I2Cx->CR2 = freqrange;
    
    /* Set I2C timing parameters */
    I2Cx->CCR = 0;
    
    if (init->ClockSpeed <= 100000) {
        /* Standard mode (100 KHz) */
        I2Cx->CCR = (uint16_t)(pclk1 / (init->ClockSpeed * 2));
        I2Cx->TRISE = freqrange + 1;
    }
    else {
        /* Fast mode (400 KHz) */
        I2Cx->CCR = I2C_CCR_FS | init->DutyCycle;
        
        if (init->DutyCycle == I2C_DUTYCYCLE_2) {
            I2Cx->CCR |= (uint16_t)(pclk1 / (init->ClockSpeed * 3));
        }
        else { // I2C_DUTYCYCLE_16_9
            I2Cx->CCR |= (uint16_t)(pclk1 / (init->ClockSpeed * 25));
        }
        
        I2Cx->TRISE = (uint16_t)((freqrange * 300) / 1000) + 1;
    }
    
    /* Set own address (not important, as we are the master) */
    I2Cx->OAR1 = 0x4000; // Add reserved bit
    
    /* Enable I2C */
    I2Cx->CR1 |= I2C_CR1_PE;
}

/**
 * @brief Initialize I2C GPIO pins
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param GPIOx GPIO port (GPIOA, GPIOB, etc.)
 * @param SCL_Pin SCL pin number (0-15)
 * @param SDA_Pin SDA pin number (0-15)
 */
void i2c_oled_gpio_init(I2C_TypeDef *I2Cx, GPIO_TypeDef *GPIOx, uint8_t SCL_Pin, uint8_t SDA_Pin) {
    uint8_t alternate_function;
    
    /* Determine alternate function number */
    if (I2Cx == I2C1 || I2Cx == I2C2) {
        alternate_function = 4; // AF4 for I2C1 and I2C2
    } else {
        alternate_function = 9; // AF9 for I2C3
    }
    
    /* Configure SCL and SDA as alternate function open-drain output */
    gpio_init(GPIOx, SCL_Pin, 0x02, 0x01, 0x03, 0x00); // Alternate function, open-drain, high-speed, no pull-up or pull-down
    gpio_set_af(GPIOx, SCL_Pin, alternate_function);
    
    gpio_init(GPIOx, SDA_Pin, 0x02, 0x01, 0x03, 0x00); // Alternate function, open-drain, high-speed, no pull-up or pull-down
    gpio_set_af(GPIOx, SDA_Pin, alternate_function);
}

/**
 * @brief Wait for I2C event
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param event Event flag to wait for
 * @return uint8_t 0=success, 1=failure (timeout)
 */
static uint8_t i2c_oled_wait_event(I2C_TypeDef *I2Cx, uint32_t event) {
    uint32_t timeout = 10000;
    //uint32_t sr;
    
    while (timeout--) {
        if (event < 0x10000) {
            /* Event in SR1 */
            if ((I2Cx->SR1 & event) == event) {
                return 0;
            }
        }
        else {
            /* Event in SR2 */
            if ((I2Cx->SR2 & (event & 0xFFFF)) == (event & 0xFFFF)) {
                return 0;
            }
        }
    }
    
    return 1; // Timeout
}

/**
 * @brief Send command to OLED display
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param DevAddress I2C address of the OLED
 * @param cmd Command to send
 * @return uint8_t 0=success, 1=failure
 */
uint8_t i2c_oled_send_command(I2C_TypeDef *I2Cx, uint8_t DevAddress, uint8_t cmd) {
    /* Wait for I2C bus to be idle */
    if (i2c_oled_wait_event(I2Cx, 0x20000) != 0) {
        return 1; // Bus busy
    }
    
    /* Generate start condition */
    I2Cx->CR1 |= I2C_CR1_START;
    
    /* Wait for start condition to be generated */
    if (i2c_oled_wait_event(I2Cx, I2C_SR1_SB) != 0) {
        return 1;
    }
    
    /* Send OLED address (write mode) */
    I2Cx->DR = DevAddress << 1;
    
    /* Wait for address to be sent */
    if (i2c_oled_wait_event(I2Cx, I2C_SR1_ADDR) != 0) {
        I2Cx->CR1 |= I2C_CR1_STOP;
        return 1;
    }
    
    /* Clear ADDR flag */
    uint32_t tmp = I2Cx->SR1;
    tmp = I2Cx->SR2;
    (void)tmp;
    
    /* Send control byte (0x00 = command) */
    I2Cx->DR = 0x00;
    
    /* Wait for data to be sent */
    if (i2c_oled_wait_event(I2Cx, I2C_SR1_TXE) != 0) {
        I2Cx->CR1 |= I2C_CR1_STOP;
        return 1;
    }
    
    /* Send command byte */
    I2Cx->DR = cmd;
    
    /* Wait for data to be sent */
    if (i2c_oled_wait_event(I2Cx, I2C_SR1_BTF) != 0) {
        I2Cx->CR1 |= I2C_CR1_STOP;
        return 1;
    }
    
    /* Generate stop condition */
    I2Cx->CR1 |= I2C_CR1_STOP;
    
    return 0; // Success
}

/**
 * @brief Send data to OLED display
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param DevAddress I2C address of the OLED
 * @param data Data buffer pointer
 * @param size Data size
 * @return uint8_t 0=success, 1=failure
 */
uint8_t i2c_oled_send_data(I2C_TypeDef *I2Cx, uint8_t DevAddress, uint8_t* data, uint16_t size) {
    /* Check parameters */
    if (size == 0 || data == NULL) {
        return 1;
    }
    
    /* Wait for I2C bus to be idle */
    if (i2c_oled_wait_event(I2Cx, 0x20000) != 0) {
        return 1; // Bus busy
    }
    
    /* Generate start condition */
    I2Cx->CR1 |= I2C_CR1_START;
    
    /* Wait for start condition to be generated */
    if (i2c_oled_wait_event(I2Cx, I2C_SR1_SB) != 0) {
        return 1;
    }
    
    /* Send OLED address (write mode) */
    I2Cx->DR = DevAddress << 1;
    
    /* Wait for address to be sent */
    if (i2c_oled_wait_event(I2Cx, I2C_SR1_ADDR) != 0) {
        I2Cx->CR1 |= I2C_CR1_STOP;
        return 1;
    }
    
    /* Clear ADDR flag */
    uint32_t tmp = I2Cx->SR1;
    tmp = I2Cx->SR2;
    (void)tmp;
    
    /* Send control byte (0x40 = data) */
    I2Cx->DR = 0x40;
    
    /* Wait for data to be sent */
    if (i2c_oled_wait_event(I2Cx, I2C_SR1_TXE) != 0) {
        I2Cx->CR1 |= I2C_CR1_STOP;
        return 1;
    }
    
    /* Send data */
    for (uint16_t i = 0; i < size; i++) {
        I2Cx->DR = data[i];
        
        /* Wait for data to be sent */
        if (i < size - 1) {
            if (i2c_oled_wait_event(I2Cx, I2C_SR1_TXE) != 0) {
                I2Cx->CR1 |= I2C_CR1_STOP;
                return 1;
            }
        } else {
            if (i2c_oled_wait_event(I2Cx, I2C_SR1_BTF) != 0) {
                I2Cx->CR1 |= I2C_CR1_STOP;
                return 1;
            }
        }
    }
    
    /* Generate stop condition */
    I2Cx->CR1 |= I2C_CR1_STOP;
    
    return 0; // Success
}

/**
 * @brief Check if OLED device is ready
 * 
 * @param I2Cx I2C peripheral (I2C1, I2C2 or I2C3)
 * @param DevAddress I2C address of the OLED
 * @param Trials Number of trials
 * @return uint8_t 0=ready, 1=not ready
 */
uint8_t i2c_oled_is_ready(I2C_TypeDef *I2Cx, uint8_t DevAddress, uint8_t Trials) {
    while (Trials--) {
        /* Wait for bus to be idle */
        if (i2c_oled_wait_event(I2Cx, 0x20000) != 0) {
            continue;
        }
        
        /* Generate start condition */
        I2Cx->CR1 |= I2C_CR1_START;
        
        /* Wait for start condition to be generated */
        if (i2c_oled_wait_event(I2Cx, I2C_SR1_SB) != 0) {
            I2Cx->CR1 |= I2C_CR1_STOP;
            continue;
        }
        
        /* Send address */
        I2Cx->DR = DevAddress << 1;
        
        /* Wait for response (ADDR or AF flag) */
        uint32_t timeout = 1000;
        while (((I2Cx->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF)) == 0) && timeout--) {
            // Wait
        }
        
        /* Check if ACK received (ADDR position 1) */
        if (I2Cx->SR1 & I2C_SR1_ADDR) {
            /* Device responded, clear ADDR flag */
            uint32_t tmp = I2Cx->SR1;
            tmp = I2Cx->SR2;
            (void)tmp;
            
            /* Generate stop condition */
            I2Cx->CR1 |= I2C_CR1_STOP;
            
            return 0; // Device is ready
        } else {
            /* Device did not respond, clear AF flag */
            I2Cx->SR1 &= ~I2C_SR1_AF;
        }
        
        /* Generate stop condition */
        I2Cx->CR1 |= I2C_CR1_STOP;
        
        /* Short delay */
        for (uint32_t i = 0; i < 10000; i++) {
            __NOP();
        }
    }
    
    return 1; // Device is not ready
}
