#ifndef Definitions_h
#define Definitions_h

// When enabled, prevents actually logging any metrics
// #define TEST_MODE

// pick a board any board
// #define ESP_32_OLED_BATT
#define ESP_32_OLED 264505746706340
#define ESP_32_POWER 171537337601956
#define ESP_32_U    255866614099504
// #define ESP_8266 // Use this one if it might blow up

// Configuration
#define NODE_LOG_PERIOD 60 * 1000 // 60 seconds

#define WIFI_CHECK_PERIOD_MS 30000 // 10s keep alive period
#define WIFI_TIMEOUT_MS 10000      // 10s WiFi connection timeout
#define WIFI_RECOVER_TIME_MS 10000 // 10s after a failed connection attempt

// This is defined using compile time flags, but clangd doesn't like it
// And if it isn't defined when compiling with the Arduino IDE, this won't break anything
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "NOT_SET"
#endif
#ifndef CHIP_ID
#define CHIP_ID ESP_32_POWER
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

#if CHIP_ID == ESP_32_OLED
#define SDA_PIN 5
#define SCL_PIN 4
// this doesn't have one but :shrug:
#define LED_PIN 16
// #define BME280_I2C
#define BME680_I2C
// #define OLED
// #define OLED_FLIP_V
// #define TOUCH_L_PIN 13
// #define TOUCH_R_PIN 15
// #define LIGHT_SENSOR_PIN 36 // SVP
// #define DHT11_PIN 25
#define SENSORS_LOG_PERIOD 60 * 1000
#endif

#if CHIP_ID == ESP_32_U
#define LED_PIN 2
#define SDA_PIN 21
#define SCL_PIN 22
#define BME280_I2C
#define SENSORS_LOG_PERIOD 60 * 1000
#endif

#ifdef ESP_32_OLED_BATT
// #define SDA_PIN 21
// #define SCL_PIN 22
#define LED_PIN 16
// #define LIGHT_SENSOR_PIN 36
// #define BME280_I2C
// #define OLED
#define SENSORS_LOG_PERIOD 60 * 1000
#endif
#if CHIP_ID == ESP_32_POWER
// #define SDA_PIN 21
// #define SCL_PIN 22
#define SCT_013_PIN A0
#define SCT_013_CALIBRATION 111.1
#define VOLTAGE 256.0 // Pretty high where I live
#define LED_PIN 2
#define SENSORS_LOG_PERIOD 1 * 1000
#endif
// #define BME280_I2C // D22 SCL & D21 SDA
// #define BH1750_I2C // D22 SCL & D21 SDA
#ifdef ESP_8266
#define LED_PIN LED_BUILTIN
#define SCT_013_PIN A0
#define VOLTAGE 256.0 // Pretty high where I live
#define SENSORS_LOG_PERIOD 1 * 1000
#endif

#endif
