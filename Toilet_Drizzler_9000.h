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

// connected hardware
extern LiquidCrystal lcd;
extern const int lcdBacklightPin, ldrPin, ledsPin, sprayPin, motionSensorPin;
extern DallasTemperature dallasTemperatureSensor;
extern NewPing sonar;

extern MotionSensor motionSensor;
extern DistanceSensor distSensor;
extern TemperatureSensor temperatureSensor;
extern LightSensor lightSensor;
extern Knop magneticSensor;

extern const int defaultSprays;


#endif /*TOILET_DRIZZLER_9000_H*/
