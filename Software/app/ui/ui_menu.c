/**
 * @file ui_menu.c
 * @brief UI menu configuration implementation
 * 
 * Contains all menu data structures, icons, and UI layout configurations
 * for the motor monitor interface.
 */

#include "ui_menu.h"

/* Motor control variables - defined here, declared extern in ui_menu.h */
float motor_pwm_duty = 50.0f;      /* PWM duty cycle (0-100%) */
float motor_current_limit = 2.0f;   /* Current limit (A) */
bool motor_enabled = false;         /* Motor enable state */
bool motor_direction = false;       /* Motor direction (CW/CCW) */
uint8_t display_brightness = 128;   /* Display brightness (5-255) */

/* Motor icon (32x32) - Circular motor with center shaft */
const uint8_t Image_motor[] = {
    /* Icon bitmap data - placeholder for actual motor icon */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* ... additional bytes for 32x32 icon ... */
};

/* Protection icon (32x32) - Shield with electrical elements */
const uint8_t Image_protection[] = {
    /* Icon bitmap data - placeholder for actual protection icon */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* ... additional bytes for 32x32 icon ... */
};

/* Device info icon (32x32) - Chip outline with info indicators */
const uint8_t Image_device_info[] = {
    /* Icon bitmap data - placeholder for actual device info icon */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* ... additional bytes for 32x32 icon ... */
};

/* Main Menu Items Configuration */
MenuItem MainMenuItems[] = {
    {
        .General_item_text = "Motor Settings",
        .Tiles_Icon = Image_motor,
        .General_OnClickFunc = ui_motor_settings_callback
    },
    {
        .General_item_text = "Overcurrent Protection", 
        .Tiles_Icon = Image_protection,
        .General_OnClickFunc = ui_overcurrent_protection_callback
    },
    {
        .General_item_text = "Device Info",
        .Tiles_Icon = Image_device_info,
        .General_OnClickFunc = ui_device_info_callback
    },
    {
        .General_item_text = NULL  /* Terminator */
    }
};

/* Motor Settings Submenu Items */
MenuItem MotorSettingsItems[] = {
    {
        .General_item_text = "PWM Duty Cycle",
        .General_OnClickFunc = ui_pwm_duty_callback
    },
    {
        .General_item_text = "Motor Enable",
        .General_item_RadioButton_Data = &motor_enabled,
        .General_item_RadioButton_DefaultValue = false
    },
    {
        .General_item_text = "Motor Direction", 
        .General_item_RadioButton_Data = &motor_direction,
        .General_item_RadioButton_DefaultValue = false
    },
    {
        .General_item_text = NULL  /* Terminator */
    }
};

/* Protection Settings Submenu Items */
MenuItem ProtectionSettingsItems[] = {
    {
        .General_item_text = "Current Limit",
        .General_OnClickFunc = ui_current_limit_callback
    },
    {
        .General_item_text = "Protection Enable",
        .General_item_RadioButton_Data = &motor_enabled,  /* Reuse for simplicity */
        .General_item_RadioButton_DefaultValue = true
    },
    {
        .General_item_text = NULL  /* Terminator */
    }
};

/* Device Info Submenu Items */
MenuItem DeviceInfoItems[] = {
    {
        .General_item_text = "Display Brightness",
        .General_OnClickFunc = ui_brightness_callback
    },
    {
        .General_item_text = "MCU: STM32F407VGT6",
        .General_OnClickFunc = NULL  /* Info only, no action */
    },
    {
        .General_item_text = "Firmware: v1.0.0",
        .General_OnClickFunc = NULL  /* Info only, no action */
    },
    {
        .General_item_text = NULL  /* Terminator */
    }
};

/* Interactive Window Configurations */

/* PWM Duty Cycle Adjustment Window */
MenuWindow PWMDutyWindow = {
    .General_Width = 80,
    .General_Height = 28,
    .Text_String = "PWM Duty %",
    .Text_FontSize = OLED_UI_FONT_12,
    .General_WindowType = WINDOW_ROUNDRECTANGLE,
    .Prob_Data_Float = &motor_pwm_duty,
    .Prob_DataStep = 1.0f,
    .Prob_MinData = 0.0f,
    .Prob_MaxData = 100.0f
};

/* Current Limit Adjustment Window */
MenuWindow CurrentLimitWindow = {
    .General_Width = 80,
    .General_Height = 28,
    .Text_String = "Current Limit A",
    .Text_FontSize = OLED_UI_FONT_12,
    .General_WindowType = WINDOW_ROUNDRECTANGLE,
    .Prob_Data_Float = &motor_current_limit,
    .Prob_DataStep = 0.1f,
    .Prob_MinData = 0.1f,
    .Prob_MaxData = 5.0f
};

/* Display Brightness Adjustment Window */
MenuWindow BrightnessWindow = {
    .General_Width = 80,
    .General_Height = 28,
    .Text_String = "Brightness",
    .Text_FontSize = OLED_UI_FONT_12,
    .General_WindowType = WINDOW_ROUNDRECTANGLE,
    .Prob_Data_Int = (int*)&display_brightness,
    .Prob_DataStep_Int = 10,
    .Prob_MinData = 5,
    .Prob_MaxData = 255
};

/* Menu Page Configurations */

/* Main Menu Page (Tile-based) */
MenuPage MainMenuPage = {
    .Menu_Title = "Motor Monitor",
    .MenuItems = MainMenuItems,
    .General_BackgroundColor = 0,  /* Black background */
    .General_ForegroundColor = 1   /* White foreground */
};

/* Motor Settings Page (List-based) */
MenuPage MotorSettingsPage = {
    .Menu_Title = "Motor Settings",
    .MenuItems = MotorSettingsItems,
    .General_BackgroundColor = 0,  /* Black background */
    .General_ForegroundColor = 1   /* White foreground */
};

/* Protection Settings Page (List-based) */
MenuPage ProtectionSettingsPage = {
    .Menu_Title = "Protection Settings",
    .MenuItems = ProtectionSettingsItems,
    .General_BackgroundColor = 0,  /* Black background */
    .General_ForegroundColor = 1   /* White foreground */
};

/* Device Info Page (List-based) */
MenuPage DeviceInfoPage = {
    .Menu_Title = "Device Info",
    .MenuItems = DeviceInfoItems,
    .General_BackgroundColor = 0,  /* Black background */
    .General_ForegroundColor = 1   /* White foreground */
};