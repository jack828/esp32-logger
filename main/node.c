#include "node.h"
#include <esp_log.h>
#include <esp_sleep.h>
#include <esp_wifi.h>

/* Node() {
  this->nodeId = WiFi.macAddress();
  ESP_LOGI("[ NODE ] ID: " + this->nodeId);
  ESP_LOGI("[ NODE ] rootUri: " + this->rootUri);
  this->identify();
}*/

static const char *TAG = "[NODE]";

void node_identify() {
  ESP_LOGI(TAG, "identify");
  return;
  /*
  String identifyUrl = ROOT_URI + "/identify/" + this->nodeId;
  Response identifyResponse = getRequest(identifyUrl);

  this->lastIdentified = timeClient.getEpochTime();

  if (!identifyResponse.body.equals(this->nodeId)) {
    // TODO now it's an issue :shrug:
    ESP_LOGI(TAG, "ID mismatch");
  }
  ESP_LOGI(TAG, "ID Check OK");
  */
}

void node_log(char* sensorType, double value) {
  /*
  ESP_LOGI(TAG, (" " + sensorType + ", value: %.2f\n"), value);
  String logUri = this->rootUri +
      "/log/" + this->nodeId + "/" + sensorType + "/" + String(value);


  Response logResponse = getRequest(logUri);
  ESP_LOGI(TAG, "response: %d\n", logResponse.statusCode);
  */
}

void node_sleep() {
  int ret;
  /*
  Serial.printf("[ NODE ] snoozin for %d seconds\n", LOG_PERIOD / 1000 / 1000);
  Serial.flush();
  ret = WiFi.disconnect();
  Serial.printf("wifidc: %d\n", ret);
  ret = esp_wifi_stop();
  Serial.printf("wifistop: %d\n", ret);
  ret = esp_sleep_enable_timer_wakeup(LOG_PERIOD);
  Serial.printf("lst: %d\n", ret);

  esp_deep_sleep_start();
  */
}

void node_wake() {
  /*
  ESP_LOGI("[ NODE ] waking...");
  this->checkWifi();
  // TODO move out
  ESP_LOGI("[ NTP ] updating time");
  timeClient.update();
  ESP_LOGI("[ NODE ] woke " + timeClient.getFormattedTime());
  */
}
