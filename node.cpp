#include "node.h"

Point node("node");

void setupNode() {
  node.addTag(F("MAC"), WiFi.macAddress());
  node.addTag(F("SSID"), WiFi.SSID());
  node.addTag(F("name"), config.getString("name"));
  node.addTag(F("location"), config.getString("location"));
  node.addTag(F("firmware"), FIRMWARE_VERSION);
}

void captureNodeFields() {
  node.clearFields();
  node.addField(F("rssi"), WiFi.RSSI());
  node.addField(F("uptime"), millis() - setupMillis);
  node.addField(F("freeHeap"), ESP.getFreeHeap());
}
