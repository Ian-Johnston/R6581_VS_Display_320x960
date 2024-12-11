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

#define DURATION_MS 5000     // 5 seconds in milliseconds
#define TIMER_INTERVAL_MS 35 // The interval of your timed sub in milliseconds

// Display colours default
uint32_t MainColourFore = 0xFFFF00; // Yellow
uint32_t AuxColourFore = 0xFFFFFF; // White
uint32_t AnnunColourFore = 0x00FF00; // Green

//uint16_t dollarPositionAUX = 0xFFFF;

//************************************************************************************************************************************************************


void DisplayMain() {

	// MAIN ROW - Print text to LCD, detect if there is an OHM symbol ($) and if so split into 3 parts, before-OHM-after
	SetTextColors(MainColourFore, 0x000000); // Foreground: Yellow, Background: Black

	char MaindisplayString[19] = "";              // String for G[1] to G[18]
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
		//int found = 0;
		//char MaindisplayStringAfter[19] = "";
		//for (int i = 1; i <= 18; i++) {
		//	if (found) MaindisplayStringAfter[i - found - 1] = G[i];
		//	if (G[i] == '$') found = i;
		//}
		//DrawText(MaindisplayStringAfter);
		char MaindisplayStringAfter[19] = ""; // Adjust the size to match your max expected characters
		int j = 0; // Index for the new string
		for (int i = dollarPosition + 1; i <= 18; i++) { // Start after the $ and loop through the rest
			MaindisplayStringAfter[j++] = MaindisplayString[i]; // Copy characters to the new string
		}
		MaindisplayStringAfter[18] = '\0'; // Null-terminate the new string
		DrawText(MaindisplayStringAfter);

	} else {

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
	}

}


//******************************************************************************


void DisplayAux() {
	
	// AUX ROW text to LCD

	// This SUB needs tidied up!
	// It has to detect number of $ symbols and print the before and after characters, print the OHM user defined character.
	// The $ symbol can appear twice.

	SetTextColors(AuxColourFore, 0x000000); // Foreground: Yellow, Background: Black

	char AuxdisplayString[30] = "";              // String for G[19] to G[47]	30 characters in total so can store the null terminator also
	char BeforeDollar[30] = "";                   // To store characters before the $
	char AfterDollar[30] = "";                    // To store characters after the $

	// Populate AuxdisplayString from G[19] to G[33]
	for (int i = 19; i <= 47; i++) {			// qty=15
		AuxdisplayString[i - 19] = G[i];
	}
	AuxdisplayString[29] = '\0';                 // Null-terminate at the 30th position because array starts at 0

	// Get number of $ symbols and their position
	int firstDollarPosition = -1;
	int secondDollarPosition = -1;
	int DollarFound = -1;

	// Loop through the string to find the first and second $ positions (there will only ever be 2 max)
	for (int i = 0; i < 30; i++) {
		if (AuxdisplayString[i] == '$') {
			if (firstDollarPosition == -1) {
				firstDollarPosition = i; // First $ found
			}
			else {
				secondDollarPosition = i; // Second $ found
				break; // Exit loop after finding the second $
			}
		}
	}

	// Check results
	if (firstDollarPosition != -1 && secondDollarPosition != -1) {
		DollarFound = 2;
	}
	else if (firstDollarPosition != -1) {
		DollarFound = 1;
	}
	else {
		DollarFound = 0;
	}


	// If in 2W or 4W Resistance measurement mode the display will contain the OHM symbol on the AUX display.
	if (DollarFound != 0) {

		// $ symbol found
		// Before
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
			60       // Cursor Y
		);
		char AuxdisplayStringBefore[30] = "";
		for (int i = 0; i < firstDollarPosition; i++) { // Corrected loop condition
			AuxdisplayStringBefore[i] = AuxdisplayString[i]; // Corrected indexing
		}
		firstDollarPosition = firstDollarPosition + 1; // compensate for original zero indexed position, i.e. in 2W ohms mode position = 6

		AuxdisplayStringBefore[firstDollarPosition] = '\0'; // Null-terminate
		DrawText(AuxdisplayStringBefore);

		HAL_Delay(5);
		ResetGraphicWritePosition_LT();
		Ohms12x24SymbolStoreUCG(); // This is called here rather than pre-defined because the 1st UCG above is a different size
		Text_Mode();

		// Calculate position of $ (OHM) symbol
		// hold on to your hats, I couldn't map 6th and 10th positions to pixels, so fudged it as follows
		uint16_t calculated_value = (firstDollarPosition * 2 * 12) + ((3 + 1) * firstDollarPosition) + (2 * (3 + 1)) + 2;
		uint16_t yposohm = (uint16_t)((calculated_value * 0.875) + 22);  // Adjusted Y position of the OHM symbol 24

		// The actual OHM
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
			4,       // Character spacing
			Xpos_AUX,     // Cursor X 170
			yposohm - 2  // Cursor Y - compensated by 2 pixels
		);
		// Write the OHM symbol
		WriteRegister(0x04);
		WriteData(0x00);    // high byte
		WriteData(0x00);    // low byte

		HAL_Delay(2);

		if (DollarFound == 1) {
			// Only one $ symbol was found so can print the AFTER and thats it finished
			
			// After to end
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
				yposohm + 25  // Cursor Y - double width is 12x2 + 3 pixels padding = 27, but set to 25
			);
			char AuxdisplayStringAfter[30] = ""; // Adjust the size to match your max expected characters
			int j = 0; // Index for the new string
			for (int i = firstDollarPosition; i < 30; i++) { // Start after the $ and loop through the rest
				AuxdisplayStringAfter[j++] = AuxdisplayString[i]; // Copy characters to the new string
			}
			AuxdisplayStringAfter[29] = '\0'; // Null-terminate the new string
			DrawText(AuxdisplayStringAfter);

		} else {
			// Two $ symbols were found - This only happens in the CALIBRATION EXTERNAL menu

			HAL_Delay(2);

			// After 1st $ symbol but before 2nd
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
				yposohm + 23  // Cursor Y - double width is 12x2 + 3 pixels padding = 27, but set to 23
			);
			char AuxdisplayStringAfter[30] = ""; // Adjust the size to match your max expected characters
			int j = 0; // Index for the new string
			for (int i = firstDollarPosition; i < secondDollarPosition; i++) { // Start after the $ and loop through the rest
				AuxdisplayStringAfter[j++] = AuxdisplayString[i]; // Copy characters to the new string
			}
			AuxdisplayStringAfter[secondDollarPosition - 1] = '\0'; // Null-terminate the new string
			DrawText(AuxdisplayStringAfter);

			HAL_Delay(2);

			// Calculate position of $ (OHM) symbol
			// hold on to your hats, I couldn't map 6th and 10th positions to pixels, so fudged it as follows
			secondDollarPosition = secondDollarPosition + 1;	// new position after OHM
			uint16_t calculated_value = (secondDollarPosition * 2 * 12) + ((3 + 1) * secondDollarPosition) + (2 * (3 + 1)) + 2;
			uint16_t yposohm = (uint16_t)((calculated_value * 0.875) + 22);  // Adjusted Y position of the OHM symbol 24

			// The actual 2nd OHM
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
				4,       // Character spacing
				Xpos_AUX,     // Cursor X 170
				yposohm - 8  // Cursor Y			// -2 just to compensate a few of pixels
			);
			// Write the OHM symbol
			WriteRegister(0x04);
			WriteData(0x00);    // high byte
			WriteData(0x00);    // low byte

			HAL_Delay(2);

			// Now print characters after 2nd $ position
			// After 2nd to end
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
				yposohm + 17  // Cursor Y - double width is 12x2 + 3 pixels padding = 27......but set to 17
			);
			char AuxdisplayStringAftersecond[30] = ""; // Adjust the size to match your max expected characters
			int k = 0; // Index for the new string
			for (int i = secondDollarPosition; i < 30; i++) { // Start after the $ and loop through the rest
				AuxdisplayStringAftersecond[k++] = AuxdisplayString[i]; // Copy characters to the new string
			}
			AuxdisplayStringAftersecond[29] = '\0'; // Null-terminate the new string
			DrawText(AuxdisplayStringAftersecond);
		}

	} else {

		// No $ symbols at all found so just print the entire string as-is
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
			60       // Cursor Y
		);

		char AuxdisplayString[30] = "";       // String for G[19] to G[33], 30 total, 29 plus null terminator
		for (int i = 19; i <= 47; i++) {
			AuxdisplayString[i - 19] = G[i];    // Copy characters
		}
		AuxdisplayString[29] = '\0';         // Null-terminate the string (ensure clean output) at 30th position
		DrawText(AuxdisplayString);

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