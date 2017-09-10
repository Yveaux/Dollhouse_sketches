/*
    Solar - MySensors dollhouse solar sensor.

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

#define MY_NODE_SOLAR
#define SKETCH_NAME        "Solar"
#define SKETCH_VERSION     "1.0"
#define PIN_LED            (4)

//#define MY_DEBUG

#include "DemoSensorConfig.h"
#include <Wire.h>
#include <Adafruit_INA219.h>

#define CHILD_ID_SOLAR    (0)

const uint32_t SleepIntervalMs = 60ul*1000ul;

static Adafruit_INA219 ina219;

void presentation()
{
  Serial.println(F("-- Init MySensors"));
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
  Serial.print(F("NodeID: "));
  Serial.println(getNodeId());

  present(CHILD_ID_SOLAR, S_MULTIMETER, "Voltage");
  present(CHILD_ID_SOLAR, S_MULTIMETER, "Current");
}

void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));

  ina219.begin();
  ina219.setCalibration_16V_400mA();

#ifdef MY_DEBUG
  Serial.print(F("Bus[V]"));
  Serial.print(F("\tCurrent[mA]"));
  Serial.println();
#endif
}

void loop()
{
  float busvoltage = ina219.getBusVoltage_V();
  float current_A = ina219.getCurrent_mA() / 1000.0;
  
#ifdef MY_DEBUG
  Serial.print(busvoltage); Serial.print("\t");
  Serial.print(current_A); Serial.print("\t");
  Serial.println();
#endif

  send(MyMessage(CHILD_ID_SOLAR, V_VOLTAGE).set(busvoltage, 3));   
  send(MyMessage(CHILD_ID_SOLAR, V_CURRENT).set(current_A, 6));   

#ifdef PIN_LED
  waitLedPatternFinished();
#endif

  (void)sleep(SleepIntervalMs);
}

