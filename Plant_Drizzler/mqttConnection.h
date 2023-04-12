#ifndef MQTT_CONNECTION_H
#define MQTT_CONNECTION_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "Plant_Drizzler.h"
#include "auth.h"


// public methods
void setupWifi();
void setupMqtt();
boolean mqttLoop();
void sendMessage(const char* message, String topic);



void setupSubscriptions();

#endif /*MQTT_CONNECTION_H*/
