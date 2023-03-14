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
unsigned long deviceActiveTime = 180l * 1000;     // device will stay on for x seconds upon sensing something
int executionDelay = 15 * 1000;                   // after detecting num_1 or num_2, time to wait before detecting if user left

int spraysShort, spraysLong;                      // how many sprays after long/short visit
long spraysShortDelay, spraysLongDelay;           // how many milliseconds delay between end of toilet use and spray

// yellowLed is fully controlled by sprayFunctions, greenLed shows a default heartbeat
int yellowLed = 0, greenLed = 2;                  // '0' for off, '1' for on, '2' for slow blink, '3' for fast blink
const int slowBlinkDelay = 3000, fastBlinkDelay = 800;
unsigned long yellowLedTimestamp = 0, greenLedTimestamp = 0;
bool yellowLedOn = false, greenLedOn = false;     // actual calculated outputs for one iteration, just one can be on or both are off
bool lastLedsPinmode = true, lastLedOutput = LOW;
unsigned long ledsTimestamp = 0, ledsDelay = 40;  // when leds logic was last checked and how long the delay in between checks should be

// vars for detecting
unsigned long personOnToiletTimestamp = 0;  // linked to distance sensor, stars up when person is sitting on toilet
unsigned long toiletTime = 0;               // save length of sitting-on-toilet visit
int personOnToiletShortThreshold = 12000;   // short visit threshold (only longer than accidental cleaning triggers)
long personOnToiletLongThreshold = 80000;   // long visit threshold (shorter than average, because detection often resets)
bool personIsOnToilet = false;
bool personHasGoneToToilet = false;
bool doorClosedDuringVisit = false;
bool doorOpenedAfterVisit = false;
int motionsForCleaningThreshold = 12;


/////////////////////////
//      FUNCTIONS      //
/////////////////////////

// setup for when we enter a specific state
void changeDeviceState(int newState) {
  if (deviceState != newState) {
    deviceTimestamp = millis();
    deviceState = newState;

    Serial.print(F("Switched to deviceState "));
    Serial.println(newState);

    // state initialisation logic
    switch (newState) {
      case 0:
        // reset sensors
        motionSensor.resetSensor();
        personIsOnToilet = false;
        toiletTime = 0;
        personHasGoneToToilet = false;
        doorClosedDuringVisit = false;
        doorOpenedAfterVisit = false;
        // slow blink for heartbeat
        greenLed = 2;
        break;
      case 1:
        // fast blink while detecting
        greenLed = 3;
        // prevent spraying someone on accident, cancel any scheduled sprays
        cancelSprays();
        activateScreen(); // activate our display
        break;
      case 2:
        // no led upon detection
        greenLed = 0;
        break;
      case 3:
        // no led upon detection
        greenLed = 0;
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

// for all detected states (num_1, num_2, cleaning) this is executed until the user is done
void executeDetectedUseCase(unsigned long curTime) {
  // wait executionDelay ms before checking if the user left the bathroom
  if (!compareTimestamps(curTime, deviceTimestamp, executionDelay)) return;

  // check if the user is done and has left the bathroom (no motion or distance changes, and either light off or door closed)
  if (!motionSensor.triggered && !distSensor.triggered && (!lightSensor.isLightOn() || magneticSensor.pressed)) {
    // print information about the (now finished) detected use case
    Serial.print(F("Duration of visit: "));
    Serial.println(toiletTime);
    Serial.print(F("Type of visit: "));
    Serial.println(deviceState);
    Serial.print(F("Motions sensed: "));
    Serial.println(motionSensor.motionsSensed);

    // it depends on the state what we execute
    switch (deviceState) {
      case 2:
        startSpray(spraysShort, spraysShortDelay);
        changeDeviceState(0);
        break;
      case 3:
        startSpray(spraysLong, spraysLongDelay);
        changeDeviceState(0);
        break;
      case 4:
        changeDeviceState(0);
        break;
      default:
        // this one shouldn't be able to run
        Serial.println("Error: Reached default case in executeDetectedUseCase() switch.");
        break;
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
bool deviceIsIdle(unsigned long curTime, int delay) {
  curTime += 500; // add 500 ms since deviceTimestamp can be bigger than curTime
  delay += 500;
  return ((deviceState == 0) && compareTimestamps(curTime, deviceTimestamp, delay));
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
  // if menu is being used, interrupt detection, delay sprays, and return to idle
  if (menuActive() && deviceState != 0) {
    changeDeviceState(0);
    return;
  }

  // update sensors
  motionSensor.update(curTime);
  distSensor.update(curTime);
  temperatureSensor.update(curTime);
  lightSensor.update(curTime);
  // magneticSensor already gets updated in other loop

  switch (deviceState) {
    case 0:
      // start detection mode if we 'detect motion AND (the light is on OR the door is open)' --> someone entered
      if (motionSensor.triggered && (lightSensor.isLightOn() || !magneticSensor.pressed)) {
        changeDeviceState(1);
        return;
      }
      break;
    case 1:
      // tick the person goes sitting on toilet, sensed with distancesensor
      if (!personIsOnToilet && distSensor.triggered) {
        personOnToiletTimestamp = curTime;
        personIsOnToilet = true;
      }

      // tick the person moves away from toilet, sensed with distancesensor
      if (personIsOnToilet && !distSensor.triggered) {
        personIsOnToilet = false;
        int ttReading = curTime - personOnToiletTimestamp;

        // if person was on toilet for long enough, store the toiletTime
        if (ttReading > personOnToiletShortThreshold) {
          personHasGoneToToilet = true;
          if (ttReading > toiletTime) {
            toiletTime = ttReading; // keep track of largest toilettime
          }
        }
      }

      // only start to check if this person closed the door 5 seconds after detection started
      if (!doorClosedDuringVisit && compareTimestamps(curTime, deviceTimestamp, 5000)) {
        if (!compareTimestamps(curTime, deviceTimestamp, 60l * 1000)) {
          // stop checking after 1 minute of usage, doorclosing after this is likely just someone leaving
          if (magneticSensor.pressed) {
            // only becomes true if door is closed during detection, and stays true
            doorClosedDuringVisit = magneticSensor.pressed;
          }
        }
      }

      if ((personIsOnToilet || personHasGoneToToilet) && doorClosedDuringVisit) {
        // check if door has opened since start of toilet usage, if it was closed at first
        if (!magneticSensor.pressed) {
          // if true, that means at some point after toilet usage the door was opened
          doorOpenedAfterVisit = true;
        }
      }

      // DECISION LOGIC
      // Make a decision once motionSensor has cooled off and someone was detected near the toilet
      if ((!motionSensor.triggered && personHasGoneToToilet)
        // ...or bathroom is empty before detection finished <-- no motion AND (no light OR (door was opened AND now closed))
        || (!motionSensor.triggered && (!lightSensor.isLightOn() || (doorOpenedAfterVisit && magneticSensor.pressed)))
        // ...or a decision is forced if device has been detecting for too long (for cleaning or false alarm)
        || (compareTimestamps(curTime, deviceTimestamp, deviceActiveTime))) {

        // if user was still on the toilet but detection stopped after deviceActiveTime, we still want that reading
        if (personIsOnToilet) {
          personIsOnToilet = false;
          int ttReading = curTime - personOnToiletTimestamp;

          // if person was on toilet for long enough, store the toiletTime and whether user used the toilet
          if (ttReading > personOnToiletShortThreshold) {
            personHasGoneToToilet = true;
            if (ttReading > toiletTime) {
              toiletTime = ttReading; // keep largest toilettime
            }
          }
        }

        // check amount of motion. if A lot happened, cleaning has happend or just a long visit
        if (motionSensor.motionsSensed >= motionsForCleaningThreshold && (!doorClosedDuringVisit || !personHasGoneToToilet)) {
          Serial.print(F("Cleaning use case detected. "));
          changeDeviceState(4);
        }
        // decide if this was a long or short visit
        else if (personHasGoneToToilet && toiletTime < personOnToiletLongThreshold && toiletTime > personOnToiletShortThreshold) {
          Serial.print(F("Num_1 use case detected. "));
          changeDeviceState(2);
        }
        else if (personHasGoneToToilet && toiletTime > personOnToiletLongThreshold) {
          Serial.print(F("Num_2 use case detected. "));
          changeDeviceState(3);
        } else {
          Serial.print(F("No use case detected, idle again. "));
          changeDeviceState(0);
        }
      }
      break;
    case 2:
    case 3:
    case 4:
      // keep trying to execute actions associated with this use case
      // if the bathroom is now empty, this function will execute those actions revert to idle state
      executeDetectedUseCase(curTime);
      break;
  }

  // update what our leds display
  updateLedsOutput(curTime);
}
