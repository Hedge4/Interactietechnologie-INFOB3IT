#ifndef MENUFUNCTIONS_H
#define MENUFUNCTIONS_H



#include "Plant_Drizzler.h"

void oledSetup();

void updateOLED(bool forced);
void changeMenuState(int newState);

void drawStartFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawAmuxFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawBmpFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawTreatmentFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);

void watercanOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);

/*
void drawTemperature();
void drawStartScreen();
void drawPressure();
void drawMoisture();
void drawLighting();
void drawTime();
void drawWateringScreen();
*/

void toggleCarousel(bool mode);

#endif /*MENUFUNCTIONS_H*/