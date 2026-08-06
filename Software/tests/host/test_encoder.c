#include <assert.h>
#include <stdint.h>

#include "encoder.h"

int main(void)
{
    TIM_TypeDef timer = {0};
    Encoder_HandleTypeDef handle;
    const Encoder_InitTypeDef init = {
        .TIMx = &timer,
        .CountsPerRevolution = 1000U,
        .IC1Polarity = ENCODER_IC_POLARITY_RISING,
        .IC2Polarity = ENCODER_IC_POLARITY_RISING,
        .MaxCount = 0xFFFFU
    };

    assert(encoder_init(&handle, &init) == DRIVER_STATUS_OK);
    assert(timer.ARR == 0xFFFFU);

    timer.CNT = 65530U;
    assert(encoder_calculate_speed_rpm(&handle, 100U) == 0);

    timer.CNT = 5U;
    assert(encoder_calculate_speed_rpm(&handle, 200U) == -6);
    assert(handle.TotalCount == -11);
    assert(encoder_get_direction(&handle) == -1);

    timer.CNT = 65530U;
    assert(encoder_calculate_speed_rpm(&handle, 300U) == 6);
    assert(handle.TotalCount == 0);
    assert(encoder_get_direction(&handle) == 1);

    assert(encoder_calculate_speed_rpm(&handle, 400U) == 0);
    assert(encoder_get_direction(&handle) == 0);
    return 0;
}
