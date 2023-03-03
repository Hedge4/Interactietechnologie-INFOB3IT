#ifndef DEVICEFUNCTIONS_H
#define DEVICEFUNCTIONS_H

#include <Arduino.h>


void deviceLoop(unsigned long curTime);
void setSpraysShort(int newAmount);
void setSpraysLong(int newAmount);
bool deviceIsIdle();


#endif /*DEVICEFUNCTIONS_H*/
