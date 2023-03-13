#include "deviceFunctions.h"


/////////////////////////
//    CONFIGURATION    //
/////////////////////////

/* Possible states:
  0 --> 'idle': Bathroom isn't being used, default mode
  1 --> 'detect': Detecting use case
  2 --> 'num_1': ...
  3 --> 'num_2': ...
  4 --> 'clean': ...
*/
int deviceState = 0;
unsigned long deviceTimestamp = 0;

// timing related vars
unsigned long deviceActiveTime = 120000;          // device will stay on for x seconds upon sensing something.
unsigned long deviceCleaningInterval = 90000;     // estimate time of 10 minutes for cleaning the toilet

int spraysShort, spraysLong;                      // how many sprays after long/short visit
unsigned long spraysShortDelay, spraysLongDelay;  // how many milliseconds delay between end of toilet use and spray

// yellowLed is fully controlled by sprayFunctions, greenLed shows a default heartbeat
int yellowLed = 0, greenLed = 2;                  // '0' for off, '1' for on, '2' for slow blink, '3' for fast blink
const int slowBlinkDelay = 3000, fastBlinkDelay = 800;
unsigned long yellowLedTimestamp = 0, greenLedTimestamp = 0;
bool yellowLedOn = false, greenLedOn = false;     // actual calculated outputs for one iteration, just one can be on or both are off
bool lastLedsPinmode = true, lastLedOutput = LOW;
unsigned long ledsTimestamp = 0, ledsDelay = 40; // when leds logic was last checked and how long the delay in between checks should be

// vars for detecting
unsigned long personOnToiletTimestamp = 0;  // linked to distance sensor, stars up when person is sitting on toilet
unsigned long toiletTime = 0;               // save length of sitting-on-toilet visit
int personOnToiletShortThreshold = 10000;   // threshold for short visit
long personOnToiletLongThreshold = 100000;  // threhsold for long visit
bool personIsOnToilet = false;
bool personHasGoneToToilet = false;
bool doorClosedDuringVisit = false;
int motionsForCleaningThreshold = 12;

/////////////////////////
//      FUNCTIONS      //
/////////////////////////

// setup for when we enter a specific state
void changeDeviceState(int newState) {
  if (deviceState != newState) {
    deviceState = newState;
    deviceTimestamp = millis();

    Serial.println("Changed to deviceState " + String(newState));

    // state initialisation logic
    switch (newState) {
      case 0:
        // reset sensors
        motionSensor.resetSensor();
        personIsOnToilet = false;
        toiletTime = 0;
        personHasGoneToToilet = false;
        doorClosedDuringVisit = false;
        // slow blink for heartbeat
        greenLed = 2;
        break;
      case 1:
        // fast blink while detecting
        greenLed = 3;
        activateScreen(); // activate our display
        break;
      case 2:
        // no led upon detection
        greenLed = 0;
        startSpray(spraysShort, spraysShortDelay);
        break;
      case 3:
        // no led upon detection
        greenLed = 0;
        startSpray(spraysLong, spraysLongDelay);
        break;
      case 4:
        // constant burning led upon cleaning detection
        greenLed = 1;
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

// used to determine if the screen can be turned off
bool deviceIsIdle() {
  return (deviceState == 0);
}

// returns the current temperature
int temperature() {
  return temperatureSensor.lastReading;
}

String deviceStateString() {
  switch (deviceState) {
    case 0:
      return "Idle";
    case 1:
      return "Detect";
    case 2:
      return "Num_1";
    case 3:
      return "Num_2";
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
  motionSensor.update(curTime);
  distSensor.update(curTime);
  temperatureSensor.update(curTime);
  lightSensor.update(curTime);
  // magneticSensor already gets updated in other loop

  if (deviceState == 0 && motionSensor.triggered && lightSensor.isLightOn()) {
    changeDeviceState(1);
    return;
  }

  switch (deviceState) {
    case 0:
      break;
    case 1:
      // tick the person goes sitting on toilet, sensed with distancesensor
      if (!personIsOnToilet && distSensor.triggered) {
        personHasGoneToToilet = false;
        personOnToiletTimestamp = curTime;
        personIsOnToilet = true;
        doorClosedDuringVisit = magneticSensor.pressed;
      }

      // tick the person moves away from toilet, sensed with distancesensor
      if (personIsOnToilet && !distSensor.triggered) {
        personHasGoneToToilet = true;
        int ttReading = curTime - personOnToiletTimestamp;
        if (ttReading > toiletTime) {
          toiletTime = ttReading;  // keep track of largest toilettime
          personIsOnToilet = false;
        }

      }

      // decision logic
      // make decision when toilet is not in use -> light is off
      // or time has elapsed
      // only make decision onces motionSensor has cooled off
      if ((!(lightSensor.isLightOn() || motionSensor.triggered)  && !motionSensor.triggered && personHasGoneToToilet)
          || (compareTimestamps(curTime, deviceTimestamp, deviceActiveTime))) {

        // check amount of motion. if A lot happened, cleaning has happend or just a long visit
        if (motionSensor.motionsSensed >= motionsForCleaningThreshold && !doorClosedDuringVisit) {
          changeDeviceState(4);
        }
        // decide if this was a long or short visit
        else if (toiletTime < personOnToiletLongThreshold && toiletTime > personOnToiletShortThreshold) {
          changeDeviceState(2);
        }
        else if (toiletTime > personOnToiletLongThreshold) {
          changeDeviceState(3);
        }
        Serial.println("Duration of visit");
        Serial.println(toiletTime);
        Serial.println("Type of visit");
        Serial.println(deviceState);
        Serial.println("Motions sensed");
        Serial.println(motionSensor.motionsSensed);
      }
      break;
    case 2:
      if (!sprayComing())
        changeDeviceState(0);
      break;
    case 3:
      if (!sprayComing())
        changeDeviceState(0);
      break;
    case 4:
      // just wait untill enough time has elapsed, then change state back to idle
      if (compareTimestamps(curTime, deviceTimestamp, deviceCleaningInterval)) {
        changeDeviceState(0);
      }
      break;
  }

  // update what our leds display
  updateLedsOutput(curTime);
}
