#include "gestureDetection.h"


/* ========================
  ===   CONFIGURATION   ===
  ====================== */

// Possible states:
#define IDLE        0 // Controller isn't being used, default mode
#define ACTIVATING  1 // In grace period before becoming activated
#define ACTIVATED   2 // Awaiting gesture initiation
#define DETECTING   3 // Detecting which gesture was used
#define DETECTED    4 // Finished detecting a gesture, in grace period

int deviceState = DETECTING; // TODO set to IDLE
unsigned long deviceTimestamp = 0;

int gracePeriod = 5000;               // delay for ACTIVATING --> ACTIVATED or DETECTED --> ACTIVATED
int idleDelay = 30000;                // delay for ACTIVATED --> IDLE
int maxDetectionTime = 10000;         // time a user has to make their gesture in DETECTING state
boolean ledState;                     // used for blinking led
BlockNot graceLedInterval(200);       // how fast the led blinks while awaiting state change
BlockNot dectectionLedInterval(500);  // led blink interval in DETECTING state
BlockNot motionDetection(500);        // how often the device should check for motion

BlockNot mpuLoopInterval(100);        // interval at which accelerometer/gyroscope data is updated
BlockNot mpuIdleLoopInterval(500);        // same thing but in IDLE state

// we store the last x rotation/acceleration values for our detection functions
const int maxValuesStored = 5;
float lastRolls[maxValuesStored], lastPitches[maxValuesStored], lastYaws[maxValuesStored],
  lastAccXs[maxValuesStored], lastAccYs[maxValuesStored], lastAccZs[maxValuesStored];
// iterator to find the value that was last added to the array
int lastValueIterator;

// initialisation (bend hand down, and back up) detection variables
int initDetectionState;
int initBeginValue;
int initMoveValue;
int initReturnValue;

// refresh (swipe left/right or opposite) gesture thresholds
int refreshDetectionState;
int refreshBeginValue;
int refreshMoveValue;
int refreshReturnValue;

// // watering (rotate arm 180 degrees and back) gesture thresholds
// float initThresholdAccX;
// float initThresholdPitch;
// int detectionValue;
// unsigned long startTimestamp;
int waterCommand;


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
        initDetectionState = initBeginValue = initMoveValue = initReturnValue = 0;
        break;

      case DETECTING:
        digitalWrite(LED_BUILTIN, HIGH);
        ledState = true;
        // reset variables used in DETECTING state
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
    >= 2
  ) return true; else return false;
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
      initBeginValue = initBeginValue < 25 ? initBeginValue + 2 : 26; // max value 26
    } else {
      initBeginValue = --initBeginValue < 0 ? 0 : initBeginValue;
    }
    if (initBeginValue > 10) {
      initDetectionState = 1;
    } else {
      // reset if gesture wasn't fast enough
      initDetectionState = 0;
      initMoveValue = 0;
      initReturnValue = 0;
    }
  }

  // detect the actual movement for this gesture
  if (initDetectionState == 1 || initDetectionState == 2) {
    if (abs(lastAccX) > 6000 && abs(lastAccZ) > 3000 && abs(lastPitch) > 20) {
      initMoveValue = initMoveValue < 13 ? initMoveValue + 3 : 15; // max value 15
    } else {
      initMoveValue = --initMoveValue < 0 ? 0 : initMoveValue;
    }
    if (initMoveValue > 8) {
      initDetectionState = 2;
    } else if (initMoveValue < 3) {
      initReturnValue = 0;
    }
  }

  // we want at least 3 stable ticks after the movement to end the gesture
  if (initDetectionState == 2) {
    // don't use pitch here since it doesn't go back to 0 fast enough
    if (abs(lastAccX) < 4000 && abs(lastAccY) < 4000 && abs(lastAccZ) < 5000) {
      initReturnValue++;
    } else {
      initReturnValue = --initReturnValue < 0 ? 0 : initReturnValue;
    }
    // when hand stops moving...
    if (initMoveValue < 4) {
      if (initReturnValue >= 3) {
        // gesture detected only if hand is now back in normal position
        return true; // no need to reset any variables, state change does that
      } else {
        // not detected if hand ends in different position
        initMoveValue = 0;
        initBeginValue = initReturnValue;
        initReturnValue = 0;
        initDetectionState = 0;
      }
    }
  }
  // return false if not detected
  return false;
}

boolean detectRefreshGesture() {
  // TODO
  Serial.print(",");
  Serial.print("AVG:");
  Serial.println(getArrayAverageAbs(lastAccYs, maxValuesStored));

  Serial.print("Begin:");
  Serial.print(refreshBeginValue);
  Serial.print(",");
  Serial.print("Move:");
  Serial.print(refreshMoveValue);
  Serial.print(",");
  Serial.print("End:");
  Serial.print(refreshReturnValue);
  Serial.print(",");
  Serial.print("State:");
  Serial.println(refreshDetectionState);

  // (absolute) average of the most recently saved sideways acceleration values
  float avgAccX = getArrayAverageAbs(lastAccXs, maxValuesStored);
  float avgAccY = getArrayAverageAbs(lastAccYs, maxValuesStored);
  float avgAccZ = getArrayAverageAbs(lastAccZs, maxValuesStored);

  // we need enough stable ticks before we start detecting the required movement for this gesture
  if (refreshDetectionState == 0 || refreshDetectionState == 1) {
    if (avgAccX < 2000 && avgAccY < 2000 && avgAccZ < 2500) {
      refreshBeginValue = refreshBeginValue < 25 ? refreshBeginValue + 2 : 26; // max value 26
    } else {
      refreshBeginValue = --refreshBeginValue < 0 ? 0 : refreshBeginValue;
    }
    if (refreshBeginValue > 10) {
      refreshDetectionState = 1;
    } else {
      // reset if gesture wasn't fast enough
      refreshDetectionState = 0;
      refreshMoveValue = 0;
      refreshReturnValue = 0;
    }
  }

  // we want to detect strong sideways acceleration (or deceleration) for at least 4 ticks
  if (refreshDetectionState == 1 || refreshDetectionState == 2) {
    if (avgAccY > 3000) {
      refreshMoveValue = refreshMoveValue < 11 ? refreshMoveValue + 2 : 12; // max value 12
    } else {
      refreshMoveValue = --refreshMoveValue < 0 ? 0 : refreshMoveValue;
    }
    if (refreshMoveValue >= 8) {
      refreshDetectionState = 2;
    } else if (refreshMoveValue < 3) {
      refreshReturnValue = 0;
    }
  }

  // we want at least 3 stable ticks before refreshMoveValue goes too far back down (in 6-10 ticks)
  if (refreshDetectionState == 2) {
    // don't use pitch here since it doesn't go back to 0 fast enough
    if (avgAccX < 2000 && avgAccY < 2000 && avgAccZ < 2500) {
      refreshReturnValue++;
    } else {
      refreshReturnValue = --refreshReturnValue < 0 ? 0 : refreshReturnValue;
    }
    // when hand stops moving...
    if (refreshMoveValue < 4) {
      if (refreshReturnValue >= 3) {
        // gesture detected only if hand is now back in normal position
        return true; // no need to reset any variables, state change does that
      } else {
        // not detected if hand ends in different position
        refreshMoveValue = 0;
        refreshBeginValue = refreshReturnValue;
        refreshReturnValue = 0;
        refreshDetectionState = 0;
      }
    }
  }
  // return false if not detected
  return false;
}

// returns 1 if detected, or 2 if 'more water' gesture detected
int detectWaterGesture() {
  // TODO

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
  if (detectRefreshGesture()) {
    Serial.println("BOEMSHAKALAKAAAA");
    delay(3000);
  }

  return; // TODO remove this obviously

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
