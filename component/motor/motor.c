//
// Created by 30547 on 2026/11/20.
//
#include "FreeRTOS.h"
#include "motor.h"
#include"can.h"
#include"led.h"
#include"task.h"




static GM6020_motor_status motor_status_GM6020[GM6020_MOTOR_NUM + 1];

motor_set GM6020 = {0x200, 0x7F0};

GM6020_motor_status error = {1,2,3,4};

void can_recieve_motor(uint16_t id, const uint8_t *data, uint8_t length)
{
    if (id >= 0x205 && id <= 0x20B)
    {
        motor_status_GM6020[id - 0x204].angle = data[0] << 8 | data[1];
        motor_status_GM6020[id - 0x204].speed_rpm = data[2] << 8 | data[3];
        motor_status_GM6020[id - 0x204].current_raw = data[4] << 8 | data[5];
        motor_status_GM6020[id - 0x204].temperature = data[6];
        motor_status_GM6020[id - 0x204].tick = HAL_GetTick();
    }


    // if (id == 0x205)
    // {
    //     motor_status_GM6020[1].angle = data[0] << 8 | data[1];
    //     motor_status_GM6020[1].speed_rpm = data[2] << 8 | data[3];
    //     motor_status_GM6020[1].current_raw = data[4] << 8 | data[5];
    //     motor_status_GM6020[1].temperature = data[6];
    // }
    // if (id == 0x206)
    // {
    //     motor_status_GM6020[2].angle = data[0] << 8 | data[1];
    //     motor_status_GM6020[2].speed_rpm = data[2] << 8 | data[3];
    //     motor_status_GM6020[2].current_raw = data[4] << 8 | data[5];
    //     motor_status_GM6020[2].temperature = data[6];
    // }
    // if (id == 0x207)
    // {
    //     motor_status_GM6020[3].angle = data[0] << 8 | data[1];
    //     motor_status_GM6020[3].speed_rpm = data[2] << 8 | data[3];
    //     motor_status_GM6020[3].current_raw = data[4] << 8 | data[5];
    //     motor_status_GM6020[3].temperature = data[6];
    // }
    // if (id == 0x208)
    // {
    //     motor_status_GM6020[4].angle = data[0] << 8 | data[1];
    //     motor_status_GM6020[4].speed_rpm = data[2] << 8 | data[3];
    //     motor_status_GM6020[4].current_raw = data[4] << 8 | data[5];
    //     motor_status_GM6020[4].temperature = data[6];
    // }
}

int GM6020_Get_Status(uint8_t motor_id, GM6020_motor_status *output)
{
    if (motor_id < 1 || motor_id > GM6020_MOTOR_NUM || output == NULL)
    {
        LED(200,0,200);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return -1;
    }
    else
    {
        if (HAL_GetTick() - motor_status_GM6020[motor_id].tick <= 100)
        {
            motor_status_GM6020[motor_id].online = 1;
            taskENTER_CRITICAL();
            *output = motor_status_GM6020[motor_id];
            taskEXIT_CRITICAL();
            return 1;
        }
        else
        {
            *output = error;
            motor_status_GM6020[motor_id].online = 0;
            LED(200,0,200);
            return -2;
        }
    }
}



int Motor_Init(CAN_ID can_id, FDCAN_HandleTypeDef *hfdcan, motor_set motor_set)
{
    int sign = CAN_Init_A(can_id, hfdcan, motor_set.filter_goal_id, motor_set.filter_mask, can_recieve_motor);
    if (sign != 1)
    {
        LED(200,0,200);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    }
    return sign;
}



int motor_control(CAN_ID can_id, data_id data_id, int16_t data1, int16_t data2, int16_t data3, int16_t data4)
{
    if (data_id == GM6020_V_control_1234 || data_id == GM6020_V_control_567)
    {
        if (data1 <= 25000 && data2 <= 25000 && data3 <= 25000 && data4 <= 25000 && data1 >= -25000 && data2 >= -25000 && data3 >= -25000 && data4 >= -25000)
        {
            uint8_t control_data[8];
            control_data[0] = data1 >> 8;
            control_data[1] = data1;
            control_data[2] = data2 >> 8;
            control_data[3] = data2;
            control_data[4] = data3 >> 8;
            control_data[5] = data3;
            control_data[6] = data4 >> 8;
            control_data[7] = data4;
            int sign = CAN_Transmit_classic(data_id, control_data, 8, can_id);
            if (sign != 1)
            {
                LED(200,0,200);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                return sign - 10;
            }
            return 1;
        }
        else
        {
            LED(200,0,200);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
            return -1;
        }
    }

    if (data_id == GM6020_I_control_1234 || data_id == GM6020_I_control_567)
    {
        if (data1 <= 16384 && data2 <= 16384 && data3 <= 16384 && data4 <= 16384 && data1 >= -16384 && data2 >= -16384 && data3 >= -16384 && data4 >= -16384)
        {
            uint8_t control_data[8];
            control_data[0] = data1 >> 8;
            control_data[1] = data1;
            control_data[2] = data2 >> 8;
            control_data[3] = data2;
            control_data[4] = data3 >> 8;
            control_data[5] = data3;
            control_data[6] = data4 >> 8;
            control_data[7] = data4;
            int sign = CAN_Transmit_classic(data_id, control_data, 8, can_id);
            if (sign != 1)
            {
                LED(200,0,200);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                return sign - 20;
            }
            return 1;
        }
        else
        {
            LED(200,0,200);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
            return -2;
        }
    }
    return -3;
}
