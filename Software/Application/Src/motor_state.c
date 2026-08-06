/**
 ******************************************************************************
 * @file           : motor_state.c
 * @brief          : Pure motor runtime state transitions
 ******************************************************************************
 */

#include "motor_state.h"

#include <stddef.h>

void motor_state_init(Motor_Current_Monitor_t *monitor, uint16_t running_threshold)
{
    if (monitor == NULL) {
        return;
    }

    monitor->state = MOTOR_STATE_STOPPED;
    monitor->startup_start_time = 0U;
    monitor->overcurrent_count = 0U;
    monitor->current_threshold = running_threshold;
}

void motor_state_start(Motor_Current_Monitor_t *monitor,
                       uint32_t now_ms,
                       uint16_t startup_threshold)
{
    if (monitor == NULL) {
        return;
    }

    monitor->state = MOTOR_STATE_STARTING;
    monitor->startup_start_time = now_ms;
    monitor->overcurrent_count = 0U;
    monitor->current_threshold = startup_threshold;
}

void motor_state_stop(Motor_Current_Monitor_t *monitor, uint16_t running_threshold)
{
    if (monitor == NULL) {
        return;
    }

    monitor->state = MOTOR_STATE_STOPPED;
    monitor->overcurrent_count = 0U;
    monitor->current_threshold = running_threshold;
}

void motor_state_update(Motor_Current_Monitor_t *monitor,
                        uint32_t now_ms,
                        uint32_t startup_timeout_ms,
                        uint16_t running_threshold)
{
    if (monitor == NULL) {
        return;
    }

    switch (monitor->state) {
        case MOTOR_STATE_STOPPED:
            monitor->overcurrent_count = 0U;
            break;

        case MOTOR_STATE_STARTING:
            if ((uint32_t)(now_ms - monitor->startup_start_time) >= startup_timeout_ms) {
                monitor->state = MOTOR_STATE_RUNNING;
                monitor->current_threshold = running_threshold;
                monitor->overcurrent_count = 0U;
            }
            break;

        case MOTOR_STATE_RUNNING:
            break;

        default:
            motor_state_stop(monitor, running_threshold);
            break;
    }
}
