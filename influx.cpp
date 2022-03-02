#include "influx.h"
#include "credentials.h"

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB);
SemaphoreHandle_t mutex = xSemaphoreCreateMutex();

bool validateInfluxConnection() {
  client.setWriteOptions(WriteOptions().writePrecision(WritePrecision::MS));
  client.setHTTPOptions(HTTPOptions().connectionReuse(true));
  xSemaphoreTake(mutex, portMAX_DELAY);
  bool influxOk = client.validateConnection();
  xSemaphoreGive(mutex);
  if (influxOk) {
    Serial.print(F("[ INFLUX ] Connected to: "));
    Serial.println(client.getServerUrl());
    return true;
  } else {
    Serial.print(F("[ INFLUX ] Connection failed: "));
    Serial.println(client.getLastErrorMessage());
    return false;
  }
}

uint8_t failedCount = 0;

bool logPoint(Point &point) {
  Serial.print(F("[ INFLUX ] Writing: "));
  Serial.println(client.pointToLineProtocol(point));
  xSemaphoreTake(mutex, portMAX_DELAY);
  bool writeOk = client.writePoint(point);
  xSemaphoreGive(mutex);
  if (!writeOk) {
    int16_t lastStatusCode = client.getLastStatusCode();
    if (lastStatusCode == 204) {
      // We didn't send any data!
      return true;
    }
    Serial.print(F("[ INFLUX ] Write failed! Status Code: "));
    Serial.print(lastStatusCode);
    Serial.print(F(", "));
    Serial.println(client.getLastErrorMessage());
    if (++failedCount > 5) {
      Serial.println(F("[ INFLUX ] Failed too often, restarting"));
      ESP.restart();
    }
    return false;
  } else {
    failedCount = 0;
    return true;
  }
}
