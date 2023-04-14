#include "Plant_Drizzler_Gestures.h"


/* ========================
  ===   CONFIGURATION   ===
  ====================== */

int waterButtonPin = 11;
int refreshButtonPin = 12;
bool prevWaterState = LOW;
bool prevRefreshState = LOW;

// interval at which the Arduino sends a ping through its Serial connection to node-red
BlockNot pingInterval(15000);


/* ===========================
  ===   PUBLIC FUNCTIONS   ===
  ========================= */

void sendWaterCommand() {
  Serial.println(F("0"));
}

void sendMoreWaterCommand() {
  Serial.println(F("1"));
}

void sendRefreshCommand() {
  Serial.println(F("2"));
}


/* =========================
  ===     MAIN SETUP     ===
  ======================= */

void setup() {
  // led is on during setup
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // to communicate with node-red
  Serial.begin(9600);

  // mpuConnectionSetup();
  mpuConnectionSetup();

  // disable setup led
  digitalWrite(LED_BUILTIN, LOW);

  // send online status
  Serial.println(F("ping"));
}


/* ========================
  ===     MAIN LOOP     ===
  ====================== */

void loop() {
  // interval not just on getting new data but also gesture recognition, since there's no use detecting without new data
  // let deviceFunction update our inputs
  gestureDetectionLoop();

  // send ping to node-red flow every x seconds
  if (pingInterval.triggered()) {
    Serial.println(F("ping"));
  }
}
