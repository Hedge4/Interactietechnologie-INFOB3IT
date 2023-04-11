#include "MQTT_Client.h"

void setup() {
  // on during setup
  digitalWrite(BUILTIN_LED, HIGH);

  // for logging purposes
  Serial.begin(9600);
  delay(500); // serial connection seems to need a long time to start
  Serial.println(); // cut off any half-transmitted data

  setupWifi();
  setupMqqt();

  // allow the MQTT client to sort itself out
  delay(3000);
  digitalWrite(BUILTIN_LED, LOW);
}


void loop() {
  // TODO show some kind of error light if false
  boolean clientConnected = mqttLoop();
}
