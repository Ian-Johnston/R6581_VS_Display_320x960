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

// Function prototypes
void DisplayMain(void);
void DisplayAuxFirstHalf(void);
void DisplayAuxSecondHalf(void);
void DisplayAnnunciatorsHalf(void);


// Settings
#define Xpos_MAIN				102			// These are actually the Y position because LCD is rotated 90deg in use. Values in pixels.
#define Xpos_AUX				200
#define Xpos_ANNUNC				70
#define Xpos_SPLASH				250


#endif // DISPLAY_H