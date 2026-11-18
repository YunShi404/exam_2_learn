//
// Created by 30547 on 2026/11/18.
//

#ifndef EXAM_2_LEARN_IMU_H
#define EXAM_2_LEARN_IMU_H
#include "stdint.h"


extern int16_t ACC_origin_data[3];
extern int16_t GYRO_origin_data[3];
extern float ACC_data_g[3];
extern float GYRO_data_dps[3];

void BMI088_Transmit(uint8_t device, uint8_t address, uint8_t choice);
void BMI088_Read(uint8_t device, uint8_t address);
void BMI088_update(void);
void BMI088_init(void);
#endif //EXAM_2_LEARN_IMU_H
