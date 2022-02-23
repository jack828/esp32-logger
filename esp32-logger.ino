#include "credentials.h"
#include "definitions.h"
#include "data.h"
#include "sensors.h"

#include <InfluxDbClient.h>
#if defined(SDA_PIN) && defined(SCL_PIN)
#define HAS_I2C
#include <Wire.h>
#endif

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB);
AsyncWebServer server(80);

Point node("node");
int setupMillis;

String processor(const String &var) {
  if (var == "NODE_MAC") {
    return WiFi.macAddress();
  } else if (var == "FIRMWARE_VERSION") {
    return F("4.2.0-69");
  } else if (var == "NODE_NAME") {
    return F("Saint Bobbington III");
  } else if (var == "NODE_LOCATION") {
    return F("next to me");
  }
  return "";
}

// TODO add http server to set nickname which is then put into EEPROM for
// logging with a nicer name
// TODO log wifi ip
void setup() {
  Serial.begin(115200);
  setupMillis = millis();
  pinMode(LED_PIN, OUTPUT);

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

  // TODO maybe
  // void addTags (Point &point) {
  // Add constant tags - only once
  node.addTag(F("MAC"), WiFi.macAddress());
  node.addTag(F("SSID"), WiFi.SSID());

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
    AsyncWebParameter *nodeNameParam = request->getParam("nodeName", true);
    Serial.printf("POST [%s]: %s\n", nodeNameParam->name().c_str(), nodeNameParam->value().c_str());
    AsyncWebParameter *nodeLocationParam = request->getParam("nodeLocation", true);
    Serial.printf("POST [%s]: %s\n", nodeLocationParam->name().c_str(), nodeLocationParam->value().c_str());
    request->redirect("/");
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });

  server.begin();

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

void captureNodeFields() {
  node.clearFields();
  node.addField(F("rssi"), WiFi.RSSI());
  node.addField(F("uptime"), millis() - setupMillis);
  node.addField(F("freeHeap"), ESP.getFreeHeap());
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
  if ((WiFi.RSSI() == 0) && (WiFi.status() != WL_CONNECTED)) {
    Serial.println(F("[ WIFI ] connection lost :( "));
    ESP.restart();
  }

  captureNodeFields();
  log(node);
  captureSensorFields();
  log(sensors);

  Serial.println(F("[ NODE ] Waiting..."));
  delay(delayTime);
}
