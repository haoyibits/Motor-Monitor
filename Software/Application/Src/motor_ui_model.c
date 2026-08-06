/**
 ******************************************************************************
 * @file           : motor_ui_model.c
 * @brief          : Motor UI view model and application actions
 ******************************************************************************
 */

#include "motor_ui_model.h"

#include "motor.h"
#include "motor_config_service.h"

#include <stddef.h>

Motor_UI_Model_t motor_ui_model;

static Motor_Config_Transaction_t config_transaction;
static Motor_UI_Field_t active_field = MOTOR_UI_FIELD_NONE;

void motor_ui_update_from_config(void)
{
    motor_ui_model.pwm_frequency_khz = motor_config.pwm_frequency / 1000.0f;
    motor_ui_model.pwm_duty_percent = motor_config.pwm_duty_cycle;
    motor_ui_model.pid_target_speed_percent = motor_config.pid_target_speed_percent;
    motor_ui_model.restart_delay_ms = motor_config.auto_restart_delay_ms;
    motor_ui_model.max_restart_attempts = motor_config.max_restart_attempts;
    motor_ui_model.overcurrent_protection = motor_config.overcurrent_protection != 0U;
    motor_ui_model.auto_restart = motor_config.auto_restart_enable != 0U;
}

void motor_ui_begin_edit(Motor_UI_Field_t field)
{
    motor_config_transaction_begin(&config_transaction);
    active_field = field;
    motor_ui_update_from_config();
}

bool motor_ui_confirm_edit(void)
{
    Motor_Config_t *draft = motor_config_transaction_draft(&config_transaction);
    if (draft == NULL) {
        return false;
    }

    switch (active_field) {
        case MOTOR_UI_FIELD_PWM_FREQUENCY:
            draft->pwm_frequency = motor_ui_model.pwm_frequency_khz * 1000.0f;
            break;
        case MOTOR_UI_FIELD_PWM_DUTY:
            draft->pwm_duty_cycle = motor_ui_model.pwm_duty_percent;
            break;
        case MOTOR_UI_FIELD_RESTART_DELAY:
            draft->auto_restart_delay_ms = (uint16_t)motor_ui_model.restart_delay_ms;
            break;
        case MOTOR_UI_FIELD_MAX_RESTART_ATTEMPTS:
            draft->max_restart_attempts = (uint8_t)motor_ui_model.max_restart_attempts;
            break;
        case MOTOR_UI_FIELD_PID_TARGET_SPEED:
            draft->pid_target_speed_percent = motor_ui_model.pid_target_speed_percent;
            break;
        case MOTOR_UI_FIELD_NONE:
        default:
            motor_config_transaction_cancel(&config_transaction);
            return false;
    }

    const bool committed = motor_config_transaction_commit(&config_transaction, false);
    active_field = MOTOR_UI_FIELD_NONE;
    motor_ui_update_from_config();
    return committed;
}

void motor_ui_cancel_edit(void)
{
    motor_config_transaction_cancel(&config_transaction);
    active_field = MOTOR_UI_FIELD_NONE;
    motor_ui_update_from_config();
}

bool motor_ui_commit_protection(void)
{
    motor_config_transaction_begin(&config_transaction);
    Motor_Config_t *draft = motor_config_transaction_draft(&config_transaction);
    draft->overcurrent_protection = motor_ui_model.overcurrent_protection ? 1U : 0U;
    draft->auto_restart_enable = motor_ui_model.auto_restart ? 1U : 0U;
    return motor_config_transaction_commit(&config_transaction, false);
}

bool motor_ui_set_output_source(uint8_t output_source)
{
    motor_config_transaction_begin(&config_transaction);
    motor_config_transaction_draft(&config_transaction)->output_source = output_source;
    const bool committed = motor_config_transaction_commit(&config_transaction, true);
    motor_ui_update_from_config();
    return committed;
}

bool motor_ui_set_direction(uint8_t direction)
{
    motor_config_transaction_begin(&config_transaction);
    motor_config_transaction_draft(&config_transaction)->motor_direction = direction;
    const bool committed = motor_config_transaction_commit(&config_transaction, false);
    motor_ui_update_from_config();
    return committed;
}

bool motor_ui_set_control_mode(uint8_t control_mode)
{
    if (motor_config.motor_control_mode == control_mode) {
        return true;
    }

    motor_config_transaction_begin(&config_transaction);
    motor_config_transaction_draft(&config_transaction)->motor_control_mode = control_mode;
    const bool committed = motor_config_transaction_commit(&config_transaction, false);
    motor_ui_update_from_config();
    return committed;
}
