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
 * For debugging, DAP-link and SEGGER RTT are used.
 ******************************************************************************
 */

#include "OLED_UI_MenuData.h"
#include "motor.h"

#include <stdint.h>
#include <SEGGER_RTT.h>
#include <bsp.h>


int main(void)
{
    /* Initialize system with early config loading */
    system_init();
    SEGGER_RTT_printf(0, "System init...\r\n");
    gpio_write(GPIOB, 2, 1); // For led test
    
    scan_init();
    OLED_UI_Init(&MainMenuPage);
    
    /* Synchronize motor configuration with UI variables after UI system is ready */
    motor_ui_update_from_config();
    
    /* Apply loaded motor configuration (auto-start based on saved output_source) */
    //motor_apply_config();
    
    /* Main loop */
    while (1)
    {
        scan_check();
    }
}
