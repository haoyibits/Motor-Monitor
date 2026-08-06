#ifndef __OLED_UI_MENUDATA_H
#define __OLED_UI_MENUDATA_H
// 检测是否是C++编译器
#ifdef __cplusplus
extern "C" {
#endif
#include "OLED_UI.h"

//进行前置声明
extern MenuItem MainMenuItems[],MotorMenuItems[],PWMControlMenuItems[],PIDControlMenuItems[],OutputSourceMenuItems[],MotorDirectionMenuItems[],ProtectionMenuItems[],DeviceInfoMenuItems[],
MaxRestartDecisionMenuItems[],AboutOLED_UIMenuItems[],DrawMenuItems[],MoreMenuItems[],Font8MenuItems[] ,Font12MenuItems[] ,
Font16MenuItems[] ,Font20MenuItems[],LongMenuItems[],SpringMenuItems[],LongListMenuItems[],SmallAreaMenuItems[];

extern MenuPage MainMenuPage,MotorMenuPage,PWMControlMenuPage,PIDControlMenuPage,OutputSourceMenuPage,MotorDirectionMenuPage,PIDDirectionMenuPage,ProtectionMenuPage,DeviceInfoMenuPage,
MaxRestartDecisionMenuPage,AboutOLED_UIMenuPage,DrawMenuPage,MoreMenuPage,Font8MenuPage,Font12MenuPage,Font16MenuPage,
Font20MenuPage,LongMenuPage,SpringMenuPage,LongListMenuPage,SmallAreaMenuPage;

/**
 * @brief Update UI variables from motor configuration
 */
void motor_ui_update_from_config(void);

/**
 * @brief Protection settings save callback
 */
void ProtectionSaveCallback(void);

/**
 * @brief Output source selection callback functions
 */
void SetOutputSourceDisabled(void);
void SetOutputSourceSTM32(void);
void SetOutputSourceFPGA(void);

/**
 * @brief Motor direction selection callback functions
 */
void SetDirectionClockwise(void);
void SetDirectionCounterclockwise(void);

/**
 * @brief Overcurrent protection popup functions
 */
void ShowOvercurrentFaultPopup(void);
void ShowMotorRestartPopup(void);
void ShowMaxAttemptsReachedPopup(void);

/**
 * @brief User decision callback functions for motor restart
 */
void UserDecisionRetryMotor(void);
void UserDecisionCancelMotor(void);

/**
 * @brief PID target speed setting window
 */
void PIDTargetSpeedSettingWindow(void);

/**
 * @brief Motor control submenu callback functions
 */
void PWMControlMenuCallback(void);
void PIDSpeedControlMenuCallback(void);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif
