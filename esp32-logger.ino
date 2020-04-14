#include "definitions.h"

#include <WiFi.h>
#include <WString.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Adafruit_BMP280.h>
#include <DHTesp.h>

#include "memory.h"
#include "network.h"
#include "node.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "uk.pool.ntp.org", 0, 1000);
Node* node;
Adafruit_BMP280 bmp;
boolean hasBmp = false;
DHTesp dht;
boolean hasDht = false;

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

  hasBmp = bmp.begin(0x76);
  if (hasBmp) {
    Serial.println("[ BMP ] sensor ok");
  } else {
    Serial.println("[ BMP ] sensor NOT ok");
  }
#ifdef DHT11_PIN
  dht.setup(DHT11_PIN, DHTesp::DHT11); hasDht = true;
#endif
  if (hasDht) {
    Serial.println("[ DHT ] sensor ok");
  } else {
    Serial.println("[ DHT ] sensor NOT ok");
  }

  node = new Node();
  initNtp();
}

#define SEALEVELPRESSURE_HPA (1024.50)

void loop() {
  node->checkWifi();

  if (hasBmp) {
    node->log("temperature", bmp.readTemperature());
    node->log("pressure", bmp.readPressure() / 100.0F);
  }
  if (hasDht) {
    TempAndHumidity reading = dht.getTempAndHumidity();
    node->log("temperature", reading.temperature);
    node->log("humidity", reading.humidity);
  }
  node->log("light", analogRead(LIGHT_SENSOR_PIN));

  /*
  Serial.print("temperature: ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");

  Serial.print("pressure: ");
  Serial.print(bmp.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("altitude: ");
  Serial.print(bmp.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("light: ");
  Serial.println(analogRead(LIGHT_SENSOR_PIN));
  */
  node->sleep();
  node->wake();
}
