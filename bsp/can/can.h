//
// Created by 30547 on 2026/11/16.
//

#ifndef EXAM_2_LEARN_CAN_H
#define EXAM_2_LEARN_CAN_H
#include "fdcan.h"


#define CAN_NUM 3
typedef void (*p_can_process)(uint16_t id, const uint8_t *data, uint8_t length);

typedef enum
{
    FDCAN_1 = 1,
    FDCAN_2 = 2,
    FDCAN_3 = 3,
}ID;

typedef struct
{
    FDCAN_HandleTypeDef *hfdcan;
    p_can_process can_process;
}CAN;


void CAN_Init_A(ID can_id, FDCAN_HandleTypeDef *hfdcan, uint16_t filter_goal_id, uint16_t filter_mask, p_can_process can_process_x);
void CAN_Transmit_classic(uint32_t data_id, const uint8_t *data, uint8_t length, ID can_id);


#endif //EXAM_2_LEARN_CAN_H
