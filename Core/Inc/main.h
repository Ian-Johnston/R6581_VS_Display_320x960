/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif


	
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/
#define TEST_OUT_Pin GPIO_PIN_13				// PC13 - LED
#define TEST_OUT_GPIO_Port GPIOC
	
// SPI Pin Definitions
#define SPI_SCK_PIN        GPIO_PIN_5       // SCK = PA5
#define SPI_SCK_PORT       GPIOA

#define SPI_MOSI_PIN       GPIO_PIN_7       // MOSI = PA7
#define SPI_MOSI_PORT      GPIOA

#define SPI_MISO_PIN       GPIO_PIN_6       // MISO = PA6
#define SPI_MISO_PORT      GPIOA
	
#define SPI_CS_PIN         GPIO_PIN_4       // CS = PA4
#define SPI_CS_PORT        GPIOA

#define VFD_RESTART_Pin GPIO_PIN_11				// PB11
#define VFD_RESTART_GPIO_Port GPIOB
#define VFD_RESTART_EXTI_IRQn EXTI15_10_IRQn

#define VFD_SCK_Pin GPIO_PIN_13					// PB13
#define VFD_SCK_GPIO_Port GPIOB

#define VFD_SDA_Pin GPIO_PIN_15					// PB15
#define VFD_SDA_GPIO_Port GPIOB
// Note: PB10 lt7680 reset pin is in lt7680.h

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


//**************************************************************************************************
// ST7701A LCD Controller
	
void DelayMicroseconds(uint16_t us);	
	
// Define LCD SPI pins - bit bang SPI port for connection to the LCD ST7701A controller
#define LCD_CS_Pin    GPIO_PIN_3    // PB3
#define LCD_SCK_Pin   GPIO_PIN_4    // PB4
#define LCD_SDI_Pin   GPIO_PIN_5    // PB5
#define LCD_CS_Port   GPIOB
#define LCD_SCK_Port  GPIOB
#define LCD_SDI_Port  GPIOB

	
	
	
	
	
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
