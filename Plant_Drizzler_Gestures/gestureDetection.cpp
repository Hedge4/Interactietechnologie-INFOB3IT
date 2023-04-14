#include "gestureDetection.h"


/* ========================
  ===   CONFIGURATION   ===
  ====================== */

// Possible states:
#define IDLE 0        // Controller isn't being used, default mode
#define ACTIVATING 1  // In grace period before becoming activated
#define ACTIVATED 2   // Awaiting gesture initiation
#define DETECTING 3   // Detecting which gesture was used
#define DETECTED 4    // Finished detecting a gesture, in grace period

int deviceState = DETECTING;  // TODO set to IDLE
unsigned long deviceTimestamp = 0;

int gracePeriod = 5000;               // delay for ACTIVATING --> ACTIVATED or DETECTED --> ACTIVATED
int idleDelay = 30000;                // delay for ACTIVATED --> IDLE
int maxDetectionTime = 10000;         // time a user has to make their gesture in DETECTING state
boolean ledState;                     // used for blinking led
BlockNot graceLedInterval(200);       // how fast the led blinks while awaiting state change
BlockNot dectectionLedInterval(500);  // led blink interval in DETECTING state
BlockNot motionDetection(500);        // how often the device should check for motion

BlockNot mpuLoopInterval(100);      // interval at which accelerometer/gyroscope data is updated
BlockNot mpuIdleLoopInterval(500);  // same thing but in IDLE state

// we store the last x rotation/acceleration values for our detection functions
const int maxValuesStored = 5;
float lastRolls[maxValuesStored], lastPitches[maxValuesStored], lastYaws[maxValuesStored],
  lastAccXs[maxValuesStored], lastAccYs[maxValuesStored], lastAccZs[maxValuesStored];
// iterator to find the value that was last added to the array
int lastValueIterator;

// initialisation (bend hand down, and back up) detection variables
int initDetectionState;
int initStartValue;
int initMoveValue;
int initEndValue;

// refresh (swipe left/right or opposite) detection variables
int refreshDetectionState;
int refreshStartValue;
int refreshMoveValue;
int refreshEndValue;

// watering (rotate arm 180 degrees and back) detection variables
int waterDetectionState;
int waterStartValue;
int waterHalfwayValue;
unsigned long waterGesturetimestamp;  // to store how long someone keeps the halfway position
int moreWaterDelay = 2000;            // how long a user should hold the gesture to give more water
int waterCommand;                     // give the result if someone kept position for a short/long time


/* ========================
  ===     FUNCTIONS     ===
  ====================== */

// setup for when we enter a specific state
void changeDeviceState(int newState) {
  if (deviceState != newState) {
    deviceTimestamp = millis();
    deviceState = newState;

    Serial.print(F("Switched to deviceState "));
    Serial.println(newState);

    // state initialisation logic
    switch (newState) {
      case IDLE:
        digitalWrite(LED_BUILTIN, LOW);
        break;

      case ACTIVATING:
        digitalWrite(LED_BUILTIN, HIGH);
        ledState = true;
        break;

      case ACTIVATED:
        digitalWrite(LED_BUILTIN, LOW);
        // reset variables used in ACTIVATED state
        initDetectionState = initStartValue = initMoveValue = initEndValue = 0;
        break;

      case DETECTING:
        digitalWrite(LED_BUILTIN, HIGH);
        ledState = true;
        // reset variables used in DETECTING state
        refreshDetectionState = refreshStartValue = refreshMoveValue = refreshEndValue = 0;
        // TODO change variables from detecting to 0
        break;

      case DETECTED:
        // no ledState since it's constant on instead blinking
        digitalWrite(LED_BUILTIN, HIGH);
        break;
    }
  }
}

float getArrayAverageAbs(float *arr, int size) {
  // get average of absolute values because for simplicity we only work with positive values
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += abs(arr[i]);
  }
  return sum / size;
}


/* ==============================
  ===   DETECTION FUNCTIONS   ===
  ============================ */

boolean motionDetected() {
  // we use <= instead of < to enforce casting the average to an int
  if (
    (getArrayAverageAbs(lastRolls, maxValuesStored) > 10)
      + (getArrayAverageAbs(lastPitches, maxValuesStored) > 10)
      + (getArrayAverageAbs(lastYaws, maxValuesStored) > 10)
      + (getArrayAverageAbs(lastAccXs, maxValuesStored) > 4000)
      + (getArrayAverageAbs(lastAccYs, maxValuesStored) > 4000)
      + (getArrayAverageAbs(lastAccZs, maxValuesStored) > 4000)
    // if any of these, there is some movement, but we require two to weed out false positives
    >= 2) return true;
  else return false;
  // we could also detect outliers to detect change, but this works just as well
}

boolean detectInitialisation() {
  // most recently saved MPU6050 values
  float lastPitch = lastPitches[lastValueIterator];
  float lastAccX = lastAccXs[lastValueIterator];
  float lastAccY = lastAccYs[lastValueIterator];
  float lastAccZ = lastAccZs[lastValueIterator];

  // hand needs to be stable before we start detecting the actual required movement for this gesture
  if (initDetectionState == 0 || initDetectionState == 1) {
    if (abs(lastAccX) < 4000 && abs(lastAccY) < 4000 && abs(lastAccZ) < 5000) {
      initStartValue = initStartValue < 25 ? initStartValue + 2 : 26;  // max value 26
    } else {
      initStartValue = --initStartValue < 0 ? 0 : initStartValue;
    }
    if (initStartValue > 10) {
      initDetectionState = 1;
    } else {
      // reset if gesture wasn't fast enough
      initDetectionState = 0;
      initMoveValue = 0;
      initEndValue = 0;
    }
  }

  // detect the actual movement for this gesture
  if (initDetectionState == 1 || initDetectionState == 2) {
    if (abs(lastAccX) > 6000 && abs(lastAccZ) > 3000 && abs(lastPitch) > 20) {
      initMoveValue = initMoveValue < 13 ? initMoveValue + 3 : 15;  // max value 15
    } else {
      initMoveValue = --initMoveValue < 0 ? 0 : initMoveValue;
    }
    if (initMoveValue > 8) {
      initDetectionState = 2;
    } else if (initMoveValue < 3) {
      initEndValue = 0;
    }
  }

  // we want at least 3 stable ticks after the movement to end the gesture
  if (initDetectionState == 2) {
    // don't use pitch here since it doesn't go back to 0 fast enough
    if (abs(lastAccX) < 4000 && abs(lastAccY) < 4000 && abs(lastAccZ) < 5000) {
      initEndValue++;
    } else {
      initEndValue = --initEndValue < 0 ? 0 : initEndValue;
    }
    // when hand stops moving...
    if (initMoveValue < 4) {
      if (initEndValue >= 3) {
        // gesture detected only if hand is now back in normal position
        return true;  // no need to reset any variables, state change does that
      } else {
        // not detected if hand ends in different position
        initMoveValue = 0;
        initStartValue = initEndValue;
        initEndValue = 0;
        initDetectionState = 0;
      }
    }
  }
  // return false if not detected
  return false;
}

boolean detectRefreshGesture() {
  // (absolute) average of the most recently saved sideways acceleration values
  float avgAccX = getArrayAverageAbs(lastAccXs, maxValuesStored);
  float avgAccY = getArrayAverageAbs(lastAccYs, maxValuesStored);
  float avgAccZ = getArrayAverageAbs(lastAccZs, maxValuesStored);

  // we need enough stable ticks before we start detecting the required movement for this gesture
  if (refreshDetectionState == 0 || refreshDetectionState == 1) {
    if (avgAccX < 2000 && avgAccY < 2000 && avgAccZ < 2500) {
      refreshStartValue = refreshStartValue < 25 ? refreshStartValue + 2 : 26;  // max value 26
    } else {
      refreshStartValue = --refreshStartValue < 0 ? 0 : refreshStartValue;
    }
    if (refreshStartValue > 10) {
      refreshDetectionState = 1;
    } else {
      // reset if gesture wasn't fast enough
      refreshDetectionState = 0;
      refreshMoveValue = 0;
      refreshEndValue = 0;
    }
  }

  // we want to detect strong sideways acceleration (or deceleration) for at least 4 ticks
  if (refreshDetectionState == 1 || refreshDetectionState == 2) {
    if (avgAccY > 3000) {
      refreshMoveValue = refreshMoveValue < 11 ? refreshMoveValue + 2 : 12;  // max value 12
    } else {
      refreshMoveValue = --refreshMoveValue < 0 ? 0 : refreshMoveValue;
    }
    if (refreshMoveValue >= 8) {
      refreshDetectionState = 2;
    } else if (refreshMoveValue < 3) {
      refreshEndValue = 0;
    }
  }

  // we want at least 3 stable ticks before refreshMoveValue goes too far back down (in 6-10 ticks)
  if (refreshDetectionState == 2) {
    // don't use pitch here since it doesn't go back to 0 fast enough
    if (avgAccX < 2000 && avgAccY < 2000 && avgAccZ < 2500) {
      refreshEndValue++;
    } else {
      refreshEndValue = --refreshEndValue < 0 ? 0 : refreshEndValue;
    }
    // when hand stops moving...
    if (refreshMoveValue < 4) {
      if (refreshEndValue >= 3) {
        // gesture detected only if hand is now back in normal position
        return true;  // no need to reset any variables, state change does that
      } else {
        // not detected if hand ends in different position
        refreshMoveValue = 0;
        refreshStartValue = refreshEndValue;
        refreshEndValue = 0;
        refreshDetectionState = 0;
      }
    }
  }
  // return false if not detected
  return false;
}

bool checkArrayTrend(float *values, int length, int startIndex, bool checkGoingUp) {
  int count = 0;
  // startIndex - length + 1 since startIndex is the newest value and has no next value
  // we're counting down since we want to go in the opposite direction as adding new values
  for (int i = startIndex; i > startIndex - length + 1; i--) {
    // add length to i and then use modulo since we're counting down instead of up
    float currentValue = values[(i + length) % length];
    float previousValue = values[(i - 1 + length) % length];
    if (checkGoingUp) {
      if (currentValue > previousValue) {
        count++;
      }
    } else {
      if (currentValue < previousValue) {
        count++;
      }
    }
  }
  // return true if two thirds of values are indeed going up/down
  return count >= length * 2 / 3;
}

// arm can be rotated 90 degrees clockwise or counter clockwise
int inWaterGesturePosition(float avgAccX, float avgAccY, float avgAccZ, float lastRoll) {
  // if ( avgAccX < 2500 && avgAccY < 9000 && avgAccZ < 12000
  //   && avgAccX > 0 && avgAccY > 5000 && avgAccZ > 4000
  // ) { Serial.println("COND 1"); }

  // if (lastRoll < -50 || (lastRoll < 50 && checkArrayTrend(lastRolls, maxValuesStored, lastValueIterator, false))
  // ) { Serial.println("COND 2"); }

  // if ( avgAccX < 2500 && avgAccY < 9000 && avgAccZ < 14000
  //   && avgAccX > 0 && avgAccY > 5000 && avgAccZ > 6000
  // ) { Serial.println("COND 3"); }

  // if (lastRoll > 50 || (lastRoll > -50 && checkArrayTrend(lastRolls, maxValuesStored, lastValueIterator, true))
  // ) { Serial.println("COND 4"); }

  // clockwise condition
  if (avgAccX < 2500 && avgAccY < 9000 && avgAccZ < 12000
      && avgAccX > 0 && avgAccY > 5000 && avgAccZ > 4000
      // check if roll is either already low or going down
      && (lastRoll < -50 || checkArrayTrend(lastRolls, maxValuesStored, lastValueIterator, false))) { return 1; }

  // counter clockwise condition
  if (avgAccX < 2500 && avgAccY < 9000 && avgAccZ < 14000
      && avgAccX > 0 && avgAccY > 5000 && avgAccZ > 6000
      // check if roll is either already high or going up
      && (lastRoll > 50 || checkArrayTrend(lastRolls, maxValuesStored, lastValueIterator, true))) { return 2; }

  // if not in position return false
  return 0;
}

// returns 1 if detected, or 2 if 'more water' gesture detected
int detectWaterGesture() {
  // TODO
  float lastRoll = lastRolls[lastValueIterator];
  float avgAccX = getArrayAverageAbs(lastAccXs, maxValuesStored);
  float avgAccY = getArrayAverageAbs(lastAccYs, maxValuesStored);
  float avgAccZ = getArrayAverageAbs(lastAccZs, maxValuesStored);

  inWaterGesturePosition(avgAccX, avgAccY, avgAccZ, lastRoll);

  // return 0 if not detected
  return 0;
}


/* ===========================
  ===   PUBLIC FUNCTIONS   ===
  ========================= */

void storeMpuValues(float roll, float pitch, float yaw, float accX, float accY, float accZ) {
  // reset lastValueIterator to 0 if it reaches the end of the arrays
  lastValueIterator = ++lastValueIterator < maxValuesStored ? lastValueIterator : 0;

  lastRolls[lastValueIterator] = roll;
  lastPitches[lastValueIterator] = pitch;
  lastYaws[lastValueIterator] = yaw;
  lastAccXs[lastValueIterator] = accX;
  lastAccYs[lastValueIterator] = accY;
  lastAccZs[lastValueIterator] = accZ;
}


/* =========================
  ===        LOOP        ===
  ======================= */

void gestureDetectionLoop() {
  // limit how often the MPU is read and data is interpreted
  if (deviceState == IDLE) {
    // in idle state we don't need to be as active
    if (!mpuIdleLoopInterval.triggered()) return;
  } else {
    if (!mpuLoopInterval.triggered()) return;
  }

  // update accelerometer and gyroscope output data
  getMpuValues();


  // TODO remove test functions
  if (detectWaterGesture()) {
    Serial.println("BOEMSHAKALAKAAAA");
    delay(3000);
  }

  return;  // TODO remove this obviously

  switch (deviceState) {
    case IDLE:
      // leave idle state if any motion was detected
      if (motionDetected()) {
        changeDeviceState(ACTIVATING);
      }
      break;

    case ACTIVATING:
      // wait fixed amount of time before starting detection
      if (millis() - gracePeriod > deviceTimestamp) {
        changeDeviceState(ACTIVATED);
        return;
      }
      // quickly blink Arduino led while 'activating'
      if (graceLedInterval.triggered()) {
        ledState != ledState;
        digitalWrite(LED_BUILTIN, ledState);
      }
      break;

    case ACTIVATED:
      // in this state deviceTimestamp stores the last detected motion...
      if (motionDetected()) {
        deviceTimestamp = millis();
      } else if (millis() - idleDelay > deviceTimestamp) {
        // ...and if no motion is detected for too long we go back into IDLE state
        changeDeviceState(IDLE);
        return;
      }

      // detect initialisation gesture, and if detected move into the actual detection mode
      if (detectInitialisation()) {
        changeDeviceState(DETECTING);
      }
      break;

    case DETECTING:
      // if no gesture was found for too long, go back to ACTIVATED state
      if (millis() - maxDetectionTime > deviceTimestamp) {
        // and if no motion is detected for too long we go back into IDLE state
        changeDeviceState(ACTIVATED);
        return;
      }

      // after recognising initialisatino gesture, slowly blink to show we're detecting now
      if (dectectionLedInterval.triggered()) {
        ledState != ledState;
        digitalWrite(LED_BUILTIN, ledState);
      }

      if (detectRefreshGesture()) {
        sendRefreshCommand();
        return;
      }

      waterCommand = detectWaterGesture();
      if (waterCommand) {
        if (waterCommand == 1) {
          sendWaterCommand();
        } else if (waterCommand == 2) {
          sendMoreWaterCommand();
        }
      }
      break;

    case DETECTED:
      // command is sent immediately in this state, so we only wait a while before restarting detection
      if (millis() - gracePeriod > deviceTimestamp) {
        changeDeviceState(ACTIVATED);
      }
      break;
  }
}
