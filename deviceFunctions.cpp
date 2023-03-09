#include "deviceFunctions.h"

/////////////////////////
//    CONFIGURATION    //
/////////////////////////

/* Possible states:
  0 --> 'wait': Bathroom isn't being used, default mode
  1 --> 'detection': Detecting use case
  2 --> 'number_1': ...
  3 --> 'number_2': ...
  4 --> 'cleaning': ...
*/
int deviceState = 0;
unsigned long deviceTimestamp = 0;

int spraysShort, spraysLong;                     // how many sprays after long/short visit
int spraysShortAddr, spraysLongAddr;             // EEPROM addresses
int spraysShortDelay, spraysLongDelay;           // EEPROM addresses
int spraysShortDelayAddr, spraysLongDelayAddr;   // EEPROM addresses

//aanpak wordt om zodra de deur open gaat (en het licht aan)
//een timer wordt gestart die 90 seconden aftelt.
//binnen 90 seconden worden verschillende kenmerken bewaard in bools:
//doorStaysOpen?
//LightStaysOn?
//MotionNearToilet? -> PersonNearbyToilet?/PersonFarAwayOfToilet?

//differentiate between going to the toilet with open door
//and cleaning the toilet by checking motion and distance



/////////////////////////
//      FUNCTIONS      //
/////////////////////////

// setup for when we enter a specific state
void changeDeviceState(int newState) {
  if (deviceState != newState) {
    deviceState = newState;
    deviceTimestamp = millis();

    // state initialisation logic
    switch (newState) {
      case 0:
        break;
      case 1:
        break;
      case 2:
        break;
      case 3:
        break;
      case 4:
        break;
    }
  }
}

//WORK IN PROGRESS

/*
//check light sensor
int lightLevelCheck() {
  int lightReading = analogRead(ldrPin);
  return lightReading;
}

//check magnetic sensor
//if door is closed, magnet will output LOW
bool doorIsOpen(){
  int doorState = digitalRead(magneticSensorPin);
  if(doorState == HIGH) {
    return true;
  } else {
    return false;
  }
}

//check distance, to be implemented
int distanceCheck(){
  return 0;
}


void makeDetection(){
  return;
}
*/

//////////////////////////
//  EXTERNAL FUNCTIONS  //
//////////////////////////

void setSpraysShort(int newValue) {
  spraysShort = newValue;
  // TODO update EEPROM
}

void setSpraysLong(int newValue) {
  spraysLong = newValue;
  // TODO update EEPROM
}

void setSpraysShortDelay(int newValue) {
  spraysShortDelay = newValue;
  // TODO update EEPROM
}

void setSpraysLongDelay(int newValue) {
  spraysLongDelay = newValue;
  // TODO update EEPROM
}

// receive EEPROM address and setup value for the amount of sprays after a short visit
void spraysShortSetup(int eeAddress, int value) {
  spraysShort = value;
  spraysShortAddr = eeAddress;
}

// receive EEPROM address and setup value for the amount of sprays after a long visit
void spraysLongSetup(int eeAddress, int value) {
  spraysLong = value;
  spraysLongAddr = eeAddress;
}

// receive EEPROM address and setup value for how long the device waits before spraying after a short visit
void spraysShortDelaySetup(int eeAddress, int value) {
  spraysShortDelay = value;
  spraysShortDelayAddr = eeAddress;
}

// receive EEPROM address and setup value for how long the device waits before spraying after a long visit
void spraysLongDelaySetup(int eeAddress, int value) {
  spraysLongDelay = value;
  spraysLongDelayAddr = eeAddress;
}


/////////////////////////
//       GETTERS       //
/////////////////////////

bool deviceIsIdle() {
  return (deviceState == 0);
}

// loops from 0-9 for now
int temperature() {
    temperatureSensor.requestTemperatures(); // Send the command to get temperatures
    float tempC = temperatureSensor.getTempCByIndex(0);
    // Check if reading was successful
    if(tempC != DEVICE_DISCONNECTED_C) 
    {
      return (int)tempC;
    }
    else{
      return 0;
    }
}

String deviceStateString() {
  switch (deviceState) {
    case 0:
      return "Idle";
    case 1:
      return "Detection";
    case 2:
      return "Num1";
    case 3:
      return "Num2";
    case 4:
      return "Cleaning";
    default:
      return "Unknown";
  }
}


////////////////////////////
//          LOOP          //
////////////////////////////

void deviceLoop(unsigned long curTime) {

}

//WORK IN PROGRESS
/*
//when bathroom is not in use, the system will stay idle untill the door is opened
void deviceIdleLoop(unsigned long curTime){
  if(doorIsOpen()){
    changeDeviceState(1);
  }
  //TODO: IMPLEMENT HEARTBEAT, IMPLEMENT LDR
}



int detectionInterval = 180000;  //3 minutes currently
bool doorClosed = true;


void deviceDetectionLoop(unsigned long curTime){

  //check if detectionInterval passed, if yes make choice
  if(compareTimestamps(curTime, deviceTimestamp, detectionInterval)){
    makeDetection();
  };

  //check if door is back closed again

}
*/
