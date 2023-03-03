#ifndef SPRAYFUNCTIONS_H
#define SPRAYFUNCTIONS_H

#include <Arduino.h>
#include "functions.h"
#include "Toilet_Drizzler_9000.h" // to enable/disable the Toilet Drizzler 9000


void sprayLoop(unsigned long curTime);
void startSpray(int amount);
void startSpray();
void cancelSprays();
void sprayButtonUpdate(bool pressed);


#endif /*SPRAYFUNCTIONS_H*/
