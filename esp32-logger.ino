#include "credentials.h"
#include "definitions.h"

#include <WString.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <HTTPClient.h>

#include "memory.h"
#include "network.h"
#include "node.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "uk.pool.ntp.org", 0, 1000);
Node* node;

void connectWifi() {
  Serial.println("[ WIFI ] connecting WiFi");
  if (WiFi.SSID() != WIFI_SSID) {
    Serial.println(WiFi.SSID());
    Serial.println("[ WIFI ] stored SSID different, beginning WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    WiFi.persistent(true);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
  }

  Serial.print("[ WIFI ] connecting");
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
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
      Serial.println("[ WIFI ] Could not connect, rebooting...");
      ESP.restart();
    }
  }
  Serial.println();
  Serial.println("[ WIFI ] connected");
  Serial.println("[ WIFI ] IP address: " + WiFi.localIP());
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
  node->wake();


  // TODO sensors
  // node->logTemperature();

  Serial.println(timeClient.getFormattedTime());
  Serial.println(node->readLight());

  node->sleep();
  /* delay(10 * 1000); */
}
