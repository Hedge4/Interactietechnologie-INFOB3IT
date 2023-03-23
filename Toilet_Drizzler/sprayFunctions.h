#ifndef SPRAYFUNCTIONS_H
#define SPRAYFUNCTIONS_H

#include <Arduino.h>
#include "functions.h"
#include "menuFunctions.h"
#include "Toilet_Drizzler.h" // to enable/disable the Toilet Drizzler 9000


void sprayLoop(unsigned long curTime);
void startSpray();
void startSpray(int amount);
void startSpray(int amount, long waitUntilSpray);
void cancelSprays();
void sprayButtonUpdate(bool pressed);
void spraysLeftSetup(int eeAddress, int value);

// getters
extern bool sprayComing();


#endif /*SPRAYFUNCTIONS_H*/
