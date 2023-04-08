// stuff to include:
#include "Plant_Drizzler_Gestures.h"

int waterButtonPin = 11;
int refreshButtonPin = 12;
int ledPin = 13;
bool prevWaterState = LOW;
bool prevRefreshState = LOW;


void setup() {
  pinMode(waterButtonPin, INPUT);
  pinMode(refreshButtonPin, INPUT);
  pinMode(ledPin, OUTPUT);

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
  
  if (waterButtonState != prevWaterState) {
    prevWaterState = waterButtonState;
    digitalWrite(ledPin, !waterButtonState || !refreshButtonState);

    if (!waterButtonState) {
      Serial.println("Water command sent!");

      // ! send MQTT command
    }
  }

  if (refreshButtonState != prevRefreshState) {
    prevRefreshState = refreshButtonState;
    digitalWrite(ledPin, !refreshButtonState || !waterButtonState);

    if (!refreshButtonState) {
      Serial.println("Read values command sent!");

      // ! send MQTT command
    }
  }
}
