/*
    SoilMoisture - MySensors dollhouse soil moisture sensor.

    Created by Ivo Pullens, Emmission, 2017

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define MY_NODE_SOILMOISTURE
#define SKETCH_NAME        "Soil moisture sensor"
#define SKETCH_VERSION     "1.0"
#define PIN_LED            (4)

//#define MY_DEBUG
#include <Vcc.h>

#include "DemoSensorConfig.h"

#define CHILD_ID_SOILMOISTURE  (0)

#define PIN_SUPPLY             (5)
#define PIN_SENSOR             (A4)

static float oldBatteryPcnt = 0.0;
const float batteryReportHysteresis = 0.5;   // Amount of change since last reported value before new value will be sent
const float VccMin        = 1.8;             // Minimum expected Vcc level, in Volts: BOD level at 1.8V.
const float VccMax        = 3.0;             // Maximum expected Vcc level, in Volts: Nominal for CR2032
const float VccCorrection = 1.0;             // Measured Vcc by multimeter divided by reported Vcc
static Vcc vcc(VccCorrection);

const uint8_t Iterations = 8; 
const float   MaxAdcInWater = 500.0;         // Maximum value when submerged in water. Indicates 100% moisture.
const float   MinAdcInAir   = 0.0;           // Minimum value when held in air. Indicates 0% moisture.

// Domoticz assumes value is in cb:
// 0-10 Saturated Soil. Occurs for a day or two after irrigation 
// 10-20 Soil is adequately wet (except coarse sands which are drying out at this range) 
// 30-60 Usual range to irrigate or water (except heavy clay soils). 
// 60-100 Usual range to irrigate heavy clay soils 
// 100-200 Soil is becoming dangerously dry for maximum production. Proceed with caution.   
// For simplicity, we do a linear map from ADC values to cb values.
const float   cbDry = 200.0;                 // cb value for dry soil
const float   cbWet = 0.0;                   // cb value for saturated soil

const uint32_t SleepIntervalMs = 60ul*1000ul;

static float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void presentation()
{
  Serial.println(F("-- Init MySensors"));
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
  Serial.print(F("NodeID: "));
  Serial.println(getNodeId());

  present(CHILD_ID_SOILMOISTURE, S_MOISTURE, "Moisture");
}

void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));
}

void loop()
{
  float avg = 0.0;
  // Power on sensor
  pinMode(PIN_SUPPLY, OUTPUT);
  digitalWrite(PIN_SUPPLY, HIGH);
  // Allow Vcc to settle
  sleep( 16 );
  for (uint8_t i = 0; i < Iterations; ++i)
  {
    avg += analogRead( PIN_SENSOR );
  }
  // Power down sensor
  digitalWrite(PIN_SUPPLY, LOW);
  pinMode(PIN_SUPPLY, INPUT);

  avg /= float(Iterations);
  avg = mapfloat( avg, MinAdcInAir, MaxAdcInWater, cbDry, cbWet );
  avg = constrain( avg, cbWet, cbDry );

  send(MyMessage(CHILD_ID_SOILMOISTURE, V_LEVEL).set(avg, 0));

  // Measure & send battery level after loading the battery
  const float batteryPcnt = vcc.Read_Perc(VccMin, VccMax);

  // Battery readout tends to jitter a little around a certain value (e.g. ADC noise).
  // Require a certian minimum change before sending an updated value.
  if (fabs(oldBatteryPcnt - batteryPcnt) >= batteryReportHysteresis)
  {
      sendBatteryLevel(batteryPcnt + 0.5); // Add 0.5 to correctly round to int
      oldBatteryPcnt = batteryPcnt;
  }

#ifdef PIN_LED
  waitLedPatternFinished();
#endif
  
  (void)sleep(SleepIntervalMs);
}

