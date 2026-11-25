//
// Created by 30547 on 2026/11/20.
//

#ifndef EXAM_2_LEARN_MOTOR_H
#define EXAM_2_LEARN_MOTOR_H
#include <stdint.h>
#include "can.h"

#define GM6020_filter_goal_id 0x200
#define GM6020_filter_mask 0x7F0

#define GM6020_MOTOR_NUM 7

typedef struct
{
    uint16_t angle;
    int16_t speed_rpm;
    int16_t current_raw;
    uint8_t temperature;
}GM6020_motor_status;

typedef struct
{
    uint16_t filter_goal_id;
    uint16_t filter_mask;
}motor;

extern motor GM6020;

typedef enum
{
    GM6020_V_control_1234 = 0x1FF,
    GM6020_V_control_567 = 0x2FF,
    GM6020_I_control_1234 = 0x1FE,
    GM6020_I_control_567 = 0x2FE,
}data_id;

int GM6020_Get_Status(uint8_t motor_id, GM6020_motor_status *output);
int Motor_Init(CAN_ID can_id, FDCAN_HandleTypeDef *hfdcan, motor motor);
int motor_control(data_id data_id, int16_t data1, int16_t data2, int16_t data3, int16_t data4);


#endif //EXAM_2_LEARN_MOTOR_H
