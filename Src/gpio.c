/**
 * @file gpio.c
 * @author Haoyi Chen
 * @date 2025-08-03
 * @brief STM32F4 GPIO register-level driver
 *
 * @details This file provides basic GPIO initialization, read, write, and toggle functions
 * for STM32F407 series microcontrollers. All operations are performed at the
 * register level, without using HAL or LL libraries.
 *
 * Created for personal learning and embedded systems experimentation.
 */

#include "gpio.h"

/**
 * @brief Initialize a GPIO pin's mode, output type, speed, and pull-up/pull-down
 * 
 * @details Configures GPIO pin by setting MODER, OTYPER, OSPEEDR and PUPDR registers.
 *          Clears relevant bits first, then sets requested values.
 * 
 * @param GPIOx Pointer to GPIO port (e.g., GPIOA, GPIOB)
 * @param pin Pin number (0-15)
 * @param mode Pin mode (GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, etc.)
 * @param otype Output type (GPIO_OTYPE_PP, GPIO_OTYPE_OD)
 * @param speed Output speed (GPIO_SPEED_LOW, etc.)
 * @param pupd Pull-up/pull-down (GPIO_NOPULL, GPIO_PULLUP, GPIO_PULLDOWN)
 * 
 * @note GPIO clock must be enabled separately via RCC_AHB1ENR
 */
void gpio_init(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t mode, uint8_t otype, uint8_t speed, uint8_t pupd) {
    // Set mode
    GPIOx->MODER &= ~(0x3 << (pin * 2));
    GPIOx->MODER |= (mode << (pin * 2));
    // Set output type
    GPIOx->OTYPER &= ~(0x1 << pin);
    GPIOx->OTYPER |= (otype << pin);
    // Set speed
    GPIOx->OSPEEDR &= ~(0x3 << (pin * 2));
    GPIOx->OSPEEDR |= (speed << (pin * 2));
    // Set pull-up/pull-down
    GPIOx->PUPDR &= ~(0x3 << (pin * 2));
    GPIOx->PUPDR |= (pupd << (pin * 2));
}

/**
 * @brief Set a GPIO pin to high or low level
 * 
 * @details Uses BSRR register for atomic bit manipulation without read-modify-write.
 *          Lower 16 bits (0-15) set pins high, upper bits (16-31) set pins low.
 *
 * @param GPIOx Pointer to GPIO port
 * @param pin Pin number (0-15)
 * @param value Pin value (1=high, 0=low)
 */
void gpio_write(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t value) {
    if (value) {
        GPIOx->BSRR = (1 << pin); // Set pin
    } else {
        GPIOx->BSRR = (1 << (pin + 16)); // Reset pin
    }
}

/**
 * @brief Read the current level of a GPIO pin
 * 
 * @details Reads IDR register and masks the specific pin bit.
 *
 * @param GPIOx Pointer to GPIO port
 * @param pin Pin number (0-15)
 * @return uint8_t 1 if high, 0 if low
 */
uint8_t gpio_read(GPIO_TypeDef *GPIOx, uint8_t pin) {
    return (GPIOx->IDR & (1 << pin)) ? 1 : 0;
}

/**
 * @brief Toggle the output level of a GPIO pin
 * 
 * @details Reads current pin state and writes the opposite value.
 *
 * @param GPIOx Pointer to GPIO port
 * @param pin Pin number (0-15)
 */
void gpio_toggle(GPIO_TypeDef *GPIOx, uint8_t pin) {
    if (gpio_read(GPIOx, pin)) {
        gpio_write(GPIOx, pin, 0);
    } else {
        gpio_write(GPIOx, pin, 1);
    }
}

/**
 * @brief Configure GPIO pin as alternate function
 * 
 * @details Sets GPIO pin alternate function by configuring the appropriate AFR register.
 *          AFR[0] is used for pins 0-7, AFR[1] for pins 8-15.
 *
 * @param GPIOx Pointer to GPIO port
 * @param pin Pin number (0-15)
 * @param alternate Alternate function number (GPIO_AF_xxx)
 * 
 * @note Before calling this function, ensure GPIO clock is enabled and
 *       pin mode is set to GPIO_MODE_AF using gpio_init()
 */
void gpio_set_af(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t alternate) {
    /* Configure alternate function mode */
    if (pin < 8) {
        /* Configure AFR[0] for pins 0-7 */
        GPIOx->AFR[0] &= ~(0xF << (pin * 4)); /* Clear the current AF */
        GPIOx->AFR[0] |= (alternate << (pin * 4));
    } else {
        /* Configure AFR[1] for pins 8-15 */
        GPIOx->AFR[1] &= ~(0xF << ((pin - 8) * 4)); /* Clear the current AF */
        GPIOx->AFR[1] |= (alternate << ((pin - 8) * 4));
    }
}

/**
 * @brief Configure GPIO pin for external interrupt
 * 
 * @details Sets up a GPIO pin to trigger external interrupts by:
 *          1. Configuring SYSCFG EXTICR registers to map GPIO pin to EXTI line
 *          2. Setting up EXTI trigger configuration (rising/falling/both edges)
 *          3. Enabling EXTI line interrupts in EXTI_IMR
 *          4. Configuring NVIC priority and enabling interrupt
 *
 * @param GPIOx Pointer to GPIO port
 * @param pin Pin number (0-15)
 * @param trigger_type Interrupt trigger type (GPIO_INT_RISING, GPIO_INT_FALLING, or GPIO_INT_BOTH)
 * @param priority Interrupt priority (0-15, 0 is highest)
 * 
 * @note SYSCFG clock must be enabled via RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN
 */
void gpio_configure_interrupt(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t trigger_type, uint8_t priority) {
    /* Enable SYSCFG clock if not already enabled */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    
    /* Get the port index for SYSCFG EXTICR register configuration */
    uint8_t port_index = 0;
    if (GPIOx == GPIOA) port_index = 0;
    else if (GPIOx == GPIOB) port_index = 1;
    else if (GPIOx == GPIOC) port_index = 2;
    else if (GPIOx == GPIOD) port_index = 3;
    else if (GPIOx == GPIOE) port_index = 4;
    else if (GPIOx == GPIOF) port_index = 5;
    else if (GPIOx == GPIOG) port_index = 6;
    else if (GPIOx == GPIOH) port_index = 7;
    else if (GPIOx == GPIOI) port_index = 8;
    
    /* Configure EXTI line source */
    uint8_t reg_index = pin / 4;
    uint8_t bit_position = (pin % 4) * 4;
    
    SYSCFG->EXTICR[reg_index] &= ~(0xF << bit_position);
    SYSCFG->EXTICR[reg_index] |= (port_index << bit_position);
    
    /* Configure EXTI trigger type */
    if (trigger_type & GPIO_INT_RISING) {
        EXTI->RTSR |= (1U << pin);  /* Enable rising edge trigger */
    } else {
        EXTI->RTSR &= ~(1U << pin); /* Disable rising edge trigger */
    }
    
    if (trigger_type & GPIO_INT_FALLING) {
        EXTI->FTSR |= (1U << pin);  /* Enable falling edge trigger */
    } else {
        EXTI->FTSR &= ~(1U << pin); /* Disable falling edge trigger */
    }
    
    /* Enable EXTI line */
    EXTI->IMR |= (1U << pin);
    
    /* Configure NVIC */
    IRQn_Type irq_num;
    
    if (pin == 0)       irq_num = EXTI0_IRQn;
    else if (pin == 1)  irq_num = EXTI1_IRQn;
    else if (pin == 2)  irq_num = EXTI2_IRQn;
    else if (pin == 3)  irq_num = EXTI3_IRQn;
    else if (pin == 4)  irq_num = EXTI4_IRQn;
    else if (pin <= 9)  irq_num = EXTI9_5_IRQn;
    else                irq_num = EXTI15_10_IRQn;
    
    /* Set priority and enable the interrupt */
    NVIC_SetPriority(irq_num, priority);
    NVIC_EnableIRQ(irq_num);
}

/**
 * @brief Initialize a GPIO pin as input with external interrupt
 * 
 * @details Combines gpio_init() and gpio_configure_interrupt() to:
 *          1. Configure pin as input with specified pull-up/pull-down
 *          2. Set up external interrupt with specified trigger type
 *
 * @param GPIOx Pointer to GPIO port
 * @param pin Pin number (0-15)
 * @param pupd Pull-up/pull-down (GPIO_NOPULL, GPIO_PULLUP, GPIO_PULLDOWN)
 * @param trigger_type Interrupt trigger type (GPIO_INT_RISING, GPIO_INT_FALLING, or GPIO_INT_BOTH)
 * @param priority Interrupt priority (0-15, 0 is highest)
 * 
 * @note This is a convenience function that combines input configuration and interrupt setup
 */
void gpio_init_input_with_interrupt(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t pupd, 
                                   uint8_t trigger_type, uint8_t priority) {
    /* First configure the GPIO pin as input with specified pull-up/pull-down */
    gpio_init(GPIOx, pin, GPIO_MODE_INPUT, 0, 0, pupd);
    
    /* Then configure the interrupt */
    gpio_configure_interrupt(GPIOx, pin, trigger_type, priority);
}
