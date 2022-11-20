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

 - [node.cpp (the ESP board itself)](./node.cpp)
 - [sensors.cpp (attached modules' data)](./sensors.cpp)

## Libraries

All required libraries for the various minor variations in this project are kept as git submodules within `./libraries`.

To initialise these, run `git submodule init && git submodule update` in the project root.

### EmonLib Patches

If you decide against the submodule approach for the libraries, an want to use the IDE provided ones:

To use the EmonLib library with the 12-bit ADC aboard the ESP32, you'll need to patch it.
 - Add the library using the board manager
 - Open the library in the editor of your choice (Neovim) and manually patch it
The location can be found depending on your setup using the [instructions here](https://support.arduino.cc/hc/en-us/articles/4411202655634-Find-Arduino-IDE-files-and-folders#:~:text=macOS%3A%20Arduino%20%3E%20Preferences-,Libraries,-Libraries%20installed%20with)

1. in `EmonLib.h`
Amend the block at lines 36-40.

```diff
-#if defined(__arm__)
#define ADC_BITS    12
-#else
-#define ADC_BITS    10
-#endif
```

2. in `EmonLib.cpp`
Replace the 3 occurrences of `1024` with `ADC_COUNTS` in lines 117, 119, 201.

```diff
-    offsetV = offsetV + ((sampleV - offsetV) / ADC_COUNTS);
+    offsetV = offsetV + ((sampleV - offsetV) / 1024);
```


## Setup

Fork the repo, clone, initialise submodules, and configure to match your setup.

Please see [credentials.h](./credentials.h) for credential information. Use `.tpl` as a starting point.
Please see [definitions.h](./definitions.h) for all configurables.

Setup or update arduino-cli:

```
$ curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.local/bin sh
```

## Usage

This project has been designed to use the [Arduino CLI](https://arduino.github.io/arduino-cli/latest/) by usage of a [Makefile](./Makefile).

Please check the file for the appropriate commands - you may need to adjust which port you use.

After the initial flash to the device, you can use the AsyncElegantOTA solution to keep the node firmware updated.

Using an mDNS browser you can discover nodes without having the IP address. They respond on `http://esp<node efuse ID>.local/`.

Config is available via `http://<node-ip>/`

OTA updates available via `http://<node-ip>/update`

CLI based automatic rollout to multiple nodes is possible using a list of nodes in the `Makefile`. You need to make a note of the numerical chip ID (which is the decimal representation of the hex portion of the mDNS URL), which can then be used in `definitions.h` to modify compilation behaviour.

Once this is setup you can run `make deploy` to rollout all nodes or `make deploy-node node=123456789` to target a specific node.

## TODO

 - flash-and-go setup with an app
 - optimise flash storage usage, remove strings etc
 - Maybe add a discovery version with the LCD that can be like a status page?
 - if ran out of space change partition scheme for `Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)` - `--build-properties build.partitions=min_spiffs,upload.maximum_size=1966080`
 - lock/stop sending metrics when performing OTA
 - Automatic update fetching from internet somewhere? https://www.lab4iot.com/2021/02/21/esp32-secure-firmware-update-over-the-air-ota/
 - Hall effect sensor support https://moderndevice.com/product/current-sensor/
 - update resolution of uptime + use when setting up batching https://forum.arduino.cc/t/question-about-esp32-millis-counter/699263
 - update energy monitor to https://innovatorsguru.com/pzem-004t-v3/ https://github.com/mandulaj/PZEM-004T-v30

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

## Monitoring information

### Environment

 - Temperature, Pressure, Humidity all done through BME280/BME680
 - IAQ, eC02, bVOCe done via BME680 after calibration burn in period

### Energy

Performed via EmonLib and a split-core current transformer. In my case, I used an SCT-013-000, but there are many different varieties out there.

Notably:
 - SCT-013-XXX (split core, 30-150A)
 - KCT38 (split core, 50-600A)
 - BH-066-XXX (solid core, 10-400A)
 - PZCT-02 (split core, 100A)
 - BZCT30AL (solid core, 50/100A)
 - PZEM-004T - comes with different CTs. Measures voltage, frequency, and power factor via MODBUS interface.

When using EmonLib and a current transformer, there is some additional hardware to ensure the ESP32 can measure the current correctly.

First, we need to convert the alternating current into an AC voltage, biased so the 0-3.3v ADC can read it without setting on fire :).

We do this by applying a burden resistor to the CT (if not already installed) and then connecting the signal output to a voltage divider to offset the signal to the ADC range.

Once complete, the signal now read by the ADC has the following performed:
For each reading, we can define the number of samples for the library to calculate; the bigger the
number of samples, the longer the method will take to calculate the Current.
The library uses the root mean square to calculate the current:
Supposing N equals the number of samples, and u(n) being the voltage sample that we get from
the ADC:
[(1) PAGE 10 EQUATION 1](https://www.researchgate.net/publication/336110101_IoT_Power_Monitoring_System_for_Smart_Environments/link/5d8eaae3299bf10cff15227c/download)
This (1) Urms value is then multiplied by the calibration coefficients, based on the value we
define at the beginning for each ADC input.
Then using urms × calcoeff results in the current value, we call this Irms that will be used to
calculate the apparent power by being multiplied with a voltage constant

nice PCB layout https://github.com/bvarner/PZCT-02-BurdenShift


# Author

[Jack Burgess](https://jackburgess.dev)

# License

MIT
