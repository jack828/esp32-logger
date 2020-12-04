#include "credentials.h"
#include "definitions.h"

#include <InfluxDbClient.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB);

Point node("node");
Point sensors("sensors");
int setupMillis;

void setup() {
  Serial.begin(115200);
  setupMillis = millis();

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PSK);

  Serial.print("[ WIFI ] Connecting");
  int retryCount = 0;
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    delay(250);
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    delay(250);
    if (retryCount++ > 20) {
      Serial.println("\n[ WIFI ] ERROR: Could not connect to wifi, rebooting...");
      ESP.restart();
    }
  }
  Serial.print("\n[ WIFI ] connected, SSID: ");
  Serial.print(WiFi.SSID());
  Serial.print(", IP:");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Add constant tags - only once
  node.addTag("MAC", WiFi.macAddress());
  node.addTag("SSID", WiFi.SSID());

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("[ INFLUX ] Connected to: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("[ INFLUX ] Connection failed: ");
    Serial.println(client.getLastErrorMessage());
    ESP.restart();
  }
}

int failedCount = 0;
int delayTime;

void log (Point& point) {
  Serial.print("[ INFLUX ] Writing: ");
  Serial.println(client.pointToLineProtocol(point));
  if (!client.writePoint(point)) {
    Serial.print("[ INFLUX ] Write failed: ");
    Serial.println(client.getLastErrorMessage());
    failedCount++;
    if (failedCount > 5) {
      Serial.println("[ NODE ] Failed too often, restarting");
      ESP.restart();
    } else {
      // wait a shorter time before trying again
      delayTime = 10 * 1000;
    }
  }
}

void loop() {
  delayTime = 60 * 1000;
  node.clearFields();
  node.addField("rssi", WiFi.RSSI());
  node.addField("uptime", millis() - setupMillis);

  if ((WiFi.RSSI() == 0) && (wifiMulti.run() != WL_CONNECTED)) {
    Serial.println("[ WIFI ] connection lost :( ");
    ESP.restart();
  }

  log(node);

  Serial.println("[ NODE ] Waiting...");
  delay(delayTime);
}
