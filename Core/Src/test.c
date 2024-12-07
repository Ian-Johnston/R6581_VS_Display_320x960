/**
  ******************************************************************************
  * @file    test.c
  * @brief   This file provides code for the testing
  *          of the system
  ******************************************************************************
  Testing routines only
  This is just a group of test code I was using during development, some of them
  utter rubbish, some of them quite good wee routines.

*/

/* Includes ------------------------------------------------------------------*/
//#include "test.h"
//#include "spi.h"
//#include "main.h"
//#include "lcd.h"
//#include "lt7680.h"

//volatile uint32_t TESTT1 = 0;

/*

RGB565 (16bpp) Color info:

Primary Colors
uint16_t red = 0xF800;    // Pure Red
uint16_t green = 0x07E0;  // Pure Green
uint16_t blue = 0x001F;   // Pure Blue

Secondary Colors
uint16_t yellow = 0xFFE0;   // Yellow (Red + Green)
uint16_t cyan = 0x07FF;     // Cyan (Green + Blue)
uint16_t magenta = 0xF81F;  // Magenta (Red + Blue)

Grayscale
uint16_t black = 0x0000;  // Black
uint16_t white = 0xFFFF;  // White
uint16_t gray = 0x8410;   // Gray (50% brightness)

Shades of Red
uint16_t lightRed = 0xFC10;  // Light Red
uint16_t darkRed = 0x7800;   // Dark Red

Shades of Green
uint16_t lightGreen = 0x07EF;  // Light Green
uint16_t darkGreen = 0x03E0;   // Dark Green

Shades of Blue
uint16_t lightBlue = 0x1C7F;  // Light Blue
uint16_t darkBlue = 0x0010;   // Dark Blue

Custom Colors
uint16_t orange = 0xFD20;    // Orange
uint16_t pink = 0xF81F;      // Pink
uint16_t brown = 0x4A69;     // Brown
uint16_t lightYellow = 0xFFF0;  // Light Yellow
uint16_t darkGray = 0x4208;  // Dark Gray
uint16_t skyBlue = 0x5D9B;   // Sky Blue
uint16_t purple = 0x8010;    // Purple
uint16_t limeGreen = 0x87F0; // Lime Green



void FillSDRAM(unsigned short color) {
    unsigned long sdram_addr;
    unsigned long total_memory = 128 * 1024 * 1024; // Example: 128Mbit SDRAM
    unsigned long pixel_count = total_memory / 2;  // RGB565 = 2 bytes per pixel

    // Set SDRAM base address
    WriteRegister(0xE0); // Base Address Low
    WriteData(0x00);
    HAL_Delay(1);
    WriteRegister(0xE1); // Base Address High
    WriteData(0x00);

    // Fill the SDRAM with test data
    for (sdram_addr = 0; sdram_addr < pixel_count; sdram_addr++) {
        // Set the SDRAM write address
        WriteDataToRegister(0xE2, (sdram_addr & 0xFF));        // Low byte
        WriteDataToRegister(0xE3, ((sdram_addr >> 8) & 0xFF)); // Middle byte
        WriteDataToRegister(0xE4, ((sdram_addr >> 16) & 0xFF)); // High byte

        // Write the data to the specified address
        WriteData(color); // Write test color (e.g., 0xF800 for red)
    }

}

void TEST_Fill_initiate() {
    uint32_t length = LCD_XSIZE_TFT * LCD_YSIZE_TFT * 2;  // Total bytes for 16bpp
    uint16_t color = 0xF800;  // Red in RGB565
    TEST_SDRAM_Fill_LT(MAIN_IMAGE_START, length, color);
}


void TEST_SDRAM_Fill_LT(uint32_t startAddress, uint32_t length, uint16_t color) {
    uint32_t currentAddress;

    // Loop through the specified range and write the color value
    for (currentAddress = startAddress; currentAddress < (startAddress + length); currentAddress += 2) {
        // Set the current SDRAM address
        WriteRegister(0x20);                  // Main Image Start Address Low Byte
        WriteData((uint8_t)(currentAddress & 0xFF));
        WriteRegister(0x21);                  // Main Image Start Address Middle Byte
        WriteData((uint8_t)((currentAddress >> 8) & 0xFF));
        WriteRegister(0x22);                  // Main Image Start Address High Byte
        WriteData((uint8_t)((currentAddress >> 16) & 0xFF));

        // Write the 16-bit color value (2 bytes per write)
        WriteRegister(0x23);                  // Write data to SDRAM
        WriteData((uint8_t)(color & 0xFF));   // Lower byte
        WriteData((uint8_t)(color >> 8));     // Upper byte
    }
}


void ClearSDRAM() {
    uint32_t currentAddress;

    // SDRAM clear for 320x960 resolution, 16bpp (2 bytes per pixel)
    for (currentAddress = 0x000000; currentAddress < (320 * 960 * 2); currentAddress += 2) {
        // Set the SDRAM address
        WriteRegister(0x20); WriteData((uint8_t)(currentAddress & 0xFF));       // MISA[7:0]
        WriteRegister(0x21); WriteData((uint8_t)((currentAddress >> 8) & 0xFF)); // MISA[15:8]
        WriteRegister(0x22); WriteData((uint8_t)((currentAddress >> 16) & 0xFF)); // MISA[23:16]

        // Write black (0x0000) to SDRAM
        WriteRegister(0x23);
        WriteData(0x00);  // Lower byte of black
        WriteData(0x00);  // Upper byte of black
    }
}


void SetActiveWindowFullScreen() {
    WriteRegister(0x56); WriteData(0x00); // X-start lower byte
    WriteRegister(0x57); WriteData(0x00); // X-start upper byte
    WriteRegister(0x58); WriteData(0x00); // Y-start lower byte
    WriteRegister(0x59); WriteData(0x00); // Y-start upper byte

    WriteRegister(0x5A); WriteData(320 & 0xFF);  // Width lower byte
    WriteRegister(0x5B); WriteData((320 >> 8) & 0xFF); // Width upper byte
    WriteRegister(0x5C); WriteData(960 & 0xFF);  // Height lower byte
    WriteRegister(0x5D); WriteData((960 >> 8) & 0xFF); // Height upper byte
}


void ConfigurePaletteEntry(uint8_t index, uint8_t red, uint8_t green, uint8_t blue) {
    WriteRegister(0xB0); WriteData(index);  // Set palette index
    WriteRegister(0xB1); WriteData(red);    // Red component
    WriteRegister(0xB2); WriteData(green);  // Green component
    WriteRegister(0xB3); WriteData(blue);   // Blue component
}



void FastClearWithBTE_16bpp(uint16_t color) {
    // Step 1: Set Foreground Color (the color to fill with)
    WriteRegister(0xD2);  // Foreground color low byte
    WriteData(color & 0xFF);
    WriteRegister(0xD3);  // Foreground color high byte
    WriteData((color >> 8) & 0xFF);

    // Step 2: Configure Destination Window (full screen)
    WriteRegister(0xA7); WriteData(0x00);  // Destination X start low byte
    WriteRegister(0xA8); WriteData(0x00);  // Destination X start high byte
    WriteRegister(0xA9); WriteData(0x00);  // Destination Y start low byte
    WriteRegister(0xAA); WriteData(0x00);  // Destination Y start high byte
    WriteRegister(0xAB); WriteData((LCD_XSIZE_TFT - 1) & 0xFF);  // X end low byte
    WriteRegister(0xAC); WriteData(((LCD_XSIZE_TFT - 1) >> 8) & 0xFF);  // X end high byte
    WriteRegister(0xAD); WriteData((LCD_YSIZE_TFT - 1) & 0xFF);  // Y end low byte
    WriteRegister(0xAE); WriteData(((LCD_YSIZE_TFT - 1) >> 8) & 0xFF);  // Y end high byte

    // Step 3: Set BTE Operation Code to Solid Fill
    WriteRegister(0x91);
    WriteData(0x0C);  // 1100b: Solid Fill without ROP

    // Step 4: Enable BTE
    WriteRegister(0x90);
    WriteData(0x80);  // Bit 7: Start BTE operation

    // Step 5: Wait for the operation to complete
    while (ReadStatus() & (1 << 3)) {
        // Bit 3 in status register indicates BTE busy
    }
}



void BTE_ClearArea_Test_16bpp(void) {
    // Step 1: Set Foreground Color (RGB565 Red as an example)
    uint16_t color = 0xF800; // Red in RGB565
    WriteRegister(0xD2); // Foreground color low byte
    WriteData(color & 0xFF);
    WriteRegister(0xD3); // Foreground color high byte
    WriteData((color >> 8) & 0xFF);

    // Step 2: Set Destination Window (middle of a 320x960 screen)
    uint16_t startX = 110;
    uint16_t startY = 430;
    uint16_t width = 100;
    uint16_t height = 100;

    WriteRegister(0xA7); WriteData(startX & 0xFF); // Destination X start low byte
    WriteRegister(0xA8); WriteData((startX >> 8) & 0xFF); // Destination X start high byte
    WriteRegister(0xA9); WriteData(startY & 0xFF); // Destination Y start low byte
    WriteRegister(0xAA); WriteData((startY >> 8) & 0xFF); // Destination Y start high byte
    WriteRegister(0xAB); WriteData((startX + width - 1) & 0xFF); // X end low byte
    WriteRegister(0xAC); WriteData(((startX + width - 1) >> 8) & 0xFF); // X end high byte
    WriteRegister(0xAD); WriteData((startY + height - 1) & 0xFF); // Y end low byte
    WriteRegister(0xAE); WriteData(((startY + height - 1) >> 8) & 0xFF); // Y end high byte

    // Step 3: Set BTE Operation Code (Solid Fill without ROP)
    WriteRegister(0x91);
    WriteData(0x0C); // 1100b: Solid Fill without ROP

    // Step 4: Enable BTE Operation
    WriteRegister(0x90);
    WriteData(0x80); // Start the BTE operation (Bit 7 = 1)

    // Step 5: Wait for BTE to complete
    while (ReadStatus() & (1 << 3)) {
        // Bit 3: BTE busy (wait until cleared)
    }
}



void FillSDRAMWithColor() {         // works
    uint32_t length = 320 * 960;  // Total pixels for 320x960 resolution
    uint16_t color = 0xFFFF;      // Solid color in RGB565 format
    //uint16_t color = 0xF800;      // Solid color in RGB565 format - Red
    //uint16_t color = 0x07E0;      // Solid color in RGB565 format - Green
    //uint16_t color = 0x001F;      // Solid color in RGB565 format - Blue

    // Set to graphics mode
    uint8_t temp = 0;
    // Set Bits 1-0 to 00b (Display RAM)
    temp &= ~(0b11);  // Clear Bits 1-0
    temp |= 0b00;     // Set to Display RAM
    // Set Bit 2 to 0 (Graphic Mode)
    temp &= ~(1 << 2);
    WriteRegister(0x03);
    WriteData(temp);

    // Directly fill SDRAM via the memory data port (0x04)
    for (uint32_t i = 0; i < length; i++) {
        WriteRegister(0x04);        // Memory Write
        WriteData(color >> 8);      // High byte of the solid color
        WriteData(color & 0xFF);    // Low byte of the solid color
    }
}




void DrawSmallSquare() {
    uint16_t color = 0xF800; // Red
    uint16_t squareSize = 50; // Square dimensions
    uint16_t startX = 100;    // Calculated to keep square away from edges
    uint16_t startY = 100;

    uint32_t startAddress = (startY * 320 + startX) * 2; // Convert to SDRAM address
    uint32_t rowOffset = 320 * 2; // Bytes per row in memory

    for (uint16_t y = 0; y < squareSize; y++) {
        uint32_t rowAddress = startAddress + y * rowOffset;
        for (uint16_t x = 0; x < squareSize; x++) {
            uint32_t pixelAddress = rowAddress + x * 2;

            // Write the color to the calculated memory address
            WriteRegister(0x20); WriteData(pixelAddress & 0xFF);
            WriteRegister(0x21); WriteData((pixelAddress >> 8) & 0xFF);
            WriteRegister(0x22); WriteData((pixelAddress >> 16) & 0xFF);
            WriteRegister(0x23);

            // Write the color
            WriteData(color & 0xFF);    // Low byte
            WriteData(color >> 8);      // High byte
        }
    }
}


void WriteSinglePixel() {
    uint16_t color = 0xF800; // Red
    uint16_t targetX = 160;  // Middle of the screen
    uint16_t targetY = 480;  // Middle of the screen

    uint32_t pixelAddress = (targetY * 320 + targetX) * 2; // Calculate SDRAM address for the pixel

    // Write the SDRAM address
    WriteRegister(0x20); WriteData(pixelAddress & 0xFF);
    WriteRegister(0x21); WriteData((pixelAddress >> 8) & 0xFF);
    WriteRegister(0x22); WriteData((pixelAddress >> 16) & 0xFF);
    WriteRegister(0x23);

    // Write the color to the address
    WriteData(color & 0xFF);    // Low byte
    WriteData(color >> 8);      // High byte
}




void FillSDRAMWithColorIntegrated() {
    uint8_t value = 0;
    uint32_t currentAddress;
    uint16_t color = 0x0000;  // Change this value to set the fill color (e.g., 0xF800 for red)
    uint32_t length = 320 * 960 * 2;  // Total bytes for 320x960 resolution at 16bpp

    // Step 1: Disable the display (clear bit 6)
    value &= ~(1 << 6);  // Clear bit 6
    WriteRegister(0x12);
    WriteData(value);

    // Optional delay to stabilize
    HAL_Delay(1);  // 1 ms delay (optional)

    // Step 2: Fill SDRAM with the chosen color
    for (currentAddress = 0x000000; currentAddress < length; currentAddress += 2) {
        WriteRegister(0x20); WriteData((uint8_t)(currentAddress & 0xFF));        // MISA[7:0]
        WriteRegister(0x21); WriteData((uint8_t)((currentAddress >> 8) & 0xFF)); // MISA[15:8]
        WriteRegister(0x22); WriteData((uint8_t)((currentAddress >> 16) & 0xFF)); // MISA[23:16]

        WriteRegister(0x23);
        WriteData((uint8_t)(color & 0xFF));  // Lower byte of color
        WriteData((uint8_t)(color >> 8));    // Upper byte of color
    }

    // Step 3: Enable the display (set bit 6)
    value |= (1 << 6);  // Set bit 6
    WriteRegister(0x12);
    WriteData(value);

    // Optional delay to stabilize the display
    HAL_Delay(10);  // 10 ms delay (optional)
}





void FillDisplayRAM() {
    uint16_t color = 0xF800;  // Red in RGB565
    uint32_t length = 614400; // Total bytes for 320x960 resolution, 16bpp
    uint32_t i;

    // Configure REG[03h][1:0] to select Display RAM (00b)
    WriteRegister(0x03);
    WriteData(0x00);  // Set REG[03h][1:0] = 00b (Display RAM)

    // Fill the Display RAM via REG[04h]
    for (i = 0; i < length / 2; i++) {  // Divide by 2 because each color is 2 bytes
        WriteRegister(0x04);           // REG[04h]: Memory Data Write Port
        WriteData((uint8_t)(color & 0xFF));  // Lower byte of color
        WriteData((uint8_t)(color >> 8));    // Upper byte of color
    }
}



#include <stdint.h>




void OhmsSymbolSaveUCG() {
    #define CGRAM_START_ADDR 0x1000  // Base address for CGRAM

    // 16x32 UCG data for the Ohm (Ω) symbol
        uint8_t ohm[64] = {
        0x00, 0x00,  // Row 1
        0x00, 0x00,  // Row 2
        0x00, 0x00,  // Row 3
        0x07, 0xE0,  // Row 4
        0x1F, 0xF8,  // Row 5
        0x3C, 0x3C,  // Row 6
        0x38, 0x1C,  // Row 7
        0x70, 0x0E,  // Row 8
        0x70, 0x0E,  // Row 9
        0x70, 0x0E,  // Row 10
        0x70, 0x0E,  // Row 11
        0x38, 0x1C,  // Row 12
        0x3C, 0x3C,  // Row 13
        0x1F, 0xF8,  // Row 14
        0x07, 0xE0,  // Row 15
        0x00, 0x00,  // Row 16
        0x00, 0x00,  // Row 17
        0x03, 0xC0,  // Row 18
        0x07, 0xE0,  // Row 19
        0x06, 0x60,  // Row 20
        0x0E, 0x70,  // Row 21
        0x0C, 0x30,  // Row 22
        0x0C, 0x30,  // Row 23
        0x0C, 0x30,  // Row 24
        0x0C, 0x30,  // Row 25
        0x1C, 0x38,  // Row 26
        0x38, 0x1C,  // Row 27
        0x70, 0x0E,  // Row 28
        0x00, 0x00,  // Row 29
        0x00, 0x00,  // Row 30
        0x00, 0x00,  // Row 31
        0x00, 0x00   // Row 32
    };

        uint16_t ucg_address = CGRAM_START_ADDR + (UGC_CODE * 64);  // Calculate UCG address
    for (uint8_t i = 0; i < 64; i++) {
        WriteRegister(ucg_address);
        WriteData(ohm[i]);
    }
}





void loadUCG(uint16_t ucg_code) {
    uint16_t ucg_address = CGRAM_START_ADDR + (ucg_code * 64);  // Calculate UCG address
    for (uint8_t i = 0; i < 64; i++) {
        WriteDataToAddress(ucg_address + i, ohm[i]);  // Write each byte to memory
    }
}

void WriteDataToAddress(uint16_t address, uint8_t data) {
    // Implement the SPI/I2C write or parallel data transfer to the display memory here.
    // Example:
    // SetAddress(address);
    // SendData(data);
}

int main() {
    uint16_t ucg_code = 0x0000;  // First UCG encoding for the Ohm symbol
    loadUCG(ucg_code);
    // After loading, you can use the Ohm symbol in the display
    return 0;
}




void DrawText(const char* text) {                                   // this version will enable detection of ohms symbol and replace with UCG char
    char buffer[64]; // Temporary buffer for writable string
    size_t i = 0;

    WriteRegister(0x04); // Register for writing text

    // Copy text into buffer and replace 'D' with '#'
    while (*text != '\0' && i < sizeof(buffer) - 1) {
        if (*text == '$') {         // the ohms symbol really
            buffer[i++] = '#'; // Replace 'D' with '#'              // the UCG routine will go here
        }
        else {
            buffer[i++] = *text; // Copy character as-is
        }
        text++; // Move to the next character
    }

    buffer[i] = '\0'; // Null-terminate the string

    // Write each character from the buffer to the display
    for (size_t j = 0; buffer[j] != '\0'; j++) {
        WriteData((uint8_t)buffer[j]); // Send each character to the display
    }
}



// Set text cursor position
void SetTextCursor(uint16_t x, uint16_t y) {
    // X-Coordinate
    WriteRegister(0x63); // Lower 8 bits of X position
    WriteData(x & 0xFF);
    WriteRegister(0x64); // Upper 5 bits of X position
    WriteData((x >> 8) & 0x1F);

    // Y-Coordinate
    WriteRegister(0x65); // Lower 8 bits of Y position
    WriteData(y & 0xFF);
    WriteRegister(0x66); // Upper 5 bits of Y position
    WriteData((y >> 8) & 0x1F);
}









// Save UGC symbol to LT7680A-R & display it
void OhmsSymbolStoreUCG() {
    #define CGRAM_START_ADDR 0x1000  // Base address for CGRAM
    #define UGC_CODE 0x0000  // UGC code for OHM symbol

    // 16x32 UCG data for the Ohm (Ω) symbol
    uint8_t ohm[64] = {
    0x00, 0x00,  // Row 1
    0x00, 0x00,  // Row 2
    0x00, 0x00,  // Row 3
    0x07, 0xE0,  // Row 4
    0x1F, 0xF8,  // Row 5
    0x3C, 0x3C,  // Row 6
    0x38, 0x1C,  // Row 7
    0x70, 0x0E,  // Row 8
    0x70, 0x0E,  // Row 9
    0x70, 0x0E,  // Row 10
    0x70, 0x0E,  // Row 11
    0x38, 0x1C,  // Row 12
    0x3C, 0x3C,  // Row 13
    0x1F, 0xF8,  // Row 14
    0x07, 0xE0,  // Row 15
    0x00, 0x00,  // Row 16
    0x00, 0x00,  // Row 17
    0x03, 0xC0,  // Row 18
    0x07, 0xE0,  // Row 19
    0x06, 0x60,  // Row 20
    0x0E, 0x70,  // Row 21
    0x0C, 0x30,  // Row 22
    0x0C, 0x30,  // Row 23
    0x0C, 0x30,  // Row 24
    0x0C, 0x30,  // Row 25
    0x1C, 0x38,  // Row 26
    0x38, 0x1C,  // Row 27
    0x70, 0x0E,  // Row 28
    0x00, 0x00,  // Row 29
    0x00, 0x00,  // Row 30
    0x00, 0x00,  // Row 31
    0x00, 0x00   // Row 32
    };

    uint16_t ucg_address = CGRAM_START_ADDR + (UGC_CODE * 64);  // Calculate UCG address

    // Memory select SDRAM
    unsigned char temp0;
    WriteRegister(0x03);
    temp0 = ReadData();
    temp0 |= (0 << 1);
    temp0 |= (0 << 0);
    WriteRegister(temp0);

    //Setup Canvas Addressing Mode, Linear Mode - REG[5Eh] bit[2] = 1
    WriteRegister(0x5E);
    unsigned short temp1 = 0;
    temp1 = ReadData();
    temp1 |= (1 << 2);
    WriteData(temp1);

    // Select_Write_Data_Position
    // Set Graphic Write X-Coordinate to 0 (lower 8 bits)
    WriteRegister(0x5F);
    WriteData(0x00);
    // Set Graphic Write X-Coordinate to 0 (upper 5 bits)
    WriteRegister(0x60);
    WriteData(0x00);

    // Goto_Linear_Addr - Set the starting address for writing the font data
    uint16_t y = 0;  // Hardcoded to 0 for the initial Y-coordinate
    // Write to REG[61h] (Lower byte of Y-coordinate)
    WriteRegister(0x61);
    WriteData(y & 0xFF);  // Lower 8 bits of the Y-coordinate
    // Write to REG[62h] (Upper byte of Y-coordinate)
    WriteRegister(0x62);
    WriteData((y >> 8) & 0x1F);  // Bits [12:8], masked to 5 bits

    // Write font data
    for (uint8_t i = 0; i < 64; i++) {
        WriteRegister(ucg_address);
        WriteRegister(0x04);
        WriteData(ohm[i]);
    }

    // CGRAM_Start_address
    CGRAM_Start_address();       // 0x0000
    Font_Select_UserDefine_Mode();




    // Now print it out

    Text_Mode();

    // Memory_XY_Mode
    // ???

    // Canvas_Image_Start_address - Canvas address
    WriteRegister(0x50);
    WriteData(0x00);
    WriteRegister(0x51);
    WriteData(0x00);
    WriteRegister(0x52);
    WriteData(0x00);
    WriteRegister(0x53);
    WriteData(0x00);

    // Font_Select_UserDefine_Mode - Set to UCG mode
    ConfigureFontAndPosition(
        0b10,    // User-Defined Font mode
        0b01,    // Font size
        0b00,    // ISO 8859-1
        0,       // Full alignment enabled
        0,       // Chroma keying disabled
        1,       // Rotate 90 degrees counterclockwise
        0b01,    // Width multiplier
        0b01,    // Height multiplier
        5,       // Line spacing
        0,       // Character spacing
        170,     // Cursor X 170
        398      // Cursor Y 480
    );

    // Assign font code
    WriteRegister(0x04);
    WriteData(0x00);    // high byte
    WriteData(0x00);    // low byte


}




















    //HAL_Delay(10);
    Text_Mode();
    //HAL_Delay(10);
    //Set_LCD_Panel_LT();
    HAL_Delay(10);
    SetTextColors(0xFF0000, 0x000000); // Foreground: white, Background: Black
    HAL_Delay(10);
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
        170,     // Cursor X
        60       // Cursor Y
    );
    DrawText("TEST 123");

    HAL_Delay(10);

    SetTextColors(0xFF0000, 0x000000); // Foreground: white, Background: Black
    ConfigureFontAndPosition(
        0b10,    // User-Defined Font mode
        0b10,    // Font size
        0b00,    // ISO 8859-1
        0,       // Full alignment enabled
        0,       // Chroma keying disabled
        1,       // Rotate 90 degrees counterclockwise
        0b10,    // Width multiplier
        0b10,    // Height multiplier
        5,       // Line spacing
        0,       // Character spacing
        10,     // Cursor X 170
        10      // Cursor Y 480
    );
    //DrawText("0");
    // Assign font code to display
    WriteRegister(0x04);
    WriteData(0x00);    // high byte
    WriteData(0x00);    // low byte








            yposohm = 50;
        // The actual OHM
        // Configure CCR0 (REG[CCh])
        uint8_t ccr0 = 0; // Character Control Register 0
        uint8_t ccr1 = 0; // Character Control Register 1
        ccr0 |= ((0b10 & 0b11) << 6);         // Font source selection
        ccr0 |= ((0b10 & 0b11) << 4);    // Character height
        ccr0 |= (0b00 & 0b11);                 // ISO coding
        WriteRegister(0xCC); // Write to CCR0
        WriteData(ccr0);
        // Set Cursor Position
        WriteRegister(0x63); // X lower byte
        WriteData(0 & 0xFF);
        WriteRegister(0x64); // X upper byte
        WriteData((0 >> 8) & 0x1F);
        WriteRegister(0x65); // Y lower byte
        WriteData(yposohm & 0xFF);
        WriteRegister(0x66); // Y upper byte
        WriteData((yposohm >> 8) & 0x1F);
        // Draw the OHM symbol
        WriteRegister(0x04);
        WriteData(0x00);    // high byte
        WriteData(0x00);    // low byte













// MAIN ROW - Print text to LCD
SetTextColors(0xFFFF00, 0x000000); // Foreground: Yellow, Background: Black
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
    70,      // Cursor X
    0        // Cursor Y
);

char MaindisplayString[19] = ""; // String for G[1] to G[18]
for (int i = 1; i <= 18; i++) {
    MaindisplayString[i - 1] = G[i]; // Copy characters
}
MaindisplayString[18] = '\0'; // Null-terminate

// Find the position of the '$' character
char* dollarSign = strchr(MaindisplayString, '$');
if (dollarSign) {
    // Print characters before the '$'
    *dollarSign = '\0'; // Temporarily terminate the string at the '$'
    DrawText(MaindisplayString);



    Text_Mode();

    WriteRegister(0xCC);
    uint8_t temp1 = 0;
    temp1 |= (1 << 7);
    temp1 |= (0 << 6);
    WriteData(temp1);

    // Assign font code to display
    WriteRegister(0x04);
    WriteData(0x00);    // high byte
    WriteData(0x00);    // low byte




    // Back to normal mode
    Text_Mode();


    WriteRegister(0xCC);
    uint8_t temp2 = 0;
    temp2 |= (0 << 7);
    temp2 |= (0 << 6);
    WriteData(temp2);





    // Print characters after the '$'
    DrawText(dollarSign + 1);
}
else {
    // No '$' found, print the entire string
    DrawText(MaindisplayString);
}






















    char MaindisplayString[19] = ""; // String for G[1] to G[18]
    char BeforeDollar[19] = "";     // String before '$'
    char AfterDollar[19] = "";      // String after '$'
    int beforeIndex = 0; // Index for BeforeDollar
    int afterIndex = 0;  // Index for AfterDollar
    int foundDollar = 0; // Flag to track if '$' was found
    for (int i = 1; i <= 18; i++) {
        char currentChar = G[i];
        if (currentChar == '$') {
            // '$' found, switch to building AfterDollar
            foundDollar = 1;
            continue;
        }
        if (!foundDollar) {
            // Build BeforeDollar
            BeforeDollar[beforeIndex++] = currentChar;
        }
        else {
            // Build AfterDollar
            AfterDollar[afterIndex++] = currentChar;
        }
    }
    // Null-terminate the strings
    BeforeDollar[beforeIndex] = '\0';
    AfterDollar[afterIndex] = '\0';
    // Check if a '$' was found
    if (foundDollar) {
        // '$' found: Print the two strings
        DrawText(BeforeDollar);

        Text_Mode();

        WriteRegister(0xCC);
        uint8_t temp1 = 0;
        temp1 |= (1 << 7);
        temp1 |= (0 << 6);
        WriteData(temp1);

        // Assign font code to display
        WriteRegister(0x04);
        WriteData(0x00);    // high byte
        WriteData(0x00);    // low byte

        // Back to normal mode
        Text_Mode();
        WriteRegister(0xCC);
        uint8_t temp2 = 0;
        temp2 |= (0 << 7);
        temp2 |= (0 << 6);
        WriteData(temp2);


        DrawText(AfterDollar);
    }
    else {
        // No '$': Print the original string
        //DrawText(MaindisplayString);
        DrawText("12345678905678");
    }
















*/