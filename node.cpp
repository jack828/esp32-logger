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