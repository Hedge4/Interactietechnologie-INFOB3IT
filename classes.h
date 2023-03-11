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
  int longPressDelay = 2000;           // how many milliseconds (not) pressed counts as a long input

public:
  Knop(int l, int h);
  bool pressed = false;
  bool longPress = false;
  bool changed = false;
  void update(int volt, unsigned long curTime);
};

//general sensor class
class Sensor {
public:
  bool active;        //determines wether sensor will be read
  int senseInterval;  //determines interval of readings
  int lastSensed = 0;
};

//distancesensor
class DistanceSensor : public Sensor {
public:
  DistanceSensor(int interval);
  int closeByThreshold;  //distinguishes between person closeby for toilet use
  int farAwayThreshold;  //and far away for when person leaves the toilet
  int lastReading;       //saves last measurement sensor has taken
  void update(unsigned long curTime);
};










#endif /*CLASSES_H*/
