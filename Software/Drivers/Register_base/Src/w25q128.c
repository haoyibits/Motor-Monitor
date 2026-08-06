/**
 * @file w25q128.c
 * @brief W25Q128 SPI flash driver implementation.
 */

#include <stddef.h>

#include "w25q128.h"

static uint8_t w25q128_config_valid(const W25Q128_Config_t *config)
{
    return ((config != NULL) && (config->spi != NULL) &&
            (config->cs_port != NULL) && (config->cs_pin < 16U)) ? 1U : 0U;
}

static uint32_t w25q128_spi_timeout(const W25Q128_Config_t *config)
{
    return (config->spi_timeout_cycles != 0U) ?
           config->spi_timeout_cycles : SPI_DEFAULT_TIMEOUT_CYCLES;
}

static void w25q128_cs_select(const W25Q128_Config_t *config)
{
    gpio_write(config->cs_port, config->cs_pin, 0U);
}

static void w25q128_cs_deselect(const W25Q128_Config_t *config)
{
    gpio_write(config->cs_port, config->cs_pin, 1U);
}

static DriverStatus w25q128_transfer_byte(const W25Q128_Config_t *config,
                                          uint8_t tx_data, uint8_t *rx_data)
{
    return spi_transfer_byte(config->spi, tx_data, rx_data,
                             w25q128_spi_timeout(config));
}

static DriverStatus w25q128_send_address(const W25Q128_Config_t *config,
                                         uint32_t address)
{
    uint8_t ignored;
    DriverStatus status = w25q128_transfer_byte(config,
                                                (uint8_t)(address >> 16),
                                                &ignored);
    if (status == DRIVER_STATUS_OK) {
        status = w25q128_transfer_byte(config, (uint8_t)(address >> 8),
                                       &ignored);
    }
    if (status == DRIVER_STATUS_OK) {
        status = w25q128_transfer_byte(config, (uint8_t)address, &ignored);
    }
    return status;
}

static DriverStatus w25q128_send_command(const W25Q128_Config_t *config,
                                         uint8_t command)
{
    uint8_t ignored;
    w25q128_cs_select(config);
    DriverStatus status = w25q128_transfer_byte(config, command, &ignored);
    w25q128_cs_deselect(config);
    return status;
}

static DriverStatus w25q128_erase_addressed(const W25Q128_Config_t *config,
                                            uint8_t command,
                                            uint32_t address,
                                            uint32_t poll_limit)
{
    if ((!w25q128_config_valid(config)) || (address >= W25Q128_CHIP_SIZE)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    DriverStatus status = w25q128_wait_ready(config, poll_limit);
    if (status != DRIVER_STATUS_OK) {
        return status;
    }
    status = w25q128_write_enable(config);
    if (status != DRIVER_STATUS_OK) {
        return status;
    }

    uint8_t ignored;
    w25q128_cs_select(config);
    status = w25q128_transfer_byte(config, command, &ignored);
    if (status == DRIVER_STATUS_OK) {
        status = w25q128_send_address(config, address);
    }
    w25q128_cs_deselect(config);

    return (status == DRIVER_STATUS_OK) ?
           w25q128_wait_ready(config, poll_limit) : status;
}

DriverStatus w25q128_init(const W25Q128_Config_t *config)
{
    if (!w25q128_config_valid(config)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    gpio_init(config->cs_port, config->cs_pin, GPIO_MODE_OUTPUT,
              GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_NOPULL);
    w25q128_cs_deselect(config);
    return w25q128_wake_up(config);
}

DriverStatus w25q128_read_jedec_id(const W25Q128_Config_t *config,
                                   uint8_t *manufacturer_id,
                                   uint16_t *device_id)
{
    if ((!w25q128_config_valid(config)) || (manufacturer_id == NULL) ||
        (device_id == NULL)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    uint8_t id[3] = {0U, 0U, 0U};
    uint8_t ignored;
    w25q128_cs_select(config);
    DriverStatus status = w25q128_transfer_byte(config, W25Q128_CMD_JEDEC_ID,
                                                &ignored);
    for (uint8_t i = 0U; (i < 3U) && (status == DRIVER_STATUS_OK); ++i) {
        status = w25q128_transfer_byte(config, 0xFFU, &id[i]);
    }
    w25q128_cs_deselect(config);

    if (status == DRIVER_STATUS_OK) {
        *manufacturer_id = id[0];
        *device_id = (uint16_t)(((uint16_t)id[1] << 8) | id[2]);
    }
    return status;
}

DriverStatus w25q128_read_status_register(const W25Q128_Config_t *config,
                                          uint8_t *status_register)
{
    if ((!w25q128_config_valid(config)) || (status_register == NULL)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    uint8_t ignored;
    w25q128_cs_select(config);
    DriverStatus status = w25q128_transfer_byte(
        config, W25Q128_CMD_READ_STATUS_REG1, &ignored);
    if (status == DRIVER_STATUS_OK) {
        status = w25q128_transfer_byte(config, 0xFFU, status_register);
    }
    w25q128_cs_deselect(config);
    return status;
}

DriverStatus w25q128_wait_ready(const W25Q128_Config_t *config,
                                uint32_t poll_limit)
{
    if ((!w25q128_config_valid(config)) || (poll_limit == 0U)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    while (poll_limit > 0U) {
        uint8_t status_register;
        DriverStatus status = w25q128_read_status_register(config,
                                                           &status_register);
        if (status != DRIVER_STATUS_OK) {
            return status;
        }
        if ((status_register & W25Q128_STATUS_BUSY) == 0U) {
            return DRIVER_STATUS_OK;
        }
        --poll_limit;
    }
    return DRIVER_STATUS_TIMEOUT;
}

DriverStatus w25q128_write_enable(const W25Q128_Config_t *config)
{
    if (!w25q128_config_valid(config)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    DriverStatus status = w25q128_send_command(config,
                                                W25Q128_CMD_WRITE_ENABLE);
    if (status != DRIVER_STATUS_OK) {
        return status;
    }

    uint8_t status_register;
    status = w25q128_read_status_register(config, &status_register);
    if (status != DRIVER_STATUS_OK) {
        return status;
    }
    return ((status_register & W25Q128_STATUS_WEL) != 0U) ?
           DRIVER_STATUS_OK : DRIVER_STATUS_NOT_READY;
}

DriverStatus w25q128_write_disable(const W25Q128_Config_t *config)
{
    if (!w25q128_config_valid(config)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    return w25q128_send_command(config, W25Q128_CMD_WRITE_DISABLE);
}

DriverStatus w25q128_read_data(const W25Q128_Config_t *config,
                               uint32_t address, uint8_t *buffer,
                               uint16_t length)
{
    if ((!w25q128_config_valid(config)) || (buffer == NULL) || (length == 0U) ||
        (address >= W25Q128_CHIP_SIZE) ||
        ((uint32_t)length > (W25Q128_CHIP_SIZE - address))) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    uint8_t ignored;
    w25q128_cs_select(config);
    DriverStatus status = w25q128_transfer_byte(config,
                                                W25Q128_CMD_READ_DATA,
                                                &ignored);
    if (status == DRIVER_STATUS_OK) {
        status = w25q128_send_address(config, address);
    }
    for (uint16_t i = 0U; (i < length) && (status == DRIVER_STATUS_OK); ++i) {
        status = w25q128_transfer_byte(config, 0xFFU, &buffer[i]);
    }
    w25q128_cs_deselect(config);
    return status;
}

DriverStatus w25q128_page_program(const W25Q128_Config_t *config,
                                  uint32_t address, const uint8_t *buffer,
                                  uint16_t length)
{
    uint32_t page_offset = address & (W25Q128_PAGE_SIZE - 1U);
    if ((!w25q128_config_valid(config)) || (buffer == NULL) || (length == 0U) ||
        (length > W25Q128_PAGE_SIZE) ||
        ((page_offset + length) > W25Q128_PAGE_SIZE) ||
        (address >= W25Q128_CHIP_SIZE) ||
        ((uint32_t)length > (W25Q128_CHIP_SIZE - address))) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    DriverStatus status = w25q128_wait_ready(
        config, W25Q128_PAGE_PROGRAM_POLL_LIMIT);
    if (status == DRIVER_STATUS_OK) {
        status = w25q128_write_enable(config);
    }
    if (status != DRIVER_STATUS_OK) {
        return status;
    }

    uint8_t ignored;
    w25q128_cs_select(config);
    status = w25q128_transfer_byte(config, W25Q128_CMD_PAGE_PROGRAM,
                                   &ignored);
    if (status == DRIVER_STATUS_OK) {
        status = w25q128_send_address(config, address);
    }
    for (uint16_t i = 0U; (i < length) && (status == DRIVER_STATUS_OK); ++i) {
        status = w25q128_transfer_byte(config, buffer[i], &ignored);
    }
    w25q128_cs_deselect(config);

    return (status == DRIVER_STATUS_OK) ?
           w25q128_wait_ready(config, W25Q128_PAGE_PROGRAM_POLL_LIMIT) : status;
}

DriverStatus w25q128_sector_erase(const W25Q128_Config_t *config,
                                  uint32_t address)
{
    return w25q128_erase_addressed(config, W25Q128_CMD_SECTOR_ERASE,
                                   address, W25Q128_SECTOR_ERASE_POLL_LIMIT);
}

DriverStatus w25q128_block_erase_64k(const W25Q128_Config_t *config,
                                     uint32_t address)
{
    return w25q128_erase_addressed(config, W25Q128_CMD_BLOCK_ERASE_64K,
                                   address, W25Q128_BLOCK_ERASE_POLL_LIMIT);
}

DriverStatus w25q128_chip_erase(const W25Q128_Config_t *config)
{
    if (!w25q128_config_valid(config)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    DriverStatus status = w25q128_wait_ready(
        config, W25Q128_CHIP_ERASE_POLL_LIMIT);
    if (status == DRIVER_STATUS_OK) {
        status = w25q128_write_enable(config);
    }
    if (status == DRIVER_STATUS_OK) {
        status = w25q128_send_command(config, W25Q128_CMD_CHIP_ERASE);
    }
    return (status == DRIVER_STATUS_OK) ?
           w25q128_wait_ready(config, W25Q128_CHIP_ERASE_POLL_LIMIT) : status;
}

DriverStatus w25q128_power_down(const W25Q128_Config_t *config)
{
    if (!w25q128_config_valid(config)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    return w25q128_send_command(config, W25Q128_CMD_POWER_DOWN);
}

DriverStatus w25q128_wake_up(const W25Q128_Config_t *config)
{
    if (!w25q128_config_valid(config)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    DriverStatus status = w25q128_send_command(
        config, W25Q128_CMD_RELEASE_POWER_DOWN);
    for (volatile uint32_t delay = 1000U; delay > 0U; --delay) {
        __NOP();
    }
    return status;
}

DriverStatus w25q128_is_busy(const W25Q128_Config_t *config,
                             uint8_t *is_busy)
{
    if (is_busy == NULL) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    uint8_t status_register;
    DriverStatus status = w25q128_read_status_register(config,
                                                       &status_register);
    if (status == DRIVER_STATUS_OK) {
        *is_busy = ((status_register & W25Q128_STATUS_BUSY) != 0U) ? 1U : 0U;
    }
    return status;
}
