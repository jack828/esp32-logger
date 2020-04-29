#include "definitions.h"

#include <WiFi.h>
#include <WString.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "memory.h"
#include "network.h"
#include "node.h"

#ifdef BME280_I2C
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;
#endif
#ifdef BMP280_I2C
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp;
extern double temperature;
extern double pressure;
#endif
#ifdef BH1750_I2C
#include <Wire.h>
#include <BH1750.h>
BH1750 lightMeter(0x23);
extern double lux;
#endif
#ifdef DHT11_PIN
#include <DHTesp.h>
DHTesp dht;
TempAndHumidity reading;
#endif
#ifdef LIGHT_SENSOR_PIN
extern int lightLevel;
#endif
#ifdef OLED
#include "oled.h"
long lastLog = 0l;
#endif

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "uk.pool.ntp.org", 0, 1000);
Node* node;

void printMem(String marker) {
  Serial.print(" [MEM] ");
  Serial.print(marker);
  Serial.print(" ");
  Serial.print(ESP.getFreeHeap());
  Serial.print(" / ");
  Serial.println(ESP.getHeapSize());
}

void initNtp() {
  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  Serial.print("[ NTP ] time: ");
  Serial.println(timeClient.getFormattedTime());
  Serial.print("[ NTP ] epoch: ");
  Serial.println(timeClient.getEpochTime());
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  Serial.print("Flash size: ");
  Serial.println(ESP.getFlashChipSize());

#ifdef BME280_I2C
  Serial.println("[ BME ] has sensor");
  boolean bmeOk = bme.begin(0x76);
  Serial.print("[ BME ] sensor ");
  Serial.print(bmeOk ? "" : "NOT ");
  Serial.println("OK");
#endif

#ifdef BMP280_I2C
  Serial.println("[ BMP ] has sensor");
  boolean bmpOk = bmp.begin(0x76);
  Serial.print("[ BMP ] sensor ");
  Serial.print(bmpOk ? "" : "NOT ");
  Serial.println("OK");
#endif

#ifdef BH1750_I2C
  Wire.begin();// ONE_TIME_HIGH_RES_MODE
  if (lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE)) {
    Serial.println("[ LUX ] sensor ok");
  } else {
    Serial.println("[ LUX ] sensor NOT ok");
  }
#endif

#ifdef DHT11_PIN
  dht.setup(DHT11_PIN, DHTesp::DHT11);
  Serial.println("[ DHT ] sensor ok");
#endif

#ifdef OLED
  initOled();
#else
  Serial.println("[ OLED ] not present");
#endif
  node = new Node();
  initNtp();
}

void logSensors() {
#ifdef BME280_I2C
  temperature = bme.readTemperature();
  pressure = bme.readPressure() / 100.0F;
  humidity = bme.readHumidity();
  node->log("temperature", temperature);
  node->log("pressure", pressure);
  node->log("humidity", humidity);
#endif
#ifdef BMP280_I2C
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure() / 100.0F;
  node->log("temperature", temperature);
  node->log("pressure", pressure);
#endif
#ifdef BH1750_I2C
  lux = lightMeter.readLightLevel(true);
  node->log("light", lux);
#endif
#ifdef LIGHT_SENSOR_PIN
  lightLevel = analogRead(LIGHT_SENSOR_PIN);
  node->log("light", lightLevel);
#endif
#ifdef DHT11_PIN
  reading = dht.getTempAndHumidity();
  node->log("temperature", reading.temperature);
  node->log("humidity", reading.humidity);
#endif
}

void loop() {
  node->wake();

#ifdef OLED
  while(1) {
    /* Serial.printf("%d, %ul\n", millis() - lastLog, LOG_PERIOD); */
    if (millis() - lastLog > LOG_PERIOD / 1000) {
      lastLog = millis();
      logSensors();
      Serial.println("logging sensors");
    }
    updateOled(); // this will delay for the update period
  }
#else
  logSensors();
#endif

  node->sleep();
}
