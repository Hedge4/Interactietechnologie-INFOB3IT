#ifndef MQTT_CONNECTION_H
#define MQTT_CONNECTION_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "auth.h"

// public methods
void setupWifi();
void setupMqtt();
boolean mqttLoop();
void sendMessage(char* message);
void sendMessage(const char* message, const char* topic);


#endif /*MQTT_CONNECTION_H*/
