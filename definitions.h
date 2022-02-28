#ifndef Definitions_h
#define Definitions_h

// pick a board any board
// #define ESP_32_OLED_BATT
// #define ESP_32_OLED
// #define ESP_32
#define ESP_32_U
// #define ESP_8266 // Use this one if it might blow up

// Configuration
#define NODE_LOG_PERIOD 60 * 1000 // 60 seconds
#define SENSORS_LOG_PERIOD 60 * 1000 // 1 second

#define WIFI_CHECK_PERIOD_MS 30000 // 10s keep alive period
#define WIFI_TIMEOUT_MS 10000      // 10s WiFi connection timeout
#define WIFI_RECOVER_TIME_MS 10000 // 10s after a failed connection attempt

// This is defined using compile time flags, but clangd doesn't like it
// And if it isn't defined when compiling with the Arduino IDE, this won't break anything
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "NOT_SET"
#endif

/*
 * You can define any of these to enable sensors
 * and additional functionality, depending on your setup:
 *
 * LED_PIN - status LED, flashes when connecting to WiFi.
 * LIGHT_SENSOR_PIN - enable analogRead of a light sensor
 * SOIL_MOISTURE_PIN - enable analogRead of a soil moisture sensor
 * BMP280_I2C - Temp & Pressure sensor
 * BME280_I2C - Temp, Humidity, and Pressure sensor
 * BH1750_I2C - Light sensor
 * DHT11_PIN - Temp & Humidity sensor
 * OLED - Show a HUD of current sensors. Node will still log readings
 * OLED_FLIP_V - if you need it
 */

// Hardware specific config
// so m a n y
#ifdef ESP_32_OLED_BATT
#define SDA_PIN 21
#define SCL_PIN 22
#define LED_PIN 16
// #define LIGHT_SENSOR_PIN 36
#define BME280_I2C
#define OLED
#endif
#ifdef ESP_32_OLED
#define SDA_PIN 5
#define SCL_PIN 4
// this doesn't have one but :shrug:
#define LED_PIN 16
#define BME280_I2C
// #define OLED
// #define OLED_FLIP_V
// #define TOUCH_L_PIN 13
// #define TOUCH_R_PIN 15
// #define LIGHT_SENSOR_PIN 36 // SVP
// #define DHT11_PIN 25
#endif
#ifdef ESP_32
// #define SDA_PIN 21
// #define SCL_PIN 22
#define SCT_013_PIN A0
#define SCT_013_CALIBRATION 111.1
#define VOLTAGE 256.0 // Pretty high where I live
#define LED_PIN 2
// #define BME280_I2C
// #define SOIL_MOISTURE_PIN 33
#endif
// #define BME280_I2C // D22 SCL & D21 SDA
// #define BH1750_I2C // D22 SCL & D21 SDA
#ifdef ESP_32_U
#define LED_PIN 2
#define SDA_PIN 21
#define SCL_PIN 22
#define BME280_I2C
#endif
#ifdef ESP_8266
#define LED_PIN LED_BUILTIN
#define SCT_013_PIN A0
#define VOLTAGE 256.0 // Pretty high where I live
#endif

// YAY GLOBALS SORRYNOTSORRY

// why, past me, did you do it like this?
// TODO these are no longer necessary
// Well...they're still "global" but only in `sensors.h`
#ifdef BMP280_I2C
extern double temperature;
extern double pressure;
#endif
#ifdef BH1750_I2C
extern double lux;
#endif
#ifdef DHT11_PIN
#include <DHTesp.h>
extern TempAndHumidity reading;
#endif
#ifdef LIGHT_SENSOR_PIN
extern int lightLevel;
#endif
#ifdef SOIL_MOISTURE_PIN
extern int soilMoisture;
#endif

#endif
