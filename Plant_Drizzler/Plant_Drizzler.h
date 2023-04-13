#ifndef PLANT_DRIZZLER_H
#define PLANT_DRIZZLER_H

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Servo.h>
#include <BlockNot.h> 
#include <ButtonDebounce.h>

//oled
#include "SSD1306Wire.h" 
#include "OLEDDisplayUi.h"

//other
#include "sensorFunctions.h"
#include "menuFunctions.h"
#include "mqttConnection.h"

//bmp definitions
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

//command definitions
#define WATER_COMMAND         '0'
#define MORE_WATER_COMMAND    '1'
#define REFRESH_COMMAND       '2'

//toggle definitions
#define TOGGLE_TRUE_PUBLISH   "1"
#define TOGGLE_FALSE_PUBLISH  "0"
#define TOGGLE_TRUE_RECEIVE   '1'
#define TOGGLE_FALSE_RECEIVE  '0'

extern SSD1306Wire display;
extern OLEDDisplayUi ui;

extern Servo myservo;
extern Adafruit_BMP280 bmp;

extern int ldrReading;
extern int moistReading;
extern float tempReading;
extern float pressureReading;
extern int lightLevel;
extern int moistLevel;
extern float lastWatered;

extern int selPin;
extern int aPin;
  
extern BlockNot ldrInterval;          
extern BlockNot moistInterval;
extern BlockNot bmpInterval;
extern int moistReadBuffer;

extern BlockNot forceSensorsInterval;
extern BlockNot moistDebouncing;

extern bool givingWater;

void performCommand(char command);
void performModeToggle(char mode);
void performSensorPing();

//servo class
//TODO: MOVE TO BETTER POSITION
class Arm {
  Servo myservo;                              //servo reference
  int movingTowards;                          //position servo is currently moving towards
  void setupArm();                            //when booting up move servo to start position
  int startPos;                               //neutral position of servo
  int waterPos;                               //position where water is flowing
  public:
    int pos;                                    //current position of servo
    bool available;                           //determines if servo can be called or not
    BlockNot moveDelay;                       //small delay between movements
    Arm(int startPosition, int endPosition);  //constructor
    void moveArm(int position);               //issue move commands to servo
    void update();                            //handles slow movement of servo
    void toggleArm(bool t);                   //if true, attaches arm. if false, detach arm
    void moveToStart();
    void moveToWatering();
};



#endif /*PLANT_DRIZZLER_H*/