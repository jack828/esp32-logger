#include "definitions.h"

#include <WString.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <Adafruit_BMP280.h>

#include "memory.h"
#include "network.h"
#include "node.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "uk.pool.ntp.org", 0, 1000);
Node* node;
Adafruit_BMP280 bmp;

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
  boolean status = bmp.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
  }
  node = new Node();
  initNtp();

}

#define SEALEVELPRESSURE_HPA (1024.50)

void loop() {
  // TODO sensors
  // node->logTemperature();
  node->log("temperature", bmp.readTemperature());
  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bmp.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print(timeClient.getFormattedTime());
  Serial.print("\t");
  Serial.println(node->readLight());

  /* node->sleep(); */
  /* node->wake(); */
  delay(5 * 1000);
}
