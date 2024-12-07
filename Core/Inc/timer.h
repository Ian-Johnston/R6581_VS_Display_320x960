/**
  ******************************************************************************
  * @file    timer.h
  * @brief   This file contains all the function prototypes for
  *          the timer.c file
  ******************************************************************************
*/

#ifndef TIMER_H
#define TIMER_H

#include "stm32f1xx.h" // Include the STM32 HAL/LL header file

// Externally accessible variables
extern volatile uint8_t timer_flag;
extern volatile uint8_t task_ready;

// Function prototypes
void TIM2_Init(void);
void TIM2_IRQHandler(void);
void SetTimerDuration(uint16_t ms);

#endif // TIMER_H


