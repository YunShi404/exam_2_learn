//
// Created by 30547 on 2026/11/7.
//

#ifndef EXAM_2_LEARN_LED_H
#define EXAM_2_LEARN_LED_H
#include "main.h"
#include "spi.h"
// 宏名用大写（方便你用），里面的段名用小写加点（这是GCC的行业规范）
#define RAM_D3_SECTION __attribute__((section(".ram_d3_buf")))
void LED(uint8_t R,uint8_t G,uint8_t B);
void LED_Init(void);
#endif //EXAM_2_LEARN_LED_H
