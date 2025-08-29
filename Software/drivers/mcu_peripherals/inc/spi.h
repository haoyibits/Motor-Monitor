/**
 * @file spi.h
 * @author Haoyi Chen
 * @date 2025-08-18
 * @brief STM32F4 SPI register-level driver header
 *
 * @details This file contains SPI function declarations and constant definitions
 * for STM32F407 series microcontrollers using register-level programming.
 */

#ifndef SPI_H
#define SPI_H

#include "stm32f407xx.h"

/**
 * @name SPI Clock Phase and Polarity
 * @{
 */
#define SPI_CPHA_1EDGE      0x0  /**< First clock transition is the first data capture edge */
#define SPI_CPHA_2EDGE      0x1  /**< Second clock transition is the first data capture edge */
#define SPI_CPOL_LOW        0x0  /**< Clock idle low */
#define SPI_CPOL_HIGH       0x1  /**< Clock idle high */
/** @} */

/**
 * @name SPI Data Frame Format
 * @{
 */
#define SPI_DATASIZE_8BIT   0x0  /**< 8-bit data frame format */
#define SPI_DATASIZE_16BIT  0x1  /**< 16-bit data frame format */
/** @} */

/**
 * @name SPI Mode
 * @{
 */
#define SPI_MODE_MASTER     0x1  /**< Master mode */
#define SPI_MODE_SLAVE      0x0  /**< Slave mode */
/** @} */

/**
 * @name SPI Prescaler
 * @{
 */
#define SPI_PRESCALER_2     0x0  /**< Baud rate = fPCLK/2 */
#define SPI_PRESCALER_4     0x1  /**< Baud rate = fPCLK/4 */
#define SPI_PRESCALER_8     0x2  /**< Baud rate = fPCLK/8 */
#define SPI_PRESCALER_16    0x3  /**< Baud rate = fPCLK/16 */
#define SPI_PRESCALER_32    0x4  /**< Baud rate = fPCLK/32 */
#define SPI_PRESCALER_64    0x5  /**< Baud rate = fPCLK/64 */
#define SPI_PRESCALER_128   0x6  /**< Baud rate = fPCLK/128 */
#define SPI_PRESCALER_256   0x7  /**< Baud rate = fPCLK/256 */
/** @} */

/**
 * @name SPI Frame Format
 * @{
 */
#define SPI_FIRSTBIT_MSB    0x0  /**< MSB transmitted first */
#define SPI_FIRSTBIT_LSB    0x1  /**< LSB transmitted first */
/** @} */

/**
 * @brief SPI Configuration Structure
 */
typedef struct {
    uint8_t mode;           /**< Master or slave mode */
    uint8_t dataSize;       /**< Data frame size */
    uint8_t clockPolarity;  /**< Clock polarity */
    uint8_t clockPhase;     /**< Clock phase */
    uint8_t prescaler;      /**< Baud rate prescaler */
    uint8_t firstBit;       /**< Frame format */
} SPI_Config_t;

/**
 * @brief Initialize SPI peripheral
 * 
 * @param SPIx Pointer to SPI peripheral (SPI1, SPI2, SPI3, etc.)
 * @param config Pointer to SPI configuration structure
 * 
 * @note GPIO pins must be configured separately before calling this function
 */
void spi_init(SPI_TypeDef *SPIx, SPI_Config_t *config);

/**
 * @brief Enable SPI peripheral
 * 
 * @param SPIx Pointer to SPI peripheral
 */
void spi_enable(SPI_TypeDef *SPIx);

/**
 * @brief Disable SPI peripheral
 * 
 * @param SPIx Pointer to SPI peripheral
 */
void spi_disable(SPI_TypeDef *SPIx);

/**
 * @brief Transmit and receive a byte via SPI
 * 
 * @param SPIx Pointer to SPI peripheral
 * @param data Data byte to transmit
 * @return uint8_t Received data byte
 */
uint8_t spi_transmit_receive(SPI_TypeDef *SPIx, uint8_t data);

/**
 * @brief Transmit data via SPI
 * 
 * @param SPIx Pointer to SPI peripheral
 * @param pData Pointer to data buffer
 * @param size Number of bytes to transmit
 */
void spi_transmit(SPI_TypeDef *SPIx, uint8_t *pData, uint16_t size);

/**
 * @brief Receive data via SPI
 * 
 * @param SPIx Pointer to SPI peripheral
 * @param pData Pointer to data buffer
 * @param size Number of bytes to receive
 */
void spi_receive(SPI_TypeDef *SPIx, uint8_t *pData, uint16_t size);

/**
 * @brief Transmit and receive data via SPI
 * 
 * @param SPIx Pointer to SPI peripheral
 * @param pTxData Pointer to transmit data buffer
 * @param pRxData Pointer to receive data buffer
 * @param size Number of bytes to transfer
 */
void spi_transmit_receive_buffer(SPI_TypeDef *SPIx, uint8_t *pTxData, uint8_t *pRxData, uint16_t size);

/**
 * @brief Check if SPI is busy
 * 
 * @param SPIx Pointer to SPI peripheral
 * @return uint8_t 1 if busy, 0 if ready
 */
uint8_t spi_is_busy(SPI_TypeDef *SPIx);

#endif /* SPI_H */