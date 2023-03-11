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

int spraysShort, spraysLong;                      // how many sprays after long/short visit
unsigned long spraysShortDelay, spraysLongDelay;  // how many milliseconds delay between end of toilet use and spray

// TODO change greenLed to 0 and somehow have it show device state instead
int yellowLed = 0, greenLed = 2;                  // '0' for off, '1' for on, '2' for slow blink, '3' for fast blink
const int slowBlinkDelay = 3000, fastBlinkDelay = 800;
unsigned long yellowLedTimestamp = 0, greenLedTimestamp = 0;
bool yellowLedOn = false, greenLedOn = false;     // actual calculated outputs for one iteration, just one can be on or both are off
bool lastLedsPinmode = true, lastLedOutput = LOW;
unsigned long ledsTimestamp = 0, ledsDelay = 40; // when leds logic was last checked and how long the delay in between checks should be

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

void updateLedsOutput(unsigned long curTime) {
  if (compareTimestamps(curTime, ledsTimestamp, ledsDelay)) {
  // calculate which led should be on right now to get the desired effects out of both leds
  // yellowLedOn, before it's updated, functions as the led's last state here
  switch (yellowLed) {
      case 0:
        yellowLedOn = false;
        break;
      case 1:
        yellowLedOn = true;
        break;
      case 2:
        if (compareTimestamps(curTime, yellowLedTimestamp, slowBlinkDelay)) {
          yellowLedTimestamp = curTime;
          yellowLedOn = !yellowLedOn;
        }
        break;
      case 3:
        if (compareTimestamps(curTime, yellowLedTimestamp, fastBlinkDelay)) {
          yellowLedTimestamp = curTime;
          yellowLedOn = !yellowLedOn;
        }
        break;
    }

    switch (greenLed) {
      case 0:
        greenLedOn = false;
        break;
      case 1:
        greenLedOn = true;
        break;
      case 2:
        if (compareTimestamps(curTime, greenLedTimestamp, slowBlinkDelay)) {
          greenLedTimestamp = curTime;
          greenLedOn = !greenLedOn;
        }
        break;
      case 3:
        if (compareTimestamps(curTime, greenLedTimestamp, fastBlinkDelay)) {
          greenLedTimestamp = curTime;
          greenLedOn = !greenLedOn;
        }
        break;
    }

    // actually update our pin with the calculated current outputs
    if (yellowLedOn || greenLedOn) {
      bool desiredOutput; // what we end up writing to the leds pin

      if (yellowLedOn && greenLedOn) {
        // if both leds want to be on, the one that wasn't on last loop wins
        desiredOutput = !lastLedOutput;
      } else {
        // high means yellow is on, low means green is on
        desiredOutput = yellowLedOn ? HIGH : LOW;
      }

      // if any leds are on, we need pinMode to be output
      if (lastLedsPinmode) {
        lastLedsPinmode = false;
        pinMode(ledsPin, OUTPUT);
      }

      // check if output should change first so we don't do an unnecessary write
      if (lastLedOutput != desiredOutput) {
        lastLedOutput = desiredOutput;
        digitalWrite(ledsPin, desiredOutput);
      }

    } else {
      // if both leds are off, pinMode should be input
      if (!lastLedsPinmode) {
        lastLedsPinmode = true;
        lastLedOutput = LOW;
        pinMode(ledsPin, INPUT);
      }
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
}

void setSpraysLong(int newValue) {
  spraysLong = newValue;
}

void setSpraysShortDelay(unsigned long newValue) {
  spraysShortDelay = newValue;
}

void setSpraysLongDelay(unsigned long newValue) {
  spraysLongDelay = newValue;
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
  if (tempC != DEVICE_DISCONNECTED_C)
  {
    return (int)tempC;
  }
  else {
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
  // update sensors

  // run detection / other states' logic

  updateLedsOutput(curTime);
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
