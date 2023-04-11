#ifndef MENUFUNCTIONS_H
#define MENUFUNCTIONS_H



#include "Plant_Drizzler.h"

void oledSetup();

void updateOLED();

void drawStartFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawAmuxFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawBmpFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawTreatmentFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);

void watercanOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);


#endif /*MENUFUNCTIONS_H*/