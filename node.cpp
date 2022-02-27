#include "node.h"
#include "definitions.h"
#include "influx.h"

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

/**
 * Task: Wait the appropriate period, and log the `node` fields.
 *
 * If logging fails, it will wait progressively smaller portions of time.
 */
void nodeLoggerTask(void *parameters) {
  double delayTime = NODE_LOG_PERIOD;
  for (;;) {
    Serial.println(F("[ NODE ] Logger task"));
    captureNodeFields();
    bool logOk = logPoint(node);
    if (!logOk) {
      // On failure, Decrease delay by 25%, 1 sec minimum
      delayTime = min(delayTime * 0.75, 1000.0);
    } else {
      // On success, reset delay to normal
      delayTime = NODE_LOG_PERIOD;
    }
    Serial.println(F("[ NODE ] Waiting..."));
    vTaskDelay(NODE_LOG_PERIOD / portTICK_PERIOD_MS);
  }
}
