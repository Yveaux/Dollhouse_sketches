/*
    Toilet - MySensors dollhouse toilet seat sensor.

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

#define MY_NODE_TOILET
#define SKETCH_NAME        "Toilet"
#define SKETCH_VERSION     "1.0"
#define PIN_LED            (4)

// #define MY_DEBUG

#include "DemoSensorConfig.h"
#include <Vcc.h>

#define CHILD_ID_TOILET    (0)

#define PIN_TOILET         (2)

#define DEBOUNCE_MS        (15)

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

  present(CHILD_ID_TOILET, S_BINARY, "Toilet seat");
}

void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));

  // Deactivate internal pull-up. We're using external pullup to save some battery power.
  pinMode(PIN_TOILET, INPUT);
}

void loop()
{
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

  sleep( digitalPinToInterrupt(PIN_TOILET), CHANGE, 0ul /*Sleep forever*/);

  // Debounce
  delay(DEBOUNCE_MS);

  send(MyMessage(CHILD_ID_TOILET, V_STATUS).set(digitalRead(PIN_TOILET) == HIGH));
}

