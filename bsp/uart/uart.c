//
// Created by 30547 on 2026/11/9.
//

#include "uart.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "led.h"
#include "buzzer.h"
#include <stdarg.h>

//我们默认使用freertos的，这里的没有在维护了，已经不能用了
//我真的不知道该怎么封装了，我没有参加过任何大量使用串口的项目，实在无从想象RM想要实现什么样的功能，那些已经写好的我也看不懂什么意思，我下面要梦到什么写什么了😅


static Device Device_List[Device_num + 1];//这里加1是为了让数组能从一开始，0那个就不要了，为了能和串口id对上（串口从1开始而数组从0开始）

//初始化函数：注册串口
void UART_init(UART_ID id, UART_HandleTypeDef *huart, p_data_process data_process_x)
{
    Device_List[id].huart = huart;
    Device_List[id].TX_DMA_busy = 0;
    Device_List[id].TX_BUF_head = 0;
    Device_List[id].TX_BUF_tail = 0;
    Device_List[id].rx_buffer_use = 0;
    Device_List[id].rx_flag = 0;
    Device_List[id].data_process = data_process_x;
    HAL_UARTEx_ReceiveToIdle_DMA(huart, Device_List[id].rx_buffer[Device_List[id].rx_buffer_use], rx_buffer_size);
}

//裸机DMA发送
void UART_DMA_Transmit(UART_ID id, uint8_t *data, uint32_t size)
{
    if (size == 0 || data == NULL || Device_List[id].huart == NULL) //错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {
        buzzer(1000,75);
        return;
    }

    __disable_irq();
    uint16_t used = (Device_List[id].TX_BUF_head >= Device_List[id].TX_BUF_tail) ? (Device_List[id].TX_BUF_head - Device_List[id].TX_BUF_tail) : (TX_BUF_SIZE - Device_List[id].TX_BUF_tail + Device_List[id].TX_BUF_head);
    uint16_t free_space = TX_BUF_SIZE - used - 1;

    if (free_space < size) //错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {
        __enable_irq();
        buzzer(1000,75);
        return;
    }
    for (uint16_t i = 0; i < size; i++)
    {
        Device_List[id].TX_BUF[Device_List[id].TX_BUF_head] = data[i];
        Device_List[id].TX_BUF_head = (Device_List[id].TX_BUF_head + 1) % TX_BUF_SIZE;
    }
    uint16_t len = 0;
    if (!Device_List[id].TX_DMA_busy)
    {
        Device_List[id].TX_DMA_busy = 1;
        while (Device_List[id].TX_BUF_tail != Device_List[id].TX_BUF_head && len < TX_Middle_BUF_SIZE)
        {
            Device_List[id].TX_Middle_BUF[len++] = Device_List[id].TX_BUF[Device_List[id].TX_BUF_tail];
            Device_List[id].TX_BUF_tail = (Device_List[id].TX_BUF_tail + 1) % TX_BUF_SIZE;
        }
        __enable_irq();
        HAL_UART_Transmit_DMA(Device_List[id].huart, Device_List[id].TX_Middle_BUF, len);
    }
    else __enable_irq();
}


//裸机DMA缓冲发送回调
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    UART_ID id = Device_num + 1;
    for (uint8_t i = 1; i < Device_num + 1; i++)
    {
        if (Device_List[i].huart == huart)
        {
            id = (UART_ID)i;
            break;
        }
    }

    if (id == Device_num + 1)         //错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {
        buzzer(1000,75);
        return;
    }

    uint16_t len = 0;
    __disable_irq();
    while (Device_List[id].TX_BUF_tail != Device_List[id].TX_BUF_head && len < TX_Middle_BUF_SIZE)
    {
        Device_List[id].TX_Middle_BUF[len++] = Device_List[id].TX_BUF[Device_List[id].TX_BUF_tail];
        Device_List[id].TX_BUF_tail = (Device_List[id].TX_BUF_tail + 1) % TX_BUF_SIZE;
    }
    if (len == 0)
    {
        Device_List[id].TX_DMA_busy = 0;
        __enable_irq();
    }
    else
    {
        __enable_irq();
        HAL_UART_Transmit_DMA(huart, Device_List[id].TX_Middle_BUF, len);
    }
}

void UART_printf_Transmit(UART_ID id, const char* format, ...)
{
    if (Device_List[id].huart == NULL || format == NULL) //错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
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
        UART_DMA_Transmit(id, (uint8_t *)buffer, (uint16_t)len);
    }
}



void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t size)
{
    UART_ID id = Device_num + 1;
    for (uint8_t i = 1; i < Device_num + 1; i++)
    {
        if (Device_List[i].huart == huart)
        {
            id = (UART_ID)i;
            break;
        }
    }

    if (id == Device_num + 1)         //错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {
        buzzer(1000,75);
        return;
    }
    if (Device_List[id].rx_flag == 0)
    {
        Device_List[id].len = size;
        Device_List[id].rx_buffer_used = Device_List[id].rx_buffer_use;
        Device_List[id].rx_buffer_use ^= 1;
        HAL_UARTEx_ReceiveToIdle_DMA(huart, Device_List[id].rx_buffer[Device_List[id].rx_buffer_use], rx_buffer_size);
        if (Device_List[id].data_process != NULL)
        {
            Device_List[id].rx_flag = 1;
            Device_List[id].data_process(Device_List[id].rx_buffer[Device_List[id].rx_buffer_used], Device_List[id].len);
        }
        else buzzer(1000,75);//错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    }
    else
    {
        LED(200,0,0);//可接受错误指示AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        HAL_UARTEx_ReceiveToIdle_DMA(huart, Device_List[id].rx_buffer[Device_List[id].rx_buffer_use], rx_buffer_size);
    }
}
void UART_data_process_over(UART_ID id)
{
    Device_List[id].rx_flag = 0;
}