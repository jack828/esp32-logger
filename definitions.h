#ifndef Definitions_h
#define Definitions_h

// pick a board any board
#define ESP_32_OLED_BATT
// #define ESP_32_OLED
// #define ESP_32

#include <NTPClient.h>

/*
 * You can define any of these to enable sensors
 * and additional functionality, depending on your setup:
 *
 * LED_PIN - status LED, flashes when connecting to WiFi.
 * LIGHT_SENSOR_PIN - enable analogRead of a light sensor
 * BMP280_I2C - Temp & Pressure sensor
 * BME280_I2C - Temp, Humidity, and Pressure sensor
 * BH1750_I2C - Light sensor
 * DHT11_PIN - Temp & Humidity sensor
 * OLED - Show a HUD of current sensors. Node will still log readings
 *
 * ROOT_URI - pi-home node.js server endpoint
 * LOG_PERIOD - Time between logs, deep-sleep between wake-ups
 */

// Hardware specific config
// so m a n y
#ifdef ESP_32_OLED_BATT
#define LED_PIN 16
// #define LIGHT_SENSOR_PIN 36
#define BME280_I2C
#endif
#ifdef ESP_32_OLED
// this doesn't have one but :shrug:
#define LED_PIN 16
#define OLED
// #define LIGHT_SENSOR_PIN 36 // SVP
#define DHT11_PIN 25
#endif
#ifdef ESP_32
#define LED_PIN 2
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
extern NTPClient timeClient;

#ifdef BME280_I2C
extern double temperature;
extern double pressure;
extern double humidity;
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
int lightLevel;
#endif

#endif
