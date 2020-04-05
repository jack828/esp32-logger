#include "credentials.h"
#include "definitions.h"

#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <HTTPClient.h>

/*
 * How this will work:
 * 1. Boot & connect to WiFi
 * 2. Authenticate node with log server
 * 3. Sleep for some duration
 * 4. Wake, record readings with log server
 * 5. Repeat 3-4
 */

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "uk.pool.ntp.org", 0, 1000);
WiFiMulti wifi;

void getRequest(String uri) {
  HTTPClient http;

  Serial.print("[HTTP] begin...\n");

  http.begin(uri);

  Serial.print("[HTTP] GET...\n");
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

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
  Serial.print("RAW: "); Serial.println(raw);
  double voltage = (raw / 2048.0) * 3300;
  Serial.print("Voltage: "); Serial.println(voltage);
  double tempC = voltage * 0.1;
  return String(tempC);
}


void setup() {
  Serial.begin(115200);
  Serial.println("Booted.");
  pinMode(LED_PIN, OUTPUT);
  connectWifi();
  initNtp();
  /* getRequest("http://192.168.0.17:3001/node/test-identifier"); */

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
    Serial.println("WiFi not connected!");
  }
  int lightRaw = analogRead(LIGHT_SENSOR_PIN);
  Serial.println(lightRaw);
  delay(100);
}
