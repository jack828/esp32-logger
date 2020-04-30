#include "definitions.h"

#include <Wire.h>
#include <WiFi.h>
#include <WString.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "memory.h"
#include "network.h"
#include "node.h"

#ifdef BME680_I2C
#include <Adafruit_BME680.h>
Adafruit_BME680 bme680;
double temperature = 0.0;
double pressure = 0.0;
double humidity = 0.0;
double airQuality = 0.0;
#endif
#ifdef BME280_I2C
#include <Adafruit_BME280.h>
Adafruit_BME280 bme280;
double temperature = 0.0;
double pressure = 0.0;
double humidity = 0.0;
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
extern int lightLevel = 0;
#endif
#ifdef OLED
#include "oled.h"
long lastLog = 0l;
#endif

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "uk.pool.ntp.org", 0, 1000);
Node* node;

float hum_weighting = 0.25; // so hum effect is 25% of the total air quality score
float gas_weighting = 0.75; // so gas effect is 75% of the total air quality score

float hum_score, gas_score;
float gas_reference = 250000;
float hum_reference = 40;
int   getgasreference_count = 0;

void bme680_getGasReference() {
   // Now run the sensor for a burn-in period, then use combination of relative humidity and gas resistance to estimate indoor air quality as a percentage.
  Serial.println("Getting a new gas reference value");
  int readings = 20;
  for (int i = 1; i <= readings; i++){ // read gas for 20 x 0.150mS = 3.0secs
    gas_reference += bme680.readGas();
  }
  gas_reference = gas_reference / readings;
}


void bme680_getSensorThing() {
 //Calculate humidity contribution to IAQ index
  float current_humidity = bme680.readHumidity();
  if (current_humidity >= 38 && current_humidity <= 42)
    hum_score = 0.25*100; // Humidity +/-5% around optimum
  else
  { //sub-optimal
    if (current_humidity < 38)
      hum_score = 0.25/hum_reference*current_humidity*100;
    else
    {
      hum_score = ((-0.25/(100-hum_reference)*current_humidity)+0.416666)*100;
    }
  }

  //Calculate gas contribution to IAQ index
  float gas_lower_limit = 5000;   // Bad air quality limit
  float gas_upper_limit = 50000;  // Good air quality limit
  if (gas_reference > gas_upper_limit) gas_reference = gas_upper_limit;
  if (gas_reference < gas_lower_limit) gas_reference = gas_lower_limit;
  gas_score = (0.75/(gas_upper_limit-gas_lower_limit)*gas_reference -(gas_lower_limit*(0.75/(gas_upper_limit-gas_lower_limit))))*100;

  //Combine results for the final IAQ index value (0-100% where 100% is good quality air)
  float air_quality_score = hum_score + gas_score;

  /* Serial.println("Air Quality = "+String(air_quality_score,1)+"% derived from 25% of Humidity reading and 75% of Gas reading - 100% is good quality air"); */
  /* Serial.println("Humidity element was : "+String(hum_score/100)+" of 0.25"); */
  /* Serial.println("     Gas element was : "+String(gas_score/100)+" of 0.75"); */
  if (bme680.readGas() < 120000) Serial.println("***** Poor air quality *****");
  if ((getgasreference_count++)%10==0) bme680_getGasReference();
  Serial.printf(CalculateIAQStr(air_quality_score).c_str(), air_quality_score);
  /* Serial.println("------------------------------------------------"); */
  delay(2000);
}
String CalculateIAQStr(float score){
  String IAQ_text = "Air quality is %2.2f%% ";
  score = (100-score)*5;
  if      (score >= 301)                  IAQ_text += "Hazardous";
  else if (score >= 201 && score <= 300 ) IAQ_text += "Very Unhealthy";
  else if (score >= 176 && score <= 200 ) IAQ_text += "Unhealthy";
  else if (score >= 151 && score <= 175 ) IAQ_text += "Unhealthy for Sensitive Groups";
  else if (score >=  51 && score <= 150 ) IAQ_text += "Moderate";
  else if (score >=  00 && score <=  50 ) IAQ_text += "Good";
  IAQ_text += "\n";
  return IAQ_text;
}

void printMem(String marker) {
  Serial.print(" [MEM] ");
  Serial.print(marker);
  Serial.print(" ");
  Serial.print(ESP.getFreeHeap());
  Serial.print(" / ");
  Serial.println(ESP.getHeapSize());
}

void initNtp() {
  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  Serial.print("[ NTP ] time: ");
  Serial.println(timeClient.getFormattedTime());
  Serial.print("[ NTP ] epoch: ");
  Serial.println(timeClient.getEpochTime());
}

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
  bme680_getGasReference();
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

#ifdef OLED
  initOled();
#else
  Serial.println("[ OLED ] not present");
#endif
  node = new Node();
  initNtp();
}

void logSensors() {
  Serial.println("[ LOG ] beginning");
#ifdef BME680_I2C
  int start = millis();
  unsigned long endTime = bme680.beginReading();
  if (endTime == 0) {
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
  pressure = bme680.pressure;

  Serial.print(F("Humidity = "));
  Serial.print(bme680.humidity);
  Serial.println(F(" %"));
  humidity = bme680.humidity;

  while(1){
  bme680_getSensorThing();
  }
  /* airQuality = avgGas / 1000.0; */
  /* Serial.printf("avgGas = %2.2f Ω\n", avgGas); */
  /* Serial.printf("avgGas = %2.2f KΩ\n", avgGas / 1000); */

  /* node->log("temperature", temperature); */
  /* node->log("pressure", pressure); */
  /* node->log("humidity", humidity); */
  /* node->log("airQuality", airQuality); */
#endif
#ifdef BME280_I2C
  bme680.takeForcedMeasurement();
  temperature = bme280.readTemperature();
  pressure = bme280.readPressure() / 100.0F;
  humidity = bme280.readHumidity();
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
  logSensors();
  do {
    /* Serial.printf("%d, %ul\n", millis() - lastLog, LOG_PERIOD); */
    if (millis() - lastLog > LOG_PERIOD / 1000) {
      lastLog = millis();
      logSensors();
      Serial.println("logging sensors");
    }
    updateOled(); // this will also delay for the update period
    delay(100);
  } while(1);
#else
  logSensors();
#endif

  node->sleep();
}
