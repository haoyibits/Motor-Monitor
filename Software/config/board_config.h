/**
 * @file board_config.h
 * @brief Hardware configuration definitions for STM32F407VGT6 motor monitor board
 * 
 * Contains all hardware-specific configurations including pin definitions,
 * clock configurations, and peripheral settings.
 */

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/* Target MCU Configuration */
#define MCU_STM32F407VGT6
#define HSE_VALUE                8000000U  /* External oscillator frequency */
#define HSI_VALUE               16000000U  /* Internal oscillator frequency */

/* System Clock Configuration */
#define SYSTEM_CLOCK_FREQ       168000000U /* System clock frequency (Hz) */
#define AHB_CLOCK_FREQ          168000000U /* AHB bus frequency (Hz) */
#define APB1_CLOCK_FREQ          42000000U /* APB1 bus frequency (Hz) */
#define APB2_CLOCK_FREQ          84000000U /* APB2 bus frequency (Hz) */

/* Motor Control Pin Definitions */
#define MOTOR_ENABLE_PORT       GPIOB
#define MOTOR_ENABLE_PIN        0          /* PB0 - Motor Enable */

#define MOTOR_DIR_PORT          GPIOB  
#define MOTOR_DIR_PIN           1          /* PB1 - Motor Direction */

#define MOTOR_PWM_PORT          GPIOE
#define MOTOR_PWM_PIN           7          /* PE7 - Motor PWM */

/* Encoder Pin Definitions (TIM2) */
#define ENCODER_A_PORT          GPIOA
#define ENCODER_A_PIN           2          /* PA2 - Encoder Channel A */

#define ENCODER_B_PORT          GPIOA
#define ENCODER_B_PIN           3          /* PA3 - Encoder Channel B */

/* Current Sensing ADC Configuration */
#define CURRENT_SENSE_PORT      GPIOA
#define CURRENT_SENSE_PIN       0          /* PA0 - ADC1_IN0 */
#define CURRENT_SENSE_ADC       ADC1
#define CURRENT_SENSE_CHANNEL   0

/* Button Pin Definitions */
#define BUTTON_UP_PORT          GPIOE
#define BUTTON_UP_PIN           9          /* PE9 - Up Button */

#define BUTTON_DOWN_PORT        GPIOE
#define BUTTON_DOWN_PIN         10         /* PE10 - Down Button */

#define BUTTON_ENTER_PORT       GPIOE
#define BUTTON_ENTER_PIN        11         /* PE11 - Enter Button */

#define BUTTON_BACK_PORT        GPIOE
#define BUTTON_BACK_PIN         12         /* PE12 - Back Button */

/* UART Communication (FPGA Interface) */
#define UART_TX_PORT            GPIOD
#define UART_TX_PIN             5          /* PD5 - UART2 TX */

#define UART_RX_PORT            GPIOD
#define UART_RX_PIN             6          /* PD6 - UART2 RX */

/* I2C OLED Display Configuration */
#define I2C_SCL_PORT            GPIOB
#define I2C_SCL_PIN             6          /* PB6 - I2C1 SCL */

#define I2C_SDA_PORT            GPIOB
#define I2C_SDA_PIN             7          /* PB7 - I2C1 SDA */

/* SPI Configuration (External Flash) */
#define SPI_SCK_PORT            GPIOA
#define SPI_SCK_PIN             5          /* PA5 - SPI1 SCK */

#define SPI_MISO_PORT           GPIOA
#define SPI_MISO_PIN            6          /* PA6 - SPI1 MISO */

#define SPI_MOSI_PORT           GPIOA
#define SPI_MOSI_PIN            7          /* PA7 - SPI1 MOSI */

#define SPI_CS_PORT             GPIOA
#define SPI_CS_PIN              4          /* PA4 - SPI1 CS */

/* ADC-DMA Configuration */
#define ADC_BUFFER_SIZE         200        /* ADC sample buffer size */
#define ADC_SAMPLE_RATE         500000     /* ADC sampling rate (Hz) */

/* Current Protection Thresholds */
#define CURRENT_LIMIT_DEFAULT   2.0f       /* Default current limit (A) */
#define CURRENT_LIMIT_MAX       5.0f       /* Maximum current limit (A) */
#define CURRENT_LIMIT_MIN       0.1f       /* Minimum current limit (A) */

/* Motor Control Parameters */
#define PWM_FREQUENCY           20000      /* PWM frequency (Hz) */
#define PWM_DUTY_DEFAULT        50.0f      /* Default PWM duty cycle (%) */

/* Timer Configuration */
#define SYSTICK_FREQ            1000       /* SysTick frequency (Hz) - 1ms */
#define UI_REFRESH_RATE         50         /* UI refresh rate (Hz) - 20ms */

/* Debug Configuration */
#define RTT_BUFFER_SIZE         1024       /* SEGGER RTT buffer size */

#endif /* BOARD_CONFIG_H */