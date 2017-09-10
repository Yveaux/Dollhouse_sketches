/*
    Lighting - MySensors dollhouse lighting. Serial gateway with local lighting controller.

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

//#define MY_NODE_LIGHTING
#define SKETCH_NAME        "Lighting"
#define SKETCH_VERSION     "1.0"
//#define PIN_LED            (4)
//#define MY_RF24_PA_LEVEL   (RF24_PA_HIGH)

#define MY_BAUD_RATE 115200
#define MY_GATEWAY_SERIAL

#define MY_DEBUG  

//#include "DemoSensorConfig.h"
#include "Adafruit_NeoPixel.h"
#include "WS2812FX_Multi.h"
#include <MySensors.h>

#define NUM_LEDS 127
//#define PIN_WS2812B_DATA  (6)
#define PIN_WS2812B_DATA  (D4)

class NeoPixelFX : public WS2812FX
{
  public:
    NeoPixelFX(Adafruit_NeoPixel& pixels, uint16_t first, uint16_t n);

  protected:
    void _begin();
    void _show();
    void _setBrightness(uint8_t b);
    void _clear();
    void _setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
    void _setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    void _setPixelColor(uint16_t n, uint32_t c);
    uint32_t _getPixelColor(uint16_t n) const;
    Adafruit_NeoPixel& m_pixels;
    const uint16_t m_first;
};

NeoPixelFX::NeoPixelFX(Adafruit_NeoPixel& pixels, uint16_t first, uint16_t n)
  :   WS2812FX( n )
    , m_pixels(pixels)
    , m_first(first)
{
}

void NeoPixelFX::_begin()
{
}

void NeoPixelFX::_show()
{
}

void NeoPixelFX::_setBrightness(uint8_t b)
{
  m_pixels.setBrightness(b);
}

void NeoPixelFX::_clear()
{
  uint16_t idx = m_first;
  for (uint16_t i = 0; i < _led_count; ++i)
  {
    m_pixels.setPixelColor(idx++, 0u, 0u, 0u); // Black
  }
}

void NeoPixelFX::_setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
  m_pixels.setPixelColor(m_first+n, r, g, b);
}

void NeoPixelFX::_setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
  m_pixels.setPixelColor(m_first+n, r, g, b, w);
}

void NeoPixelFX::_setPixelColor(uint16_t n, uint32_t c)
{
  m_pixels.setPixelColor(m_first+n, c);
}

uint32_t NeoPixelFX::_getPixelColor(uint16_t n) const
{
  return (m_pixels.getPixelColor(m_first+n));
}


enum state {
  off    = 0,
  on     = 1,
  toggle = 2
};

enum room {
  room_first = 0,
  kitchen = 0,
  diner   = 1,
  living  = 2,
  toilet  = 3,
  sleep1  = 4,
  sleep2  = 5,
  attic1  = 6,
  attic2  = 7,
  room_last = attic2,
  all     = 8,
  num_rooms = all+1
};

//static CRGB leds[NUM_LEDS];
static Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, PIN_WS2812B_DATA, NEO_GRB + NEO_KHZ800);

static struct {
//  uint8_t    bright;
//  CRGB       color;
  NeoPixelFX  fx;
} room_state[num_rooms] = {
#if 1
  { NeoPixelFX(pixels,   0,  10- 0+1) }, 
  { NeoPixelFX(pixels,  11,  26-11+1) }, 
  { NeoPixelFX(pixels,  27,  51-27+1) }, 
  { NeoPixelFX(pixels,  52,  60-52+1) }, 
  { NeoPixelFX(pixels,  61,  76-61+1) }, 
  { NeoPixelFX(pixels,  77,  91-77+1) }, 
  { NeoPixelFX(pixels,  92, 108-92+1) }, 
  { NeoPixelFX(pixels, 109, NUM_LEDS-1-109+1) }, 
  { NeoPixelFX(pixels,   0, NUM_LEDS) },          // all rooms
#else
  { NeoPixelFX(pixels,   0,  3) }, 
  { NeoPixelFX(pixels,   3,  3) }, 
  { NeoPixelFX(pixels,   6,  3) }, 
  { NeoPixelFX(pixels,   9,  3) }, 
  { NeoPixelFX(pixels,  12,  3) }, 
  { NeoPixelFX(pixels,  15,  3) }, 
  { NeoPixelFX(pixels,  18,  3) }, 
  { NeoPixelFX(pixels,  21,  3) }, 
  { NeoPixelFX(pixels,   0, 24) },          // all rooms
#endif
};

void presentation()
{
  Serial.println(F("-- Init MySensors"));
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
  Serial.print(F("NodeID: "));
  Serial.println(getNodeId());

  present(kitchen, S_RGB_LIGHT, "Kitchen");
  present(diner,   S_RGB_LIGHT, "Diner");
  present(living,  S_RGB_LIGHT, "Living");
  present(toilet,  S_RGB_LIGHT, "Toilet");
  present(sleep1,  S_RGB_LIGHT, "Sleep1");
  present(sleep2,  S_RGB_LIGHT, "Sleep2");
  present(attic1,  S_RGB_LIGHT, "Attic1");
  present(attic2,  S_RGB_LIGHT, "Attic2");
  present(all,     S_RGB_LIGHT, "All");
}

void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));

  pixels.begin();

  // Initialize LED states: all off, set to white
  for (size_t i = 0; i < sizeof(room_state)/sizeof(room_state[0]); ++i)
  {
    NeoPixelFX* fx = &room_state[i].fx;
    fx->init();
    fx->setBrightness(255);
    fx->setSpeed(150);
    fx->setColor(0xFFFFFFul);
    fx->setMode(FX_MODE_STATIC);
    fx->stop();
  }
  pixels.show();
}


void receive(const MyMessage &message)
{
  Serial.println(F("RX"));
  auto r = room(message.sensor);
  if (r >= num_rooms)
  {
    return;    
  }

  bool     setbrightness = false;
  uint8_t  brightness = 0u;
  bool     setcolor = false;
  uint32_t color = 0ul;
  bool     setstate = false;
  uint8_t  state = 0;;
  bool     setspeed = false;
  uint8_t  speed = 0;;
  uint8_t  mode = FX_MODE_STATIC;    // Will always be set

  if (message.type == V_RGB)
  {
    // RGB as text, in hex RRGGBB
    color = strtol(message.getString(), NULL, 16);
    Serial.print(F("V_RGB=")); Serial.println(color, HEX);
    setcolor = true;
  }
  else if (message.type == V_DIMMER)
  {
    // Value 0..99
    brightness = message.getLong();
    Serial.print(F("V_DIMMER=")); Serial.println(brightness);
    setbrightness = true;
  }
  else if (message.type == V_STATUS)
  {
    // On/off 1/0
    state = message.getInt();
    Serial.print(F("V_STATUS=")); Serial.println(state);
    setstate = true;
  }
  else if (message.type == V_VAR1)
  {
    // "mode,color,brightness,speed"
    // mode       = 0..46  (See FX_MODE_xxx)
    // color      = rrggbb (hex)
    // brightness = [0..100] [%] 
    // speed      = [0..100] [%] 
    char* msg = const_cast<char*>(message.getString());
    Serial.print(F("V_VAR1=")); Serial.println(msg);
    char *str, *p;
    int i = 0;
    for (str = strtok_r(msg, ",", &p); // split using comma
            str && i <= 4; // loop while str is not null an max n times
            str = strtok_r(NULL, ",", &p) )// get subsequent tokens
    {
      switch (i)
      {
        case 0: // Mode
          mode = atoi(str);
          break;
        case 1: // Color
          color = strtol(str, NULL, 16);
          break;
        case 2: // Brightness
          brightness = atoi(str);
          break;
        case 3: // Speed
          speed = atoi(str);
          // Input is known-valid now
          setcolor = true;
          setbrightness = true;
          state = true;
          setstate = true;
          setspeed = true;
          break;
      }
      i++;
    }
  }

  if (setbrightness)
  {
    if (brightness > 100) brightness = 100;
    brightness = map( brightness, 0, 100, 0, 255 );
  }
  if (setspeed)
  {
    if (speed > 100) speed = 100;
    speed = map( speed, 0, 100, 0, 255 );
  }

  // Now update the requested rooms
  auto rfirst = r;
  auto rlast  = r;
  bool all_active = false;
  if ( all == r )
  {
//    Serial.println(F("All"));
    // All is just a synonym for all rooms.
    // The static, non-moving patterns will be distributed over all rooms, the
    // moving patterns will use the 'all' alias fx.
    bool isstatic = false;
    isstatic |= FX_MODE_STATIC == mode;
    isstatic |= FX_MODE_BLINK  == mode;
    isstatic |= FX_MODE_BREATH == mode;
    // TODO: More?

    if (isstatic)
    {
      // Static pattern, individual rooms enabled, 'all' disabled
//      Serial.println(F("Static"));
      rfirst = room_first;
      rlast  = room_last;
    }
    else
    {
      // Dynamic pattern, individual rooms disabled, 'all' enabled
//      Serial.println(F("Dynamic"));
      all_active = true;
    }
  }
  Serial.print(F("Rooms [")); Serial.print(rfirst); Serial.print(F("..")); Serial.print(rlast); Serial.println(']'); 
  if (setbrightness) { Serial.print(F("Bright\t")); Serial.println(brightness); }
  if (setcolor)      { Serial.print(F("Color\t0x")); Serial.println(color, HEX); }
                     { Serial.print(F("Mode\t")); Serial.println(mode); }
  if (setstate)      { Serial.print(F("State\t")); Serial.println(state); }
  if (setspeed)      { Serial.print(F("Speed\t")); Serial.println(speed); }

  for (size_t i = 0; i < sizeof(room_state)/sizeof(room_state[0]); ++i)
  {
    NeoPixelFX* fx = &room_state[i].fx;
    if ((i >= rfirst) and (i <= rlast))
    {
      if (setbrightness) fx->setBrightness(brightness);
      if (setcolor)      fx->setColor(color);
      if (setspeed)      fx->setSpeed(speed);
      fx->setMode(mode);
      if (setstate)
      {
        if (state)
        {
          fx->start();
          //Serial.print(F("Start room ")); Serial.println(i); 
        }
        else
        {
          fx->stop(true);
          //Serial.print(F("Stop room ")); Serial.println(i); 
        }
      }
    }
    else if (i < all)
    {
      if (all_active)
      {
        fx->stop(true);
        // Serial.print(F("Stop room ")); Serial.println(i);
      }
    }
    else if (i == all)
    {
      if (!all_active)
      {
        fx->stop(true);
        // Serial.print(F("Stop room ")); Serial.println(i);
      }
    }
    Serial.print(F("Room:")); Serial.print(i);
    Serial.print(F("\t")); Serial.print(fx->isRunning() ? "Run" : "Stp"); 
    Serial.print(F("\tMod:")); Serial.print(fx->getMode()); 
    Serial.print(F("\tSpd:")); Serial.print(fx->getSpeed()); 
    Serial.print(F("\tBri:")); Serial.print(fx->getBrightness()); 
    Serial.print(F("\tCol:0x")); Serial.println(fx->getColor(), HEX); 
  }
}

void loop()
{
  for (size_t i = 0; i < sizeof(room_state)/sizeof(room_state[0]); ++i)
  {
    room_state[i].fx.service();
  }
  pixels.show();
}

