#ifndef MENUFUNCTIONS_H
#define MENUFUNCTIONS_H

#include "Plant_Drizzler.h"

void updateOLED(bool forced);
void changeMenuState(int newState);


void drawTemperature();
void drawStartScreen();
void drawPressure();
void drawMoisture();
void drawLighting();


#endif /*MENUFUNCTIONS_H*/