#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[] = { 0xA9, 0x92, 0x05, 0xED, 0x77, 0x52 }; // can be random as long as it's unique
//IPAddress ip(37, 251, 12, 191);
IPAddress ip(168, 2, 1, 1);
IPAddress server(131, 211, 12, 29);

EthernetClient ethClient;
PubSubClient client(ethClient);


void setup() {
  // for logging purposes
  Serial.begin(9600);

  client.setServer(server, 1883);
  client.setCallback(callback);
  Ethernet.begin(mac, ip);

  // allow the MQTT client to sort itself out
  delay(3000);
  digitalWrite(LED_BUILTIN, LOW);

  // TODO remove test code below
  Serial.println(Ethernet.localIP());
  Serial.println(Ethernet.hardwareStatus());
  Serial.println(Ethernet.linkStatus());
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
}


// TODO move to different file
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// TODO move to different file
void reconnect() {
  // TODO make non-blocking

  // loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // attempt to connect
    if (client.connect("arduinoClient", "Dx3n2WLW", "student099")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("infob3it/099/status/plant_drizzler", "Online");
      // ... and resubscribe
      client.subscribe("infob3it/099/#");
      // TODO disable error light
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {
  // make sure we are connected to the MQTT broker
  if (!client.connected()) {
    // TODO enable error light
    reconnect();
  }

  // required for processing incoming messages, etc.
  client.loop();
}
