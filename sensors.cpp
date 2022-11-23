#include "sensors.h"
#include "definitions.h"
#include "influx.h"

#ifdef BME280_I2C
#include <Adafruit_BME280.h>
Adafruit_BME280 bme280;
#endif
#ifdef BME680_I2C
#include <bsec.h>
Bsec iaqSensor;
void checkIaqSensorStatus(void);
void saveBsecState(void);
void loadBsecState(void);
const uint8_t bsec_config_iaq[] = {
#include "config/generic_33v_3s_4d/bsec_iaq.txt"
};
#define STATE_SAVE_PERIOD UINT32_C(360 * 60 * 1000) // 360 mins - 4 times a day
#define BSEC_SENSOR_COUNT 10
uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE] = {0};
uint16_t stateUpdateCounter = 0;
#endif
#ifdef SCT_013_PIN
#include "EmonLib.h"
EnergyMonitor emon;
#endif
#if defined(MHZ19_RX) && defined(MHZ19_TX)
#define HAS_MHZ19
#include "MHZ19.h"
MHZ19 *mhz19 = new MHZ19(MHZ19_RX, MHZ19_TX);
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

#ifdef BME680_I2C
  Serial.println(F("[ BME680 ] has sensor"));
  iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
  Serial.printf("[ BME680 ] BSEC library v%d.%d.%d.%d\n",
                iaqSensor.version.major, iaqSensor.version.minor,
                iaqSensor.version.major_bugfix, iaqSensor.version.minor_bugfix);

  checkIaqSensorStatus();

  iaqSensor.setConfig(bsec_config_iaq);

  checkIaqSensorStatus();

  // TODO maybe we need to run this sensor in 5 minute read intervals
  if (config.getBytesLength("bsecState") == 0) {
    // No state saved, zero fill
    Serial.println("[ BME680 ] Zero filled state");
    config.putBytes("bsecState", &bsecState, BSEC_MAX_STATE_BLOB_SIZE);
  } else {
    stateUpdateCounter = 1;
    // TODO not enough space in buffer 139 < 140
    config.getBytes("bsecState", &bsecState, BSEC_MAX_STATE_BLOB_SIZE);
    Serial.printf("[ BME680 ] Loaded state (%lu) ",
                  config.getBytesLength("bsecState"));
    for (int i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
      Serial.printf("%d", bsecState[i]);
    }
    Serial.println();
  }

  checkIaqSensorStatus();

  bsec_virtual_sensor_t sensorList[] = {
      BSEC_OUTPUT_RAW_TEMPERATURE,
      BSEC_OUTPUT_RAW_PRESSURE,
      BSEC_OUTPUT_RAW_HUMIDITY,
      BSEC_OUTPUT_RAW_GAS,
      BSEC_OUTPUT_IAQ,
      BSEC_OUTPUT_STATIC_IAQ,
      BSEC_OUTPUT_CO2_EQUIVALENT,
      BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
      BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
      BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY};

  iaqSensor.updateSubscription(sensorList,
                               sizeof(sensorList) / sizeof(sensorList[0]),
                               BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();
#endif

#ifdef SCT_013_PIN
  Serial.println(F("[ SCT013 ] has sensor"));
  emon.current(SCT_013_PIN, SCT_013_CALIBRATION);
  delay(5000); // Sensor needs to chill after boot or values spike
#endif

#ifdef HAS_MHZ19
  mhz19->begin();
  mhz19->setAutoCalibration(false); // if outdoors set to true
  delay(3000);                      // apparently fixes a infinite warming issue
  Serial.print(F(" [ MH-Z19 ] has sensor, status: "));
  Serial.println(mhz19->getStatus());
#endif
} /* setupSensors */

#ifdef BME680_I2C
void checkIaqSensorStatus(void) {
  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      Serial.print(F("[ BME680 ] BSEC error code: "));
      Serial.println(iaqSensor.status);
    } else {
      Serial.print(F("[ BME680 ] BSEC warning code: "));
      Serial.println(iaqSensor.status);
    }
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      Serial.print(F("[ BME680 ] BME680 error code: "));
      Serial.println(iaqSensor.status);
    } else {
      Serial.print(F("[ BME680 ] BME680 warning code: "));
      Serial.println(iaqSensor.status);
    }
  }
  Serial.println(F("[ BME680 ] sensor OK"));
}
#endif

double calculateVpd(double temperature, double humidity) {
  double e = 2.71828;
  /* in pascals */
  double SVP = 610.78 * pow(e, (temperature / (temperature + 238.3) * 17.2694));
  double vpd = (SVP / 1000) * (1 - humidity / 100); // kPa
  return vpd;
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
  double temperature = bme280.readTemperature();
  double pressure = bme280.readPressure() / 100.0F;
  double humidity = bme280.readHumidity();
  double vpd = calculateVpd(temperature, humidity);

  sensors.addField(F("temperature"), temperature);
  sensors.addField(F("pressure"), pressure);
  sensors.addField(F("humidity"), humidity);
  sensors.addField(F("vpd"), vpd);
#endif

#ifdef BME680_I2C
  if (iaqSensor.run()) {
    Serial.printf(
        "Pressure %.2f\nIAQ %.2f\nIAQ "
        "acc %d\nTemp %.2f\nrH %.2f\nStaticIAQ %.2f\nco2e %.2f\nbVOCe %.2f\n",
        iaqSensor.pressure, iaqSensor.iaq, iaqSensor.iaqAccuracy,
        iaqSensor.temperature, iaqSensor.humidity, iaqSensor.staticIaq,
        iaqSensor.co2Equivalent, iaqSensor.breathVocEquivalent);

    double vpd = calculateVpd(iaqSensor.temperature, iaqSensor.humidity);

    Serial.printf("%f, %f, %d\n", iaqSensor.runInStatus, iaqSensor.stabStatus,
                  iaqSensor.status);
    sensors.addField(F("temperature"), iaqSensor.temperature);
    sensors.addField(F("pressure"), iaqSensor.pressure / 100.0F);
    sensors.addField(F("humidity"), iaqSensor.humidity);
    sensors.addField(F("eCO2"), iaqSensor.co2Equivalent);
    sensors.addField(F("bVOCe"), iaqSensor.breathVocEquivalent);
    // Good reading about what this field actually represents
    // https://community.bosch-sensortec.com/t5/MEMS-sensors-forum/BME680-strange-IAQ-and-CO2-values/m-p/9667/highlight/true#M1505
    sensors.addField(F("iaq"), iaqSensor.staticIaq);

    if (stateUpdateCounter == 0) {
      // Only persist state when calibration completes (maybe never)
      if (iaqSensor.iaqAccuracy >= 3) {
        stateUpdateCounter++;
        iaqSensor.getState(bsecState);
        config.putBytes("bsecState", bsecState, BSEC_MAX_STATE_BLOB_SIZE);
      }
    } else {
      // Update every STATE_SAVE_PERIOD milliseconds
      if ((stateUpdateCounter * STATE_SAVE_PERIOD) < millis()) {
        stateUpdateCounter++;
        iaqSensor.getState(bsecState);
        config.putBytes("bsecState", bsecState, BSEC_MAX_STATE_BLOB_SIZE);
      }
    }
  } else {
    checkIaqSensorStatus();
  }
#endif

#ifdef SCT_013_PIN
  double irms = emon.calcIrms(1480);
  sensors.addField(F("irms"), irms);
  sensors.addField(F("power"), irms * VOLTAGE);
#endif

#ifdef HAS_MHZ19
  measurement_t measurement = mhz19->getMeasurement();
  sensors.addField(F("co2"), measurement.co2_ppm);
  sensors.addField(F("temperature"), (float)measurement.temperature);
#endif
} /* captureSensorsFields */

/**
 * Task: Wait the appropriate period, and log the `sensors` fields.
 *
 * If logging fails, it will wait progressively smaller portions of time.
 */
void sensorsLoggerTask(void *parameters) {
  uint64_t delayTime = SENSORS_LOG_PERIOD;
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
    vTaskDelay(delayTime / portTICK_PERIOD_MS);
  }
}
