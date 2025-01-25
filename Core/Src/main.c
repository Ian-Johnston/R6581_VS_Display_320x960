/*
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  *
  * Original code by MickleT, this version modded by Ian Johnston (IanSJohnston on YouTube),
  * for 4.58" 960x320 TFT LCD (ST7701S) and using LT7680 controller adaptor
  * Visual Studio 2022 with VisualGDB plugin:
  * To upload HEX from VS2022 = BUILD then PROGRAM AND START WITHOUT DEBUGGING
  * Use LIVE WATCH to view variables live debug
  * 
  * For use with a LT7680A-R & 320x960 TFT LCD
  *
  ******************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "gpio.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "lt7680.h"
#include "timer.h"
#include <stdbool.h>    // bool support, otherwise use _Bool
//#include <stdlib.h> // For rand()
#include "display.h"
#include "stm32f1xx_hal.h"
#include <stdlib.h>		// required for float (soft FPU)

/* Variables ---------------------------------------------------------*/
static char main_display_debug[LINE1_LEN + 1]; // Main display debug string
#define FONT_HEIGHT 7
uint16_t dollarPosition = 0;
_Bool oneVoltmode = false;
_Bool oneVoltmodepreviousState = false;

// EEProm emulation (Flash)
#define EEPROM_START_ADDRESS 0x0800FC00 // Last page for 64KB Flash
#define EEPROM_PAGE_SIZE     1024       // Page size in bytes

// TFT timing vars
_Bool timingModsOnBoot = false;
_Bool timingModsOnBootDCV = false;
_Bool timingModspreviousstate = false;
uint8_t currentTimingSet = 0;		// Variable to track the current timing set (0 to 5)
static bool isFirstPress = true; // Tracks whether this is the first press
const uint32_t LCD_VBPD_SETTINGS[6]         = { 17, 17, 17, 17, 17, 10 };		// Define the timing settings for each mode
const uint32_t LCD_VFPD_SETTINGS[6]         = { 14, 14, 14, 15, 15, 12 };
const uint32_t LCD_VSPW_SETTINGS[6]         = { 2,  3,  4,  2,  2,  3 };
const uint32_t LCD_HBPD_SETTINGS[6]         = { 50, 50, 50, 50, 50, 80 };
const uint32_t LCD_HFPD_SETTINGS[6]         = { 30, 30, 30, 30, 30, 30 };
const uint32_t LCD_HSPW_SETTINGS[6]         = { 10, 10, 10, 10, 10, 20 };
const uint32_t REFRESH_RATE_SETTINGS[6]     = { 60, 60, 60, 60, 45, 60 };
const char ADA_BUY_SETTINGS[6][5]           = {"AdaF", "AdaF", "AdaF", "AdaF", "AdaF", "BuyD"};		// 4 characters plus the null terminator('\0').
uint8_t currentTimingSetNumberentries = 6;	// number of entries	
uint32_t LCD_VBPD = 17;
uint32_t LCD_VFPD = 14;
uint32_t LCD_VSPW = 2;
uint32_t LCD_HBPD = 50;
uint32_t LCD_HFPD = 30;
uint32_t LCD_HSPW = 10;
uint32_t REFRESH_RATE = 60;
char ADA_BUY[5] = "AdaF";

// Buffers for each LCD graphical item
char LCD_buffer_packets[128];  // For packet data
char LCD_buffer_bitmaps[128];  // For decoded bitmap data
char LCD_buffer_chars[128];    // For decoded characters

char G[48];  // MAIN: G1 to G18, AUX: G19 to G47
_Bool Annunc[19]; // Annunciators re-ordered, 18off, G1 to G18 in left-to-right order
_Bool AnnuncTemp[37]; // Temp array for annunciators. 18off, the order on LCD left to right = 8,7,6,5,4,3,2,1,18,17,16,15,14,13,12,11,10,9

// Global variable to store the unmatched bitmap
uint8_t unmatchedBitmap[FONT_HEIGHT] = { 0 }; // Initialize to zero

//******************************************************************************

// SPI receive buffer for packets data
volatile uint8_t rx_buffer[PACKET_WIDTH * PACKET_COUNT];

// Array with character bitmaps
uint8_t chars[CHAR_COUNT][CHAR_HEIGHT];

// Array with annunciators flags (boolean)
uint8_t flags[CHAR_COUNT];

// When scanning the display, the order of the characters output is not sequential due to optimization of the VFD PCB layout.
// The Reorder[] array is used as a lookup table to determine the correct position of characters.
const uint8_t Reorder[PACKET_COUNT] = { 8, 7, 6, 5, 4, 3, 2, 1, 0, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 17, 16, 15, 14, 13, 12, 11, 10, 9, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36 };
// Order of data to the display (effectively into the shift registers):
// 8, 7, 6, 5, 4, 3, 2, 1, 0,                                               // MAIN: G9 to G1
// 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,  // AUX: G19 to G36
// 17, 16, 15, 14, 13, 12, 11, 10, 9,                                       // MAIN: G18 to G10
// 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36                               // AUX: G47 to G37
// But the actual display left to right:
// G1 to G18 - MAIN
// G19 to G47 - AUX
// Annunc[1] to Annunc[18]
// Annunciators = SMPL IDLE AUTO LOP NULL DFILT MATH AZERO ERR INFO FRONT REAR SLOT LO_G RMT TLK LTN SRQ

// Flag indicating finish of SPI transmission to OLED
volatile uint8_t SPI1_TX_completed_flag = 1;

// Flag indicating finish of SPI start-up initialization
volatile uint8_t Init_Completed_flag = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

//******************************************************************************

// TFT LCD settings
uint32_t boot_LCD_VBPD;
uint32_t boot_LCD_VFPD;
uint32_t boot_LCD_VSPW;
uint32_t boot_LCD_HBPD;
uint32_t boot_LCD_HFPD;
uint32_t boot_LCD_HSPW;
uint32_t boot_REFRESH_RATE;
char boot_ADA_BUY[5];
uint32_t setting_LCD_VBPD;
uint32_t setting_LCD_VFPD;
uint32_t setting_LCD_VSPW;
uint32_t setting_LCD_HBPD;
uint32_t setting_LCD_HFPD;
uint32_t setting_LCD_HSPW;
uint32_t setting_REFRESH_RATE;
char setting_ADA_BUY[5];

//******************************************************************************

// Buffer to store the converted string representation of the main display line
//char main_display_line[CHAR_COUNT + 1]; // +1 for null terminator
static char main_display_line[CHAR_COUNT + 1]; // Static ensures scope is global within the file


//SPI transmission finished interrupt callback
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi) {
	if (hspi->Instance == SPI1)
	{
		SPI1_TX_completed_flag = 1;
	}
}


//******************************************************************************
static uint8_t InverseByte(uint8_t a) {
	a = ((a & 0x55) << 1) | ((a & 0xAA) >> 1);
	a = ((a & 0x33) << 2) | ((a & 0xCC) >> 2);
	return (a >> 4) | (a << 4);
}


//******************************************************************************
// Function to map character bitmaps to ASCII characters
typedef struct {
	uint8_t bitmap[7]; // 7 bytes for 5x7 character bitmaps
	char ascii;        // Corresponding ASCII character
} BitmapChar;


// Font data: 96 characters, 7 bytes per character (each row)
// These are relative to ISO 8859-1 which is the font installed in the LT7680A-R
const BitmapChar bitmap_characters[] = {
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, ' '},  // Space
	{{0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04}, '!'},  // 0x21, !
	{{0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00}, '"'},  // 0x22, "
	{{0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A}, '#'},  // 0x23, #
	{{0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04}, '$'},  // 0x24, $
	{{0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03}, '%'},  // 0x25, %
	{{0x04, 0x0A, 0x0A, 0x0A, 0x15, 0x12, 0x0D}, '&'},  // 0x26, &
	{{0x04, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00}, '\''}, // 0x27, '
	{{0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02}, '('},  // 0x28, (
	{{0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08}, ')'},  // 0x29, )
	{{0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00}, '*'},  // 0x2A, *
	{{0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00}, '+'},  // 0x2B, +
	{{0x00, 0x00, 0x00, 0x00, 0x0C, 0x04, 0x08}, ','},  // 0x2C, ,
	{{0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}, '-'},  // 0x2D, -
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C}, '.'},  // 0x2E, .
	{{0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10}, '/'},  // 0x2F, /
	{{0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, '0'},  // 0x30, 0
	{{0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, '1'},  // 0x31, 1
	{{0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, '2'},  // 0x32, 2
	{{0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, '3'},  // 0x33, 3
	{{0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, '4'},  // 0x34, 4
	{{0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, '5'},  // 0x35, 5
	{{0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, '6'},  // 0x36, 6
	{{0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, '7'},  // 0x37, 7
	{{0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, '8'},  // 0x38, 8
	{{0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}, '9'},  // 0x39, 9
	{{0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00}, ':'},  // 0x3A, :
	{{0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x04, 0x08}, ';'},  // 0x3B, ;
	{{0x00, 0x02, 0x06, 0x0E, 0x06, 0x02, 0x00}, '\x11'},  // 0x3C, <
	{{0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00}, '='},  // 0x3D, =
	{{0x00, 0x08, 0x0C, 0x0E, 0x0C, 0x08, 0x00}, '\x10'},  // 0x3E, >
	{{0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04}, '?'},  // 0x3F, ?
	{{0x0E, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0F}, '@'},  // 0x40, @
	{{0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, 'A'},  // 0x41, A
	{{0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, 'B'},  // 0x42, B
	{{0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, 'C'},  // 0x43, C
	{{0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C}, 'D'},  // 0x44, D
	{{0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}, 'E'},  // 0x45, E
	{{0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}, 'F'},  // 0x46, F
	{{0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}, 'G'},  // 0x47, G
	{{0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, 'H'},  // 0x48, H
	{{0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, 'I'},  // 0x49, I
	{{0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}, 'J'},  // 0x4A, J
	{{0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, 'K'},  // 0x4B, K
	{{0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, 'L'},  // 0x4C, L
	{{0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}, 'M'},  // 0x4D, M
	{{0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}, 'N'},  // 0x4E, N
	{{0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, 'O'},  // 0x4F, O
	{{0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}, 'P'},  // 0x50, P
	{{0x0E, 0x11, 0x11, 0x11, 0x15, 0x13, 0x0D}, 'Q'},  // 0x51, Q
	{{0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, 'R'},  // 0x52, R
	{{0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}, 'S'},  // 0x53, S
	{{0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, 'T'},  // 0x54, T
	{{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, 'U'},  // 0x55, U
	{{0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}, 'V'},  // 0x56, V
	{{0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}, 'W'},  // 0x57, W
	{{0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}, 'X'},  // 0x58, X
	{{0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04}, 'Y'},  // 0x59, Y
	{{0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}, 'Z'},  // 0x5A, Z
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, '['},  // 0x5B, [
	{{0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04}, '\\'}, // 0x5C, backslash
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, ']'},  // 0x5D, ]
	{{0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x01}, '^'},  // 0x5E, ^
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F}, '_'},  // 0x5F, _
	{{0x01, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00}, '`'},  // 0x60, `
	{{0x00, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F}, 'a'},  // 0x61, a
	{{0x10, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x1E}, 'b'},  // 0x62, b
	{{0x00, 0x00, 0x0F, 0x10, 0x10, 0x10, 0x0F}, 'c'},  // 0x63, c
	{{0x01, 0x01, 0x01, 0x0F, 0x11, 0x11, 0x0F}, 'd'},  // 0x64, d
	{{0x00, 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0F}, 'e'},  // 0x65, e
	{{0x02, 0x05, 0x04, 0x1F, 0x04, 0x04, 0x04}, 'f'},  // 0x66, f
	{{0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x1F}, 'g'},  // 0x67, g
	{{0x10, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x11}, 'h'},  // 0x68, h
	{{0x00, 0x04, 0x00, 0x04, 0x04, 0x04, 0x04}, 'i'},  // 0x69, i
	{{0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0C}, 'j'},  // 0x6A, j
	{{0x08, 0x08, 0x09, 0x0A, 0x0C, 0x0A, 0x09}, 'k'},  // 0x6B, k
	{{0x0C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, 'l'},  // 0x6C, l
	{{0x00, 0x00, 0x1A, 0x15, 0x15, 0x15, 0x11}, 'm'},  // 0x6D, m
	{{0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11}, 'n'},  // 0x6E, n
	{{0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E}, 'o'},  // 0x6F, o
	{{0x00, 0x00, 0x1E, 0x11, 0x1E, 0x10, 0x10}, 'p'},  // 0x70, p
	{{0x00, 0x00, 0x0D, 0x13, 0x0F, 0x01, 0x01}, 'q'},  // 0x71, q
	{{0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10}, 'r'},  // 0x72, r
	{{0x00, 0x00, 0x0F, 0x10, 0x0E, 0x01, 0x1E}, 's'},  // 0x73, s
	{{0x04, 0x04, 0x1F, 0x04, 0x04, 0x05, 0x02}, 't'},  // 0x74, t
	{{0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0D}, 'u'},  // 0x75, u
	{{0x00, 0x00, 0x11, 0x11, 0x11, 0x0A, 0x04}, 'v'},  // 0x76, v
	{{0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A}, 'w'},  // 0x77, w
	{{0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x00}, 'x'},  // 0x78, x	{0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11}
	{{0x00, 0x00, 0x11, 0x11, 0x0F, 0x01, 0x1E}, 'y'},  // 0x79, y
	{{0x00, 0x00, 0x1F, 0x02, 0x04, 0x08, 0x1F}, 'z'},  // 0x7A, z
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, '{'},  // 0x7B, {
	{{0x01, 0x02, 0x04, 0x00, 0x04, 0x02, 0x01}, '|'},  // 0x7C, |
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, '}'},  // 0x7D, }
	{{0x00, 0x00, 0x09, 0x15, 0x12, 0x00, 0x00}, '~'},  // 0x7E, ~
	{{0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00}, '/'},  // forward slash
	{{0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00}, '°'},	// DegC symbol
	{{0x0E, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x1B}, '$'},	// Ohm symbol placeholder uses the $ symbol for detection of Ohm symbol
	{{0x11, 0x12, 0x14, 0x0B, 0x11, 0x02, 0x03}, '½'},	// half symbol
	{{0x00, 0x00, 0x04, 0x0E, 0x1F, 0x00, 0x00}, '\x1E'},	// up arrow
	{{0x00, 0x00, 0x1F, 0x0E, 0x04, 0x00, 0x00}, '\x1F'},	// down arrow
	{{0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x16}, '\xB5' },	// micro u
	{{0x00, 0x04, 0x02, 0x1F, 0x02, 0x04, 0x00}, '\x1A' },	// arrow right
	{{0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}, '\x08' },	// Diag mode display check 1		Unit separator usually. all pixels lit, this one appears on the display chack and the memorycard file name selection (albeit invalid)
	{{0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}, '\x01' },	// Diag mode display check 2
	{{0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}, '\x02' },	// Diag mode display check 3
	{{0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F}, '\x03' },	// Diag mode display check 4
	{{0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F}, '\x04' },	// Diag mode display check 5
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F}, '\x05' },	// Diag mode display check 6
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F}, '\x06' },	// Diag mode display check 7
	{{0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F}, '\x07' },	// Diag mode display check 8
	{{0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07}, '\x0B' },	// Diag mode display check 9
	{{0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03}, '\x0C' },	// Diag mode display check 10
	{{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}, '\x16' },	// Diag mode display check 11
};


// Convert a 5x7 bitmap to an ASCII character
// The bitmap (7 rows of 5 bits each) is compared row by row against the font_data.
// The comparison involves the 7 rows of the bitmap against the corresponding 7 rows in each font_data entry.
char BitmapToChar(const uint8_t* bitmap) {
	// Iterate over the bitmap_characters array
	for (int i = 0; i < sizeof(bitmap_characters) / sizeof(BitmapChar); i++) {
		// Compare the input bitmap with the current character's bitmap
		if (memcmp(bitmap, bitmap_characters[i].bitmap, FONT_HEIGHT) == 0) {
			return bitmap_characters[i].ascii; // Return the matching ASCII character
		}
	}

	// uncomment this to capture the unmatched bitmap and use LIVE WATCH to display the array for it
	memcpy(unmatchedBitmap, bitmap, FONT_HEIGHT);

	// If no match is found, return '?'
	return '?';
}


//******************************************************************************

// Each character on the display is encoded by a matrix of 40 bits packed
// into 5 consecutive bytes. 5x7=35 bits (S1-S35) define the pixel image of the character,
// 1 bit (S36) is the annunciator, 4 bits are not used. To optimize VFD PCB routing,
// the bit order in packets is shuffled:
//
// S17 S16 S15 S14 S13 S12 S11 S10
// S9  S8  S7  S6  S5  S4  S3  S2
// S1  S36 0   0   0   0   S35 S34
// S33 S32 S31 S30 S29 S28 S27 S26
// S25 S24 S23 S22 S21 S20 S19 S18
//
// The Packets_to_chars function sorts the character bitmap, extracts the annunciator
// flag, and stores the result in separate arrays chars[][] and flags[]
//
// 0 0 0 S1  S2  S3  S4  S5
// 0 0 0 S6  S7  S8  S9  S10
// 0 0 0 S11 S12 S13 S14 S15
// 0 0 0 S16 S17 S18 S19 S20
// 0 0 0 S21 S22 S23 S24 S25
// 0 0 0 S26 S27 S28 S29 S30
// 0 0 0 S31 S32 S33 S34 S35
//
void Packets_to_chars(void) {
	for (int i = 0; i < PACKET_COUNT; i++) {
		uint8_t d0 = rx_buffer[i * PACKET_WIDTH + 0];
		uint8_t d1 = rx_buffer[i * PACKET_WIDTH + 1];
		uint8_t d2 = rx_buffer[i * PACKET_WIDTH + 2];
		uint8_t d3 = rx_buffer[i * PACKET_WIDTH + 3];
		uint8_t d4 = rx_buffer[i * PACKET_WIDTH + 4];

		chars[Reorder[i]][0] = 0x1F & InverseByte((d1 << 4) | ((d2 & 0x80) >> 4));
		chars[Reorder[i]][1] = 0x1F & InverseByte((d0 << 7) | ((d1 & 0xF0) >> 1));
		chars[Reorder[i]][2] = 0x1F & InverseByte((d0 & 0xFE) << 2);
		chars[Reorder[i]][3] = 0x1F & InverseByte(((d0 & 0xC0) >> 3) | (d4 << 5));
		chars[Reorder[i]][4] = 0x1F & InverseByte(d4 & 0xF8);
		chars[Reorder[i]][5] = 0x1F & InverseByte(d3 << 3);
		chars[Reorder[i]][6] = 0x1F & InverseByte((d2 << 6) | ((d3 & 0xE0) >> 2));
		flags[Reorder[i]] = (d2 & 0x40) == 0x40;

		// Update annunciator boolean array for MAIN annunciators (G1 to G18)
		if (i < 37) {
			AnnuncTemp[i] = flags[Reorder[i]];
		}
	}
	// Null-terminate the main display line string
	main_display_line[LINE1_LEN] = '\0';
}


void ReorderAnnunciators(void) {
	// Map AnnuncTemp[] to Annunc[] for left-to-right order.
	// 8, 7, 6, 5, 4, 3, 2, 1, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9
	// See 'Order of data packets' on MickleT's schematic
	Annunc[1] = AnnuncTemp[8];  // G1		SAMPLE
	Annunc[2] = AnnuncTemp[7];  // G2		IDLE
	Annunc[3] = AnnuncTemp[6];  // G3		AUTO
	Annunc[4] = AnnuncTemp[5];  // G4		LOP
	Annunc[5] = AnnuncTemp[4];  // G5		NULL
	Annunc[6] = AnnuncTemp[3];  // G6		DFILT
	Annunc[7] = AnnuncTemp[2];  // G7		MATH
	Annunc[8] = AnnuncTemp[1];  // G8		AZERO
	Annunc[9] = AnnuncTemp[0];  // G9		ERR
	Annunc[10] = AnnuncTemp[35]; // G10		INFO
	Annunc[11] = AnnuncTemp[34]; // G11		FRONT
	Annunc[12] = AnnuncTemp[33]; // G12		REAR
	Annunc[13] = AnnuncTemp[32]; // G13		SLOT
	Annunc[14] = AnnuncTemp[31]; // G14		LO.G
	Annunc[15] = AnnuncTemp[30]; // G15		RMT
	Annunc[16] = AnnuncTemp[29]; // G16		TLK
	Annunc[17] = AnnuncTemp[28]; // G17		LSN
	Annunc[18] = AnnuncTemp[27]; // G18		SRQ
}


void Main_Aux_R6581(void) {
	// Clear LCD buffers
	memset(LCD_buffer_packets, 0, sizeof(LCD_buffer_packets));
	memset(LCD_buffer_bitmaps, 0, sizeof(LCD_buffer_bitmaps));
	memset(LCD_buffer_chars, 0, sizeof(LCD_buffer_chars));

	ReorderAnnunciators();  // re-order the annunciators so Annunnciator[1] is above G1
	//char annunciator_debug[256] = "Annunciators: "; // Buffer for annunciator state debug

	for (int i = 0; i <= 17; i++) {      // G1 to G18
		// Use already-decoded data from Packets_to_chars
		uint8_t* bitmap = chars[i]; // Get the bitmap for this character
		char ascii_char = BitmapToChar(bitmap); // Convert bitmap to ASCII character

		// MAIN Update individual variables G1 to G18
		if (i == 0) G[1] = ascii_char;
		else if (i == 1) G[2] = ascii_char;
		else if (i == 2) G[3] = ascii_char;
		else if (i == 3) G[4] = ascii_char;
		else if (i == 4) G[5] = ascii_char;
		else if (i == 5) G[6] = ascii_char;
		else if (i == 6) G[7] = ascii_char;
		else if (i == 7) G[8] = ascii_char;
		else if (i == 8) G[9] = ascii_char;
		else if (i == 9) G[10] = ascii_char;
		else if (i == 10) G[11] = ascii_char;
		else if (i == 11) G[12] = ascii_char;
		else if (i == 12) G[13] = ascii_char;
		else if (i == 13) G[14] = ascii_char;
		else if (i == 14) G[15] = ascii_char;
		else if (i == 15) G[16] = ascii_char;
		else if (i == 16) G[17] = ascii_char;
		else if (i == 17) G[18] = ascii_char;
	}

	// Null-terminate the Main display debug string
	main_display_debug[LINE1_LEN] = '\0';

	for (int i = 18; i <= 46; i++) {      // G19 to G47
		// Use already-decoded data from Packets_to_chars
		uint8_t* bitmap = chars[i]; // Get the bitmap for character
		char ascii_char = BitmapToChar(bitmap); // Convert bitmap to ASCII character

		// AUX Update individual variables
		if (i == 18) G[19] = ascii_char;
		else if (i == 19) G[20] = ascii_char;
		else if (i == 20) G[21] = ascii_char;
		else if (i == 21) G[22] = ascii_char;
		else if (i == 22) G[23] = ascii_char;
		else if (i == 23) G[24] = ascii_char;
		else if (i == 24) G[25] = ascii_char;
		else if (i == 25) G[26] = ascii_char;
		else if (i == 26) G[27] = ascii_char;
		else if (i == 27) G[28] = ascii_char;
		else if (i == 28) G[29] = ascii_char;
		else if (i == 29) G[30] = ascii_char;
		else if (i == 30) G[31] = ascii_char;
		else if (i == 31) G[32] = ascii_char;
		else if (i == 32) G[33] = ascii_char;
		else if (i == 33) G[34] = ascii_char;
		else if (i == 34) G[35] = ascii_char;
		else if (i == 35) G[36] = ascii_char;
		else if (i == 36) G[37] = ascii_char;
		else if (i == 37) G[38] = ascii_char;
		else if (i == 38) G[39] = ascii_char;
		else if (i == 39) G[40] = ascii_char;
		else if (i == 40) G[41] = ascii_char;
		else if (i == 41) G[42] = ascii_char;
		else if (i == 42) G[43] = ascii_char;
		else if (i == 43) G[44] = ascii_char;
		else if (i == 44) G[45] = ascii_char;
		else if (i == 45) G[46] = ascii_char;
		else if (i == 46) G[47] = ascii_char;

		// Append AUX to debug buffers for additional debugging
		snprintf(LCD_buffer_bitmaps + strlen(LCD_buffer_bitmaps),
			sizeof(LCD_buffer_bitmaps) - strlen(LCD_buffer_bitmaps),
			"%d : [%02X, %02X, %02X, %02X, %02X, %02X, %02X]\n",
			i, bitmap[0], bitmap[1], bitmap[2], bitmap[3],
			bitmap[4], bitmap[5], bitmap[6]);
	}

	// Null-terminate the Aux display debug string
	main_display_debug[LINE2_LEN] = '\0';
}


//************************************************************************************************************************************************************
//************************************************************************************************************************************************************

// Main
int main(void) {

	// Reset of all peripherals, Initializes the Flash interface and the Systick.
	HAL_Init();

	// Configure the system clock
	SystemClock_Config();

	// Initialize all configured peripherals (except bit-bang SPI for S7701S LCD glass)
	MX_GPIO_Init();					// I/O pins
	MX_DMA_Init();					// DMA1 Ch.2 & Ch.4
	MX_SPI1_Init();					// SPI1 - LT760A-R
	MX_SPI2_Init();					// SPI2 - VFD
	TIM2_Init();					// Initialize the timer

	// Pull CS high and SCLK low immediately after reset
	HAL_GPIO_WritePin(LCD_CS_Port, LCD_CS_Pin, GPIO_PIN_SET);			// Pull CS high
	HAL_GPIO_WritePin(LCD_SCK_Port, LCD_SCK_Pin, GPIO_PIN_RESET);		// CLK pin low

	// Read pin B0 - Set colours for MAIN & AUX
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET) {
		// B0 high
		MainColourFore = 0xFFFFFF;		// WHite
		AuxColourFore = 0xFFFF00;		// Yellow
	}
	else {
		// B0 low
		MainColourFore = 0xFFFF00;		// Yellow
		AuxColourFore = 0xFFFFFF;		// White
	}

	// Read pin B1 - Set colour for ANNUNCIATORS
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) {
		// B1 high
		AnnunColourFore = 0x00FF00;		// Green
	}
	else {
		// B1 low
		AnnunColourFore = 0x00FFFF;		// Cyan
	}

	if (oneVoltmode) {
		HAL_GPIO_TogglePin(GPIOC, TEST_OUT_Pin); // Test LED toggle
	}
		
	HardwareReset();				// Reset LT7680 - Pull LCM_RESET low for 100ms and wait

	HAL_Delay(1000);
	
	SendAllToLT7680_LT();			// run subs to setup LT7680 based on Levetop info

	HAL_Delay(10);

	// Main loop timer
	SetTimerDuration(35);			// 35 ms timed action set

	HAL_Delay(5);
	ConfigurePWMAndSetBrightness(BACKLIGHTFULL);  // Configure Timer-1 and PWM-1 for backlighting. Settable 0-100%

	ClearScreen();					// Again.....

	// Read pins A11/A12 - Enter timing changes on boot if DCV button held in during power up
	GPIO_PinState pinA11 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
	if (pinA11 == GPIO_PIN_RESET) {
		timingModsOnBoot = true;
	} else {
		timingModsOnBoot = false;
	}


	// test eeprom (flash)
	//EEPROM_ErasePage(EEPROM_START_ADDRESS) == HAL_OK;
	//EEPROM_WriteData(EEPROM_START_ADDRESS + 28, 123);
	//TEST1 = EEPROM_ReadData(EEPROM_START_ADDRESS + 28);
	// 
	//EEPROM_ErasePage(EEPROM_START_ADDRESS) == HAL_OK;
	//EEPROM_Write4CharString(EEPROM_START_ADDRESS + 28, "AdaF");

	// Load settings from EEProm (Flash)
	setting_LCD_VBPD = EEPROM_ReadData(EEPROM_START_ADDRESS);
	setting_LCD_VFPD = EEPROM_ReadData(EEPROM_START_ADDRESS + 4);
	setting_LCD_VSPW = EEPROM_ReadData(EEPROM_START_ADDRESS + 8);
	setting_LCD_HBPD = EEPROM_ReadData(EEPROM_START_ADDRESS + 12);
	setting_LCD_HFPD = EEPROM_ReadData(EEPROM_START_ADDRESS + 16);
	setting_LCD_HSPW = EEPROM_ReadData(EEPROM_START_ADDRESS + 20);
	setting_REFRESH_RATE = EEPROM_ReadData(EEPROM_START_ADDRESS + 24);
	EEPROM_Read4CharString(EEPROM_START_ADDRESS + 28, setting_ADA_BUY);


	// Copy retrieved vars from Flash for showing on splash screen
	boot_LCD_VBPD = setting_LCD_VBPD;
	boot_LCD_VFPD = setting_LCD_VFPD;
	boot_LCD_VSPW = setting_LCD_VSPW;
	boot_LCD_HBPD = setting_LCD_HBPD;
	boot_LCD_HFPD = setting_LCD_HFPD;
	boot_LCD_HSPW = setting_LCD_HSPW;
	boot_REFRESH_RATE = setting_REFRESH_RATE;
	strcpy(boot_ADA_BUY, setting_ADA_BUY);

	// Check if loaded values are ok
	if (setting_LCD_VBPD < 5 || setting_LCD_VBPD > 50) {

		// Assign default values
		setting_LCD_VBPD = LCD_VBPD;
		setting_LCD_VFPD = LCD_VFPD;
		setting_LCD_VSPW = LCD_VSPW;
		setting_LCD_HBPD = LCD_HBPD;
		setting_LCD_HFPD = LCD_HFPD;
		setting_LCD_HSPW = LCD_HSPW;
		setting_REFRESH_RATE = REFRESH_RATE;
		strcpy(setting_ADA_BUY, ADA_BUY);

		EEPROM_ErasePage(EEPROM_START_ADDRESS) == HAL_OK;
	
		EEPROM_WriteData(EEPROM_START_ADDRESS, setting_LCD_VBPD);
		EEPROM_WriteData(EEPROM_START_ADDRESS + 4, setting_LCD_VFPD);
		EEPROM_WriteData(EEPROM_START_ADDRESS + 8, setting_LCD_VSPW);
		EEPROM_WriteData(EEPROM_START_ADDRESS + 12, setting_LCD_HBPD);
		EEPROM_WriteData(EEPROM_START_ADDRESS + 16, setting_LCD_HFPD);
		EEPROM_WriteData(EEPROM_START_ADDRESS + 20, setting_LCD_HSPW);
		EEPROM_WriteData(EEPROM_START_ADDRESS + 24, setting_REFRESH_RATE);
		EEPROM_Write4CharString(EEPROM_START_ADDRESS + 28, setting_ADA_BUY);
	
	}

	// Populate the final vars to be used
	LCD_VBPD = setting_LCD_VBPD;
	LCD_VFPD = setting_LCD_VFPD;
	LCD_VSPW = setting_LCD_VSPW;
	LCD_HBPD = setting_LCD_HBPD;
	LCD_HFPD = setting_LCD_HFPD;
	LCD_HSPW = setting_LCD_HSPW;
	REFRESH_RATE = setting_REFRESH_RATE;
	strcpy(ADA_BUY, setting_ADA_BUY);

	// ST7701S critical setting
	if (strcmp(ADA_BUY, "AdaF") == 0) {
		AdaFruit_Init(); // Initialize AdaFruit driver
	}
	else if (strcmp(ADA_BUY, "BuyD") == 0) {
		BuyDisplay_Init(); // Initialize BuyDisplay driver
	}
	else {
		strcpy(ADA_BUY, "AdaF");
		AdaFruit_Init(); // Default - Initialize AdaFruit driver
	}



//**************************************************************************************************
// Main loop initialize

	Init_Completed_flag = 1; // Now is a safe time to enable the EXTI interrupt handler

	while (1) {

		// demo float (confirmation of Soft FP)
		//char inputString[] = "123.456";
		//test15 = atof(inputString);

		Packets_to_chars();         // Convert packets from R6581 to characters
		Main_Aux_R6581();           // Get R6581 VFD drive data

		task_ready = 1; // Mark tasks as complete so the timer driven code is allowed to run again

		//*******************************************************************************************
		// Timed Action - Check if timer flag is set and tasks are ready and run the LCD sub
		if (timer_flag && task_ready) {
			timer_flag = 0;   // Clear the timer flag
			task_ready = 0;   // Reset task-ready flag    
			
			if (timingModsOnBoot == false) {

				HAL_GPIO_TogglePin(GPIOC, TEST_OUT_Pin); // Test LED toggle

				DisplaySplash();

				HAL_Delay(6); // Allow the LT7680 sufficient processing time

				DisplayMain();

				HAL_Delay(6); // Allow the LT7680 sufficient processing time

				DisplayAux();

				HAL_Delay(6); // Allow the LT7680 sufficient processing time

				DisplayAnnunciators();

				HAL_Delay(6); // Allow the LT7680 sufficient processing time

				// Right wipe
				DrawLine(0, 959, 399, 959, 0x00, 0x00, 0x00);	// far right hand vertical line, black, 1 pixel line. (this line hidden!)
				DrawLine(0, 958, 399, 958, 0x00, 0x00, 0x00);	// (this line hidden!)
				DrawLine(0, 957, 399, 957, 0x00, 0x00, 0x00);
				DrawLine(0, 956, 399, 956, 0x00, 0x00, 0x00);
				DrawLine(0, 955, 399, 955, 0x00, 0x00, 0x00);
				DrawLine(0, 954, 399, 954, 0x00, 0x00, 0x00);
				DrawLine(0, 953, 399, 953, 0x00, 0x00, 0x00);
				DrawLine(0, 952, 399, 952, 0x00, 0x00, 0x00);

				// Test only - 400pixel based test lines for viewing the centre line and the left, middle and far right positions.
				// The internal memory is set up as 400x960 but the leftmost 80 pixels are considered overscan and don't show up, thus 320
				//DrawLine(0, 0, 399, 0, 0xFF, 0xFF, 0xFF);		// far left hand vertical line, black, 1 pixel line. 938 not 960 seems to be far right edge!
				//DrawLine(0, 480, 399, 480, 0xFF, 0xFF, 0xFF);	// mid-way
				//DrawLine(0, 959, 399, 959, 0xFF, 0xFF, 0xFF);	// far right
				//DrawLine(199, 0, 199, 959, 0xFF, 0x00, 0x00);	// centred on R6581T horizontally

				HAL_Delay(6); // Allow the LT7680 sufficient processing time

				// Read pins A11/A12 - Front panel DCV switch momentary - Enable 1VDC mode
				GPIO_PinState pinA11 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
				GPIO_PinState pinA12 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12);

				// 1000mVdc / 1Vdc mode select
				if (pinA11 == GPIO_PIN_SET && pinA12 == GPIO_PIN_RESET) {
					// Button is NOT pressed (normal state)
					oneVoltmodepreviousState = false;
				}
				else if (pinA11 == GPIO_PIN_RESET && pinA12 == GPIO_PIN_RESET && pinA11 == pinA12) {
					// Button is pressed (both pins are the same, and LOW)
					if (!oneVoltmodepreviousState) {
						// Toggle the mode on the first detection of the press
						oneVoltmode = !oneVoltmode;
					}
					// Update the previous state
					oneVoltmodepreviousState = true;
				}

			} else {

				// Timing mode adjust, toggle round TFT LCD timings using DCV button
				
				// Read pins A11/A12 - Front panel DCV switch momentary
				GPIO_PinState pinA11 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
				GPIO_PinState pinA12 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12);

				// DCV button pressed
				if (pinA11 == GPIO_PIN_SET && pinA12 == GPIO_PIN_RESET) {
					// Button is NOT pressed (normal state)
					timingModspreviousstate = false;
				} else if (pinA11 == GPIO_PIN_RESET && pinA12 == GPIO_PIN_RESET && pinA11 == pinA12) {
					// Button is pressed (both pins are the same, and LOW)
					if (!timingModspreviousstate) {
						// Toggle the mode on the first detection of the press
						timingModsOnBootDCV = !timingModsOnBootDCV;

						// On each press cycle round the various settings
						// Rotate through the timing settings
						currentTimingSet = (currentTimingSet + 1) % currentTimingSetNumberentries;  // Cycle through 0 to n
						// Update the user settings based on the current set

						if (isFirstPress == false) {
							setting_LCD_VBPD = LCD_VBPD_SETTINGS[currentTimingSet];
							setting_LCD_VFPD = LCD_VFPD_SETTINGS[currentTimingSet];
							setting_LCD_VSPW = LCD_VSPW_SETTINGS[currentTimingSet];
							setting_LCD_HBPD = LCD_HBPD_SETTINGS[currentTimingSet];
							setting_LCD_HFPD = LCD_HFPD_SETTINGS[currentTimingSet];
							setting_LCD_HSPW = LCD_HSPW_SETTINGS[currentTimingSet];
							setting_REFRESH_RATE = REFRESH_RATE_SETTINGS[currentTimingSet];
							strcpy(setting_ADA_BUY, ADA_BUY_SETTINGS[currentTimingSet]);

							LCD_VBPD = setting_LCD_VBPD;
							LCD_VFPD = setting_LCD_VFPD;
							LCD_VSPW = setting_LCD_VSPW;
							LCD_HBPD = setting_LCD_HBPD;
							LCD_HFPD = setting_LCD_HFPD;
							LCD_HSPW = setting_LCD_HSPW;
							REFRESH_RATE = setting_REFRESH_RATE;
							strcpy(ADA_BUY, setting_ADA_BUY);

							// run startup subs again to apply new settings
							AdaFruit_Init();
							HAL_Delay(5);
							LT7680_PLL_Initial_LT();
							HAL_Delay(5);
							LCD_Horizontal_Non_Display_LT(LCD_HBPD);  // Horizontal Back Porch
							HAL_Delay(5);
							LCD_HSYNC_Start_Position_LT(LCD_HFPD);    // HSYNC Start Position
							HAL_Delay(5);
							LCD_HSYNC_Pulse_Width_LT(LCD_HSPW);       // HSYNC Pulse Width
							HAL_Delay(5);
							LCD_Vertical_Non_Display_LT(LCD_VBPD);    // Vertical Back Porch
							HAL_Delay(5);
							LCD_VSYNC_Start_Position_LT(LCD_VFPD);    // VSYNC Start Position
							HAL_Delay(5);
							LCD_VSYNC_Pulse_Width_LT(LCD_VSPW);       // VSYNC Pulse Width
							HAL_Delay(5);

							EEPROM_ErasePage(EEPROM_START_ADDRESS) == HAL_OK;		// Erase Flash

							// Save the updated settings to flash
							EEPROM_WriteData(EEPROM_START_ADDRESS, setting_LCD_VBPD);
							EEPROM_WriteData(EEPROM_START_ADDRESS + 4, setting_LCD_VFPD);
							EEPROM_WriteData(EEPROM_START_ADDRESS + 8, setting_LCD_VSPW);
							EEPROM_WriteData(EEPROM_START_ADDRESS + 12, setting_LCD_HBPD);
							EEPROM_WriteData(EEPROM_START_ADDRESS + 16, setting_LCD_HFPD);
							EEPROM_WriteData(EEPROM_START_ADDRESS + 20, setting_LCD_HSPW);
							EEPROM_WriteData(EEPROM_START_ADDRESS + 24, setting_REFRESH_RATE);
							EEPROM_Write4CharString(EEPROM_START_ADDRESS + 28, setting_ADA_BUY);
						}

						HAL_Delay(6);

						SetTextColors(0x00FF00, 0x000000); // Foreground: green, Background: Black
						ConfigureFontAndPosition(
							0b00,    // Internal CGROM
							0b10,    // Font size
							0b00,    // ISO 8859-1
							0,       // Full alignment enabled
							0,       // Chroma keying disabled
							1,       // Rotate 90 degrees counterclockwise
							0b00,    // Width multiplier
							0b00,    // Height multiplier
							1,       // Line spacing
							4,       // Character spacing
							140,     // Cursor X
							0      // Cursor Y
						);
						char text1[] = "TFT LCD Timing Adjust";
						DrawText(text1);

						HAL_Delay(6);

						SetTextColors(0xFFFFFF, 0x000000); // Foreground: green, Background: Black
						ConfigureFontAndPosition(
							0b00,    // Internal CGROM
							0b01,    // Font size
							0b00,    // ISO 8859-1
							0,       // Full alignment enabled
							0,       // Chroma keying disabled
							1,       // Rotate 90 degrees counterclockwise
							0b00,    // Width multiplier
							0b00,    // Height multiplier
							1,       // Line spacing
							4,       // Character spacing
							170,     // Cursor X
							0      // Cursor Y
						);
						char text2[] = "Hit DCV to cycle round new TFT LCD settings";
						DrawText(text2);

						HAL_Delay(6);

						ConfigureFontAndPosition(
							0b00,    // Internal CGROM
							0b01,    // Font size
							0b00,    // ISO 8859-1
							0,       // Full alignment enabled
							0,       // Chroma keying disabled
							1,       // Rotate 90 degrees counterclockwise
							0b00,    // Width multiplier
							0b00,    // Height multiplier
							1,       // Line spacing
							4,       // Character spacing
							195,     // Cursor X
							0      // Cursor Y
						);
						char text3[] = "Power cycle may be necessary to achieve full effect";
						DrawText(text3);

						HAL_Delay(6);

						ConfigureFontAndPosition(
							0b00,    // Internal CGROM
							0b01,    // Font size
							0b00,    // ISO 8859-1
							0,       // Full alignment enabled
							0,       // Chroma keying disabled
							1,       // Rotate 90 degrees counterclockwise
							0b00,    // Width multiplier
							0b00,    // Height multiplier
							1,       // Line spacing
							4,       // Character spacing
							250,     // Cursor X
							0       // Cursor Y
						);
						char text4[] = "        VBPD VFPD VSPW HBPD HFPD HSPW REFR COG";
						DrawText(text4);

						HAL_Delay(6);

						SetTextColors(0xFFFF00, 0x000000); // Foreground: Yellow, Background: Black
						ConfigureFontAndPosition(
							0b00,    // Internal CGROM
							0b01,    // Font size
							0b00,    // ISO 8859-1
							0,       // Full alignment enabled
							0,       // Chroma keying disabled
							1,       // Rotate 90 degrees counterclockwise
							0b00,    // Width multiplier
							0b00,    // Height multiplier
							1,       // Line spacing
							4,       // Character spacing
							275,     // Cursor X
							0      // Cursor Y
						);
						char redefineValuesCurr[128]; // Ensure the buffer is large enough
						snprintf(redefineValuesCurr, sizeof(redefineValuesCurr),
							"CURRENT %d   %d   %d    %d   %d   %d   %d   %s",
							boot_LCD_VBPD,
							boot_LCD_VFPD,
							boot_LCD_VSPW,
							boot_LCD_HBPD,
							boot_LCD_HFPD,
							boot_LCD_HSPW,
							boot_REFRESH_RATE,
							boot_ADA_BUY
						);
						DrawText(redefineValuesCurr);

						HAL_Delay(6);

						if (isFirstPress == false) {
							SetTextColors(0x00FF00, 0x000000); // Foreground: Yellow, Background: Black
							ConfigureFontAndPosition(
								0b00,    // Internal CGROM
								0b01,    // Font size
								0b00,    // ISO 8859-1
								0,       // Full alignment enabled
								0,       // Chroma keying disabled
								1,       // Rotate 90 degrees counterclockwise
								0b00,    // Width multiplier
								0b00,    // Height multiplier
								1,       // Line spacing
								4,       // Character spacing
								300,     // Cursor X
								0      // Cursor Y
							);
							char redefineValues[128]; // Ensure the buffer is large enough
							snprintf(redefineValues, sizeof(redefineValues),
								"NEW     %d   %d   %d    %d   %d   %d   %d   %s",
								setting_LCD_VBPD,
								setting_LCD_VFPD,
								setting_LCD_VSPW,
								setting_LCD_HBPD,
								setting_LCD_HFPD,
								setting_LCD_HSPW,
								setting_REFRESH_RATE,
								setting_ADA_BUY
							);
							DrawText(redefineValues);
						}

						// Delay for button
						HAL_Delay(120);

					}

					timingModspreviousstate = true;		// Update the previous state
					isFirstPress = false;				// Reset the flag after the first press
				}

			}
			

		}
	}

}



// Write uint32_t to EEprom (Flash)
HAL_StatusTypeDef EEPROM_WriteData(uint32_t address, uint32_t data) {
	if (address < EEPROM_START_ADDRESS || address >= (EEPROM_START_ADDRESS + EEPROM_PAGE_SIZE)) {
		return HAL_ERROR; // Address out of range
	}
	HAL_FLASH_Unlock();
	HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
	HAL_FLASH_Lock();
	return status;
}


// Write CHAR string to EEprom (Flash), 4 chars max
HAL_StatusTypeDef EEPROM_Write4CharString(uint32_t address, const char* str) {
	if (strlen(str) > 4) {
		return HAL_ERROR; // Limit the string length to 4 characters
	}

	uint32_t data = 0;

	// Copy up to 4 characters into a 32-bit word
	memcpy(&data, str, 4);

	HAL_FLASH_Unlock();
	HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
	HAL_FLASH_Lock();

	return status;
}


// Read uint32_t from EEProm (Flash)
uint32_t EEPROM_ReadData(uint32_t address) {
	if (address < EEPROM_START_ADDRESS || address >= (EEPROM_START_ADDRESS + EEPROM_PAGE_SIZE)) {
		return 0xFFFFFFFF; // Invalid address
	}
	return *(uint32_t*)address;
}


// Read CHAR string from EEprom (Flash), 4 chars max
void EEPROM_Read4CharString(uint32_t address, char* buffer) {
	uint32_t data = *(uint32_t*)address; // Read the 32-bit word

	// Copy the data into the buffer
	memcpy(buffer, &data, 4);

	// Ensure the string is null-terminated
	buffer[4] = '\0';
}



// Erase EEprom (Flash) - necessary before write
HAL_StatusTypeDef EEPROM_ErasePage(uint32_t address) {
	if (address < EEPROM_START_ADDRESS || address >= (EEPROM_START_ADDRESS + EEPROM_PAGE_SIZE)) {
		return HAL_ERROR; // Address out of range
	}

	HAL_FLASH_Unlock();

	FLASH_EraseInitTypeDef eraseInit;
	uint32_t pageError;

	eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit.PageAddress = address;
	eraseInit.NbPages = 1; // Erase a single page

	HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&eraseInit, &pageError);

	HAL_FLASH_Lock();
	return status;
}


// System Clock Configuration
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	//Initializes the RCC Oscillators according to the specified parameters in the RCC_OscInitTypeDef structure.
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	// Initializes the CPU, AHB and APB buses clocks
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
		| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
}


// This function is executed in case of error occurrence.
void Error_Handler(void) {
	// User can add his own implementation to report the HAL error return state
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	   /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
