/*
    Breadboard button - A basic MySensors breadboard setup containing only one button.

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

#define MY_NODE_BREADBOARDBUTTON
#define SKETCH_NAME        "Breadboard button sensor"
#define SKETCH_VERSION     "1.0"
//#define PIN_LED            (4)

//#define MY_DEBUG
#include <Vcc.h>

#include "DemoSensorConfig.h"

#define CHILD_ID_BUTTON      (0)

#define PIN_BUTTON           (2)

#define DEBOUNCE_MS          (15)
//#define BLIND_TIME_MS        (1000)

static int State_Button;

const float VccMin        = 1.8;             // Minimum expected Vcc level, in Volts: BOD level at 1.8V.
const float VccMax        = 3.2;             // Maximum expected Vcc level, in Volts: Nominal for 2xAA
const float VccCorrection = 1.0;             // Measured Vcc by multimeter divided by reported Vcc
static Vcc vcc(VccCorrection);

void presentation()
{
  Serial.println(F("-- Init MySensors"));
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
  Serial.print(F("NodeID: "));
  Serial.println(getNodeId());

  present(CHILD_ID_BUTTON,  S_BINARY, "Button");
}

void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  State_Button  = digitalRead(PIN_BUTTON);
}

// Loop will iterate on changes on the BUTTON_PINs
void loop()
{
  // Debounce
  delay(DEBOUNCE_MS);

  send(MyMessage(CHILD_ID_BUTTON, V_STATUS).set(LOW == digitalRead(PIN_BUTTON)));

  // Measure & send battery level after loading the battery
  const float batteryPcnt = vcc.Read_Perc(VccMin, VccMax);
  sendBatteryLevel(batteryPcnt + 0.5); // Add 0.5 to correctly round to int

  #ifdef PIN_LED
    waitLedPatternFinished();
  #endif

  #ifdef BLIND_TIME_MS
    // Allow battery to recover
    sleep( BLIND_TIME_MS );
  #endif
    
  sleep( digitalPinToInterrupt(PIN_BUTTON), CHANGE, 0ul, false );
}

