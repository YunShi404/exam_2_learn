//
// Created by 30547 on 2026/11/22.
//

#include "attitude.h"

#include <math.h>
#include <stddef.h>

#include "cmsis_os2.h"
#include "imu.h"
#include "led.h"

#define RAD_TO_DEG      57.2957795f
#define DEG_TO_RAD      0.0174532925f
#define MAHONY_TWO_KP   1.0f
#define MAHONY_TWO_KI   0.05f

static float gyro_bias[3];

void Attitude_Reset(Attitude *attitude)
{
    if (attitude == NULL)
    {
        LED(0,200,200);
        return;
    }

    attitude->roll = 0.0f;
    attitude->pitch = 0.0f;
    attitude->yaw = 0.0f;

    attitude->q[0] = 1.0f;
    attitude->q[1] = 0.0f;
    attitude->q[2] = 0.0f;
    attitude->q[3] = 0.0f;

    attitude->integral[0] = 0.0f;
    attitude->integral[1] = 0.0f;
    attitude->integral[2] = 0.0f;
}

void Attitude_Init(Attitude *attitude)
{
    BMI088_Data_t sample;
    uint16_t valid_count = 0;

    Attitude_Reset(attitude);

    gyro_bias[0] = 0.0f;
    gyro_bias[1] = 0.0f;
    gyro_bias[2] = 0.0f;

    for (uint16_t i = 0; i < 100; i++)
    {
        if (BMI088_update(&sample))
        {
            gyro_bias[0] += sample.gyro_dps[0];
            gyro_bias[1] += sample.gyro_dps[1];
            gyro_bias[2] += sample.gyro_dps[2];
            valid_count++;
        }

        osDelay(1);
    }

    if (valid_count > 0)
    {
        gyro_bias[0] /= valid_count;
        gyro_bias[1] /= valid_count;
        gyro_bias[2] /= valid_count;
    }
}

void Attitude_Update(Attitude *attitude,
                     const float gyro_dps[3],
                     const float accel_g[3],
                     float dt)
{
    if (attitude == NULL || gyro_dps == NULL ||
        accel_g == NULL || dt <= 0.0f)
    {
        LED(0,200,200);
        return;
    }

    float q0 = attitude->q[0];
    float q1 = attitude->q[1];
    float q2 = attitude->q[2];
    float q3 = attitude->q[3];

    const float half_vx = q1 * q3 - q0 * q2;
    const float half_vy = q0 * q1 + q2 * q3;
    const float half_vz = q0 * q0 - 0.5f + q3 * q3;

    const float accel_norm =
        sqrtf(accel_g[0] * accel_g[0] +
              accel_g[1] * accel_g[1] +
              accel_g[2] * accel_g[2]);

    float error_x = 0.0f;
    float error_y = 0.0f;
    float error_z = 0.0f;

    if (accel_norm > 0.1f)
    {
        const float ax = accel_g[0] / accel_norm;
        const float ay = accel_g[1] / accel_norm;
        const float az = accel_g[2] / accel_norm;

        error_x = ay * half_vz - az * half_vy;
        error_y = az * half_vx - ax * half_vz;
        error_z = ax * half_vy - ay * half_vx;

        attitude->integral[0] += MAHONY_TWO_KI * error_x * dt;
        attitude->integral[1] += MAHONY_TWO_KI * error_y * dt;
        attitude->integral[2] += MAHONY_TWO_KI * error_z * dt;
    }

    float gx = (gyro_dps[0] - gyro_bias[0]) * DEG_TO_RAD;
    float gy = (gyro_dps[1] - gyro_bias[1]) * DEG_TO_RAD;
    float gz = (gyro_dps[2] - gyro_bias[2]) * DEG_TO_RAD;

    gx += MAHONY_TWO_KP * error_x + attitude->integral[0];
    gy += MAHONY_TWO_KP * error_y + attitude->integral[1];
    gz += MAHONY_TWO_KP * error_z + attitude->integral[2];

    const float half_dt = 0.5f * dt;

    attitude->q[0] += (-q1 * gx - q2 * gy - q3 * gz) * half_dt;
    attitude->q[1] += ( q0 * gx + q2 * gz - q3 * gy) * half_dt;
    attitude->q[2] += ( q0 * gy - q1 * gz + q3 * gx) * half_dt;
    attitude->q[3] += ( q0 * gz + q1 * gy - q2 * gx) * half_dt;

    const float norm =
        sqrtf(attitude->q[0] * attitude->q[0] +
              attitude->q[1] * attitude->q[1] +
              attitude->q[2] * attitude->q[2] +
              attitude->q[3] * attitude->q[3]);

    if (norm > 0.0f)
    {
        attitude->q[0] /= norm;
        attitude->q[1] /= norm;
        attitude->q[2] /= norm;
        attitude->q[3] /= norm;
    }

    q0 = attitude->q[0];
    q1 = attitude->q[1];
    q2 = attitude->q[2];
    q3 = attitude->q[3];

    attitude->roll =
        atan2f(2.0f * (q0 * q1 + q2 * q3),
               1.0f - 2.0f * (q1 * q1 + q2 * q2)) * RAD_TO_DEG;

    attitude->pitch =
        asinf(2.0f * (q0 * q2 - q3 * q1)) * RAD_TO_DEG;

    attitude->yaw =
        atan2f(2.0f * (q0 * q3 + q1 * q2),
               1.0f - 2.0f * (q2 * q2 + q3 * q3)) * RAD_TO_DEG;
}
