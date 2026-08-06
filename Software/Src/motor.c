/**
 ******************************************************************************
 * @file           : motor.c
 * @author         : Haoyi Chen
 * @date           : 2025-08-23
 * @brief          : Motor control and configuration management implementation
 ******************************************************************************
 * @details
 * This file implements motor control functions, configuration management,
 * and SPI Flash persistent storage functionality for motor parameters.
 ******************************************************************************
 */

#include "bsp.h"
#include "uart.h"
#include "systick.h"
#include "SEGGER_RTT.h"
#include <string.h>
/* External motor encoder handle - defined in event.c */
extern Encoder_HandleTypeDef motor_encoder;

/* Global motor configuration instance */
Motor_Config_t motor_config;

/* Global motor protection state */
Motor_Protection_State_t motor_protection_state;
Motor_Current_Monitor_t motor_monitor = {
    .state = MOTOR_STATE_STOPPED,
    .startup_start_time = 0U,
    .overcurrent_count = 0U,
    .current_threshold = CURRENT_RUNNING_THRESHOLD
};
static Motor_Notification_Callback_t motor_notification_callback;

void motor_set_notification_callback(Motor_Notification_Callback_t callback)
{
    motor_notification_callback = callback;
}

static void motor_notify(Motor_Notification_t notification)
{
    if (motor_notification_callback != NULL) {
        motor_notification_callback(notification);
    }
}

/* UART handle for FPGA communication */
static UART_HandleTypeDef huart_fpga;

/* W25Q128 Flash configuration */
W25Q128_Config_t flash_config = {
    .spi = SPI1,
    .cs_port = W25Q128_CS_PORT,
    .cs_pin = W25Q128_CS_PIN,
    .spi_timeout_cycles = SPI_DEFAULT_TIMEOUT_CYCLES
};
static uint8_t flash_available = 0U;

/**
 * @brief Default motor configuration values
 */
static const Motor_Config_t default_motor_config = {
    .magic_number = MOTOR_CONFIG_MAGIC_NUMBER,
    .pwm_frequency = 10000.0f,       // 10kHz PWM frequency (higher for smoother motor operation)
    .pwm_duty_cycle = 50.0f,         // 80% duty cycle (higher for reliable motor startup)
    .output_source = 0,              // Motor disabled by default
    .overcurrent_protection = 1,     // Enable overcurrent protection
    .motor_direction = 0,            // Clockwise direction
    .auto_restart_enable = 1,        // Enable auto-restart after overcurrent
    .auto_restart_delay_ms = 3000,   // 3 second restart delay
    .max_restart_attempts = 3,       // Maximum 3 restart attempts
    .motor_control_mode = 0,         // PWM control mode by default
    .pid_target_speed_percent = 50.0f, // 50% target speed
    .motor_max_rpm = 3000.0f,        // 3000 RPM maximum speed
    .checksum = 0                    // Will be calculated
};


/**
 * @brief Initialize UART for FPGA communication
 * 
 * @details Configures UART2 for FPGA communication at 115200 baud
 */
static DriverStatus motor_uart_init(void)
{
    /* Configure UART pins for FPGA communication */
    UART_PinConfig uart_pins = {
        .tx_port = FPGA_UART_TX_PORT,
        .tx_pin = FPGA_UART_TX_PIN,
        .rx_port = FPGA_UART_RX_PORT,
        .rx_pin = FPGA_UART_RX_PIN,
        .alt_func = 7                   // AF7 for UART2
    };
    
    /* Configure UART initialization structure */
    huart_fpga.Instance = USART2;                     // Use USART2 peripheral
    huart_fpga.Init.BaudRate = 115200;                // 115200 baud rate
    huart_fpga.Init.WordLength = UART_WORDLENGTH_8B;  // 8 data bits
    huart_fpga.Init.StopBits = UART_STOPBITS_1;       // 1 stop bit
    huart_fpga.Init.Parity = UART_PARITY_NONE;        // No parity
    huart_fpga.Init.Mode = UART_MODE_TX_RX;           // TX/RX mode
    huart_fpga.Init.HardwareFlowControl = UART_HWCONTROL_NONE; // No flow control
    
    /* Initialize UART */
    return uart_init(&huart_fpga, &uart_pins);
}

/**
 * @brief Calculate simple checksum for motor configuration data
 * 
 * @param config Pointer to motor configuration structure
 * @return uint16_t Calculated checksum
 */
static uint16_t motor_config_calculate_checksum(const Motor_Config_t *config)
{
    uint16_t checksum = 0;
    
    /* Calculate checksum field by field to avoid padding issues */
    checksum += config->magic_number;
    
    /* Convert float to bytes for checksum calculation */
    const uint8_t *freq_bytes = (const uint8_t *)&config->pwm_frequency;
    for (size_t i = 0U; i < sizeof(float); i++) {
        checksum += freq_bytes[i];
    }
    
    const uint8_t *duty_bytes = (const uint8_t *)&config->pwm_duty_cycle;
    for (size_t i = 0U; i < sizeof(float); i++) {
        checksum += duty_bytes[i];
    }
    
    checksum += config->output_source;
    checksum += config->overcurrent_protection;
    checksum += config->motor_direction;
    checksum += config->auto_restart_enable;
    checksum += config->auto_restart_delay_ms;
    checksum += config->max_restart_attempts;
    checksum += config->motor_control_mode;
    
    /* PID parameters */
    const uint8_t *pid_speed_bytes = (const uint8_t *)&config->pid_target_speed_percent;
    for (size_t i = 0U; i < sizeof(float); i++) {
        checksum += pid_speed_bytes[i];
    }
    
    const uint8_t *max_rpm_bytes = (const uint8_t *)&config->motor_max_rpm;
    for (size_t i = 0U; i < sizeof(float); i++) {
        checksum += max_rpm_bytes[i];
    }
    
    return checksum;
}

/**
 * @brief Validate motor configuration data integrity
 * 
 * @param config Pointer to motor configuration structure to validate
 * @return uint8_t 1 if valid, 0 if invalid
 */
static uint8_t motor_config_validate(const Motor_Config_t *config)
{
    /* Check magic number */
    if (config->magic_number != MOTOR_CONFIG_MAGIC_NUMBER) {
        return 0;
    }
    
    /* Validate parameter ranges */
    if (config->pwm_frequency < 100.0f || config->pwm_frequency > 50000.0f) {
        return 0;
    }
    
    if (config->pwm_duty_cycle < 0.0f || config->pwm_duty_cycle > 100.0f) {
        return 0;
    }
    
    if (config->output_source > 2) {
        return 0;
    }
    
    if (config->overcurrent_protection > 1) {
        return 0;
    }

    
    if (config->motor_direction > 1) {
        return 0;
    }
    
    if (config->auto_restart_enable > 1) {
        return 0;
    }
    
    if (config->auto_restart_delay_ms < 1000 || config->auto_restart_delay_ms > 60000) {
        return 0;
    }
    
    if (config->max_restart_attempts < 1U || config->max_restart_attempts > 10U) {
        return 0;
    }
    
    /* Validate PID parameters */
    if (config->motor_control_mode > 1) {
        return 0;
    }
    
    if (config->pid_target_speed_percent < 0.0f || config->pid_target_speed_percent > 100.0f) {
        return 0;
    }
    
    if (config->motor_max_rpm < 100.0f || config->motor_max_rpm > 50000.0f) {
        return 0;
    }
    
    /* Validate checksum */
    uint16_t calculated_checksum = motor_config_calculate_checksum(config);
    if (config->checksum != calculated_checksum) {
        SEGGER_RTT_printf(0, "  ✗ Checksum mismatch: stored=0x%04X, calculated=0x%04X\r\n", 
                         config->checksum, calculated_checksum);
        SEGGER_RTT_printf(0, "  Note: This may be due to checksum algorithm update\r\n");
        return 0;
    }
    
    return 1;
}

/**
 * @brief Load motor configuration from SPI Flash
 * 
 * @details This function attempts to read motor configuration from flash memory.
 *          If no valid configuration is found, it loads default values.
 * 
 * @return uint8_t 1 if config loaded from flash, 0 if defaults were used
 */
uint8_t motor_config_load_from_flash(void)
{
    Motor_Config_t temp_config;
    
    SEGGER_RTT_printf(0, "=== Loading motor config from flash ===\r\n");
    SEGGER_RTT_printf(0, "  Reading from address: 0x%08X, size: %d bytes\r\n", 
                     MOTOR_CONFIG_FLASH_ADDRESS, sizeof(Motor_Config_t));
    
    /* Read configuration from flash */
    DriverStatus flash_status = w25q128_read_data(
        &flash_config, MOTOR_CONFIG_FLASH_ADDRESS,
        (uint8_t *)&temp_config, sizeof(Motor_Config_t));
    if (flash_status != DRIVER_STATUS_OK) {
        SEGGER_RTT_printf(0, "  Flash read failed, status=%d\r\n", flash_status);
        motor_config = default_motor_config;
        motor_config.checksum = motor_config_calculate_checksum(&motor_config);
        return 0;
    }
    
    SEGGER_RTT_printf(0, "  Raw data read - Magic: 0x%08X, Checksum: 0x%04X\r\n",
                     temp_config.magic_number, temp_config.checksum);
    
    /* Validate the loaded configuration */
    if (motor_config_validate(&temp_config)) {
        /* Valid configuration found, use it */
        motor_config = temp_config;
        
        const char* source_str = (motor_config.output_source == 0) ? "Disabled" :
                                (motor_config.output_source == 1) ? "STM32" : "FPGA";
        const char* control_mode_str = (motor_config.motor_control_mode == 0) ? "PWM" : "PID";
        
        SEGGER_RTT_printf(0, "  ✓ Valid config loaded from flash\r\n");
        SEGGER_RTT_printf(0, "  PWM: Freq=%.1fHz, Duty=%.1f%%\r\n", 
                         motor_config.pwm_frequency, motor_config.pwm_duty_cycle);
        SEGGER_RTT_printf(0, "  Source=%s (%d), Direction=%s, Protection=%s\r\n",
                         source_str, motor_config.output_source,
                         motor_config.motor_direction ? "CCW" : "CW",
                         motor_config.overcurrent_protection ? "ON" : "OFF");
        SEGGER_RTT_printf(0, "  Control Mode=%s, Target Speed=%.1f%%, Max RPM=%.0f\r\n",
                         control_mode_str, motor_config.pid_target_speed_percent, motor_config.motor_max_rpm);
        
        SEGGER_RTT_printf(0, "=== Flash config load: SUCCESS ===\r\n");
        return 1;
    } else {
        /* No valid configuration, use defaults and save them */
        SEGGER_RTT_printf(0, "  ✗ Invalid config data - validation failed\r\n");
        if (temp_config.magic_number != MOTOR_CONFIG_MAGIC_NUMBER) {
            SEGGER_RTT_printf(0, "    Expected Magic: 0x%04X, Got: 0x%04X\r\n", 
                             MOTOR_CONFIG_MAGIC_NUMBER, temp_config.magic_number);
        }
        
        motor_config = default_motor_config;
        motor_config.checksum = motor_config_calculate_checksum(&motor_config);
        
        SEGGER_RTT_printf(0, "  Using defaults: Freq=%.1fHz, Duty=%.1f%%, Source=%d\r\n",
                         motor_config.pwm_frequency, motor_config.pwm_duty_cycle, motor_config.output_source);
        
        /* Save default configuration to flash with new checksum algorithm */
        SEGGER_RTT_printf(0, "  Saving default config with updated checksum algorithm...\r\n");
        motor_config_save_to_flash();
        
        SEGGER_RTT_printf(0, "=== Flash config load: DEFAULTS USED ===\r\n");
        return 0;
    }
}

/**
 * @brief Save current motor configuration to SPI Flash
 * 
 * @details This function saves the current motor configuration to flash memory
 *          for persistence across system restarts.
 * 
 * @return uint8_t 1 if successful, 0 if failed
 */
uint8_t motor_config_save_to_flash(void)
{
    if (flash_available == 0U) {
        SEGGER_RTT_printf(0, "Flash unavailable; configuration was not saved\r\n");
        return 0;
    }

    const char* source_str = (motor_config.output_source == 0) ? "Disabled" :
                            (motor_config.output_source == 1) ? "STM32" : "FPGA";
    const char* control_mode_str = (motor_config.motor_control_mode == 0) ? "PWM" : "PID";
    
    SEGGER_RTT_printf(0, "=== Saving motor config to flash ===\r\n");
    SEGGER_RTT_printf(0, "  PWM: Freq=%.1fHz, Duty=%.1f%%\r\n", 
                     motor_config.pwm_frequency, motor_config.pwm_duty_cycle);
    SEGGER_RTT_printf(0, "  Source=%s (%d), Direction=%s, Protection=%s\r\n",
                     source_str, motor_config.output_source,
                     motor_config.motor_direction ? "CCW" : "CW",
                     motor_config.overcurrent_protection ? "ON" : "OFF");
    SEGGER_RTT_printf(0, "  Control Mode=%s, Target Speed=%.1f%%, Max RPM=%.0f\r\n",
                     control_mode_str, motor_config.pid_target_speed_percent, motor_config.motor_max_rpm);
    SEGGER_RTT_printf(0, "  Magic: 0x%08X, Size: %d bytes\r\n", 
                     motor_config.magic_number, sizeof(Motor_Config_t));
    
    /* Calculate checksum before saving */
    uint16_t old_checksum = motor_config.checksum;
    motor_config.checksum = motor_config_calculate_checksum(&motor_config);
    SEGGER_RTT_printf(0, "  Checksum: 0x%04X (was 0x%04X)\r\n", motor_config.checksum, old_checksum);
    
    /* Erase sector before writing */
    SEGGER_RTT_printf(0, "  Erasing flash sector at 0x%08X...\r\n", MOTOR_CONFIG_FLASH_ADDRESS);
    DriverStatus flash_status = w25q128_sector_erase(
        &flash_config, MOTOR_CONFIG_FLASH_ADDRESS);
    if (flash_status != DRIVER_STATUS_OK) {
        SEGGER_RTT_printf(0, "  Sector erase failed, status=%d\r\n", flash_status);
        return 0;
    }
    SEGGER_RTT_printf(0, "  Sector erase completed\r\n");
    
    /* Write configuration to flash */
    SEGGER_RTT_printf(0, "  Writing %d bytes to flash...\r\n", sizeof(Motor_Config_t));
    flash_status = w25q128_page_program(
        &flash_config, MOTOR_CONFIG_FLASH_ADDRESS,
        (const uint8_t *)&motor_config, sizeof(Motor_Config_t));
    if (flash_status != DRIVER_STATUS_OK) {
        SEGGER_RTT_printf(0, "  Flash write failed, status=%d\r\n", flash_status);
        return 0;
    }
    SEGGER_RTT_printf(0, "  Flash write completed\r\n");
    
    /* Verify the write by reading back */
    Motor_Config_t verify_config;
    flash_status = w25q128_read_data(
        &flash_config, MOTOR_CONFIG_FLASH_ADDRESS,
        (uint8_t *)&verify_config, sizeof(Motor_Config_t));
    if (flash_status != DRIVER_STATUS_OK) {
        SEGGER_RTT_printf(0, "  Verification read failed, status=%d\r\n",
                         flash_status);
        return 0;
    }
    
    if (memcmp(&verify_config, &motor_config, sizeof(Motor_Config_t)) == 0) {
        SEGGER_RTT_printf(0, "  Flash write verification: PASS\r\n");
    } else {
        SEGGER_RTT_printf(0, "  Flash write verification: FAIL (Magic: 0x%08X, Checksum: 0x%04X)\r\n",
                         verify_config.magic_number, verify_config.checksum);
        return 0;
    }
    
    SEGGER_RTT_printf(0, "=== Motor config flash save completed ===\r\n");
    return 1;
}

/**
 * @brief Initialize motor configuration system
 * 
 * @details This function initializes the SPI Flash, UART, and loads motor configuration.
 *          This should be called during system startup after SPI is initialized.
 */
void motor_config_init(void)
{
    uint8_t manufacturer_id;
    uint16_t device_id;
    
    flash_available = 0U;

    /* Initialize UART for FPGA communication */
    (void)motor_uart_init();
    
    /* Initialize motor protection system */
    motor_protection_init();
    
    /* Initialize W25Q128 Flash */
    SEGGER_RTT_printf(0, "Initializing W25Q128 Flash...\r\n");
    DriverStatus flash_status = w25q128_init(&flash_config);
    if (flash_status != DRIVER_STATUS_OK) {
        SEGGER_RTT_printf(0, "Flash initialization failed, status=%d\r\n",
                         flash_status);
        motor_config = default_motor_config;
        motor_config.checksum = motor_config_calculate_checksum(&motor_config);
        return;
    }
    SEGGER_RTT_printf(0, "Flash init completed, reading JEDEC ID...\r\n");
    
    /* Verify flash connection */
    flash_status = w25q128_read_jedec_id(&flash_config, &manufacturer_id,
                                         &device_id);
    if (flash_status == DRIVER_STATUS_OK) {
        if (manufacturer_id == W25Q128_JEDEC_MANUFACTURER_ID && 
            device_id == W25Q128_JEDEC_DEVICE_ID) {
            SEGGER_RTT_printf(0, "W25Q128 Flash detected successfully\r\n");
            flash_available = 1U;
            
            /* Load motor configuration from flash */
            motor_config_load_from_flash();
        } else {
            SEGGER_RTT_printf(0, "Flash ID mismatch: 0x%02X:0x%04X\r\n", 
                             manufacturer_id, device_id);
            /* Use default configuration */
            motor_config = default_motor_config;
            motor_config.checksum = motor_config_calculate_checksum(&motor_config);
        }
    } else {
        SEGGER_RTT_printf(0, "Failed to read Flash JEDEC ID\r\n");
        /* Use default configuration */
        motor_config = default_motor_config;
        motor_config.checksum = motor_config_calculate_checksum(&motor_config);
    }
    
    /* UI synchronization will be done after UI system is initialized */
    SEGGER_RTT_printf(0, "Motor configuration loaded, UI sync deferred until UI init\r\n");
}

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

    /* Configure motor pin initial states - motor should start in STOPPED state */
    gpio_write(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, 0);    // Initially disable motor for safety
    gpio_write(MOTOR_P_PORT, MOTOR_P_PIN, 0);              // Both direction pins low for safety
    gpio_write(MOTOR_M_PORT, MOTOR_M_PIN, 0);              // Both direction pins low for safety

    //Configure encoder GPIO pins for TIMx in encoder mode
    encoder_gpio_init(ENCODER_TIM, ENCODER_CH1_PORT, ENCODER_CH1_PIN, ENCODER_CH2_PORT, ENCODER_CH2_PIN, ENCODER_AF);  
    
    //Initialize encoder with TIM4
    static Encoder_InitTypeDef encoder_config = {
        .TIMx = ENCODER_TIM,                              // Use TIMx for encoder interface
        .CountsPerRevolution = 4680,                      // 1:90 geared motor: 13 lines × 90 gear ratio × 4 quad decoding = 4680
        .IC1Polarity = ENCODER_IC_POLARITY_RISING,        // A phase rising edge polarity
        .IC2Polarity = ENCODER_IC_POLARITY_RISING,        // B phase rising edge polarity
        .MaxCount = 0xFFFF                                // 16-bit timer maximum count value
    };
    
    // Initialize and start encoder counting 
    if (encoder_init(&motor_encoder, &encoder_config) == DRIVER_STATUS_OK) {
        encoder_start(&motor_encoder);                    // Start counting encoder pulses
    }
}

/**
 * @brief Start motor with startup-aware current protection
 * 
 * @details This function starts the motor and initializes startup protection:
 *          1. Sets motor state to STARTING with high current threshold
 *          2. Records startup timestamp for timeout tracking
 *          3. Enables motor with specified direction
 */
/**
 * @brief Set motor direction
 * 
 * @param direction Motor direction (0=clockwise, 1=counter-clockwise)
 * 
 * @details Controls motor direction via GPIO pins P/M regardless of PWM source.
 *          This function works with both STM32 and FPGA PWM configurations.
 */
void motor_set_direction(uint8_t direction)
{
    if (direction) {
        gpio_write(MOTOR_P_PORT, MOTOR_P_PIN, 1);        /* Counter-clockwise direction */
        gpio_write(MOTOR_M_PORT, MOTOR_M_PIN, 0);
    } else {
        gpio_write(MOTOR_P_PORT, MOTOR_P_PIN, 0);        /* Clockwise direction */
        gpio_write(MOTOR_M_PORT, MOTOR_M_PIN, 1);
    }
    
    SEGGER_RTT_printf(0, "Motor direction set: %s\r\n", direction ? "Counter-clockwise" : "Clockwise");
}

/**
 * @brief Enable motor with startup current protection
 * 
 * @details Enables motor operation and sets up startup current monitoring.
 *          Should be called after PWM configuration and direction setting.
 */
void motor_start(void)
{
    motor_state_start(&motor_monitor, systick_get_ms(), CURRENT_STARTUP_THRESHOLD);
    
    /* Enable motor */
    gpio_write(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, 1);
    
    SEGGER_RTT_printf(0, "Motor enabled with startup protection\r\n");
}

/**
 * @brief Perform emergency motor shutdown and reset monitoring state
 * 
 * @details This function safely shuts down the motor and resets all control signals:
 *          1. Immediately disables motor enable pin
 *          2. Sets both direction pins to safe state (both low)
 *          3. Resets motor monitoring state to STOPPED
 *          4. Clears overcurrent counter
 */
void motor_emergency_shutdown(void)
{
    /* Immediate motor shutdown */
    gpio_write(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, 0);  /* Disable motor */
    gpio_write(MOTOR_P_PORT, MOTOR_P_PIN, 0);            /* Stop both directions */
    gpio_write(MOTOR_M_PORT, MOTOR_M_PIN, 0);
    
    motor_state_stop(&motor_monitor, CURRENT_RUNNING_THRESHOLD);
}

/**
 * @brief Update motor state and current threshold based on operational conditions
 * 
 * @details This function manages motor state transitions:
 *          - STOPPED: Motor is disabled, no current monitoring needed
 *          - STARTING: Motor just enabled, allows high inrush current for limited time
 *          - RUNNING: Motor in normal operation, strict current protection
 */
void motor_current_state_update(void)
{
    motor_state_update(&motor_monitor,
                       systick_get_ms(),
                       STARTUP_TIMEOUT_MS,
                       CURRENT_RUNNING_THRESHOLD);
}

/**
 * @brief Initialize motor protection system
 * 
 * @details Initializes protection state and sets default values.
 */
void motor_protection_init(void)
{
    motor_protection_state.protection_fault = 0;
    motor_protection_state.restart_attempts = 0;
    motor_protection_state.last_fault_time = 0;
    motor_protection_state.restart_scheduled_time = 0;
    motor_protection_state.fault_display_pending = 0;
    motor_protection_state.waiting_user_decision = 0;
    motor_protection_state.user_decision_retry = 0;
    
    SEGGER_RTT_printf(0, "Motor protection system initialized\r\n");
}

/**
 * @brief Send motor parameters to FPGA via UART
 * 
 * @details Transmits PWM frequency, duty cycle, and direction to FPGA
 *          using structured communication packet.
 */
void motor_send_fpga_command(float frequency, float duty_cycle)
{
    FPGA_Command_Packet_t packet;
    
    /* Build packet */
    packet.header[0] = FPGA_PACKET_HEADER_1;
    packet.header[1] = FPGA_PACKET_HEADER_2;
    packet.pwm_frequency = frequency;
    packet.pwm_duty_cycle = duty_cycle;
    
    /* Calculate simple checksum */
    packet.checksum = 0;
    uint8_t *data = (uint8_t *)&packet;
    for (size_t i = 0; i < sizeof(FPGA_Command_Packet_t) - 1; i++) {
        packet.checksum ^= data[i];
    }
    
    /* Send packet via UART */
    (void)uart_transmit(&huart_fpga, (const uint8_t *)&packet,
                        sizeof(FPGA_Command_Packet_t), 1000U);
    
    SEGGER_RTT_printf(0, "FPGA command sent: freq=%.1fHz, duty=%.1f\r\n",
                     packet.pwm_frequency, packet.pwm_duty_cycle);
}

/**
 * @brief Update motor protection state and handle auto-restart logic
 * 
 * @details Manages overcurrent protection, restart attempts, and fault displays.
 */
void motor_protection_update(void)
{
    uint32_t current_time = systick_get_ms();
    
    /* Check if motor protection is enabled */
    if (!motor_config.overcurrent_protection) {
        return;
    }
    
    /* If motor is running normally for 10 seconds without fault, reset restart attempts */
    static uint32_t last_successful_run_time = 0;
    if (!motor_protection_state.protection_fault && motor_config.output_source > 0) {
        if (last_successful_run_time == 0) {
            last_successful_run_time = current_time;
        } else if ((current_time - last_successful_run_time) > 10000) { // 10 seconds
            if (motor_protection_state.restart_attempts > 0) {
                SEGGER_RTT_printf(0, "Motor running successfully - resetting restart attempts\r\n");
                motor_protection_state.restart_attempts = 0;
            }
            last_successful_run_time = current_time; // Update for next check
        }
    } else {
        last_successful_run_time = 0; // Reset timer if motor is not running or has fault
    }
    
    /* Check for current overcurrent condition using appropriate threshold */
    extern volatile uint16_t current_adcAverage;
    uint16_t current_threshold = (motor_monitor.state == MOTOR_STATE_STARTING) ? 
                                CURRENT_STARTUP_THRESHOLD : CURRENT_RUNNING_THRESHOLD;
    
    if (current_adcAverage > current_threshold) {
        /* Overcurrent detected */
        if (!motor_protection_state.protection_fault) {
            /* New fault detected */
            motor_protection_state.protection_fault = 1;
            motor_protection_state.last_fault_time = current_time;
            
            /* Emergency shutdown */
            motor_emergency_shutdown();
            
            SEGGER_RTT_printf(0, "Overcurrent fault detected! Current: %d\r\n", current_adcAverage);
            
            /* Show overcurrent fault popup */
            motor_notify(MOTOR_NOTIFICATION_OVERCURRENT);
            
            /* Schedule restart if auto-restart is enabled */
            if (motor_config.auto_restart_enable && 
                motor_protection_state.restart_attempts < motor_config.max_restart_attempts) {
                motor_protection_state.restart_scheduled_time = 
                    current_time + motor_config.auto_restart_delay_ms;
                SEGGER_RTT_printf(0, "Auto-restart scheduled in %d ms (attempt %d/%d)\r\n",
                                 motor_config.auto_restart_delay_ms,
                                 motor_protection_state.restart_attempts + 1,
                                 motor_config.max_restart_attempts);
            } else {
                /* Maximum attempts reached or auto-restart disabled */
                motor_protection_state.fault_display_pending = 1;
                if (motor_protection_state.restart_attempts >= motor_config.max_restart_attempts) {
                    SEGGER_RTT_printf(0, "Maximum restart attempts exceeded - showing user decision popup\r\n");
                    motor_protection_state.waiting_user_decision = 1;
                    motor_notify(MOTOR_NOTIFICATION_MAX_RESTARTS);
                }
            }
        }
    }
    
    /* Handle auto-restart logic */
    if (motor_protection_state.protection_fault && 
        motor_config.auto_restart_enable &&
        motor_protection_state.restart_attempts < motor_config.max_restart_attempts &&
        motor_protection_state.restart_scheduled_time > 0 &&
        current_time >= motor_protection_state.restart_scheduled_time) {
        
        /* Time to attempt restart */
        motor_protection_state.restart_attempts++;
        motor_protection_state.restart_scheduled_time = 0;
        motor_protection_state.protection_fault = 0;
        
        SEGGER_RTT_printf(0, "Attempting auto-restart (attempt %d/%d)\r\n",
                         motor_protection_state.restart_attempts,
                         motor_config.max_restart_attempts);
        
        /* Show motor restart popup */
        motor_notify(MOTOR_NOTIFICATION_RESTARTING);
        
        /* Apply current motor configuration */
        motor_apply_config();
    }
}

/**
 * @brief Apply motor configuration based on current settings
 * 
 * @details Configures PWM output source (STM32/FPGA) and applies parameters.
 *          Sends UART commands to FPGA if needed.
 */
void motor_apply_config(void)
{
    const char* source_str = (motor_config.output_source == 0) ? "Disabled" :
                            (motor_config.output_source == 1) ? "STM32" : "FPGA";
    SEGGER_RTT_printf(0, "Applying motor config: Freq=%.1fHz, Duty=%.1f%%, Source=%s (%d)\r\n",
                     motor_config.pwm_frequency, motor_config.pwm_duty_cycle, 
                     source_str, motor_config.output_source);
    
    /* Use unified PWM configuration function */
    motor_pwm_config(motor_config.pwm_frequency, motor_config.pwm_duty_cycle, motor_config.output_source);
    
    /* Handle motor enable/direction for non-disabled modes */
    if (motor_config.output_source > 0) {
        // Set motor direction (controlled by STM32 GPIO for both STM32 and FPGA PWM) */
        motor_set_direction(motor_config.motor_direction);
        // Enable motor */
        motor_start();
    } else {
        motor_emergency_shutdown();
    }
    
    /* Reset protection fault status when applying new config */
    motor_protection_state.protection_fault = 0;
    /* Note: Don't reset restart_attempts here - only reset when motor runs successfully or user manually resets */
    motor_protection_state.fault_display_pending = 0;
}

/**
 * @brief Configure PWM output based on current motor configuration
 * 
 * @param frequency PWM frequency in Hz
 * @param duty_cycle PWM duty cycle in percentage (0-100)
 * @param output_source Output source selection (0=disabled, 1=STM32, 2=FPGA)
 * 
 * @details Handles PWM configuration based on output source:
 *          - STM32: Configures TIM3_CH1 on PB4 for PWM output
 *          - FPGA: Sends PWM parameters via UART
 *          - Disabled: Stops PWM output
 */
void motor_pwm_config(float frequency, float duty_cycle, uint8_t output_source)
{
    switch (output_source) {
        case 0:
            /* Motor disabled - stop PWM output */
            tim_disable(TIM3);
            gpio_write(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, 0);
            SEGGER_RTT_printf(0, "PWM output disabled\r\n");
            break;
            
        case 1:
            /* STM32 PWM output */
            /* Disable motor first to prevent glitches during configuration */
            gpio_write(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, 0);
            
            /* Enable TIM3 clock */
            rcc_enable_tim_clock(TIM3);
            
            /* Configure PB4 as alternate function for TIM3_CH1 */
            gpio_init(MOTOR_PWM_PORT, MOTOR_PWM_PIN, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_NOPULL);
            gpio_set_af(MOTOR_PWM_PORT, MOTOR_PWM_PIN, MOTOR_PWM_AF);
            
            /* Calculate prescaler and period for desired frequency */
            uint32_t timer_clock = 84000000; // 84 MHz APB1 timer clock
            uint32_t desired_freq = (uint32_t)frequency;
            
            /* Find optimal prescaler and period combination */
            uint32_t prescaler = 1;
            uint32_t period = timer_clock / (prescaler * desired_freq);
            
            /* Adjust prescaler if period is too large (max 65535) */
            while (period > 65535 && prescaler < 65536) {
                prescaler *= 2;
                period = timer_clock / (prescaler * desired_freq);
            }
            
            /* Always disable timer first for clean reconfiguration */
            tim_disable(TIM3);
            
            /* Configure timer with calculated values */
            TIM_InitTypeDef tim_init_config = {
                .Prescaler = prescaler - 1,
                .Period = period - 1,
                .ClockDivision = TIM_CLOCKDIVISION_DIV1,
                .CounterMode = TIM_COUNTERMODE_UP
            };
            
            tim_init(TIM3, &tim_init_config);
            
            /* Use startup boost if duty cycle is low */
            float startup_duty = duty_cycle;
            if (duty_cycle < 70.0f) {
                startup_duty = 85.0f;  // Use higher duty cycle for reliable startup
                SEGGER_RTT_printf(0, "Using startup boost: %.1f%% -> %.1f%%\r\n", duty_cycle, startup_duty);
            }
            
            /* Calculate PWM pulse value for startup duty cycle */
            uint32_t pulse = (uint32_t)((startup_duty / 100.0f) * period);
            
            /* Configure PWM channel 1 */
            TIM_PWM_ConfigTypeDef pwm_config = {
                .Channel = TIM_CHANNEL_1,
                .Pulse = pulse,
                .OCMode = TIM_OCMODE_PWM1,
                .OCPolarity = TIM_OCPOLARITY_HIGH
            };
            
            tim_pwm_config(TIM3, &pwm_config);
            
            /* Ensure PWM output is properly enabled */
            TIM3->CR1 |= TIM_CR1_ARPE;  // Enable auto-reload preload
            TIM3->EGR |= TIM_EGR_UG;    // Generate update event to load all values
            
            tim_enable(TIM3);
            
            /* Wait for PWM to stabilize */
            for (volatile int i = 0; i < 10000; i++);
            
            SEGGER_RTT_printf(0, "STM32 PWM configured: %.1fHz, %.1f%% on PB4 (TIM3_CH1)\r\n", frequency, duty_cycle);
            SEGGER_RTT_printf(0, "TIM3 registers: PSC=%lu, ARR=%lu, CCR1=%lu, CR1=0x%lx, CCER=0x%lx\r\n", 
                             TIM3->PSC, TIM3->ARR, TIM3->CCR1, TIM3->CR1, TIM3->CCER);
            
            /* Enable motor after PWM is stable */
            gpio_write(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, 1);
            SEGGER_RTT_printf(0, "Motor enabled after PWM stabilization\r\n");
            
            /* If we used startup boost, wait then reduce to target duty cycle */
            if (startup_duty != duty_cycle) {
                /* Wait for motor to start */
                for (volatile int i = 0; i < 500000; i++);  // ~100ms delay at 84MHz
                
                /* Now set the target duty cycle */
                uint32_t target_pulse = (uint32_t)((duty_cycle / 100.0f) * period);
                TIM3->CCR1 = target_pulse;
                TIM3->EGR |= TIM_EGR_UG;  // Update immediately
                
                SEGGER_RTT_printf(0, "Startup complete, reduced to target: %.1f%%\r\n", duty_cycle);
            }
            break;
            
        case 2:
            /* FPGA PWM output - send via UART */
            /* Disable STM32 PWM if it was running */
            tim_disable(TIM3);
            gpio_write(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, 0);
            /* Send FPGA PWM command (only frequency and duty cycle) */
            motor_send_fpga_command(frequency, duty_cycle);
        gpio_write(MOTOR_ENABLE_PORT, MOTOR_ENABLE_PIN, 1);
            SEGGER_RTT_printf(0, "FPGA PWM command sent: %.1fHz, %.1f%%\r\n", frequency, duty_cycle);
            break;
            
        default:
            SEGGER_RTT_printf(0, "Invalid output source: %d\r\n", output_source);
            break;
    }
}

/**
 * @brief Handle user decision for motor restart after max attempts
 * 
 * @param retry 1 to retry motor start, 0 to cancel and disable motor
 */
void motor_handle_user_decision(uint8_t retry)
{
    if (!motor_protection_state.waiting_user_decision) {
        return; // Not waiting for decision
    }
    
    motor_protection_state.waiting_user_decision = 0;
    motor_protection_state.user_decision_retry = retry;
    
    if (retry) {
        SEGGER_RTT_printf(0, "User chose to retry motor start\r\n");
        /* Reset restart attempts and try again */
        motor_protection_state.restart_attempts = 0;
        motor_protection_state.protection_fault = 0;
        motor_apply_config();
    } else {
        SEGGER_RTT_printf(0, "User chose to cancel motor operation\r\n");
        /* Disable motor completely */
        motor_emergency_shutdown();
        motor_config.output_source = 0; // Set to disabled
        motor_config_save_to_flash();   // Save disabled state
    }
}
