/**
 * @file i2c.h
 * @brief STM32F4 register-level I2C master driver.
 */

#ifndef I2C_H
#define I2C_H

#include "driver_status.h"
#include "stm32f407xx.h"

#define I2C_DEFAULT_TIMEOUT_CYCLES 100000U

typedef struct {
    uint32_t ClockSpeed;
    uint32_t DutyCycle;
} I2C_InitTypeDef;

#define I2C_DUTYCYCLE_2            0x00000000U
#define I2C_DUTYCYCLE_16_9         0x00004000U

#define I2C_CLOCKSPEED_100KHZ      100000U
#define I2C_CLOCKSPEED_400KHZ      400000U

#define I2C1_SCL_PIN_PB8           8U
#define I2C1_SDA_PIN_PB9           9U
#define I2C1_SCL_PIN_PB6           6U
#define I2C1_SDA_PIN_PB7           7U

#define I2C_OLED_CMD_BYTE          0x00U
#define I2C_OLED_DATA_BYTE         0x40U

DriverStatus i2c_init(I2C_TypeDef *I2Cx, const I2C_InitTypeDef *init);
DriverStatus i2c_gpio_init(I2C_TypeDef *I2Cx, GPIO_TypeDef *GPIOx,
                           uint8_t scl_pin, uint8_t sda_pin);
DriverStatus i2c_write(I2C_TypeDef *I2Cx, uint8_t device_address,
                       const uint8_t *data, uint16_t size,
                       uint32_t timeout_cycles);
DriverStatus i2c_read(I2C_TypeDef *I2Cx, uint8_t device_address,
                      uint8_t *data, uint16_t size,
                      uint32_t timeout_cycles);
DriverStatus i2c_is_ready(I2C_TypeDef *I2Cx, uint8_t device_address,
                          uint8_t trials, uint32_t timeout_cycles);
DriverStatus i2c_recover_bus(I2C_TypeDef *I2Cx);

#endif /* I2C_H */
