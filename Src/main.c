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
    gpio_write(GPIOB,2, 1);
    motor_init();
    scan_init();
    /* Main loop */
    while (1)
    {
        scan_check();
    }
}