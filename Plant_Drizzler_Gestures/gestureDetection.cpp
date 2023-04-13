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

int deviceState = ACTIVATED; // TODO
unsigned long deviceTimestamp = 0;

int gracePeriod = 5000;               // delay for ACTIVATING --> ACTIVATED or DETECTED --> ACTIVATED or ACTIVATED --> IDLE
int maxDetectionTime = 10000;         // time a user has to make their gesture in DETECTING state
boolean ledState;                     // used for blinking led
BlockNot graceLedInterval(200);       // how fast the led blinks while awaiting state change
BlockNot dectectionLedInterval(500);  // led blink interval in DETECTING state

// we store the latest rotation/acceleration values for our detection functions
float storedRoll, storedPitch, storedYaw, storedAccX, storedAccY, storedAccZ;

// initialisation (bend hand down, and back up) detection variables
int initDetectionStage;
int initBeginValue;
int initMoveValue;
int initReturnValue;


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
        // reset variables from ACTIVATED state
        initDetectionStage = initBeginValue = initMoveValue = initReturnValue = 0;
        break;
        
      case ACTIVATING:
        digitalWrite(LED_BUILTIN, HIGH);
        ledState = true;
        break;

      case ACTIVATED:
        digitalWrite(LED_BUILTIN, LOW);
        // reset variables from DETECTING state
        // TODO change variables from detecting to 0
        break;

      case DETECTING:
        digitalWrite(LED_BUILTIN, HIGH);
        ledState = true;
        // reset variables from ACTIVATED state
        initDetectionStage = initBeginValue = initMoveValue = initReturnValue = 0;
        break;

      case DETECTED:
        // no ledState since it's constant on instead blinking
        digitalWrite(LED_BUILTIN, HIGH);
        // reset variables from DETECTING state
        // TODO change variables from detecting to 0
        break;
    }
  }
}


/* ==============================
  ===   DETECTION FUNCTIONS   ===
  ============================ */

boolean motionDetected() {
  return true;
  // TODO
}

boolean detectInitialisation() {
  if (initDetectionStage == 0 || initDetectionStage == 1) {
    if (abs(storedAccX) < 4000 && abs(storedAccY) < 4000 && abs(storedAccZ) < 1000) {
      initBeginValue = initBeginValue < 25 ? initBeginValue + 2 : 26; // max value 26
    } else {
      initBeginValue = --initBeginValue < 0 ? 0 : initBeginValue;
    }
    if (initBeginValue > 10) {
      initDetectionStage = 1;
    } else {
      // reset if gesture wasn't fast enough
      initDetectionStage = 0;
      initMoveValue = 0;
      initReturnValue = 0;
    }
  }

  if (initDetectionStage == 1 || initDetectionStage == 2) {
    if (abs(storedAccX) > 6000 && abs(storedAccZ) > 3000 && abs(storedPitch) > 20) {
      initMoveValue = initMoveValue < 13 ? initMoveValue + 3 : 15; // max value 15
    } else {
      initMoveValue = --initMoveValue < 0 ? 0 : initMoveValue;
    }
    if (initMoveValue > 8) {
      initDetectionStage = 2;
    } else if (initMoveValue < 3) {
      initReturnValue = 0;
    }
  }

  if (initDetectionStage == 2) {
    // don't use pitch here since it doesn't go back to 0 fast enough
    if (abs(storedAccX) < 4000 && abs(storedAccY) < 4000 && abs(storedAccZ) < 1000) {
      initReturnValue++;
    } else {
      initReturnValue = --initReturnValue < 0 ? 0 : initReturnValue;
    }
    // when hand stops moving...
    if (initMoveValue < 4) {
      if (initReturnValue >= 3) {
        // gesture detected only if hand is now back in normal position
        initDetectionStage = 5;
      } else {
        // not detected if hand ends in different position
        initMoveValue = 0;
        initBeginValue = initReturnValue;
        initReturnValue = 0;
        initDetectionStage = 0;
      }
    }
  }

  Serial.print(',');
  Serial.print("BeginValue:");
  Serial.print(initBeginValue * 100);
  Serial.print(',');
  Serial.print("MoveValue:");
  Serial.print(initMoveValue * 100);
  Serial.print(',');
  Serial.print("ReturnValue:");
  Serial.print(initReturnValue * 100);
  Serial.print(',');
  Serial.println(initDetectionStage * -2);

  return false;
}


/* =============================
  ===   EXTERNAL FUNCTIONS   ===
  =========================== */

void storeMpuValues(float roll, float pitch, float yaw, float accX, float accY, float accZ) {
  storedRoll = roll;
  storedPitch = pitch;
  storedYaw = yaw;
  storedAccX = accX;
  storedAccY = accY;
  storedAccZ = accZ;
}


/* =========================
  ===        LOOP        ===
  ======================= */

void gestureDetectionLoop() {
  // update accelerometer and gyroscope output data
  getMpuValues();

  // TODO remove test functions
  Serial.print("Motion: ");
  Serial.println(detectInitialisation());


  return; // TODO

  switch (deviceState) {
    case IDLE:
      // leave idle state in case any motion was detected
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
      // in this state deviceTimestamp stores the last detected motion
      if (motionDetected()) {
        deviceTimestamp = millis();
      } else if (millis() - gracePeriod > deviceTimestamp) {
        // and if no motion is detected for too long we go back into IDLE state
        changeDeviceState(IDLE);
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
      }

      // after recognising initialisatino gesture, slowly blink to show we're detecting now
      if (dectectionLedInterval.triggered()) {
        ledState != ledState;
        digitalWrite(LED_BUILTIN, ledState);
      }

      // TODO move the remaining gesture functions here
      break;

    case DETECTED:
      // command is sent immediately in this state, so we only wait a while before restarting detection
      if (millis() - gracePeriod > deviceTimestamp) {
        changeDeviceState(ACTIVATED);
      }
      break;
  }
}
