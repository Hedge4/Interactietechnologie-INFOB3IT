#include "mqttConnection.h"


/* ========================
  ===   CONFIGURATION   ===
  ====================== */

WiFiClient espClient;
PubSubClient client(espClient);

// convert to string, concatenate, convert back to char*
const char* statusTopic = (std::string(MQTT_TOPIC_PREFIX) + "/status/plant_drizzler").c_str();
const char* subcribeTopic = (std::string(MQTT_TOPIC_PREFIX) + "/#").c_str();


/* ========================
  ===     FUNCTIONS     ===
  ====================== */

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived: [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void mqttReconnect() {
  // TODO probably better to make this non-blocking

  // loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection... ");
    // attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQQT_PASSWORD, statusTopic, 0, true, "Offline", true)) {
      Serial.println("connected!");
      // Once connected, publish an announcement...
      client.publish(statusTopic, "Online", true);
      // ... and resubscribe
      client.subscribe(subcribeTopic);
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

void setupMqqt() {
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
}

void sendMessage(char *payload) {
  sendMessage(payload, "");
}

void sendMessage(char *payload, char *topic) {
  const char* topic = (std::string(MQTT_TOPIC_PREFIX) + topic).c_str();
  client.publish(topic, payload);
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
