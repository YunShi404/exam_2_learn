//
// Created by 30547 on 2026/11/18.
//

#ifndef EXAM_2_LEARN_IMU_H
#define EXAM_2_LEARN_IMU_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    float accel_g[3];
    float gyro_dps[3];
} BMI088_Data_t;

typedef enum
{
    ACC = 1,
    GYRO = 2,
}BMI088_Device;

int BMI088_Transmit(BMI088_Device device, uint8_t address, uint8_t choice);
int BMI088_Read(uint8_t device, uint8_t address);
int BMI088_update(BMI088_Data_t *output);
int BMI088_init(void);

#endif //EXAM_2_LEARN_IMU_H
