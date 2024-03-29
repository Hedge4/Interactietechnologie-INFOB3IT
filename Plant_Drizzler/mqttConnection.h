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
bool setupWifiInLoop();

void sendMessage(const char *payload, bool retain = false);
void sendMessage(const char* message, const char *topic, bool retain = false);

extern String modeTopic,          modeSmartTopic,
              commandTopic, 
              moistReadingTopic,  moistLevelTopic, 
              lightReadingTopic,  lightLevelTopic,
              pressureTopic,      temperatureTopic,
              lastWateredTopic; 
//extern const String subscribeTopic, statusTopic;              


#endif /*MQTT_CONNECTION_H*/
