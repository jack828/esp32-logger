#include "definitions.h"

#include <Wire.h>
#include <WiFi.h>

#include "network.h"
#include "node.h"

#ifdef BME680_I2C
#include <Adafruit_BME680.h>
#include <math.h>
#include "G6EJD_BME680.h"
Adafruit_BME680 bme680;
double temperature = 0.0;
double pressure = 0.0;
double humidity = 0.0;
double airQuality = 0.0;
double vpd = 0.0;
#endif
#ifdef BME280_I2C
#include <Adafruit_BME280.h>
Adafruit_BME280 bme280;
double temperature = 0.0;
double pressure = 0.0;
double humidity = 0.0;
double vpd = 0.0;
#endif
#ifdef BMP280_I2C
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp280;
double temperature = 0.0;
double pressure = 0.0;
#endif
#ifdef BH1750_I2C
#include <BH1750.h>
BH1750 lightMeter(0x23);
extern double lux;
#endif
#ifdef DHT11_PIN
#include <DHTesp.h>
DHTesp dht;
TempAndHumidity reading;
#endif
#ifdef LIGHT_SENSOR_PIN
int lightLevel = 0;
#endif
#ifdef SOIL_MOISTURE_PIN
int soilMoisture = 0;
#endif
#ifdef OLED
#include "oled.h"
long lastLog = 0l;
#endif

Node* node;

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(LED_PIN, OUTPUT);
  Serial.print("Flash size: ");
  Serial.println(ESP.getFlashChipSize());

#ifdef BME680_I2C
  Serial.println("[ BME680 ] has sensor");
  boolean bme680Ok = bme680.begin(0x77);
  Serial.print("[ BME680 ] sensor ");
  Serial.print(bme680Ok ? "" : "NOT ");
  Serial.println("OK");

  boolean ret;
  ret = bme680.setTemperatureOversampling(BME680_OS_8X);
  if (!ret) { Serial.print("Temp set fail"); };
  bme680.setHumidityOversampling(BME680_OS_2X);
  if (!ret) { Serial.print("Humidity set fail"); };
  bme680.setPressureOversampling(BME680_OS_4X);
  if (!ret) { Serial.print("Pressure set fail"); };
  bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);
  if (!ret) { Serial.print("IIR set fail"); };
  bme680.setGasHeater(320, 150); // 320*C for 150 ms
  if (!ret) { Serial.print("Gas set fail"); };
  bme680_getGasReference(bme680);
#endif

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

#ifdef BMP280_I2C
  Serial.println("[ BMP280 ] has sensor");
  boolean bmp280Ok = bmp280.begin(0x76);
  Serial.print("[ BMP280 ] sensor ");
  Serial.print(bmp280Ok ? "" : "NOT ");
  Serial.println("OK");
#endif

#ifdef BH1750_I2C
  if (lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE)) {
    Serial.println("[ LUX ] sensor ok");
  } else {
    Serial.println("[ LUX ] sensor NOT ok");
  }
#endif

#ifdef DHT11_PIN
  dht.setup(DHT11_PIN, DHTesp::DHT11);
  Serial.println("[ DHT ] sensor ok");
#endif

#ifdef SOIL_MOISTURE_PIN
  analogReadResolution(11);
  analogSetAttenuation(ADC_6db);
#endif

#ifdef OLED
  initOled();
#else
  Serial.println("[ OLED ] not present");
#endif

  node = new Node();
}

void logSensors() {
  Serial.println("[ LOG ] beginning");

#ifdef BME680_I2C
  int start = millis();
  unsigned long endTime = bme680.beginReading();
  if (endTime == 0) {
    // TODO re-init sensor?
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  Serial.print(F("Reading started at "));
  Serial.print(start);
  Serial.print(F(" and will finish at "));
  Serial.print(endTime);
  Serial.print(F(" total work time ms "));
  Serial.println(endTime - start);

  Serial.println(F("You can do other work during BME680 measurement."));
  /* delay(50); // This represents parallel work. */
  // There's no need to delay() until millis() >= endTime: bme680.endReading()
  // takes care of that. It's okay for parallel work to take longer than
  // BME680's measurement time.

  // Obtain measurement results from BME680. Note that this operation isn't
  // instantaneous even if milli() >= endTime due to I2C/SPI latency.
  if (!bme680.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  Serial.print(F("Reading completed at "));
  Serial.println(millis());

  Serial.print(F("Temperature = "));
  Serial.print(bme680.temperature);
  Serial.println(F(" *C"));
  temperature = bme680.temperature;

  Serial.print(F("Pressure = "));
  Serial.print(bme680.pressure / 100.0);
  Serial.println(F(" hPa"));
  pressure = bme680.pressure / 100.0;

  Serial.print(F("Humidity = "));
  Serial.print(bme680.humidity);
  Serial.println(F(" %"));
  humidity = bme680.humidity;

  airQuality = 0.0;
  char count = 10;
  for (int i = count; i; i--) {
    airQuality += bme680_getAirQuality(bme680);
  }
  airQuality /= count;
  Serial.printf("Air Quality: %2.2f\n", airQuality);
  /* airQuality = avgGas / 1000.0; */
  /* Serial.printf("avgGas = %2.2f Ω\n", avgGas); */
  /* Serial.printf("avgGas = %2.2f KΩ\n", avgGas / 1000); */

  node->log("temperature", temperature);
  node->log("pressure", pressure);
  node->log("humidity", humidity);
  node->log("airQuality", airQuality);
#endif

#ifdef BME280_I2C
  bme280.takeForcedMeasurement();
  temperature = bme280.readTemperature();
  pressure = bme280.readPressure() / 100.0F;
  humidity = bme280.readHumidity();

  double e = 2.71828;
  /* in pascals */
  double SVP = 610.78 * pow(e, (temperature / (temperature + 238.3) * 17.2694));
  vpd = (SVP / 1000) * (1 - humidity / 100); // kPa

  Serial.printf("SVP: %2.2f\n", SVP);
  Serial.printf("VPD: %2.2f\n", vpd);

  node->log("temperature", temperature);
  node->log("pressure", pressure);
  node->log("humidity", humidity);
#endif

#ifdef BMP280_I2C
  temperature = bmp280.readTemperature();
  pressure = bmp280.readPressure() / 100.0F;
  node->log("temperature", temperature);
  node->log("pressure", pressure);
#endif

#ifdef BH1750_I2C
  lux = lightMeter.readLightLevel(true);
  node->log("light", lux);
#endif

#ifdef LIGHT_SENSOR_PIN
  lightLevel = analogRead(LIGHT_SENSOR_PIN);
  node->log("light", lightLevel);
#endif

#ifdef SOIL_MOISTURE_PIN
  soilMoisture = 0;
  char count = 10;
  for (int i = count; i; i--) {
    soilMoisture += analogRead(SOIL_MOISTURE_PIN);
  }
  soilMoisture /= count;
  node->log("soilMoisture", soilMoisture);
#endif

#ifdef DHT11_PIN
  reading = dht.getTempAndHumidity();
  node->log("temperature", reading.temperature);
  node->log("humidity", reading.humidity);
#endif
  Serial.println("[ LOG ] finished");
}

void loop() {
  node->wake();

#ifdef OLED
  /* TODO change this mode
   * should:
   *  - Log readings
   *  - Show display for LOG_PERIOD
   *  - Log readings
   *  - Snooze
   */
  int logCount = 0;
  logSensors(); logCount++;
  do {
    /* Serial.printf("%d, %ul\n", millis() - lastLog, LOG_PERIOD); */
    if (millis() - lastLog > LOG_PERIOD / 1000 / 10) {
      lastLog = millis();
      logSensors(); logCount++;
      Serial.println("logging sensors");
      if (logCount > 1) {
        clearOled();
        break;
      }
    }
    updateOled(); // this will also delay for the update period
  } while(1);
#else
  logSensors();
#endif

  node->sleep();
}
