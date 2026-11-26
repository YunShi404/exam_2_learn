//
// Created by 30547 on 2026/11/22.
//

#include "pid.h"
#include <stddef.h>


int PID_Init(PID_member *pid,
             float p,
             float i,
             float d,
             float dt,
             double integral_limit,
             double output_limit,
             double goal,
             double real)
{
    if (pid == NULL || dt <= 0)
    {
        return -1;
    }

    pid->goal = goal;
    pid->real = real;

    pid->p = p;
    pid->i = i;
    pid->d = d;

    pid->dt = dt;

    pid->integral = 0.0;
    pid->last_error = goal - real;
    pid->error = 0.0;
    pid->vary_speed = 0.0f;
    pid->last_vary_speed = 0;

    pid->integral_limit = integral_limit;
    pid->output_limit = output_limit;
    return 1;
}

int PID_Reset(PID_member *pid)
{
    if (pid == NULL || pid->dt <= 0)
    {
        return -1;
    }

    pid->integral = 0.0;
    pid->last_error = pid->goal - pid->real;
    pid->error = 0.0;
    pid->vary_speed = 0.0f;
    pid->last_vary_speed = 0;
    return 1;
}


double PID(PID_member *pid)
{
    if (pid == NULL || pid->dt <= 0)
    {
        //LED(200,0,200);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return 0;
    }



    pid->error = pid->goal - pid->real;

    pid->integral =pid->integral + pid->error * pid->dt;

    if (pid->integral > pid->integral_limit)
    {
        pid->integral = pid->integral_limit;
    }

    if (pid->integral < -pid->integral_limit)
    {
        pid->integral = -pid->integral_limit;
    }



    pid->vary_speed = (pid->error - pid->last_error) / pid->dt;
    pid->vary_speed = 0.25 * pid->vary_speed + 0.75 * pid->last_vary_speed;//微分滤波

    double output = pid->p * pid->error + pid->i * pid->integral + pid->d * pid->vary_speed;

    if (output > pid->output_limit)
    {
        output = pid->output_limit;
    }

    if (output < -pid->output_limit)
    {
        output = -pid->output_limit;
    }

    pid->last_error = pid->error;
    pid->last_vary_speed = pid->vary_speed;

    return output;
}
