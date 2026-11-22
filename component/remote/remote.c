//
// Created by 30547 on 2026/11/21.
//

#include"freertos.h"
#include "remote.h"
#include <stdint.h>
#include "freertos_uart.h"
#include "led.h"
#include "task.h"

static remote_data_type remote_data;

static uint32_t remote_last_rx_ms;

void Remote_rx(uint8_t *data, uint16_t length)
{


    if (data == NULL || length < 18)
    {

        UART_data_process_over(UART_5);
        LED(0,200,0);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return;
    }

    remote_data.ch0 = (data[0] | data[1] << 8) & 0x7FF;
    remote_data.ch0 -= 1024;
    remote_data.ch1 = (data[1] >> 3 | data[2] << 5) & 0x7FF;
    remote_data.ch1 -= 1024;
    remote_data.ch2 = (data[2] >> 6| data[3] << 2 | data[4] << 10) & 0x7FF;
    remote_data.ch2 -= 1024;
    remote_data.ch3 = (data[4] >> 1 | data[5] << 7) & 0x7FF;
    remote_data.ch3 -= 1024;

    remote_data.sw1 = ((data[5] >> 4) & 0xC) >> 2;
    remote_data.sw2 = (data[5] >> 4) & 0x3;
    remote_last_rx_ms = HAL_GetTick();
    UART_data_process_over(UART_5);
}

void Get_Remote_data(remote_data_type *get_struct)
{
    if (get_struct == NULL)
    {
        LED(0,200,0);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return;
    }

    taskENTER_CRITICAL();
    *get_struct = remote_data;
    taskEXIT_CRITICAL();

    get_struct->online = (HAL_GetTick() - remote_last_rx_ms) <= 100U;//这句AI给的，我先不管了
}
