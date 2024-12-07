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

//************************************************************************************************************************************************************


void DisplayMain() {

	// MAIN ROW - Print text to LCD, detect if there is an OHM symbol ($) and if so split into 3 parts, before-OHM-after
	SetTextColors(0xFFFF00, 0x000000); // Foreground: Yellow, Background: Black

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
		int found = 0;
		char MaindisplayStringAfter[19] = "";
		for (int i = 1; i <= 18; i++) {
			if (found) MaindisplayStringAfter[i - found - 1] = G[i];
			if (G[i] == '$') found = i;
		}
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

void DisplayAuxFirstHalf() {

	// AUX ROW 1st half - Print text to LCD

	SetTextColors(0xFFFFFF, 0x000000); // Foreground: Yellow, Background: Black

	char AuxdisplayString[16] = "";              // String for G[19] to G[33]
	char BeforeDollar[16] = "";                   // To store characters before the $
	char AfterDollar[16] = "";                    // To store characters after the $
	uint16_t dollarPositionAUX = 0xFFFF;            // Initialize to an invalid position

	// Populate AuxdisplayString from G[19] to G[33]
	for (int i = 19; i <= 33; i++) {
		AuxdisplayString[i - 19] = G[i];
	}
	AuxdisplayString[15] = '\0';                 // Null-terminate MaindisplayString

	// Find the position of the '$' symbol
	for (int i = 0; i < 15; i++) {
		if (AuxdisplayString[i] == '$') {
			dollarPositionAUX = i;                // Record the position of '$'
			break;                                // Exit loop once found
		}
	}

	// If in 2W or 4W Resistance measurement mode the display will contain the OHM symbol on the AUX display.
	// If it appears then the dollarPositionAUX var will not be 0xFFFF
	if (dollarPositionAUX != 0xFFFF) {

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
		char AuxdisplayStringBefore[15] = "";
		for (int i = 0; i < dollarPositionAUX; i++) { // Corrected loop condition
			AuxdisplayStringBefore[i] = AuxdisplayString[i]; // Corrected indexing
		}
		AuxdisplayStringBefore[dollarPositionAUX] = '\0'; // Null-terminate
		DrawText(AuxdisplayStringBefore);

		HAL_Delay(5);
		ResetGraphicWritePosition_LT();
		Ohms12x24SymbolStoreUCG();			// This is called here rather than pre-defined because the 1st UCG above is a different size
		Text_Mode();

		// Calculate position of $(OHM) symbol
		//       yposohm = (dollarPositionAUX * widthmultiplier * fontwidth) + (characterspacing * dollarPositionAUX) + startoffset(60)
		uint16_t yposohm = (dollarPositionAUX * 1 * 12) + (4 * dollarPositionAUX) + 100;  // Y position of the OHM symbol
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
			yposohm  // Cursor Y
		);
		// Write the OHM symbol
		WriteRegister(0x04);
		WriteData(0x00);    // high byte
		WriteData(0x00);    // low byte

		HAL_Delay(2);
		
		// After
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
			(dollarPositionAUX * 32) + 44  // Cursor     includes a tweak factor because calc didn't quite set Y properly
		);
		int found = 0;
		char AuxdisplayStringAfter[15] = "";
		for (int i = 19; i <= 33; i++) {
			if (found) AuxdisplayStringAfter[i - found - 1] = G[i];
			if (G[i] == '$') found = i;
		}
		DrawText(AuxdisplayStringAfter);

	} else {

		//SetTextColors(0xFFFFFF, 0x0000FF); // Foreground: White, Background: Black
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

		char AuxdisplayString[15] = "";				// String for G[19] to G[33]
		for (int i = 19; i <= 33; i++) {
			AuxdisplayString[i - 19] = G[i];			// Copy characters
		}
		AuxdisplayString[14] = '\0';					// Null-terminate the string (ensure clean output)
		DrawText(AuxdisplayString);

	}
}


//******************************************************************************

void DisplayAuxSecondHalf() {

	// AUX ROW 2nd half - Print text to LCD (no OHM symbols in this 2nd half so no need for extra work)
	SetTextColors(0xFFFFFF, 0x000000); // Foreground: white, Background: Black
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
		Xpos_AUX,     // Cursor X 170
		398      // Cursor Y 480
	);

	char AuxdisplayString2[14] = "";				// String for G[34] to G[47]
	for (int i = 33; i <= 47; i++) {
		AuxdisplayString2[i - 33] = G[i];			// Copy characters
	}
	AuxdisplayString2[15] = '\0';					// Null-terminate the string (ensure clean output)
	DrawText(AuxdisplayString2);

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
			SetTextColors(0x00FF00, 0x000000); // Foreground: Green, Background: Black
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
				80      // Cursor Y
			);
			char text[] = "                                                               ";
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
				80      // Cursor Y	360
			);
			char text[] = "Reverse engineering by By MickleT / TFT LCD by IanscottJohnston";
			DrawText(text);
		}
	}

}