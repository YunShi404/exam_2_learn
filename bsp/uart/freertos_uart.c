//
// Created by 30547 on 2026/11/15.
//
#include "freertos.h"
#include "freertos_uart.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "led.h"
#include "buzzer.h"
#include <stdarg.h>
#include "task.h"
static Device Device_List[Device_num + 1];//这里加1是为了让数组能从一开始，0那个就不要了，为了能和串口id对上（串口从1开始而数组从0开始）
volatile uint16_t rx_lost = 0;
volatile uint16_t tx_lost = 0;
//初始化函数：注册串口
void UART_Init(UART_ID id, UART_HandleTypeDef *huart, p_data_process data_process_x)
{
    Device_List[id].huart = huart;
    Device_List[id].TX_DMA_busy = 0;
    Device_List[id].TX_BUF_head = 0;
    Device_List[id].TX_BUF_tail = 0;
    Device_List[id].rx_buffer_use = 0;
    Device_List[id].process_flag = 0;
    Device_List[id].data_process = data_process_x;

    // 清除启动前已经挂起的串口错误
    __HAL_UART_CLEAR_FLAG(
        huart,
        UART_CLEAR_PEF |
        UART_CLEAR_FEF |
        UART_CLEAR_NEF |
        UART_CLEAR_OREF
    );

    // 清掉 HAL 保存的软件错误状态
    huart->ErrorCode = HAL_UART_ERROR_NONE;

    HAL_UARTEx_ReceiveToIdle_DMA(huart, Device_List[id].rx_buffer[Device_List[id].rx_buffer_use], rx_buffer_size);
}


int UART_DMA_Transmit(UART_ID id, uint8_t *data, uint32_t size)
{
    if (size == 0 || data == NULL || Device_List[id].huart == NULL || id < 1 || id > 10) //错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {
        buzzer(1000,75);
        return 0;
    }

    taskENTER_CRITICAL();
    uint16_t used = (Device_List[id].TX_BUF_head >= Device_List[id].TX_BUF_tail) ? (Device_List[id].TX_BUF_head - Device_List[id].TX_BUF_tail) : (TX_BUF_SIZE - Device_List[id].TX_BUF_tail + Device_List[id].TX_BUF_head);
    uint16_t free_space = TX_BUF_SIZE - used - 1;

    if (free_space < size) //错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {
        taskEXIT_CRITICAL();
        buzzer(1000,75);
        return 0;
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
        taskEXIT_CRITICAL();
        if (HAL_UART_Transmit_DMA(Device_List[id].huart, Device_List[id].TX_Middle_BUF, len) != HAL_OK )
        {
            tx_lost++;
            // if (Device_List[id].TX_BUF_tail >= len)//尾指针回退
            // {
            //     Device_List[id].TX_BUF_tail -= len;
            // }
            // else
            // {
            //     Device_List[id].TX_BUF_tail =
            //         TX_BUF_SIZE - (len - Device_List[id].TX_BUF_tail);
            // }
            Device_List[id].TX_DMA_busy = 0;
            return 0;
        }
        else return 1;

    }
    else
    {
        taskEXIT_CRITICAL();
        return 0;
    }
}


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
    UBaseType_t save_status;
    save_status = taskENTER_CRITICAL_FROM_ISR();
    while (Device_List[id].TX_BUF_tail != Device_List[id].TX_BUF_head && len < TX_Middle_BUF_SIZE)
    {
        Device_List[id].TX_Middle_BUF[len++] = Device_List[id].TX_BUF[Device_List[id].TX_BUF_tail];
        Device_List[id].TX_BUF_tail = (Device_List[id].TX_BUF_tail + 1) % TX_BUF_SIZE;
    }
    if (len == 0)
    {
        Device_List[id].TX_DMA_busy = 0;
        taskEXIT_CRITICAL_FROM_ISR(save_status);
    }
    else
    {
        taskEXIT_CRITICAL_FROM_ISR(save_status);
        if (HAL_UART_Transmit_DMA(Device_List[id].huart, Device_List[id].TX_Middle_BUF, len) != HAL_OK )
        {
            // if (Device_List[id].TX_BUF_tail >= len)//尾指针回退
            // {
            //     Device_List[id].TX_BUF_tail -= len;
            // }
            // else
            // {
            //     Device_List[id].TX_BUF_tail =
            //         TX_BUF_SIZE - (len - Device_List[id].TX_BUF_tail);
            // }
            Device_List[id].TX_DMA_busy = 0;
            buzzer(1000,75);
            tx_lost += 1;
        }
    }
}

void UART_printf_Transmit(UART_ID id, const char* format, ...)
{
    if (Device_List[id].huart == NULL || format == NULL) //错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {
        buzzer(1000,75);
        return;
    }
    char buffer[128];//这里是裸机的一半，因为任务栈大小默认是512，256太多了
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
    if (Device_List[id].process_flag == 0)
    {
        Device_List[id].len = size;
        Device_List[id].rx_buffer_used = Device_List[id].rx_buffer_use;
        Device_List[id].rx_buffer_use ^= 1;
        HAL_UARTEx_ReceiveToIdle_DMA(huart, Device_List[id].rx_buffer[Device_List[id].rx_buffer_use], rx_buffer_size);
        if (Device_List[id].data_process != NULL)
        {
            Device_List[id].process_flag = 1;
            Device_List[id].data_process(Device_List[id].rx_buffer[Device_List[id].rx_buffer_used], Device_List[id].len);
        }
    }
    else
    {
        rx_lost += 1;
        LED(200,0,0);//可接受错误指示AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        HAL_UARTEx_ReceiveToIdle_DMA(huart, Device_List[id].rx_buffer[Device_List[id].rx_buffer_use], rx_buffer_size);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
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

    if (id == Device_num + 1)
    {
        return;
    }

    uint32_t uart_error = huart->ErrorCode;

    /*
     * 任意 DMA 错误都可能已经中止发送 DMA，因此必须先恢复发送端状态。
     * 若接收 DMA 仍在工作，也不能让后面的提前返回跳过这一步。
     */
    if ((uart_error & HAL_UART_ERROR_DMA) != 0U)
    {
        UBaseType_t save_status;
        save_status = taskENTER_CRITICAL_FROM_ISR();
        Device_List[id].TX_DMA_busy = 0;
        taskEXIT_CRITICAL_FROM_ISR(save_status);
    }

    /*
     * 非阻塞接收错误时 DMA 仍在工作，不能重复启动；
     * 只有 HAL 已将接收状态恢复为 READY 时才重新启动。
     */
    if (huart->RxState != HAL_UART_STATE_READY)
    {
        huart->ErrorCode = HAL_UART_ERROR_NONE;
        return;
    }


    __HAL_UART_CLEAR_FLAG(
        huart,
        UART_CLEAR_PEF |
        UART_CLEAR_FEF |
        UART_CLEAR_NEF |
        UART_CLEAR_OREF |
        UART_CLEAR_RTOF
    );

    huart->ErrorCode = HAL_UART_ERROR_NONE;

    HAL_UARTEx_ReceiveToIdle_DMA(
        huart,
        Device_List[id].rx_buffer[Device_List[id].rx_buffer_use],
        rx_buffer_size
    );
}

void UART_data_process_over(UART_ID id)
{
    Device_List[id].process_flag = 0;
}
