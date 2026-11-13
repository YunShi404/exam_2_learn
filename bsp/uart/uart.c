//
// Created by 30547 on 2026/11/9.
//

#include "uart.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "led.h"
#include "buzzer.h"
#include <stdarg.h>


//我真的不知道该怎么封装了，我没有参加过任何大量使用串口的项目，实在无从想象RM想要实现什么样的功能，那些已经写好的我也看不懂什么意思，我下面要梦到什么写什么了😅


static Device Device_List[Device_num];

//初始化函数：注册串口
void UART_init(UART_HandleTypeDef *huart)
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

//裸机DMA发送
void UART_DMA_Transmit(UART_HandleTypeDef *huart, uint8_t *data, uint32_t size)
{
    if (size == 0 || huart == NULL ||data == NULL) //错误指示
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

    if (Devicex == 100) //错误指示
    {
        buzzer(1000,75);
        return;
    }

    __disable_irq();
    uint16_t used = (Device_List[Devicex].TX_BUF_head >= Device_List[Devicex].TX_BUF_tail) ? (Device_List[Devicex].TX_BUF_head - Device_List[Devicex].TX_BUF_tail) : (TX_BUF_SIZE - Device_List[Devicex].TX_BUF_tail + Device_List[Devicex].TX_BUF_head);
    uint16_t free_space = TX_BUF_SIZE - used - 1;

    if (free_space < size) //错误指示
    {
        __enable_irq();
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
        __enable_irq();
        HAL_UART_Transmit_DMA(huart, Device_List[Devicex].TX_Middle_BUF, len);
    }
    else __enable_irq();
}


//裸机DMA缓冲发送回调
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

    if (Devicex == 100)         //错误指示
    {
        buzzer(1000,75);
        return;
    }

    uint16_t len = 0;
    __disable_irq();
    while (Device_List[Devicex].TX_BUF_tail != Device_List[Devicex].TX_BUF_head && len < TX_Middle_BUF_SIZE)
    {
        Device_List[Devicex].TX_Middle_BUF[len++] = Device_List[Devicex].TX_BUF[Device_List[Devicex].TX_BUF_tail];
        Device_List[Devicex].TX_BUF_tail = (Device_List[Devicex].TX_BUF_tail + 1) % TX_BUF_SIZE;
    }
    if (len == 0)
    {
        Device_List[Devicex].TX_DMA_busy = 0;
        __enable_irq();
    }
    else
    {
        __enable_irq();
        HAL_UART_Transmit_DMA(huart, Device_List[Devicex].TX_Middle_BUF, len);
    }
}

void UART_printf_Transmit(UART_HandleTypeDef* huart, const char* format, ...)
{
    if (huart == NULL || format == NULL) //错误指示
    {
        buzzer(1000,75);
        return;
    }
    char buffer[256];
    va_list arg;
    va_start(arg, format);
    int len = vsnprintf(buffer,sizeof(buffer), format, arg);
    va_end(arg);
    if (len > 0)
    {
        // 如果返回值大于等于 256，说明字符串太长了，实际只写了 255 个字符
        if (len >= sizeof(buffer))
        {
            len = sizeof(buffer) - 1;
        }
        UART_DMA_Transmit(huart, (uint8_t *)buffer, (uint16_t)len);
    }
}
