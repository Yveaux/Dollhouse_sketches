/*
    Fireplace - MySensors simulated fireplace for dollhouse.

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

#define MY_NODE_FIREPLACE
#define SKETCH_NAME        "Fireplace"
#define SKETCH_VERSION     "1.0"
#define PIN_LED            (4)

#define MY_DEBUG

#include "DemoSensorConfig.h"
#include <FastLED.h>

#define CHILD_ID_FIRE      (0)

#define PIN_WS2812B_DATA   (6)
#define NUM_LEDS           (2)

#define BRIGHTNESS         (155)
#define FRAMES_PER_SECOND  (20)

CRGB leds[NUM_LEDS];

const uint8_t state_off      = 0;
const uint8_t state_turn_on  = 1;
const uint8_t state_on       = state_turn_on + 1;
const uint8_t state_turn_off = state_on + 1;
const uint8_t state_max      = state_turn_off + FRAMES_PER_SECOND; // turning off will take state_max-state_turn_off number of frames

static uint8_t fire_state = state_off;

void switchFire( const bool switchOn )
{
  if (switchOn)
  {
    if ((fire_state == state_off) || ((fire_state >= state_turn_off) && (fire_state < state_max)) )
    {
      // Switch on and currently off or switching off -> switch on
      fire_state = state_turn_on;
    }
  } else {
    if ((fire_state == state_on) || ((fire_state >= state_turn_on) && (fire_state < state_on)) )
    {
      // Switch off and currently on or switching on -> switch off
      fire_state = state_turn_off;
    }
  }
}

void updateFire()
{
  static unsigned int t_prev = millis();
  unsigned int t_now = millis();
  if ((t_now - t_prev) > (1000 / FRAMES_PER_SECOND))
  {
    t_prev = t_now;
    if (fire_state == state_off)
    {
      FastLED.setBrightness( 0 );
      for ( int j = 0; j < NUM_LEDS; j++)
      {
        leds[j] = 0;
      }
    }
    else if ((fire_state >= state_turn_on) && (fire_state < state_on))
    {
      FastLED.setBrightness( BRIGHTNESS );
      fire_state++;
    }
    else if (fire_state == state_on)
    {
      FastLED.setBrightness( BRIGHTNESS );
    }
    else
    {
      uint8_t bright = map(fire_state - state_turn_off, state_turn_off, state_max, BRIGHTNESS, 0);
      FastLED.setBrightness( bright );
      fire_state++;
      if (fire_state == state_max)
      {
        fire_state = state_off;
      }
    }

    Fire2012();
    FastLED.show();
  }
}

void menu()
{
  Serial.println(F("\n-- Menu\n"));
  Serial.println(F("0 - Fire on"));
  Serial.println(F("1 - Fire off"));
  Serial.println(F("? - This help menu"));
}

void presentation()
{
  Serial.println(F("-- Init MySensors"));
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
  Serial.print(F("NodeID: "));
  Serial.println(getNodeId());

  present(CHILD_ID_FIRE, S_BINARY, "Fire");
}


void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));

  FastLED.addLeds<NEOPIXEL, PIN_WS2812B_DATA>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  switchFire(false);

  menu();
}

void receive(const MyMessage &message)
{
  if (message.type == V_STATUS)
  {
    if (message.sensor == CHILD_ID_FIRE)
    {
      // On/off 1/0
      const bool state = message.getInt();
      switchFire( state );
      Serial.print(F("Fire=")); Serial.println(state);
    }
  }
}

void loop()
{
  if (Serial.available())
  {
    char c = Serial.read();
    switch (c)
    {
      case '0':
        Serial.println(F("Switch fire off "));
        switchFire(false);
        break;
      case '1':
        Serial.println(F("Switch fire on"));
        switchFire(true);
        break;
      case '?':
        menu();
        break;
    }
  }

  // Add entropy to random number generator; we use a lot of it.
  // random16_add_entropy( random());

  updateFire();
}


// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
////
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation,
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120


void Fire2012()
{
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
#if NUM_LEDS >= 3
  for ( int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }
#endif

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < SPARKING ) {
    int y = random8(min(7, NUM_LEDS));
    //        heat[y] = qadd8( heat[y], random8(160,255) );
    //        heat[y] = qadd8( heat[y], random8(20,60) );
    heat[y] = qadd8( heat[y], random8(20, 40) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = 0; j < NUM_LEDS; j++) {
    CRGB color = HeatColor( heat[j] );
    leds[j] = color;
  }
}

