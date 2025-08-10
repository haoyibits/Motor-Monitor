/**
******************************************************************************
 * @file           : main.c
 * @author         : Haoyi Chen
 * @date           : 2025-08-01
 * @brief          : Entry point of the STM32 application (bare-metal style)
 ******************************************************************************
 * @details
 * This file contains the main loop of a minimal STM32 project.
 * No HAL or LL library is used. Only pure C with register-level programming.
 *
 * Created for personal learning and embedded systems experimentation.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include <SEGGER_RTT.h>
#include <bsp.h>


int main(void)
{
    /* Initialize system */
    system_init();
    SEGGER_RTT_printf(0, "System init...\r\n");
    
    gpio_write(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, 1);
    gpio_write(MOTOR_P_PORT, MOTOR_P_PIN, 1);
    gpio_write(MOTOR_M_PORT, MOTOR_M_PIN, 0);
    gpio_write(GPIOB,2, 1);
    
    /* Main loop */
    while (1)
    {
        /* Check if new ADC data is ready */
        if (current_adcAverageReady)
        {
            sum = 0;  // Reset sum before calculating new average
            for (int i = 0; i < 200; i++) 
                sum += current_adcBuffer[i];
            current_adcAverage = sum / 200;  // Calculate average
            if (current_adcAverage > CURRENT_CRITICAL_THRESHOLD) {
                gpio_write(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, 0); // Disable motor
            }
            current_adcAverageReady = 0;
        }
    }
}