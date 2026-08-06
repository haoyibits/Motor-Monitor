/**
 ******************************************************************************
 * @file           : bsp.c
 * @author         : Haoyi Chen
 * @date           : 2025-08-08
 * @brief          : Board Support Package implementation
 ******************************************************************************
 * @details
 * This file implements system initialization functions for required hardware modules
 ******************************************************************************
 */

#include "bsp.h"
#include "rcc.h"
#include "spi.h"
#include "event.h"
#include "motor.h"

/* Global ADC buffer for 200 samples */
volatile uint16_t current_adcBuffer[200];  /* Removed static to allow access from irq.c and made volatile for DMA writes */
volatile uint16_t current_adcAverage = 0;  /* Latest calculated average */
volatile uint8_t current_adcAverageReady = 0;  /* Flag indicating new average is ready */
volatile uint8_t current_adcDmaError = 0;

/**
 * @brief Initialize RCC (Reset and Clock Control)
 * 
 * Configures system clock to maximum frequency using external oscillator
 */
DriverStatus rcc_init(void)
{
    /* Configure system clock to maximum frequency */
    // uint8_t use_hse = 1;  /* Use external crystal (HSE) */
    uint8_t use_hse = 0;  /* Use internal crystal (HSI) */
    uint32_t hse_freq = 8000000;  /* 8MHz external crystal */
    
    DriverStatus status = rcc_config_max_frequency(use_hse, hse_freq);
    if (status != DRIVER_STATUS_OK) {
        const RCC_ClockConfigTypeDef safe_clock = {
            .ClockSource = RCC_CLOCK_HSI,
            .PLLSource = RCC_PLL_SOURCE_HSI,
            .HSEFrequency = 0U,
            .PLL_M = 0U,
            .PLL_N = 0U,
            .PLL_P = 0U,
            .PLL_Q = 0U,
            .AHB_Prescaler = RCC_AHB_DIV1,
            .APB1_Prescaler = RCC_APB_DIV1,
            .APB2_Prescaler = RCC_APB_DIV1,
            .Latency = 0U
        };
        DriverStatus fallback_status = rcc_system_clock_config(&safe_clock);
        if (fallback_status != DRIVER_STATUS_OK) {
            return fallback_status;
        }
    }
    
    SystemCoreClockUpdate(); // Update SystemCoreClock variable after clock configuration
    /* Enable peripheral clocks */
    rcc_enable_gpio_clock(GPIOA);
    rcc_enable_gpio_clock(GPIOB);
    rcc_enable_gpio_clock(GPIOE);
    rcc_enable_gpio_clock(GPIOD);
    rcc_enable_adc_clock(ADC1);
    rcc_enable_dma_clock(DMA2);
    rcc_enable_tim_clock(TIM4);
    rcc_enable_usart_clock(USART2);
    rcc_enable_i2c_clock(I2C1);
    rcc_enable_spi_clock(SPI1);

    return status;
}

/**
 * @brief Initialize GPIO pins
 * 
 * Configures required GPIO pins including SWD debug interface
 */
void gpio_system_init(void)
{
    
    /* Motor initialize with encoder */
    motor_init();
    /* Note: Motor start is handled by motor_apply_config() when configuration is loaded */
    /* Configure GPIO pin for ADC input */
    gpio_init(CURRENT_ADC_PORT, CURRENT_ADC_PIN, GPIO_MODE_ANALOG, 0, 0, GPIO_NOPULL);  // ADC input for motor monitoring

    /* Button initialize */
    button_system_init();
}

/**
 * @brief Initialize ADC and DMA
 * 
 * Configures ADC for continuous sampling with DMA on PA0
 * Using library functions while maintaining the critical sequence of operations
 * 
 * @note Critical aspects for correct ADC-DMA operation:
 * 1. DDS bit (ADC_CR2_DDS) must be explicitly set to generate DMA requests after each conversion
 * 2. DMA must be enabled before starting ADC conversion
 * 3. Proper flag clearing before enabling DMA is essential
 * 4. Sequence of operations: DMA config → DMA enable → ADC config → ADC DMA enable → Set DDS → Start ADC
 */
void adc_dma_init(void)
{
    /* ADC1 initialization */  
    ADC_InitTypeDef adc_config;  
    ADC_ChannelConfTypeDef adc_channel_config;  
    DMA_InitTypeDef dma_config;
      
    /* Configure ADC1 with high speed and DMA enabled */  
    adc_config.Resolution = ADC_RESOLUTION_12BIT;        /* 12-bit resolution for high accuracy */  
    adc_config.Align = ADC_DATAALIGN_RIGHT;              /* Right alignment of data */  
    adc_config.ScanMode = ADC_SCAN_DISABLE;              /* Single channel, no scan needed */  
    adc_config.ContMode = ADC_CONTINUOUS_ENABLE;         /* Enable continuous conversion mode */  
    adc_config.ExternalTrigger = 0;                      /* No external trigger needed */  
    adc_config.ExternalTrigConv = 0;                     /* No external trigger edge */  
    adc_config.DataManagement = ADC_DMA_CIRCULAR;        /* Enable circular DMA mode */  
    if (adc_init(ADC1, &adc_config) != DRIVER_STATUS_OK) {
        return;
    }
      
    /* Configure ADC channel for current sensing on PA0 */  
    adc_channel_config.Channel = ADC_CHANNEL_0;        /* PA0 = ADC1_IN0 */  
    adc_channel_config.Rank = ADC_REGULAR_RANK_1;      /* Set as first and only conversion */  
    adc_channel_config.SamplingTime = ADC_SAMPLETIME_28CYCLES;  /* Sampling time for ~500kHz */  
    if (adc_config_channel(ADC1, &adc_channel_config) != DRIVER_STATUS_OK) {
        return;
    }
      
    /* Clear the ADC buffer to avoid confusion during debugging */  
    for (int i = 0; i < 200; i++) {  
        current_adcBuffer[i] = 0;  
    }  
      
    /* Configure DMA parameters */
    dma_config.Channel = DMA_CHANNEL_0;                  /* ADC uses DMA2 Stream0 Channel 0 */
    dma_config.Direction = DMA_PERIPH_TO_MEMORY;         /* Data transfer from ADC to memory */
    dma_config.PeriphInc = DMA_PINC_DISABLE;             /* Don't increment peripheral address */
    dma_config.MemInc = DMA_MINC_ENABLE;                 /* Increment memory address */
    dma_config.PeriphDataAlign = DMA_PDATAALIGN_HALFWORD; /* ADC data is 16-bit */
    dma_config.MemDataAlign = DMA_MDATAALIGN_HALFWORD;    /* Memory is also 16-bit */
    dma_config.Mode = DMA_CIRCULAR;                      /* Circular mode enabled */
    dma_config.Priority = DMA_PRIORITY_HIGH;             /* High priority */
    dma_config.FIFOMode = DMA_FIFOMODE_DISABLE;          /* FIFO disabled, direct mode */
    dma_config.FIFOThreshold = 0;                        /* Not used in direct mode */
    dma_config.MemBurst = DMA_MBURST_SINGLE;             /* Single transfer */
    dma_config.PeriphBurst = DMA_PBURST_SINGLE;          /* Single transfer */
    
    /* Initialize DMA using library function */
    if (dma_init(DMA2, DMA_STREAM0, &dma_config,
                 DMA_DEFAULT_TIMEOUT_CYCLES) != DRIVER_STATUS_OK) {
        return;
    }
    
    /* Configure DMA transfer parameters */
    if (dma_config_transfer(DMA2, DMA_STREAM0,
                            (uint32_t)&ADC1->DR,
                            (uint32_t)current_adcBuffer,
                            200U,
                            DMA_DEFAULT_TIMEOUT_CYCLES) != DRIVER_STATUS_OK) {
        return;
    }
    
    /* Enable DMA interrupts for transfer complete and half transfer */
    (void)dma_enable_interrupt(DMA2, DMA_STREAM0,
                               DMA_SxCR_TCIE | DMA_SxCR_HTIE |
                               DMA_SxCR_TEIE | DMA_SxCR_DMEIE |
                               DMA_SxFCR_FEIE);
    
    /* Configure interrupt priority and enable in NVIC */
    NVIC_SetPriority(DMA2_Stream0_IRQn, 0);
    NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    
    /* Critical sequence: Enable DMA stream before configuring ADC */
    if (dma_enable(DMA2, DMA_STREAM0) != DRIVER_STATUS_OK) {
        return;
    }
    
    /* Enable ADC with explicit DDS bit setting (critical for continuous DMA operation) */
    adc_enable(ADC1);
    
    /* Critical: Setting DDS bit explicitly to ensure DMA requests after each conversion */
    ADC1->CR2 |= ADC_CR2_DMA;  /* Enable DMA mode */
    ADC1->CR2 |= ADC_CR2_DDS;  /* DMA requests generated after each conversion */
    ADC1->CR2 |= ADC_CR2_CONT; /* Ensure continuous mode is set */
    
    /* Start ADC conversion */
    adc_start_conversion(ADC1);
}

/**
 * @brief Initialize UART interface
 * 
 * Configures UART2 for communication at 115200 baud, 8N1 on PD5/PD6
 */
void uart_system_init(void)
{
    UART_HandleTypeDef huart2;
    /* Configure UART pins */
    UART_PinConfig uart_pins;
    uart_pins.tx_port = FPGA_UART_TX_PORT;
    uart_pins.tx_pin = FPGA_UART_TX_PIN;
    uart_pins.rx_port = FPGA_UART_RX_PORT;
    uart_pins.rx_pin = FPGA_UART_RX_PIN;
    uart_pins.alt_func = GPIO_AF_USART2;  /* Alternate function for USART2 */
    
    /* Configure UART initialization structure */
    huart2.Instance = USART2;             /* Use USART2 peripheral */
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HardwareFlowControl = UART_HWCONTROL_NONE;
    
    /* Initialize UART */
    uart_init(&huart2, &uart_pins);
}

/**
 * @brief Initialize SPI Flash system
 * 
 * @details Configures SPI1 peripheral and initializes W25Q128 Flash memory
 *          for persistent storage of motor parameters.
 *          
 * @note Currently commented out until SPI driver is implemented
 */
void spi_flash_system_init(void)
{
    /* Configure SPI pins for W25Q128 Flash */
    gpio_init(W25Q128_SPI_PORT, W25Q128_SCK_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_NOPULL);     // PA5 - SCK
    gpio_init(W25Q128_SPI_PORT, W25Q128_MISO_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PULLUP);    // PA6 - MISO
    gpio_init(W25Q128_SPI_PORT, W25Q128_MOSI_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_NOPULL);    // PA7 - MOSI
    gpio_init(W25Q128_CS_PORT, W25Q128_CS_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_NOPULL);   // PA4 - CS
    
    /* Set alternate function for SPI pins (AF5 for SPI1) */
    gpio_set_af(W25Q128_SPI_PORT, W25Q128_SCK_PIN, 5);   // AF5 = SPI1
    gpio_set_af(W25Q128_SPI_PORT, W25Q128_MISO_PIN, 5);  // AF5 = SPI1
    gpio_set_af(W25Q128_SPI_PORT, W25Q128_MOSI_PIN, 5);  // AF5 = SPI1
    
    /* Set CS pin high (inactive) initially */
    gpio_write(W25Q128_CS_PORT, W25Q128_CS_PIN, 1);
    
    /* Enable SPI1 clock */
    rcc_enable_spi_clock(SPI1);
    
    /* Initialize SPI1 peripheral */
    SPI_Config_t spi_config = {
        .mode = SPI_MODE_MASTER,
        .dataSize = SPI_DATASIZE_8BIT,
        .clockPolarity = SPI_CPOL_LOW,
        .clockPhase = SPI_CPHA_1EDGE,
        .prescaler = SPI_PRESCALER_8,    // APB2=84MHz, SPI=10.5MHz
        .firstBit = SPI_FIRSTBIT_MSB
    };
    
    if (spi_init(SPI1, &spi_config) == DRIVER_STATUS_OK) {
        (void)spi_enable(SPI1);
    }
}

/**
 * @brief Initialize all system components
 * 
 * This function initializes the complete system by calling
 * individual initialization functions in the proper sequence.
 * In the main application, only this function needs to be called.
 */
void system_init(void)
{
    /* Initialize basic hardware first */
    (void)rcc_init();       // First initialize system clock and peripheral clocks
    systick_init(SystemCoreClock); // Initialize SysTick for 1ms timing
    gpio_system_init();     // Initialize GPIO pins
    /* Early motor configuration loading (highest priority) */
    motor_config_early_load();
    
    /* Continue with remaining system initialization */
    adc_dma_init();         // Initialize ADC with DMA in continuous mode
    
    /* UI hardware is initialized once by OLED_UI_Init() in the application layer. */
}
