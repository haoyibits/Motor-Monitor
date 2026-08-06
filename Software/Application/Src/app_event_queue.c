/**
 ******************************************************************************
 * @file           : app_event_queue.c
 * @brief          : Application event queue implementation
 ******************************************************************************
 */

#include "app_event_queue.h"

#include <stddef.h>

void app_event_queue_init(App_Event_Queue_t *queue)
{
    if (queue == NULL) {
        return;
    }

    queue->head = 0U;
    queue->tail = 0U;
    queue->count = 0U;
    queue->overflow_count = 0U;
}

bool app_event_queue_push(App_Event_Queue_t *queue, App_Event_t event)
{
    if (queue == NULL) {
        return false;
    }

    if (queue->count >= APP_EVENT_QUEUE_CAPACITY) {
        queue->overflow_count++;
        return false;
    }

    queue->events[queue->head] = event;
    queue->head = (uint8_t)((queue->head + 1U) % APP_EVENT_QUEUE_CAPACITY);
    queue->count++;
    return true;
}

bool app_event_queue_pop(App_Event_Queue_t *queue, App_Event_t *event)
{
    if (queue == NULL || event == NULL || queue->count == 0U) {
        return false;
    }

    *event = queue->events[queue->tail];
    queue->tail = (uint8_t)((queue->tail + 1U) % APP_EVENT_QUEUE_CAPACITY);
    queue->count--;
    return true;
}

uint32_t app_event_queue_overflow_count(const App_Event_Queue_t *queue)
{
    return queue == NULL ? 0U : queue->overflow_count;
}
