// stuff to include:
#include "Plant_Drizzler_Gestures.h"

int waterButtonPin = 11;
int refreshButtonPin = 12;
bool prevWaterState = LOW;
bool prevRefreshState = LOW;

// interval at which buttons get checked
BlockNot waterButtonInterval(50);
BlockNot refreshButtonInterval(50);


void setup() {
  // for logging purposes
  Serial.begin(9600);
}


void loop() {
  // we only use millis() for time, so curTime is clear enough
  unsigned long curTime = millis();

  // let deviceFunction update our inputs
  deviceLoop(curTime);

  bool waterButtonState = digitalRead(waterButtonPin);
  bool refreshButtonState = digitalRead(refreshButtonPin);

  if (waterButtonState != prevWaterState && waterButtonInterval.triggered()) {
    prevWaterState = waterButtonState;
    digitalWrite(LED_BUILTIN, !refreshButtonState || !waterButtonState);

    if (!waterButtonState) {
      // communicate the command to node-red through the serial connection
      Serial.println("water");
    }
  }

  if (refreshButtonState != prevRefreshState && refreshButtonInterval.triggered()) {
    prevRefreshState = refreshButtonState;
    digitalWrite(LED_BUILTIN, !refreshButtonState || !waterButtonState);

    if (!refreshButtonState) {
      // communicate the command to node-red through the serial connection
      Serial.println("refresh");
    }
  }
}
