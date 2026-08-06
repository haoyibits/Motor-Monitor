/**
 * @file rcc.c
 * @brief STM32F407 RCC register-level driver.
 */

#include <stddef.h>

#include "rcc.h"

#define RCC_HSI_FREQUENCY 16000000UL
#define RCC_MAX_SYSCLK    168000000UL
#define RCC_READY_TIMEOUT 1000000UL

static uint32_t hclk_frequency = RCC_HSI_FREQUENCY;
static uint32_t pclk1_frequency = RCC_HSI_FREQUENCY;
static uint32_t pclk2_frequency = RCC_HSI_FREQUENCY;
uint32_t system_clock = RCC_HSI_FREQUENCY;

static DriverStatus rcc_wait_mask(volatile uint32_t *reg, uint32_t mask,
                                  uint32_t expected)
{
    uint32_t timeout = RCC_READY_TIMEOUT;
    while (timeout > 0U) {
        if ((*reg & mask) == expected) {
            return DRIVER_STATUS_OK;
        }
        --timeout;
    }
    return DRIVER_STATUS_TIMEOUT;
}

static uint32_t rcc_ahb_divisor(RCC_AHB_PrescalerTypeDef prescaler)
{
    static const uint16_t divisors[8] = {2U, 4U, 8U, 16U, 64U, 128U, 256U, 512U};
    uint32_t encoding = (uint32_t)prescaler;
    return (encoding < 8U) ? 1U : divisors[encoding - 8U];
}

static uint32_t rcc_apb_divisor(RCC_APB_PrescalerTypeDef prescaler)
{
    uint32_t encoding = (uint32_t)prescaler;
    return (encoding < 4U) ? 1U : (1UL << (encoding - 3U));
}

static uint8_t rcc_ahb_prescaler_valid(RCC_AHB_PrescalerTypeDef prescaler)
{
    uint32_t value = (uint32_t)prescaler;
    return ((value == RCC_AHB_DIV1) ||
            ((value >= RCC_AHB_DIV2) && (value <= RCC_AHB_DIV512))) ? 1U : 0U;
}

static uint8_t rcc_apb_prescaler_valid(RCC_APB_PrescalerTypeDef prescaler)
{
    uint32_t value = (uint32_t)prescaler;
    return ((value == RCC_APB_DIV1) ||
            ((value >= RCC_APB_DIV2) && (value <= RCC_APB_DIV16))) ? 1U : 0U;
}

static uint8_t rcc_flash_latency_required(uint32_t hclk)
{
    if (hclk > 150000000UL) return 5U;
    if (hclk > 120000000UL) return 4U;
    if (hclk > 90000000UL) return 3U;
    if (hclk > 60000000UL) return 2U;
    if (hclk > 30000000UL) return 1U;
    return 0U;
}

static DriverStatus rcc_enable_source(RCC_PLLSourceTypeDef source)
{
    if (source == RCC_PLL_SOURCE_HSE) {
        RCC->CR |= RCC_CR_HSEON;
        return rcc_wait_mask(&RCC->CR, RCC_CR_HSERDY, RCC_CR_HSERDY);
    }
    RCC->CR |= RCC_CR_HSION;
    return rcc_wait_mask(&RCC->CR, RCC_CR_HSIRDY, RCC_CR_HSIRDY);
}

static DriverStatus rcc_switch_system_clock(uint32_t sw, uint32_t sws)
{
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | sw;
    return rcc_wait_mask(&RCC->CFGR, RCC_CFGR_SWS, sws);
}

static DriverStatus rcc_validate_config(const RCC_ClockConfigTypeDef *config,
                                        uint32_t *sysclk,
                                        uint32_t *hclk)
{
    if ((config == NULL) || (sysclk == NULL) || (hclk == NULL) ||
        (!rcc_ahb_prescaler_valid(config->AHB_Prescaler)) ||
        (!rcc_apb_prescaler_valid(config->APB1_Prescaler)) ||
        (!rcc_apb_prescaler_valid(config->APB2_Prescaler)) ||
        (config->Latency > 7U)) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    uint32_t source_frequency;
    if (config->ClockSource == RCC_CLOCK_HSI) {
        source_frequency = RCC_HSI_FREQUENCY;
    } else if (config->ClockSource == RCC_CLOCK_HSE) {
        source_frequency = config->HSEFrequency;
        if ((source_frequency < 4000000UL) ||
            (source_frequency > 26000000UL)) {
            return DRIVER_STATUS_OUT_OF_RANGE;
        }
    } else if (config->ClockSource == RCC_CLOCK_PLL) {
        source_frequency = (config->PLLSource == RCC_PLL_SOURCE_HSE) ?
                           config->HSEFrequency : RCC_HSI_FREQUENCY;
        if ((config->PLLSource != RCC_PLL_SOURCE_HSI) &&
            (config->PLLSource != RCC_PLL_SOURCE_HSE)) {
            return DRIVER_STATUS_INVALID_ARGUMENT;
        }
        if ((config->PLLSource == RCC_PLL_SOURCE_HSE) &&
            ((source_frequency < 4000000UL) ||
             (source_frequency > 26000000UL))) {
            return DRIVER_STATUS_OUT_OF_RANGE;
        }
        if ((source_frequency == 0U) || (config->PLL_M < 2U) ||
            (config->PLL_M > 63U) || (config->PLL_N < 50U) ||
            (config->PLL_N > 432U) || (config->PLL_Q < 2U) ||
            (config->PLL_Q > 15U) ||
            ((config->PLL_P != 2U) && (config->PLL_P != 4U) &&
             (config->PLL_P != 6U) && (config->PLL_P != 8U))) {
            return DRIVER_STATUS_INVALID_ARGUMENT;
        }

        uint32_t pll_input = source_frequency / config->PLL_M;
        uint64_t vco = ((uint64_t)source_frequency * config->PLL_N) /
                       config->PLL_M;
        if ((pll_input < 1000000UL) || (pll_input > 2000000UL) ||
            (vco < 100000000ULL) || (vco > 432000000ULL)) {
            return DRIVER_STATUS_OUT_OF_RANGE;
        }
        source_frequency = (uint32_t)(vco / config->PLL_P);
    } else {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }

    uint32_t calculated_hclk = source_frequency /
                               rcc_ahb_divisor(config->AHB_Prescaler);
    if ((source_frequency > RCC_MAX_SYSCLK) ||
        (calculated_hclk > RCC_MAX_SYSCLK) ||
        ((calculated_hclk / rcc_apb_divisor(config->APB1_Prescaler)) >
         42000000UL) ||
        ((calculated_hclk / rcc_apb_divisor(config->APB2_Prescaler)) >
         84000000UL) ||
        (config->Latency < rcc_flash_latency_required(calculated_hclk))) {
        return DRIVER_STATUS_OUT_OF_RANGE;
    }

    *sysclk = source_frequency;
    *hclk = calculated_hclk;
    return DRIVER_STATUS_OK;
}

DriverStatus rcc_system_clock_config(const RCC_ClockConfigTypeDef *config)
{
    uint32_t sysclk;
    uint32_t hclk;
    DriverStatus status = rcc_validate_config(config, &sysclk, &hclk);
    if (status != DRIVER_STATUS_OK) {
        return status;
    }

    /* Move to the known-safe HSI clock before changing dividers or PLL fields. */
    status = rcc_enable_source(RCC_PLL_SOURCE_HSI);
    if (status == DRIVER_STATUS_OK) {
        status = rcc_switch_system_clock(RCC_CFGR_SW_HSI,
                                         RCC_CFGR_SWS_HSI);
    }
    if (status != DRIVER_STATUS_OK) {
        return status;
    }

    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_VOS;
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) |
                 ((uint32_t)config->Latency << FLASH_ACR_LATENCY_Pos) |
                 FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;

    RCC->CFGR = (RCC->CFGR & ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 |
                               RCC_CFGR_PPRE2)) |
                ((uint32_t)config->AHB_Prescaler << RCC_CFGR_HPRE_Pos) |
                ((uint32_t)config->APB1_Prescaler << RCC_CFGR_PPRE1_Pos) |
                ((uint32_t)config->APB2_Prescaler << RCC_CFGR_PPRE2_Pos);

    if (config->ClockSource == RCC_CLOCK_PLL) {
        status = rcc_enable_source(config->PLLSource);
        if (status != DRIVER_STATUS_OK) return status;

        RCC->CR &= ~RCC_CR_PLLON;
        status = rcc_wait_mask(&RCC->CR, RCC_CR_PLLRDY, 0U);
        if (status != DRIVER_STATUS_OK) return status;

        uint32_t pll_source_bit = (config->PLLSource == RCC_PLL_SOURCE_HSE) ?
                                  RCC_PLLCFGR_PLLSRC : 0U;
        RCC->PLLCFGR = ((uint32_t)config->PLL_M << RCC_PLLCFGR_PLLM_Pos) |
                       ((uint32_t)config->PLL_N << RCC_PLLCFGR_PLLN_Pos) |
                       ((((uint32_t)config->PLL_P / 2U) - 1U) <<
                        RCC_PLLCFGR_PLLP_Pos) |
                       ((uint32_t)config->PLL_Q << RCC_PLLCFGR_PLLQ_Pos) |
                       pll_source_bit;
        RCC->CR |= RCC_CR_PLLON;
        status = rcc_wait_mask(&RCC->CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY);
        if (status == DRIVER_STATUS_OK) {
            status = rcc_switch_system_clock(RCC_CFGR_SW_PLL,
                                             RCC_CFGR_SWS_PLL);
        }
    } else if (config->ClockSource == RCC_CLOCK_HSE) {
        status = rcc_enable_source(RCC_PLL_SOURCE_HSE);
        if (status == DRIVER_STATUS_OK) {
            status = rcc_switch_system_clock(RCC_CFGR_SW_HSE,
                                             RCC_CFGR_SWS_HSE);
        }
    } else {
        status = rcc_enable_source(RCC_PLL_SOURCE_HSI);
        if (status == DRIVER_STATUS_OK) {
            status = rcc_switch_system_clock(RCC_CFGR_SW_HSI,
                                             RCC_CFGR_SWS_HSI);
        }
    }

    if (status != DRIVER_STATUS_OK) {
        return status;
    }

    hclk_frequency = hclk;
    pclk1_frequency = hclk / rcc_apb_divisor(config->APB1_Prescaler);
    pclk2_frequency = hclk / rcc_apb_divisor(config->APB2_Prescaler);
    system_clock = hclk;
    SystemCoreClock = hclk;
    return DRIVER_STATUS_OK;
}

DriverStatus rcc_config_max_frequency(uint8_t use_hse, uint32_t hse_freq)
{
    RCC_ClockConfigTypeDef config = {
        .ClockSource = RCC_CLOCK_PLL,
        .PLLSource = use_hse ? RCC_PLL_SOURCE_HSE : RCC_PLL_SOURCE_HSI,
        .HSEFrequency = use_hse ? hse_freq : 0U,
        .PLL_M = use_hse ? (uint8_t)(hse_freq / 1000000UL) : 8U,
        .PLL_N = use_hse ? 336U : 168U,
        .PLL_P = 2U,
        .PLL_Q = 7U,
        .AHB_Prescaler = RCC_AHB_DIV1,
        .APB1_Prescaler = RCC_APB_DIV4,
        .APB2_Prescaler = RCC_APB_DIV2,
        .Latency = 5U
    };

    if (use_hse && ((hse_freq < 4000000UL) ||
                    (hse_freq > 26000000UL) ||
                    ((hse_freq % 1000000UL) != 0U))) {
        return DRIVER_STATUS_INVALID_ARGUMENT;
    }
    return rcc_system_clock_config(&config);
}

uint32_t rcc_get_pclk1_freq(void) { return pclk1_frequency; }
uint32_t rcc_get_pclk2_freq(void) { return pclk2_frequency; }

void rcc_enable_gpio_clock(GPIO_TypeDef *gpio)
{
    if (gpio == GPIOA) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    else if (gpio == GPIOB) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    else if (gpio == GPIOC) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    else if (gpio == GPIOD) RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    else if (gpio == GPIOE) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    else if (gpio == GPIOF) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
    else if (gpio == GPIOG) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    else if (gpio == GPIOH) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
    else if (gpio == GPIOI) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
}

void rcc_enable_tim_clock(TIM_TypeDef *tim)
{
    if (tim == TIM1) RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    else if (tim == TIM2) RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    else if (tim == TIM3) RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    else if (tim == TIM4) RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    else if (tim == TIM5) RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
    else if (tim == TIM6) RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    else if (tim == TIM7) RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    else if (tim == TIM8) RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
    else if (tim == TIM9) RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
    else if (tim == TIM10) RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
    else if (tim == TIM11) RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
    else if (tim == TIM12) RCC->APB1ENR |= RCC_APB1ENR_TIM12EN;
    else if (tim == TIM13) RCC->APB1ENR |= RCC_APB1ENR_TIM13EN;
    else if (tim == TIM14) RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
}

void rcc_enable_adc_clock(ADC_TypeDef *adc)
{
    if (adc == ADC1) RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    else if (adc == ADC2) RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
    else if (adc == ADC3) RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;
}

void rcc_enable_i2c_clock(I2C_TypeDef *i2c)
{
    if (i2c == I2C1) RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    else if (i2c == I2C2) RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    else if (i2c == I2C3) RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
}

void rcc_enable_dma_clock(DMA_TypeDef *dma)
{
    if (dma == DMA1) RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    else if (dma == DMA2) RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
}

void rcc_enable_usart_clock(USART_TypeDef *usart)
{
    if (usart == USART1) RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    else if (usart == USART2) RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    else if (usart == USART3) RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    else if (usart == UART4) RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
    else if (usart == UART5) RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
    else if (usart == USART6) RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
}

void rcc_enable_spi_clock(SPI_TypeDef *spi)
{
    if (spi == SPI1) RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    else if (spi == SPI2) RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    else if (spi == SPI3) RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
}
