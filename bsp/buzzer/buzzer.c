//
// Created by 30547 on 2026/11/9.
//
#include "main.h"
#include "buzzer.h"
#include "tim.h"
#include "assert.h"
void buzzer(int x, uint8_t y)
{
    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
    assert (x <= 1000000 && x >= 16 && y <= 100 );//uint8_t本身就大于等于0
    __HAL_TIM_SET_AUTORELOAD(&htim12,1000000/x);
    __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, (1000000.0/x)/2*y/100);
}
