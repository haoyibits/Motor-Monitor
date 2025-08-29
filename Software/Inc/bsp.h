/**
 ******************************************************************************
 * @file           : bsp.h
 * @author         : Haoyi Chen
 * @date           : 2025-08-08
 * @brief          : Board Support Package header for Motor Monitor project
 ******************************************************************************
 * @details
 * This file serves as the central include point for required hardware modules
 * Include this file in your application to access all BSP functionalities.
 ******************************************************************************
 */

#ifndef BSP_H
#define BSP_H


/* * Include standard libraries */
#include <stm32f407xx.h>
#include "stdio.h"

/* General peripherals headers */
#include "rcc.h"
#include "gpio.h"
#include "tim.h"
#include "adc.h"
#include "dma.h"
#include "irq.h"
#include "encoder.h"
#include "systick.h"
#include "event.h"
#include "motor.h"
#include "button.h"
#include "uart.h"
#include "i2c.h"
#include "spi.h"

/* Specific hardware headers */
#include "ssd1309.h"
#include "w25q128.h"

/* OLED UI Library */
#include "../Drivers/OLED_UI_Core/Driver/Hardware_Driver/OLED_driver.h"
#include "../Drivers/OLED_UI_Core/Driver/Hardware_Driver/OLED_UI_Driver.h"
#include "../Drivers/OLED_UI_Core/OLED_UI/OLED_UI.h"



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

#define MOTOR_PWM_PORT      GPIOB
#define MOTOR_PWM_PIN       4
#define MOTOR_PWM_TIM       TIM3
#define MOTOR_PWM_AF        GPIO_AF_TIM3
#define MOTOR_PWM_CHANNEL   TIM_CH1

/* Motor encoder pin definitions */
#define ENCODER_CH1_PORT    GPIOD
#define ENCODER_CH1_PIN     12

#define ENCODER_CH2_PORT    GPIOD
#define ENCODER_CH2_PIN     13

#define ENCODER_TIM         TIM4
#define ENCODER_AF          GPIO_AF_TIM4

/* Current sensing ADC pin definition */
#define CURRENT_ADC_PORT    GPIOA
#define CURRENT_ADC_PIN     0

/* I2C pin definitions for OLED display */
#define OLED_I2C            I2C1
#define OLED_I2C_PORT       GPIOB
#define OLED_I2C_SCL_PIN    6
#define OLED_I2C_SDA_PIN    7


/**
 * @brief UART pin definitions
 */
#define FPGA_UART_TX_PORT        GPIOD
#define FPGA_UART_TX_PIN         5
#define FPGA_UART_RX_PORT        GPIOD
#define FPGA_UART_RX_PIN         6

/**
 * @brief SPI pin definitions for W25Q128 flash memory
 * @note Using SPI1 on pins PA5 (SCK), PA6 (MISO), PA7 (MOSI), PA4 (CS)
 */
#define W25Q128_SPI_PORT         GPIOA
#define W25Q128_SCK_PIN          5    // SPI1_SCK
#define W25Q128_MISO_PIN         6    // SPI1_MISO  
#define W25Q128_MOSI_PIN         7    // SPI1_MOSI
#define W25Q128_CS_PORT          GPIOA
#define W25Q128_CS_PIN           4    // Chip Select

/* UART handle structure */



/* Current protection thresholds */
#define CURRENT_STARTUP_THRESHOLD     3800   // Startup phase high threshold (allows inrush current)
#define CURRENT_RUNNING_THRESHOLD     3200   // Normal running threshold
#define STARTUP_TIMEOUT_MS            2000   // Maximum startup phase duration (ms)
#define OVERCURRENT_COUNT_LIMIT       3     // Number of consecutive overcurrent detections before shutdown

/**
 * @brief Motor state enumeration for current protection
 */
typedef enum {
    MOTOR_STATE_STOPPED = 0,    // Motor is stopped
    MOTOR_STATE_STARTING,       // Motor is in startup phase (allows higher current)
    MOTOR_STATE_RUNNING         // Motor is running normally (strict current limit)
} Motor_State_t;

/**
 * @brief Motor current monitoring structure
 */
typedef struct {
    Motor_State_t state;                    // Current motor state
    uint32_t startup_start_time;            // Timestamp when startup began
    uint8_t overcurrent_count;              // Consecutive overcurrent detection counter
    uint16_t current_threshold;             // Active current threshold based on state
} Motor_Current_Monitor_t;

/* Global shared variables for ADC data handling */
extern volatile uint16_t current_adcBuffer[200];  // ADC sample buffer
extern uint16_t current_adcAverage;               // Calculated average value
extern volatile uint8_t current_adcAverageReady;  // Flag indicating new data is ready
extern uint32_t sum;
extern Motor_Current_Monitor_t motor_monitor;     // Motor current monitoring instance

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
 * @brief Initialize SPI Flash system
 * 
 * Configures SPI1 peripheral and initializes W25Q128 Flash memory
 * for persistent storage of motor parameters.
 */
void spi_flash_system_init(void);

/**
 * @brief Initialize all system components
 * 
 * This function initializes the complete system by calling
 * individual initialization functions in the proper sequence.
 * In the main application, only this function needs to be called.
 */
void system_init(void);

#endif //BSP_H
