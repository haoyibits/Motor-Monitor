#include <assert.h>
#include <stdint.h>

#include "app_event_queue.h"
#include "motor_state.h"

static void test_event_queue_fifo_and_overflow(void)
{
    App_Event_Queue_t queue;
    App_Event_t event;

    app_event_queue_init(&queue);
    for (uint8_t i = 0U; i < APP_EVENT_QUEUE_CAPACITY; ++i) {
        assert(app_event_queue_push(&queue, (App_Event_t){.type = APP_EVENT_UI_TICK}));
    }
    assert(!app_event_queue_push(&queue, (App_Event_t){.type = APP_EVENT_CURRENT_SAMPLE}));
    assert(app_event_queue_overflow_count(&queue) == 1U);

    for (uint8_t i = 0U; i < APP_EVENT_QUEUE_CAPACITY; ++i) {
        assert(app_event_queue_pop(&queue, &event));
        assert(event.type == APP_EVENT_UI_TICK);
    }
    assert(!app_event_queue_pop(&queue, &event));
}

static void test_motor_state_transitions_and_tick_wrap(void)
{
    Motor_Current_Monitor_t monitor;

    motor_state_init(&monitor, 3200U);
    assert(monitor.state == MOTOR_STATE_STOPPED);

    motor_state_start(&monitor, UINT32_MAX - 999U, 3800U);
    motor_state_update(&monitor, 500U, 2000U, 3200U);
    assert(monitor.state == MOTOR_STATE_STARTING);
    motor_state_update(&monitor, 1001U, 2000U, 3200U);
    assert(monitor.state == MOTOR_STATE_RUNNING);
    assert(monitor.current_threshold == 3200U);

    motor_state_stop(&monitor, 3200U);
    assert(monitor.state == MOTOR_STATE_STOPPED);
}

int main(void)
{
    test_event_queue_fifo_and_overflow();
    test_motor_state_transitions_and_tick_wrap();
    return 0;
}
