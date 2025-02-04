/**
  ******************************************************************************
  * @file    display.h
  * @brief   This file contains all the function prototypes for
  *          the display.c file
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stddef.h>
#include "main.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

// External global variable
extern char G[48];
extern _Bool Annunc[19];
extern uint32_t MainColourFore;
extern uint32_t AuxColourFore;
extern uint32_t AnnunColourFore;
extern _Bool oneVoltmode;

extern uint32_t LCD_VBPD;
extern uint32_t LCD_VFPD;
extern uint32_t LCD_VSPW;
extern uint32_t LCD_HBPD;
extern uint32_t LCD_HFPD;
extern uint32_t LCD_HSPW;
extern uint32_t REFRESH_RATE;
extern char ADA_BUY[5];

// Function prototypes
void DisplayMain(void);
void DisplayAuxFirstHalf(void);
void DisplayAuxSecondHalf(void);
void DisplayAnnunciatorsHalf(void);


// Settings suited for 400x960 TFT LCD (320x960 physical)
#define Xpos_MAIN				182			// These are actually the Y position on the R6581 because LCD is rotated 90deg in use. Values in pixels.
#define Xpos_AUX				280
#define Xpos_ANNUNC				150
#define Xpos_SPLASH				330


#endif // DISPLAY_H