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
#define rx_buffer_size 256

typedef void (*p_data_process)(uint8_t* , uint16_t);
typedef struct
{
    UART_HandleTypeDef *huart;
    uint8_t TX_BUF[TX_BUF_SIZE];
    uint8_t TX_Middle_BUF[TX_Middle_BUF_SIZE];
    volatile uint16_t TX_BUF_head;
    volatile uint16_t TX_BUF_tail;
    volatile uint8_t TX_DMA_busy;
    uint8_t rx_buffer[2][rx_buffer_size];
    uint16_t len;
    uint8_t rx_buffer_use ;
    p_data_process data_process;
    volatile uint8_t rx_flag ;
    volatile uint8_t rx_buffer_used ;
} Device;

typedef enum
{
    UART_1 = 1,
    UART_2,
    UART_3,
    UART_4,
    UART_5,
    UART_6,
    UART_7,
    UART_8,
    UART_9,
    UART_10,
}ID;



void UART_init(ID id, UART_HandleTypeDef *huart, p_data_process data_process_x);
void UART_DMA_Transmit(ID id, uint8_t *data, uint32_t size);
void UART_printf_Transmit(ID id, const char* format, ...);
void UART_data_process_over(ID id);
#endif //EXAM_2_LEARN_UART_H
