/**
 * @file w25q128.c
 * @author Haoyi Chen
 * @date 2025-08-18
 * @brief W25Q128 SPI Flash Memory driver implementation
 *
 * @details This file contains the implementation of W25Q128 128Mbit SPI Flash
 * Memory driver functions using register-level SPI communication.
 */

#include "bsp.h"

// Static configuration pointer for internal use
static W25Q128_Config_t *flash_config = NULL;

/**
 * @brief Select the flash chip (CS low)
 */
static void w25q128_cs_select(void)
{
    if (flash_config) {
        gpio_write(flash_config->cs_port, flash_config->cs_pin, 0);
    }
}

/**
 * @brief Deselect the flash chip (CS high)
 */
static void w25q128_cs_deselect(void)
{
    if (flash_config) {
        gpio_write(flash_config->cs_port, flash_config->cs_pin, 1);
    }
}

void w25q128_init(W25Q128_Config_t *config)
{
    flash_config = config;
    
    // Initialize CS pin as output, high
    gpio_init(config->cs_port, config->cs_pin, GPIO_MODE_OUTPUT, 
              GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_NOPULL);
    
    // Deselect chip
    w25q128_cs_deselect();
    
    // Wake up the flash in case it's in power-down mode
    w25q128_wake_up(config);
}

uint8_t w25q128_read_jedec_id(W25Q128_Config_t *config, uint8_t *manufacturer_id, uint16_t *device_id)
{
    uint8_t id_buffer[3];
    
    w25q128_cs_select();
    
    // Send JEDEC ID command
    spi_transmit_receive(config->spi, W25Q128_CMD_JEDEC_ID);
    
    // Read 3 bytes: Manufacturer ID, Device ID high, Device ID low
    id_buffer[0] = spi_transmit_receive(config->spi, 0xFF);  // Manufacturer ID
    id_buffer[1] = spi_transmit_receive(config->spi, 0xFF);  // Device ID high byte
    id_buffer[2] = spi_transmit_receive(config->spi, 0xFF);  // Device ID low byte
    
    w25q128_cs_deselect();
    
    *manufacturer_id = id_buffer[0];
    *device_id = (id_buffer[1] << 8) | id_buffer[2];
    
    // Verify it's a W25Q128
    return (*manufacturer_id == W25Q128_JEDEC_MANUFACTURER_ID && 
            *device_id == W25Q128_JEDEC_DEVICE_ID) ? 1 : 0;
}

uint8_t w25q128_read_status_register(W25Q128_Config_t *config)
{
    uint8_t status;
    
    w25q128_cs_select();
    
    // Send read status register command
    spi_transmit_receive(config->spi, W25Q128_CMD_READ_STATUS_REG1);
    
    // Read status
    status = spi_transmit_receive(config->spi, 0xFF);
    
    w25q128_cs_deselect();
    
    return status;
}

void w25q128_wait_for_write_complete(W25Q128_Config_t *config)
{
    while (w25q128_read_status_register(config) & W25Q128_STATUS_BUSY) {
        // Wait for write operation to complete
    }
}

void w25q128_write_enable(W25Q128_Config_t *config)
{
    w25q128_cs_select();
    
    // Send write enable command
    spi_transmit_receive(config->spi, W25Q128_CMD_WRITE_ENABLE);
    
    w25q128_cs_deselect();
}

void w25q128_write_disable(W25Q128_Config_t *config)
{
    w25q128_cs_select();
    
    // Send write disable command
    spi_transmit_receive(config->spi, W25Q128_CMD_WRITE_DISABLE);
    
    w25q128_cs_deselect();
}

void w25q128_read_data(W25Q128_Config_t *config, uint32_t address, uint8_t *buffer, uint16_t length)
{
    uint16_t i;
    
    w25q128_cs_select();
    
    // Send read command
    spi_transmit_receive(config->spi, W25Q128_CMD_READ_DATA);
    
    // Send 24-bit address
    spi_transmit_receive(config->spi, (address >> 16) & 0xFF);
    spi_transmit_receive(config->spi, (address >> 8) & 0xFF);
    spi_transmit_receive(config->spi, address & 0xFF);
    
    // Read data
    for (i = 0; i < length; i++) {
        buffer[i] = spi_transmit_receive(config->spi, 0xFF);
    }
    
    w25q128_cs_deselect();
}

void w25q128_page_program(W25Q128_Config_t *config, uint32_t address, uint8_t *buffer, uint16_t length)
{
    uint16_t i;
    
    // Ensure length doesn't exceed page size
    if (length > W25Q128_PAGE_SIZE) {
        length = W25Q128_PAGE_SIZE;
    }
    
    // Enable write operations
    w25q128_write_enable(config);
    
    w25q128_cs_select();
    
    // Send page program command
    spi_transmit_receive(config->spi, W25Q128_CMD_PAGE_PROGRAM);
    
    // Send 24-bit address
    spi_transmit_receive(config->spi, (address >> 16) & 0xFF);
    spi_transmit_receive(config->spi, (address >> 8) & 0xFF);
    spi_transmit_receive(config->spi, address & 0xFF);
    
    // Write data
    for (i = 0; i < length; i++) {
        spi_transmit_receive(config->spi, buffer[i]);
    }
    
    w25q128_cs_deselect();
    
    // Wait for write to complete
    w25q128_wait_for_write_complete(config);
}

void w25q128_sector_erase(W25Q128_Config_t *config, uint32_t address)
{
    // Enable write operations
    w25q128_write_enable(config);
    
    w25q128_cs_select();
    
    // Send sector erase command
    spi_transmit_receive(config->spi, W25Q128_CMD_SECTOR_ERASE);
    
    // Send 24-bit address
    spi_transmit_receive(config->spi, (address >> 16) & 0xFF);
    spi_transmit_receive(config->spi, (address >> 8) & 0xFF);
    spi_transmit_receive(config->spi, address & 0xFF);
    
    w25q128_cs_deselect();
    
    // Wait for erase to complete
    w25q128_wait_for_write_complete(config);
}

void w25q128_block_erase_64k(W25Q128_Config_t *config, uint32_t address)
{
    // Enable write operations
    w25q128_write_enable(config);
    
    w25q128_cs_select();
    
    // Send 64KB block erase command
    spi_transmit_receive(config->spi, W25Q128_CMD_BLOCK_ERASE_64K);
    
    // Send 24-bit address
    spi_transmit_receive(config->spi, (address >> 16) & 0xFF);
    spi_transmit_receive(config->spi, (address >> 8) & 0xFF);
    spi_transmit_receive(config->spi, address & 0xFF);
    
    w25q128_cs_deselect();
    
    // Wait for erase to complete
    w25q128_wait_for_write_complete(config);
}

void w25q128_chip_erase(W25Q128_Config_t *config)
{
    // Enable write operations
    w25q128_write_enable(config);
    
    w25q128_cs_select();
    
    // Send chip erase command
    spi_transmit_receive(config->spi, W25Q128_CMD_CHIP_ERASE);
    
    w25q128_cs_deselect();
    
    // Wait for erase to complete (this takes a long time)
    w25q128_wait_for_write_complete(config);
}

void w25q128_power_down(W25Q128_Config_t *config)
{
    w25q128_cs_select();
    
    // Send power down command
    spi_transmit_receive(config->spi, W25Q128_CMD_POWER_DOWN);
    
    w25q128_cs_deselect();
}

void w25q128_wake_up(W25Q128_Config_t *config)
{
    w25q128_cs_select();
    
    // Send release power down command
    spi_transmit_receive(config->spi, W25Q128_CMD_RELEASE_POWER_DOWN);
    
    w25q128_cs_deselect();
    
    // Wait for device to wake up (typically 3us)
    // Simple delay loop (adjust as needed based on system clock)
    volatile uint32_t delay = 1000;
    while (delay--);
}

uint8_t w25q128_is_busy(W25Q128_Config_t *config)
{
    return (w25q128_read_status_register(config) & W25Q128_STATUS_BUSY) ? 1 : 0;
}