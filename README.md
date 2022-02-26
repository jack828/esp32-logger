# esp32-logger

## Features

 - mDNS discovery
 - InfluxDB support
 - Auto WiFi keep-alive task
 - FLASHING LED
 - AsyncElegantOTA updates
 - Sexy & sleek custom dashboard for updating config options + InfluxDB tags

### Logging

Logging is the primary function of this project. Use this chunk of silicon to collect data, and send it to a bigger chunk of silicon to process and store the data on bits of magnetic metal. Then, at a later date, use another chunk of silicon to view the data in really pretty graphs.

As this README is likely to fall out of date as to what exactly is being tracked, check out these two files for info:

 - [Node (the ESP board itself) data in node.cpp](./node.cpp)
 - [Sensors (attached modules' data in sensors.cpp](./sensors.cpp)

## Libraries

You need one of:
 - Adafruit BME 280 + Adafruit Unified Sensor
 - EmonLib + SCT-013-000 & associated hardware

General libraries:
 - [InfluxDB Arduino Client](https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino)
 - [AsyncElegantOTA](https://github.com/ayushsharma82/AsyncElegantOTA)

### EmonLib Patches

To use the EmonLib library with the 12-bit ADC aboard the ESP32, you'll need to patch it.
 - Add the library using the board manager
 - Open the library in the editor of your choice (Neovim) and manually patch it
 - Or apply the patch in the project
The location can be found depending on your setup using the [instructions here](https://support.arduino.cc/hc/en-us/articles/4411202655634-Find-Arduino-IDE-files-and-folders#:~:text=macOS%3A%20Arduino%20%3E%20Preferences-,Libraries,-Libraries%20installed%20with)

## Setup

Fork the repo, clone, and configure to match your setup.

Please see [credentials.h](./credentials.h) for credential information. Use `.tpl` as a starting point.
Please see [definitions.h](./definitions.h) for all configurables.

## Usage

After the initial flash to the device, you can use the AsyncElegantOTA solution to keep the node firmware updated.

Using an mDNS browser you can discover nodes without having the IP address. They respond on `http://ESP<node efuse ID>.local/`.

Config is available via `http://<node-ip>/`

OTA updates available via `http://<node-ip>/update`

## TODO

 - flash-and-go setup with an app
 - optimise flash storage usage, remove strings etc
 - Maybe add a discovery version with the LCD that can be like a status page?

## Useful links

 - https://lastminuteengineers.com/esp32-deep-sleep-wakeup-sources/
 - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/deep-sleep-stub.html
 - https://desire.giesecke.tk/index.php/2018/01/30/esp32-wiki-entries/

## ESP_32_OLED_BATT

This is a all-in-one kit from AliExpress.
 - ESP-WROOM-32 Module (Wifi, Bluetooth, two cores)
 - USB to serial bridge with Silicon Labs CP210X Chip
 - Charge Circuit for an 18650 battery (backside of board)
 - OLED display (SSD1306 or compatible) I2C version
 - LED on GPIO16 (was `(int8_t) 16` for me with  ESP32 Dev Module board)
 - Power switch
 - AMS1117 voltage regulator (has a high quiescent current!)


# Author

[Jack Burgess](https://jackburgess.dev)

# License

MIT
