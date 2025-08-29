/**
 * @file ui_callback.c  
 * @brief UI callback function implementations for motor monitor interface
 * 
 * Contains all UI business logic and callback function implementations.
 * Handles interaction between UI events and application logic.
 */

#include "ui_callback.h"
#include "ui_menu.h"

/* Button event type definitions */
#define BUTTON_EVENT_PRESS      0
#define BUTTON_EVENT_RELEASE    1
#define BUTTON_EVENT_LONG_PRESS 2

/* Button ID definitions */
#define BUTTON_UP_ID            0
#define BUTTON_DOWN_ID          1
#define BUTTON_ENTER_ID         2
#define BUTTON_BACK_ID          3

/**
 * @brief Main menu callback - Motor Settings
 */
void ui_motor_settings_callback(void)
{
    /* Navigate to motor settings page */
    OLED_UI_LoadPage(&MotorSettingsPage);
}

/**
 * @brief Main menu callback - Overcurrent Protection  
 */
void ui_overcurrent_protection_callback(void)
{
    /* Navigate to protection settings page */
    OLED_UI_LoadPage(&ProtectionSettingsPage);
}

/**
 * @brief Main menu callback - Device Info
 */
void ui_device_info_callback(void)
{
    /* Navigate to device info page */
    OLED_UI_LoadPage(&DeviceInfoPage);
}

/**
 * @brief Motor settings callback - PWM Duty Cycle
 */
void ui_pwm_duty_callback(void)
{
    /* Open PWM duty cycle adjustment window */
    OLED_UI_OpenWindow(&PWMDutyWindow);
}

/**
 * @brief Protection settings callback - Current Limit
 */
void ui_current_limit_callback(void)
{
    /* Open current limit adjustment window */
    OLED_UI_OpenWindow(&CurrentLimitWindow);
}

/**
 * @brief Device info callback - Display Brightness
 */
void ui_brightness_callback(void)
{
    /* Open brightness adjustment window */
    OLED_UI_OpenWindow(&BrightnessWindow);
}

/**
 * @brief UI initialization function
 */
void ui_init(void)
{
    /* Initialize OLED hardware driver */
    OLED_UI_Driver_Init();
    
    /* Initialize UI core system */
    OLED_UI_Init();
    
    /* Load main menu page */
    OLED_UI_LoadPage(&MainMenuPage);
    
    /* Apply initial motor settings */
    ui_apply_motor_settings();
}

/**
 * @brief UI main loop function
 */
void ui_update(void)
{
    /* Update UI core - handles animations, rendering, input processing */
    OLED_UI_Update();
    
    /* Apply any changed settings to motor control */
    ui_apply_motor_settings();
    
    /* Update UI with current motor status */
    ui_update_motor_status();
}

/**
 * @brief Button event handler for UI
 */
void ui_button_event(uint8_t button_id, uint8_t event_type)
{
    /* Only process button press events for UI navigation */
    if (event_type != BUTTON_EVENT_PRESS) {
        return;
    }
    
    switch (button_id) {
        case BUTTON_UP_ID:
            OLED_UI_ButtonEvent(OLED_UI_BUTTON_UP);
            break;
            
        case BUTTON_DOWN_ID:
            OLED_UI_ButtonEvent(OLED_UI_BUTTON_DOWN);
            break;
            
        case BUTTON_ENTER_ID:
            OLED_UI_ButtonEvent(OLED_UI_BUTTON_ENTER);
            break;
            
        case BUTTON_BACK_ID:
            OLED_UI_ButtonEvent(OLED_UI_BUTTON_BACK);
            break;
            
        default:
            break;
    }
}

/**
 * @brief Apply motor control changes from UI
 */
void ui_apply_motor_settings(void)
{
    static float last_pwm_duty = 0.0f;
    static float last_current_limit = 0.0f;
    static bool last_motor_enabled = false;
    static bool last_motor_direction = false;
    static uint8_t last_brightness = 0;
    
    /* Check if PWM duty cycle changed */
    if (motor_pwm_duty != last_pwm_duty) {
        /* Apply PWM duty cycle to motor control */
        motor_set_pwm_duty(motor_pwm_duty);
        last_pwm_duty = motor_pwm_duty;
    }
    
    /* Check if current limit changed */
    if (motor_current_limit != last_current_limit) {
        /* Apply current limit to protection system */
        motor_set_current_limit(motor_current_limit);
        last_current_limit = motor_current_limit;
    }
    
    /* Check if motor enable state changed */
    if (motor_enabled != last_motor_enabled) {
        /* Apply motor enable/disable */
        motor_set_enable(motor_enabled);
        last_motor_enabled = motor_enabled;
    }
    
    /* Check if motor direction changed */
    if (motor_direction != last_motor_direction) {
        /* Apply motor direction */
        motor_set_direction(motor_direction);
        last_motor_direction = motor_direction;
    }
    
    /* Check if display brightness changed */
    if (display_brightness != last_brightness) {
        /* Apply brightness to OLED display */
        OLED_UI_SetBrightness(display_brightness);
        last_brightness = display_brightness;
    }
}

/**
 * @brief Update UI with current motor status
 */
void ui_update_motor_status(void)
{
    /* Get current motor status from motor control system */
    float current_reading = motor_get_current_reading();
    uint32_t encoder_position = motor_get_encoder_position();
    float motor_speed = motor_get_speed();
    
    /* Update UI status indicators if needed */
    /* This would typically update status bars, indicators, etc. */
    /* Implementation depends on specific UI library capabilities */
    
    /* Example: Update current reading display */
    if (current_reading > motor_current_limit * 0.9f) {
        /* Show warning indication if approaching current limit */
        /* Implementation specific to UI library */
    }
}