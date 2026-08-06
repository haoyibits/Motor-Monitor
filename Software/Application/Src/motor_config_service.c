/**
 ******************************************************************************
 * @file           : motor_config_service.c
 * @brief          : Transactional motor configuration editing
 ******************************************************************************
 */

#include "motor_config_service.h"

#include <stddef.h>

bool motor_config_values_valid(const Motor_Config_t *config)
{
    if (config == NULL ||
        config->magic_number != MOTOR_CONFIG_MAGIC_NUMBER ||
        config->pwm_frequency < 100.0f || config->pwm_frequency > 50000.0f ||
        config->pwm_duty_cycle < 0.0f || config->pwm_duty_cycle > 100.0f ||
        config->output_source > 2U ||
        config->overcurrent_protection > 1U ||
        config->motor_direction > 1U ||
        config->auto_restart_enable > 1U ||
        config->auto_restart_delay_ms < 1000U || config->auto_restart_delay_ms > 60000U ||
        config->max_restart_attempts < 1U || config->max_restart_attempts > 10U ||
        config->motor_control_mode > 1U ||
        config->pid_target_speed_percent < 0.0f || config->pid_target_speed_percent > 100.0f ||
        config->motor_max_rpm < 100.0f || config->motor_max_rpm > 50000.0f) {
        return false;
    }

    return true;
}

void motor_config_transaction_begin(Motor_Config_Transaction_t *transaction)
{
    if (transaction == NULL) {
        return;
    }

    transaction->draft = motor_config;
    transaction->active = true;
}

Motor_Config_t *motor_config_transaction_draft(Motor_Config_Transaction_t *transaction)
{
    if (transaction == NULL || !transaction->active) {
        return NULL;
    }

    return &transaction->draft;
}

bool motor_config_transaction_commit(Motor_Config_Transaction_t *transaction,
                                     bool apply_immediately)
{
    if (transaction == NULL || !transaction->active ||
        !motor_config_values_valid(&transaction->draft)) {
        return false;
    }

    motor_config = transaction->draft;
    transaction->active = false;

    if (!motor_config_save_to_flash()) {
        return false;
    }

    if (apply_immediately) {
        motor_apply_config();
    }

    return true;
}

void motor_config_transaction_cancel(Motor_Config_Transaction_t *transaction)
{
    if (transaction == NULL) {
        return;
    }

    transaction->active = false;
}
