#ifndef DEVICEFUNCTIONS_H
#define DEVICEFUNCTIONS_H

#include <Arduino.h>
#include "Toilet_Drizzler_9000.h"
// TODO add libraries


void deviceLoop(unsigned long curTime);
void setSpraysShort(int newValue);
void setSpraysLong(int newValue);
void setSpraysShortDelay(int newValue);
void setSpraysLongDelay(int newValue);
void spraysShortSetup(int eeAddress, int value);
void spraysLongSetup(int eeAddress, int value);
void spraysShortDelaySetup(int eeAddress, int value);
void spraysLongDelaySetup(int eeAddress, int value);

// getters
bool deviceIsIdle();
int temperature();
String deviceStateString();


#endif /*DEVICEFUNCTIONS_H*/
