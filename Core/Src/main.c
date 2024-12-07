﻿/*
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
  * For use with LT7680A-R & 320x960 TFT LCD
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

// Test only
//volatile int myVariable1 = 0; // Prevent optimization
//volatile int myVariable2 = 0; // Prevent optimization
//volatile int STSR = 0; // Prevent optimization

/* Variables ---------------------------------------------------------*/
static char main_display_debug[LINE1_LEN + 1]; // Main display debug string
#define FONT_HEIGHT 7
uint16_t dollarPosition = 0;

// Buffers for each LCD graphical item
char LCD_buffer_packets[128];  // For packet data
char LCD_buffer_bitmaps[128];  // For decoded bitmap data
char LCD_buffer_chars[128];    // For decoded characters

char G[48];  // MAIN: G1 to G18, AUX: G19 to G47
_Bool Annunc[19]; // Annunciators re-ordered, 18off, G1 to G18 in left-to-right order
_Bool AnnuncTemp[37]; // Temp array for annunciators. 18off, the order on LCD left to right = 8,7,6,5,4,3,2,1,18,17,16,15,14,13,12,11,10,9

#define White          0xFFFF
#define Black          0x0000
#define Grey           0xF7DE
#define Blue           0x001F
#define Blue2          0x051F
#define Red            0xF800
#define Magenta        0xF81F
#define Green          0x07E0
#define Cyan           0x7FFF
#define Yellow         0xFFE0

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
	{{0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13}, '%'},  // 0x25, %
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
	{{0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x02}, 'J'},  // 0x4A, J
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
	{{0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x0E}, 'b'},  // 0x62, b
	{{0x00, 0x00, 0x0F, 0x10, 0x10, 0x10, 0x0F}, 'c'},  // 0x63, c
	{{0x01, 0x01, 0x01, 0x0F, 0x11, 0x11, 0x0F}, 'd'},  // 0x64, d
	{{0x00, 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0F}, 'e'},  // 0x65, e
	{{0x06, 0x09, 0x08, 0x1C, 0x08, 0x08, 0x08}, 'f'},  // 0x66, f
	{{0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x1F}, 'g'},  // 0x67, g
	{{0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x11}, 'h'},  // 0x68, h
	{{0x00, 0x04, 0x00, 0x04, 0x04, 0x04, 0x04}, 'i'},  // 0x69, i
	{{0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0C}, 'j'},  // 0x6A, j
	{{0x08, 0x08, 0x09, 0x0A, 0x0C, 0x0A, 0x09}, 'k'},  // 0x6B, k
	{{0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C}, 'l'},  // 0x6C, l
	{{0x00, 0x00, 0x1A, 0x15, 0x15, 0x15, 0x11}, 'm'},  // 0x6D, m
	{{0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11}, 'n'},  // 0x6E, n
	{{0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E}, 'o'},  // 0x6F, o
	{{0x00, 0x00, 0x1E, 0x11, 0x1E, 0x10, 0x10}, 'p'},  // 0x70, p
	{{0x00, 0x00, 0x0D, 0x13, 0x0F, 0x01, 0x01}, 'q'},  // 0x71, q
	{{0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10}, 'r'},  // 0x72, r
	{{0x00, 0x00, 0x0E, 0x10, 0x0E, 0x01, 0x1E}, 's'},  // 0x73, s
	{{0x04, 0x04, 0x1F, 0x04, 0x04, 0x05, 0x02}, 't'},  // 0x74, t
	{{0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0D}, 'u'},  // 0x75, u
	{{0x00, 0x00, 0x11, 0x11, 0x11, 0x0A, 0x04}, 'v'},  // 0x76, v
	{{0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A}, 'w'},  // 0x77, w
	{{0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11}, 'x'},  // 0x78, x
	{{0x00, 0x00, 0x11, 0x11, 0x0F, 0x01, 0x1E}, 'y'},  // 0x79, y
	{{0x00, 0x00, 0x1F, 0x02, 0x04, 0x08, 0x1F}, 'z'},  // 0x7A, z
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, '{'},  // 0x7B, {
	{{0x01, 0x02, 0x04, 0x00, 0x04, 0x02, 0x01}, '|'},  // 0x7C, |
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, '}'},  // 0x7D, }
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, '~'},  // 0x7E, ~
	{{0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00}, '/'},  // forward slash
	{{0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00}, '°'},	// DegC symbol
	{{0x0E, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x1B}, '$'},	// Ohm symbol not supported by LT7680A-R, will need to use UCG
	{{0x11, 0x12, 0x14, 0x0B, 0x11, 0x02, 0x03}, '½'},	// half symbol
	{{0x00, 0x00, 0x04, 0x0E, 0x1F, 0x00, 0x00}, '\x1E'},	// up arrow
	{{0x00, 0x00, 0x1F, 0x0E, 0x04, 0x00, 0x00}, '\x1F'},	// down arrow
	// Needs E0 to E8 extended chars added for DIAG screen for vertical and horizontal fill.
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

		// Append MAIN to debug buffers for additional debugging
		//snprintf(LCD_buffer_bitmaps + strlen(LCD_buffer_bitmaps),
		//	sizeof(LCD_buffer_bitmaps) - strlen(LCD_buffer_bitmaps),
		//	"%d : [%02X, %02X, %02X, %02X, %02X, %02X, %02X]\n",
		//	i, bitmap[0], bitmap[1], bitmap[2], bitmap[3],
		//	bitmap[4], bitmap[5], bitmap[6]);

		// Append annunciator states to debug string
		//snprintf(annunciator_debug + strlen(annunciator_debug), 
		//     sizeof(annunciator_debug) - strlen(annunciator_debug),
		//     "G%d=%s ", i + 1, Annunciators[i] ? "ON" : "OFF");
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

// Timed Action - Write to the TFT LCD - Every XXms
void LT7680TFTLCD(void) {

	// Blue Pill LCD update rate:
	// Resistance mode = 14Hz
	// All other modes = 20.4Hz

	HAL_GPIO_TogglePin(GPIOC, TEST_OUT_Pin); // Test LED toggle

	DisplaySplash();

	HAL_Delay(6); // Allow the LT7680 sufficient processing time

	DisplayMain();

	HAL_Delay(6); // Allow the LT7680 sufficient processing time

	DisplayAuxFirstHalf();

	HAL_Delay(6); // Allow the LT7680 sufficient processing time

	DisplayAuxSecondHalf();

	HAL_Delay(6); // Allow the LT7680 sufficient processing time

	DisplayAnnunciators();

	HAL_Delay(6); // Allow the LT7680 sufficient processing time

}

//************************************************************************************************************************************************************
//************************************************************************************************************************************************************

// Main
int main(void) {

	// Pull CS high and SCLK low immediately after reset
	HAL_GPIO_WritePin(LCD_CS_Port, LCD_CS_Pin, GPIO_PIN_SET);			// Pull CS high
	HAL_GPIO_WritePin(LCD_SCK_Port, LCD_SCK_Pin, GPIO_PIN_RESET);		// CLK pin low

	// Reset of all peripherals, Initializes the Flash interface and the Systick.
	HAL_Init();

	// Configure the system clock
	SystemClock_Config();

	// Initialize all configured peripherals
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_SPI1_Init();
	MX_SPI2_Init();

	TIM2_Init();					// Initialize the timer
		
	HardwareReset();				// Reset LT7680 - Pull LCM_RESET low for 100ms and wait

	HAL_Delay(1000);
	
	BuyDisplay_Init();				// Initialize ST7701S BuyDisplay 4.58" driver IC

	HAL_Delay(100);

	SendAllToLT7680_LT();			// run subs to setup LT7680 based on Levetop info

	HAL_Delay(10);

	// Main loop timer
	SetTimerDuration(35);			// 20 ms (100Hz) timed action for LT7680TFTLCD()

	LCDConfigTurnOn_LT();           // Turn on the LCD (at last minute)

//**************************************************************************************************
// Main loop initialize

	Init_Completed_flag = 1; // Now is a safe time to enable the EXTI interrupt handler

	while (1) {

		Packets_to_chars();         // Convert packets from R6581 to characters
		Main_Aux_R6581();           // Get R6581 VFD drive data

		task_ready = 1; // Mark tasks as complete so the timer driven code is allowed to run again

		//myVariable1++; // Modify the value to observe changes

		// Check if timer flag is set and tasks are ready and run the LCD sub
		if (timer_flag && task_ready) {
			timer_flag = 0;   // Clear the timer flag
			task_ready = 0;   // Reset task-ready flag    
			LT7680TFTLCD();   // Run the new subroutine at the designated interval
		}
	}

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