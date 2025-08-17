/**
 * @file uart.h
 * @author GitHub Copilot
 * @date 2025-08-15
 * @brief STM32F4 UART register-level driver header
 *
 * @details This file contains UART function declarations and constant definitions
 * for STM32F407 series microcontrollers.
 */

#ifndef UART_H
#define UART_H

#include "stm32f407xx.h"
#include "gpio.h"
#include "rcc.h"

/**
 * @name UART Word Length Definitions
 * @{
 */
#define UART_WORDLENGTH_8B      0x0000  /**< 8 data bits */
#define UART_WORDLENGTH_9B      0x1000  /**< 9 data bits */
/** @} */

/**
 * @name UART Stop Bits Definitions
 * @{
 */
#define UART_STOPBITS_1         0x0000  /**< 1 stop bit */
#define UART_STOPBITS_0_5       0x1000  /**< 0.5 stop bit */
#define UART_STOPBITS_2         0x2000  /**< 2 stop bits */
#define UART_STOPBITS_1_5       0x3000  /**< 1.5 stop bits */
/** @} */

/**
 * @name UART Parity Definitions
 * @{
 */
#define UART_PARITY_NONE        0x0000  /**< No parity */
#define UART_PARITY_EVEN        0x0400  /**< Even parity */
#define UART_PARITY_ODD         0x0600  /**< Odd parity */
/** @} */

/**
 * @name UART Mode Definitions
 * @{
 */
#define UART_MODE_TX            0x0008  /**< Transmitter enable */
#define UART_MODE_RX            0x0004  /**< Receiver enable */
#define UART_MODE_TX_RX         0x000C  /**< Transmitter and Receiver enable */
/** @} */

/**
 * @name UART Hardware Flow Control Definitions
 * @{
 */
#define UART_HWCONTROL_NONE     0x0000  /**< No hardware flow control */
#define UART_HWCONTROL_RTS      0x0100  /**< RTS hardware flow control */
#define UART_HWCONTROL_CTS      0x0200  /**< CTS hardware flow control */
#define UART_HWCONTROL_RTS_CTS  0x0300  /**< RTS and CTS hardware flow control */
/** @} */

/**
 * @name UART Status Flags
 * @{
 */
#define UART_FLAG_TXE           0x0080  /**< Transmit data register empty flag */
#define UART_FLAG_TC            0x0040  /**< Transmission complete flag */
#define UART_FLAG_RXNE          0x0020  /**< Read data register not empty flag */
#define UART_FLAG_IDLE          0x0010  /**< Idle line detected flag */
#define UART_FLAG_ORE           0x0008  /**< Overrun error flag */
#define UART_FLAG_NE            0x0004  /**< Noise error flag */
#define UART_FLAG_FE            0x0002  /**< Framing error flag */
#define UART_FLAG_PE            0x0001  /**< Parity error flag */
/** @} */

/**
 * @name UART Interrupt Flags
 * @{
 */
#define UART_IT_TXE             0x0080  /**< Transmit data register empty interrupt */
#define UART_IT_TC              0x0040  /**< Transmission complete interrupt */
#define UART_IT_RXNE            0x0020  /**< Read data register not empty interrupt */
#define UART_IT_IDLE            0x0010  /**< Idle line detected interrupt */
#define UART_IT_ORE             0x0008  /**< Overrun error interrupt */
#define UART_IT_NE              0x0004  /**< Noise error interrupt */
#define UART_IT_FE              0x0002  /**< Framing error interrupt */
#define UART_IT_PE              0x0001  /**< Parity error interrupt */
/** @} */

/**
 * @brief UART initialization structure
 */
typedef struct {
    uint32_t BaudRate;          /**< Baud rate */
    uint16_t WordLength;        /**< Word length (8 or 9 bits) */
    uint16_t StopBits;          /**< Stop bits */
    uint16_t Parity;            /**< Parity mode */
    uint16_t Mode;              /**< UART mode (RX/TX) */
    uint16_t HardwareFlowControl; /**< Hardware flow control mode */
} UART_InitTypeDef;

/**
 * @brief UART handle structure
 */
typedef struct {
    USART_TypeDef *Instance;    /**< UART registers base address */
    UART_InitTypeDef Init;      /**< UART communication parameters */
    uint8_t *pTxBuffer;         /**< Pointer to TX buffer */
    uint16_t TxSize;            /**< TX buffer size */
    uint16_t TxCount;           /**< TX counter */
    uint8_t *pRxBuffer;         /**< Pointer to RX buffer */
    uint16_t RxSize;            /**< RX buffer size */
    uint16_t RxCount;           /**< RX counter */
    uint8_t TxBusy;             /**< Transmission ongoing flag */
    uint8_t RxBusy;             /**< Reception ongoing flag */
} UART_HandleTypeDef;

/**
 * @brief GPIO configuration structure for UART pins
 */
typedef struct {
    GPIO_TypeDef *tx_port;      /**< TX pin GPIO port */
    uint8_t tx_pin;             /**< TX pin number */
    GPIO_TypeDef *rx_port;      /**< RX pin GPIO port */
    uint8_t rx_pin;             /**< RX pin number */
    uint8_t alt_func;           /**< Alternate function value for UART */
} UART_PinConfig;

/**
 * @brief Initialize UART peripheral
 * 
 * @param huart Pointer to UART handle structure
 * @param pins Pointer to UART pin configuration
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t uart_init(UART_HandleTypeDef *huart, UART_PinConfig *pins);

/**
 * @brief Send data through UART (blocking mode)
 * 
 * @param huart Pointer to UART handle structure
 * @param data Pointer to data buffer
 * @param size Buffer size
 * @param timeout Timeout in milliseconds
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t uart_transmit(UART_HandleTypeDef *huart, uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief Receive data through UART (blocking mode)
 * 
 * @param huart Pointer to UART handle structure
 * @param data Pointer to data buffer
 * @param size Buffer size
 * @param timeout Timeout in milliseconds
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t uart_receive(UART_HandleTypeDef *huart, uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief Send single character through UART
 * 
 * @param huart Pointer to UART handle structure
 * @param data Character to send
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t uart_transmit_char(UART_HandleTypeDef *huart, uint8_t data);

/**
 * @brief Receive single character through UART
 * 
 * @param huart Pointer to UART handle structure
 * @return int16_t Received character or -1 if error
 */
int16_t uart_receive_char(UART_HandleTypeDef *huart);

/**
 * @brief Send string through UART
 * 
 * @param huart Pointer to UART handle structure
 * @param str Null-terminated string to send
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t uart_transmit_string(UART_HandleTypeDef *huart, const char *str);

/**
 * @brief Check if UART flag is set
 * 
 * @param huart Pointer to UART handle structure
 * @param flag Flag to check
 * @return uint8_t 1 if flag is set, 0 otherwise
 */
uint8_t uart_get_flag_status(UART_HandleTypeDef *huart, uint16_t flag);

/**
 * @brief Enable UART interrupt
 * 
 * @param huart Pointer to UART handle structure
 * @param interrupt Interrupt to enable
 */
void uart_enable_interrupt(UART_HandleTypeDef *huart, uint16_t interrupt);

/**
 * @brief Disable UART interrupt
 * 
 * @param huart Pointer to UART handle structure
 * @param interrupt Interrupt to disable
 */
void uart_disable_interrupt(UART_HandleTypeDef *huart, uint16_t interrupt);

#endif /* UART_H */
