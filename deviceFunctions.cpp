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
  return (int)(millis() / 1000 % 10);
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
