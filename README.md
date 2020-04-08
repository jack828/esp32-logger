# esp32-logger

## Libraries

You need:
 - Adafruit DHT
 - Adafruit Unified Sensor


## Flow

How this will work:
1. Boot & connect to WiFi
2. Authenticate node using mac address with log server
3. Record sensors with log server
4. Sleep for some duration
5. Wake, record readings with log server
6. Repeat 3-4

## Setup

Fork the repo, clone, and configure to match your setup.

Please see [credentials.h](./credentials.h) for credential information. Use `.tpl` as a starting point.
Please see [definitions.h](./definitions.h) for all configurables.

## TODO

 - Collision detection, no idea if identifying as an existing node
 - flash-and-go setup with an app
 - It would be neato a node could "hunt" for the root, and then pass that information onto new nodes in the network, reducing setup configuration

## Useful links

 - https://lastminuteengineers.com/esp32-deep-sleep-wakeup-sources/
 - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/deep-sleep-stub.html
 - https://desire.giesecke.tk/index.php/2018/01/30/esp32-wiki-entries/
