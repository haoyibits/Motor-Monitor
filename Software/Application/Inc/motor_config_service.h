/**
 ******************************************************************************
 * @file           : motor_config_service.h
 * @brief          : Transactional motor configuration editing
 ******************************************************************************
 */

#ifndef MOTOR_CONFIG_SERVICE_H
#define MOTOR_CONFIG_SERVICE_H

#include <stdbool.h>

#include "motor.h"

typedef struct {
    Motor_Config_t draft;
    bool active;
} Motor_Config_Transaction_t;

void motor_config_transaction_begin(Motor_Config_Transaction_t *transaction);
Motor_Config_t *motor_config_transaction_draft(Motor_Config_Transaction_t *transaction);
bool motor_config_transaction_commit(Motor_Config_Transaction_t *transaction,
                                     bool apply_immediately);
void motor_config_transaction_cancel(Motor_Config_Transaction_t *transaction);
bool motor_config_values_valid(const Motor_Config_t *config);

#endif /* MOTOR_CONFIG_SERVICE_H */
