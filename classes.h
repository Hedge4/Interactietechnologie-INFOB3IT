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
  bool active = true;     //determines whether sensor will be read
  int senseInterval;      //determines interval of readings
  int lastReading = 0;    //saves last measurement sensor has taken 
  int lastSensed = 0;     //saves when last  measurement was taken
  bool changed = false;   //saves if sensors measurement changed this tick
};

//distancesensor
class DistanceSensor : public Sensor {
public:     
  int readSensitivity = 10;   //only update lastReading when reading differs by at least readSensitivity
  //THESE logic should go to devicefunctions:
  int closeByThreshold;       //distinguishes between person closeby for toilet use
  int farAwayThreshold;       //and far away for when person leaves the toilet
  DistanceSensor(int interval);
  void update(unsigned long curTime);
};

//lightsensor
class LightSensor : public Sensor {
public:
  int readSensitivity = 30;   //only update lastReading when reading differs by at least readSensitivity
  LightSensor(int interval);
  void update(unsigned long curTime);
};


class MotionSensor : public Sensor {
public:
  unsigned long motionsSensed; //keep track of how much motion is being sensed. If high, likely cleaning 
  MotionSensor(int interval);
  void update(unsigned long curTime);  
  void resetSensor();
};

class TemperatureSensor : public Sensor {
public:
  TemperatureSensor(int interval);
  void update(unsigned long curTime);
};


#endif /*CLASSES_H*/
