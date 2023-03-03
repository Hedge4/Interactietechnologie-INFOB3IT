#ifndef CLASSES_H
#define CLASSES_H

#include <Arduino.h>
#include "functions.h"


// button class
class Knop {
    int lowVolt;
    int highVolt;
    int buttonState;
    int lastButtonState = HIGH;

    unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
    unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

  public:
    Knop(int l, int h);
    bool pressed = false;
    bool changed = false;
    void update(int volt, unsigned long curTime);
};


#endif /*CLASSES_H*/
