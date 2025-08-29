/**
 * @file w25q128.h
 * @author Haoyi Chen
 * @date 2025-08-18
 * @brief W25Q128 SPI Flash Memory driver header
 *
 * @details This file contains function declarations and constants for the W25Q128
 * 128Mbit (16MB) SPI Flash Memory. The driver provides basic operations including
 * read, write, erase, and status checking.
 */

#ifndef W25Q128_H
#define W25Q128_H

#include "stm32f407xx.h"
#include "spi.h"
#include "gpio.h"

/**
 * @name W25Q128 Commands
 * @{
 */
#define W25Q128_CMD_WRITE_ENABLE        0x06  /**< Write enable */
#define W25Q128_CMD_WRITE_DISABLE       0x04  /**< Write disable */
#define W25Q128_CMD_READ_STATUS_REG1    0x05  /**< Read status register 1 */
#define W25Q128_CMD_READ_STATUS_REG2    0x35  /**< Read status register 2 */
#define W25Q128_CMD_WRITE_STATUS_REG    0x01  /**< Write status register */
#define W25Q128_CMD_PAGE_PROGRAM        0x02  /**< Page program */
#define W25Q128_CMD_QUAD_PAGE_PROGRAM   0x32  /**< Quad page program */
#define W25Q128_CMD_SECTOR_ERASE        0x20  /**< Sector erase (4KB) */
#define W25Q128_CMD_BLOCK_ERASE_32K     0x52  /**< Block erase (32KB) */
#define W25Q128_CMD_BLOCK_ERASE_64K     0xD8  /**< Block erase (64KB) */
#define W25Q128_CMD_CHIP_ERASE          0xC7  /**< Chip erase */
#define W25Q128_CMD_READ_DATA           0x03  /**< Read data */
#define W25Q128_CMD_FAST_READ           0x0B  /**< Fast read data */
#define W25Q128_CMD_POWER_DOWN          0xB9  /**< Power down */
#define W25Q128_CMD_RELEASE_POWER_DOWN  0xAB  /**< Release power down */
#define W25Q128_CMD_DEVICE_ID           0xAB  /**< Device ID */
#define W25Q128_CMD_JEDEC_ID            0x9F  /**< JEDEC ID */
#define W25Q128_CMD_UNIQUE_ID           0x4B  /**< Unique ID */
/** @} */

/**
 * @name W25Q128 Status Register Bits
 * @{
 */
#define W25Q128_STATUS_BUSY             0x01  /**< Busy bit */
#define W25Q128_STATUS_WEL              0x02  /**< Write enable latch */
#define W25Q128_STATUS_BP0              0x04  /**< Block protect bit 0 */
#define W25Q128_STATUS_BP1              0x08  /**< Block protect bit 1 */
#define W25Q128_STATUS_BP2              0x10  /**< Block protect bit 2 */
#define W25Q128_STATUS_TB               0x20  /**< Top/Bottom protect */
#define W25Q128_STATUS_SEC              0x40  /**< Sector protect */
#define W25Q128_STATUS_SRP0             0x80  /**< Status register protect 0 */
/** @} */

/**
 * @name W25Q128 Memory Organization
 * @{
 */
#define W25Q128_PAGE_SIZE               256   /**< Page size in bytes */
#define W25Q128_SECTOR_SIZE             4096  /**< Sector size in bytes (4KB) */
#define W25Q128_BLOCK_SIZE_32K          32768 /**< 32KB block size */
#define W25Q128_BLOCK_SIZE_64K          65536 /**< 64KB block size */
#define W25Q128_CHIP_SIZE               16777216  /**< Total chip size (16MB) */
#define W25Q128_SECTOR_COUNT            4096  /**< Total number of sectors */
#define W25Q128_PAGE_COUNT              65536 /**< Total number of pages */
/** @} */

/**
 * @name W25Q128 JEDEC ID Values
 * @{
 */
#define W25Q128_JEDEC_MANUFACTURER_ID   0xEF  /**< Winbond manufacturer ID */
#define W25Q128_JEDEC_DEVICE_ID         0x4018  /**< W25Q128 device ID */
/** @} */

/**
 * @brief W25Q128 Configuration Structure
 */
typedef struct {
    SPI_TypeDef *spi;       /**< SPI peripheral to use */
    GPIO_TypeDef *cs_port;  /**< Chip select GPIO port */
    uint8_t cs_pin;         /**< Chip select GPIO pin */
} W25Q128_Config_t;

/**
 * @brief Initialize W25Q128 flash memory
 * 
 * @param config Pointer to W25Q128 configuration structure
 * 
 * @note SPI peripheral must be initialized before calling this function
 */
void w25q128_init(W25Q128_Config_t *config);

/**
 * @brief Read JEDEC ID from W25Q128
 * 
 * @param config Pointer to W25Q128 configuration structure
 * @param manufacturer_id Pointer to store manufacturer ID
 * @param device_id Pointer to store device ID (16-bit)
 * @return uint8_t 1 if successful, 0 if failed
 */
uint8_t w25q128_read_jedec_id(W25Q128_Config_t *config, uint8_t *manufacturer_id, uint16_t *device_id);

/**
 * @brief Read status register
 * 
 * @param config Pointer to W25Q128 configuration structure
 * @return uint8_t Status register value
 */
uint8_t w25q128_read_status_register(W25Q128_Config_t *config);

/**
 * @brief Wait for write operation to complete
 * 
 * @param config Pointer to W25Q128 configuration structure
 */
void w25q128_wait_for_write_complete(W25Q128_Config_t *config);

/**
 * @brief Enable write operations
 * 
 * @param config Pointer to W25Q128 configuration structure
 */
void w25q128_write_enable(W25Q128_Config_t *config);

/**
 * @brief Disable write operations
 * 
 * @param config Pointer to W25Q128 configuration structure
 */
void w25q128_write_disable(W25Q128_Config_t *config);

/**
 * @brief Read data from flash memory
 * 
 * @param config Pointer to W25Q128 configuration structure
 * @param address Start address to read from (24-bit)
 * @param buffer Pointer to data buffer
 * @param length Number of bytes to read
 */
void w25q128_read_data(W25Q128_Config_t *config, uint32_t address, uint8_t *buffer, uint16_t length);

/**
 * @brief Write data to a page
 * 
 * @param config Pointer to W25Q128 configuration structure
 * @param address Start address to write to (24-bit)
 * @param buffer Pointer to data buffer
 * @param length Number of bytes to write (max 256 bytes)
 * 
 * @note Address must be page-aligned and length must not exceed page boundary
 */
void w25q128_page_program(W25Q128_Config_t *config, uint32_t address, uint8_t *buffer, uint16_t length);

/**
 * @brief Erase a 4KB sector
 * 
 * @param config Pointer to W25Q128 configuration structure
 * @param address Address within the sector to erase
 */
void w25q128_sector_erase(W25Q128_Config_t *config, uint32_t address);

/**
 * @brief Erase a 64KB block
 * 
 * @param config Pointer to W25Q128 configuration structure
 * @param address Address within the block to erase
 */
void w25q128_block_erase_64k(W25Q128_Config_t *config, uint32_t address);

/**
 * @brief Erase entire chip
 * 
 * @param config Pointer to W25Q128 configuration structure
 * 
 * @warning This operation takes a long time (several seconds)
 */
void w25q128_chip_erase(W25Q128_Config_t *config);

/**
 * @brief Enter power-down mode
 * 
 * @param config Pointer to W25Q128 configuration structure
 */
void w25q128_power_down(W25Q128_Config_t *config);

/**
 * @brief Exit power-down mode
 * 
 * @param config Pointer to W25Q128 configuration structure
 */
void w25q128_wake_up(W25Q128_Config_t *config);

/**
 * @brief Check if flash is busy
 * 
 * @param config Pointer to W25Q128 configuration structure
 * @return uint8_t 1 if busy, 0 if ready
 */
uint8_t w25q128_is_busy(W25Q128_Config_t *config);

#endif /* W25Q128_H */