#include "credentials.h"
#include "definitions.h"

#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <EEPROM.h>

#include "memory.h"
#include "network.h"

/*
 * How this will work:
 * 1. Boot & connect to WiFi
 * 2. Authenticate node using mac address with log server
 * 3. Record sensors with log server
 * 4. Sleep for some duration
 * 5. Wake, record readings with log server
 * 6. Repeat 3-4
 */


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "uk.pool.ntp.org", 0, 1000);
WiFiMulti wifi;

class Node {
  private:
    String rootUri;
    String nodeId;
    long lastIdentified;
  public:
    int identifier;
    Node(String rootUri) {
      this->rootUri = rootUri;
      this->nodeId = WiFi.macAddress();
      Serial.println("[ NODE ] ID: " + this->nodeId);
      Serial.println("[ NODE ] rootUri: " + this->rootUri);
      this->identify();
    }
    virtual void identify() {
      Serial.println("[ NODE ] identify");
      String identifyUrl = this->rootUri + "/identify/" + this->nodeId;
      String identifyResponse = getRequest(identifyUrl);

      Serial.print("Got time: ");
      Serial.println(timeClient.getFormattedTime());
      Serial.print("Got epoch: ");
      Serial.println(timeClient.getEpochTime());

      this->lastIdentified = timeClient.getEpochTime();
      if (!identifyResponse.equals(this->nodeId)) {
        // TODO now it's an issue :shrug:
        Serial.println("[ NODE ] ID mismatch");
      }
      Serial.println("[ NODE ] ID Check OK");
    }
};

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
  EEPROM.begin(512);

  pinMode(LED_PIN, OUTPUT);
  connectWifi();
  initNtp();

  Node *node = new Node("http://192.168.0.17:3001/node");

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
  int lightRaw = analogRead(LIGHT_SENSOR_PIN);
  Serial.println(timeClient.getFormattedTime());
  Serial.println(lightRaw);
  delay(10000);
}
