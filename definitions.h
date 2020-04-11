#ifndef Definitions_h
#define Definitions_h

#include <NTPClient.h>

// Your connected status LED (not necessary)
#define LED_PIN 2

// Light sensor
// #define LIGHT_SENSOR_PIN 32 // ESP32 - D32
#define LIGHT_SENSOR_PIN 36 // ESP32 OLED - SVP

// DH11 sensor
#define DHT11_PIN 25 // ESP OLED

// Root PiHome logger location
#define ROOT_URI "http://192.168.0.17:3001/node"
// Time between wake-ups, in microseconds
// will drift during uptime from starting time
// if using deep sleep, max 1 hour on ESP8266
#define LOG_PERIOD 5 * 1000 * 1000
#define DELAY_PERIOD 10 * 60 * 60 * 1000 // 10 minutes

// YAY GLOBALS SORRYNOTSORRY
extern NTPClient timeClient;

#endif
