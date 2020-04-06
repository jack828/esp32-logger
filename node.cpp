#include "node.h"

Node::Node() {
  this->nodeId = WiFi.macAddress();
  Serial.println("[ NODE ] ID: " + this->nodeId);
  Serial.println("[ NODE ] rootUri: " + this->rootUri);
  this->identify();
}

void Node::identify() {
  Serial.println("[ NODE ] identify");
  String identifyUrl = this->rootUri + "/identify/" + this->nodeId;
  String identifyResponse = getRequest(identifyUrl);

  this->lastIdentified = timeClient.getEpochTime();

  if (!identifyResponse.equals(this->nodeId)) {
    // TODO now it's an issue :shrug:
    Serial.println("[ NODE ] ID mismatch");
  }
  Serial.println("[ NODE ] ID Check OK");
  Serial.printf("Light: %d\n", this->readLight());
}

int Node::readLight() {
  int lightRaw = analogRead(LIGHT_SENSOR_PIN);
  return lightRaw;
}

void Node::sleep() {
  Serial.println("[ NODE ] sleeping...");
  esp_sleep_enable_timer_wakeup(LOG_PERIOD);
  // Make sure we've written everything else
  Serial.flush();
  WiFi.disconnect();

  int ret = esp_light_sleep_start();
  Serial.printf("esp_light_sleep_start: %d\n", ret);
  if (ret != 0) {
    Serial.println("Error entering light sleep");
  }
}

void Node::wake() {
  Serial.println("[ NODE ] waking...");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ WIFI ] not connected!");
    /* connectWifi(); */
  }
  timeClient.update();
}
