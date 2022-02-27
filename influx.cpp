#include "influx.h"
#include "credentials.h"

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB);

void validateInfluxConnection() {
  if (client.validateConnection()) {
    Serial.print(F("[ INFLUX ] Connected to: "));
    Serial.println(client.getServerUrl());
  } else {
    Serial.print(F("[ INFLUX ] Connection failed: "));
    Serial.println(client.getLastErrorMessage());
    ESP.restart();
  }
}

uint8_t failedCount = 0;

bool logPoint(Point &point) {
  Serial.print(F("[ INFLUX ] Writing: "));
  Serial.println(client.pointToLineProtocol(point));
  if (!client.writePoint(point)) {
    Serial.print(F("[ INFLUX ] Write failed! Status Code: "));
    Serial.print(client.getLastStatusCode());
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
