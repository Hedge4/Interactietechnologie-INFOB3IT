#ifndef DEVICEFUNCTIONS_H
#define DEVICEFUNCTIONS_H

#include <Arduino.h>
// TODO add libraries


void deviceLoop(unsigned long curTime);
void setSpraysShort(int newAmount);
void setSpraysLong(int newAmount);

int temperature();
bool deviceIsIdle();


#endif /*DEVICEFUNCTIONS_H*/
