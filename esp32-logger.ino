#include "credentials.h"
#include "definitions.h"

#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "uk.pool.ntp.org", 0, 1000);
WiFiMulti wifi;

void connectWifi() {
  wifi.addAP(WIFI_SSID_1, WIFI_PSK_1);

  Serial.println("Connecting WiFi...");
  if (wifi.run() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Not connected, rebooting...");
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    delay(250);
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    delay(250);
    ESP.restart();
  }
}

void initNtp() {
  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  Serial.print("Got time: ");
  Serial.println(timeClient.getFormattedTime());
  Serial.print("Got epoch: ");
  Serial.println(timeClient.getEpochTime());
}

String readTemperature() {
  int raw = analogRead(13);
  Serial.print("RAW: ");
  Serial.println(raw);
  double voltage = (raw / 2048.0) * 3300;
  Serial.print("Voltage: ");
  Serial.println(voltage);
  double tempC = voltage * 0.1;
  return String(tempC);
}


void setup() {
  Serial.begin(115200);
  Serial.println("Booted.");
  pinMode(LED_PIN, OUTPUT);
  connectWifi();
  initNtp();
}

void loop() {
  if (wifi.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
  }
  Serial.print("Temp: ");
  Serial.println(readTemperature());
  int lightRaw = analogRead(LIGHT_SENSOR_PIN);
  Serial.println(lightRaw);
  delay(100);
}
