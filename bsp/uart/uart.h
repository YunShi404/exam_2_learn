//
// Created by 30547 on 2026/11/9.
//

#ifndef EXAM_2_LEARN_UART_H
#define EXAM_2_LEARN_UART_H

#include <stdint.h>
#include "usart.h"

#define Device_num 10
#define TX_BUF_SIZE 256
#define TX_Middle_BUF_SIZE 64

typedef struct
{
    UART_HandleTypeDef *huart;
    uint8_t TX_BUF[TX_BUF_SIZE];
    uint8_t TX_Middle_BUF[TX_Middle_BUF_SIZE];
    volatile uint16_t TX_BUF_head;
    volatile uint16_t TX_BUF_tail;
    volatile uint8_t TX_DMA_busy;
} Device;

void UART_init(UART_HandleTypeDef *huart);
void UART_DMA_Transmit(UART_HandleTypeDef *huart, uint8_t *data, uint32_t size);
#endif //EXAM_2_LEARN_UART_H
