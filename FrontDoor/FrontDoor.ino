/*
    Frontdoor - MySensors dollhouse front dooralarm and doorbell sensor.

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

#define MY_NODE_FRONTDOOR
#define SKETCH_NAME        "Front door sensor"
#define SKETCH_VERSION     "1.0"
#define PIN_LED            (4)

//#define MY_DEBUG  
#include <Vcc.h>

#include "DemoSensorConfig.h"

#define CHILD_ID_DOORBELL    (0)
#define CHILD_ID_DOORALARM   (1)

#define PIN_DOORBELL         (2)
#define PIN_DOORALARM        (3)

#define DEBOUNCE_MS          (15)
//#define BLIND_TIME_MS        (1000)

static int State_Doorbell;
static int State_DoorAlarm;

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

  present(CHILD_ID_DOORBELL,  S_BINARY, "Doorbell");
  present(CHILD_ID_DOORALARM, S_DOOR,   "Alarm");
}

void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));

  // Deactivate internal pull-up. We're using external pullup to save some battery power.
  pinMode(PIN_DOORBELL,  INPUT);
  pinMode(PIN_DOORALARM, INPUT);
  State_Doorbell  = digitalRead(PIN_DOORBELL);
  State_DoorAlarm = digitalRead(PIN_DOORALARM);
}

// Loop will iterate on changes on the BUTTON_PINs
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

  #ifdef BLIND_TIME_MS
    // Allow battery to recover
    sleep( BLIND_TIME_MS );
  #endif
    
  switch( sleep( digitalPinToInterrupt(PIN_DOORBELL), CHANGE, digitalPinToInterrupt(PIN_DOORALARM), CHANGE, 0ul /*Sleep forever*/, false) )
  {
    case digitalPinToInterrupt(PIN_DOORBELL):
      delay(DEBOUNCE_MS);
      send(MyMessage(CHILD_ID_DOORBELL, V_STATUS).set(LOW == digitalRead(PIN_DOORBELL)));
      break;
    case digitalPinToInterrupt(PIN_DOORALARM):
      delay(DEBOUNCE_MS);
      send(MyMessage(CHILD_ID_DOORALARM, V_TRIPPED).set(LOW == digitalRead(PIN_DOORALARM)));
      break;
    default:
      break;
  }
}

