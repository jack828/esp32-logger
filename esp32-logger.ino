#include "credentials.h"
#include "definitions.h"

#include <WString.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <HTTPClient.h>

#include "memory.h"
#include "network.h"
#include "node.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "uk.pool.ntp.org", 0, 1000);
WiFiMulti wifi;
Node* node;

void connectWifi() {
  wifi.addAP(WIFI_SSID_1, WIFI_PSK_1);

  Serial.println("[ WIFI ] connecting");

  if (wifi.run() == WL_CONNECTED) {
    Serial.println("[ WIFI ] connected");
    Serial.println("[ WIFI ] IP address: " + WiFi.localIP());
  } else {
    Serial.println("[ WIFI ] Not connected, rebooting...");
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
  Serial.print("[ NTP ] time: ");
  Serial.println(timeClient.getFormattedTime());
  Serial.print("[ NTP ] epoch: ");
  Serial.println(timeClient.getEpochTime());
}

String readTemperature() {
  int raw = analogRead(13);
  Serial.print("RAW: "); Serial.println(raw);
  double voltage = (raw / 2048.0) * 3300;
  Serial.print("Voltage: "); Serial.println(voltage);
  double tempC = voltage * 0.1;
  return String(tempC);
}


void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  connectWifi();
  initNtp();

  node = new Node();

  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
  delay(200);
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  if (wifi.run() != WL_CONNECTED) {
    Serial.println("[ WIFI ] not connected!");
  }
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  Serial.println(timeClient.getFormattedTime());
  Serial.println(node->readLight());
  delay(10 * 1000);
}
