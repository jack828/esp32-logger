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

  node = new Node();
  initNtp();
}

void loop() {
  // TODO sensors
  // node->logTemperature();

  Serial.print(timeClient.getFormattedTime());
  Serial.print("\t");
  Serial.println(node->readLight());

  /* node->sleep(); */
  /* node->wake(); */
  delay(10 * 1000);
}
