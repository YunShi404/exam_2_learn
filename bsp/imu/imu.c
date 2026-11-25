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
int8_t ACC_check = 0;
int8_t GYRO_check = 0;

int BMI088_Transmit(BMI088_Device device, uint8_t address, uint8_t choice)
{
    if (device == ACC)
    {
        HAL_GPIO_WritePin(GPIOC, SPI2_CS0_Pin, GPIO_PIN_RESET);
        uint8_t set[] = {address, choice};
        if (HAL_SPI_Transmit(&hspi2, set, sizeof(set), HAL_MAX_DELAY) == HAL_OK)
        {
            HAL_GPIO_WritePin(GPIOC, SPI2_CS0_Pin, GPIO_PIN_SET);
            return 1;
        }
        else
        {
            HAL_GPIO_WritePin(GPIOC, SPI2_CS0_Pin, GPIO_PIN_SET);
            return -1;
        }

    }
    else if (device == GYRO)
    {
        HAL_GPIO_WritePin(GPIOC, SPI2_CS1_Pin, GPIO_PIN_RESET);
        uint8_t set[] = {address, choice};
        if (HAL_SPI_Transmit(&hspi2, set, sizeof(set), HAL_MAX_DELAY) == HAL_OK)
        {
            HAL_GPIO_WritePin(GPIOC, SPI2_CS1_Pin, GPIO_PIN_SET);
            return 2;
        }
        else
        {
            HAL_GPIO_WritePin(GPIOC, SPI2_CS1_Pin, GPIO_PIN_SET);
            return -2;
        }

    }
    else
    {
        buzzer(325, 85);
        return -3;
    }

}

int BMI088_Read(uint8_t device, uint8_t address)
{
    HAL_StatusTypeDef status;

    if (device == ACC)
    {
        HAL_GPIO_WritePin(GPIOC, SPI2_CS0_Pin, GPIO_PIN_RESET);
        uint8_t ACC_rx_data[8];
        uint8_t tx_data[8] = {address, 0, 0, 0, 0, 0, 0, 0};
        status = HAL_SPI_TransmitReceive(&hspi2, tx_data, ACC_rx_data, 8, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOC, SPI2_CS0_Pin, GPIO_PIN_SET);

        if (status != HAL_OK)
        {
            buzzer(1500, 85);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
            return -1;
        }

        ACC_origin_data[0] = ACC_rx_data[3]<<8 | ACC_rx_data[2];
        ACC_origin_data[1] = ACC_rx_data[5]<<8 | ACC_rx_data[4];
        ACC_origin_data[2] = ACC_rx_data[7]<<8 | ACC_rx_data[6];

        ACC_check = ACC_rx_data[2];
    }
    else if (device == GYRO)
    {
        HAL_GPIO_WritePin(GPIOC, SPI2_CS1_Pin, GPIO_PIN_RESET);
        uint8_t GYRO_rx_data[7];
        uint8_t tx_data[7] = {address, 0, 0, 0, 0, 0, 0};
        status = HAL_SPI_TransmitReceive(&hspi2, tx_data, GYRO_rx_data, 7, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOC, SPI2_CS1_Pin, GPIO_PIN_SET);

        if (status != HAL_OK)
        {
            buzzer(1500, 85);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
            return -2;
        }

        GYRO_origin_data[0] = GYRO_rx_data[2]<<8 | GYRO_rx_data[1];
        GYRO_origin_data[1] = GYRO_rx_data[4]<<8 | GYRO_rx_data[3];
        GYRO_origin_data[2] = GYRO_rx_data[6]<<8 | GYRO_rx_data[5];

        GYRO_check = GYRO_rx_data[1];
    }
    else
    {
        buzzer(325, 85);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return -3;
    }

    return 1;
}

int BMI088_update(BMI088_Data_t *output)
{
    if (output == NULL || !BMI088_Read(ACC, 0x92) || !BMI088_Read(GYRO, 0x82))
    {
        buzzer(325, 85);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return -1;
    }


    output->accel_g[0] = ACC_origin_data[0]/5460.0f;
    output->accel_g[1] = ACC_origin_data[1]/5460.0f;
    output->accel_g[2] = ACC_origin_data[2]/5460.0f;

    output->gyro_dps[0] = GYRO_origin_data[0]/16.384f;
    output->gyro_dps[1] = GYRO_origin_data[1]/16.384f;
    output->gyro_dps[2] = GYRO_origin_data[2]/16.384f;

    return 1;
}

int BMI088_init(void)
{
    BMI088_Read(ACC, 0x80);                                 // 先读一次加速度计CHIP_ID，把加速度计从I2C模式切换到SPI模式
    HAL_Delay(1);                                                       // 加速度计上电后至少等待1ms再开始配置
    BMI088_Read(ACC, 0x80);//读一下地址
    BMI088_Read(GYRO, 0x80);//读一下地址
    if (ACC_check == 0x1E && GYRO_check == 0x0F)//看地址对不对
    {
        BMI088_Transmit(ACC, 0x7D, 0x04);                 // ACC_PWR_CTRL=0x04：让加速度计进入正常工作模式
        HAL_Delay(1);                                                      // 进入工作模式后至少等待450us，这里取1ms
        BMI088_Transmit(ACC, 0x7C, 0x00);                // ACC_PWR_CONF=0x00：使能连续测量，退出挂起模式
        BMI088_Transmit(ACC, 0x40, 0xAB);                // ACC_CONF=0xAB：普通低通滤波，输出数据率800Hz
        BMI088_Transmit(ACC, 0x41, 0x01);                // ACC_RANGE=0x01：加速度计量程设为±6g
        BMI088_Transmit(GYRO, 0x11, 0x00);                // GYRO_LPM1=0x00：陀螺仪进入正常模式
        BMI088_Transmit(GYRO, 0x0F, 0x00);                // GYRO_RANGE=0x00：陀螺仪量程设为±2000dps，分辨率16.384LSB/(°/s)
        BMI088_Transmit(GYRO, 0x10, 0x82);                // GYRO_BANDWIDTH=0x82：输出数据率1000Hz，滤波带宽116Hz
        HAL_Delay(50);                                                       // 等待传感器配置和输出稳定
        return 1;
    }
    else
    {
        buzzer(325, 85);//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
        return -1;
    }
}
