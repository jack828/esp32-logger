#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "credentials.h"
#include "definitions.h"

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
    delay(1000);
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

void setup() {
  Serial.begin(115200);
  Serial.println("Booted.");
  connectWifi();
  initNtp();
}

void loop() {
  if (wifi.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
  }
}
