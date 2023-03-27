#ifndef PLANT_DRIZZLER_H
#define PLANT_DRIZZLER_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <Servo.h>
#include <BlockNot.h> 

#include "sensorFunctions.h"
#include "menuFunctions.h"

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


//servo class
class Arm {
  Servo myservo;                              //servo reference
  int movingTowards;                          //position servo is currently moving towards
  void setupArm();                            //when booting up move servo to start position
  int pos;                                    //current position of servo
  int startPos;                               //neutral position of servo
  int waterPos;                               //position where water is flowing
  public:
    bool available;                           //determines if servo can be called or not
    BlockNot moveDelay;                       //small delay between movements
    Arm(int startPosition, int endPosition);  //constructor
    void moveArm(int position);               //issue move commands to servo
    void update();                            //handles slow movement of servo
    void toggleArm(bool t);                   //if true, attaches arm. if false, detach arm
};



#endif /*PLANT_DRIZZLER_H*/