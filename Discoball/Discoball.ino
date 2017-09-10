/*
    Discoball - MySensors controller for dollhouse discoball and LED spot.

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

#define MY_NODE_DISCOBALL
#define SKETCH_NAME        "Discoball"
#define SKETCH_VERSION     "1.0"
#define PIN_LED            (4)

#define MY_DEBUG  

#include "DemoSensorConfig.h"
#include <Servo.h>

#define CHILD_ID_DISCOBALL (0)
#define CHILD_ID_LIGHT     (1)

#define PIN_SERVO          (6)
#define PIN_LIGHT          (A4)
static Servo servo;

const int max_speed = 10;
const int stopped   = 89;   // Roughly 90. Tweak depending on servo connected.

static int prev_velocity = 3;

void menu()
{
  Serial.println(F("\n-- Menu\n"));
  Serial.println(F("0..9 - Rotate min..max speed"));
  Serial.println(F("+,-  - Change rotation direction"));
  Serial.println(F("?    - This help menu"));
}

void setSpeed( const int velocity )
{
  if (velocity == 0)
  {
    servo.detach();
  }
  else
  {
    servo.attach( PIN_SERVO );
    servo.write( stopped + velocity );
  }
}

void presentation()
{
  Serial.println(F("-- Init MySensors"));
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
  Serial.print(F("NodeID: "));
  Serial.println(getNodeId());

  present(CHILD_ID_DISCOBALL,  S_DIMMER, "Discoball speed");
  present(CHILD_ID_LIGHT,      S_BINARY, "Light");
}

void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));

  digitalWrite(PIN_LIGHT, LOW);
  pinMode(PIN_LIGHT, OUTPUT);

  setSpeed(0);

  Serial.println(F("-- Done"));
  menu();
}

void receive(const MyMessage &message)
{
  if (message.type == V_PERCENTAGE)
  {
    if (message.sensor == CHILD_ID_DISCOBALL)
    {
      int32_t vel = message.getLong();
      vel = constrain( vel, 0, 100 );
      vel = map( vel, 0, 100, 0, max_speed );
      setSpeed( vel );
      Serial.print(F("Velocity=")); Serial.println(vel);
      prev_velocity = vel;
    }    
  }
  else if (message.type == V_STATUS)
  {
    if (message.sensor == CHILD_ID_LIGHT)
    {
      // On/off 1/0
      const bool state = message.getInt();
      digitalWrite(PIN_LIGHT, state ? HIGH : LOW);
      Serial.print(F("Light=")); Serial.println(state);
    }
    else if (message.sensor == CHILD_ID_DISCOBALL)
    {
      // On/off 1/0
      const bool state = message.getInt();
      int32_t vel = state ? prev_velocity : 0;
      setSpeed( vel );
      Serial.print(F("Velocity=")); Serial.println(vel);
    }
  }
}

void loop()
{
  if (Serial.available())
  {
    static int velocity = 0;
    static bool dir = true;
    
    char c = Serial.read();
    if (c >= '0' && c <= '9')
    {
      velocity = (int(c-'0') * max_speed) / 10;
      Serial.print(F("Velocity=")); Serial.println(velocity);
    }
    switch (c)
    {
      case '+':
      case '-':
        dir = c == '+';
        Serial.print(F("Direction=")); Serial.println(dir ? F("forward") : F("reverse"));
        break;
      case '?':
        menu();
        break;
    }
    setSpeed( dir ? +velocity : -velocity );
  }
}

