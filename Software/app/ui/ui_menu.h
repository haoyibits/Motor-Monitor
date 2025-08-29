/**
 * @file ui_menu.h
 * @brief UI menu configuration and data structures for motor monitor interface
 * 
 * Contains all menu item definitions, window configurations, and UI layout data.
 * This file separates UI configuration from business logic.
 */

#ifndef UI_MENU_H
#define UI_MENU_H

#include "OLED_UI.h"

/* Forward declarations for callback functions */
extern void ui_motor_settings_callback(void);
extern void ui_overcurrent_protection_callback(void);
extern void ui_device_info_callback(void);
extern void ui_pwm_duty_callback(void);
extern void ui_current_limit_callback(void);
extern void ui_brightness_callback(void);

/* Motor monitor menu icons (32x32) */
extern const uint8_t Image_motor[];
extern const uint8_t Image_protection[];
extern const uint8_t Image_device_info[];

/* Main menu items configuration */
extern MenuItem MainMenuItems[];

/* Submenu items configuration */
extern MenuItem MotorSettingsItems[];
extern MenuItem ProtectionSettingsItems[];
extern MenuItem DeviceInfoItems[];

/* Interactive window configurations */
extern MenuWindow PWMDutyWindow;
extern MenuWindow CurrentLimitWindow;
extern MenuWindow BrightnessWindow;

/* Menu page configurations */
extern MenuPage MainMenuPage;
extern MenuPage MotorSettingsPage;
extern MenuPage ProtectionSettingsPage;
extern MenuPage DeviceInfoPage;

/* Global motor control variables (extern declarations) */
extern float motor_pwm_duty;
extern float motor_current_limit;
extern bool motor_enabled;
extern bool motor_direction;
extern uint8_t display_brightness;

#endif /* UI_MENU_H */