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
 *
 * This file is part of a bare-metal STM32F407VGT6 project.
 ******************************************************************************
 */

#include "bsp.h"
#include "SEGGER_RTT.h"

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

/**
 * @brief Initialize motor control system
 * 
 * @details This function performs complete motor system initialization:
 *          1. Configures motor control GPIO pins as outputs
 *          2. Sets initial motor states (enabled, forward direction)  
 *          3. Configures encoder GPIO pins for TIM2 input capture
 *          4. Initializes encoder with quadrature decoding
 *          5. Starts encoder counting for position feedback
 * 
 * @note Motor control pins:
 *       - PB0 (MOTOR_P_PIN): Motor positive control
 *       - PB1 (MOTOR_M_PIN): Motor negative control  
 *       - PE7 (MOTOR_ENABLE_PIN): Motor enable control
 *       - PB2: Additional GPIO output
 * 
 * @note Encoder pins:
 *       - PA2 (ENCODER_CH1_PIN): Encoder A phase input (TIM2_CH1)
 *       - PA3 (ENCODER_CH2_PIN): Encoder B phase input (TIM2_CH2)
 * 
 * @warning This function assumes RCC clocks are already enabled for:
 *          GPIOA, GPIOB, GPIOE, and TIM2 peripherals
 */
void motor_init(void)
{
    /* Configure motor control pins as GPIO outputs */
    gpio_init(GPIOB, 2, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_SPEED_MED, GPIO_NOPULL);    // Additional GPIO output
    gpio_init(MOTOR_P_PORT, MOTOR_P_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_SPEED_MED, GPIO_NOPULL);        // Motor P control
    gpio_init(MOTOR_M_PORT, MOTOR_M_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_SPEED_MED, GPIO_NOPULL);        // Motor M control
    gpio_init(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_SPEED_MED, GPIO_NOPULL); // Motor enable

    /* Configure motor pin initial states */
    gpio_write(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, 1);    // Enable motor driver
    gpio_write(MOTOR_P_PORT, MOTOR_P_PIN, 1);              // Set forward direction (P=1, M=0)
    gpio_write(MOTOR_M_PORT, MOTOR_M_PIN, 0);              // Set forward direction (P=1, M=0)

    /* Configure encoder GPIO pins for TIM2 input capture on PA2/PA3 (CH3/CH4) */
    encoder_gpio_init(ENCODER_TIM, ENCODER_CH3_PORT, ENCODER_CH3_PIN, ENCODER_CH4_PORT, ENCODER_CH4_PIN, 1);  
    
    /* Initialize encoder with TIM2 */
    static Encoder_InitTypeDef encoder_config = {
        .TIMx = ENCODER_TIM,                              // Use TIM2 for encoder interface
        .CountsPerRevolution = 1000,                      // Adjust based on your encoder specification
        .IC1Polarity = ENCODER_IC_POLARITY_RISING,        // A phase rising edge polarity
        .IC2Polarity = ENCODER_IC_POLARITY_RISING,        // B phase rising edge polarity
        .MaxCount = 0xFFFF                                // 16-bit timer maximum count value
    };
    
    /* Initialize and start encoder counting */
    encoder_init(&motor_encoder, &encoder_config);        // Configure TIM2 in encoder mode
    encoder_start(&motor_encoder);                        // Start counting encoder pulses
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
        static uint8_t motor_running = 1;  // Track motor state (initially running)
        motor_running = !motor_running;    // Toggle state
        
        gpio_write(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, motor_running);
        SEGGER_RTT_printf(0, "ENTER pressed - Motor %s\r\n", motor_running ? "STARTED" : "STOPPED");
    }
    
    // RETURN Button (PE11): Go back or emergency stop
    if (button_pressed(&button_return)) {
        // Emergency stop functionality
        gpio_write(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, 0);  // Immediately disable motor
        gpio_write(MOTOR_P_PORT, MOTOR_P_PIN, 0);            // Stop both directions
        gpio_write(MOTOR_M_PORT, MOTOR_M_PIN, 0);
        SEGGER_RTT_printf(0, "RETURN pressed - EMERGENCY STOP!\r\n");
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
    /* Check if encoder timer has expired */
    if (systick_timer_expired(&encoder_timer)) {
        // 添加调试信息来检查编码器状态
        int32_t total_count = motor_encoder.TotalCount;
        uint32_t current_time = systick_get_ms();
        int32_t rpm = encoder_calculate_speed_rpm(&motor_encoder, current_time);
        
        SEGGER_RTT_printf(0, "TotalCount: %d, Time: %d ms, Speed: %d RPM\r\n",
                         total_count, current_time, rpm);
    }
}

/**
 * @brief Monitor motor current and perform safety shutdown if needed
 * 
 * @details This function processes ADC readings for motor current monitoring:
 *          1. Checks if the current timer period has elapsed
 *          2. When new ADC data is ready (flag set by DMA interrupt), calculates average
 *          3. Compares average current to critical threshold
 *          4. Performs emergency motor shutdown if current exceeds safe limits
 * 
 * @note This function relies on DMA to continuously fill the current_adcBuffer
 *       and set the current_adcAverageReady flag when buffer is full
 */
void current_handler(void)
{
    /* Check if current timer has expired */
    if (systick_timer_expired(&current_timer)) {
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
    encoder_handler();  // Handle encoder events
    current_handler();  // Handle current monitoring events
    
    /* Check button states using optimized manager (all 4 buttons scanned with single timer) */
    button_handler();
    
}