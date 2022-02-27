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
    Serial.print(F("[ INFLUX ] Write failed: "));
    Serial.println(client.getLastErrorMessage());
    failedCount++;
    if (failedCount > 5) {
      Serial.println(F("[ INFLUX ] Failed too often, restarting"));
      ESP.restart();
    } else {
      // wait a shorter time before trying again
      // TODO this just gets reset in loop...
      // delayTime = LOG_PERIOD / 10;
    }
  } else {
    failedCount = 0;
  }
}
