/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "attitude.h"
#include "motor.h"
#include "freertos_uart.h"
#include "imu.h"
#include "pid.h"
#include "remote.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
PID_member speed_pid;
remote_data_type remote_data_a;
GM6020_motor_status GM6020_motor_status_a;
int16_t goal = 0;
int16_t real = 0;
double output = 0;
int statu = 0;
int last_statu = 0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myTask02 */
osThreadId_t myTask02Handle;
const osThreadAttr_t myTask02_attributes = {
  .name = "myTask02",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of myTask02 */
  myTask02Handle = osThreadNew(StartTask02, NULL, &myTask02_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */

/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */

  PID_Init(&speed_pid, 10, 0, 0, 0.001, 600, 10000);
  /* Infinite loop */
  for(;;)
  {
    GM6020_Get_Status(1, &GM6020_motor_status_a);
    Get_Remote_data(&remote_data_a);
    output = PID(&speed_pid, 3 * remote_data_a.ch0, GM6020_motor_status_a.speed_rpm);
    if (can[1].online == 1 && remote_data_a.online == 1 && GM6020_motor_status_a.online == 1)
    {
      if (remote_data_a.sw1 == 1 && remote_data_a.sw2 == 1)
      {
        motor_control(1 ,GM6020_I_control_1234, output, 0, 0, 0);
        statu = 1;
      }
      else
      {
        motor_control(1 ,GM6020_I_control_1234, 0, 0, 0, 0);
        statu = 0;
      }
    }
    else
    {
      motor_control(1 ,GM6020_I_control_1234, 0, 0, 0, 0);
      statu = -1;
    }

    if (last_statu != statu)
    {
      if (statu == 0)
      {
        LED(0,200,0);
      }
      if (statu == 1)
      {
        LED(0,0,200);
      }
      if (statu == -1)
      {
        LED(200,0,0);
      }
    }
      last_statu = statu;
      osDelay(1);
    }
    /* USER CODE END StartDefaultTask */
}

  /* USER CODE BEGIN Header_StartTask02 */
  /**
  * @brief Function implementing the myTask02 thread.
  * @param argument: Not used
  * @retval None
  */
  /* USER CODE END Header_StartTask02 */
  void StartTask02(void *argument)
  {
    /* USER CODE BEGIN StartTask02 */
    // Attitude attitude;
    // Attitude_Init(&attitude);
    /* Infinite loop */
    for(;;)
    {
      // BMI088_Data_t data;
      // if (BMI088_update(&data))
      // {
      //   Attitude_Update(
      //       &attitude,
      //       data.gyro_dps,
      //       data.accel_g,
      //       0.01f
      //   );
      // }
      goal = 3 * remote_data_a.ch0;
      real = GM6020_motor_status_a.speed_rpm;
      UART_printf_Transmit(1, "%d, %f, %d, %d, %d, %d\n", goal, output, real, can[1].online, remote_data_a.online, GM6020_motor_status_a.online/*attitude.roll, attitude.pitch, attitude.yaw*/);
      osDelay(20);
    }
    /* USER CODE END StartTask02 */
  }
  /* Private application code --------------------------------------------------*/
  /* USER CODE BEGIN Application */

  /* USER CODE END Application */
