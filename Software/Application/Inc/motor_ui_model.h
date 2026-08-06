/**
 ******************************************************************************
 * @file           : motor_ui_model.h
 * @brief          : Motor UI view model and application actions
 ******************************************************************************
 */

#ifndef MOTOR_UI_MODEL_H
#define MOTOR_UI_MODEL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float pwm_frequency_khz;
    float pwm_duty_percent;
    float pid_target_speed_percent;
    int32_t restart_delay_ms;
    int32_t max_restart_attempts;
    bool overcurrent_protection;
    bool auto_restart;
} Motor_UI_Model_t;

typedef enum {
    MOTOR_UI_FIELD_NONE = 0,
    MOTOR_UI_FIELD_PWM_FREQUENCY,
    MOTOR_UI_FIELD_PWM_DUTY,
    MOTOR_UI_FIELD_RESTART_DELAY,
    MOTOR_UI_FIELD_MAX_RESTART_ATTEMPTS,
    MOTOR_UI_FIELD_PID_TARGET_SPEED
} Motor_UI_Field_t;

extern Motor_UI_Model_t motor_ui_model;

void motor_ui_update_from_config(void);
void motor_ui_begin_edit(Motor_UI_Field_t field);
bool motor_ui_confirm_edit(void);
void motor_ui_cancel_edit(void);
bool motor_ui_commit_protection(void);
bool motor_ui_set_output_source(uint8_t output_source);
bool motor_ui_set_direction(uint8_t direction);
bool motor_ui_set_control_mode(uint8_t control_mode);

#endif /* MOTOR_UI_MODEL_H */
