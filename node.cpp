#include "node.h"

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
  Serial.println("[ NODE ] sleeping...");
  esp_sleep_enable_timer_wakeup(LOG_PERIOD);
  Serial.flush();
  WiFi.disconnect();

  int ret = esp_light_sleep_start();
  Serial.printf("ret: %d\n", ret);
  if (ret != 0) {
    Serial.println("Error entering light sleep");
  }
}

void Node::wake() {
  Serial.println("[ NODE ] waking...");
  this->checkWifi();
  timeClient.update();
  Serial.println("[ NODE ] woke " + timeClient.getFormattedTime());
}
