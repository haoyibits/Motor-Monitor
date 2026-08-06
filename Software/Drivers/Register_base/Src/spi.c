/**
 * @file spi.c
 * @brief STM32F4 SPI register-level driver implementation.
 */

#include <stddef.h>

#include "spi.h"

static DriverStatus spi_wait_flag(SPI_TypeDef *spi, uint32_t flag,
                                  uint8_t expected_set, uint32_t timeout)
{
    while (timeout > 0U) {
        uint8_t is_set = ((spi->SR & flag) != 0U) ? 1U : 0U;
        if (is_set == expected_set) {
            return DRIVER_STATUS_OK;
        }
        --timeout;
    }
    return DRIVER_STATUS_TIMEOUT;
}

static DriverStatus spi_check_error(SPI_TypeDef *spi)
{
    if ((spi->SR & (SPI_SR_MODF | SPI_SR_CRCERR)) != 0U) {
        return DRIVER_STATUS_IO_ERROR;
    }
    return DRIVER_STATUS_OK;
}

DriverStatus spi_init(SPI_TypeDef *spi, const SPI_Config_t *config)
{
    if ((spi == NULL) || (config == NULL) ||
        (config->prescaler > SPI_PRESCALER_256) ||
        (config->dataSize != SPI_DATASIZE_8BIT) ||
        ((config->mode != SPI_MODE_MASTER) &&
         (config->mode != SPI_MODE_SLAVE)) ||
        ((config->clockPhase != SPI_CPHA_1EDGE) &&
         (config->clockPhase != SPI_CPHA_2EDGE)) ||
        ((config->clockPolarity != SPI_CPOL_LOW) &&
         (config->clockPolarity != SPI_CPOL_HIGH)) ||
        ((config->firstBit != SPI_FIRSTBIT_MSB) &&
         (config->firstBit != SPI_FIRSTBIT_LSB))) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    spi->CR1 &= ~SPI_CR1_SPE;

    uint32_t cr1 = ((uint32_t)(config->prescaler & 0x7U) << SPI_CR1_BR_Pos);
    if (config->clockPhase == SPI_CPHA_2EDGE) {
        cr1 |= SPI_CR1_CPHA;
    }
    if (config->clockPolarity == SPI_CPOL_HIGH) {
        cr1 |= SPI_CR1_CPOL;
    }
    if (config->mode == SPI_MODE_MASTER) {
        cr1 |= SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
    }
    if (config->firstBit == SPI_FIRSTBIT_LSB) {
        cr1 |= SPI_CR1_LSBFIRST;
    }

    spi->CR1 = cr1;
    spi->CR2 = 0U;
    return DRIVER_STATUS_OK;
}

DriverStatus spi_enable(SPI_TypeDef *spi)
{
    if (spi == NULL) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    spi->CR1 |= SPI_CR1_SPE;
    return DRIVER_STATUS_OK;
}

DriverStatus spi_disable(SPI_TypeDef *spi)
{
    if (spi == NULL) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    spi->CR1 &= ~SPI_CR1_SPE;
    return DRIVER_STATUS_OK;
}

DriverStatus spi_transfer_byte(SPI_TypeDef *spi, uint8_t tx_data,
                               uint8_t *rx_data, uint32_t timeout_cycles)
{
    if ((spi == NULL) || (rx_data == NULL) || (timeout_cycles == 0U) ||
        ((spi->CR1 & SPI_CR1_SPE) == 0U) || ((spi->CR1 & SPI_CR1_DFF) != 0U)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    DriverStatus status = spi_wait_flag(spi, SPI_SR_TXE, 1U, timeout_cycles);
    if (status != DRIVER_STATUS_OK) {
        return status;
    }

    *(__IO uint8_t *)&spi->DR = tx_data;

    status = spi_wait_flag(spi, SPI_SR_RXNE, 1U, timeout_cycles);
    if (status != DRIVER_STATUS_OK) {
        return status;
    }

    *rx_data = *(__IO uint8_t *)&spi->DR;
    return spi_check_error(spi);
}

DriverStatus spi_transfer(SPI_TypeDef *spi, const uint8_t *tx_data,
                          uint8_t *rx_data, uint16_t size,
                          uint32_t timeout_cycles)
{
    if ((spi == NULL) || (size == 0U) || (timeout_cycles == 0U) ||
        ((tx_data == NULL) && (rx_data == NULL))) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    for (uint16_t i = 0U; i < size; ++i) {
        uint8_t received = 0U;
        uint8_t outgoing = (tx_data != NULL) ? tx_data[i] : 0xFFU;
        DriverStatus status = spi_transfer_byte(spi, outgoing, &received,
                                                timeout_cycles);
        if (status != DRIVER_STATUS_OK) {
            return status;
        }
        if (rx_data != NULL) {
            rx_data[i] = received;
        }
    }

    return spi_wait_flag(spi, SPI_SR_BSY, 0U, timeout_cycles);
}

DriverStatus spi_transmit(SPI_TypeDef *spi, const uint8_t *data,
                          uint16_t size, uint32_t timeout_cycles)
{
    if (data == NULL) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    return spi_transfer(spi, data, NULL, size, timeout_cycles);
}

DriverStatus spi_receive(SPI_TypeDef *spi, uint8_t *data,
                         uint16_t size, uint32_t timeout_cycles)
{
    if (data == NULL) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    return spi_transfer(spi, NULL, data, size, timeout_cycles);
}

uint8_t spi_is_busy(SPI_TypeDef *spi)
{
    return ((spi != NULL) && ((spi->SR & SPI_SR_BSY) != 0U)) ? 1U : 0U;
}
