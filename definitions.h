#ifndef Definitions_h
#define Definitions_h

// pick a board any board
// #define ESP_32_OLED_BATT
#define ESP_32_OLED
// #define ESP_32

#include <NTPClient.h>

// Hardware specific config
#ifdef ESP_32_OLED_BATT
#define LED_PIN 16
#define LIGHT_SENSOR_PIN 36 // TODO
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
extern double temperature = 0.0;
extern double pressure = 0.0;
#endif
#ifdef BH1750_I2C
extern double lux = 0.0;
#endif
#ifdef DHT11_PIN
#include <DHTesp.h>
extern TempAndHumidity reading;
#endif
#ifdef LIGHT_SENSOR_PIN
int lightLevel;
#endif

#endif
