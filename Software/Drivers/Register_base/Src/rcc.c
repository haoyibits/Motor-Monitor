/**
 * @file rcc.c
 * @author Haoyi Chen
 * @date 2025-08-08
 * @brief STM32F4 RCC (Reset and Clock Control) register-level driver
 *
 * @details This file provides functions for configuring the system clock and
 * peripheral clocks for STM32F407 series microcontrollers. All operations are
 * performed at the register level, without using HAL or LL libraries.
 *
 * Created for personal learning and embedded systems experimentation.
 */

#include "../Inc/rcc.h"

/* Private variables for tracking clock frequencies */
static uint32_t HSE_VALUE = 8000000; /* Default HSE frequency (8MHz) */
static uint32_t SystemClockFreq = 16000000; /* Default HSI frequency */
static uint32_t HCLKFreq = 16000000;
static uint32_t PCLK1Freq = 16000000;
static uint32_t PCLK2Freq = 16000000;
uint32_t system_clock =168 * 1000000; /* 168MHz system clock */
/**
 * @brief Configure the system clock
 * 
 * @details Sets up the system clock according to the provided configuration.
 *          Handles PLL configuration, clock source selection, prescalers, and flash latency.
 * 
 * @param config Pointer to clock configuration structure
 * @return uint8_t 0 if successful, 1 if error occurred
 */
uint8_t rcc_system_clock_config(RCC_ClockConfigTypeDef *config) {
    uint32_t timeout = 0;
    uint32_t pll_source = 0;
    uint32_t system_clock_freq = 0;
    
    /* Configure Flash latency */
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | (config->Latency << FLASH_ACR_LATENCY_Pos);
    
    /* Configure clock source */
    switch (config->ClockSource) {
        case RCC_CLOCK_HSI:
            /* Enable HSI if not already on */
            if ((RCC->CR & RCC_CR_HSION) == 0) {
                RCC->CR |= RCC_CR_HSION;
                /* Wait for HSI to be stable */
                timeout = 10000;
                while ((RCC->CR & RCC_CR_HSIRDY) == 0) {
                    if (--timeout == 0) return 1;
                }
            }
            system_clock_freq = 16000000; /* HSI is fixed at 16MHz */
            /* Select HSI as system clock */
            RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
            /* Wait for HSI to be used as system clock */
            timeout = 10000;
            while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI) {
                if (--timeout == 0) return 1;
            }
            break;
            
        case RCC_CLOCK_HSE:
            /* Enable HSE */
            RCC->CR |= RCC_CR_HSEON;
            /* Wait for HSE to be stable */
            timeout = 10000;
            while ((RCC->CR & RCC_CR_HSERDY) == 0) {
                if (--timeout == 0) return 1;
            }
            system_clock_freq = HSE_VALUE; /* HSE_VALUE defined in system header */
            /* Select HSE as system clock */
            RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSE;
            /* Wait for HSE to be used as system clock */
            timeout = 10000;
            while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE) {
                if (--timeout == 0) return 1;
            }
            break;
            
        case RCC_CLOCK_PLL:
            /* Disable PLL first */
            RCC->CR &= ~RCC_CR_PLLON;
            /* Wait till PLL is disabled */
            timeout = 10000;
            while ((RCC->CR & RCC_CR_PLLRDY) != 0) {
                if (--timeout == 0) return 1;
            }
            
            /* Select PLL source */
            if (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) {
                pll_source = HSE_VALUE; /* HSE is PLL source */
            } else {
                pll_source = 16000000; /* HSI is PLL source */
            }
            
            /* Configure the main PLL */
            RCC->PLLCFGR = (config->PLL_M << RCC_PLLCFGR_PLLM_Pos) |
                           (config->PLL_N << RCC_PLLCFGR_PLLN_Pos) |
                           ((config->PLL_P >> 1) - 1) << RCC_PLLCFGR_PLLP_Pos |
                           (config->PLL_Q << RCC_PLLCFGR_PLLQ_Pos);
            
            /* Enable PLL */
            RCC->CR |= RCC_CR_PLLON;
            /* Wait till PLL is ready */
            timeout = 10000;
            while ((RCC->CR & RCC_CR_PLLRDY) == 0) {
                if (--timeout == 0) return 1;
            }
            
            /* Calculate PLL output frequency */
            system_clock_freq = (pll_source / config->PLL_M) * config->PLL_N / config->PLL_P;
            
            /* Select PLL as system clock source */
            RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
            /* Wait till PLL is used as system clock source */
            timeout = 10000;
            while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
                if (--timeout == 0) return 1;
            }
            break;
            
        default:
            return 1; /* Invalid clock source */
    }
    
    /* Configure the HCLK, PCLK1, and PCLK2 clock dividers */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE) | (config->AHB_Prescaler << RCC_CFGR_HPRE_Pos);
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE1) | (config->APB1_Prescaler << RCC_CFGR_PPRE1_Pos);
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_PPRE2) | (config->APB2_Prescaler << RCC_CFGR_PPRE2_Pos);
    
    /* Update the global clock frequency variables */
    SystemClockFreq = system_clock_freq;
    
    /* Calculate HCLK frequency */
    if ((config->AHB_Prescaler & 0x08) == 0) {
        HCLKFreq = SystemClockFreq; /* Not divided */
    } else {
        HCLKFreq = SystemClockFreq >> (((config->AHB_Prescaler & 0x07) + 1));
    }
    
    /* Calculate PCLK1 frequency */
    if ((config->APB1_Prescaler & 0x04) == 0) {
        PCLK1Freq = HCLKFreq; /* Not divided */
    } else {
        PCLK1Freq = HCLKFreq >> (((config->APB1_Prescaler & 0x03) + 1));
    }
    
    /* Calculate PCLK2 frequency */
    if ((config->APB2_Prescaler & 0x04) == 0) {
        PCLK2Freq = HCLKFreq; /* Not divided */
    } else {
        PCLK2Freq = HCLKFreq >> (((config->APB2_Prescaler & 0x03) + 1));
    }
    
    return 0; /* Configuration successful */
}

/**
 * @brief Enable peripheral clock for a GPIO port
 * 
 * @param GPIOx Pointer to GPIO port (e.g. GPIOA, GPIOB)
 */
void rcc_enable_gpio_clock(GPIO_TypeDef *GPIOx) {
    if (GPIOx == GPIOA)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    else if (GPIOx == GPIOB)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    else if (GPIOx == GPIOC)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    else if (GPIOx == GPIOD)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    else if (GPIOx == GPIOE)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    else if (GPIOx == GPIOF)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
    else if (GPIOx == GPIOG)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    else if (GPIOx == GPIOH)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
    else if (GPIOx == GPIOI)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
}

/**
 * @brief Enable peripheral clock for a timer
 * 
 * @param TIMx Pointer to timer (e.g. TIM1, TIM2)
 */
void rcc_enable_tim_clock(TIM_TypeDef *TIMx) {
    if (TIMx == TIM1)
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    else if (TIMx == TIM2)
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    else if (TIMx == TIM3)
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    else if (TIMx == TIM4)
        RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    else if (TIMx == TIM5)
        RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
    else if (TIMx == TIM6)
        RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    else if (TIMx == TIM7)
        RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    else if (TIMx == TIM8)
        RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
    else if (TIMx == TIM9)
        RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
    else if (TIMx == TIM10)
        RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
    else if (TIMx == TIM11)
        RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
    else if (TIMx == TIM12)
        RCC->APB1ENR |= RCC_APB1ENR_TIM12EN;
    else if (TIMx == TIM13)
        RCC->APB1ENR |= RCC_APB1ENR_TIM13EN;
    else if (TIMx == TIM14)
        RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
}

/**
 * @brief Enable peripheral clock for ADC
 * 
 * @param ADCx Pointer to ADC (e.g. ADC1, ADC2)
 */
void rcc_enable_adc_clock(ADC_TypeDef *ADCx) {
    if (ADCx == ADC1)
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    else if (ADCx == ADC2)
        RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
    else if (ADCx == ADC3)
        RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;
}

/**
 * @brief Enable peripheral clock for I2C
 * 
 * @param I2Cx Pointer to I2C (e.g. I2C1, I2C2)
 */
void rcc_enable_i2c_clock(I2C_TypeDef *I2Cx) {
    if (I2Cx == I2C1)
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    else if (I2Cx == I2C2)
        RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    else if (I2Cx == I2C3)
        RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
}

/**
 * @brief Enable DMA clocks
 * 
 * @param DMA_number Pointer to DMA instance (DMA1 or DMA2)
 */
void rcc_enable_dma_clock(DMA_TypeDef *DMA_number) {
    if (DMA_number == DMA1)
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    else if (DMA_number == DMA2)
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
}

/**
 * @brief Enable peripheral clock for USART/UART
 * 
 * @param USARTx Pointer to USART (e.g. USART1, USART2, UART4)
 */
void rcc_enable_usart_clock(USART_TypeDef *USARTx) {
    if (USARTx == USART1)
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    else if (USARTx == USART2)
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    else if (USARTx == USART3)
        RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    else if (USARTx == UART4)
        RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
    else if (USARTx == UART5)
        RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
    else if (USARTx == USART6)
        RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
}

/**
 * @brief Configure and set system to maximum frequency using either HSI or HSE
 * 
 * @details This is a convenient function to set the system clock to 168MHz,
 *          which is the maximum frequency for STM32F407. It can use either
 *          the HSI (internal oscillator) or HSE (external crystal) as the 
 *          PLL clock source, depending on the use_hse parameter.
 * 
 * @param use_hse 0 to use internal oscillator (HSI), 1 to use external crystal (HSE)
 * @param hse_freq External crystal frequency in Hz (typically 8000000 for 8MHz)
 *                 This parameter is ignored when use_hse = 0
 * @return uint8_t 0 if successful, 1 if error occurred
 */
uint8_t rcc_config_max_frequency(uint8_t use_hse, uint32_t hse_freq) {
    RCC_ClockConfigTypeDef config;

    /* Configure for 168MHz operation */
    config.ClockSource = RCC_CLOCK_PLL;

    if (use_hse) {
        /* Using HSE as PLL source */
        RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC;  /* Set PLLSRC bit for HSE */
        config.PLL_M = hse_freq / 2000000;   /* HSE_freq / M = 2MHz */
    } else {
        /* Using HSI as PLL source (16MHz) */
        RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC; /* Clear PLLSRC bit for HSI */
        config.PLL_M = 8;                   /* 16MHz / 8 = 2MHz */
    }

    config.PLL_N = 168;          /* 2MHz * 168 = 336MHz */
    config.PLL_P = 2;            /* 336MHz / 2 = 168MHz */
    config.PLL_Q = 7;            /* USB clock = 336MHz / 7 = 48MHz */
    config.AHB_Prescaler = RCC_AHB_DIV1;   /* AHB at 168MHz */
    config.APB1_Prescaler = RCC_APB_DIV4;  /* APB1 at 42MHz */
    config.APB2_Prescaler = RCC_APB_DIV2;  /* APB2 at 84MHz */
    config.Latency = 5;          /* 5 wait states for 168MHz operation */
    return rcc_system_clock_config(&config);
}

/**
 * @brief Get APB1 peripheral clock frequency
 * 
 * @return uint32_t APB1 clock frequency in Hz
 */
uint32_t rcc_get_pclk1_freq(void)
{
    uint32_t pclk1;
    uint32_t apb1_prescaler;
    
    /* Get APB1 prescaler value from CFGR register */
    apb1_prescaler = (RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos;
    
    /* Calculate PCLK1 frequency based on prescaler */
    if ((apb1_prescaler & 0x04) == 0) {
        /* APB1 clock not divided */
        pclk1 = system_clock;
    } else {
        /* APB1 clock divided by 2, 4, 8, or 16 */
        pclk1 = system_clock >> (((apb1_prescaler & 0x03) + 1));
    }
    
    return pclk1;
}

/**
 * @brief Get APB2 peripheral clock frequency
 * 
 * @return uint32_t APB2 clock frequency in Hz
 */
uint32_t rcc_get_pclk2_freq(void)
{
    uint32_t pclk2;
    uint32_t apb2_prescaler;
    
    /* Get APB2 prescaler value from CFGR register */
    apb2_prescaler = (RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos;
    
    /* Calculate PCLK2 frequency based on prescaler */
    if ((apb2_prescaler & 0x04) == 0) {
        /* APB2 clock not divided */
        pclk2 = system_clock;
    } else {
        /* APB2 clock divided by 2, 4, 8, or 16 */
        pclk2 = system_clock >> (((apb2_prescaler & 0x03) + 1));
    }
    
    return pclk2;
}
