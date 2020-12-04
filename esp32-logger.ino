#include "credentials.h"
#include "definitions.h"

#include <InfluxDbClient.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;


InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB);

Point sensor("wifi");

void setup() {
  Serial.begin(115200);

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
  sensor.addTag("MAC", WiFi.macAddress());
  sensor.addTag("SSID", WiFi.SSID());

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

void loop() {
  sensor.clearFields();
  sensor.addField("rssi", WiFi.RSSI());

  Serial.print("[ INFLUX ] Writing: ");
  Serial.println(client.pointToLineProtocol(sensor));
  if ((WiFi.RSSI() == 0) && (wifiMulti.run() != WL_CONNECTED)) {
    Serial.println("[ WIFI ] connection lost :( ");
    ESP.restart();
  }
  if (!client.writePoint(sensor)) {
    Serial.print("[ INFLUX ] Write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  Serial.println("[ NODE ] Waiting...");
  delay(10000);
}
