/**
 * @file system_includes.h
 * @brief Centralized header file management for the motor monitor project
 * 
 * This file includes all necessary headers for the project, providing a single
 * entry point for all source files. This simplifies dependency management and
 * avoids duplicate includes.
 */

#ifndef SYSTEM_INCLUDES_H
#define SYSTEM_INCLUDES_H

/* CMSIS Standard Headers */
#include "stm32f4xx.h"
#include "core_cm4.h"
#include "SEGGER_RTT.h"

/* MCU Peripheral Drivers */
#include "rcc.h"
#include "gpio.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "uart.h"
#include "systick.h"
#include "i2c.h"
#include "spi.h"

/* External Device Drivers */
#include "ssd1309.h"
#include "w25q128.h"
#include "encoder.h"
#include "button.h"

/* UI Core Headers */
#include "OLED_UI_Driver.h"
#include "OLED_driver.h"
#include "OLED.h"
#include "OLED_Fonts.h"
#include "OLED_UI.h"
#include "OLED_UI_MenuData.h"

/* Application Layer Headers */
#include "bsp.h"
#include "event.h"
#include "irq.h"
#include "motor.h"

/* Board Configuration */
#include "board_config.h"

/* Standard C Library Headers */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#endif /* SYSTEM_INCLUDES_H */