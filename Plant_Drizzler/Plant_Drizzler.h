#ifndef PLANT_DRIZZLER_H
#define PLANT_DRIZZLER_H

#include "menuFunctions.h"
#include "sensorFunctions.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <Servo.h>
#include <BlockNot.h> 

extern Adafruit_SSD1306 display;
extern Servo myservo;
extern Adafruit_BMP280 bmp;

extern int ldrReading;
extern int moistReading;
extern float tempReading;
extern float pressureReading;
extern int lightLevel;
extern int moistLevel;

extern int selPin;
extern int aPin;
  
extern BlockNot ldrInterval;          
extern BlockNot moistInterval;
extern BlockNot bmpInterval;
extern int moistReadBuffer;

extern BlockNot oledRefreshRate;
extern BlockNot changeMenuInterval;

extern int menuState;

#endif /*PLANT_DRIZZLER_H*/