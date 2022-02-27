#include "sensors.h"
#include "definitions.h"
#include "influx.h"

#ifdef BME280_I2C
#include <Adafruit_BME280.h>
Adafruit_BME280 bme280;
double temperature = 0.0;
double pressure = 0.0;
double humidity = 0.0;
double vpd = 0.0;
#endif
#ifdef SCT_013_PIN
#include "EmonLib.h"
EnergyMonitor emon;
#endif

Point sensors("sensors");

void setupSensors() {
#ifdef BME280_I2C
  Serial.println(F("[ BME280 ] has sensor"));
  boolean bme280Ok = bme280.begin(0x76, &Wire);
  Serial.print(F("[ BME280 ] sensor "));
  Serial.print(bme280Ok ? F("") : F("NOT "));
  Serial.println(F("OK"));

  bme280.setSampling(Adafruit_BME280::MODE_FORCED,
                     Adafruit_BME280::SAMPLING_X4, // temperature
                     Adafruit_BME280::SAMPLING_X4, // pressure
                     Adafruit_BME280::SAMPLING_X4, // humidity
                     Adafruit_BME280::FILTER_X4,
                     Adafruit_BME280::STANDBY_MS_0_5);
#endif

#ifdef SCT_013_PIN
  Serial.println(F("[ SCT013 ] has sensor"));
  emon.current(SCT_013_PIN, SCT_013_CALIBRATION);
#endif
}

void setSensorsTags() {
  sensors.clearTags();
  sensors.addTag(F("MAC"), WiFi.macAddress());
  sensors.addTag(F("name"), config.getString("name"));
  sensors.addTag(F("location"), config.getString("location"));
}

void captureSensorsFields() {
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

  sensors.addField(F("temperature"), temperature);
  sensors.addField(F("pressure"), pressure);
  sensors.addField(F("humidity"), humidity);
  sensors.addField(F("vpd"), vpd);
#endif

#ifdef SCT_013_PIN
  double irms = emon.calcIrms(1480);
  Serial.printf("%f, %f\n", irms, irms * VOLTAGE);
#endif
}

/**
 * Task: Wait the appropriate period, and log the `sensors` fields.
 *
 * If logging fails, it will wait progressively smaller portions of time.
 */
void sensorsLoggerTask(void *parameters) {
  double delayTime = SENSORS_LOG_PERIOD;
  for (;;) {
    Serial.println(F("[ SENSORS ] Logger task"));
    captureSensorsFields();
    bool logOk = logPoint(sensors);
    if (!logOk) {
      // On failure, Decrease delay by 25%, 1 sec minimum
      delayTime = min(delayTime * 0.75, 1000.0);
    } else {
      // On success, reset delay to normal
      delayTime = SENSORS_LOG_PERIOD;
    }
    Serial.println(F("[ SENSORS ] Waiting..."));
    vTaskDelay(SENSORS_LOG_PERIOD / portTICK_PERIOD_MS);
  }
}
