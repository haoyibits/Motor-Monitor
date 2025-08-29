#include "OLED_UI_MenuData.h"
#include "../../../Inc/motor.h"
#include "SEGGER_RTT.h"

/*Motor Monitor Menu Data - Main interface with PWM settings, overcurrent protection, and device info*/

// External motor control variables from motor.c
extern Motor_Config_t motor_config;

// UI system control variables
extern bool ColorMode;
extern bool OLED_UI_ShowFps;
extern int16_t OLED_UI_Brightness;

// Helper variables for UI display (derived from motor_config)
static float motor_pwm_frequency_khz = 10.0f;  // PWM frequency in kHz for UI display
static bool motor_overcurrent_protection_bool = true;  // Bool conversion for UI
static bool motor_auto_restart_bool = true;            // Bool conversion for UI
static float motor_current_limit = 2.0f;               // Current limit for UI (not saved to Flash)

// Speed setting for animations
#define SPEED 8

// Overcurrent Protection Popup Windows
MenuWindow OvercurrentFaultWindow = {
	.General_Width = 110,
	.General_Height = 40,
	.Text_String = "OVERCURRENT FAULT!",
	.Text_FontSize = OLED_UI_FONT_8,
	.Text_FontSideDistance = 8,
	.Text_FontTopDistance = 15,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 2.0f,
};

MenuWindow MotorRestartWindow = {
	.General_Width = 110,
	.General_Height = 40,
	.Text_String = "Restarting...",
	.Text_FontSize = OLED_UI_FONT_8,
	.Text_FontSideDistance = 12,
	.Text_FontTopDistance = 15,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 1.5f,
};

// System restart required popup window (persistent)
MenuWindow SystemRestartWindow = {
	.General_Width = 120,
	.General_Height = 50,
	.Text_String = "!!Motor status is abnormal. Please manually restart!!",
	.Text_FontSize = OLED_UI_FONT_8,
	.Text_FontSideDistance = 5,
	.Text_FontTopDistance = 20,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = -1.0f,  // Negative value means permanent window
};

// Max restart attempts reached decision menu items
MenuItem MaxRestartDecisionMenuItems[] = {
	{.General_item_text = "Retry Motor Start",.General_callback = UserDecisionRetryMotor,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL,.List_IntBox = NULL},
	{.General_item_text = "Cancel & Disable",.General_callback = UserDecisionCancelMotor,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL,.List_IntBox = NULL},
	{.General_item_text = NULL}  // Terminator
};

// Max restart decision menu page
MenuPage MaxRestartDecisionMenuPage = {
	.General_MenuType = MENU_TYPE_LIST,
	.General_MovingSpeed = SPEED,
	.General_CursorStyle = 0,
	.General_MoveStyle = 0,
	.General_FontSize = OLED_UI_FONT_12,
	.General_ParentMenuPage = &MainMenuPage,
	.General_MenuItems = MaxRestartDecisionMenuItems,
	.General_LineSpace = 2,
	.General_ShowAuxiliaryFunction = NULL,
};

// PWM Duty Cycle Setting Window
MenuWindow PWMDutyWindow = {
	.General_Width = 80,
	.General_Height = 28,
	.Text_String = "PWM Duty %",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4,
	.Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0f,

	.Prob_Data_Float = &motor_config.pwm_duty_cycle,
	.Prob_DataStep = 1.0f,
	.Prob_MinData = 0.0f,
	.Prob_MaxData = 100.0f,
	.Prob_BottomDistance = 3,
	.Prob_LineHeight = 8,
	.Prob_SideDistance = 4,
};

// PWM Frequency Setting Window
MenuWindow PWMFrequencyWindow = {
	.General_Width = 80,
	.General_Height = 28,
	.Text_String = "PWM Freq kHz",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4,
	.Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0f,

	.Prob_Data_Float = &motor_pwm_frequency_khz,
	.Prob_DataStep = 1.0f,
	.Prob_MinData = 1.0f,
	.Prob_MaxData = 100.0f,
	.Prob_BottomDistance = 3,
	.Prob_LineHeight = 8,
	.Prob_SideDistance = 4,
};

// Restart Delay Setting Window  
MenuWindow RestartDelayWindow = {
	.General_Width = 80,
	.General_Height = 28,
	.Text_String = "Restart Delay ms",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4,
	.Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0f,

	.Prob_Data_Int = (int16_t*)&motor_config.auto_restart_delay_ms,
	.Prob_DataStep = 500,
	.Prob_MinData = 1000,
	.Prob_MaxData = 60000,
	.Prob_BottomDistance = 3,
	.Prob_LineHeight = 8,
	.Prob_SideDistance = 4,
};

// Max Restart Attempts Setting Window
MenuWindow MaxRestartAttemptsWindow = {
	.General_Width = 80,
	.General_Height = 28,
	.Text_String = "Max Attempts",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4,
	.Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0f,

	.Prob_Data_Int = (int16_t*)&motor_config.max_restart_attempts,
	.Prob_DataStep = 1,
	.Prob_MinData = 1,
	.Prob_MaxData = 10,
	.Prob_BottomDistance = 3,
	.Prob_LineHeight = 8,
	.Prob_SideDistance = 4,
};

// Current Limit Setting Window
MenuWindow CurrentLimitWindow = {
	.General_Width = 80,
	.General_Height = 28,
	.Text_String = "Current Limit A",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4,
	.Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0f,

	.Prob_Data_Float = &motor_current_limit,
	.Prob_DataStep = 0.1f,
	.Prob_MinData = 0.1f,
	.Prob_MaxData = 5.0f,
	.Prob_BottomDistance = 3,
	.Prob_LineHeight = 8,
	.Prob_SideDistance = 4,
};

// Screen Brightness Setting Window
MenuWindow SetBrightnessWindow = {
	.General_Width = 80,
	.General_Height = 28,
	.Text_String = "Screen Brightness",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4,
	.Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0f,

	.Prob_Data_Int = &OLED_UI_Brightness,
	.Prob_DataStep = 5,
	.Prob_MinData = 5,
	.Prob_MaxData = 255,
	.Prob_BottomDistance = 3,
	.Prob_LineHeight = 8,
	.Prob_SideDistance = 4,
};

// PID Target Speed Setting Window (as percentage of max RPM)
MenuWindow PIDTargetSpeedWindow = {
	.General_Width = 80,
	.General_Height = 28,
	.Text_String = "Target Speed %",
	.Text_FontSize = OLED_UI_FONT_12,
	.Text_FontSideDistance = 4,
	.Text_FontTopDistance = 3,
	.General_WindowType = WINDOW_ROUNDRECTANGLE,
	.General_ContinueTime = 4.0f,

	.Prob_Data_Float = &motor_config.pid_target_speed_percent,
	.Prob_DataStep = 5.0f,
	.Prob_MinData = 0.0f,
	.Prob_MaxData = 100.0f,
	.Prob_BottomDistance = 3,
	.Prob_LineHeight = 8,
	.Prob_SideDistance = 4,
};


/**
 * @brief Create PWM duty cycle setting window
 */
void PWMDutySettingWindow(void){
	SEGGER_RTT_printf(0, "UI: Opening PWM duty cycle setting window\r\n");
	OLED_UI_CreateWindow(&PWMDutyWindow);
	/* After window closes, save configuration to Flash */
	SEGGER_RTT_printf(0, "UI: PWM duty cycle changed to %.1f%%, saving to flash\r\n", motor_config.pwm_duty_cycle);
	motor_config_save_to_flash();
}



/**
 * @brief Create PWM frequency setting window
 */

void PWMFrequencySettingWindow(void){
	/* Update motor_config frequency from kHz display value */
	motor_config.pwm_frequency = motor_pwm_frequency_khz * 1000.0f;
	SEGGER_RTT_printf(0, "UI: Opening PWM frequency setting window (%.1f kHz)\r\n", motor_pwm_frequency_khz);
	OLED_UI_CreateWindow(&PWMFrequencyWindow);
	/* After window closes, save configuration to Flash */
	SEGGER_RTT_printf(0, "UI: PWM frequency changed to %.1f Hz, saving to flash\r\n", motor_config.pwm_frequency);
	motor_config_save_to_flash();
}

/**
 * @brief Create restart delay setting window
 */
void RestartDelaySettingWindow(void){
	OLED_UI_CreateWindow(&RestartDelayWindow);
	/* After window closes, save configuration to Flash */
	motor_config_save_to_flash();
}

/**
 * @brief Create max restart attempts setting window
 */
void MaxRestartAttemptsSettingWindow(void){
	OLED_UI_CreateWindow(&MaxRestartAttemptsWindow);
	/* After window closes, save configuration to Flash */
	motor_config_save_to_flash();
}

/**
 * @brief Protection settings save callback
 */
void ProtectionSaveCallback(void){
	/* Sync bool helper variables back to motor_config uint8_t fields */
	motor_config.overcurrent_protection = motor_overcurrent_protection_bool ? 1 : 0;
	motor_config.auto_restart_enable = motor_auto_restart_bool ? 1 : 0;
	motor_config_save_to_flash();
}

/**
 * @brief Output Source Selection Functions
 */
void SetOutputSourceDisabled(void){
	SEGGER_RTT_printf(0, "UI: Setting output source to Disabled\r\n");
	motor_config.output_source = 0;  // Disabled
	motor_config_save_to_flash();
	motor_apply_config();  // Apply changes immediately
	OLED_UI_Back();  // Go back to menu
}

void SetOutputSourceSTM32(void){
	SEGGER_RTT_printf(0, "UI: Setting output source to STM32\r\n");
	motor_config.output_source = 1;  // STM32
	motor_config_save_to_flash();
	motor_apply_config();  // Apply changes immediately
	OLED_UI_Back();  // Go back to menu  
}

void SetOutputSourceFPGA(void){
	SEGGER_RTT_printf(0, "UI: Setting output source to FPGA\r\n");
	motor_config.output_source = 2;  // FPGA
	motor_config_save_to_flash();
	motor_apply_config();  // Apply changes immediately
	OLED_UI_Back();  // Go back to menu
}

/**
 * @brief Motor Direction Selection Functions
 */
void SetDirectionClockwise(void){
	motor_config.motor_direction = 0;  // Clockwise = 0
	motor_config_save_to_flash();
	OLED_UI_Back();  // Go back to menu
}

void SetDirectionCounterclockwise(void){
	motor_config.motor_direction = 1;   // Counter-clockwise = 1  
	motor_config_save_to_flash();
	OLED_UI_Back();  // Go back to menu
}

/**
 * @brief Create current limit setting window
 */
void CurrentLimitSettingWindow(void){
	OLED_UI_CreateWindow(&CurrentLimitWindow);
	/* After window closes, save configuration to Flash */
	motor_config_save_to_flash();
}

/**
 * @brief Create screen brightness setting window
 */
void BrightnessWindow(void){
	OLED_UI_CreateWindow(&SetBrightnessWindow);
}

/**
 * @brief Create PID target speed setting window
 */
void PIDTargetSpeedSettingWindow(void){
	SEGGER_RTT_printf(0, "UI: Opening PID target speed setting window\r\n");
	OLED_UI_CreateWindow(&PIDTargetSpeedWindow);
	/* After window closes, save configuration to Flash */
	SEGGER_RTT_printf(0, "UI: PID target speed changed to %.1f%%, saving to flash\r\n", motor_config.pid_target_speed_percent);
	motor_config_save_to_flash();
}

/**
 * @brief Show overcurrent fault popup window
 */
void ShowOvercurrentFaultPopup(void){
	OLED_UI_CreateWindow(&OvercurrentFaultWindow);
}

/**
 * @brief Show motor restart popup window
 */
void ShowMotorRestartPopup(void){
	OLED_UI_CreateWindow(&MotorRestartWindow);
}

/**
 * @brief Show max restart attempts reached popup
 * 
 * @details Shows a persistent popup that instructs user to manually restart the system
 */
void ShowMaxAttemptsReachedPopup(void){
	// Show persistent restart instruction window
	OLED_UI_CreateWindow(&SystemRestartWindow);
}

/**
 * @brief User decision callback - Retry motor start
 */
void UserDecisionRetryMotor(void){
	motor_handle_user_decision(1); // 1 = retry
	OLED_UI_Back(); // Return to previous menu
}

/**
 * @brief User decision callback - Cancel and disable motor
 */
void UserDecisionCancelMotor(void){
	motor_handle_user_decision(0); // 0 = cancel
	OLED_UI_Back(); // Return to previous menu
}

/**
 * @brief Update UI display helper variables from motor configuration
 * 
 * @details Only synchronizes helper variables needed for UI display (like frequency in kHz)
 *          All other UI elements directly reference motor_config fields
 */
void motor_ui_update_from_config(void)
{
    SEGGER_RTT_printf(0, "=== Syncing UI display variables ===\r\n");
    SEGGER_RTT_printf(0, "  Config: Freq=%.1fHz, Duty=%.1f%%\r\n", 
                     motor_config.pwm_frequency, motor_config.pwm_duty_cycle);
    
    /* Sync helper variables for UI display */
    motor_pwm_frequency_khz = motor_config.pwm_frequency / 1000.0f;  // Convert Hz to kHz for display
    motor_overcurrent_protection_bool = (motor_config.overcurrent_protection != 0);  // Convert uint8_t to bool
    motor_auto_restart_bool = (motor_config.auto_restart_enable != 0);              // Convert uint8_t to bool
    
}

/**
 * @brief PWM Control submenu callback - Apply PWM motor control when entering submenu
 * 
 * @details This function is called when entering the PWM Control submenu.
 *          It configures the motor for PWM control mode and applies current settings.
 */
void PWMControlMenuCallback(void)
{
    SEGGER_RTT_printf(0, "UI: Entering PWM Control submenu\r\n");
    
    /* Set motor to PWM control mode */
    motor_config.motor_control_mode = 0;  // 0 = PWM control mode
    
    /* Sync UI variables from current motor config */
    motor_ui_update_from_config();
    
    /* Apply PWM configuration if motor is enabled */
    if (motor_config.output_source > 0) {
        SEGGER_RTT_printf(0, "UI: Applying PWM control - Source: %d, Freq: %.1fHz, Duty: %.1f%%\r\n",
                         motor_config.output_source, motor_config.pwm_frequency, motor_config.pwm_duty_cycle);
        motor_apply_config();
    } else {
        SEGGER_RTT_printf(0, "UI: Motor disabled, PWM mode set but not applied\r\n");
    }
    
    /* Save updated control mode to flash */
    //motor_config_save_to_flash();
}

/**
 * @brief PID Speed Control submenu callback - Apply PID control when entering submenu
 * 
 * @details This function is called when entering the PID Speed Control submenu.
 *          It configures the motor for PID speed control mode and applies current settings.
 */
void PIDSpeedControlMenuCallback(void)
{
    SEGGER_RTT_printf(0, "UI: Entering PID Speed Control submenu\r\n");
    
    /* Set motor to PID control mode */
    motor_config.motor_control_mode = 1;  // 1 = PID speed control mode
    
    /* Sync UI variables from current motor config */
    motor_ui_update_from_config();
    
    /* Log PID settings */
    SEGGER_RTT_printf(0, "UI: PID control settings - Target Speed: %.1f%%, Max RPM: %.0f\r\n",
                     motor_config.pid_target_speed_percent, motor_config.motor_max_rpm);
    
    /* Apply PID configuration if motor is enabled */
    if (motor_config.output_source > 0) {
        SEGGER_RTT_printf(0, "UI: Applying PID speed control\r\n");
        /* Note: PID control implementation would be added in motor_apply_config() */
        motor_apply_config();
    } else {
        SEGGER_RTT_printf(0, "UI: Motor disabled, PID mode set but not applied\r\n");
    }
    
    /* Save updated control mode to flash */
    motor_config_save_to_flash();
}

/* Note: motor_ui_update_config() and motor_ui_save_to_config() functions removed
 * All UI elements now directly reference motor_config fields, eliminating the need for complex synchronization
 * Only motor_ui_update_from_config() remains for syncing display helper variables like frequency kHz conversion
 */

//主菜单的菜单项
MenuItem MainMenuItems[] = {
	{.General_item_text = "Motor Settings",.General_callback = NULL,.General_SubMenuPage = &MotorMenuPage,.Tiles_Icon = Image_motor},
	{.General_item_text = "Overcurrent Protection",.General_callback = NULL,.General_SubMenuPage = &ProtectionMenuPage,.Tiles_Icon = Image_protection},
	{.General_item_text = "Device Info",.General_callback = NULL,.General_SubMenuPage = &DeviceInfoMenuPage,.Tiles_Icon = Image_device_info},
	
	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};

// Motor Settings Tiles Menu Items
MenuItem MotorMenuItems[] = {
	{.General_item_text = "Output Source",.General_callback = NULL,.General_SubMenuPage = &OutputSourceMenuPage,.Tiles_Icon = Image_output_source},
	{.General_item_text = "PWM Control",.General_callback = PWMControlMenuCallback,.General_SubMenuPage = &PWMControlMenuPage,.Tiles_Icon = Image_pwm_control},
	{.General_item_text = "PID Speed Control",.General_callback = PIDSpeedControlMenuCallback,.General_SubMenuPage = &PIDControlMenuPage,.Tiles_Icon = Image_pid_control},
	
	{.General_item_text = NULL}, // Terminator
};

// Output Source Selection Menu Items
MenuItem OutputSourceMenuItems[] = {
	{.General_item_text = "Disabled",.General_callback = SetOutputSourceDisabled,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "STM32",.General_callback = SetOutputSourceSTM32,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "FPGA",.General_callback = SetOutputSourceFPGA,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "[Back]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},

	{.General_item_text = NULL},
};

// PWM Control Menu Items
MenuItem PWMControlMenuItems[] = {
	{.General_item_text = "PWM Frequency",.General_callback = PWMFrequencySettingWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL,.List_FloatBox = &motor_pwm_frequency_khz},
	{.General_item_text = "PWM Duty",.General_callback = PWMDutySettingWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL,.List_FloatBox = &motor_config.pwm_duty_cycle},
	{.General_item_text = "Motor Direction",.General_callback = NULL,.General_SubMenuPage = &MotorDirectionMenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "[Back]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},

	{.General_item_text = NULL},
};

// PID Speed Control Menu Items
MenuItem PIDControlMenuItems[] = {
	{.General_item_text = "Target Speed %",.General_callback = PIDTargetSpeedSettingWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL,.List_FloatBox = &motor_config.pid_target_speed_percent},
	{.General_item_text = "Motor Direction",.General_callback = NULL,.General_SubMenuPage = &PIDDirectionMenuPage,.List_BoolRadioBox = NULL},
	{.General_item_text = "[Back]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},

	{.General_item_text = NULL},
};

//电机方向选择菜单项
MenuItem MotorDirectionMenuItems[] = {
	{.General_item_text = "Clockwise",.General_callback = SetDirectionClockwise,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "Counter-clockwise",.General_callback = SetDirectionCounterclockwise,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "[Back]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},

	{.General_item_text = NULL},
};

//过流保护菜单项内容数组
MenuItem ProtectionMenuItems[] = {
	{.General_item_text = "Protection Enable",.General_callback = ProtectionSaveCallback,.General_SubMenuPage = NULL,.List_BoolRadioBox = &motor_overcurrent_protection_bool},
	{.General_item_text = "Auto Restart",.General_callback = ProtectionSaveCallback,.General_SubMenuPage = NULL,.List_BoolRadioBox = &motor_auto_restart_bool},
	{.General_item_text = "Restart Delay",.General_callback = RestartDelaySettingWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL,.List_IntBox = (int16_t*)&motor_config.auto_restart_delay_ms},
	{.General_item_text = "Max Attempts",.General_callback = MaxRestartAttemptsSettingWindow,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL,.List_IntBox = (int16_t*)&motor_config.max_restart_attempts},
	{.General_item_text = "[Back]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},

	{.General_item_text = NULL}, // Terminator
};

MenuItem DeviceInfoMenuItems[] = {
        {.General_item_text = "-[Author:]",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
        {.General_item_text = " Haoyi Chen",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "-[MCU:]",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = " STM32F407VGT6",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = " RAM:192",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = " ROM:1024KB",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "-[Display:]",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = " SSD1309 128x64 OLED",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
        {.General_item_text = " Motor Drive Chip ",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
        {.General_item_text = " TB6612FNG",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "-[Current Detect:]",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = " INA 169",.General_callback = NULL,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},
	{.General_item_text = "[Back]",.General_callback = OLED_UI_Back,.General_SubMenuPage = NULL,.List_BoolRadioBox = NULL},

	{.General_item_text = NULL},/*最后一项的General_item_text置为NULL，表示该项为分割线*/
};

MenuPage MainMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_TILES,  		 //菜单类型为磁贴类型
	.General_CursorStyle = NOT_SHOW,			 //光标类型
	.General_FontSize = OLED_UI_FONT_16,			//字高
	.General_ParentMenuPage = NULL,				//由于这是根菜单，所以父菜单为NULL
	.General_LineSpace = 5,						//磁贴间距 单位：像素（对于磁贴类型菜单，此值表示每个磁贴之间的间距）
	.General_MoveStyle = PID_CURVE,				//移动方式
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = MainMenuItems,			//菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.Tiles_ScreenHeight = 64,					//屏幕高度
	.Tiles_ScreenWidth = 128,						//屏幕宽度
	.Tiles_TileWidth = 32,						 //磁贴宽度
	.Tiles_TileHeight = 32,						 //磁贴高度
};

MenuPage MotorMenuPage = {
	// Common properties, required
	.General_MenuType = MENU_TYPE_TILES,  		// Tiles menu type
	.General_CursorStyle = NOT_SHOW,			// Cursor type
	.General_FontSize = OLED_UI_FONT_12,		// Font size
	.General_ParentMenuPage = &MainMenuPage,	// Parent menu is main menu
	.General_LineSpace = 5,						// Tile spacing in pixels
	.General_MoveStyle = PID_CURVE,				// Movement animation style
	.General_MovingSpeed = SPEED,				// Animation speed
	.General_ShowAuxiliaryFunction = NULL,		// Auxiliary display function
	.General_MenuItems = MotorMenuItems,		// Menu items array

	// Special properties for tiles menu type
	.Tiles_ScreenHeight = 64,					// Screen height
	.Tiles_ScreenWidth = 128,					// Screen width
	.Tiles_TileWidth = 32,						// Tile width
	.Tiles_TileHeight = 32,						// Tile height
};

MenuPage PWMControlMenuPage = {
	// Common properties, required
	.General_MenuType = MENU_TYPE_LIST,  		// List menu type
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	// Cursor style
	.General_FontSize = OLED_UI_FONT_12,		// Font size
	.General_ParentMenuPage = &MotorMenuPage,	// Parent menu is motor menu
	.General_LineSpace = 4,						// Line spacing in pixels
	.General_MoveStyle = UNLINEAR,				// Non-linear animation
	.General_MovingSpeed = SPEED,				// Animation speed
	.General_ShowAuxiliaryFunction = NULL,		// Auxiliary display function
	.General_MenuItems = PWMControlMenuItems,	// Menu items array

	// Special properties for list menu type
	.List_MenuArea = {0, 0, 128, 64},			// List display area
	.List_IfDrawFrame = false,					// Whether to draw frame
	.List_IfDrawLinePerfix = true,				// Whether to draw line prefix
	.List_StartPointX = 4,                      // List start X coordinate
	.List_StartPointY = 2,                      // List start Y coordinate
};

MenuPage PIDControlMenuPage = {
	// Common properties, required
	.General_MenuType = MENU_TYPE_LIST,  		// List menu type
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	// Cursor style
	.General_FontSize = OLED_UI_FONT_12,		// Font size
	.General_ParentMenuPage = &MotorMenuPage,	// Parent menu is motor menu
	.General_LineSpace = 4,						// Line spacing in pixels
	.General_MoveStyle = UNLINEAR,				// Non-linear animation
	.General_MovingSpeed = SPEED,				// Animation speed
	.General_ShowAuxiliaryFunction = NULL,		// Auxiliary display function
	.General_MenuItems = PIDControlMenuItems,	// Menu items array

	// Special properties for list menu type
	.List_MenuArea = {0, 0, 128, 64},			// List display area
	.List_IfDrawFrame = false,					// Whether to draw frame
	.List_IfDrawLinePerfix = true,				// Whether to draw line prefix
	.List_StartPointX = 4,                      // List start X coordinate
	.List_StartPointY = 2,                      // List start Y coordinate
};

MenuPage ProtectionMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	 //光标类型为线型
	.General_FontSize = OLED_UI_FONT_12,			//字高
	.General_ParentMenuPage = &MainMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 4,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = ProtectionMenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {0, 0, 128, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = true,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标
};

MenuPage DeviceInfoMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_BLOCK,	 //光标类型为圆角矩形
	.General_FontSize = OLED_UI_FONT_12,			//字高
	.General_ParentMenuPage = &MainMenuPage,		 //父菜单为主菜单
	.General_LineSpace = 4,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = DeviceInfoMenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {0, 0, 128, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = false,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标
};

MenuPage OutputSourceMenuPage = {
	//通用属性，必填
	.General_MenuType = MENU_TYPE_LIST,  		 //菜单类型为列表类型
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	 //光标类型为圆角矩形
	.General_FontSize = OLED_UI_FONT_12,			//字高
	.General_ParentMenuPage = &MotorMenuPage,		 //父菜单为电机菜单
	.General_LineSpace = 4,						//行间距 单位：像素
	.General_MoveStyle = UNLINEAR,				//移动方式为非线性曲线动画
	.General_MovingSpeed = SPEED,					//动画移动速度(此值根据实际效果调整)
	.General_ShowAuxiliaryFunction = NULL,		 //显示辅助函数
	.General_MenuItems = OutputSourceMenuItems,		 //菜单项内容数组

	//特殊属性，根据.General_MenuType的类型选择
	.List_MenuArea = {0, 0, 128, 64},			 //列表显示区域
	.List_IfDrawFrame = false,					 //是否显示边框
	.List_IfDrawLinePerfix = false,				 //是否显示行前缀
	.List_StartPointX = 4,                        //列表起始点X坐标
	.List_StartPointY = 2,                        //列表起始点Y坐标
};

MenuPage MotorDirectionMenuPage = {
	// Common properties, required
	.General_MenuType = MENU_TYPE_LIST,  		// List menu type
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	// Round rectangle cursor
	.General_FontSize = OLED_UI_FONT_12,		// Font size
	.General_ParentMenuPage = &PWMControlMenuPage,	// Parent is PWM control menu
	.General_LineSpace = 4,						// Line spacing in pixels
	.General_MoveStyle = UNLINEAR,				// Non-linear animation
	.General_MovingSpeed = SPEED,				// Animation speed
	.General_ShowAuxiliaryFunction = NULL,		// Auxiliary display function
	.General_MenuItems = MotorDirectionMenuItems,	// Menu items array

	// Special properties for list menu type
	.List_MenuArea = {0, 0, 128, 64},			// List display area
	.List_IfDrawFrame = false,					// Whether to draw frame
	.List_IfDrawLinePerfix = false,				// Whether to draw line prefix
	.List_StartPointX = 4,                      // List start X coordinate
	.List_StartPointY = 2,                      // List start Y coordinate
};

MenuPage PIDDirectionMenuPage = {
	// Common properties, required
	.General_MenuType = MENU_TYPE_LIST,  		// List menu type
	.General_CursorStyle = REVERSE_ROUNDRECTANGLE,	// Round rectangle cursor
	.General_FontSize = OLED_UI_FONT_12,		// Font size
	.General_ParentMenuPage = &PIDControlMenuPage,	// Parent is PID control menu
	.General_LineSpace = 4,						// Line spacing in pixels
	.General_MoveStyle = UNLINEAR,				// Non-linear animation
	.General_MovingSpeed = SPEED,				// Animation speed
	.General_ShowAuxiliaryFunction = NULL,		// Auxiliary display function
	.General_MenuItems = MotorDirectionMenuItems,	// Menu items array

	// Special properties for list menu type
	.List_MenuArea = {0, 0, 128, 64},			// List display area
	.List_IfDrawFrame = false,					// Whether to draw frame
	.List_IfDrawLinePerfix = false,				// Whether to draw line prefix
	.List_StartPointX = 4,                      // List start X coordinate
	.List_StartPointY = 2,                      // List start Y coordinate
};
