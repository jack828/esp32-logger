#ifndef Definitions_h
#define Definitions_h

#define ESP_32_OLED_BATT
#define ESP_32_OLED
// #define ESP_32

#include <NTPClient.h>

// Your connected status LED (not necessary)
#ifdef ESP_32_OLED_BATT
#define LED_PIN 16 // BATT only
#endif
#ifdef ESP_32_OLED
#define LED_PIN 16 // TODO Check
#endif
#ifdef ESP_32_OLED_BATT
#define LED_PIN 2
#endif

// Light sensor
#ifdef ESP_32_OLED_BATT
#define LIGHT_SENSOR_PIN 36 // TODO
#endif
#ifdef ESP_32_OLED
#define LIGHT_SENSOR_PIN 36 // SVP
#endif
#ifdef ESP_32
#define LIGHT_SENSOR_PIN 32 // D32
#endif

// DH11 sensor
#define DHT11_PIN 25 // ESP OLED

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
