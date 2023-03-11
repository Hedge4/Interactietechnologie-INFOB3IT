#ifndef DEVICEFUNCTIONS_H
#define DEVICEFUNCTIONS_H

#include <Arduino.h>
#include "Toilet_Drizzler_9000.h"
// TODO add libraries


void deviceLoop(unsigned long curTime);
void setSpraysShort(int newValue);
void setSpraysLong(int newValue);
void setSpraysShortDelay(unsigned long newValue);
void setSpraysLongDelay(unsigned long newValue);

// getters
bool deviceIsIdle();
int temperature();
String deviceStateString();


#endif /*DEVICEFUNCTIONS_H*/
