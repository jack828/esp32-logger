#ifndef G6EJD_BME680
#define G6EJD_BME680

#include <WString.h>
#include <Adafruit_BME680.h>

void bme680_getGasReference(Adafruit_BME680 bme680);
double bme680_getAirQuality(Adafruit_BME680 bme680);
String CalculateIAQStr(float score);

#endif
