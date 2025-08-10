/**
 ******************************************************************************
 * @file           : bsp.c
 * @author         : Haoyi Chen
 * @date           : 2025-08-08
 * @brief          : Board Support Package implementation
 ******************************************************************************
 * @details
 * This file implements system initialization functions for required hardware modules
 * including clock, GPIO, and ADC with DMA.
 *
 * This file is part of a bare-metal STM32F407VGT6 project.
 ******************************************************************************
 */

#include "bsp.h"

/* Global ADC buffer for 200 samples */
volatile uint16_t current_adcBuffer[200];  /* Removed static to allow access from irq.c and made volatile for DMA writes */
uint16_t current_adcAverage = 0;  /* Latest calculated average */
volatile uint8_t current_adcAverageReady = 0;  /* Flag indicating new average is ready */
uint32_t sum = 0;
/**
 * @brief Initialize RCC (Reset and Clock Control)
 * 
 * Configures system clock to maximum frequency using external oscillator
 */
void rcc_init(void)
{
    /* Configure system clock to maximum frequency */
    // uint8_t use_hse = 1;  /* Use external crystal (HSE) */
    uint8_t use_hse = 0;  /* Use internal crystal (HSI) */
    uint32_t hse_freq = 8000000;  /* 8MHz external crystal */
    
    rcc_config_max_frequency(use_hse, hse_freq);
    
    /* Enable peripheral clocks */
    rcc_enable_gpio_clock(GPIOA);
    rcc_enable_gpio_clock(GPIOB);
    rcc_enable_gpio_clock(GPIOE);
    rcc_enable_adc_clock(ADC1);
    rcc_enable_dma_clock(DMA2);  
}

/**
 * @brief Initialize GPIO pins
 * 
 * Configures required GPIO pins including SWD debug interface
 */
void gpio_system_init(void)
{
    /* Configure SWD debug interface pins for OpenOCD */
    gpio_init(GPIOA, 13, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_VHIGH, GPIO_PULLUP);   // SWDIO (PA13)
    gpio_init(GPIOA, 14, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_VHIGH, GPIO_PULLDOWN); // SWCLK (PA14)
    
    /* Set alternate function for SWD pins (AF0 for SWD) */
    GPIOA->AFR[1] &= ~((0xF << ((13-8)*4)) | (0xF << ((14-8)*4))); // Clear PA13 and PA14 AF bits
    GPIOA->AFR[1] |=  ((0x0 << ((13-8)*4)) | (0x0 << ((14-8)*4))); // Set PA13 and PA14 to AF0 (SWD)
    
    /* Configure motor control pins as GPIO outputs */
    gpio_init(GPIOB, 2, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_SPEED_MED, GPIO_NOPULL);
    gpio_init(MOTOR_P_PORT, MOTOR_P_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_SPEED_MED, GPIO_NOPULL);        // Motor P control
    gpio_init(MOTOR_M_PORT, MOTOR_M_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_SPEED_MED, GPIO_NOPULL);        // Motor M control
    gpio_init(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_SPEED_MED, GPIO_NOPULL); // Motor enable
    
    /* Configure GPIO pin for ADC input */
    gpio_init(CURRENT_ADC_PORT, CURRENT_ADC_PIN, GPIO_MODE_ANALOG, 0, 0, GPIO_NOPULL);  // ADC input for motor monitoring
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
    adc_init(ADC1, &adc_config);  
      
    /* Configure ADC channel for current sensing on PA0 */  
    adc_channel_config.Channel = ADC_CHANNEL_0;        /* PA0 = ADC1_IN0 */  
    adc_channel_config.Rank = ADC_REGULAR_RANK_1;      /* Set as first and only conversion */  
    adc_channel_config.SamplingTime = ADC_SAMPLETIME_28CYCLES;  /* Sampling time for ~500kHz */  
    adc_config_channel(ADC1, &adc_channel_config);  
      
    /* Clear the ADC buffer to avoid confusion during debugging */  
    for (int i = 0; i < 200; i++) {  
        current_adcBuffer[i] = 0;  
    }  
      
    /* Reset DMA configuration before setup */  
    /* Critical: First disable DMA if it's enabled */
    DMA_Stream_TypeDef *DMAStream = (DMA_Stream_TypeDef *)(DMA2_BASE + 0x10); /* DMA2 Stream0 */
    DMAStream->CR &= ~DMA_SxCR_EN;  /* Disable stream */
    
    /* Wait for DMA reset to complete */  
    while (DMAStream->CR & DMA_SxCR_EN) { }
    
    /* Clear all flags for Stream 0 */
    DMA2->LIFCR = 0x3F << 0;
      
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
    dma_init(DMA2, DMA_STREAM0, &dma_config);
    
    /* Configure DMA transfer parameters */
    dma_config_transfer(DMA2, DMA_STREAM0, 
                       (uint32_t)&ADC1->DR,              /* Source: ADC data register */
                       (uint32_t)current_adcBuffer,      /* Destination: ADC buffer */
                       200);                             /* Buffer size: 200 samples */
    
    /* Enable DMA interrupts for transfer complete and half transfer */
    dma_enable_interrupt(DMA2, DMA_STREAM0, DMA_SxCR_TCIE | DMA_SxCR_HTIE);
    
    /* Configure interrupt priority and enable in NVIC */
    NVIC_SetPriority(DMA2_Stream0_IRQn, 0);
    NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    
    /* Critical sequence: Enable DMA stream before configuring ADC */
    dma_enable(DMA2, DMA_STREAM0);
    
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
 * @brief Initialize all system components
 * 
 * This function initializes the complete system by calling
 * individual initialization functions in the proper sequence.
 * In the main application, only this function needs to be called.
 */
void system_init(void)
{
    rcc_init();             // First initialize system clock and peripheral clocks
    gpio_system_init();     // Then initialize GPIO pins
    adc_dma_init();         // Initialize ADC with DMA in continuous mode
}
