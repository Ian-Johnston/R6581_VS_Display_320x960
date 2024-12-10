/**
  ******************************************************************************
  * @file    lt7680.c
  * @brief   This file provides code for the
  *          LT7680 commands
  ******************************************************************************
  * Commands:
  * WriteRegister = Specify address, and typicall follow up with WriteData (1 or more times) to write data to a register.
  * uint8_t ReadDataFromRegister(reg_address) = write to register and it will return the data within.
  * uint8_t status = ReadData() = Call after a WriteRegister and it will return data at register address.
  * WriteDataToRegister(uint8_t reg, uint8_t value) = In one go spedify address and data to send.
  *
  * Status Registers can read data through the state reading period, and it can only be 
  * read and cannot be written. Instruction Registers can control most of the functions through the 
  * "Command Write" cycle and the "Data Write" cycle. "Command Write" specifies the address of the 
  * register, and then the "Data Write" period can be written to the specified register. And when the specified 
  * register data is to be read, the master will need to send the "Command write" cycle first. Then use the 
  * "Data Read" cycle to read the data. In other words, "Command Write" is the set register address, "Data 
  * Read" is to read register data. 
  * 
  * ER-TFT4.58-1 TFT LCD, 320x960 pixels
  * LCD Controller = ST7701S
  * Response time = 25ms
  * Type = IPS
  * Colours = 262K
  * 
  * Controller = LT7680A-R
  * 64Mb version
*/

#include "lt7680.h"
#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

// SPI handle (ensure this matches your actual SPI instance)
extern SPI_HandleTypeDef hspi1;

char LT7680StatusMessages[8][50]; // 8 messages, each up to 50 characters long
volatile uint8_t system_ok = 0;
volatile uint8_t LT7680_SPI_Read_ok = 0;
volatile uint8_t System_Check = 0;
volatile uint8_t SystemCheckTempValue = 0;
volatile uint32_t TEST1 = 0;
volatile uint32_t TEST2 = 0;
volatile uint32_t TEST3 = 0;
volatile uint32_t TEST4 = 0;
volatile uint32_t TEST5 = 0;
volatile uint32_t TEST6 = 0;
volatile uint32_t TEST7 = 0;
volatile uint32_t TEST8 = 0;
volatile uint32_t TEST9 = 0;
volatile uint32_t TEST10 = 0;
volatile uint32_t TEST11 = 0;
volatile uint32_t TEST12 = 0;
volatile uint32_t TEST13 = 0;
volatile uint32_t TEST14 = 0;
volatile uint32_t TEST15 = 0;
volatile uint32_t TEST16 = 0;

void HardwareReset(void) {
    HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_RESET); // Pull reset low
    HAL_Delay(100); // Delay 100 ms
    HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_SET);   // Release reset
    HAL_Delay(100); // Delay 100 ms
}


//#define RESET_LOW()  HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_RESET)
//#define RESET_HIGH() HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_SET)

//**************************************************************************************************
// Core commands

// Write Register Address
void WriteRegister(uint8_t reg) {
    uint8_t controlByte = 0x00; // A0 = 0, RW = 0
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET); // CS Low
    HAL_SPI_Transmit(&hspi1, &controlByte, 1, HAL_MAX_DELAY);                 // Send control byte
    HAL_SPI_Transmit(&hspi1, &reg, 1, HAL_MAX_DELAY);                         // Send register address
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET);   // CS High
}

// Write Data
void WriteData(uint8_t data) {
    uint8_t controlByte = 0x80; // A0 = 1, RW = 0
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET); // CS Low
    HAL_SPI_Transmit(&hspi1, &controlByte, 1, HAL_MAX_DELAY);                 // Send control byte
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);                        // Send data byte
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET);   // CS High
}

// Read Status Register
uint8_t ReadStatus(void) {
    uint8_t controlByte = 0x40; // A0 = 0, RW = 1
    uint8_t status = 0x00;      // Variable to hold the status byte
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET); // CS Low

    // Attempt SPI transmit and receive
    if (HAL_SPI_Transmit(&hspi1, &controlByte, 1, HAL_MAX_DELAY) == HAL_OK &&
        HAL_SPI_Receive(&hspi1, &status, 1, HAL_MAX_DELAY) == HAL_OK) {
        LT7680_SPI_Read_ok = 1; // SPI read was successful
    }
    else {
        LT7680_SPI_Read_ok = 0; // SPI read failed
    }

    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET);   // CS High
    return status;
}

// Read Data from Register
uint8_t ReadData(void) {
    uint8_t controlByte = 0xC0; // A0 = 1, RW = 1
    uint8_t data = 0x00; // Variable to hold the data byte
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET); // CS Low
    HAL_SPI_Transmit(&hspi1, &controlByte, 1, HAL_MAX_DELAY);                 // Send control byte
    HAL_SPI_Receive(&hspi1, &data, 1, HAL_MAX_DELAY);                         // Read data byte
    HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET);   // CS High
    return data;
}

// Write Register Address and Data (combined) - optional
void WriteDataToRegister(uint8_t reg, uint8_t value) {
    WriteRegister(reg); // Write the register address
    WriteData(value);   // Write the data to the register
}


//**************************************************************************************************
// Subs to run and sent to the LT7680

void SendAllToLT7680_LT() {
  
    Software_Reset_LT();
    HAL_Delay(10);
    LT7680_PLL_Initial_LT();                  // Initialize PLL first for stable clocks
    HAL_Delay(100);
    SDRAM_Init_LT();                          // Initialize SDRAM after the reset
    HAL_Delay(5);
    Set_LCD_Panel_LT();                       // Set up the panel interface
    HAL_Delay(100);

    LCDConfigTurnOn_LT();
    HAL_Delay(5);

    LCD_HorizontalWidth_VerticalHeight_LT(LCD_XSIZE_TFT, LCD_YSIZE_TFT);
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
    SetColorDepth_LT();                       // Configure canvas color depth
    HAL_Delay(5);
    Configure_Main_PIP_Window_LT();
    HAL_Delay(5);
    SetMainImageWidth_LT();
    HAL_Delay(5);
    SetMainWindowUpperLeftX_LT();
    HAL_Delay(5);
    ConfigureActiveDisplayArea_LT();
    HAL_Delay(5);
    SetActiveWindow_LT();                     // Set active window dimensions
    HAL_Delay(5);
    SetCanvasStartAddress_LT();
    HAL_Delay(5);
    SetCanvasImageWidth_LT();
    HAL_Delay(5);
    ResetGraphicWritePosition_LT();
    HAL_Delay(5);
    SetGraphicRWYCoordinate_LT();
    HAL_Delay(5);
    Set_MISA_LT();                            // Configure the Main Image Start Address
    HAL_Delay(5);
    ConfigurePWMAndSetBrightness(BACKLIGHT);  // Configure Timer-1 and PWM-1 for backlighting. Settable 0-100%
    HAL_Delay(5);
    
    Text_Mode();
    ClearScreen();                          // Draws black 'spaces' across the whole screen - fast
     
}


//******************************************************************************
// ROUTINES

// UGC symbol - 16x32
void Ohms16x32SymbolStoreUCG() {
    // CGRAM_START_ADDR 0x1000  // Base address for CGRAM
    // UGC_CODE 0x0000 // UGC code for 16/32 & 12x24 OHM symbols
    // CGRAM ADDR + (UGC_CODE * 64);  // Calculate UCG address for 16x32

    // 16x32 UCG data for the Ohm symbol
    // written to 1000h
    uint8_t ohm[64] = {
        0x00, 0x00,  // Row 1:  0000000000000000        // UCG code = 0000h
        0x00, 0x00,  // Row 2:  0000000000000000
        0x00, 0x00,  // Row 3:  0000000000000000
        0x00, 0x00,  // Row 4:  0000000000000000
        0x00, 0x00,  // Row 5:  0000000000000000
        0x00, 0x00,  // Row 6:  0000000000000000
        0x0F, 0xF0,  // Row 7:  0000111111110000
        0x1F, 0xF8,  // Row 8:  0001111111111000
        0x30, 0x0C,  // Row 9:  0011000000001100
        0x60, 0x06,  // Row 10: 0110000000000110
        0x60, 0x06,  // Row 11: 0110000000000110
        0x60, 0x06,  // Row 12: 0110000000000110
        0x60, 0x06,  // Row 13: 0110000000000110
        0x60, 0x06,  // Row 14: 0110000000000110
        0x60, 0x06,  // Row 15: 0110000000000110
        0x60, 0x06,  // Row 16: 0110000000000110
        0x60, 0x06,  // Row 17: 0110000000000110
        0x60, 0x06,  // Row 18: 0110000000000110
        0x60, 0x06,  // Row 19: 0110000000000110
        0x30, 0x0C,  // Row 20: 0011000000001100
        0x18, 0x18,  // Row 21: 0001100000011000
        0x0C, 0x30,  // Row 22: 0000110000110000
        0x0C, 0x30,  // Row 23: 0000110000110000
        0x0C, 0x30,  // Row 24: 0000110000110000
        0x7C, 0x3E,  // Row 25: 0111110000111110
        0x7C, 0x3E,  // Row 26: 0111110000111110
        0x00, 0x00,  // Row 27: 0000000000000000
        0x00, 0x00,  // Row 28: 0000000000000000
        0x00, 0x00,  // Row 29: 0000000000000000
        0x00, 0x00,  // Row 30: 0000000000000000
        0x00, 0x00,  // Row 31: 0000000000000000
        0x00, 0x00,  // Row 32: 0000000000000000
    };

    // Set memory destination for user-characters - 00b user characters
    WriteRegister(0x03);
    uint8_t temp = 0;
    temp |= (0 << 0);
    temp |= (0 << 1);
    WriteData(temp);

    for (uint8_t i = 0; i < 64; i++) {
        WriteRegister(0x04);
        WriteData(ohm[i]);
    }

}


// UGC symbol - 12x24
void Ohms12x24SymbolStoreUCG() {
    // CGRAM_START_ADDR 0x1000  // Base address for CGRAM
    // UGC_CODE 0x0000 // UGC code for 16/32 & 12x24 OHM symbols
    // CGRAM ADDR + (UGC_CODE * 48);  // Calculate UCG address for 12x24

    // 12x24 UCG data for the Ohm symbol
    // written to 1000h
    uint8_t ohm[48] = {
        0x00, 0x00,  // Row 1:  000000000000
        0x00, 0x00,  // Row 2:  000000000000
        0x00, 0x00,  // Row 3:  000000000000
        0x00, 0x00,  // Row 4:  000000000000
        0x00, 0x00,  // Row 5:  000000000000
        0x07, 0x00,  // Row 6:  000001110000
        0x0F, 0x80,  // Row 7:  000011111000
        0x18, 0xC0,  // Row 8:  000110001100
        0x30, 0x60,  // Row 9:  001100000110
        0x30, 0x60,  // Row 10:  001100000110
        0x30, 0x60,  // Row 11: 001100000110
        0x30, 0x60,  // Row 12: 001100000110
        0x30, 0x60,  // Row 13: 001100000110
        0x30, 0x60,  // Row 14: 001100000110
        0x18, 0xC0,  // Row 15: 000110001100
        0x0C, 0xC0,  // Row 16: 000011001100
        0x0C, 0xC0,  // Row 17: 000011001100
        0x3C, 0xF0,  // Row 18: 011110001111
        0x3C, 0xF0,  // Row 19: 011110001111
        0x00, 0x00,  // Row 20: 000000000000
        0x00, 0x00,  // Row 21: 000000000000
        0x00, 0x00,  // Row 22: 000000000000
        0x00, 0x00,  // Row 23: 000000000000
        0x00, 0x00   // Row 24: 000000000000
    };


    // Set memory destination for user-characters - 00b user characters
    WriteRegister(0x03);
    uint8_t temp = 0;
    temp |= (0 << 0);
    temp |= (0 << 1);
    WriteData(temp);

    for (uint8_t i = 0; i < 48; i++) {
        WriteRegister(0x04);
        WriteData(ohm[i]);
    }

}

// Print character to LCD
void ConfigureFontAndPosition(uint8_t fontSource, uint8_t characterHeight, uint8_t isoCoding, uint8_t fullAlignment, uint8_t chromaKeying, uint8_t rotation, uint8_t widthFactor, uint8_t heightFactor, uint8_t lineGap, uint8_t charSpacing, uint16_t cursorX, uint16_t cursorY) {

    /*
    fontSource,       memory area
    characterHeight,  00b: 16 dots, 01b: 24 dots, 10b: 32 dots
    isoCoding,        00b: ISO 8859-1, 01b: ISO 8859-2, 10b: ISO 8859-4, 11b: ISO 8859-5
    fullAlignment,    0: Disable, 1: Enable
    chromaKeying,     0: Disable, 1: Enable
    rotation,         0: Normal, 1: Counterclockwise 90 degrees
    widthFactor,      00b: X1, 01b: X2, 10b: X3, 11b: X4
    heightFactor,     00b: X1, 01b: X2, 10b: X3, 11b: X4
    lineGap,          Line gap in pixels (0-31)
    charSpacing,      Character-to-character spacing in pixels (0-63)
    cursorX,          X-coordinate for text cursor
    cursorY           Y-coordinate for text cursor
    */

    uint8_t ccr0 = 0; // Character Control Register 0
    uint8_t ccr1 = 0; // Character Control Register 1

    // Configure CCR0 (REG[CCh])
    ccr0 |= ((fontSource & 0b11) << 6);         // Font source selection
    ccr0 |= ((characterHeight & 0b11) << 4);    // Character height
    ccr0 |= (isoCoding & 0b11);                 // ISO coding

    WriteRegister(0xCC); // Write to CCR0
    WriteData(ccr0);

    // Configure CCR1 (REG[CDh])
    ccr1 |= (fullAlignment << 7);               // Full alignment
    ccr1 |= (chromaKeying << 6);                // Chroma keying
    ccr1 |= (rotation << 4);                    // Rotation
    ccr1 |= ((widthFactor & 0b11) << 2);        // Character width enlargement
    ccr1 |= (heightFactor & 0b11);              // Character height enlargement

    WriteRegister(0xCD); // Write to CCR1
    WriteData(ccr1);

    // Configure Character Line Gap (REG[D0h])
    WriteRegister(0xD0);
    WriteData(lineGap & 0x1F); // Line gap (5 bits)

    // Configure Character-to-Character Space (REG[D1h])
    WriteRegister(0xD1);
    WriteData(charSpacing & 0x3F); // Character spacing (6 bits)

    // Set Cursor Position
    WriteRegister(0x63); // X lower byte
    WriteData(cursorX & 0xFF);
    WriteRegister(0x64); // X upper byte
    WriteData((cursorX >> 8) & 0x1F);

    WriteRegister(0x65); // Y lower byte
    WriteData(cursorY & 0xFF);
    WriteRegister(0x66); // Y upper byte
    WriteData((cursorY >> 8) & 0x1F);
}


void ClearScreen() {
    uint16_t charWidth = 8;      // Character width in pixels
    uint16_t charHeight = 16;    // Character height in pixels

    SetTextColors(0x000000, 0x000000); // foreground, background - black

    // Configure the font and position once
    ConfigureFontAndPosition(
        0b00,    // Internal CGROM
        0b00,    // Font size
        0b00,    // ISO 8859-1
        0,       // Full alignment disabled
        0,       // Chroma keying disabled
        1,       // Rotate 90 degrees counterclockwise
        0b01,    // Width X1
        0b01,    // Height X1
        0,       // Line spacing
        0,       // Char spacing
        0,       // Cursor X
        0        // Cursor Y
    );

    // Iterate through rows and columns
    for (uint16_t y = 0; y < LCD_YSIZE_TFT; y += charWidth) {   // In rotation, width becomes Y step
        for (uint16_t x = 0; x < LCD_XSIZE_TFT; x += charHeight) { // In rotation, height becomes X step
            // Set cursor position
            WriteRegister(0x63); // X lower byte
            WriteData(x & 0xFF);
            WriteRegister(0x64); // X upper byte
            WriteData((x >> 8) & 0x1F);

            WriteRegister(0x65); // Y lower byte
            WriteData(y & 0xFF);
            WriteRegister(0x66); // Y upper byte
            WriteData((y >> 8) & 0x1F);

            // Print the character
            DrawText(" ");
        }
    }

}


// Draw Text Chunks
void DrawTextChunks(char* text) {
    uint8_t maxChunkSize = 20; // Limit to 20 characters
    char buffer[21];           // Buffer for 20 characters + null terminator
    uint8_t length = strlen(text);
    uint8_t index = 0;

    while (index < length) {
        uint8_t chunkSize = (length - index > maxChunkSize) ? maxChunkSize : (length - index);
        strncpy(buffer, text + index, chunkSize); // Copy up to 20 characters
        buffer[chunkSize] = '\0';                // Null-terminate the chunk
        DrawText(buffer);                        // Send to the LCD
        index += chunkSize;                      // Move to the next chunk
    }
}


// Draw text with FIFO checking
void DrawText(char* text) {
    while (*text != '\0') {
        // Check the Tx FIFO Full Flag (Bit 6 of REG[BAh])
        WriteRegister(0xBA);     // Point to the SPI Master Status Register
        WriteData(0x00);         // Dummy write to follow WriteRegister requirement

        uint8_t Registerdata = ReadData(); // Read the register value

        // Wait until the FIFO is not full
        while (Registerdata & (1 << 6)) {
            WriteRegister(0xBA);     // Repoint to the SPI Master Status Register
            WriteData(0x00);         // Dummy write to follow WriteRegister requirement
            Registerdata = ReadData(); // Update the register value
        }

        WriteRegister(0x04);         // Register for writing text
        WriteData((uint8_t)*text); // Write each character
        ++text; // Move to the next character
    }
}



/*
// Draw text
void DrawText(char* text) {                                         // simple version
    WriteRegister(0x04); // Register for writing text
    while (*text != '\0') {
        WriteData((uint8_t)*text); // Write each character
        ++text; // Move to the next character
    }
}
*/


// Set text colours
void SetTextColors(uint32_t foreground, uint32_t background) {
    // Set foreground color
    WriteRegister(0xD2); // Foreground Red
    WriteData((foreground >> 16) & 0xFF);
    WriteRegister(0xD3); // Foreground Green
    WriteData((foreground >> 8) & 0xFF);
    WriteRegister(0xD4); // Foreground Blue
    WriteData(foreground & 0xFF);

    // Set background color
    WriteRegister(0xD5); // Background Red
    WriteData((background >> 16) & 0xFF);
    WriteRegister(0xD6); // Background Green
    WriteData((background >> 8) & 0xFF);
    WriteRegister(0xD7); // Background Blue
    WriteData(background & 0xFF);
}


// Register 0x03 - Bit 2 only
void Text_Mode(void) // Set the LCD to Text Mode
{
    uint8_t temp = 0;

    // Set Bit 2 to 1 for Text Mode
    temp |= (1 << 2);

    // Configure Bits 1-0 for Display RAM (if required)
    temp &= ~(0b11); // Clear Bits 1-0 to select Display RAM

    // Write the value to Register 0x03 (Text/Graphic Mode register)
    WriteRegister(0x03);
    WriteData(temp);
}


void SetFontTypeSize(uint8_t fontType, uint8_t fontSize) {               // - OK
    uint8_t regValue = 0;

    // Configure the Font Control Register (0xCC)
    // Set bits for font size (bits 5:4) and font type (bits 1:0)
    regValue |= ((fontSize & 0x03) << 4) | (fontType & 0x03);

    // Write the configured value directly to the Font Control Register
    WriteRegister(0xCC);
    WriteData(regValue);
}


// Register 0x03 - Bit 2 only
void Graphics_Mode()     // - OK
{
    uint8_t temp = 0;

    // Configure the register directly for Text Mode
    temp |= (0 << 2); // Enable Graphics Mode (clear bit 2)

    // Write the value directly to the Text/Graphic Mode register (0x03)
    WriteRegister(0x03);
    WriteData(temp);
}


//******************************************************************************
// REGISTER CONFIG

void CGRAM_Start_address()
{
    uint8_t temp = 0x0000;

    WriteRegister(0xDB);
    WriteData(temp);
    WriteRegister(0xDC);
    WriteData(temp >> 8);
    WriteRegister(0xDD);
    WriteData(temp >> 16);
    WriteRegister(0xDE);
    WriteData(temp >> 24);
}

void Font_Select_UserDefine_Mode(void)
{
    /*[bit7-6]
    User-defined Font /CGROM Font Selection Bit in Text Mode
    00 : Internal CGROM
    01 : Genitop serial flash
    10 : User-defined Font
    */
    unsigned char temp;
    WriteRegister(0xCC);
    temp = ReadData();
    temp |= (1 << 7);
    temp |= (0 << 6);    //&= cClrb6;
    WriteRegister(0xCC);
    WriteData(temp);
}


void ConfigureActiveDisplayArea_LT() {
    // Set Active Window to cover the entire screen
    WriteRegister(0x56); WriteData(0x00);  // X Start Low
    WriteRegister(0x57); WriteData(0x00);  // X Start High
    WriteRegister(0x58); WriteData(0x00);  // Y Start Low
    WriteRegister(0x59); WriteData(0x00);  // Y Start High
    WriteRegister(0x5A); WriteData((LCD_XSIZE_TFT - 1) & 0xFF);  // X End Low
    WriteRegister(0x5B); WriteData(((LCD_XSIZE_TFT - 1) >> 8) & 0xFF);  // X End High
    WriteRegister(0x5C); WriteData((LCD_YSIZE_TFT - 1) & 0xFF);  // Y End Low
    WriteRegister(0x5D); WriteData(((LCD_YSIZE_TFT - 1) >> 8) & 0xFF);  // Y End High
}

/*
void SetTextCursor(uint16_t x, uint16_t y) {                             // - OK
    // Set X-Coordinate
    WriteRegister(0x63); // Lower 8 bits of X position
    WriteData(x & 0xFF);
    WriteRegister(0x64); // Upper 5 bits of X position
    WriteData((x >> 8) & 0x1F); // Only bits 12:8 are valid

    // Set Y-Coordinate
    WriteRegister(0x65); // Lower 8 bits of Y position
    WriteData(y & 0xFF);
    WriteRegister(0x66); // Upper 5 bits of Y position
    WriteData((y >> 8) & 0x1F); // Only bits 12:8 are valid
}



void DrawText(uint8_t encoding, char *text) {                            // - OK
    // Set the font encoding in Register 0xCC
    uint8_t regValue = (encoding & 0x03); // Encoding occupies Bit 1-0
    WriteRegister(0xCC);
    WriteData(regValue);

    // Send the text to the display
    WriteRegister(0x04); // Register for writing text
    while (*text != '\0') {
        WriteData((uint8_t)*text);
        ++text; // Advance to the next character
    }
}
*/

//**************************************************************/

void SoftwareReset(void) {       // From LT7680 datasheet
    // Write to the control register to initiate a software reset
    WriteRegister(REG_CONTROL); // Select the control register
    WriteData(0x01);                  // Set bit 0 to initiate the reset

    // Optional: Add a delay to ensure the reset completes
    HAL_Delay(100); // 10 ms delay
}

//**************************************************************/

/*
void ClearScreen(uint32_t color) {
    
	SetBackgroundColor(color);
    WriteRegister(0x2C); // Memory Write

	
    for (uint32_t i = 0; i < (320 * 960); i++) {            // NOTE: This is slow, will inhibit startup by a few secs
        WriteData((color >> 16) & 0xFF); // Red
        WriteData((color >> 8) & 0xFF);  // Green
        WriteData(color & 0xFF);         // Blue
    }
  
	
	uint8_t red = (color >> 16) & 0xFF;
    uint8_t green = (color >> 8) & 0xFF;
    uint8_t blue = color & 0xFF;

    WriteRegister(0x01); // Example: Clear Screen Command
    WriteData(red);
    WriteData(green);
    WriteData(blue);
    
}
*/


/*
void SetTextColor(uint32_t color) {
    uint8_t red = (color >> 16) & 0xFF;
    uint8_t green = (color >> 8) & 0xFF;
    uint8_t blue = color & 0xFF;

    WriteRegister(0x02); // Example: Set Text Color Command
    WriteData(red);
    WriteData(green);
    WriteData(blue);
}
*/


//void SetFont(uint8_t fontIndex) {
//    WriteRegister(0x03); // Example: Set Font Command
//    WriteData(fontIndex);
//}




void SetBacklightFull(void) {
	
	WriteRegister(0x84); // Set Prescaler Register (adjust as needed)
    WriteData(0x00);    // Default prescaler

    WriteRegister(0x85); // Timer Clock Divider (adjust as needed)
    WriteData(0x00);    // Default divisor

    WriteRegister(0x88); // Timer Compare (TCMPB0)
    WriteData(0xFF);    // Maximum brightness (100% duty cycle)

    WriteRegister(0x8A); // Timer Counter (TCNTB0)
    WriteData(0xFF);    // Match compare for full PWM

    WriteRegister(0x86); // Enable PWM0
    WriteData(0x01);    // Turn on PWM0
}

// Test routine
void FillScreen(uint32_t color) {
    // Set background color
    WriteRegister(0x63); // Register for Background Color Low
    WriteData(color & 0xFF);
    WriteRegister(0x64); // Register for Background Color Middle
    WriteData((color >> 8) & 0xFF);
    WriteRegister(0x65); // Register for Background Color High
    WriteData((color >> 16) & 0xFF);

    // Set area to fill (entire screen, adjust to your resolution)
    WriteRegister(0x22); // Main Image Width Low
    WriteData(0x40); // 320 pixels
    WriteRegister(0x23); // Main Image Height Low
    WriteData(0x78); // 960 pixels

    // Execute fill command
    WriteRegister(0x30); // Command to fill area
    WriteData(0x01);     // Start the fill
}


//**************************************************************************************************
// Subs to run and sent to the LT7680 - Translated from Levetop sample info

// Register 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x00
void LT7680_PLL_Initial_LT() {
    // Parameters

    // Clock calculations
    unsigned int temp = (LCD_HBPD + LCD_HFPD + LCD_HSPW + LCD_XSIZE_TFT) *
        (LCD_VBPD + LCD_VFPD + LCD_VSPW + LCD_YSIZE_TFT) * REFRESH_RATE;              // = 38208000
  
    temp = (temp + 500000) / 1000000; // Round to the nearest MHz           1000000

    unsigned short SCLK = temp;
    unsigned short MCLK = temp * 2;
    unsigned short CCLK = temp * 2;

    // Limit frequencies to specified max values
    if (SCLK > SCLK_MAX) SCLK = SCLK_MAX;
    if (MCLK > MCLK_MAX) MCLK = MCLK_MAX;
    if (CCLK > CCLK_MAX) CCLK = CCLK_MAX;

    // PLL parameters
    unsigned short lpllOD_sclk = 2, lpllOD_cclk = 2, lpllOD_mclk = 2;
    unsigned short lpllR_sclk = 5, lpllR_cclk = 5, lpllR_mclk = 5;
    unsigned short lpllN_sclk = SCLK, lpllN_cclk = CCLK, lpllN_mclk = MCLK;

    // Configure PCLK PLL - TFT pixel clock (max=80MHz) (Registers 0x05 and 0x06)
    WriteRegister(0x05);
    WriteData((lpllOD_sclk << 6) | (lpllR_sclk << 1) | ((lpllN_sclk >> 8) & 0x1));      // 8A
    //WriteData(0x56);          // test
    
    WriteRegister(0x06);
    WriteData(lpllN_sclk & 0xFF);                                                       // 1B
    //WriteData(0x10);        // test, fixes wierd colour on the "J" on "IanJ" text, but causes flicker

    // Configure MCLK PLL - Display memory clock (max=133MHz) (Registers 0x07 and 0x08)
    WriteRegister(0x07);
    WriteData((lpllOD_mclk << 6) | (lpllR_mclk << 1) | ((lpllN_mclk >> 8) & 0x1));      // 8A
    //WriteData(0x8A);          // test
    
    WriteRegister(0x08);                // 36
    WriteData(lpllN_mclk & 0xFF);                                                       // 36
    //WriteData(0x56);          // test

    // Configure CCLK PLL - Core clock (max=100MHz) (Registers 0x09 and 0x0A)
    WriteRegister(0x09);
    WriteData((lpllOD_cclk << 6) | (lpllR_cclk << 1) | ((lpllN_cclk >> 8) & 0x1));      // 8A
    //WriteData(0x56);          // test
    
    WriteRegister(0x0A);
    WriteData(lpllN_cclk & 0xFF);                                                       // 36
    //WriteData(0x56);          // test

    // Trigger PLL reconfiguration (Register 0x00)
    WriteRegister(0x00);
    WriteData(0x80);

    // Add delay to allow PLL settings to stabilize
    HAL_Delay(10); // delay
}


// Register 0x10, 0x11, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35
void Configure_Main_PIP_Window_LT() {
    uint8_t regValue = 0;

    // Hardcoded configuration:
    regValue |= (0 << 7);  // Bit 7: Disable PIP-1 Window
    regValue |= (0 << 6);  // Bit 6: Disable PIP-2 Window
    regValue |= (0 << 4);  // Bit 4: Configure PIP-1 parameters - Don't need to set this
    regValue |= (0b01 << 2);  // Bits 3-2: 16bpp Generic TFT (65K color) - Ties up with ST7701S setting
    regValue |= (0 << 0);  // Bit 0: Sync Mode (VSYNC, HSYNC, DE enabled)

    // Write the configuration to REG[10h]
    WriteRegister(0x10);
    WriteData(regValue);

    // PIP Window Upper-Left Corner (0,0)
    WriteRegister(0x2A); WriteData(0x00);  // Upper-left X coord Low Byte
    WriteRegister(0x2B); WriteData(0x00);  // Upper-left X coord High Byte
    WriteRegister(0x2C); WriteData(0x00);  // Upper-left Y coord Low Byte
    WriteRegister(0x2D); WriteData(0x00);  // Upper-left Y coord High Byte

    // PIP Window Bottom-Right Corner (100,100)
    WriteRegister(0x2E); WriteData((100 & 0xFC));     // Lower-right X coord Low Byte (ensure bit[1:0] = 0)
    WriteRegister(0x2F); WriteData((100 >> 8) & 0x1F); // Lower-right X coord High Byte (bits [12:8])
    WriteRegister(0x30); WriteData(100 & 0xFF);       // Lower-right Y coord Low Byte
    WriteRegister(0x31); WriteData((100 >> 8) & 0x1F); // Lower-right Y coord High Byte (bits [12:8])

    // Set PIP Image Start Address to 0x000000 (example, ensure data exists here in SDRAM)
    WriteRegister(0x32); WriteData(0x00);  // Address bits [7:0]
    WriteRegister(0x33); WriteData(0x00);  // Address bits [15:8]
    WriteRegister(0x34); WriteData(0x00);  // Address bits [23:16]
    WriteRegister(0x35); WriteData(0x00);  // Address bits [31:24]

    // Set PIP-1 and PIP-2 color depth to 16bpp (Bits 3-2 and 1-0 set to 01)
    uint8_t regValue2 = 0;
    regValue2 |= (0b01 << 2);  // PIP-1 Color Depth: 16bpp
    regValue2 |= (0b01 << 0);  // PIP-2 Color Depth: 16bpp
    // Write to Register 0x11
    WriteRegister(0x11);
    WriteData(regValue2);

}



// Register 0xE0, 0xE1, 0xE2, 0xE3, 0xE4
void SDRAM_Init_LT() {

    unsigned short sdram_itv;
    uint8_t regValue1 = 0;

    // Step 1: Enable SDRAM Timing Parameter Registers (Bit 2 = 1)
    WriteRegister(0xE4);        // Allow addresses 0xE0 to 0xE3 to be set
    regValue1 |= (1 << 2);      // Set Bit 2
    WriteData(regValue1);

    // Step 2: Configure SDRAM settings
    WriteRegister(0xE0);        // Register 0xE0: SDRAM Control Register
    WriteData(0x29);            // Default SDRAM control value specifically for LT7680A-R      LT7680A = 64MB, LT&^*)A-R 128MB I think!   set to 0x21 or 0x29

    WriteRegister(0xE1);        // Register 0xE1: SDRAM CAS Latency
    WriteData(0x03);            // Set CAS latency to 0x03 as suggested by manual

    // Calculate SDRAM refresh interval
    sdram_itv = (SDRAM_CLKFREQ / SDRAM_SIZE) / (1000 / SDRAM_MCLK); // Based on MCLK
    sdram_itv -= 2;

    // Write SDRAM interval (lower byte)
    WriteRegister(0xE2);        // Register 0xE2: SDRAM Refresh Interval Low Byte
    WriteData(sdram_itv);                                               // sdram_itv & 0xFF
    //WriteData(0x1A);            // Set to 0x06 as reference setting by manual for LT7680A-R

    // Write SDRAM interval (upper byte)
    WriteRegister(0xE3);        // Register 0xE3: SDRAM Refresh Interval High Byte
    WriteData(sdram_itv >> 8);                                 // sdram_itv >> 8) & 0xFF
    //WriteData(0x06);            // Set to 0x06 as reference setting by manual for LT7680A-R

    // Step 3: Trigger SDRAM Initialization (Set Bit 0 = 1)
    WriteRegister(0xE4);
    //regValue1 |= (1 << 0);      // Set Bit 0 (SDR_INITDONE)
    //WriteData(regValue1);
    WriteData(0x01);

    // Step 4: Disable Timing Parameter Registers (Clear Bit 2)
    WriteRegister(0xE4);        // Disable, addresses 0xE0 to 0xE3 can no longer be set
    regValue1 &= ~(1 << 2);     // Clear Bit 2
    //regValue1 |= (0 << 2);
    WriteData(regValue1);

    HAL_Delay(1); // 1 ms delay

    Check_SDRAM_Ready_LT();     // Call sub to wait for SDRAM initialization to complete

}


// Check if the SDRAM is ready for use - Address = 0xE4
void Check_SDRAM_Ready_LT() {
    
    unsigned char status;
    
    // Poll the SDRAM Ready Flag (Bit 0) in Register 0xE4
    do {
        WriteRegister(0xE4);
        status = ReadData(); // Read the SDRAM status register
    } while ((status & 0x01) == 0);  // Wait until the Ready bit (bit 0) is set
    
    // Optional delay to ensure SDRAM is stable after initialization
    HAL_Delay(10);  // 10 ms delay

}


// Register 0x01, 0x02, 0x03, 0x012, 0x13
void Set_LCD_Panel_LT() {
    uint8_t temp = 0;

    // Configure Register 0x01: TFT Panel I/F and Host Bus Width
    temp |= (TFT_BIT << 3);         // Set Bit 4-3 to 01b (18-bit TFT Panel I/F)
    temp |= (HOST_BUS << 0);        // Set Bit 0 to 0 (8-bit Host Bus)
    temp |= (0 << 1);               // Set Bit 1 to 0 (Disable SPI Master)
    temp |= (0 << 2);               // Set Bit 2 to 0 (Disable I2C Master)
    temp |= (0 << 5);               // Set Bit 5 to 0 (Disable Keypad-scan)
    temp |= (1 << 6);               // Set Bit 6 to 1 (Mask, WAIT# de-assert when CS# de-assert.)
    WriteRegister(0x01);
    WriteData(temp);

    // Configure Register 0x02: Host Read/Write Image Data Format
    temp = 0;                       // Reset temp for next register
    temp |= (0b00 << 6);            // Set Bit 7-6 to 00b (Direct Write using SPI)
    temp |= (0b00 << 4);            // Set Bit 5-4 to 00b (Read: Left to Right, Top to Bottom)
    temp |= (0b00 << 1);            // Set Bit 2-1 to 00b (Write: Left to Right, Top to Bottom)
    WriteRegister(0x02);
    WriteData(temp);

    // Configure Register 0x03: Graphic Mode and Memory Selection
    temp = 0;                       // Reset temp for next register
    temp |= (0 << 2);               // Set Bit 2 to 1 (Text Mode)
    temp |= (0 << 1);               // Set Bit 1 to 0
    temp |= (0 << 0);               // Set Bit 0 to 0 (Select SDRAM)
    WriteRegister(0x03);
    WriteData(temp);

    // Configure Display Parameters in Register 0x12
    temp = 0;                       // Reset temp for next register
    temp |= (PCLK_EDGE << 7);       // Set Bit 7 to 1 (PCLK Rising Edge)
    temp |= (VSCAN_DIRECTION << 3); // Set Bit 3 to 0 (VSCAN Top to Bottom)
    temp |= PD_OUTPUT_SEQ;          // Set Bits 2-0 to 000 (PDATA RGB Mode)
    //temp |= (0b000);
    WriteRegister(0x12);
    WriteData(temp);

    // Configure Display Parameters in Register 0x13
    temp = 0;                       // Reset temp for next register
    temp |= (HSYNC_ACTIVE << 7);    // Set Bit 7 to 0 (HSYNC Low Active)
    temp |= (VSYNC_ACTIVE << 6);    // Set Bit 6 to 0 (VSYNC Low Active)
    temp |= (DE_ACTIVE << 5);       // Set Bit 5 to 0 (DE High Active)
    temp |= (PDE_IDLE_STATE << 4);  // Bit 4
    temp |= (PCLK_IDLE_STATE << 3); // Bit 3
    temp |= (PD_IDLE_STATE << 2);   // Bit 2
    temp |= (HSYNC_IDLE_STATE << 1);// Bit 1
    temp |= (VSYNC_IDLE_STATE << 0);// Bit 0
    WriteRegister(0x13);
    WriteData(temp);

}


// Register 0x14, 0x15, 0x1A, 0x1B
void LCD_HorizontalWidth_VerticalHeight_LT(uint16_t WX, uint16_t HY) {             // 320x960 pixels

    uint16_t tempWidth = (WX / 8) - 1;      // Horizontal Display Width (pixels) = (HDWR + 1) * 8 + HDWFTR
    uint16_t tempWidthFineTune = 0;

    // Horizontal Width
    WriteRegister(0x14);
    WriteData(tempWidth);
    WriteRegister(0x15);
    WriteData(tempWidthFineTune & 0x0F);    // Bits 0-3 only
    TEST1 = tempWidth;
    TEST2 = tempWidthFineTune & 0x0F;

    // Vertical Height
    // Ensure height is within valid range
    if (HY > 1024) {
        HY = 1024; // Maximum valid height
    }
    if (HY < 1) {
        HY = 1; // Minimum valid height
    }
    uint16_t vdhr = HY - 1;             // Subtract 1 from the height as per the formula: VDHR = Vertical Display Height - 1
    // Write the lower 8 bits of VDHR to Register 0x1A
    WriteRegister(0x1A);
    WriteData(vdhr & 0xFF); // Lower 8 bits of VDHR
    // Write the upper 3 bits of VDHR to Register 0x1B
    WriteRegister(0x1B);
    WriteData(vdhr >> 8); // Upper 3 bits of VDHR (bits 10-8)
    TEST3 = (vdhr & 0xFF);
    TEST4 = (vdhr >> 8);

}


// Register 0x16, 0x17
void LCD_Horizontal_Non_Display_LT(uint16_t val) {

    uint16_t tempHBPD = (val / 8) - 1;      // Horizontal Display Width (pixels) = (HDWR + 1) * 8 + HDWFTR
    uint16_t tempHBPDFineTune = 0;

    // Horizontal HBPD
    WriteRegister(0x16);
    WriteData(tempHBPD);
    WriteRegister(0x17);
    WriteData(tempHBPDFineTune & 0x0F);    // Bits 0-3 only
    TEST5 = tempHBPD;
    TEST6 = tempHBPDFineTune & 0x0F;

}


// Register 0x18
void LCD_HSYNC_Start_Position_LT(uint16_t val) {

    uint16_t tempHFPD;

    if (val < 8) {
        tempHFPD = 0;  // Minimum valid value
    }
    else {
        tempHFPD = (val / 8) - 1;  // HSYNC Start Position = (HSTR + 1)* 8
    }

    // Write the calculated value to the register
    WriteRegister(0x18);
    WriteData(tempHFPD);
    TEST7 = tempHFPD;

}


// Register 0x19
void LCD_HSYNC_Pulse_Width_LT(uint16_t val) {

    uint16_t tempHSPW;

    if (val < 8) {
        tempHSPW = 0;  // Minimum valid value
    }
    else {
        tempHSPW = (val / 8) - 1;  // HSYNC Start Position = (HSTR + 1)* 8
    }

    // Write the calculated value to the register
    WriteRegister(0x19);
    WriteData(tempHSPW);
    TEST8 = tempHSPW;
}


// Register 0x1C, 0x1D
void LCD_Vertical_Non_Display_LT(uint16_t val) {

    uint8_t temp = val - 1;
    WriteRegister(0x1C);
    WriteData(temp & 0xFF);
    WriteRegister(0x1D);
    WriteData((temp >> 8) & 0xFF);
    TEST9 = temp & 0xFF;
    TEST10 = (temp >> 8) & 0xFF;
}


// Register 0x1E
void LCD_VSYNC_Start_Position_LT(uint16_t val) {

    uint8_t temp = val - 1;
    WriteRegister(0x1E);
    WriteData(temp & 0xFF);
    TEST11 = temp & 0xFF;
}


// Register 0x1F
void LCD_VSYNC_Pulse_Width_LT(uint16_t val) {

    uint8_t temp = val - 1;
    WriteRegister(0x1F);
    WriteData(temp & 0xFF);
    TEST12 = temp & 0xFF;
}


// Register 0x12
void LCDConfigTurnOn_LT() {                                                 // OK
    uint8_t regValue = 0;

    // Bit 7: PCLK Inversion
    // 0: TFT Panel fetches PD at PCLK rising edge
    // 1: TFT Panel fetches PD at PCLK falling edge
    regValue |= (0 << 7); // Set to 0 (default to rising edge, adjust if necessary)

    // Bit 6: Display ON/OFF
    regValue |= (1 << 6); // Set to 1 (Display On)

    // Bit 5: Display Test Color Bar
    regValue |= (DISP_TEST << 5); // Set to 1 (Enable Color Bar for testing, adjust if needed)

    // Bit 4: Reserved
    regValue |= (0 << 4); // Set to 0 as per specification

    // Bit 3: VDIR - Vertical Scan Direction
    // 0: From Top to Bottom
    // 1: From Bottom to Top
    regValue |= (1 << 3); // Default to 0 (Top to Bottom, adjust if necessary)

    // Bits 2-0: Parallel PD[23:0] Output Sequence
    // 000: RGB, 001: RBG, 010: GRB, 011: GBR, 100: BRG, 101: BGR, 110: Gray, 111: Idle State
    regValue |= (OUTPUT_SEQ); // Set to RGB (default, adjust if necessary)

    // Write the value to Register 0x12
    WriteRegister(0x12);
    WriteData(regValue);
}


void ConfigurePWMAndSetBrightness(uint8_t brightnessPercentage) {

    // Configure Timer - 1 and PWM - 1 for backlighting.
    // Settable 0 - 100 %

    if (brightnessPercentage > 100) brightnessPercentage = 100; // Cap brightness to 100%

    // Step 1: Set the Prescaler (REG[84h])
    // Core Frequency: 54MHz -> Base Frequency = Core_Freq / (Prescaler + 1)
    // Example: Prescaler = 16 -> Base Frequency = 54MHz / (16 + 1) = ~3.18MHz
    WriteRegister(0x84);
    WriteData(0x10); // Prescaler = 16

    // Step 2: Configure PWM Clock Mux Register (REG[85h])
    // Timer-1 divisor = 1/4, PWM[1] = Timer-1 events
    WriteRegister(0x85);
    WriteData((2 << 6) | // Timer-1 divisor = 1/4
        (2 << 2)); // PWM[1] output Timer-1 events

    // Step 3: Calculate ON and OFF times for Timer-1
    // Count Buffer = Total period, Compare Buffer = ON time
    uint16_t compareValue = (brightnessPercentage * 255) / 100; // ON time
    uint16_t countValue = 255; // Fixed total period

    // Step 4: Set Compare Buffer for Timer-1 (REG[8Ch-8Dh])
    WriteRegister(0x8C); // Timer-1 Compare Buffer (low byte)
    WriteData(compareValue & 0xFF);
    WriteRegister(0x8D); // Timer-1 Compare Buffer (high byte)
    WriteData((compareValue >> 8) & 0xFF);

    // Step 5: Set Count Buffer for Timer-1 (REG[8Eh-8Fh])
    WriteRegister(0x8E); // Timer-1 Count Buffer (low byte)
    WriteData(countValue & 0xFF);
    WriteRegister(0x8F); // Timer-1 Count Buffer (high byte)
    WriteData((countValue >> 8) & 0xFF);

    // Step 6: Enable Timer-1 (REG[86h])
    // Auto-reload enabled, Timer-1 started, no inversion
    WriteRegister(0x86);
    WriteData((1 << 5) | (1 << 4)); // Auto-reload and Start
}


// Register 0x00
void Software_Reset_LT() {                                      // OK - needs verified
    uint8_t regValue = 0;

    // Bit 7: Reconfigure PLL Frequency
    // Set this to 1 to reconfigure the PLL frequency
    // 1. When user change PLL relative parameters, PLL clock won‟t 
    // change immediately, user must set this bit as “1” again. 
    // 2. User may read (check) this bit to know whether system already 
    // switch to PLL clock or not yet. 
    regValue |= (0 << 7);

    // Bit 6-1: Reserved, keep as 0
    //regValue |= (0 << 1);

    // Bit 0: Software Reset (Write-only)
    // Set this to 1 to perform a software reset
    regValue |= (1 << 0);

    // Write the configured value to Register 0x00
    WriteDataToRegister(0x00, regValue);

    // Optional: Add a delay to allow reset/reconfiguration to complete
    //HAL_Delay(10);

    // Optional: Clear Bit 7 and Bit 0 (reset finished)
    //regValue &= ~(1 << 7); // Clear Bit 7
    //regValue &= ~(1 << 0); // Clear Bit 0
    //WriteDataToRegister(0x00, regValue);
}


// Register 0x00
void Software_ResetPLL_LT() {                                      // OK - needs verified
    uint8_t regValue = 0;

    // Bit 7: Reconfigure PLL Frequency
    // Set this to 1 to reconfigure the PLL frequency
    // 1. When user change PLL relative parameters, PLL clock won‟t 
    // change immediately, user must set this bit as “1” again. 
    // 2. User may read (check) this bit to know whether system already 
    // switch to PLL clock or not yet. 
    regValue |= (1 << 7);

    // Write the configured value to Register 0x00
    WriteDataToRegister(0x00, regValue);

    // Optional: Clear Bit 7 and Bit 0 (reset finished)
    //regValue &= ~(1 << 7); // Clear Bit 7
    //regValue &= ~(1 << 0); // Clear Bit 0
    //WriteDataToRegister(0x00, regValue);
}


// Registers 0x20 to 0x23
void Set_MISA_LT() {
    
    // Hardcoded Main Image Start Address = 0x00000000
    WriteRegister(0x20); WriteData(0x00);  // MISA[7:0], ensure bit[1:0] = 0
    WriteRegister(0x21); WriteData(0x00);  // MISA[15:8]
    WriteRegister(0x22); WriteData(0x00);  // MISA[23:16]
    WriteRegister(0x23); WriteData(0x00);  // MISA[31:24]
     
    /*
    // Ensure bit[1:0] are 0 (align to 4-byte boundary)
    uint32_t address = MAIN_IMAGE_START;

    // Write MISA to LT7680 registers
    WriteRegister(0x20);
    WriteData((uint8_t)(address & 0xFF));        // Lower byte (MISA[7:0])

    WriteRegister(0x21);
    WriteData((uint8_t)((address >> 8) & 0xFF)); // Second byte (MISA[15:8])

    WriteRegister(0x22);
    WriteData((uint8_t)((address >> 16) & 0xFF)); // Third byte (MISA[23:16])

    WriteRegister(0x23);
    WriteData((uint8_t)((address >> 24) & 0xFF)); // Upper byte (MISA[31:24])
    */
}


// Register 0x24, 0x25
void SetMainImageWidth_LT() {

    // Set to 
    WriteRegister(0x24);
    WriteData(LCD_XSIZE_TFT & 0xFF);
    WriteRegister(0x25);
    WriteData((LCD_XSIZE_TFT >> 8) & 0x1F);  // Mask to 5 bits
    TEST13 = LCD_XSIZE_TFT & 0xFF;
    TEST14 = (LCD_XSIZE_TFT >> 8) & 0x1F;

}


// Register 0x26, 0x27
void SetMainWindowUpperLeftX_LT() {
    uint16_t xCoord = 0;  // Upper-left corner X-coordinate (0 for the left edge of the display)
    uint8_t lowByte, highByte;

    // Split the X-coordinate into low and high bytes
    lowByte = xCoord & 0xFC;         // Mask out bit[1:0] to ensure they are 0
    highByte = (xCoord >> 8) & 0x1F; // Only use bits[12:8] and ignore bits[7:5]

    // Write to MWULX registers
    WriteRegister(0x26);
    WriteData(lowByte);            // MWULX[7:0]
    WriteRegister(0x27);
    WriteData(highByte);           // MWULX[12:8]
    TEST15 = lowByte;
    TEST16 = highByte;
}


// Registers 0x28, 0x29
void SetActiveWindow_LT() {
    // Main Window Horizontal Start = 0, End = 319
    WriteRegister(0x28); WriteData(0x00);  // Start X Low Byte
    WriteRegister(0x29); WriteData(0x00);  // Start X High Byte

}


// Register 0x5E
void SetColorDepth_LT() {
    uint8_t value = 0;

    // Hardcoded settings for AW_COLOR (Register 0x5E):
    // Bit 7-4: Not used (set to 0).
    // Bit 3: 0 (Read back Graphic Write position).
    // Bit 2: 0 (Block mode - X-Y coordinate addressing).
    // Bits 1-0: 01 (16bpp RGB565).

    value |= (0 << 3);  // Bit 3: Graphic Write position
    value |= (0 << 2);  // Bit 2: Block mode
    value |= (0 << 1);  // Bit 1: 16bpp
    value |= (1 << 0);  // Bit 0: RGB565 (16bpp)

    // Write the value to the AW_COLOR register
    WriteRegister(0x5E);
    WriteData(value);
}


void ResetGraphicWritePosition_LT() {
    // Set Graphic Write X-Coordinate to 0 (lower 8 bits)
    WriteRegister(0x5F);
    WriteData(0x00);

    // Set Graphic Write X-Coordinate to 0 (upper 5 bits)
    WriteRegister(0x60);
    WriteData(0x00);
}


void SetGraphicRWYCoordinate_LT() {
    uint16_t y = 0;  // Hardcoded to 0 for the initial Y-coordinate

    // Write to REG[61h] (Lower byte of Y-coordinate)
    WriteRegister(0x61);
    WriteData(y & 0xFF);  // Lower 8 bits of the Y-coordinate

    // Write to REG[62h] (Upper byte of Y-coordinate)
    WriteRegister(0x62);
    WriteData((y >> 8) & 0x1F);  // Bits [12:8], masked to 5 bits
}


void SetCanvasStartAddress_LT() {
    uint32_t startAddress = 0x00000000;  // Hardcoded to 0 (start of SDRAM)

    WriteRegister(0x50);
    WriteData(startAddress & 0xFF);         // Lower byte (CVSSA[7:0])

    WriteRegister(0x51);
    WriteData((startAddress >> 8) & 0xFF);  // Middle byte (CVSSA[15:8])

    WriteRegister(0x52);
    WriteData((startAddress >> 16) & 0xFF); // Upper byte (CVSSA[23:16])

    WriteRegister(0x53);
    WriteData((startAddress >> 24) & 0xFF); // Highest byte (CVSSA[31:24])

}


void SetCanvasImageWidth_LT() {
    WriteRegister(0x54);
    WriteData(LCD_XSIZE_TFT & 0xFF);          // Lower byte (CVS_IMWTH[7:0])

    WriteRegister(0x55);
    WriteData((LCD_XSIZE_TFT >> 8) & 0x3F);   // Upper byte (CVS_IMWTH[13:8]), masked to 6 bits
}

