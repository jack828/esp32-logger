#ifndef Sensors_h
#define Sensors_h

#include <InfluxDbClient.h>

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

extern Point sensors;

void setupSensors(void);
void captureSensorFields(void);

#endif
