//
// Created by 30547 on 2026/11/18.
//
#include "buzzer.h"
#include "imu.h"
#include "spi.h"
#include "main.h"
#include "stm32h7xx_hal_gpio.h"

int16_t ACC_origin_data[3];
int16_t GYRO_origin_data[3];
float ACC_data_g[3];
float GYRO_data_dps[3];


void BMI088_Transmit(uint8_t device, uint8_t address, uint8_t choice)
{
    if (device == 0)
    {
        HAL_GPIO_WritePin(GPIOC, SPI2_CS0_Pin, GPIO_PIN_RESET);
        uint8_t set[] = {address, choice};
        HAL_SPI_Transmit(&hspi2, set, sizeof(set), HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOC, SPI2_CS0_Pin, GPIO_PIN_SET);
    }
    else if (device == 1)
    {
        HAL_GPIO_WritePin(GPIOC, SPI2_CS1_Pin, GPIO_PIN_RESET);
        uint8_t set[] = {address, choice};
        HAL_SPI_Transmit(&hspi2, set, sizeof(set), HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOC, SPI2_CS1_Pin, GPIO_PIN_SET);
    }
    else buzzer(325, 85);//错误指示aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
}


void BMI088_Read(uint8_t device, uint8_t address)
{
    if (device == 0)
    {
        HAL_GPIO_WritePin(GPIOC, SPI2_CS0_Pin, GPIO_PIN_RESET);
        uint8_t ACC_rx_data[8];
        uint8_t tx_data[8] = {address, 0, 0, 0, 0, 0, 0, 0};
        HAL_SPI_TransmitReceive(&hspi2, tx_data, ACC_rx_data, 8, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOC, SPI2_CS0_Pin, GPIO_PIN_SET);
        ACC_origin_data[0] = ACC_rx_data[3]<<8 | ACC_rx_data[2];
        ACC_origin_data[1] = ACC_rx_data[5]<<8 | ACC_rx_data[4];
        ACC_origin_data[2] = ACC_rx_data[7]<<8 | ACC_rx_data[6];
    }
    else if (device == 1)
    {
        HAL_GPIO_WritePin(GPIOC, SPI2_CS1_Pin, GPIO_PIN_RESET);
        uint8_t GYRO_rx_data[7];
        uint8_t tx_data[7] = {address, 0, 0, 0, 0, 0, 0};
        HAL_SPI_TransmitReceive(&hspi2, tx_data, GYRO_rx_data, 7, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOC, SPI2_CS1_Pin, GPIO_PIN_SET);
        GYRO_origin_data[0] = GYRO_rx_data[2]<<8 | GYRO_rx_data[1];
        GYRO_origin_data[1] = GYRO_rx_data[4]<<8 | GYRO_rx_data[3];
        GYRO_origin_data[2] = GYRO_rx_data[6]<<8 | GYRO_rx_data[5];
    }
    else buzzer(325, 85);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
}

void BMI088_update(void)
{
    BMI088_Read(0, 0x92);
    BMI088_Read(1, 0x82);
    ACC_data_g[0] = ACC_origin_data[0]/5460.0f;
    ACC_data_g[1] = ACC_origin_data[1]/5460.0f;
    ACC_data_g[2] = ACC_origin_data[2]/5460.0f;
    GYRO_data_dps[0] = GYRO_origin_data[0]/16.384f;
    GYRO_data_dps[1] = GYRO_origin_data[1]/16.384f;
    GYRO_data_dps[2] = GYRO_origin_data[2]/16.384f;
}

void BMI088_init(void)
{
    BMI088_Read(0, 0x80);//产生一个SPI操作以转换到SPI模式
    HAL_Delay(1);
    BMI088_Transmit(0, 0x7D, 0x04);
    HAL_Delay(1);
    BMI088_Transmit(0, 0x7C, 0x00);
    BMI088_Transmit(0, 0x40, 0xA8);
    BMI088_Transmit(0, 0x41, 0x01);
    BMI088_Transmit(1, 0x11, 0x00);
    BMI088_Transmit(1, 0x0F, 0x00);//这里量程选多少待定bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
    BMI088_Transmit(1, 0x10, 0x02);
    HAL_Delay(50);
}

// void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
// {
//     if (hspi->Instance == SPI2)
//     {
//         ACC_origin_data[0] = ACC_rx_data[2]<<8 | ACC_rx_data[3];
//         ACC_origin_data[1] = ACC_rx_data[4]<<8 | ACC_rx_data[5];
//         ACC_origin_data[2] = ACC_rx_data[6]<<8 | ACC_rx_data[7];
//         GYRO_origin_data[0] = GYRO_rx_data[2]<<8 | GYRO_rx_data[3];
//         GYRO_origin_data[1] = GYRO_rx_data[4]<<8 | GYRO_rx_data[5];
//         GYRO_origin_data[2] = GYRO_rx_data[6]<<8 | GYRO_rx_data[7];
//         //弄一个函数指针接口
//     }
// }