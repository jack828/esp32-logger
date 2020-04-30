#include "BME680.h"
#include "../../definitions.h"

void bme680_doSensorThing() {

}

void bme680_getGasReference(String &bme680) {
   // Now run the sensor for a burn-in period, then use combination of relative humidity and gas resistance to estimate indoor air quality as a percentage.
  Serial.println("Getting a new gas reference value");
  int readings = 10;
  for (int i = 1; i <= readings; i++){ // read gas for 10 x 0.150mS = 1.5secs
    gas_reference += bme.readGas();
  }
  gas_reference = gas_reference / readings;
}
