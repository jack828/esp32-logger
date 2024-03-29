#include "credentials.h"
#include "definitions.h"
#include "data.h"
#include "node.h"
#include "sensors.h"
#include "influx.h"

#include <WiFi.h>
#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#if defined(SDA_PIN) && defined(SCL_PIN)
#define HAS_I2C
#include <Wire.h>
#endif

AsyncWebServer server(80);
Preferences config;

uint64_t setupMillis;

String processor(const String &var) {
  if (var == "MDNS_HOSTNAME") {
#ifdef ESP8266
    char host[12];
    snprintf(host, 12, "esp%08X", ESP.getChipId());
#else
    char host[16];
    snprintf(host, 16, "esp%012" PRIx64, ESP.getEfuseMac());
#endif
    return host;
  } else if (var == "CHIP_ID") {
#ifdef ESP8266
    // TODO support
    char chipId[12] = "00000000000";
#else
    char chipId[16];
    snprintf(chipId, 16, "%015" PRIu64, ESP.getEfuseMac());
#endif
    return chipId;
  } else if (var == "CHIP_ID_HEX") {
#ifdef ESP8266
    char chipIdHex[12];
    snprintf(chipIdHex, 12, "%08X", ESP.getChipId());
#else
    char chipIdHex[16];
    snprintf(chipIdHex, 16, "%012" PRIx64, ESP.getEfuseMac());
#endif
    return chipIdHex;
  } else if (var == "IP") {
    return WiFi.localIP().toString();
  } else if (var == "MAC") {
    return WiFi.macAddress();
  } else if (var == "FIRMWARE_VERSION") {
    return FIRMWARE_VERSION;
  } else if (var == "NAME") {
    return config.getString("name");
  } else if (var == "LOCATION") {
    return config.getString("location");
  } else if (var == "UPTIME") {
    uint64_t uptime = millis() - setupMillis;
    uint64_t seconds = uptime / 1000;
    uint16_t days = seconds / 86400;
    seconds %= 86400;
    byte hours = seconds / 3600;
    seconds %= 3600;
    byte minutes = seconds / 60;
    seconds %= 60;

    char output[16];
    snprintf(output, sizeof(output), "%02dd%02dh%02dm%02llus", days, hours, minutes, seconds);
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

  /* <WIFI> */
  Serial.print(F("[ WIFI ] Connecting"));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PSK);

  uint8_t retryCount = 0;
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
  /* </WIFI> */

  /* <INFLUXDB> */
  setupInfluxOptions();
  boolean influxOk = validateInfluxConnection();
  if (!influxOk) {
    // literally no point in being here
    ESP.restart();
  }
  setNodeTags();
  setSensorsTags();
  /* </INFLUXDB> */

  /* <SENSORS> */
#ifdef HAS_I2C
  Wire.begin(SDA_PIN, SCL_PIN);
#endif
  setupSensors();
  /* </SENSORS> */

  /* <MDNS> */
#ifdef ESP8266
  // TODO support
  char chipId[12] = "00000000000";
  char host[12];
  snprintf(host, 12, "esp%08X", ESP.getChipId());
#else
  char host[16];
  snprintf(host, 16, "esp%012" PRIx64, ESP.getEfuseMac());
  char chipId[16];
  snprintf(chipId, 16, "%" PRIu64, ESP.getEfuseMac());
#endif

  MDNS.begin(host);
  Serial.printf("[ NODE ] MDNS listening http://%s.local\n", host);
  Serial.printf("[ NODE ] Chip ID %s\n", chipId);
  MDNS.addService("_http", "_tcp", 80);
  MDNS.addServiceTxt("_http", "_tcp", "id", (const char*) chipId);
  MDNS.addServiceTxt("_http", "_tcp", "board", "ESP32-LOGGER");
  MDNS.addServiceTxt("_http", "_tcp", "name", config.getString("name"));
  MDNS.addServiceTxt("_http", "_tcp", "location", config.getString("location"));
  /* </MDNS> */

  /* <WEBSERVER> */
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html, processor);
    /* TODO ElegantOTA has a nice node script to compress html and output it
       response->addHeader("Content-Encoding", "gzip");
     */
    request->send(response);
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebParameter *nameParam = request->getParam("name", true);
    config.putString("name", nameParam->value().c_str());
    Serial.print(F("[ NODE ] Set config.name: "));
    Serial.println(config.getString("name"));
    MDNS.addServiceTxt("_http", "_tcp", "name", config.getString("name"));

    AsyncWebParameter *locationParam = request->getParam("location", true);
    config.putString("location", locationParam->value().c_str());
    Serial.print(F("[ NODE ] Set config.location: "));
    Serial.println(config.getString("location"));
    MDNS.addServiceTxt("_http", "_tcp", "location", config.getString("location"));

    setNodeTags();
    setSensorsTags();

    request->send(200, "text/html", update_html);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });

  server.begin();
  /* </WEBSERVER> */

  AsyncElegantOTA.begin(&server);

  // Only create the tasks after all setup is done, and we're ready
  xTaskCreate(
    wifiKeepAlive,
    "wifiKeepAlive",  // Task name
    16192,            // Stack size (bytes)
    NULL,             // Parameter
    1,                // Task priority
    NULL              // Task handle
  );

  // Offset to (hopefully) prevent log messages from crashing into each other
  delay(333);

  xTaskCreate(
    nodeLoggerTask,
    "nodeLoggerTask",
    32768,
    NULL,
    4,
    NULL
  );

  delay(333);

  xTaskCreate(
    sensorsLoggerTask,
    "sensorsLoggerTask",
    32768,
    NULL,
    4,
    NULL
  );
} /* SETUP */

/**
 * Task: monitor the WiFi connection and keep it alive
 *
 * When a WiFi connection is established, this task will check it every 10
 * seconds to make sure it's still alive.
 *
 * If not, a reconnect is attempted. If this fails to finish within the timeout,
 * the ESP32 will wait for it to recover and try again.
 */
void wifiKeepAlive(void *parameter) {
  uint8_t failCount = 0;
  for (;;) {
    Serial.print(F("[ WIFI ] Keep alive "));
    bool wifiConnected = (WiFi.RSSI() == 0) || WiFi.status() == WL_CONNECTED;
    Serial.println(wifiConnected ? F("OK") : F("NOT OK"));
    if (wifiConnected) {
      bool influxOk = validateInfluxConnection();
      if (!influxOk) {
        Serial.println(F("[ NODE ] Rebooting..."));
        Serial.flush();
        ESP.restart();
      }
      vTaskDelay(10000 / portTICK_PERIOD_MS);
      continue;
    }

    Serial.println(F("[ WIFI ] Connecting"));
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PSK);

    uint64_t startAttemptTime = millis();

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

void loop() {
  /* beep boop nothing to do here! */
  /* It's all handled in the tasks created in `setup` */
}
