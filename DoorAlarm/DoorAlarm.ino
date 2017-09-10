/*
    Dooralarm - MySensors dooralarm sensor, for 2 magnetic door switches.

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

#define MY_NODE_DOORALARM1
//#define MY_NODE_DOORALARM2
#define SKETCH_NAME        "Door alarm sensor"
#define SKETCH_VERSION     "1.0"
#define PIN_LED            (4)

//#define MY_DEBUG
#include <Vcc.h>

#include "DemoSensorConfig.h"

#define CHILD_ID_DOORALARM1  (0)
#define CHILD_ID_DOORALARM2  (1)

#define PIN_DOORALARM1       (2)
#define PIN_DOORALARM2       (3)

#define DEBOUNCE_MS          (15)
#define BLIND_TIME_MS        (1000)

static int State_DoorAlarm1;
static int State_DoorAlarm2;

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

  present(CHILD_ID_DOORALARM1, S_DOOR, "Alarm1");
  present(CHILD_ID_DOORALARM2, S_DOOR, "Alarm2");
}

void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));

  // Deactivate internal pull-up. We're using external pullup to save some battery power.
  pinMode(PIN_DOORALARM1,  INPUT);
  pinMode(PIN_DOORALARM2, INPUT);
  State_DoorAlarm1 = digitalRead(PIN_DOORALARM1);
  State_DoorAlarm2 = digitalRead(PIN_DOORALARM2);
}

// Loop will iterate on changes on the BUTTON_PINs
void loop()
{
  // Debounce
  delay(DEBOUNCE_MS);

  int state = digitalRead(PIN_DOORALARM1);
  if (state != State_DoorAlarm1)
  {
    send(MyMessage(CHILD_ID_DOORALARM1, V_TRIPPED).set(LOW == state));
    State_DoorAlarm1 = state;
  }

  state = digitalRead(PIN_DOORALARM2);
  if (state != State_DoorAlarm2)
  {
    send(MyMessage(CHILD_ID_DOORALARM2, V_TRIPPED).set(LOW == state));
    State_DoorAlarm2 = state;
  }

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
    waitLedPatternFinished(false);
  #endif
    
  #ifdef BLIND_TIME_MS
    // Allow battery to recover
    sleep( BLIND_TIME_MS );
  #endif
  
  sleep( digitalPinToInterrupt(PIN_DOORALARM1), CHANGE, digitalPinToInterrupt(PIN_DOORALARM2), CHANGE, 0ul /*Sleep forever*/, false);
}

