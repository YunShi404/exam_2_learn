//
// Created by 30547 on 2026/11/22.
//

#include "pid.h"

#include "led.h"


double PID(PID_member *pid)
{
    if (pid == NULL)
    {
        LED(200,0,200);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return 0.0;
    }

    pid->error = pid->goal - pid->real;

    pid->integral =pid->integral + pid->error * pid->integral_speed;

    if (pid->integral > pid->integral_limit)
    {
        pid->integral = pid->integral_limit;
    }

    if (pid->integral < -pid->integral_limit)
    {
        pid->integral = -pid->integral_limit;
    }

    pid->vary_speed = pid->error - pid->last_error;

    double output =
        pid->p * pid->error +
        pid->i * pid->integral +
        pid->d * pid->vary_speed;

    if (output > pid->output_limit)
    {
        output = pid->output_limit;
    }

    if (output < -pid->output_limit)
    {
        output = -pid->output_limit;
    }

    pid->last_error = pid->error;

    return output;
}
