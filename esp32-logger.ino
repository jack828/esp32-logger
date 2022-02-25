#include "credentials.h"
#include "definitions.h"
#include "data.h"
#include "node.h"
#include "sensors.h"

#include <WiFi.h>
#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <InfluxDbClient.h>
#if defined(SDA_PIN) && defined(SCL_PIN)
#define HAS_I2C
#include <Wire.h>
#endif

// This is defined using compile time flags, but clangd doesn't like it
// And if it isn't defined when compiling with the Arduino IDE, this won't break anything
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "NOT_SET"
#endif

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB);
AsyncWebServer server(80);
Preferences config;

uint64_t setupMillis;

String processor(const String &var) {
  if (var == "MAC") {
    return WiFi.macAddress();
  } else if (var == "FIRMWARE_VERSION") {
    return FIRMWARE_VERSION;
  } else if (var == "NAME") {
    return config.getString("name");
  } else if (var == "LOCATION") {
    return config.getString("location");
  } else if (var == "UPTIME") {
    uint64_t uptime = millis() - setupMillis;
    unsigned long seconds = uptime / 1000;
    int days = seconds / 86400;
    seconds %= 86400;
    byte hours = seconds / 3600;
    seconds %= 3600;
    byte minutes = seconds / 60;
    seconds %= 60;

    char output[14];
    snprintf(output, sizeof(output), "%02dd%02dh%02dm%02ds", days, hours, minutes, seconds);
    return output;
  }
  return "";
}

void setup() {
  Serial.begin(115200);
  setupMillis = millis();
  pinMode(LED_PIN, OUTPUT);

  // Init config and set defaults on first run
  config.begin("config", false);
  if (config.getString("name") == "") {
    config.putString("name", "NAME_NOT_SET");
  }
  if (config.getString("location") == "") {
    config.putString("location", "LOCATION_NOT_SET");
  }

  Serial.print(F("[ WIFI ] Connecting"));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PSK);

  int retryCount = 0;
  do {
    Serial.print(F("."));
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    delay(250);
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    delay(250);
    if (retryCount++ > 20) {
      Serial.println(
        F("\n[ WIFI ] ERROR: Could not connect to wifi, rebooting..."));
      ESP.restart();
    }
  } while (WiFi.status() != WL_CONNECTED);

  Serial.print(F("\n[ WIFI ] connected, SSID: "));
  Serial.print(WiFi.SSID());
  Serial.print(F(", IP: "));
  Serial.println(WiFi.localIP());
  Serial.println();

  // Check server connection
  if (client.validateConnection()) {
    Serial.print(F("[ INFLUX ] Connected to: "));
    Serial.println(client.getServerUrl());
  } else {
    Serial.print(F("[ INFLUX ] Connection failed: "));
    Serial.println(client.getLastErrorMessage());
    ESP.restart();
  }

  setupNode();

#ifdef HAS_I2C
  Wire.begin(SDA_PIN, SCL_PIN);
#endif

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html, processor);
    /* TODO
       response->addHeader("Content-Encoding", "gzip");
     */
    request->send(response);
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebParameter *nameParam = request->getParam("name", true);
    config.putString("name", nameParam->value().c_str());
    Serial.print(F("[ NODE ] Set config.name: "));
    Serial.println(config.getString("name"));

    AsyncWebParameter *locationParam = request->getParam("location", true);
    config.putString("location", locationParam->value().c_str());
    Serial.print(F("[ NODE ] Set config.location: "));
    Serial.println(config.getString("location"));

    setupNode();

    request->redirect("/");
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });

  server.begin();
  AsyncElegantOTA.begin(&server);

  // Only create the task after all setup is done, and we're ready
  xTaskCreate(
    wifiKeepAlive,
    "wifiKeepAlive",  // Task name
    2048,             // Stack size (bytes)
    NULL,             // Parameter
    1,                // Task priority
    NULL              // Task handle
  );
} /* SETUP */

/**
 * Task: monitor the WiFi connection and keep it alive
 *
 * When a WiFi connection is established, this task will check it every 10 seconds
 * to make sure it's still alive.
 *
 * If not, a reconnect is attempted. If this fails to finish within the timeout,
 * the ESP32 will wait for it to recover and try again.
 */
void wifiKeepAlive(void *parameter) {
  int failCount = 0;
  for (;;) {
    Serial.print(F("[ WIFI ] Keep alive "));
    bool wifiConnected = (WiFi.RSSI() == 0) || WiFi.status() == WL_CONNECTED;
    Serial.println(wifiConnected ? F("OK") : F("NOT OK"));
    if (wifiConnected) {
      vTaskDelay(10000 / portTICK_PERIOD_MS);
      continue;
    }

    Serial.println(F("[ WIFI ] Connecting"));
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PSK);

    unsigned long startAttemptTime = millis();

    // Keep looping while we're not connected and haven't reached the timeout
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS) {
      digitalWrite(LED_PIN, HIGH);
      delay(250);
      digitalWrite(LED_PIN, LOW);
      delay(250);
    }

    // When we couldn't make a WiFi connection (or the timeout expired)
    // sleep for a while and then retry.
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print(F("[ WIFI ] Failed"));
      Serial.println(failCount);
      if (failCount++ > 5) {
        Serial.println(
          F("\n[ WIFI ] ERROR: Could not connect to wifi, rebooting..."));
        ESP.restart();
      }
      vTaskDelay(WIFI_RECOVER_TIME_MS / portTICK_PERIOD_MS);
      continue;
    }

    Serial.print(F("\n[ WIFI ] Connected: "));
    Serial.println(WiFi.localIP());
  }
}

int failedCount = 0;
int delayTime;

void log(Point &point) {
  Serial.print(F("[ INFLUX ] Writing: "));
  Serial.println(client.pointToLineProtocol(point));
  if (!client.writePoint(point)) {
    Serial.print(F("[ INFLUX ] Write failed: "));
    Serial.println(client.getLastErrorMessage());
    failedCount++;
    if (failedCount > 5) {
      Serial.println(F("[ NODE ] Failed too often, restarting"));
      ESP.restart();
    } else {
      // wait a shorter time before trying again
      // TODO this just gets reset in loop...
      delayTime = LOG_PERIOD / 10;
    }
  }
}

void loop() {
  delayTime = LOG_PERIOD;

  captureNodeFields();
  log(node);
  captureSensorFields();
  log(sensors);

  Serial.println(F("[ NODE ] Waiting..."));
  delay(delayTime);
}
