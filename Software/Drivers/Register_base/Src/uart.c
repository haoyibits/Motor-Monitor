/**
 * @file uart.c
 * @brief STM32F4 register-level UART driver implementation.
 */

#include <stddef.h>

#include "systick.h"
#include "uart.h"

#define UART_ERROR_FLAGS (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE)
#define UART_CR1_INTERRUPT_MASK (USART_CR1_TXEIE | USART_CR1_TCIE | \
                                 USART_CR1_RXNEIE | USART_CR1_IDLEIE | \
                                 USART_CR1_PEIE)

static uint8_t uart_instance_valid(USART_TypeDef *instance)
{
    return ((instance == USART1) || (instance == USART2) ||
            (instance == USART3) || (instance == UART4) ||
            (instance == UART5) || (instance == USART6)) ? 1U : 0U;
}

static DriverStatus uart_gpio_init(const UART_PinConfig *pins)
{
    if ((pins == NULL) || (pins->tx_port == NULL) ||
        (pins->rx_port == NULL) || (pins->tx_pin >= 16U) ||
        (pins->rx_pin >= 16U) || (pins->alt_func > 15U)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    gpio_init(pins->tx_port, pins->tx_pin, GPIO_MODE_AF, GPIO_OTYPE_PP,
              GPIO_SPEED_HIGH, GPIO_PULLUP);
    gpio_init(pins->rx_port, pins->rx_pin, GPIO_MODE_AF, GPIO_OTYPE_PP,
              GPIO_SPEED_HIGH, GPIO_PULLUP);
    gpio_set_af(pins->tx_port, pins->tx_pin, pins->alt_func);
    gpio_set_af(pins->rx_port, pins->rx_pin, pins->alt_func);
    return DRIVER_STATUS_OK;
}

static void uart_clear_errors(USART_TypeDef *instance)
{
    volatile uint32_t clear = instance->SR;
    clear = instance->DR;
    (void)clear;
}

static DriverStatus uart_wait_flag(UART_HandleTypeDef *huart, uint32_t flag,
                                   uint32_t timeout_ms)
{
    uint32_t start = systick_get_ms();
    uint32_t spins = UART_FALLBACK_SPIN_LIMIT;
    while (spins > 0U) {
        uint32_t status = huart->Instance->SR;
        if ((status & UART_ERROR_FLAGS) != 0U) {
            uart_clear_errors(huart->Instance);
            return DRIVER_STATUS_IO_ERROR;
        }
        if ((status & flag) != 0U) {
            return DRIVER_STATUS_OK;
        }
        if ((systick_get_ms() - start) >= timeout_ms) {
            return DRIVER_STATUS_TIMEOUT;
        }
        --spins;
    }
    return DRIVER_STATUS_TIMEOUT;
}

DriverStatus uart_init(UART_HandleTypeDef *huart, const UART_PinConfig *pins)
{
    if ((huart == NULL) || (!uart_instance_valid(huart->Instance)) ||
        (huart->Init.BaudRate == 0U) ||
        ((huart->Init.WordLength != UART_WORDLENGTH_8B) &&
         (huart->Init.WordLength != UART_WORDLENGTH_9B)) ||
        ((huart->Init.StopBits != UART_STOPBITS_1) &&
         (huart->Init.StopBits != UART_STOPBITS_0_5) &&
         (huart->Init.StopBits != UART_STOPBITS_2) &&
         (huart->Init.StopBits != UART_STOPBITS_1_5)) ||
        ((huart->Init.Parity != UART_PARITY_NONE) &&
         (huart->Init.Parity != UART_PARITY_EVEN) &&
         (huart->Init.Parity != UART_PARITY_ODD)) ||
        ((huart->Init.Mode != UART_MODE_TX) &&
         (huart->Init.Mode != UART_MODE_RX) &&
         (huart->Init.Mode != UART_MODE_TX_RX)) ||
        (huart->Init.HardwareFlowControl != UART_HWCONTROL_NONE)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    DriverStatus status = uart_gpio_init(pins);
    if (status != DRIVER_STATUS_OK) {
        return status;
    }

    uint32_t pclk = ((huart->Instance == USART1) ||
                     (huart->Instance == USART6)) ?
                    rcc_get_pclk2_freq() : rcc_get_pclk1_freq();
    uint32_t brr = (pclk + (huart->Init.BaudRate / 2U)) /
                   huart->Init.BaudRate;
    if ((brr == 0U) || (brr > 0xFFFFU)) {
        return DRIVER_STATUS_OUT_OF_RANGE;
    }

    huart->Instance->CR1 = 0U;
    huart->Instance->CR2 = huart->Init.StopBits;
    huart->Instance->CR3 = huart->Init.HardwareFlowControl;
    huart->Instance->BRR = brr;
    huart->Instance->CR1 = huart->Init.WordLength | huart->Init.Parity |
                           huart->Init.Mode | USART_CR1_UE;
    huart->TxBusy = 0U;
    huart->RxBusy = 0U;
    return DRIVER_STATUS_OK;
}

DriverStatus uart_transmit(UART_HandleTypeDef *huart, const uint8_t *data,
                           uint16_t size, uint32_t timeout_ms)
{
    if ((huart == NULL) || (!uart_instance_valid(huart->Instance)) ||
        (data == NULL) || (size == 0U) || (timeout_ms == 0U)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    if (huart->TxBusy != 0U) {
        return DRIVER_STATUS_BUSY;
    }

    huart->TxBusy = 1U;
    DriverStatus status = DRIVER_STATUS_OK;
    for (uint16_t i = 0U; (i < size) && (status == DRIVER_STATUS_OK); ++i) {
        status = uart_wait_flag(huart, USART_SR_TXE, timeout_ms);
        if (status == DRIVER_STATUS_OK) {
            huart->Instance->DR = data[i];
        }
    }
    if (status == DRIVER_STATUS_OK) {
        status = uart_wait_flag(huart, USART_SR_TC, timeout_ms);
    }
    huart->TxBusy = 0U;
    return status;
}

DriverStatus uart_receive(UART_HandleTypeDef *huart, uint8_t *data,
                          uint16_t size, uint32_t timeout_ms)
{
    if ((huart == NULL) || (!uart_instance_valid(huart->Instance)) ||
        (data == NULL) || (size == 0U) || (timeout_ms == 0U)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    if (huart->RxBusy != 0U) {
        return DRIVER_STATUS_BUSY;
    }

    huart->RxBusy = 1U;
    DriverStatus status = DRIVER_STATUS_OK;
    for (uint16_t i = 0U; (i < size) && (status == DRIVER_STATUS_OK); ++i) {
        status = uart_wait_flag(huart, USART_SR_RXNE, timeout_ms);
        if (status == DRIVER_STATUS_OK) {
            data[i] = (uint8_t)huart->Instance->DR;
        }
    }
    huart->RxBusy = 0U;
    return status;
}

DriverStatus uart_transmit_char(UART_HandleTypeDef *huart, uint8_t data,
                                uint32_t timeout_ms)
{
    return uart_transmit(huart, &data, 1U, timeout_ms);
}

DriverStatus uart_receive_char(UART_HandleTypeDef *huart, uint8_t *data,
                               uint32_t timeout_ms)
{
    return uart_receive(huart, data, 1U, timeout_ms);
}

DriverStatus uart_transmit_string(UART_HandleTypeDef *huart, const char *str,
                                  uint32_t timeout_ms)
{
    if (str == NULL) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    uint32_t length = 0U;
    while ((str[length] != '\0') && (length < 0xFFFFU)) {
        ++length;
    }
    if ((length == 0U) || (length == 0xFFFFU)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    return uart_transmit(huart, (const uint8_t *)str,
                         (uint16_t)length, timeout_ms);
}

uint8_t uart_get_flag_status(const UART_HandleTypeDef *huart, uint16_t flag)
{
    if ((huart == NULL) || (!uart_instance_valid(huart->Instance))) {
        return 0U;
    }
    return ((huart->Instance->SR & flag) != 0U) ? 1U : 0U;
}

DriverStatus uart_enable_interrupt(UART_HandleTypeDef *huart,
                                   uint32_t interrupts)
{
    if ((huart == NULL) || (!uart_instance_valid(huart->Instance))) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    huart->Instance->CR1 |= interrupts & UART_CR1_INTERRUPT_MASK;
    if ((interrupts & UART_IT_ERROR) != 0U) {
        huart->Instance->CR3 |= USART_CR3_EIE;
    }
    return DRIVER_STATUS_OK;
}

DriverStatus uart_disable_interrupt(UART_HandleTypeDef *huart,
                                    uint32_t interrupts)
{
    if ((huart == NULL) || (!uart_instance_valid(huart->Instance))) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    huart->Instance->CR1 &= ~(interrupts & UART_CR1_INTERRUPT_MASK);
    if ((interrupts & UART_IT_ERROR) != 0U) {
        huart->Instance->CR3 &= ~USART_CR3_EIE;
    }
    return DRIVER_STATUS_OK;
}
