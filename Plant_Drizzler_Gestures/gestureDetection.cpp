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

int deviceState = 0;
unsigned long deviceTimestamp = 0;


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
        break;
      case ACTIVATING:
        break;
      case ACTIVATED:
        break;
      case DETECTING:
        break;
      case DETECTED:
        break;
    }
  }
}

void storeMpuValues(float roll, float pitch, float yaw, float correctedAccX, float correctedAccY, float correctedAccZ) {
  // TODO
}


/* ========================
  ===      GETTERS      ===
  ====================== */

// TODO implement public methods


/* =========================
  ===        LOOP        ===
  ======================= */

void gestureDetectionLoop() {
  // update accelerometer and gyroscope output data
  getMpuValues();

  // TODO implement state logic
  switch (deviceState) {
    case IDLE:
      break;
    case ACTIVATING:
      break;
    case ACTIVATED:
      break;
    case DETECTING:
      break;
    case DETECTED:
      break;
  }
}
