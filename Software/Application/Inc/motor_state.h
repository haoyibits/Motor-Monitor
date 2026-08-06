/**
 ******************************************************************************
 * @file           : motor_state.h
 * @brief          : Pure motor runtime state transitions
 ******************************************************************************
 */

#ifndef MOTOR_STATE_H
#define MOTOR_STATE_H

#include <stdint.h>

typedef enum {
    MOTOR_STATE_STOPPED = 0,
    MOTOR_STATE_STARTING,
    MOTOR_STATE_RUNNING
} Motor_State_t;

typedef struct {
    Motor_State_t state;
    uint32_t startup_start_time;
    uint8_t overcurrent_count;
    uint16_t current_threshold;
} Motor_Current_Monitor_t;

void motor_state_init(Motor_Current_Monitor_t *monitor, uint16_t running_threshold);
void motor_state_start(Motor_Current_Monitor_t *monitor,
                       uint32_t now_ms,
                       uint16_t startup_threshold);
void motor_state_stop(Motor_Current_Monitor_t *monitor, uint16_t running_threshold);
void motor_state_update(Motor_Current_Monitor_t *monitor,
                        uint32_t now_ms,
                        uint32_t startup_timeout_ms,
                        uint16_t running_threshold);

#endif /* MOTOR_STATE_H */
