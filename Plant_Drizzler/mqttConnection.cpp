#include "mqttConnection.h"


/* ========================
  ===   CONFIGURATION   ===
  ====================== */

WiFiClient espClient;
PubSubClient client(espClient);

// convert to string and concatenate
const String statusTopic = String(MQTT_TOPIC_PREFIX) + "/status/plant_drizzler";
const String subscribeTopic = String(MQTT_TOPIC_PREFIX) + "/#";

//hardcoded topic strings
String commandTopicPrefixed = String(MQTT_TOPIC_PREFIX) +"/commands";
String modeTopic            = "/mode/plant_drizzler";
String modeTopicPrefixed    = String(MQTT_TOPIC_PREFIX) +"/mode/plant_drizzler";
String moistReadingTopic    = String(MQTT_TOPIC_PREFIX) + "/sensor/moisture/reading";
String moistLevelTopic      = String(MQTT_TOPIC_PREFIX) + "/sensor/moisture/level";
String lightReadingTopic    = String(MQTT_TOPIC_PREFIX) + "/sensor/light/reading";
String lightLevelTopic      = String(MQTT_TOPIC_PREFIX) + "/sensor/light/level";
String pressureTopic        = String(MQTT_TOPIC_PREFIX) + "/sensor/pressure";
String temperatureTopic     = String(MQTT_TOPIC_PREFIX) + "/sensor/temperature";


/* ========================
  ===     FUNCTIONS     ===
  ====================== */

//TODO: write callback functions for each topic
void callback(char* topic, byte* payload, unsigned int length) {
  //log incoming messages
  Serial.print("Message arrived: [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();


  //decide what to do based on topic
  //convert char* to string
  String inTopic = topic;
  if(inTopic == commandTopicPrefixed){
    //command payloads should be single byte
    performCommand(payload[0]);
  }
  else if(inTopic == modeTopicPrefixed){
    performModeToggle(payload[0]);
  }

  
}

void mqttReconnect() {
  // TODO probably better to make this non-blocking

  // loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection... ");
    // attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, statusTopic.c_str(), 0, true, "Offline")) {
      Serial.println("connected!");
      // Once connected, publish an announcement...
      client.publish(statusTopic.c_str(), "Online", true);
      // ... and resubscribe
      client.subscribe(subscribeTopic.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" - trying again in 5 seconds");
      // wait 5 seconds before retrying
      delay(5000);
    }
  }
}


/* =========================
  ===  PUBLIC FUNCTIONS  ===
  ======================= */

void setupWifi() {
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // TODO this is blocking, which it probably shouldn't be
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void setupMqtt() {
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  client.setSocketTimeout(30);
  client.setKeepAlive(60);
}

// sendMessage but with a default topic
void sendMessage(const char *payload, bool retain) { 
  sendMessage(payload, nullptr, retain); 

  // alternative method I decided to use at first 
  // const char* topic = new char[1]; 
  // topic[0] = '\0'; 
  // sendMessage(payload, topic); 
  // delete topic;

  // another alternative method 
  // static const char* emptyTopic = ""; 
  // sendMessage(payload, emptyTopic); 

  // the last, and probably simplest approach is a default value 
  // for topic in the method below. But I already wrote these...
}

void sendMessage(const char *payload, const char *topic, bool retain) {
  String fullTopic = String(MQTT_TOPIC_PREFIX) + topic;
  client.publish(fullTopic.c_str(), payload, retain);
}


/* =========================
  ===        LOOP        ===
  ======================= */

// returns whether the client is connected or not
boolean mqttLoop() {
  // make sure we are connected to the MQTT broker
  if (!client.connected()) {
    mqttReconnect();
  }

  // required for processing incoming messages, etc.
  client.loop();

  return client.connected();
}





