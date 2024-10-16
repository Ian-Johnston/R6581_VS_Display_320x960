/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TEST_OUT_Pin GPIO_PIN_13
#define TEST_OUT_GPIO_Port GPIOC
#define OLED_DC_Pin GPIO_PIN_2
#define OLED_DC_GPIO_Port GPIOA
#define OLED_RES_Pin GPIO_PIN_3
#define OLED_RES_GPIO_Port GPIOA
#define OLED_CS_Pin GPIO_PIN_4
#define OLED_CS_GPIO_Port GPIOA
#define OLED_SCK_Pin GPIO_PIN_5
#define OLED_SCK_GPIO_Port GPIOA
#define OLED_SDA_Pin GPIO_PIN_7
#define OLED_SDA_GPIO_Port GPIOA
#define VFD_RESTART_Pin GPIO_PIN_11
#define VFD_RESTART_GPIO_Port GPIOB
#define VFD_RESTART_EXTI_IRQn EXTI15_10_IRQn
#define VFD_SCK_Pin GPIO_PIN_13
#define VFD_SCK_GPIO_Port GPIOB
#define VFD_SDA_Pin GPIO_PIN_15
#define VFD_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

// The number of bytes in one data packet loaded into the U4 shift register
#define PACKET_WIDTH    5
// Number of packets in one display refresh cycle
#define PACKET_COUNT    47
// Total number of character cells on the display 
#define CHAR_COUNT      PACKET_COUNT
// Number of rows in the character matrix
#define CHAR_HEIGHT     7
// Number of columns in the character matrix
#define CHAR_WIDTH      5
// Number of characters in the first (main) line of the display
#define LINE1_LEN       18
// Number of characters in the second (auxiliary) line of the display
#define LINE2_LEN       29
// Vertical offset of the first line (in pixels)
#define LINE1_Y         10
// Second line vertical offset (in pixels)
#define LINE2_Y         50

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
