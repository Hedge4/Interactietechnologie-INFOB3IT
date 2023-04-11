#ifndef MQTT_CONNECTION_H
#define MQTT_CONNECTION_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string>
#include "auth.h"

// public methods
void setupWifi();
void setupMqqt();
boolean mqttLoop();
void sendMessage(char* message);
void sendMessage(char* message, char* topic);


#endif /*MQTT_CONNECTION_H*/
