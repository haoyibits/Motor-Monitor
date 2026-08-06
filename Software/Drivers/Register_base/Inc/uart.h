/**
 * @file uart.h
 * @brief STM32F4 register-level UART driver.
 */

#ifndef UART_H
#define UART_H

#include "driver_status.h"
#include "gpio.h"
#include "rcc.h"

#define UART_WORDLENGTH_8B      0x0000U
#define UART_WORDLENGTH_9B      USART_CR1_M
#define UART_STOPBITS_1         0x0000U
#define UART_STOPBITS_0_5       USART_CR2_STOP_0
#define UART_STOPBITS_2         USART_CR2_STOP_1
#define UART_STOPBITS_1_5       USART_CR2_STOP
#define UART_PARITY_NONE        0x0000U
#define UART_PARITY_EVEN        USART_CR1_PCE
#define UART_PARITY_ODD         (USART_CR1_PCE | USART_CR1_PS)
#define UART_MODE_TX            USART_CR1_TE
#define UART_MODE_RX            USART_CR1_RE
#define UART_MODE_TX_RX         (USART_CR1_TE | USART_CR1_RE)
#define UART_HWCONTROL_NONE     0x0000U
#define UART_HWCONTROL_RTS      USART_CR3_RTSE
#define UART_HWCONTROL_CTS      USART_CR3_CTSE
#define UART_HWCONTROL_RTS_CTS  (USART_CR3_RTSE | USART_CR3_CTSE)

#define UART_FLAG_TXE           USART_SR_TXE
#define UART_FLAG_TC            USART_SR_TC
#define UART_FLAG_RXNE          USART_SR_RXNE
#define UART_FLAG_IDLE          USART_SR_IDLE
#define UART_FLAG_ORE           USART_SR_ORE
#define UART_FLAG_NE            USART_SR_NE
#define UART_FLAG_FE            USART_SR_FE
#define UART_FLAG_PE            USART_SR_PE

#define UART_IT_TXE             USART_CR1_TXEIE
#define UART_IT_TC              USART_CR1_TCIE
#define UART_IT_RXNE            USART_CR1_RXNEIE
#define UART_IT_IDLE            USART_CR1_IDLEIE
#define UART_IT_PE              USART_CR1_PEIE
#define UART_IT_ERROR           0x80000000UL

#define UART_FALLBACK_SPIN_LIMIT 1000000UL

typedef struct {
    uint32_t BaudRate;
    uint16_t WordLength;
    uint16_t StopBits;
    uint16_t Parity;
    uint16_t Mode;
    uint16_t HardwareFlowControl;
} UART_InitTypeDef;

typedef struct {
    USART_TypeDef *Instance;
    UART_InitTypeDef Init;
    uint8_t *pTxBuffer;
    uint16_t TxSize;
    uint16_t TxCount;
    uint8_t *pRxBuffer;
    uint16_t RxSize;
    uint16_t RxCount;
    uint8_t TxBusy;
    uint8_t RxBusy;
} UART_HandleTypeDef;

typedef struct {
    GPIO_TypeDef *tx_port;
    uint8_t tx_pin;
    GPIO_TypeDef *rx_port;
    uint8_t rx_pin;
    uint8_t alt_func;
} UART_PinConfig;

DriverStatus uart_init(UART_HandleTypeDef *huart,
                       const UART_PinConfig *pins);
DriverStatus uart_transmit(UART_HandleTypeDef *huart, const uint8_t *data,
                           uint16_t size, uint32_t timeout_ms);
DriverStatus uart_receive(UART_HandleTypeDef *huart, uint8_t *data,
                          uint16_t size, uint32_t timeout_ms);
DriverStatus uart_transmit_char(UART_HandleTypeDef *huart, uint8_t data,
                                uint32_t timeout_ms);
DriverStatus uart_receive_char(UART_HandleTypeDef *huart, uint8_t *data,
                               uint32_t timeout_ms);
DriverStatus uart_transmit_string(UART_HandleTypeDef *huart, const char *str,
                                  uint32_t timeout_ms);
uint8_t uart_get_flag_status(const UART_HandleTypeDef *huart, uint16_t flag);
DriverStatus uart_enable_interrupt(UART_HandleTypeDef *huart,
                                   uint32_t interrupts);
DriverStatus uart_disable_interrupt(UART_HandleTypeDef *huart,
                                    uint32_t interrupts);

#endif /* UART_H */
