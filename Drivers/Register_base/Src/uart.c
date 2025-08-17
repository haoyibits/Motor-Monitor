/**
 * @file uart.c
 * @author GitHub Copilot
 * @date 2025-08-15
 * @brief STM32F4 UART register-level driver implementation
 *
 * @details This file contains UART function implementations for STM32F407 series
 * microcontrollers using direct register access.
 */

#include "uart.h"
#include "stdio.h"
/**
 * @brief Configure GPIO pins for UART
 * 
 * @param pins Pointer to UART pin configuration structure
 * @return uint8_t 0 if successful, 1 if error
 */
static uint8_t uart_gpio_init(UART_PinConfig *pins)
{
    if (pins == NULL) {
        return 1;
    }
    
    /* Configure TX pin as alternate function */
    gpio_init(pins->tx_port, pins->tx_pin, GPIO_MODE_AF, GPIO_OTYPE_PP, 
              GPIO_SPEED_HIGH, GPIO_PULLUP);
    
    /* Configure RX pin as alternate function */
    gpio_init(pins->rx_port, pins->rx_pin, GPIO_MODE_AF, GPIO_OTYPE_PP, 
              GPIO_SPEED_HIGH, GPIO_PULLUP);
    
    /* Set alternate function for both pins */
    gpio_set_af(pins->tx_port, pins->tx_pin, pins->alt_func);
    gpio_set_af(pins->rx_port, pins->rx_pin, pins->alt_func);
    
    return 0;
}

/**
 * @brief Calculate UART BRR register value for given baud rate
 * 
 * @param pclk Peripheral clock frequency in Hz
 * @param baud Desired baud rate in bps
 * @return uint16_t BRR register value
 */
static uint16_t uart_calculate_brr(uint32_t pclk, uint32_t baud)
{
    uint16_t brr;
    
    /* Calculate BRR value based on baud rate formula */
    brr = (uint16_t)(pclk / baud);
    
    return brr;
}


/**
 * @brief Initialize UART peripheral
 * 
 * @param huart Pointer to UART handle structure
 * @param pins Pointer to UART pin configuration
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t uart_init(UART_HandleTypeDef *huart, UART_PinConfig *pins)
{
    uint32_t pclk;
    uint16_t brr_value;
    
    /* Validate input parameters */
    if (huart == NULL || pins == NULL || huart->Instance == NULL) {
        return 1;
    }
    
    /* Initialize GPIO pins for UART */
    if (uart_gpio_init(pins) != 0) {
        return 1;
    }
    
    /* Calculate peripheral clock frequency */
    if (huart->Instance == USART1 || huart->Instance == USART6) {
        pclk = rcc_get_pclk2_freq();
    } else {
        pclk = rcc_get_pclk1_freq();
    }
    
    /* Calculate BRR register value for desired baud rate */
    brr_value = uart_calculate_brr(pclk, huart->Init.BaudRate);
    
    /* Reset UART peripheral */
    huart->Instance->CR1 = 0;
    huart->Instance->CR2 = 0;
    huart->Instance->CR3 = 0;
    
    /* Configure UART parameters */
    huart->Instance->CR1 |= huart->Init.WordLength | huart->Init.Parity | huart->Init.Mode;
    huart->Instance->CR2 |= huart->Init.StopBits;
    huart->Instance->CR3 |= huart->Init.HardwareFlowControl;
    
    /* Set baud rate */
    huart->Instance->BRR = brr_value;
    
    /* Enable UART */
    huart->Instance->CR1 |= USART_CR1_UE;
    
    /* Initialize handle state variables */
    huart->TxBusy = 0;
    huart->RxBusy = 0;
    
    return 0;
}

/**
 * @brief Send data through UART (blocking mode)
 * 
 * @param huart Pointer to UART handle structure
 * @param data Pointer to data buffer
 * @param size Buffer size
 * @param timeout Timeout in milliseconds
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t uart_transmit(UART_HandleTypeDef *huart, uint8_t *data, uint16_t size, uint32_t timeout)
{
    uint32_t start_time = 0;
    uint32_t current_time = 0;
    
    /* Validate input parameters */
    if (huart == NULL || data == NULL || size == 0) {
        return 1;
    }
    
    /* Check if transmission is already ongoing */
    if (huart->TxBusy == 1) {
        return 1;
    }
    
    /* Set transmission state to busy */
    huart->TxBusy = 1;
    
    /* Get start time for timeout calculation */
    start_time = systick_get_ms();
    
    /* Transmit data byte by byte */
    for (uint16_t i = 0; i < size; i++) {
        /* Wait until TXE flag is set (transmit data register empty) */
        while (!(huart->Instance->SR & USART_SR_TXE)) {
            /* Check for timeout */
            current_time = systick_get_ms();
            if ((current_time - start_time) > timeout) {
                huart->TxBusy = 0;
                return 1;
            }
        }
        
        /* Transmit byte */
        huart->Instance->DR = (data[i] & 0xFF);
    }
    
    /* Wait until TC flag is set (transmission complete) */
    while (!(huart->Instance->SR & USART_SR_TC)) {
        /* Check for timeout */
        current_time = systick_get_ms();
        if ((current_time - start_time) > timeout) {
            huart->TxBusy = 0;
            return 1;
        }
    }
    
    /* Set transmission state to idle */
    huart->TxBusy = 0;
    
    return 0;
}

/**
 * @brief Receive data through UART (blocking mode)
 * 
 * @param huart Pointer to UART handle structure
 * @param data Pointer to data buffer
 * @param size Buffer size
 * @param timeout Timeout in milliseconds
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t uart_receive(UART_HandleTypeDef *huart, uint8_t *data, uint16_t size, uint32_t timeout)
{
    uint32_t start_time = 0;
    uint32_t current_time = 0;
    
    /* Validate input parameters */
    if (huart == NULL || data == NULL || size == 0) {
        return 1;
    }
    
    /* Check if reception is already ongoing */
    if (huart->RxBusy == 1) {
        return 1;
    }
    
    /* Set reception state to busy */
    huart->RxBusy = 1;
    
    /* Get start time for timeout calculation */
    start_time = systick_get_ms();
    
    /* Receive data byte by byte */
    for (uint16_t i = 0; i < size; i++) {
        /* Wait until RXNE flag is set (read data register not empty) */
        while (!(huart->Instance->SR & USART_SR_RXNE)) {
            /* Check for timeout */
            current_time = systick_get_ms();
            if ((current_time - start_time) > timeout) {
                huart->RxBusy = 0;
                return 1;
            }
        }
        
        /* Receive byte */
        data[i] = (uint8_t)(huart->Instance->DR & 0xFF);
    }
    
    /* Set reception state to idle */
    huart->RxBusy = 0;
    
    return 0;
}

/**
 * @brief Send single character through UART
 * 
 * @param huart Pointer to UART handle structure
 * @param data Character to send
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t uart_transmit_char(UART_HandleTypeDef *huart, uint8_t data)
{
    /* Validate input parameter */
    if (huart == NULL) {
        return 1;
    }
    
    /* Wait until TXE flag is set (transmit data register empty) */
    while (!(huart->Instance->SR & USART_SR_TXE));
    
    /* Transmit byte */
    huart->Instance->DR = (data & 0xFF);
    
    /* Wait until TC flag is set (transmission complete) */
    while (!(huart->Instance->SR & USART_SR_TC));
    
    return 0;
}

/**
 * @brief Receive single character through UART
 * 
 * @param huart Pointer to UART handle structure
 * @return int16_t Received character or -1 if error
 */
int16_t uart_receive_char(UART_HandleTypeDef *huart)
{
    /* Validate input parameter */
    if (huart == NULL) {
        return -1;
    }
    
    /* Check for errors */
    if (huart->Instance->SR & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE)) {
        /* Clear error flags by reading DR register */
        (void)huart->Instance->DR;
        return -1;
    }
    
    /* Wait until RXNE flag is set (read data register not empty) */
    if (huart->Instance->SR & USART_SR_RXNE) {
        /* Receive byte */
        return (int16_t)(huart->Instance->DR & 0xFF);
    }
    
    return -1;
}

/**
 * @brief Send string through UART
 * 
 * @param huart Pointer to UART handle structure
 * @param str Null-terminated string to send
 * @return uint8_t 0 if successful, 1 if error
 */
uint8_t uart_transmit_string(UART_HandleTypeDef *huart, const char *str)
{
    uint16_t i = 0;
    
    /* Validate input parameters */
    if (huart == NULL || str == NULL) {
        return 1;
    }
    
    /* Transmit string character by character */
    while (str[i] != '\0') {
        uart_transmit_char(huart, str[i++]);
    }
    
    return 0;
}

/**
 * @brief Check if UART flag is set
 * 
 * @param huart Pointer to UART handle structure
 * @param flag Flag to check
 * @return uint8_t 1 if flag is set, 0 otherwise
 */
uint8_t uart_get_flag_status(UART_HandleTypeDef *huart, uint16_t flag)
{
    /* Validate input parameter */
    if (huart == NULL) {
        return 0;
    }
    
    /* Check if flag is set */
    if (huart->Instance->SR & flag) {
        return 1;
    }
    
    return 0;
}

/**
 * @brief Enable UART interrupt
 * 
 * @param huart Pointer to UART handle structure
 * @param interrupt Interrupt to enable
 */
void uart_enable_interrupt(UART_HandleTypeDef *huart, uint16_t interrupt)
{
    /* Validate input parameter */
    if (huart == NULL) {
        return;
    }
    
    /* Enable specified interrupt */
    huart->Instance->CR1 |= interrupt;
}

/**
 * @brief Disable UART interrupt
 * 
 * @param huart Pointer to UART handle structure
 * @param interrupt Interrupt to disable
 */
void uart_disable_interrupt(UART_HandleTypeDef *huart, uint16_t interrupt)
{
    /* Validate input parameter */
    if (huart == NULL) {
        return;
    }
    
    /* Disable specified interrupt */
    huart->Instance->CR1 &= ~interrupt;
}



