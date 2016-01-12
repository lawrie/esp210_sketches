#include <ESP8266WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>
#include "Opt3001.h"
#include <BME280_MOD-1022.h>

#define wifi_ssid "virginmedia6527155"
#define wifi_password ""

#define mqtt_server "192.168.0.101"
#define mqtt_user ""
#define mqtt_password ""

#define light_topic "/esp210/sensor/light"
#define humidity_topic "/esp210/sensor/humidity"
#define temperature_topic "/esp210/sensor/temperature"
#define pressure_topic "/esp210/sensor/pressure"

#define SERIAL

WiFiClient espClient;
PubSubClient client(espClient);
Opt3001 opt3001;

void publish();

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
#ifdef SERIAL  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
#endif
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
#ifdef SERIAL  
    Serial.print(".");
#endif
  }
#ifdef SERIAL 
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif
}

void setup() {
#ifdef SERIAL  
  Serial.begin(115200);
#endif
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  // Set SDA and SDL ports
  Wire.begin(2, 14);
  opt3001.begin(); 

  BME280.readCompensationParams();
  BME280.writeStandbyTime(tsb_0p5ms); 
  BME280.writeFilterCoefficient(fc_16);
  BME280.writeOversamplingPressure(os16x); 
  BME280.writeOversamplingTemperature(os1x);
  BME280.writeOversamplingHumidity(os2x);
  BME280.writeMode(smNormal);

  publish();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
#ifdef SERIAL     
    Serial.print("Attempting MQTT connection...");
#endif    
    // Attempt to connect
    if (client.connect("esp210_"  + millis(), mqtt_user, mqtt_password)) {
#ifdef SERIAL 
      Serial.println("connected");
#endif
    } else {
#ifdef SERIAL       
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      
#endif      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void loop() { 
}

void publish() {
  reconnect();
   
  // Read OPT3001
  uint32_t light = opt3001.readResult();

  // Read BME280
  while (BME280.isMeasuring()) delay(50);
  BME280.readMeasurements();

  float temperature = BME280.getTemperatureMostAccurate();
  float humidity  = BME280.getHumidityMostAccurate();
  float pressure  = BME280.getPressureMostAccurate();
#ifdef SERIAL 
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Pressure: ");
  Serial.println(pressure);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Light: ");
  Serial.println(light);
#endif
  client.publish(temperature_topic, String(temperature).c_str(), true);
  client.publish(humidity_topic, String(humidity).c_str(), true);
  client.publish(pressure_topic, String(pressure).c_str(), true);
  client.publish(light_topic, String(light).c_str(), true);

  client.disconnect();

  ESP.deepSleep(30000000, WAKE_RF_DEFAULT); // Sleep for 30 seconds
}


