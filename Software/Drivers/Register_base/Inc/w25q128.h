/**
 * @file w25q128.h
 * @brief W25Q128 SPI flash driver.
 */

#ifndef W25Q128_H
#define W25Q128_H

#include "driver_status.h"
#include "gpio.h"
#include "spi.h"

#define W25Q128_CMD_WRITE_ENABLE        0x06U
#define W25Q128_CMD_WRITE_DISABLE       0x04U
#define W25Q128_CMD_READ_STATUS_REG1    0x05U
#define W25Q128_CMD_PAGE_PROGRAM        0x02U
#define W25Q128_CMD_SECTOR_ERASE        0x20U
#define W25Q128_CMD_BLOCK_ERASE_64K     0xD8U
#define W25Q128_CMD_CHIP_ERASE          0xC7U
#define W25Q128_CMD_READ_DATA           0x03U
#define W25Q128_CMD_POWER_DOWN          0xB9U
#define W25Q128_CMD_RELEASE_POWER_DOWN  0xABU
#define W25Q128_CMD_JEDEC_ID            0x9FU

#define W25Q128_STATUS_BUSY             0x01U
#define W25Q128_STATUS_WEL              0x02U

#define W25Q128_PAGE_SIZE               256U
#define W25Q128_SECTOR_SIZE             4096U
#define W25Q128_BLOCK_SIZE_64K          65536U
#define W25Q128_CHIP_SIZE               16777216UL

#define W25Q128_JEDEC_MANUFACTURER_ID   0xEFU
#define W25Q128_JEDEC_DEVICE_ID         0x4018U

#define W25Q128_PAGE_PROGRAM_POLL_LIMIT 100000UL
#define W25Q128_SECTOR_ERASE_POLL_LIMIT 2000000UL
#define W25Q128_BLOCK_ERASE_POLL_LIMIT  5000000UL
#define W25Q128_CHIP_ERASE_POLL_LIMIT   120000000UL

typedef struct {
    SPI_TypeDef *spi;
    GPIO_TypeDef *cs_port;
    uint8_t cs_pin;
    uint32_t spi_timeout_cycles;
} W25Q128_Config_t;

DriverStatus w25q128_init(const W25Q128_Config_t *config);
DriverStatus w25q128_read_jedec_id(const W25Q128_Config_t *config,
                                   uint8_t *manufacturer_id,
                                   uint16_t *device_id);
DriverStatus w25q128_read_status_register(const W25Q128_Config_t *config,
                                          uint8_t *status_register);
DriverStatus w25q128_wait_ready(const W25Q128_Config_t *config,
                                uint32_t poll_limit);
DriverStatus w25q128_write_enable(const W25Q128_Config_t *config);
DriverStatus w25q128_write_disable(const W25Q128_Config_t *config);
DriverStatus w25q128_read_data(const W25Q128_Config_t *config,
                               uint32_t address, uint8_t *buffer,
                               uint16_t length);
DriverStatus w25q128_page_program(const W25Q128_Config_t *config,
                                  uint32_t address, const uint8_t *buffer,
                                  uint16_t length);
DriverStatus w25q128_sector_erase(const W25Q128_Config_t *config,
                                  uint32_t address);
DriverStatus w25q128_block_erase_64k(const W25Q128_Config_t *config,
                                     uint32_t address);
DriverStatus w25q128_chip_erase(const W25Q128_Config_t *config);
DriverStatus w25q128_power_down(const W25Q128_Config_t *config);
DriverStatus w25q128_wake_up(const W25Q128_Config_t *config);
DriverStatus w25q128_is_busy(const W25Q128_Config_t *config,
                             uint8_t *is_busy);

#endif /* W25Q128_H */
