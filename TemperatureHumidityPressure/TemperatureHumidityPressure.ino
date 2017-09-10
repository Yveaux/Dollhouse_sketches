/*
    TemperatureHumidityPressure - MySensors dollhouse temp/hum/pres sensor.

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

#define MY_NODE_TEMPHUMPRES
#define SKETCH_NAME        "TempHumPres"
#define SKETCH_VERSION     "1.0"
#define PIN_LED            (4)

//#define MY_DEBUG

#include "DemoSensorConfig.h"
#include <BME280I2C.h>
#include <Vcc.h>

#define CHILD_ID_TEMPERATURE  (0)
#define CHILD_ID_HUMIDITY     (1)
#define CHILD_ID_PRESSURE     (2)

static BME280I2C bme;         // Default : forced mode, standby time = 1000 ms, Current Consumption =  0.16 μA
const bool metric = true;
const uint8_t prsUnit_hPa = B001; // B000 = Pa, B001 = hPa, B010 = Hg, B011 = atm, B100 = bar, B101 = torr, B110 = N/m^2, B111 = psi

static float oldBatteryPcnt = 0.0;
const float batteryReportHysteresis = 0.5;   // Amount of change since last reported value before new value will be sent
const float VccMin        = 1.8;             // Minimum expected Vcc level, in Volts: BOD level at 1.8V.
const float VccMax        = 3.0;             // Maximum expected Vcc level, in Volts: Nominal for CR2032
const float VccCorrection = 1.0;             // Measured Vcc by multimeter divided by reported Vcc
static Vcc vcc(VccCorrection);

const uint32_t SleepTime_Ms = 60UL*1000UL;

void presentation()
{
  Serial.println(F("-- Init MySensors"));
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
  Serial.print(F("NodeID: "));
  Serial.println(getNodeId());

  present(CHILD_ID_TEMPERATURE, S_TEMP, "Temperature");
  present(CHILD_ID_HUMIDITY,    S_HUM,  "Humidity");
  present(CHILD_ID_PRESSURE,    S_BARO, "Pressure");
}

void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));

  while( !bme.begin() )
  {
    Serial.println(F("BME280 not found"));
    delay(1000);
  }
}

void loop()
{
  float pres;
  float temp;
  float hum;
  bme.read(pres, temp, hum, prsUnit_hPa, metric);
  
  send( MyMessage(CHILD_ID_TEMPERATURE, V_TEMP).set(double(temp), 1 /*#decimals*/) );
  send( MyMessage(CHILD_ID_HUMIDITY,    V_HUM).set(double(hum), 1 /*#decimals*/) );
  send( MyMessage(CHILD_ID_PRESSURE,    V_PRESSURE).set(double(pres), 1 /*#decimals*/) );

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
  
  sleep( SleepTime_Ms );
}

