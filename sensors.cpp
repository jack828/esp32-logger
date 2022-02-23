#include "sensors.h"

Point sensors("sensors");

void setupSensors() {
  sensors.addTag(F("MAC"), WiFi.macAddress());

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
  emon.current(SCT_013_PIN, 111.1);
#endif
}

void captureSensorFields() {
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
