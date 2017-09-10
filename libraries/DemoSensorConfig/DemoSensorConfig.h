/*
    DemoSensorConfig - MySensors dollhouse generic settings library.

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

#ifndef GENERAL_CONFIG_H_INCLUDED
#define GENERAL_CONFIG_H_INCLUDED

#ifndef MY_BAUD_RATE
#define MY_BAUD_RATE           (57600)
#endif

#define MY_RADIO_NRF24
#define MY_RF24_CHANNEL	       (110)                     // 110 = demo
#define MY_RF24_BASE_RADIO_ID  0x00,0xFC,0xE1,0xA8,0xA8  // 0x00,0xFC,0xE1,0xA8,0xA8 = demo
#define MY_RF24_DATARATE       (RF24_1MBPS)
#ifndef MY_RF24_PA_LEVEL
#define MY_RF24_PA_LEVEL       (RF24_PA_LOW)
#endif

#ifdef MY_NODE_TV
#define MY_NODE_ID             (100)
#endif
#ifdef MY_NODE_FRONTDOOR
#define MY_NODE_ID             (101)
#endif
#ifdef MY_NODE_LIGHTING
#define MY_NODE_ID             (102)
#endif
#ifdef MY_NODE_FIREPLACE
#define MY_NODE_ID             (103)
#endif
#ifdef MY_NODE_SOLAR
#define MY_NODE_ID             (104)
#endif
#ifdef MY_NODE_MP3PLAYER
#define MY_NODE_ID             (105)
#endif
#ifdef MY_NODE_GHOST
#define MY_NODE_ID             (106)
#endif
#ifdef MY_NODE_DISCOBALL
#define MY_NODE_ID             (107)
#endif
#ifdef MY_NODE_TOILET
#define MY_NODE_ID             (108)
#endif
#ifdef MY_NODE_TEMPHUMPRES
#define MY_NODE_ID             (109)
#endif
#ifdef MY_NODE_DOORALARM1
#define MY_NODE_ID             (110)
#endif
#ifdef MY_NODE_DOORALARM2
#define MY_NODE_ID             (111)
#endif
#ifdef MY_NODE_SOILMOISTURE
#define MY_NODE_ID             (112)
#endif
#ifdef MY_NODE_WINDSPEED
#define MY_NODE_ID             (113)
#endif
#ifdef MY_NODE_PIRMOTION
#define MY_NODE_ID             (114)
#endif
#ifdef MY_NODE_RGBGESTURE
#define MY_NODE_ID             (115)
#endif
#ifdef MY_NODE_BREADBOARDBUTTON
#define MY_NODE_ID             (116)
#endif
#ifdef MY_NODE_SMARTPLUG
#define MY_NODE_ID             (117)
#endif


#ifdef PIN_LED
#define MY_INDICATION_HANDLER
#endif

#include <MySensors.h>

//    #define MY_WITH_LEDS_BLINKING_INVERSE
//    #define MY_DEFAULT_ERR_LED_PIN      (A0)
//    #define MY_DEFAULT_TX_LED_PIN       (A1)
//    #define MY_DEFAULT_RX_LED_PIN       (A2)

#ifdef PIN_LED

#include "LedPattern_Mono.h"
#include <MsTimer2.h>

#define LED_CYCLE_MS           (10)

// Define Led ON/OFF symbolic names
#undef LED_ON
#define LED_ON  (255)
#undef LED_OFF
#define LED_OFF (0)

// Create the Led pattern handling class.
static LedPattern_Mono ledPattern(PIN_LED);

const uint8_t ledPatternJoin[] = {
  LedPattern::CMD_REPEAT, LedPattern::repeatForever,
  LedPattern::CMD_SET, LED_ON,
  LedPattern::CMD_WAIT, 10,
  LedPattern::CMD_SET, LED_OFF,
  LedPattern::CMD_WAIT, 50,
  LedPattern::CMD_ENDREPEAT
};

const uint8_t ledPatternJoined[] = {
  LedPattern::CMD_SET, LED_ON,
  LedPattern::CMD_WAIT, 50,
  LedPattern::CMD_SET, LED_OFF,
  LedPattern::CMD_FINISHED
};

const uint8_t ledPatternTxRx[] = {
  LedPattern::CMD_SET, LED_ON,
  LedPattern::CMD_WAIT, 1,
  LedPattern::CMD_SET, LED_OFF,
  LedPattern::CMD_FINISHED
};

uint8_t ledPatternErr[] = {
  LedPattern::CMD_REPEAT, LedPattern::repeatForever,
  LedPattern::CMD_SET, LED_ON,
  LedPattern::CMD_WAIT, 30,
  LedPattern::CMD_SET, LED_OFF,
  LedPattern::CMD_WAIT, 20,
  LedPattern::CMD_ENDREPEAT
};

const uint8_t ledPatternToSleep[] = {
//  LedPattern::CMD_SET, LED_ON, LedPattern::CMD_WAIT, 2, LedPattern::CMD_SET, LED_OFF, LedPattern::CMD_WAIT, 16,
  LedPattern::CMD_SET, LED_ON, LedPattern::CMD_WAIT, 2, LedPattern::CMD_SET, LED_OFF, LedPattern::CMD_WAIT, 20,
  LedPattern::CMD_SET, LED_ON, LedPattern::CMD_WAIT, 2, LedPattern::CMD_SET, LED_OFF, LedPattern::CMD_WAIT, 24,
  LedPattern::CMD_SET, LED_ON, LedPattern::CMD_WAIT, 2, LedPattern::CMD_SET, LED_OFF, LedPattern::CMD_WAIT, 28,
  LedPattern::CMD_SET, LED_ON, LedPattern::CMD_WAIT, 2, LedPattern::CMD_SET, LED_OFF, LedPattern::CMD_WAIT, 32,
  LedPattern::CMD_SET, LED_ON, LedPattern::CMD_WAIT, 2, LedPattern::CMD_SET, LED_OFF, LedPattern::CMD_WAIT, 36,
  LedPattern::CMD_FINISHED
};

static void updateLedPattern(void)
{
  // Update the pattern (setting leds).
  ledPattern.update();
}

void before()
{
  // Configure timer interrupt
  MsTimer2::set(LED_CYCLE_MS, updateLedPattern);
  MsTimer2::start();
  ledPattern.start(ledPatternJoin);
}

void indication( const indication_t ind )
{
  static bool joining = false;
  static bool error   = false;
  error &= !ledPattern.finished();    // If error indication was running, it's done when led pattern is finished.
  if (    (INDICATION_TX == ind) || (INDICATION_GW_TX == ind)
          || (INDICATION_RX == ind) || (INDICATION_GW_RX == ind) ) {
    if (!joining && !error)
    {
      ledPattern.start(ledPatternTxRx);
    }
  } else if (INDICATION_FIND_PARENT == ind) {
    ledPattern.start(ledPatternJoin);
    joining = true;
  } else if (INDICATION_GOT_PARENT == ind) {
    ledPattern.start(ledPatternJoined);
    joining = false;
  } else if (ind > INDICATION_ERR_START) {
    // Blink LED, depending on the error number
    ledPatternErr[1] = ind - INDICATION_ERR_START;
    ledPattern.start(ledPatternErr);
    Serial.print(F("ERROR ")); Serial.println(ind - INDICATION_ERR_START);
    error = true;
  }
}

void waitLedPatternFinished(const bool showsleep = true)
{
    while (!ledPattern.finished())
    {
        wait(LED_CYCLE_MS);
    }
    if (showsleep)
    {
      // Show going-to-sleep pattern
      ledPattern.start(ledPatternToSleep);
      while (!ledPattern.finished())
      {
          wait(LED_CYCLE_MS);
      }
    }
}
                
#endif

#endif /* GENERAL_CONFIG_H_INCLUDED */
