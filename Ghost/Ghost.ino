/*
    Ghost - MySensors dollhouse ghost servo motion.

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

#define MY_NODE_GHOST
#define SKETCH_NAME        "Ghost"
#define SKETCH_VERSION     "1.0"
#define PIN_LED            (4)

#define MY_DEBUG  

#include "DemoSensorConfig.h"
#include <Servo.h>

#define CHILD_ID_GHOST      (0)

#define NUM_SERVOS (1)
const uint8_t pins[NUM_SERVOS] = { 5 };         // List of IO pins to control each servo

const float pos_min = 0.0;                      // [degrees]
const float pos_max = 180.0;                    // [degrees]
const float vel_max = 1.0 / (0.19 / 60.0);      // 0.19 sec/60 degr (=315), [degrees/sec], for Modelcraft RS-2 under no load

const float ghost_in_pos   = 25.0;
const float ghost_in_vel   = 0.5*vel_max;
const float ghost_out_pos  = 140.0;
const float ghost_out_vel  = 0.5*vel_max;
const float ghost_wiggle_min = 10.0;
const float ghost_wiggle_max = 15.0;
const float ghost_wiggle_vel = 0.2*vel_max;

const unsigned long ghost_wiggle_time_ms = 4000;

const float pos_start = ghost_in_pos; //(pos_max - pos_min) / 2.0; // [degrees]
const float max_move_time = 2 * (pos_max - pos_min) / vel_max; // [sec], 2x the maximum stroke travel time under no load
const unsigned long settle_time_ms = 100;       // [ms]

inline float sign(const float value)
{ 
 return float((value>0)-(value<0)); 
}

enum state {
  stopped,
  moving,
  settling
};

static struct {
  Servo         m_servo;
  uint8_t       m_pin;
  state         m_state;
  float         m_pos;
  float         m_start;
  float         m_distance;
  unsigned long m_starttime_ms;
  unsigned long m_endtime_ms;
} servos[NUM_SERVOS];

void menu()
{
  Serial.println(F("\n-- Menu\n"));
  Serial.println(F("0..9 - Rotate min..max position"));
  Serial.println(F("a..z - Set active velocity, in steps of 50 degr/sec "));
  Serial.println(F("A..Z - Change active servo"));
  Serial.println(F("!    - Ghost move!"));
  Serial.println(F("?    - This help menu"));
}

void move_servo(const size_t i, const float target, const float velocity)
{
  servos[i].m_start        = servos[i].m_pos;
  servos[i].m_distance     = target - servos[i].m_start;  // pos or neg, in [degrees]
  servos[i].m_starttime_ms = millis();
  servos[i].m_endtime_ms   = servos[i].m_starttime_ms + 1000.0*(fabs(servos[i].m_distance)/velocity);

  // Close the loop
  servos[i].m_servo.attach( servos[i].m_pin );
  servos[i].m_state = moving;
  
  Serial.print(F("Distance [deg] = ")); Serial.println(servos[i].m_distance);
  Serial.print(F("Travel time [ms] = ")); Serial.println(servos[i].m_endtime_ms - servos[i].m_starttime_ms);
}

bool is_servo_stopped(const size_t i)
{
  return servos[i].m_state == stopped;
}

void update_servo(const size_t i)
{
  const unsigned long now_ms = millis();
  switch( servos[i].m_state )
  {
    case moving:
      {
        const float move_time_ms  = now_ms - servos[i].m_starttime_ms;
        const float total_time_ms = servos[i].m_endtime_ms - servos[i].m_starttime_ms;
        const float ratio = constrain( move_time_ms/total_time_ms, 0.0, 1.0 );
        servos[i].m_pos = servos[i].m_start + ratio * servos[i].m_distance;
        servos[i].m_servo.write( servos[i].m_pos );
        if (ratio >= 1.0)
        {
          servos[i].m_endtime_ms = now_ms + settle_time_ms;
          servos[i].m_state = settling;
        }
      }
      break;
    case settling:
      if (now_ms >= servos[i].m_endtime_ms)
      {
        servos[i].m_servo.detach();
        servos[i].m_state = stopped;
        Serial.println(F("Stopped"));
      }
      break;
  }
}

void presentation()
{
  Serial.println(F("-- Init MySensors"));
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
  Serial.print(F("NodeID: "));
  Serial.println(getNodeId());

  present(CHILD_ID_GHOST,  S_BINARY, "Trigger ghost");
}

void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));
  Serial.println(F("-- Move to start"));
  for (size_t i = 0; i < NUM_SERVOS; ++i)
  {
    servos[i].m_pin   = pins[i];
    servos[i].m_state = stopped;
    servos[i].m_pos   = pos_start;
    servos[i].m_servo.attach( servos[i].m_pin );
    // Move to start position. Uncontrolled, as we don't know the starting position.
    servos[i].m_servo.write( servos[i].m_pos );
  }
  delay(1000.0 * max_move_time);
  for (size_t i = 0; i < NUM_SERVOS; ++i)
  {
    servos[i].m_servo.detach();
  }
  Serial.println(F("-- Done"));
  menu();
}

void receive(const MyMessage &message)
{
  if (message.type == V_STATUS)
  {
    if (message.sensor == CHILD_ID_GHOST)
    {
      // Don't care about status sent; show the ghost!
      Serial.println(F("Peek-a-boo!"));
      move_ghost(0);
    }
  }
}

void loop()
{
  if (Serial.available())
  {
    static size_t active_servo = 0;
    static float active_velocity = vel_max;
    
    char c = Serial.read();
    if (c >= '0' && c <= '9')
    {
      float target = pos_min + (pos_max - pos_min) * float(c - '0') / 9.0;
      Serial.print(F("Move to ")); Serial.println(target);
      move_servo(active_servo, target, active_velocity);
    }
    else if (c >= 'A' && c <= 'Z')
    {
      size_t active = c-'A';
      if (active >= NUM_SERVOS)
      {
        Serial.print(F("Error: number of supported servos is ")); Serial.println(NUM_SERVOS);
      }
      else
      {
        active_servo = active;
      }
      Serial.print(F("Activate servo ")); Serial.println(active_servo);
    }
    else if (c >= 'a' && c <= 'z')
    {
      float velocity = float(c-'a')*50.0;
      active_velocity = velocity;
      Serial.print(F("Activate velocity ")); Serial.println(active_velocity);
    }
    switch (c)
    {
      case '!':
        move_ghost(active_servo);
        break;
      case '?':
        menu();
        break;
    }
  }

  for (size_t i = 0; i < NUM_SERVOS; ++i)
  {
    update_servo(i);
    update_ghost(i);
  }
}

enum ghost_state {
  idle,
  move_out,
  move_out_wait,
  wiggle,
  move_in,
  move_in_wait,
};

static struct {
  ghost_state m_state;
  unsigned long m_wiggle_endtime_ms;
  float         m_wiggle_distance;
} ghosts[NUM_SERVOS];


void move_ghost(const size_t i)
{
  ghosts[i].m_state = move_out;
}

void update_ghost(const size_t i)
{
  switch( ghosts[i].m_state )
  {
    case move_out:
      move_servo(i, ghost_out_pos, ghost_out_vel);
      ghosts[i].m_state = move_out_wait;
      break;
    case move_out_wait:
      if (is_servo_stopped(i))
      {
        ghosts[i].m_wiggle_endtime_ms = millis() + ghost_wiggle_time_ms;
        ghosts[i].m_state = wiggle;
      }
      break;
    case wiggle:
      if (is_servo_stopped(i))
      {
        if (millis() >= ghosts[i].m_wiggle_endtime_ms)
        {
          ghosts[i].m_state = move_in;
        }
        else
        {
          float prev_distance = ghosts[i].m_wiggle_distance;
          ghosts[i].m_wiggle_distance = random(ghost_wiggle_min,ghost_wiggle_max);
          if (sign(prev_distance) == sign(ghosts[i].m_wiggle_distance))
          {
            ghosts[i].m_wiggle_distance = -ghosts[i].m_wiggle_distance;
          }
          move_servo(i, servos[i].m_pos + ghosts[i].m_wiggle_distance, vel_max);
        }
      }
      break;
    case move_in:
      move_servo(i, ghost_in_pos, ghost_in_vel);
      ghosts[i].m_state = move_in_wait;
      break;
    case move_in_wait:
      if (is_servo_stopped(i))
      {
        ghosts[i].m_state = idle;
      }
      break;
  }
}

