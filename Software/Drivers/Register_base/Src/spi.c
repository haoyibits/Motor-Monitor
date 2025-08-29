/**
 * @file spi.c
 * @author Haoyi Chen
 * @date 2025-08-18
 * @brief STM32F4 SPI register-level driver implementation
 *
 * @details This file contains SPI function implementations for STM32F407
 * series microcontrollers using direct register manipulation.
 */

#include "bsp.h"

void spi_init(SPI_TypeDef *SPIx, SPI_Config_t *config)
{
    // Disable SPI before configuration
    SPIx->CR1 &= ~SPI_CR1_SPE;
    
    // Configure SPI control register 1 (CR1)
    uint32_t cr1_config = 0;
    
    // Set clock phase
    if (config->clockPhase == SPI_CPHA_2EDGE) {
        cr1_config |= SPI_CR1_CPHA;
    }
    
    // Set clock polarity
    if (config->clockPolarity == SPI_CPOL_HIGH) {
        cr1_config |= SPI_CR1_CPOL;
    }
    
    // Set master/slave mode
    if (config->mode == SPI_MODE_MASTER) {
        cr1_config |= SPI_CR1_MSTR;
    }
    
    // Set baud rate prescaler
    cr1_config |= ((config->prescaler & 0x7) << 3);
    
    // Set frame format (MSB/LSB first)
    if (config->firstBit == SPI_FIRSTBIT_LSB) {
        cr1_config |= SPI_CR1_LSBFIRST;
    }
    
    // Set data frame format
    if (config->dataSize == SPI_DATASIZE_16BIT) {
        cr1_config |= SPI_CR1_DFF;
    }
    
    // Software slave management enabled for master mode
    if (config->mode == SPI_MODE_MASTER) {
        cr1_config |= SPI_CR1_SSM | SPI_CR1_SSI;
    }
    
    // Apply configuration
    SPIx->CR1 = cr1_config;
}

void spi_enable(SPI_TypeDef *SPIx)
{
    SPIx->CR1 |= SPI_CR1_SPE;
}

void spi_disable(SPI_TypeDef *SPIx)
{
    SPIx->CR1 &= ~SPI_CR1_SPE;
}

uint8_t spi_transmit_receive(SPI_TypeDef *SPIx, uint8_t data)
{
    volatile uint32_t timeout = 100000; // Timeout counter
    
    // Wait until TX buffer is empty
    timeout = 100000;
    while (!(SPIx->SR & SPI_SR_TXE) && timeout--);
    if (timeout == 0) return 0xFF; // Timeout error
    
    // Send data
    SPIx->DR = data;
    
    // Wait until RX buffer is not empty
    timeout = 100000;
    while (!(SPIx->SR & SPI_SR_RXNE) && timeout--);
    if (timeout == 0) return 0xFF; // Timeout error
    
    // Return received data
    return (uint8_t)SPIx->DR;
}

void spi_transmit(SPI_TypeDef *SPIx, uint8_t *pData, uint16_t size)
{
    uint16_t i;
    
    for (i = 0; i < size; i++) {
        // Wait until TX buffer is empty
        while (!(SPIx->SR & SPI_SR_TXE));
        
        // Send data
        SPIx->DR = pData[i];
        
        // Wait until RX buffer is not empty (clear RX buffer)
        while (!(SPIx->SR & SPI_SR_RXNE));
        
        // Read and discard received data
        (void)SPIx->DR;
    }
    
    // Wait until SPI is not busy
    while (SPIx->SR & SPI_SR_BSY);
}

void spi_receive(SPI_TypeDef *SPIx, uint8_t *pData, uint16_t size)
{
    uint16_t i;
    
    for (i = 0; i < size; i++) {
        // Wait until TX buffer is empty
        while (!(SPIx->SR & SPI_SR_TXE));
        
        // Send dummy byte to generate clock
        SPIx->DR = 0xFF;
        
        // Wait until RX buffer is not empty
        while (!(SPIx->SR & SPI_SR_RXNE));
        
        // Read received data
        pData[i] = (uint8_t)SPIx->DR;
    }
    
    // Wait until SPI is not busy
    while (SPIx->SR & SPI_SR_BSY);
}

void spi_transmit_receive_buffer(SPI_TypeDef *SPIx, uint8_t *pTxData, uint8_t *pRxData, uint16_t size)
{
    uint16_t i;
    
    for (i = 0; i < size; i++) {
        // Wait until TX buffer is empty
        while (!(SPIx->SR & SPI_SR_TXE));
        
        // Send data
        SPIx->DR = pTxData[i];
        
        // Wait until RX buffer is not empty
        while (!(SPIx->SR & SPI_SR_RXNE));
        
        // Read received data
        pRxData[i] = (uint8_t)SPIx->DR;
    }
    
    // Wait until SPI is not busy
    while (SPIx->SR & SPI_SR_BSY);
}

uint8_t spi_is_busy(SPI_TypeDef *SPIx)
{
    return (SPIx->SR & SPI_SR_BSY) ? 1 : 0;
}