/**
 ******************************************************************************
 * @file           : bsp.h
 * @author         : Haoyi Chen
 * @date           : 2025-08-08
 * @brief          : Board Support Package header for Motor Monitor project
 ******************************************************************************
 * @details
 * This file serves as the central include point for required hardware modules
 * including RCC, GPIO, TIM, ADC and DMA functionalities.
 * Include this file in your application to access all BSP functionalities.
 *
 * This file is part of a bare-metal STM32F407VGT6 project.
 ******************************************************************************
 */

#ifndef BSP_H
#define BSP_H

#include <stm32f407xx.h>
#include "rcc.h"
#include "gpio.h"
#include "tim.h"
#include "adc.h"
#include "dma.h"
#include "irq.h"
#include "encoder.h"
#include "systick.h"
#include "event.h"
#include "button.h"
#include "uart.h"

/* Buttion pin definitions */
#define BUTTON_UP_PORT      GPIOE
#define BUTTON_UP_PIN       9
#define BUTTON_DOWN_PORT    GPIOE
#define BUTTON_DOWN_PIN     10
#define BUTTON_RETURN_PORT  GPIOE
#define BUTTON_RETURN_PIN   11
#define BUTTON_ENTER_PORT   GPIOE
#define BUTTON_ENTER_PIN    12

/* Motor control pin definitions */
#define MOTOR_P_PORT        GPIOB
#define MOTOR_P_PIN         0

#define MOTOR_M_PORT        GPIOB
#define MOTOR_M_PIN         1

#define MOTOR_ENABLE_PORT   GPIOE
#define MOTOR_ENABLE_PIN    7

/* Motor encoder pin definitions */
#define ENCODER_CH3_PORT    GPIOA
#define ENCODER_CH3_PIN     2

#define ENCODER_CH4_PORT    GPIOA
#define ENCODER_CH4_PIN     3

#define ENCODER_TIM         TIM2

/* Current sensing ADC pin definition */
#define CURRENT_ADC_PORT    GPIOA
#define CURRENT_ADC_PIN     0

/**
 * @brief UART pin definitions
 */
#define FPGA_UART_TX_PORT        GPIOD
#define FPGA_UART_TX_PIN         5
#define FPGA_UART_RX_PORT        GPIOD
#define FPGA_UART_RX_PIN         6

/* UART handle structure */



/* Current protection threshold */
#define CURRENT_CRITICAL_THRESHOLD    3400  // ADC value threshold, adjust based on system requirements

/* Global shared variables for ADC data handling */
extern volatile uint16_t current_adcBuffer[200];  // ADC sample buffer
extern uint16_t current_adcAverage;               // Calculated average value
extern volatile uint8_t current_adcAverageReady;  // Flag indicating new data is ready
extern uint32_t sum;

/**
 * @brief Initialize RCC (Reset and Clock Control)
 * 
 * Configures system clock to maximum frequency using external oscillator
 */
void rcc_init(void);

/**
 * @brief Initialize GPIO pins
 * 
 * Configures required GPIO pins (PB0, PB1, PB12 as outputs, PA0 for ADC)
 */
void gpio_system_init(void);

/**
 * @brief Initialize timer for ADC trigger
 * 
 * Configures TIM2 to generate periodic trigger for ADC sampling at 500kHz
 */
void timer_init(void);

/**
 * @brief Initialize ADC and DMA
 * 
 * Configures ADC for timer-triggered sampling with DMA on PA0
 */
void adc_dma_init(void);

/**
 * @brief Initialize UART interface
 * 
 * Configures UART1 for communication at 115200 baud, 8N1
 */
void uart_system_init(void);

/**
 * @brief Initialize all system components
 * 
 * This function initializes the complete system by calling
 * individual initialization functions in the proper sequence.
 * In the main application, only this function needs to be called.
 */
void system_init(void);

#endif //BSP_H
