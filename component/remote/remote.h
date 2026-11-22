//
// Created by 30547 on 2026/11/21.
//

#ifndef EXAM_2_LEARN_REMOTE_H
#define EXAM_2_LEARN_REMOTE_H

#include <stdint.h>

typedef struct
{
    int16_t ch0;
    int16_t ch1;
    int16_t ch2;
    int16_t ch3;

    uint8_t sw1;
    uint8_t sw2;

    uint8_t online;

    /* 临时诊断字段：确认串口事件的长度和原始字节。 */
    uint16_t last_length;
    uint8_t raw[18];
}remote_data_type;


void Remote_rx(uint8_t *data, uint16_t length);
void Get_Remote_data(remote_data_type *get_struct);
#endif //EXAM_2_LEARN_REMOTE_H
