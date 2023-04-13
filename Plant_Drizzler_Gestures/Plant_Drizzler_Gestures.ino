#include "Plant_Drizzler_Gestures.h"


/* ========================
  ===   CONFIGURATION   ===
  ====================== */

int waterButtonPin = 11;
int refreshButtonPin = 12;
bool prevWaterState = LOW;
bool prevRefreshState = LOW;

// interval at which buttons get checked
BlockNot waterButtonInterval(50);
BlockNot refreshButtonInterval(50);
// interval at which accelerometer/gyroscope data is updated, so MPU has time to process
BlockNot mpuLoopInterval(100);


/* ===========================
  ===   PUBLIC FUNCTIONS   ===
  ========================= */

void sendWaterCommand() {
  // TODO
}

void sendMoreWaterCommand() {
  // TODO
}

void sendRefreshCommand() {
  // TODO
}


/* =========================
  ===     MAIN SETUP     ===
  ======================= */

void setup() {
  // led is on during setup
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  // for logging purposes
  Serial.begin(9600);

  // mpuConnectionSetup();
  mpuConnectionSetup();

  // disable setup led
  digitalWrite(LED_BUILTIN, LOW);
}


/* ========================
  ===     MAIN LOOP     ===
  ====================== */

void loop() {
  // interval not just on getting new data but also gesture recognition, since there's no use detecting without new data
  if (mpuLoopInterval.triggered()) {
  // let deviceFunction update our inputs
    gestureDetectionLoop();
  }

  // bool waterButtonState = digitalRead(waterButtonPin);
  // bool refreshButtonState = digitalRead(refreshButtonPin);

  // if (waterButtonState != prevWaterState && waterButtonInterval.triggered()) {
  //   prevWaterState = waterButtonState;
  //   digitalWrite(LED_BUILTIN, !refreshButtonState || !waterButtonState);

  //   if (!waterButtonState) {
  //     // communicate the command to node-red through the serial connection
  //     Serial.println(F("water"));
  //   }
  // }

  // if (refreshButtonState != prevRefreshState && refreshButtonInterval.triggered()) {
  //   prevRefreshState = refreshButtonState;
  //   digitalWrite(LED_BUILTIN, !refreshButtonState || !waterButtonState);

  //   if (!refreshButtonState) {
  //     // communicate the command to node-red through the serial connection
  //     Serial.println(F("refresh"));
  //   }
  // }
}
