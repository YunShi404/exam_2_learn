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
static remote_data_type error = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static remote_data_type check;

static uint32_t remote_last_rx_ms;

static uint8_t buffer_point = 0;
static uint8_t check_point = 0;
static uint8_t buffer[BUFFER_SIZE];



void Remote_rx(uint8_t *data, uint16_t length)
{

    if (data == NULL || length > 36 || length == 0)
    {
        UART_data_process_over(UART_5);
        LED(0,200,0);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return;
    }

    for (uint16_t i = 0; i < length; i++)
    {
        buffer[buffer_point] = data[i];
        buffer_point++;
        if (buffer_point > BUFFER_SIZE - 1) buffer_point = 0;
    }


    for (uint8_t i = 0; i < BUFFER_SIZE; i++)
    {
        check.ch0 = (buffer[check_point] | buffer[(check_point + 1) % BUFFER_SIZE] << 8) & 0x7FF;
        check.ch1 = (buffer[(check_point + 1) % BUFFER_SIZE] >> 3 | buffer[(check_point + 2) % BUFFER_SIZE] << 5) & 0x7FF;
        check.ch2 = (buffer[(check_point + 2) % BUFFER_SIZE] >> 6| buffer[(check_point + 3) % BUFFER_SIZE] << 2 | buffer[(check_point + 4) % BUFFER_SIZE] << 10) & 0x7FF;
        check.ch3 = (buffer[(check_point + 4) % BUFFER_SIZE] >> 1 | buffer[(check_point + 5) % BUFFER_SIZE] << 7) & 0x7FF;

        check.sw1 = (buffer[(check_point + 5) % BUFFER_SIZE] >> 4) & 0x3;
        check.sw2 = (buffer[(check_point + 5) % BUFFER_SIZE] >> 6) & 0x3;

        if (check.ch0 >= 364 && check.ch0 <= 1684 && check.ch1 >= 364 && check.ch1 <= 1684 && check.ch2 >= 364 && check.ch2 <= 1684 && check.ch3 >= 364 && check.ch3 <= 1684 && check.sw1 >= 1 && check.sw1 <= 3 && check.sw2 >= 1 && check.sw2 <= 3)
        {
            remote_data.ch0 = check.ch0;
            remote_data.ch0 -= 1024;
            remote_data.ch1 = check.ch1;
            remote_data.ch1 -= 1024;
            remote_data.ch2 = check.ch2;
            remote_data.ch2 -= 1024;
            remote_data.ch3 = check.ch3;
            remote_data.ch3 -= 1024;

            remote_data.sw1 = check.sw1;
            remote_data.sw2 = check.sw2;
            for (uint8_t j = 0; j < 18; j++)
            {
                buffer[check_point] = 0;
                check_point++;
                if (check_point > BUFFER_SIZE - 1) check_point = 0;
            }
            remote_last_rx_ms = HAL_GetTick();
            UART_data_process_over(UART_5);
            return;
        }

        check_point++;
        if (check_point > BUFFER_SIZE - 1) check_point = 0;
    }
    LED(0,200,0);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    UART_data_process_over(UART_5);
}

int Get_Remote_data(remote_data_type *get_struct)
{
    if (get_struct == NULL)
    {
        LED(0,200,0);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return -1;
    }

    taskENTER_CRITICAL();
    *get_struct = remote_data;
    taskEXIT_CRITICAL();

    get_struct->online = (HAL_GetTick() - remote_last_rx_ms) <= 100U;
    if (get_struct->online == 0)
    {
        *get_struct = error;
        return -2;
    }
    return 1;
}
