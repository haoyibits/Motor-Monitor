/**
 ******************************************************************************
 * @file           : app_event_queue.h
 * @brief          : Application event queue
 ******************************************************************************
 */

#ifndef APP_EVENT_QUEUE_H
#define APP_EVENT_QUEUE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    APP_EVENT_NONE = 0,
    APP_EVENT_ENCODER_SAMPLE,
    APP_EVENT_CURRENT_SAMPLE,
    APP_EVENT_UI_TICK,
    APP_EVENT_MOTOR_OVERCURRENT,
    APP_EVENT_MOTOR_RESTARTING,
    APP_EVENT_MOTOR_MAX_RESTARTS
} App_Event_Type_t;

typedef struct {
    App_Event_Type_t type;
} App_Event_t;

#define APP_EVENT_QUEUE_CAPACITY 8U

typedef struct {
    App_Event_t events[APP_EVENT_QUEUE_CAPACITY];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
    uint32_t overflow_count;
} App_Event_Queue_t;

void app_event_queue_init(App_Event_Queue_t *queue);
bool app_event_queue_push(App_Event_Queue_t *queue, App_Event_t event);
bool app_event_queue_pop(App_Event_Queue_t *queue, App_Event_t *event);
uint32_t app_event_queue_overflow_count(const App_Event_Queue_t *queue);

#endif /* APP_EVENT_QUEUE_H */
