#ifndef TOILET_DRIZZLER_9000_H
#define TOILET_DRIZZLER_9000_H

#include <Arduino.h>

#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NewPing.h>

#include "classes.h"
#include "deviceFunctions.h"
#include "menuFunctions.h"
#include "sprayFunctions.h"


extern LiquidCrystal lcd;
extern const int lcdBacklightPin, ldrPin, motionSensorPin, magneticSensorPin;
extern DallasTemperature sensors;


#endif /*TOILET_DRIZZLER_9000_H*/
