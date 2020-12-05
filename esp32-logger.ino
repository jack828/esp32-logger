#include "credentials.h"
#include "definitions.h"

#include <Wire.h>
#include <InfluxDbClient.h>
#include <WiFiMulti.h>
#ifdef BME280_I2C
#include <Adafruit_BME280.h>
Adafruit_BME280 bme280;
double temperature = 0.0;
double pressure = 0.0;
double humidity = 0.0;
double vpd = 0.0;
#endif

WiFiMulti wifiMulti;

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB);

Point node("node");
Point sensors("sensors");
int setupMillis;

void setup() {
  Serial.begin(115200);
  setupMillis = millis();

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PSK);

  Serial.print("[ WIFI ] Connecting");
  int retryCount = 0;
  while (wifiMulti.run() != WL_CONNECTED) {
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
      Serial.println("\n[ WIFI ] ERROR: Could not connect to wifi, rebooting...");
      ESP.restart();
    }
  }
  Serial.print("\n[ WIFI ] connected, SSID: ");
  Serial.print(WiFi.SSID());
  Serial.print(", IP:");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Add constant tags - only once
  node.addTag("MAC", WiFi.macAddress());
  node.addTag("SSID", WiFi.SSID());
  sensors.addTag("MAC", WiFi.macAddress());

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("[ INFLUX ] Connected to: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("[ INFLUX ] Connection failed: ");
    Serial.println(client.getLastErrorMessage());
    ESP.restart();
  }

  Wire.begin(SDA_PIN, SCL_PIN);
#ifdef BME280_I2C
  Serial.println("[ BME280 ] has sensor");
  boolean bme280Ok = bme280.begin(0x76, &Wire);
  Serial.print("[ BME280 ] sensor ");
  Serial.print(bme280Ok ? "" : "NOT ");
  Serial.println("OK");

  bme280.setSampling(
    Adafruit_BME280::MODE_FORCED,
    Adafruit_BME280::SAMPLING_X4, // temperature
    Adafruit_BME280::SAMPLING_X4, // pressure
    Adafruit_BME280::SAMPLING_X4, // humidity
    Adafruit_BME280::FILTER_X4,
    Adafruit_BME280::STANDBY_MS_0_5
  );
#endif
}

void captureSensorsFields () {
  sensors.clearFields();
#ifdef BME280_I2C
  bme280.takeForcedMeasurement();
  temperature = bme280.readTemperature();
  pressure = bme280.readPressure() / 100.0F;
  humidity = bme280.readHumidity();

  double e = 2.71828;
  /* in pascals */
  double SVP = 610.78 * pow(e, (temperature / (temperature + 238.3) * 17.2694));
  vpd = (SVP / 1000) * (1 - humidity / 100); // kPa

  sensors.addField("temperature", temperature);
  sensors.addField("pressure", pressure);
  sensors.addField("humidity", humidity);
  sensors.addField("vpd", vpd);
#endif
}

void captureNodeFields () {
  node.clearFields();
  node.addField("rssi", WiFi.RSSI());
  node.addField("uptime", millis() - setupMillis);
  node.addField("freeHeap", ESP.getFreeHeap());
}

int failedCount = 0;
int delayTime;

void log (Point& point) {
  Serial.print("[ INFLUX ] Writing: ");
  Serial.println(client.pointToLineProtocol(point));
  if (!client.writePoint(point)) {
    Serial.print("[ INFLUX ] Write failed: ");
    Serial.println(client.getLastErrorMessage());
    failedCount++;
    if (failedCount > 5) {
      Serial.println("[ NODE ] Failed too often, restarting");
      ESP.restart();
    } else {
      // wait a shorter time before trying again
      delayTime = 10 * 1000;
    }
  }
}

void loop() {
  delayTime = 60 * 1000;
  if ((WiFi.RSSI() == 0) && (wifiMulti.run() != WL_CONNECTED)) {
    Serial.println("[ WIFI ] connection lost :( ");
    ESP.restart();
  }

  captureNodeFields();
  captureSensorsFields();
  log(node);
  log(sensors);

  Serial.println("[ NODE ] Waiting...");
  delay(delayTime);
}
