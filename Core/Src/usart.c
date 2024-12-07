/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for debugging
  *          
  ******************************************************************************
*/

#include "usart.h"
#include "stm32f1xx_hal.h" // Adjust for STM32F1

UART_HandleTypeDef huart1; // Replace with the correct instance (e.g., huart2 for USART2)

void USART_Init(void) {
    // USART Initialization Code
    huart1.Instance = USART1;  // Replace with the correct USART instance
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;

    if (HAL_UART_Init(&huart1) != HAL_OK) {
        // Initialization Error
        Error_Handler(); // Implement your error handler
    }
}

void USART_Transmit(char *data, uint16_t size) {
    HAL_UART_Transmit(&huart1, (uint8_t *)data, size, HAL_MAX_DELAY);
}

void USART_Receive(uint8_t *buffer, uint16_t size) {
    HAL_UART_Receive(&huart1, buffer, size, HAL_MAX_DELAY);
}


