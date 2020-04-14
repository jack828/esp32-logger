#include "definitions.h"

#include <WiFi.h>
#include <WString.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "memory.h"
#include "network.h"
#include "node.h"

#ifdef BME280_I2C
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp;
#endif
#ifdef DHT11_PIN
#include <DHTesp.h>
DHTesp dht;
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
  Serial.println("[ BMP ] has sensor");
  boolean bmpOk = bmp.begin(0x76);
  Serial.print("[ BMP ] sensor ");
  Serial.print(bmpOk ? "" : "NOT ");
  Serial.println("OK");
#else
    Serial.println("[ BMP ] sensor NOT ok");
#endif

#ifdef DHT11_PIN
  dht.setup(DHT11_PIN, DHTesp::DHT11);
  Serial.println("[ DHT ] sensor ok");
#else
  Serial.println("[ DHT ] sensor NOT ok");
#endif

  node = new Node();
  initNtp();
}

void loop() {
  node->checkWifi();

#ifdef BME280_I2C
    node->log("temperature", bmp.readTemperature());
    node->log("pressure", bmp.readPressure() / 100.0F);
#endif
#ifdef DHT11_PIN
    TempAndHumidity reading = dht.getTempAndHumidity();
    node->log("temperature", reading.temperature);
    node->log("humidity", reading.humidity);
#endif

  node->log("light", analogRead(LIGHT_SENSOR_PIN));

  node->sleep();
  node->wake();
}
