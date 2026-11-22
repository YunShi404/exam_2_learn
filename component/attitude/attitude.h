//
// Created by 30547 on 2026/11/22.
//

#ifndef EXAM_2_LEARN_ATTITUDE_H
#define EXAM_2_LEARN_ATTITUDE_H

typedef struct
{
    float roll;
    float pitch;
    float yaw;

    float q[4];
    float integral[3];
} Attitude;

void Attitude_Reset(Attitude *attitude);
void Attitude_Init(Attitude *attitude);
void Attitude_Update(Attitude *attitude,
                     const float gyro_dps[3],
                     const float accel_g[3],
                     float dt);

#endif //EXAM_2_LEARN_ATTITUDE_H
