//
// Created by 30547 on 2026/11/22.
//

#ifndef EXAM_2_LEARN_PID_H
#define EXAM_2_LEARN_PID_H

typedef struct
{
    double goal;
    double real;

    float p;
    float i;
    float d;

    /* 本次调用距离上次调用的时间，单位：秒。 */
    float dt;

    double integral;
    double last_error;
    double error;
    float vary_speed;
    float last_vary_speed;

    double integral_limit;
    double output_limit;
} PID_member;

int PID_Init(PID_member *pid, float p, float i, float d, float dt, double integral_limit, double output_limit, double goal, double real);
int PID_Reset(PID_member *pid);
double PID(PID_member *pid);

#endif //EXAM_2_LEARN_PID_H
