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

class Node {
  private:
    String rootUri;
    String nodeId;
  public:
    int identifier;
    Node(String rootUri) {
      this->rootUri = rootUri;
      this->nodeId = readDeviceId();
      Serial.println("[ NODE ] ID: " + this->nodeId);
      this->identify();
    }
    virtual void identify() {
      Serial.println(" [ NODE ] identify");
      Serial.println(this->rootUri);
      String identifyUrl = this->rootUri + "/identify/" + this->nodeId;
      String identifyResponse = getRequest(identifyUrl);
      if (!identifyResponse.equals(this->nodeId)) {
        Serial.println("[ NODE ] ID mismatch");
        writeDeviceId(identifyResponse);
        this->nodeId = readDeviceId();
        Serial.println("[ NODE ] ID: " + this->nodeId);
      }
      Serial.println("[ NODE ] ID Check OK");
    }
};

/*
 * How this will work:
 * 1. Boot & connect to WiFi
 * 2. Authenticate node with log server
 *  2.1 If UUID present, send it to auth with root
 *  2.2 If no UUID present, get one from root
 * 3. Sleep for some duration
 * 4. Wake, record readings with log server
 * 5. Repeat 3-4
 */

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
  Serial.print("RAW: "); Serial.println(raw);
  double voltage = (raw / 2048.0) * 3300;
  Serial.print("Voltage: "); Serial.println(voltage);
  double tempC = voltage * 0.1;
  return String(tempC);
}


void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  /* Serial.println("Booted."); */
  pinMode(LED_PIN, OUTPUT);
  connectWifi();
  initNtp();

  /* writeDeviceId("test"); */

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
    /* Serial.println("WiFi not connected!"); */
  }
  int lightRaw = analogRead(LIGHT_SENSOR_PIN);
  Serial.println(lightRaw);
  delay(10000);
}
