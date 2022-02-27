#include "node.h"

Point node("node");

void setNodeTags() {
  node.clearTags();
  node.addTag(F("MAC"), WiFi.macAddress());
  node.addTag(F("name"), config.getString("name"));
  node.addTag(F("location"), config.getString("location"));
  node.addTag(F("firmware"), FIRMWARE_VERSION);
}

void captureNodeFields() {
  node.clearFields();
  node.addField(F("rssi"), WiFi.RSSI());
  node.addField(F("uptime"), millis() - setupMillis);
  node.addField(F("freeHeap"), ESP.getFreeHeap());
  node.addField(F("heapFragmentation"),
                100 - ESP.getMaxAllocHeap() * 100.0 / ESP.getFreeHeap());
}
