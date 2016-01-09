#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include "Opt3001.h"

#define wifi_ssid "virginmedia6527155"
#define wifi_password ""

#define mqtt_server "192.168.0.101"
#define mqtt_user ""
#define mqtt_password ""

#define light_topic "/esp210/sensor/light"

WiFiClient espClient;
PubSubClient client(espClient);
Opt3001 opt3001;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  // Set SDA and SDL ports
  Wire.begin(2, 14);
  opt3001.begin(); 
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

long lastMsg = 0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
  
   uint32_t light;
     
  // Read OPT3001
  light = opt3001.readResult(); 
  
  client.publish(light_topic, String(light).c_str(), true);
  }
}
