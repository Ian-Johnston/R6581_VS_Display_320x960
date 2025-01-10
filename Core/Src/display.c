/**
  ******************************************************************************
  * @file    display.c
  * @brief   This file provides code for the
  *          display MAIN, AUX, ANNUNCIATORS & SPLASH.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include "main.h"
#include "lcd.h"
#include "lt7680.h"
#include "display.h"
#include <string.h>  // For strchr, strncpy
#include <stdio.h>   // For debugging (optional)
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>


#define DURATION_MS 5000     // 5 seconds in milliseconds
#define TIMER_INTERVAL_MS 35 // The interval of your timed sub in milliseconds

// Display colours default
uint32_t MainColourFore = 0xFFFF00; // Yellow
uint32_t AuxColourFore = 0xFFFFFF; // White
uint32_t AnnunColourFore = 0x00FF00; // Green

_Bool onethousandmVmodedetected;
char MaindisplayString[19] = "";              // String for G[1] to G[18]
_Bool displayBlank = false;
_Bool displayBlankPrevious = false;


//uint16_t dollarPositionAUX = 0xFFFF;

//************************************************************************************************************************************************************


void DisplayMain() {

	// This sub needs re-written in the same way the AUX line is now written - it's on the todo list.

	// MAIN ROW - Print text to LCD, detect if there is an OHM symbol ($) and if so split into 3 parts, before-OHM-after
	SetTextColors(MainColourFore, 0x000000); // Foreground, Background

	//char MaindisplayString[19] = "";              // String for G[1] to G[18]
	char BeforeDollar[19] = "";                   // To store characters before the $
	char AfterDollar[19] = "";                    // To store characters after the $
	uint16_t dollarPosition = 0xFFFF;            // Initialize to an invalid position

	// Populate MaindisplayString from G[1] to G[18]
	for (int i = 1; i <= 18; i++) {
		MaindisplayString[i - 1] = G[i];
	}
	MaindisplayString[18] = '\0';                 // Null-terminate MaindisplayString

	// Find the position of the '$' symbol
	for (int i = 0; i < 18; i++) {
		if (MaindisplayString[i] == '$') {
			dollarPosition = i;                   // Record the position of '$'
			break;                                // Exit loop once found
		}
	}

	// If in 2W or 4W Resistance measurement mode the display will contain the OHM symbol on the MAIN display.
	// If it appears then the dollarPositionAUX var will not be 0xFFFF
	if (dollarPosition != 0xFFFF) {

		// $ symbol found
		// Before
		ConfigureFontAndPosition(
			0b00,    // Internal CGROM
			0b10,    // Font size
			0b00,    // ISO 8859-1
			0,       // Full alignment enabled
			0,       // Chroma keying disabled
			1,       // Rotate 90 degrees counterclockwise
			0b10,    // Width multiplier
			0b10,    // Height multiplier
			1,       // Line spacing
			4,       // Character spacing
			Xpos_MAIN,      // Cursor X
			0        // Cursor Y
		);
		char MaindisplayStringBefore[19] = "";
		for (int i = 1; i <= 18; i++) {
			if (G[i] == '$') break;
			MaindisplayStringBefore[i - 1] = G[i];
		}
		DrawText(MaindisplayStringBefore);

		HAL_Delay(5);
		ResetGraphicWritePosition_LT();
		Ohms16x32SymbolStoreUCG();			// This is called here rather than pre-defined because the 2nd UCG below is a different size
		Text_Mode();

		// Ohm Symbol
		// Calculate position of $ symbol
		//       yposohm = (dollarPosition * widthmultiplier * fontwidth) + (characterspacing * dollarPosition) + startoffset(6)
		uint16_t yposohm = (dollarPosition * 3 * 16) + (4 * dollarPosition);  // Y position of the OHM symbol
		// The actual OHM
		ConfigureFontAndPosition(
			0b10,    // User-Defined Font mode
			0b10,    // Font size
			0b00,    // ISO 8859-1
			0,       // Full alignment enabled
			0,       // Chroma keying disabled
			1,       // Rotate 90 degrees counterclockwise
			0b10,    // Width multiplier
			0b10,    // Height multiplier
			1,       // Line spacing
			4,       // Character spacing
			Xpos_MAIN,      // Cursor X 170
			yposohm  // Cursor Y 480	780
		);
		// Write the OHM symbol
		WriteRegister(0x04);
		WriteData(0x00);    // high byte
		WriteData(0x00);    // low byte

		HAL_Delay(2);

		// After
		ConfigureFontAndPosition(
			0b00,    // Internal CGROM
			0b10,    // Font size
			0b00,    // ISO 8859-1
			0,       // Full alignment enabled
			0,       // Chroma keying disabled
			1,       // Rotate 90 degrees counterclockwise
			0b10,    // Width multiplier
			0b10,    // Height multiplier
			1,       // Line spacing
			4,       // Character spacing
			Xpos_MAIN,      // Cursor X
			(dollarPosition * 52) + 52        // Cursor Y	832
		);
		char MaindisplayStringAfter[19] = ""; // Adjust the size to match your max expected characters
		int j = 0; // Index for the new string
		for (int i = dollarPosition + 1; i <= 18; i++) { // Start after the $ and loop through the rest
			MaindisplayStringAfter[j++] = MaindisplayString[i]; // Copy characters to the new string
		}
		MaindisplayStringAfter[18] = '\0'; // Null-terminate the new string
		DrawText(MaindisplayStringAfter);

	} else {

		// Every mode other than one that includes an OHM symbol
		// and also if OVERLOAD is not being displayed
		// and also that there are numbers being displayed, because when changing ranges manually the display can be blanked (no numbers)

		if (oneVoltmode && onethousandmVmodedetected && (strstr(MaindisplayString, "OVERLOAD") == NULL) && (strpbrk(MaindisplayString, "0123456789") != NULL)) {

			// Standard R6581 display non-OHM mode, user has selected 1VDC rather than 1000mV mode by pressing DCV button whilst on 1000mV mode

			// Take MaindisplayString and replace numerical part, i.e.:
			// "+ 999.99709   mVDC"
			// change to
			// "+ 0.99999709   VDC"

			char MaindisplayString[19] = "";              // String for G[1] to G[18]
			for (int i = 1; i <= 18; i++) {
				MaindisplayString[i - 1] = G[i];          // Copy characters
			}

			char prefix[19] = { 0 };       // To store the string before the numeric part
			char numericPart[13] = { 0 };  // To store the numeric part
			char suffix[] = "VDC";         // Fixed suffix
			char result[13] = { 0 };       // To store the transformed numeric part

			// Extract the prefix (including the sign) before the numeric part
			sscanf(MaindisplayString, "%[^0-9]", prefix);

			// Extract the numeric part (ignoring the sign)
			sscanf(MaindisplayString, "%*[^0-9]%12s", numericPart);

			// Find the position of the decimal point
			char* dot = strchr(numericPart, '.');
			int integerLength = dot ? (dot - numericPart) : strlen(numericPart); // Length of the integer part

			// Rebuild the result by dividing the value by 1000
			int resultIndex = 0;
			if (integerLength > 3) {
				// Copy digits before the new decimal point
				for (int i = 0; i < integerLength - 3; i++) {
					result[resultIndex++] = numericPart[i];
				}
				result[resultIndex++] = '.'; // Add the decimal point

				// Copy the remaining digits from the original integer part
				for (int i = integerLength - 3; i < integerLength; i++) {
					result[resultIndex++] = numericPart[i];
				}
			}
			else {
				// For numbers smaller than 1000, prepend "0." and leading zeros
				result[resultIndex++] = '0';
				result[resultIndex++] = '.';
				for (int i = 0; i < 3 - integerLength; i++) {
					result[resultIndex++] = '0';
				}
				// Copy the entire integer part
				for (int i = 0; i < integerLength; i++) {
					result[resultIndex++] = numericPart[i];
				}
			}

			// Copy the fractional part after the decimal point
			if (dot) {
				strcpy(result + resultIndex, dot + 1);
			}

			// Clear and rebuild MaindisplayString
			memset(MaindisplayString, ' ', 18);  // Fill with spaces
			MaindisplayString[18] = '\0';        // Null-terminate the string
			MaindisplayString[0] = prefix[0];    // Set the sign (+ or -) at position 0
			MaindisplayString[1] = ' ';          // Add space after the sign
			strncpy(MaindisplayString + 2, result, strlen(result));            // Copy numericPart starting at position 2
			strncpy(MaindisplayString + 18 - strlen(suffix), suffix, strlen(suffix)); // Add the suffix

			ConfigureFontAndPosition(
				0b00,    // Internal CGROM
				0b10,    // Font size
				0b00,    // ISO 8859-1
				0,       // Full alignment enabled
				0,       // Chroma keying disabled
				1,       // Rotate 90 degrees counterclockwise
				0b10,    // Width multiplier
				0b10,    // Height multiplier
				1,       // Line spacing
				4,       // Character spacing
				Xpos_MAIN,      // Cursor X
				0        // Cursor Y
			);
			DrawText(MaindisplayString);

		} else {

			// Standard R6581 display non-OHM mode, including standard 1000mV mode

			ConfigureFontAndPosition(
				0b00,    // Internal CGROM
				0b10,    // Font size
				0b00,    // ISO 8859-1
				0,       // Full alignment enabled
				0,       // Chroma keying disabled
				1,       // Rotate 90 degrees counterclockwise
				0b10,    // Width multiplier
				0b10,    // Height multiplier
				1,       // Line spacing
				4,       // Character spacing
				Xpos_MAIN,      // Cursor X
				0        // Cursor Y
			);
			char MaindisplayString[19] = "";              // String for G[1] to G[18]
			for (int i = 1; i <= 18; i++) {
				MaindisplayString[i - 1] = G[i];          // Copy characters
			}
			MaindisplayString[18] = '\0';                 // Null-terminate
			DrawText(MaindisplayString);

			// (strstr(MaindisplayString, "DISPLAY OFF") != NULL))
			// LCDConfigTurnOn_LT();
			// LCDConfigTurnOff_LT();
			CheckDisplayStatus();

		}

	}

}


//******************************************************************************


// "DISPLAY OFF" logic
void CheckDisplayStatus() {

	// Check if "DISPLAY OFF" is present in MaindisplayString
	displayBlank = (strstr(MaindisplayString, "DISPLAY OFF") != NULL);

	// If the display status has changed
	if (displayBlank != displayBlankPrevious) {
		if (displayBlank) {
			// Changed to "DISPLAY OFF"
			LCDConfigTurnOff_LT();
		}
		else {
			// Changed from "DISPLAY OFF" to something else, i.e. user has pressed a button to revive
			LCDConfigTurnOn_LT();
		}
	}

	// Update the previous status
	displayBlankPrevious = displayBlank;

}


//******************************************************************************


void DisplayAux() {

	// AUX ROW text to LCD

	SetTextColors(AuxColourFore, 0x000000); // Foreground, Background

	char AuxdisplayString[30] = "";               // String for G[19] to G[47]
	uint16_t dollarPositions[5] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF }; // To store positions of up to 5 $
	int dollarCount = 0; // Count of $ symbols found

	// Customizable fudge factors for each $ symbol position
	//int fudgeFactors[5] = { 23, 23, 32, 104 }; // Adjust these values for each $ symbol's position

	// Populate AuxdisplayString from G[19] to G[47]
	for (int i = 19; i <= 47; i++) {
		AuxdisplayString[i - 19] = G[i];
	}
	AuxdisplayString[29] = '\0'; // Null-terminate at the 30th position because array starts at 0

	// Find positions of up to 5 $ symbols
	for (int i = 0; i < 30 && dollarCount < 5; i++) {
		if (AuxdisplayString[i] == '$') {
			dollarPositions[dollarCount++] = i; // Store position and increment count
		}
	}

	uint16_t yposohm = 0; // Initialize y-position for OHM symbol
	uint16_t yposoffset = 60; // Initialize y-position for OHM symbol

	if (dollarCount != 0) {

		// Process text and OHM symbols based on dollarCount
		for (int d = 0; d <= dollarCount; d++) {
			// Calculate start and end positions for text
			int start = (d == 0) ? 0 : dollarPositions[d - 1] + 1;
			int end = (d < dollarCount) ? dollarPositions[d] : 30;

			// Print text before or between $ symbols
			if (start < end) {
				char AuxdisplaySegment[30] = "";
				for (int i = start; i < end; i++) {
					AuxdisplaySegment[i - start] = AuxdisplayString[i];
				}
				AuxdisplaySegment[end - start] = '\0'; // Null-terminate

				ConfigureFontAndPosition(
					0b00,    // Internal CGROM
					0b01,    // Font size
					0b00,    // ISO 8859-1
					0,       // Full alignment enabled
					0,       // Chroma keying disabled
					1,       // Rotate 90 degrees counterclockwise
					0b01,    // Width multiplier
					0b01,    // Height multiplier
					1,       // Line spacing
					0,       // Character spacing
					Xpos_AUX,     // Cursor X
					yposohm = yposoffset + (start * 12 * 2) // Dynamically adjust position, offset + (position * 12 pixel width character * 2)
				);
				DrawText(AuxdisplaySegment);
			}

			HAL_Delay(5);
			ResetGraphicWritePosition_LT();
			Ohms12x24SymbolStoreUCG(); // This is called here rather than pre-defined because the 1st UCG above is a different size
			Text_Mode();

			// Print OHM symbol if within dollarCount
			if (d < dollarCount) {
				uint16_t calculated_value = (dollarPositions[d] * 12 * 2);		// 12 pixel width character * 2
				yposohm = yposoffset + (uint16_t)calculated_value;

				ConfigureFontAndPosition(
					0b10,    // User-Defined Font mode
					0b01,    // Font size
					0b00,    // ISO 8859-1
					0,       // Full alignment enabled
					0,       // Chroma keying disabled
					1,       // Rotate 90 degrees counterclockwise
					0b01,    // Width multiplier
					0b01,    // Height multiplier
					1,       // Line spacing
					0,       // Character spacing
					Xpos_AUX,     // Cursor X
					yposohm  // Cursor Y
				);
				WriteRegister(0x04);
				WriteData(0x00);    // high byte
				WriteData(0x00);    // low byte
			}
		}

	}

	// If no $ symbols were found, print the entire string as-is	

	if (dollarCount == 0) {

		// Detect 1000mV Range
		if ((strstr(MaindisplayString, "OVERLOAD") == NULL) &&
			(strpbrk(MaindisplayString, "0123456789") != NULL) &&
			(strstr(AuxdisplayString, "1000mV Range") != NULL)) {
			onethousandmVmodedetected = true;
		} else {
			onethousandmVmodedetected = false;
			oneVoltmode = false;
		}

		// If in 1000mV range and user has enabled the new 1VDC mode
		if (oneVoltmode && onethousandmVmodedetected) {
			yposohm = 60;
		}
		else {
			yposohm = 60;
		}

		ConfigureFontAndPosition(
			0b00,    // Internal CGROM
			0b01,    // Font size
			0b00,    // ISO 8859-1
			0,       // Full alignment enabled
			0,       // Chroma keying disabled
			1,       // Rotate 90 degrees counterclockwise
			0b01,    // Width multiplier
			0b01,    // Height multiplier
			5,       // Line spacing
			0,       // Character spacing
			Xpos_AUX,     // Cursor X
			yposohm       // Cursor Y
		);

		// If in 1000mV range and user has enabled the new 1VDC mode
		if (oneVoltmode && onethousandmVmodedetected) {
			DrawText("   1 V Range");
		}
		else {
			DrawText(AuxdisplayString);
		}

	}
}


//******************************************************************************

void DisplayAnnunciators() {

	// ANNUNCIATORS - Print or clear text on the LCD
	const char* AnnuncNames[19] = {
		"SMPL", "IDLE", "AUTO", "LOP", "NULL", "DFILT", "MATH", "AZERO",
		"ERR", "INFO", "FRONT", "REAR", "SLOT", "LO_G", "RMT", "TLK",
		"LTN", "SRQ"
	};


	// Set Y-position of the annunciators
	int AnnuncYCoords[19] = {
		10,   // SMPL
		62,   // IDLE
		114,  // AUTO
		166,  // LOP
		218,  // NULL
		270,  // DFILT
		322,  // MATH
		374,  // AZERO
		426,  // ERR
		478,  // INFO
		530,  // FRONT
		582,  // REAR
		634,  // SLOT
		686,  // LO_G
		738,  // RMT
		790,  // TLK
		842,  // LTN
		900   // SRQ
	};


	for (int i = 0; i < 18; i++) {
		if (Annunc[i + 1] == 1) {  // Turn the annunciator ON
			SetTextColors(AnnunColourFore, 0x000000); // Foreground: Green, Background: Black
			ConfigureFontAndPosition(
				0b00,    // Internal CGROM
				0b00,    // 16-dot font size
				0b00,    // ISO 8859-1
				0,       // Full alignment enabled
				0,       // Chroma keying disabled
				1,       // Rotate 90 degrees counterclockwise
				0b00,    // Width X0
				0b01,    // Height X0
				5,       // Line spacing
				0,       // Character spacing
				Xpos_ANNUNC,  // Cursor X (fixed)
				AnnuncYCoords[i] // Cursor Y (from array)
			);
			DrawText(AnnuncNames[i]); // Print the corresponding name
		}
		else {  // Turn the annunciator OFF
			SetTextColors(0x000000, 0x000000); // Foreground: Black, Background: Black
			ConfigureFontAndPosition(
				0b00,    // Internal CGROM
				0b00,    // 16-dot font size
				0b00,    // ISO 8859-1
				0,       // Full alignment enabled
				0,       // Chroma keying disabled
				1,       // Rotate 90 degrees counterclockwise
				0b00,    // Width X0
				0b01,    // Height X0
				5,       // Line spacing
				0,       // Character spacing
				Xpos_ANNUNC,  // Cursor X (fixed)
				AnnuncYCoords[i] // Cursor Y (from array)
			);
			DrawText(AnnuncNames[i]); // Clear the text by drawing in black
		}
	}

}

//******************************************************************************

void DisplaySplash() {

	// Splash text to display
	static uint32_t cycle_count = 0; // Persistent counter for cycles
	static uint8_t timer_active = 1; // Flag to track timer status
	if (timer_active) {
		cycle_count++;
		// Check if the 5-second period has elapsed
		if (cycle_count >= (DURATION_MS / TIMER_INTERVAL_MS)) {
			// Runs once
			timer_active = 0; // Stop counting after 5 seconds
			SetTextColors(0x00FF00, 0x000000); // Foreground: Yellow, Background: Black
			ConfigureFontAndPosition(
				0b00,    // Internal CGROM
				0b00,    // Font size
				0b00,    // ISO 8859-1
				0,       // Full alignment enabled
				0,       // Chroma keying disabled
				1,       // Rotate 90 degrees counterclockwise
				0b00,    // Width multiplier
				0b00,    // Height multiplier
				1,       // Line spacing
				4,       // Character spacing
				Xpos_SPLASH,     // Cursor X
				100      // Cursor Y
			);
			char text[] = "                                                           ";
			DrawText(text);
		}
		else {
			// Perform operations within the 5-second window
			// Splash text
			SetTextColors(0x00FF00, 0x000000); // Foreground: Yellow, Background: Black
			ConfigureFontAndPosition(
				0b00,    // Internal CGROM
				0b00,    // Font size
				0b00,    // ISO 8859-1
				0,       // Full alignment enabled
				0,       // Chroma keying disabled
				1,       // Rotate 90 degrees counterclockwise
				0b00,    // Width multiplier
				0b00,    // Height multiplier
				1,       // Line spacing
				4,       // Character spacing
				Xpos_SPLASH,     // Cursor X	230
				100      // Cursor Y	360
			);

			char text[] = "Reverse engineering by By MickleT / TFT LCD by Ian Johnston";	
			DrawText(text);
		}
	}

}