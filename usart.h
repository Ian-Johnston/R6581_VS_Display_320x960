/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file provides code for debugging
  *          
  ******************************************************************************
*/

#ifndef __USART_H
#define __USART_H

#include "stm32f1xx_hal.h" // Use the STM32F1 HAL library

// Define the USART handle (declare it extern if it's defined elsewhere)
extern UART_HandleTypeDef huart1; // Adjust for the USART instance you are using

// Function prototypes
void USART_Init(void);
void USART_Transmit(char *data, uint16_t size);
void USART_Receive(uint8_t *buffer, uint16_t size);

#endif /* __USART_H */
