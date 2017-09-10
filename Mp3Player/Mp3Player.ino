/*
    MP3Player - MySensors dollhouse MP3 player, using JQ6500.

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

#define MY_NODE_MP3PLAYER
#define SKETCH_NAME        "Mp3Player"
#define SKETCH_VERSION     "1.0"
#define PIN_LED            (4)

#define MY_DEBUG  

#include "DemoSensorConfig.h"
#include <SoftwareSerial.h>
#include <JQ6500_Serial.h>

#define PIN_JQ6500_TX      (7)
#define PIN_JQ6500_RX      (8)

#define CHILD_ID_PLAY      (0)
#define CHILD_ID_VOLUME    (1)

static JQ6500_Serial mp3(PIN_JQ6500_TX, PIN_JQ6500_RX);

void presentation()
{
  Serial.println(F("-- Init MySensors"));
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
  Serial.print(F("NodeID: "));
  Serial.println(getNodeId());

  present(CHILD_ID_PLAY,   S_CUSTOM, "Index to play");
  present(CHILD_ID_VOLUME, S_SOUND,  "Volume");
}

void setup()
{
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));
  mp3.begin(9600);
  mp3.reset();
  mp3.setVolume(20);
  Serial.print(F("Files\t"));  Serial.println(mp3.countFiles(MP3_SRC_BUILTIN));
}

static uint8_t g_idxPlaying = 0;  // 0 indicates not playing

void receive(const MyMessage &message)
{
  if (message.type == V_CUSTOM)
  {
    if (message.sensor == CHILD_ID_PLAY)
    {
      const uint8_t idx = message.getByte();
      Serial.print(F("Play file with index ")); Serial.println(idx);
      if (idx != g_idxPlaying)
      {
        mp3.playFileByIndexNumber(idx);
        Serial.print(F("Duration:")); Serial.print(mp3.currentFileLengthInSeconds()); Serial.println('s');
        g_idxPlaying = idx;
      }
      else
      {
        Serial.println(F("Ignored: already playing"));
      }
    }
  }
  else if (message.type == V_LEVEL)
  {
    if (message.sensor == CHILD_ID_VOLUME)
    {
      uint8_t volume = constrain( message.getByte(), 0u, 30u);
      Serial.print(F("Set volume ")); Serial.println(volume);
      mp3.setVolume(volume);
    }
  }
  else if (message.type == V_STOP)  // Meant for roller-shutter, with this sensor is non-standard anyway...
  {
    if (message.sensor == CHILD_ID_PLAY)
    {
      // JQ6500 library does not support stop.
      // Play stop mp3
      Serial.print(F("Stop"));
      mp3.playFileByIndexNumber(1);
      g_idxPlaying = 0;
    }
  }
}

void loop()
{
  if ((mp3.getStatus() == MP3_STATUS_STOPPED))
  {
    g_idxPlaying = 0;
  }
}

