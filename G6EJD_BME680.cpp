/*
    Calculate IAQ indices scaled 0-100% (100% is excellent) and 0-500 where 0 is excellent and 500 Hazardous
    The index is a hybrid of humidity and adverse gas content. See the Redme for further detaisl of the index.
    This software, the ideas and concepts is Copyright (c) David Bird 2019. All rights to this software are reserved.
    Any redistribution or reproduction of any part or all of the contents in any form is prohibited other than the following:
    1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
    2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material.
    3. You may not, except with my express written permission, distribute or commercially exploit the content.
    4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.
    The above copyright ('as annotated') notice and this permission notice shall be included in all copies or substantial portions of the Software and where the
    software use is visible to an end-user.
    THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT. FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY
    OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
    See more at http://www.dsbird.org.uk
*/
#include "G6EJD_BME680.h"
#include "definitions.h"

float hum_weighting = 0.25; // so hum effect is 25% of the total air quality score
float gas_weighting = 0.75; // so gas effect is 75% of the total air quality score

float hum_score, gas_score;
float gas_reference = 250000;
float hum_reference = 40;
int   getgasreference_count = 0;

void bme680_getGasReference(Adafruit_BME680 bme680) {
   // Now run the sensor for a burn-in period, then use combination of relative humidity and gas resistance to estimate indoor air quality as a percentage.
  Serial.println("[ BME680 ] Getting a new gas reference value");
  int readings = 20;
  for (int i = 1; i <= readings; i++){ // read gas for 20 x 0.150mS = 3.0secs
    gas_reference += bme680.readGas();
  }
  gas_reference = gas_reference / readings;
}

double bme680_getAirQuality(Adafruit_BME680 bme680) {
 //Calculate humidity contribution to IAQ index
  float current_humidity = bme680.readHumidity();
  if (current_humidity >= 38 && current_humidity <= 42)
    hum_score = 0.25*100; // Humidity +/-5% around optimum
  else
  { //sub-optimal
    if (current_humidity < 38)
      hum_score = 0.25/hum_reference*current_humidity*100;
    else
    {
      hum_score = ((-0.25/(100-hum_reference)*current_humidity)+0.416666)*100;
    }
  }

  //Calculate gas contribution to IAQ index
  float gas_lower_limit = 5000;   // Bad air quality limit
  float gas_upper_limit = 50000;  // Good air quality limit
  if (gas_reference > gas_upper_limit) gas_reference = gas_upper_limit;
  if (gas_reference < gas_lower_limit) gas_reference = gas_lower_limit;
  gas_score = (0.75/(gas_upper_limit-gas_lower_limit)*gas_reference -(gas_lower_limit*(0.75/(gas_upper_limit-gas_lower_limit))))*100;

  //Combine results for the final IAQ index value (0-100% where 100% is good quality air)
  double air_quality_score = hum_score + gas_score;

  /* Serial.println("Air Quality = "+String(air_quality_score,1)+"% derived from 25% of Humidity reading and 75% of Gas reading - 100% is good quality air"); */
  /* Serial.println("Humidity element was : "+String(hum_score/100)+" of 0.25"); */
  /* Serial.println("     Gas element was : "+String(gas_score/100)+" of 0.75"); */
  if (bme680.readGas() < 120000) Serial.println("***** Poor air quality *****");
  if ((getgasreference_count++)%10==0) bme680_getGasReference(bme680);
  Serial.printf("%2.2f%% ", air_quality_score);
  Serial.println(CalculateIAQStr(air_quality_score));
  /* Serial.println("------------------------------------------------"); */
  return air_quality_score;
}

String CalculateIAQStr(float score){
  score = (100-score)*5;
  if      (score >= 301)                  return "Hazardous";
  else if (score >= 201 && score <= 300 ) return "Very Unhealthy";
  else if (score >= 176 && score <= 200 ) return "Unhealthy";
  else if (score >= 151 && score <= 175 ) return "Unhealthy for Sensitive Groups";
  else if (score >=  51 && score <= 150 ) return "Moderate";
  else if (score >=  00 && score <=  50 ) return "Good";
  return "Error";
}
