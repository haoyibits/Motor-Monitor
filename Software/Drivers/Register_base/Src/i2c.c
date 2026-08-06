/**
 * @file i2c.c
 * @brief STM32F4 register-level I2C master driver implementation.
 */

#include <stddef.h>

#include "gpio.h"
#include "i2c.h"
#include "rcc.h"

#define I2C_ERROR_FLAGS (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | \
                         I2C_SR1_OVR | I2C_SR1_PECERR | I2C_SR1_TIMEOUT | \
                         I2C_SR1_SMBALERT)

typedef struct {
    GPIO_TypeDef *gpio;
    uint8_t scl_pin;
    uint8_t sda_pin;
    uint8_t alternate_function;
    I2C_InitTypeDef init;
    uint8_t pins_configured;
    uint8_t peripheral_configured;
} I2C_Context;

static I2C_Context i2c_contexts[3];

static int8_t i2c_instance_index(I2C_TypeDef *i2c)
{
    if (i2c == I2C1) return 0;
    if (i2c == I2C2) return 1;
    if (i2c == I2C3) return 2;
    return -1;
}

static uint8_t i2c_instance_valid(I2C_TypeDef *i2c)
{
    return (i2c_instance_index(i2c) >= 0) ? 1U : 0U;
}

static void i2c_recovery_delay(void)
{
    for (volatile uint32_t delay = 512U; delay > 0U; --delay) {
        __NOP();
    }
}

static void i2c_clear_addr(I2C_TypeDef *i2c)
{
    volatile uint32_t clear = i2c->SR1;
    clear = i2c->SR2;
    (void)clear;
}

static void i2c_clear_errors(I2C_TypeDef *i2c)
{
    i2c->SR1 &= ~I2C_ERROR_FLAGS;
}

static void i2c_abort(I2C_TypeDef *i2c)
{
    if ((i2c->SR2 & I2C_SR2_MSL) != 0U) {
        i2c->CR1 |= I2C_CR1_STOP;
    }
    i2c->CR1 &= ~I2C_CR1_POS;
    i2c->CR1 |= I2C_CR1_ACK;
    i2c_clear_errors(i2c);
}

static DriverStatus i2c_wait_sr1(I2C_TypeDef *i2c, uint32_t flags,
                                 uint32_t timeout)
{
    while (timeout > 0U) {
        uint32_t sr1 = i2c->SR1;
        if ((sr1 & I2C_ERROR_FLAGS) != 0U) {
            return DRIVER_STATUS_IO_ERROR;
        }
        if ((sr1 & flags) == flags) {
            return DRIVER_STATUS_OK;
        }
        --timeout;
    }
    return DRIVER_STATUS_TIMEOUT;
}

static DriverStatus i2c_wait_bus_idle(I2C_TypeDef *i2c, uint32_t timeout)
{
    while (timeout > 0U) {
        if ((i2c->SR2 & I2C_SR2_BUSY) == 0U) {
            return DRIVER_STATUS_OK;
        }
        --timeout;
    }
    return DRIVER_STATUS_BUSY;
}

static DriverStatus i2c_start_and_address(I2C_TypeDef *i2c,
                                          uint8_t device_address,
                                          uint8_t read,
                                          uint32_t timeout)
{
    i2c_clear_errors(i2c);
    i2c->CR1 |= I2C_CR1_START;
    DriverStatus status = i2c_wait_sr1(i2c, I2C_SR1_SB, timeout);
    if (status != DRIVER_STATUS_OK) {
        return status;
    }

    i2c->DR = ((uint32_t)device_address << 1) | (read ? 1U : 0U);
    return i2c_wait_sr1(i2c, I2C_SR1_ADDR, timeout);
}

DriverStatus i2c_init(I2C_TypeDef *i2c, const I2C_InitTypeDef *init)
{
    if ((!i2c_instance_valid(i2c)) || (init == NULL) ||
        (init->ClockSpeed == 0U) ||
        (init->ClockSpeed > I2C_CLOCKSPEED_400KHZ)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    uint32_t pclk1 = rcc_get_pclk1_freq();
    uint32_t freq_mhz = pclk1 / 1000000U;
    if ((freq_mhz < 2U) || (freq_mhz > 50U)) {
        return DRIVER_STATUS_OUT_OF_RANGE;
    }

    i2c->CR1 &= ~I2C_CR1_PE;
    i2c->CR1 = I2C_CR1_SWRST;
    i2c->CR1 = 0U;
    i2c->CR2 = freq_mhz & I2C_CR2_FREQ;
    i2c->CCR = 0U;

    if (init->ClockSpeed <= I2C_CLOCKSPEED_100KHZ) {
        uint32_t ccr = pclk1 / (init->ClockSpeed * 2U);
        if (ccr < 4U) {
            ccr = 4U;
        }
        i2c->CCR = ccr;
        i2c->TRISE = freq_mhz + 1U;
    } else {
        uint32_t divisor = (init->DutyCycle == I2C_DUTYCYCLE_16_9) ? 25U : 3U;
        uint32_t ccr = pclk1 / (init->ClockSpeed * divisor);
        if (ccr == 0U) {
            ccr = 1U;
        }
        i2c->CCR = I2C_CCR_FS | init->DutyCycle | ccr;
        i2c->TRISE = ((freq_mhz * 300U) / 1000U) + 1U;
    }

    i2c->OAR1 = (1U << 14);
    i2c->CR1 = I2C_CR1_PE | I2C_CR1_ACK;

    I2C_Context *context = &i2c_contexts[i2c_instance_index(i2c)];
    context->init = *init;
    context->peripheral_configured = 1U;
    return DRIVER_STATUS_OK;
}

DriverStatus i2c_gpio_init(I2C_TypeDef *i2c, GPIO_TypeDef *gpio,
                           uint8_t scl_pin, uint8_t sda_pin)
{
    if ((!i2c_instance_valid(i2c)) || (gpio == NULL) ||
        (scl_pin >= 16U) || (sda_pin >= 16U)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    uint8_t alternate_function = (i2c == I2C3) ? 9U : 4U;
    gpio_init(gpio, scl_pin, GPIO_MODE_AF, GPIO_OTYPE_OD,
              GPIO_SPEED_HIGH, GPIO_PULLUP);
    gpio_set_af(gpio, scl_pin, alternate_function);
    gpio_init(gpio, sda_pin, GPIO_MODE_AF, GPIO_OTYPE_OD,
              GPIO_SPEED_HIGH, GPIO_PULLUP);
    gpio_set_af(gpio, sda_pin, alternate_function);

    I2C_Context *context = &i2c_contexts[i2c_instance_index(i2c)];
    context->gpio = gpio;
    context->scl_pin = scl_pin;
    context->sda_pin = sda_pin;
    context->alternate_function = alternate_function;
    context->pins_configured = 1U;
    return DRIVER_STATUS_OK;
}

DriverStatus i2c_recover_bus(I2C_TypeDef *i2c)
{
    int8_t index = i2c_instance_index(i2c);
    if (index < 0) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    I2C_Context *context = &i2c_contexts[index];
    if ((context->pins_configured == 0U) ||
        (context->peripheral_configured == 0U)) {
        return DRIVER_STATUS_NOT_READY;
    }

    i2c->CR1 &= ~I2C_CR1_PE;
    gpio_init(context->gpio, context->scl_pin, GPIO_MODE_OUTPUT,
              GPIO_OTYPE_OD, GPIO_SPEED_HIGH, GPIO_PULLUP);
    gpio_init(context->gpio, context->sda_pin, GPIO_MODE_OUTPUT,
              GPIO_OTYPE_OD, GPIO_SPEED_HIGH, GPIO_PULLUP);
    gpio_write(context->gpio, context->sda_pin, 1U);
    gpio_write(context->gpio, context->scl_pin, 1U);

    for (uint8_t pulse = 0U; pulse < 9U; ++pulse) {
        gpio_write(context->gpio, context->scl_pin, 0U);
        i2c_recovery_delay();
        gpio_write(context->gpio, context->scl_pin, 1U);
        i2c_recovery_delay();
    }

    gpio_write(context->gpio, context->sda_pin, 0U);
    i2c_recovery_delay();
    gpio_write(context->gpio, context->scl_pin, 1U);
    i2c_recovery_delay();
    gpio_write(context->gpio, context->sda_pin, 1U);
    i2c_recovery_delay();

    gpio_init(context->gpio, context->scl_pin, GPIO_MODE_AF, GPIO_OTYPE_OD,
              GPIO_SPEED_HIGH, GPIO_PULLUP);
    gpio_set_af(context->gpio, context->scl_pin,
                context->alternate_function);
    gpio_init(context->gpio, context->sda_pin, GPIO_MODE_AF, GPIO_OTYPE_OD,
              GPIO_SPEED_HIGH, GPIO_PULLUP);
    gpio_set_af(context->gpio, context->sda_pin,
                context->alternate_function);

    I2C_InitTypeDef saved_init = context->init;
    return i2c_init(i2c, &saved_init);
}

DriverStatus i2c_write(I2C_TypeDef *i2c, uint8_t device_address,
                       const uint8_t *data, uint16_t size,
                       uint32_t timeout)
{
    if ((!i2c_instance_valid(i2c)) || (data == NULL) || (size == 0U) ||
        (device_address > 0x7FU) || (timeout == 0U)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    DriverStatus status = i2c_wait_bus_idle(i2c, timeout);
    if (status == DRIVER_STATUS_BUSY) {
        status = i2c_recover_bus(i2c);
        if (status == DRIVER_STATUS_OK) {
            status = i2c_wait_bus_idle(i2c, timeout);
        }
    }
    if (status == DRIVER_STATUS_OK) {
        status = i2c_start_and_address(i2c, device_address, 0U, timeout);
    }
    if (status != DRIVER_STATUS_OK) {
        i2c_abort(i2c);
        return status;
    }

    i2c_clear_addr(i2c);
    for (uint16_t i = 0U; i < size; ++i) {
        status = i2c_wait_sr1(i2c, I2C_SR1_TXE, timeout);
        if (status != DRIVER_STATUS_OK) {
            i2c_abort(i2c);
            return status;
        }
        i2c->DR = data[i];
    }

    status = i2c_wait_sr1(i2c, I2C_SR1_BTF, timeout);
    if (status != DRIVER_STATUS_OK) {
        i2c_abort(i2c);
        return status;
    }
    i2c->CR1 |= I2C_CR1_STOP;
    return DRIVER_STATUS_OK;
}

DriverStatus i2c_read(I2C_TypeDef *i2c, uint8_t device_address,
                      uint8_t *data, uint16_t size, uint32_t timeout)
{
    if ((!i2c_instance_valid(i2c)) || (data == NULL) || (size == 0U) ||
        (device_address > 0x7FU) || (timeout == 0U)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    DriverStatus status = i2c_wait_bus_idle(i2c, timeout);
    if (status == DRIVER_STATUS_BUSY) {
        status = i2c_recover_bus(i2c);
        if (status == DRIVER_STATUS_OK) {
            status = i2c_wait_bus_idle(i2c, timeout);
        }
    }
    if (status == DRIVER_STATUS_OK) {
        status = i2c_start_and_address(i2c, device_address, 1U, timeout);
    }
    if (status != DRIVER_STATUS_OK) {
        i2c_abort(i2c);
        return status;
    }

    if (size == 1U) {
        i2c->CR1 &= ~I2C_CR1_ACK;
        i2c_clear_addr(i2c);
        i2c->CR1 |= I2C_CR1_STOP;
        status = i2c_wait_sr1(i2c, I2C_SR1_RXNE, timeout);
        if (status == DRIVER_STATUS_OK) {
            data[0] = (uint8_t)i2c->DR;
        }
    } else if (size == 2U) {
        i2c->CR1 |= I2C_CR1_POS;
        i2c->CR1 &= ~I2C_CR1_ACK;
        i2c_clear_addr(i2c);
        status = i2c_wait_sr1(i2c, I2C_SR1_BTF, timeout);
        if (status == DRIVER_STATUS_OK) {
            i2c->CR1 |= I2C_CR1_STOP;
            data[0] = (uint8_t)i2c->DR;
            data[1] = (uint8_t)i2c->DR;
        }
    } else {
        uint16_t index = 0U;
        uint16_t remaining = size;
        i2c->CR1 |= I2C_CR1_ACK;
        i2c_clear_addr(i2c);

        while ((remaining > 3U) && (status == DRIVER_STATUS_OK)) {
            status = i2c_wait_sr1(i2c, I2C_SR1_RXNE, timeout);
            if (status == DRIVER_STATUS_OK) {
                data[index++] = (uint8_t)i2c->DR;
                --remaining;
            }
        }

        if (status == DRIVER_STATUS_OK) {
            status = i2c_wait_sr1(i2c, I2C_SR1_BTF, timeout);
        }
        if (status == DRIVER_STATUS_OK) {
            i2c->CR1 &= ~I2C_CR1_ACK;
            data[index++] = (uint8_t)i2c->DR;
            --remaining;
            status = i2c_wait_sr1(i2c, I2C_SR1_BTF, timeout);
        }
        if (status == DRIVER_STATUS_OK) {
            i2c->CR1 |= I2C_CR1_STOP;
            data[index++] = (uint8_t)i2c->DR;
            --remaining;
            data[index] = (uint8_t)i2c->DR;
            --remaining;
        }
        (void)remaining;
    }

    i2c->CR1 &= ~I2C_CR1_POS;
    i2c->CR1 |= I2C_CR1_ACK;
    if (status != DRIVER_STATUS_OK) {
        i2c_abort(i2c);
    }
    return status;
}

DriverStatus i2c_is_ready(I2C_TypeDef *i2c, uint8_t device_address,
                          uint8_t trials, uint32_t timeout)
{
    if ((!i2c_instance_valid(i2c)) || (device_address > 0x7FU) ||
        (trials == 0U) || (timeout == 0U)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    while (trials > 0U) {
        DriverStatus status = i2c_wait_bus_idle(i2c, timeout);
        if (status == DRIVER_STATUS_BUSY) {
            status = i2c_recover_bus(i2c);
            if (status == DRIVER_STATUS_OK) {
                status = i2c_wait_bus_idle(i2c, timeout);
            }
        }
        if (status == DRIVER_STATUS_OK) {
            status = i2c_start_and_address(i2c, device_address, 0U, timeout);
        }
        if (status == DRIVER_STATUS_OK) {
            i2c_clear_addr(i2c);
            i2c->CR1 |= I2C_CR1_STOP;
            return DRIVER_STATUS_OK;
        }
        i2c_abort(i2c);
        --trials;
    }
    return DRIVER_STATUS_NOT_READY;
}
