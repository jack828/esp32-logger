#ifndef Definitions_h
#define Definitions_h

#include <NTPClient.h>

// Your connected status LED (not necessary)
#define LED_PIN 2
// Light sensor
#define LIGHT_SENSOR_PIN 32 // D32
// Root PiHome logger location
#define ROOT_URI "http://192.168.0.17:3001/node"
// Time between wake-ups, in microseconds
// will drift during uptime from starting time
#define LOG_PERIOD 5 * 1000 * 1000

// YAY GLOBALS SORRYNOTSORRY
extern NTPClient timeClient;

#endif
