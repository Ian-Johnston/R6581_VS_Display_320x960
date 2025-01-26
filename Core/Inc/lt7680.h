/**
  ******************************************************************************
  * @file    lt7680.h
  * @brief   This file provides code for the setup
  *          of the LT7680
  ******************************************************************************
*/

#ifndef H
#define H

#include "stm32f1xx_hal.h"
#include "main.h"  // Assuming your pin definitions are in main.h
#include <stdint.h>

// Hardware control
void HardwareReset(void);

// Core commands
void WriteRegister(uint8_t reg);
void WriteData(uint8_t data);
uint8_t ReadStatus(void);
uint8_t ReadData(void);
void WriteDataToRegister(uint8_t reg, uint8_t value);

// Testing routines
//void OriginalFillSDRAM_LT(void);
//void BootClearToRed(void);
//void ClearSDRAM_LT(uint16_t color);
//void FillSDRAM_LT(uint16_t color);
void FillSDRAM(unsigned short color);
void TEST_Fill_initiate(void);
void TEST_SDRAM_Fill_LT(uint32_t startAddress, uint32_t length, uint16_t color);
void ClearSDRAM(void);
void SetActiveWindowFullScreen(void);
//void FastClearWithBTE(uint16_t color);
void FillSDRAMWithColor(void);
void DrawSmallSquare(void);
void WriteSinglePixel(void);
void FillSDRAMWithColorIntegrated(void);
void FillDisplayRAM(void);

// LT7680 Commands and Configuration
void SoftwareReset(void);
void SetBacklightFull(void);
void FillScreen(uint32_t color);
void SendAllToLT7680_LT(void);
//void SetBackgroundColor(color);
void Text_Mode(void);
void SetTextColors(uint32_t foreground, uint32_t background);
//void SetFontTypeSize(uint8_t fontType, uint8_t fontSize);
//void SetTextCursor(uint16_t x, uint16_t y);
//void DrawText(char* text);
//void ConfigureFontAndPosition(uint8_t fontSource, uint8_t characterHeight, uint8_t isoCoding, uint8_t fullAlignment, uint8_t chromaKeying, uint8_t rotation, uint8_t widthFactor, uint8_t heightFactor, uint8_t lineGap, uint8_t charSpacing, uint16_t cursorX, uint16_t cursorY)

// Register Configuration
void LT7680_PLL_Initial_LT(void);
void Configure_Main_PIP_Window_LT(void);
void SDRAM_Init_LT(void);
void Check_SDRAM_Ready_LT(void);
void Set_LCD_Panel_LT(void);
void LCD_HorizontalWidth_VerticalHeight_LT(uint16_t WX, uint16_t HY);
void LCD_Horizontal_Non_Display_LT(uint16_t val);
void LCD_HSYNC_Start_Position_LT(uint16_t val);
void LCD_HSYNC_Pulse_Width_LT(uint16_t val);
void LCD_Vertical_Non_Display_LT(uint16_t val);
void LCD_VSYNC_Start_Position_LT(uint16_t val);
void LCD_VSYNC_Pulse_Width_LT(uint16_t val);
void LCDConfig_LT(void);
void LCDTurnOn_LT(void);
void PWM_Prescaler_LT(void);
void PWM_Clock_Mux_LT(void);
void PWM_Configuration_LT(void);
void Software_Reset_LT(void);
void Software_ResetPLL_LT(void);
void Set_MISA_LT(void);
void SetMainImageWidth_LT(void);
void SetMainWindowUpperLeftX_LT(void);
void SetActiveWindow_LT(void);
void SetColorDepth_LT(void);
void ConfigureActiveDisplayArea_LT(void);
void ResetGraphicWritePosition_LT(void);
void SetGraphicRWYCoordinate_LT(void);
void SetCanvasStartAddress_LT(void);
void SetCanvasImageWidth_LT(void);
//void ClearScreen(void);

// Pin definitions for LT7680 controller
// The SCK, MOSI, MISO, and CS pins are defined and configured as part of the SPI peripheral initialization in the STM32 HAL driver setup.
// These are typically specified in the MX_SPI_Init function in the main.c file when using STM32CubeMX to generate code.
// SCK = PA5
// MISO = PA6
// MOSI = PA7
// CS = PA4

// Required by LT7680 controller interface
#define RESET_PIN				GPIO_PIN_10		// PB10
#define RESET_PORT				GPIOB

// GPIO macros
#define RESET_LOW()  HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_RESET)
#define RESET_HIGH() HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_SET)

// Chip Select Control Macros
#define SPI_CS_LOW()       HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET)
#define SPI_CS_HIGH()      HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET)

// TFT LCD Panel parameters - (Some of them plus refresh rate are used to calculate CCLK, MCLK & PCLK)
extern uint32_t LCD_VBPD;
extern uint32_t LCD_VFPD;
extern uint32_t LCD_VSPW;
extern uint32_t LCD_HBPD;
extern uint32_t LCD_HFPD;
extern uint32_t LCD_HSPW;
extern uint32_t REFRESH_RATE;

// These have been moved to main.c as part of the user selectable timings
//#define LCD_VBPD				17			// Vertical Back Porch				17	17	17
//#define LCD_VFPD				14			// Vertical Front Porch				15	14	14		Adafruit tft timings say 15 but per forum user changed to 14 to stop flickering
//#define LCD_VSPW				4			// Vertical Start Position			2	4	2		Adafruit tft timings says 2 but leaves single vertical line at far right, 4 is no line but may flicker, 2 works better but has line
//#define LCD_HBPD				50			// Horizontal Back Porch			50
//#define LCD_HFPD				30			// Horizontal Front Porch			30
//#define LCD_HSPW				10			// Horizontal Start Position		10
//#define REFRESH_RATE			60			// Hz								60

#define LCD_XSIZE_TFT			400			// Resolution - Horizontal pixels (320 LCD is the 400 with first 80 missing)
#define LCD_YSIZE_TFT			960			// Resolution - Vertical Pixels
#define HSYNC_ACTIVE			0			// 0 = HSYNC Low Active
#define VSYNC_ACTIVE			0			// 0 = VSYNC Low Active
#define DE_ACTIVE				0			// 0 = DE High Active
#define PCLK_EDGE				0			// 0 = PCLK Rising Edge
#define HSYNC_IDLE_STATE		1			// HSYNC Idle State
#define VSYNC_IDLE_STATE		1			// VSYNC Idle State
#define PDE_IDLE_STATE			0			// PDE Idle STATE
#define PCLK_IDLE_STATE			0			// PCLK Idle State
#define PD_IDLE_STATE			0			// PD Idle State
#define SCLK_MAX				65			// Max allowable	65
#define MCLK_MAX				100			// Max allowable	100
#define CCLK_MAX				100			// Max allowable	100
#define SDRAM_MCLK				20			// MHz
#define SDRAM_CLKFREQ			64000000	// SDRAM clock frequency in Hz (clock source)		64000000
#define SDRAM_SIZE				8192		// 2048 = 64Mb, 8192 = 128Mb (depends on LT768x used), LT7680A-R is 128Mb
#define VSCAN_DIRECTION			0			// 0 = VSCAN Top to Bottom
#define TFT_BIT					0b01		// 0b01 = 18bit, 0b00 = 24bit, 0b10 = 16bit, 0b11 = No TFT output - 18bit bus LT7680 to ST7701S
#define HOST_BUS				0			// 0 = 8bit, 1 = 16bit
#define OUTPUT_SEQ				0b000		// 0b000 = RGB, 0b001 = RBG, 0b010 = GRB, 0b011 = GBR, 0b100 = BRG, 0b101 = BGR, 0b110 = Grey, 0b111 = Idle State			100
#define REG_CONTROL				0x00		// Control register address
#define REG_TEXT_CURSOR_X       0x20		// X-coordinate of text cursor
#define REG_TEXT_CURSOR_Y       0x21		// Y-coordinate of text cursor
#define MAIN_IMAGE_START		0x000000	// Main image start address set to 0 (MISA)			0x20004000
#define PD_OUTPUT_SEQ			0b000		// Parallel PD[23:0] Output Sequence				RGB

// User
#define DISP_TEST				0			// Display the LT7680 test card: 1 = test card, 0 = normal operation
#define BACKLIGHTFULL			100			// Backlighting brightness 0-100%
#define BACKLIGHTOFF			0			// Backlighting brightness 0-100%

#endif
