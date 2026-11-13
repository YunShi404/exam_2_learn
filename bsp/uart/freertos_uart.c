//
// Created by 30547 on 2026/11/13.
//



#include "FreeRTOS.h"
#include "freertos_uart.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "led.h"
#include "buzzer.h"
#include "task.h"
static Device Device_List[Device_num];
void FreeRTOS_UART_init(UART_HandleTypeDef *huart)
{
    for (int i = 0; i < Device_num; i++)
    {
        if (Device_List[i].huart == NULL)
        {
            Device_List[i].huart = huart;
            Device_List[i].TX_DMA_busy = 0;
            Device_List[i].TX_BUF_head = 0;
            Device_List[i].TX_BUF_tail = 0;
            break;
        }
    }
}


void FreeRTOS_UART_DMA_Transmit(UART_HandleTypeDef *huart, uint8_t *data, uint32_t size)
{
    if (size == 0 || huart == NULL ||data == NULL)
    {
        LED(0,0,200);
        return;
    }

    uint8_t Devicex = 100;
    for (uint8_t i = 0; i < Device_num; i++)
    {
        if (Device_List[i].huart == huart)
        {
            Devicex = i;
            break;
        }
    }

    if (Devicex == 100)
    {
        buzzer(1000,75);
        return;
    }

    taskENTER_CRITICAL();
    uint16_t used = (Device_List[Devicex].TX_BUF_head >= Device_List[Devicex].TX_BUF_tail) ? (Device_List[Devicex].TX_BUF_head - Device_List[Devicex].TX_BUF_tail) : (TX_BUF_SIZE - Device_List[Devicex].TX_BUF_tail + Device_List[Devicex].TX_BUF_head);
    uint16_t free_space = TX_BUF_SIZE - used - 1;

    if (free_space < size)
    {
        taskEXIT_CRITICAL();
        LED(200,0,0);
        return;
    }
    for (uint16_t i = 0; i < size; i++)
    {
        Device_List[Devicex].TX_BUF[Device_List[Devicex].TX_BUF_head] = data[i];
        Device_List[Devicex].TX_BUF_head = (Device_List[Devicex].TX_BUF_head + 1) % TX_BUF_SIZE;
    }
    uint16_t len = 0;
    if (!Device_List[Devicex].TX_DMA_busy)
    {
        Device_List[Devicex].TX_DMA_busy = 1;
        while (Device_List[Devicex].TX_BUF_tail != Device_List[Devicex].TX_BUF_head && len < TX_Middle_BUF_SIZE)
        {
            Device_List[Devicex].TX_Middle_BUF[len++] = Device_List[Devicex].TX_BUF[Device_List[Devicex].TX_BUF_tail];
            Device_List[Devicex].TX_BUF_tail = (Device_List[Devicex].TX_BUF_tail + 1) % TX_BUF_SIZE;
        }
        taskEXIT_CRITICAL();
        HAL_UART_Transmit_DMA(huart, Device_List[Devicex].TX_Middle_BUF, len);
    }
    else taskEXIT_CRITICAL();
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t Devicex = 100;
    for (uint8_t i = 0; i < Device_num; i++)
    {
        if (Device_List[i].huart == huart)
        {
            Devicex = i;
            break;
        }
    }

    if (Devicex == 100)
    {
        buzzer(1000,75);
        return;
    }

    uint16_t len = 0;
    uint32_t save_status;
    save_status = taskENTER_CRITICAL_FROM_ISR();

    while (Device_List[Devicex].TX_BUF_tail != Device_List[Devicex].TX_BUF_head && len < TX_Middle_BUF_SIZE)
    {
        Device_List[Devicex].TX_Middle_BUF[len++] = Device_List[Devicex].TX_BUF[Device_List[Devicex].TX_BUF_tail];
        Device_List[Devicex].TX_BUF_tail = (Device_List[Devicex].TX_BUF_tail + 1) % TX_BUF_SIZE;
    }
    if (len == 0)
    {
        Device_List[Devicex].TX_DMA_busy = 0;
        taskEXIT_CRITICAL_FROM_ISR(save_status);
    }
    else
    {
        taskEXIT_CRITICAL_FROM_ISR(save_status);
        HAL_UART_Transmit_DMA(huart, Device_List[Devicex].TX_Middle_BUF, len);
    }
}


void FreeRTOS_UART_printf_Transmit(UART_HandleTypeDef* huart, const char* format, ...)
{
    if (huart == NULL || format == NULL) //错误指示
    {
        buzzer(1000,75);
        return;
    }
    char buffer[128];//这里小一点是因为freertos任务栈是单独的，默认512大小，要是不改的话要小心使用
    va_list arg;
    va_start(arg, format);
    int len = vsnprintf(buffer,sizeof(buffer), format, arg);
    va_end(arg);
    if (len > 0)
    {
        // 如果返回值大于等于 128，说明字符串太长了，实际只写了 127 个字符
        if (len >= sizeof(buffer))
        {
            len = sizeof(buffer) - 1;
        }
        FreeRTOS_UART_DMA_Transmit(huart, (uint8_t *)buffer, (uint16_t)len);
    }
}