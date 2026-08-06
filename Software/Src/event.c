/**
 ******************************************************************************
 * @file           : event.c
 * @author         : Haoyi Chen
 * @date           : 2025-08-10
 * @brief          : User event handling implementation
 ******************************************************************************
 * @details
 * This file implements the user event system for the Motor Monitor project.
 * It provides functions for motor control initialization including GPIO
 * configuration for motor control pins and encoder setup for position
 * feedback using quadrature decoding with TIM2.
 ******************************************************************************
 */

#include "bsp.h"
#include "motor.h"
#include "app_event_queue.h"
#include "SEGGER_RTT.h"
#include "../Drivers/OLED_UI_Core/OLED_UI/OLED_UI_MenuData.h"

/* Global timer variables for periodic scanning */
SysTick_Timer_t encoder_timer;      // Timer for encoder position/speed monitoring
SysTick_Timer_t current_timer;      // Timer for current monitoring
Encoder_HandleTypeDef motor_encoder; // Global encoder handle for system-wide access

/* Global button variables for system control */
Button_HandleTypeDef button_up;      // UP button (PE9)
Button_HandleTypeDef button_down;    // DOWN button (PE10)  
Button_HandleTypeDef button_enter;   // ENTER button (PE12)
Button_HandleTypeDef button_return;  // RETURN button (PE11)
Button_Manager_t button_manager;     // Button manager for efficient scanning
Button_HandleTypeDef *button_array[] = {&button_up, &button_down, &button_enter, &button_return}; // Button array for manager

static App_Event_Queue_t app_event_queue;

static void motor_notification_handler(Motor_Notification_t notification)
{
    App_Event_Type_t event_type = APP_EVENT_NONE;

    switch (notification) {
        case MOTOR_NOTIFICATION_OVERCURRENT:
            event_type = APP_EVENT_MOTOR_OVERCURRENT;
            break;
        case MOTOR_NOTIFICATION_RESTARTING:
            event_type = APP_EVENT_MOTOR_RESTARTING;
            break;
        case MOTOR_NOTIFICATION_MAX_RESTARTS:
            event_type = APP_EVENT_MOTOR_MAX_RESTARTS;
            break;
        default:
            break;
    }

    if (event_type != APP_EVENT_NONE) {
        (void)app_event_queue_push(&app_event_queue, (App_Event_t){.type = event_type});
    }
}

/**
 * @brief Initialize button system for user interface
 * 
 * @details This function configures all user interface buttons:
 *          1. Button UP (PE9): Used for increasing values or navigating up
 *          2. Button DOWN (PE10): Used for decreasing values or navigating down
 *          3. Button ENTER (PE12): Used for confirming selections or start/stop motor
 *          4. Button RETURN (PE11): Used for cancelling actions or emergency stop
 *          
 * @note All buttons are configured as active-low with internal pull-ups enabled
 *       for reliable operation without external resistors.
 * 
 * @warning This function assumes RCC clock is already enabled for GPIOE
 */
void button_system_init(void)
{
    /* Initialize button system for user control */
    // Button UP: PE9 (active low with pull-up)
    Button_InitTypeDef button_up_config = {
        .GPIOx = BUTTON_UP_PORT,
        .pin = BUTTON_UP_PIN,
        .active_level = BUTTON_ACTIVE_LOW,
        .pullup_enable = 1
    };
    
    // Button DOWN: PE10 (active low with pull-up)  
    Button_InitTypeDef button_down_config = {
        .GPIOx = BUTTON_DOWN_PORT,
        .pin = BUTTON_DOWN_PIN,
        .active_level = BUTTON_ACTIVE_LOW,
        .pullup_enable = 1
    };

    // Button RETURN: PE11 (active low with pull-up)
    Button_InitTypeDef button_return_config = {
        .GPIOx = BUTTON_RETURN_PORT,
        .pin = BUTTON_RETURN_PIN,
        .active_level = BUTTON_ACTIVE_LOW,
        .pullup_enable = 1
    };
    
    // Button ENTER: PE12 (active low with pull-up)
    Button_InitTypeDef button_enter_config = {
        .GPIOx = BUTTON_ENTER_PORT,
        .pin = BUTTON_ENTER_PIN,
        .active_level = BUTTON_ACTIVE_LOW,
        .pullup_enable = 1
    };
    
    /* Initialize individual buttons */
    button_init(&button_up, &button_up_config);
    button_init(&button_down, &button_down_config);
    button_init(&button_enter, &button_enter_config);
    button_init(&button_return, &button_return_config);
    
    button_manager_init(&button_manager, button_array, 4);
}


/**
 * @brief Process button state changes and handle button events
 * 
 * @details This function performs two main operations:
 *          1. Checks all buttons in the button manager when the scan timer expires
 *          2. Processes button press events to perform corresponding actions:
 *             - UP button: Could be used for navigation or increasing values
 *             - DOWN button: Could be used for navigation or decreasing values
 *             - ENTER button: Toggles motor operation (start/stop)
 *             - RETURN button: Performs emergency stop by disabling motor
 * 
 * @note This function should be called periodically in the main loop
 */
void button_handler(void)
{
    /* Check if shared timer has expired */
    if (systick_timer_expired(&button_manager.scan_timer)) {
        /* Scan all buttons in one timer cycle - much more efficient */
        for (uint8_t i = 0; i < button_manager.button_count; i++) {
            Button_HandleTypeDef *handle = button_manager.buttons[i];
            
            if (handle != NULL && handle->initialized) {
                /* Read raw state and apply optimized debouncing */
                uint8_t raw_state = button_read_raw(handle);
                button_debounce_shift_register(handle, raw_state);
            }
        }
    }
    /* Handle button events for motor control */
    // UP Button (PE9): Increase motor speed or navigate up
    if (button_pressed(&button_up)) {
        SEGGER_RTT_printf(0, "UP button pressed\r\n");
        // Add your UP button functionality here
        // Example: increase speed, navigate menu up, etc.
    }
    
    // DOWN Button (PE10): Decrease motor speed or navigate down
    if (button_pressed(&button_down)) {
        SEGGER_RTT_printf(0, "DOWN button pressed\r\n");
        // Add your DOWN button functionality here
        // Example: decrease speed, navigate menu down, etc.
    }
    
    // ENTER Button (PE12): Confirm selection or start/stop motor
    if (button_pressed(&button_enter)) {
        SEGGER_RTT_printf(0, "ENTER button pressed\r\n");
        
    }
    
    // RETURN Button (PE11): Go back or emergency stop
    if (button_pressed(&button_return)) {
        SEGGER_RTT_printf(0, "RETURN button pressed\r\n");
        
    }
}

/**
 * @brief Process encoder feedback data and calculate speed
 * 
 * @details This function monitors encoder position and calculates motor speed:
 *          1. Checks if the encoder timer period has elapsed
 *          2. Reads current encoder position (total count)
 *          3. Calculates motor speed in RPM based on encoder counts
 *          4. Outputs debug information via SEGGER RTT
 * 
 * @note This function is called periodically by scan_check()
 *       and uses the global encoder_timer to control update frequency
 */
void encoder_handler(void)
{
    (void)encoder_calculate_speed_rpm(&motor_encoder, systick_get_ms());
}

/**
 * @brief Monitor motor current with startup-aware protection and auto-restart
 * 
 * @details This function implements intelligent current monitoring with motor state awareness:
 *          1. Tracks motor startup vs running states with different current thresholds
 *          2. Allows higher current during startup phase (inrush current protection)
 *          3. Applies strict current limits during normal operation
 *          4. Uses consecutive overcurrent detection to avoid false triggers
 *          5. Automatic state transition from startup to running after timeout
 *          6. Integrates with new auto-restart protection system
 * 
 * @note Current thresholds:
 *       - Startup: CURRENT_STARTUP_THRESHOLD (3700) - allows motor inrush current
 *       - Running: CURRENT_RUNNING_THRESHOLD (3000) - strict protection
 */
void current_handler(void)
{
    if (current_adcDmaError != 0U) {
        current_adcDmaError = 0U;
        if (motor_config.overcurrent_protection != 0U) {
            motor_emergency_shutdown();
        }
        return;
    }

    if (current_adcAverageReady) {
            /* Claim this notification first; an ISR during processing posts the next one. */
            current_adcAverageReady = 0U;

            /* Update motor state and threshold based on current conditions */
            motor_current_state_update();
            
            /* Update motor protection system with auto-restart logic */
            motor_protection_update();
            
            /* Keep the consecutive-sample safeguard aligned with the enable flag. */
            if (motor_config.overcurrent_protection &&
                current_adcAverage > motor_monitor.current_threshold) {
                motor_monitor.overcurrent_count++;
                //SEGGER_RTT_printf(0, "Overcurrent detected: %d (threshold: %d) count: %d\r\n", 
                //                 current_adcAverage, motor_monitor.current_threshold, motor_monitor.overcurrent_count);
                
                /* Emergency shutdown after consecutive overcurrent detections */
                if (motor_monitor.overcurrent_count >= OVERCURRENT_COUNT_LIMIT) {
                    motor_emergency_shutdown();
                //    SEGGER_RTT_printf(0, "EMERGENCY SHUTDOWN - Overcurrent protection triggered!\r\n");
                }
            } else {
                /* Reset overcurrent counter if current is within limits */
                motor_monitor.overcurrent_count = 0;
            }
            
            /* Debug output for monitoring */
            if (motor_monitor.state != MOTOR_STATE_STOPPED) {
                //SEGGER_RTT_printf(0, "Motor State: %d, Current: %d, Threshold: %d\r\n", 
                //                 motor_monitor.state, current_adcAverage, motor_monitor.current_threshold);
            }
            
    }
}

void oled_handler(void)
{
    OLED_UI_InterruptHandler();
    OLED_UI_MainLoop();
}


/**
 * @brief Initialize all system scanning timers
 * 
 * @details This function initializes the timer system for periodic scanning:
 *          1. Encoder scanning timer (100ms period, auto-reload)
 *          2. Current monitoring timer (1ms period, auto-reload)
 *          3. Button scanning timer (5ms period, auto-reload) for shared button manager
 *          
 * @note These timers control the periodic execution of handler functions
 *       which are called by scan_check() in the main loop
 */
void scan_init(void)
{
    app_event_queue_init(&app_event_queue);
    motor_set_notification_callback(motor_notification_handler);

    /* Initialize encoder timer for periodic scanning */
    systick_timer_init(&encoder_timer, 100, 1);          // 100ms auto-reload timer
    systick_timer_start(&encoder_timer);                // Start timer for periodic updates

    /*Initialize current timer*/
    systick_timer_init(&current_timer, 1, 1);          // 1ms auto-reload timer
    systick_timer_start(&current_timer);                // Start timer


    /* Initialize shared timer for all buttons */
    systick_timer_init(&button_manager.scan_timer, 5, 1);
    systick_timer_start(&button_manager.scan_timer);

}

/**
 * @brief Main system scanning function to handle all periodic tasks
 * 
 * @details This function serves as the central control point for all periodic tasks:
 *          1. Calls encoder_handler() to monitor encoder position and speed
 *          2. Calls current_handler() to monitor motor current and perform safety checks
 *          3. Calls button_handler() to process user button inputs
 * 
 * @note This function should be called repeatedly in the main loop
 *       Each handler has its own timer and will only execute when its timer expires
 */
void scan_check(void)
{
    /* Refresh debounced input before the UI consumes button state. */
    button_handler();

    if (systick_timer_expired(&current_timer) &&
        ((current_adcAverageReady != 0U) || (current_adcDmaError != 0U))) {
        (void)app_event_queue_push(&app_event_queue,
                                   (App_Event_t){.type = APP_EVENT_CURRENT_SAMPLE});
    }
    if (systick_timer_expired(&encoder_timer)) {
        (void)app_event_queue_push(&app_event_queue,
                                   (App_Event_t){.type = APP_EVENT_ENCODER_SAMPLE});
    }
    if (systick_timer_expired(&oledui_timer)) {
        (void)app_event_queue_push(&app_event_queue,
                                   (App_Event_t){.type = APP_EVENT_UI_TICK});
    }

    App_Event_t event;
    while (app_event_queue_pop(&app_event_queue, &event)) {
        switch (event.type) {
            case APP_EVENT_ENCODER_SAMPLE:
                encoder_handler();
                break;
            case APP_EVENT_CURRENT_SAMPLE:
                current_handler();
                break;
            case APP_EVENT_UI_TICK:
                oled_handler();
                break;
            case APP_EVENT_MOTOR_OVERCURRENT:
                ShowOvercurrentFaultPopup();
                break;
            case APP_EVENT_MOTOR_RESTARTING:
                ShowMotorRestartPopup();
                break;
            case APP_EVENT_MOTOR_MAX_RESTARTS:
                ShowMaxAttemptsReachedPopup();
                break;
            case APP_EVENT_NONE:
            default:
                break;
        }
    }
}

/**
 * @brief Early motor configuration loading during system startup
 * 
 * @details This function loads motor configuration from SPI Flash early in the boot process.
 *          It should be called from main() before other system initialization.
 */
void motor_config_early_load(void)
{
    /* Initialize SPI Flash system for parameter storage */
    SEGGER_RTT_printf(0, "Early motor config loading...\r\n");
    
    /* Initialize SPI Flash hardware first */
    spi_flash_system_init();
    
    /* Load motor configuration from flash */
    motor_config_init();
    
    SEGGER_RTT_printf(0, "Motor config early load completed\r\n");
}
