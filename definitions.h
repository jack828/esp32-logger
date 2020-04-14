#ifndef Definitions_h
#define Definitions_h

// pick a board any board
// #define ESP_32_OLED_BATT
// #define ESP_32_OLED
#define ESP_32

#include <NTPClient.h>

// Hardware specific config
#ifdef ESP_32_OLED_BATT
#define LED_PIN 16 // BATT only
#define LIGHT_SENSOR_PIN 36 // TODO
#endif
#ifdef ESP_32_OLED
#define LED_PIN 16 // this doesn't have one but :shrug:
#define LIGHT_SENSOR_PIN 36 // SVP
#define DHT11_PIN 25
#endif
#ifdef ESP_32
#define LED_PIN 2
#define LIGHT_SENSOR_PIN 32 // D32
#define BME280_I2C
#endif


// Root PiHome logger location
#define ROOT_URI "http://192.168.0.42:3001/node"
// Time between wake-ups, in microseconds
// will drift during uptime from starting time
// if using deep sleep, max 1 hour on ESP8266
#define LOG_PERIOD 5 * 1000 * 1000
#define DELAY_PERIOD 10 * 60 * 1000 // 10 minutes

// YAY GLOBALS SORRYNOTSORRY
extern NTPClient timeClient;

#endif
