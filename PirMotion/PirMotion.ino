/*
    PirMotion - MySensors dollhouse PIR motion sensor.

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

#define MY_NODE_PIRMOTION
#define SKETCH_NAME        "Pir motion sensor"
#define SKETCH_VERSION     "1.0"
#define PIN_LED            (4)

//#define MY_DEBUG
#include <Vcc.h>

#include "DemoSensorConfig.h"

#define CHILD_ID_PIRMOTION   (0)

#define PIN_PIR              (2)

static float oldBatteryPcnt = 0.0;
const float batteryReportHysteresis = 0.5;   // Amount of change since last reported value before new value will be sent
const float VccMin        = 1.8;             // Minimum expected Vcc level, in Volts: BOD level at 1.8V.
const float VccMax        = 3.0;             // Maximum expected Vcc level, in Volts: Nominal for CR2032
const float VccCorrection = 1.0;             // Measured Vcc by multimeter divided by reported Vcc
static Vcc vcc(VccCorrection);

void presentation()
{
  Serial.println(F("-- Init MySensors"));
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
  Serial.print(F("NodeID: "));
  Serial.println(getNodeId());

  present(CHILD_ID_PIRMOTION, S_MOTION, "Motion");
}

void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));

  pinMode(PIN_PIR, INPUT);
}

void loop()
{
#ifdef PIN_LED
  waitLedPatternFinished();
#endif
  
  sleep( digitalPinToInterrupt(PIN_PIR), CHANGE, 0ul /*Sleep forever*/, false);

  send(MyMessage(CHILD_ID_PIRMOTION, V_TRIPPED).set(digitalRead(PIN_PIR) == HIGH));

  // Measure & send battery level after loading the battery
  const float batteryPcnt = vcc.Read_Perc(VccMin, VccMax);

  // Battery readout tends to jitter a little around a certain value (e.g. ADC noise).
  // Require a certian minimum change before sending an updated value.
  if (fabs(oldBatteryPcnt - batteryPcnt) >= batteryReportHysteresis)
  {
      sendBatteryLevel(batteryPcnt + 0.5); // Add 0.5 to correctly round to int
      oldBatteryPcnt = batteryPcnt;
  }
}

