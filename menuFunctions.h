#ifndef MENUFUNCTIONS_H
#define MENUFUNCTIONS_H

#include <Arduino.h>
#include <EEPROM.h>

#include "functions.h"
#include "Toilet_Drizzler_9000.h"
#include "deviceFunctions.h"


void menuLoop(unsigned long curTime);
void activateScreen();
void menuButtonUpdate(bool pressed, bool longPress);
void okButtonUpdate(bool pressed);

void spraysShortSetup(int eeAddress, int value);
void spraysLongSetup(int eeAddress, int value);
void spraysShortDelaySetup(int eeAddress, int value);
void spraysLongDelaySetup(int eeAddress, int value);
void spraysLeftSetup(int eeAddress, int value);
void totalSpraysToEEPROM(int newValue);

extern int spraysLeft;
extern const int defaultTotalSprays;
bool menuActive();


#endif /*MENUFUNCTIONS_H*/
