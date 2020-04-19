#ifndef Definitions_h
#define Definitions_h

#include "credentials.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"

// pick a board any board
// #define ESP_32_OLED_BATT
// #define ESP_32_OLED
#define ESP_32

// TODO is NTP actually worth it?
// #include <NTPClient.h>
// extern NTPClient timeClient;

// Hardware specific config
#ifdef ESP_32_OLED_BATT
#define LED_PIN 16
#define LIGHT_SENSOR_PIN 36 // TODO
#define BME280_I2C
#endif
#ifdef ESP_32_OLED
// this doesn't have one but :shrug:
#define LED_PIN 16
#define LIGHT_SENSOR_PIN 36 // SVP
#define DHT11_PIN 25
#endif
#ifdef ESP_32
#define LED_PIN 2
#define LIGHT_SENSOR_PIN 32 // D32
#define BME280_I2C // D22 & D21
#endif


// Root PiHome logger location
#define ROOT_URI "http://192.168.0.42:3001/node"
// #define ROOT_URI "http://192.168.0.17:3001/node" // Laptop
// Time between wake-ups, in microseconds
// will drift during uptime from starting time
// if using deep sleep, max 1 hour on ESP8266
#define LOG_PERIOD 10 * 60 * 1000 * 1000 // 10 minutes, microseconds

extern EventGroupHandle_t wifi_event_group;

extern esp_ip4_addr_t ip_addr;
extern esp_netif_t *netif;

extern const int8_t READY_BIT;

#endif
