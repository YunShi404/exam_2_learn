//
// Created by 30547 on 2026/11/9.
//
#include "main.h"
#include "buzzer.h"
#include "tim.h"
#include "assert.h"
#include "led.h"

void buzzer(int Hz, uint8_t volume)
{
    if(Hz <= 1000000 && Hz >= 16 && volume <= 100 )//uint8_t本身就大于等于0
    {
        HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
        __HAL_TIM_SET_AUTORELOAD(&htim12,1000000/Hz);
        __HAL_TIM_SET_COMPARE(&htim12, TIM_CHANNEL_2, (1000000.0/Hz)/2*volume/100);
    }
    else
    {
        HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2);
        LED(0,230,0);
    }
}
