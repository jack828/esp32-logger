#ifndef Definitions_h
#define Definitions_h

// pick a board any board
// #define ESP_32_OLED_BATT
// #define ESP_32_OLED
#define ESP_32

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
 *
 * ROOT_URI - pi-home node.js server endpoint
 * LOG_PERIOD - Time between logs, deep-sleep between wake-ups
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
#define OLED
#define OLED_FLIP_V
#define TOUCH_L_PIN 13
#define TOUCH_R_PIN 15
// #define LIGHT_SENSOR_PIN 36 // SVP
// #define DHT11_PIN 25
#define BME280_I2C
#endif
#ifdef ESP_32
#define SDA_PIN 21
#define SCL_PIN 22
#define LED_PIN 2
#define BME280_I2C
#define SOIL_MOISTURE_PIN 33
#endif
// #define BH1750_I2C // D22 SCL & D21 SDA
// #define BME280_I2C // D22 SCL & D21 SDA

// Root PiHome logger location
#define ROOT_URI "http://192.168.0.42:3001/node"
// #define ROOT_URI "http://192.168.0.17:3001/node" // Laptop
// Time between wake-ups, in microseconds
// will drift during uptime from starting time
// if using deep sleep, max 1 hour on ESP8266
#define LOG_PERIOD 10 * 60 * 1000 * 1000 // 10 minutes, microseconds

// YAY GLOBALS SORRYNOTSORRY

#ifdef BME680_I2C
extern double temperature;
extern double pressure;
extern double humidity;
extern double airQuality;
extern double vpd;
#endif
#ifdef BME280_I2C
extern double temperature;
extern double pressure;
extern double humidity;
extern double vpd;
#endif
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
