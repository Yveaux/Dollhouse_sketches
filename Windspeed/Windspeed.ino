/*
    Windspeed - MySensors dollhouse windspeed sensor.

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

#define MY_NODE_WINDSPEED
#define SKETCH_NAME        "Windspeed sensor"
#define SKETCH_VERSION     "1.0"
#define PIN_LED            (4)

//#define MY_DEBUG  
#include <Vcc.h>

#include "DemoSensorConfig.h"

#define CHILD_ID_WINDSPEED   (0)
//#define CHILD_ID_WINDGUST    (1)

#define PIN_REED             (2)

const unsigned long DebounceTimeUs = 10000ul;
const uint32_t UpdateIntervalMs = 2ul*1000ul; // How often to send windspeed when movement is detected
const float KmHPerRevsSec = 1.0;            // Calibration of revs/sec to Km/h

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

  present(CHILD_ID_WINDSPEED, S_WIND, "Wind speed");
//  present(CHILD_ID_WINDGUST,  S_WIND, "Wind gust");
}

void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));

  // Deactivate internal pull-up. We're using external pullup to save some battery power.
  pinMode(PIN_REED, INPUT);
}

static unsigned long DebounceStartTimeUs;
static unsigned long StartTimeUs;
static uint16_t Revolutions;

static void interruptHandler()
{
  unsigned long NowUs = micros();
  if (NowUs - DebounceStartTimeUs >= DebounceTimeUs)
  {
    DebounceStartTimeUs = NowUs;
    Revolutions++;
  }
  // Clear pending interrupt (if any)
  EIFR = _BV( digitalPinToInterrupt(PIN_REED) );
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
  
  sleep( digitalPinToInterrupt(PIN_REED), RISING, 0ul /*Sleep forever*/, false);

  // Store time of IRQ that woke us up. During sleep we had no notion of time.
  StartTimeUs = micros();
  DebounceStartTimeUs = StartTimeUs;
  Revolutions = 0;

  attachInterrupt( digitalPinToInterrupt(PIN_REED), interruptHandler, RISING );

  for (;;)
  {
    wait( UpdateIntervalMs );
    // Get #revolutions, the time it took to make these revolutions and restart the revolution counter.
    noInterrupts();
    unsigned long NowUs = micros();
    uint16_t Revs = Revolutions;
    Revolutions = 0u;
    interrupts();

    unsigned long ElapsedUs = NowUs - StartTimeUs;
    StartTimeUs = NowUs;
    const float RevsPerSec = 1000000.0 / (ElapsedUs / float(Revs));
    const float SpeedKmH = RevsPerSec * KmHPerRevsSec;
    send(MyMessage(CHILD_ID_WINDSPEED, V_WIND).set(SpeedKmH, 1));

    if (0 == Revs)
    {
      // No revolutions detected. Go back to sleep.
      break;
    }
  }
  detachInterrupt( digitalPinToInterrupt(PIN_REED) );
}
