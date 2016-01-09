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

  Serial.println("OPT3001 Initialized----------------------------------");

  unsigned int readings = 0;
  // get manufacturer ID from OPT3001. Default = 0101010001001001
  readings = opt3001.readManufacturerId();  
  Serial.print("Manufacturer ID: "); 
  Serial.println(readings, BIN);

  // get device ID from OPT3001. Default = 0011000000000001
  readings = opt3001.readDeviceId();  
  Serial.print("Device ID: "); 
  Serial.println(readings, BIN);
  
  // read config register from OPT3001. Default = 1100110000010000
  readings = opt3001.readConfigReg();  
  Serial.print("Configuration Register: "); 
  Serial.println(readings, BIN);

  // read Low Limit register from OPT3001. Default = 0000000000000000
  readings = opt3001.readLowLimitReg();  
  Serial.print("Low Limit Register: "); 
  Serial.println(readings, BIN);
  
  // read High Limit register from OPT3001. Default = 1011111111111111
  readings = opt3001.readHighLimitReg();  
  Serial.print("High Limit Register: "); 
  Serial.println(readings, BIN);  
  
  Serial.println("\nOPT3001 READINGS-------------------------------------");


  Serial.println("BMP280 Compensation parameters");

    // need to read the NVM compensation parameters
  BME280.readCompensationParams();

    // Need to turn on 1x oversampling, default is os_skipped, which means it doesn't measure anything
  BME280.writeOversamplingPressure(os1x);  // 1x over sampling (ie, just one sample)
  BME280.writeOversamplingTemperature(os1x);
  BME280.writeOversamplingHumidity(os1x);
  
  // example of a forced sample.  After taking the measurement the chip goes back to sleep
  BME280.writeMode(smForced);
  while (BME280.isMeasuring()) {
    delay(50);
  }
  BME280.readMeasurements();

  BME280.writeStandbyTime(tsb_0p5ms);        // tsb = 0.5ms
  BME280.writeFilterCoefficient(fc_16);      // IIR Filter coefficient 16
  BME280.writeOversamplingPressure(os16x); 
  BME280.writeOversamplingTemperature(os1x);
  BME280.writeOversamplingHumidity(os2x);
  
  Serial.println("BMP280 Normal mode");

  BME280.writeMode(smNormal);

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
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
  if (now - lastMsg > 5000) {
    lastMsg = now;

    Serial.println("Read light");
  
    uint32_t light = 0;
     
    // Read OPT3001
    light = opt3001.readResult();
    
    Serial.println("BME280 Read measurements");
    while (BME280.isMeasuring()) delay(50);
    BME280.readMeasurements();

    float temperature = BME280.getTemperatureMostAccurate();
    float humidity  = BME280.getHumidityMostAccurate();
    float pressure  = BME280.getPressureMostAccurate();

    Serial.print("Temperature: ");
    Serial.println(temperature);
    Serial.print("Pressure: ");
    Serial.println(pressure);
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.print("Light: ");
    Serial.println(light);
  
    client.publish(temperature_topic, String(temperature).c_str(), true);
    client.publish(humidity_topic, String(humidity).c_str(), true);
    client.publish(pressure_topic, String(pressure).c_str(), true);
    client.publish(light_topic, String(light).c_str(), true);
  }
}
