/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "spi.h"
#include "lt7680.h"
#include "main.h"

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/

void MX_GPIO_Init(void) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
	
    /* Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(TEST_OUT_GPIO_Port, TEST_OUT_Pin, GPIO_PIN_RESET);

    /* Configure GPIO pin : TEST_OUT_Pin */
    GPIO_InitStruct.Pin = TEST_OUT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TEST_OUT_GPIO_Port, &GPIO_InitStruct);

    /* Configure GPIO pin for LT7680 RESET */
    GPIO_InitStruct.Pin = RESET_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RESET_PORT, &GPIO_InitStruct);

    /* Configure GPIO pin : VFD_RESTART_Pin */
    GPIO_InitStruct.Pin = VFD_RESTART_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(VFD_RESTART_GPIO_Port, &GPIO_InitStruct);
	
	
	
	// Configure bit bang SPI pins for direct LCD SPI
	GPIO_InitStruct.Pin = LCD_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(LCD_CS_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = LCD_SCK_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(LCD_SCK_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = LCD_SDI_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(LCD_SDI_Port, &GPIO_InitStruct);



    // Configure GPIO pin B0
    GPIO_InitStruct.Pin = GPIO_PIN_0;       // Select pin B0
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Set as input
    GPIO_InitStruct.Pull = GPIO_PULLUP;   // Enable pull-up resistor
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // Low speed is sufficient for input
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	
	
    /* Configure GPIO pin : LT7680 MISO Pin */
    //GPIO_InitStruct.Pin = LT7680_SPI_MISO_PIN;   // MISO = PA6
    //GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;  // Alternate function input mode
	//GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    //GPIO_InitStruct.Pull = GPIO_NOPULL;         // No pull-up or pull-down resistor
	//GPIO_InitStruct.Pull = GPIO_PULLUP;         // Pull-up resistor
    //GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  // High speed for SPI
    //HAL_GPIO_Init(LT7680_SPI_MISO_PORT, &GPIO_InitStruct);

    /* EXTI interrupt init */
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

