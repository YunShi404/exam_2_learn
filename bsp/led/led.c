//
// Created by 30547 on 2026/11/7.
//

#include "led.h"
#include <stdint.h>

#define LOW (0xC0 >> 1)
#define HIGH (0xF0 >> 1)

volatile uint8_t static led_transmit[24] RAM_D3_SECTION ;
void LED(uint8_t R,uint8_t G,uint8_t B)
{
    uint8_t i;

    // 1. 处理绿色 (Green) - 第 1~8 个字节
    for (i = 0; i < 8; i++)
    {
        if (G & (0x80 >> i))
            led_transmit[i] = HIGH; // 逻辑 1
        else
            led_transmit[i] = LOW; // 逻辑 0
    }

    // 2. 处理红色 (Red) - 第 9~16 个字节
    for (i = 0; i < 8; i++)
    {
        if (R & (0x80 >> i))
            led_transmit[8 + i] = HIGH;
        else
            led_transmit[8 + i] = LOW;
    }

    // 3. 处理蓝色 (Blue) - 第 17~24 个字节
    for (i = 0; i < 8; i++)
    {
        if (B & (0x80 >> i))
            led_transmit[16 + i] = HIGH;
        else
            led_transmit[16 + i] = LOW;
    }

    // 4. 启动 DMA 发送 24 个字节
    HAL_SPI_Transmit_DMA(&hspi6, (uint8_t *)led_transmit, 24);

    // 5. 延时等待 WS2812 锁存（RESET信号）
    HAL_Delay(100);
}


void LED_Init(void)
{
    LED(0,0,0);
}