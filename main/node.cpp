#include "node.h"
#include <esp_sleep.h>
#include <esp_wifi.h>

Node::Node() {
  this->nodeId = WiFi.macAddress();
  Serial.println("[ NODE ] ID: " + this->nodeId);
  Serial.println("[ NODE ] rootUri: " + this->rootUri);
  this->initWifi();
  this->connectWifi();
  this->identify();
}

void Node::identify() {
  Serial.println("[ NODE ] identify");
  String identifyUrl = this->rootUri + "/identify/" + this->nodeId;
  Response identifyResponse = getRequest(identifyUrl);

  this->lastIdentified = timeClient.getEpochTime();

  if (!identifyResponse.body.equals(this->nodeId)) {
    // TODO now it's an issue :shrug:
    Serial.println("[ NODE ] ID mismatch");
  }
  Serial.println("[ NODE ] ID Check OK");
}

void Node::log(String sensorType, double value) {
  Serial.printf(("[ NODE ] [ LOG ] " + sensorType + ", value: %.2f\n").c_str(), value);
  String logUri = this->rootUri +
      "/log/" + this->nodeId + "/" + sensorType + "/" + String(value);


  Response logResponse = getRequest(logUri);
  Serial.printf("[ NODE ] [ LOG ] response: %d\n", logResponse.statusCode);
}

void Node::initWifi() {
  Serial.println("[ NODE ] [ WIFI ] init SSID: " + WiFi.SSID() + "");

  WiFi.mode(WIFI_STA);
  WiFi.persistent(true);
  WiFi.setAutoReconnect(true);

  if (WiFi.SSID() != WIFI_SSID) {
    Serial.println("[ NODE ] [ WIFI ] no stored SSID, init WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PSK);
  }
}

void Node::connectWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[ NODE ] [ WIFI ] connected");
    return;
  }

  Serial.print("[ NODE ] [ WIFI ] connecting");
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
      Serial.println("\n[ NODE ] ERROR: Could not connect to wifi, rebooting...");
      ESP.restart();
    }
  }
  Serial.print("\n[ WIFI ] connected, SSID: ");
  Serial.print(WiFi.SSID());
  Serial.print(", IP:");
  Serial.println(WiFi.localIP());
#ifdef ESP_32_OLED_BATT
  digitalWrite(LED_PIN, HIGH);
#endif // annoying, i know
}

void Node::checkWifi() {
  this->wifiStatus = WiFi.status();
  Serial.println("[ NODE ] [ WIFI ] check SSID: " + WiFi.SSID());
  Serial.printf("[ NODE ] [ WIFI ] status '%d'\n", this->wifiStatus);
  if (this->wifiStatus != WL_CONNECTED) {
    Serial.println("[ NODE ] [ WIFI ] not connected!");
    // TODO hasn't saved wifi connection to chip before sleep
    this->initWifi();
    this->connectWifi();
  }
}

void Node::sleep() {
  int ret;
  Serial.printf("[ NODE ] snoozin for %d seconds\n", LOG_PERIOD / 1000 / 1000);
  Serial.flush();
  ret = WiFi.disconnect();
  Serial.printf("wifidc: %d\n", ret);
  ret = esp_wifi_stop();
  Serial.printf("wifistop: %d\n", ret);
  ret = esp_sleep_enable_timer_wakeup(LOG_PERIOD);
  Serial.printf("lst: %d\n", ret);

  esp_deep_sleep_start();
}

void Node::wake() {
  Serial.println("[ NODE ] waking...");
  this->checkWifi();
  // TODO move out
  Serial.println("[ NTP ] updating time");
  timeClient.update();
  Serial.println("[ NODE ] woke " + timeClient.getFormattedTime());
}
