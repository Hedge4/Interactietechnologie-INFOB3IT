#ifndef MENUFUNCTIONS_H
#define MENUFUNCTIONS_H

#include <Arduino.h>
#include "functions.h"
#include "Toilet_Drizzler_9000.h"
#include "deviceFunctions.h"


void menuLoop(unsigned long curTime);
void activateScreen();
void menuButtonUpdate(bool pressed);
void okButtonUpdate(bool pressed);

bool menuActive();


#endif /*MENUFUNCTIONS_H*/
