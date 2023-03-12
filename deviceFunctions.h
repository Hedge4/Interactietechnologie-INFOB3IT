#ifndef DEVICEFUNCTIONS_H
#define DEVICEFUNCTIONS_H

#include <Arduino.h>
#include "Toilet_Drizzler_9000.h"
#include "classes.h"
#include "sprayFunctions.h"

// methods
void deviceLoop(unsigned long curTime);
void setSpraysShort(int newValue);
void setSpraysLong(int newValue);
void setSpraysShortDelay(unsigned long newValue);
void setSpraysLongDelay(unsigned long newValue);

// getters
bool deviceIsIdle();
int temperature();
String deviceStateString();
extern int yellowLed; // '0' for off, '1' for on, '2' for slow blink, '3' for fast blink
extern int greenLed;


#endif /*DEVICEFUNCTIONS_H*/
