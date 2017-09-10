/*
    MySensorsTV - MySensors dollhouse TV display.

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

/*====================================================================================

  This sketch demonstrates loading images which have been stored as files in the
  built-in FLASH memory on a NodeMCU 1.0 (ESP8266 based, ESP-12E Module) rendering the
  images onto a ILI9341 SPI 320 x 240 pixel TFT screen.

  The images are stored in the SPI FLASH Filing System (SPIFFS), which effectively
  functions like a tiny "hard drive". This filing system is built into the ESP8266
  Core that can be loaded from the IDE "Boards manager" menu option. This is at
  version 2.3.0 at the time of sketch creation.

  The size of the SPIFFS partition can be set in the IDE as 1Mbyte or 3Mbytes. Either
  will work with this sketch. Typically most sketches easily fit within 1 Mbyte so a
  3 Mbyte SPIFS partition can be used, in which case it can contain ~18 full screen
  320 x 240 raw images (150 Kbytes each) or 100's of Jpeg full screem images.

  Place the images inside the sketch folder, in a folder called "Data".  Then upload
  all the files in the folder using the Arduino IDE "ESP8266 Sketch Data Upload" option
  in the "Tools" menu:
  http://www.esp8266.com/viewtopic.php?f=32&t=10081
  https://github.com/esp8266/arduino-esp8266fs-plugin/releases

  This takes some time, but the SPIFFS content is not altered when a new sketch is
  uploaded, so there is no need to upload the same files again!
  Note: If open, you must close the "Serial Monitor" window to upload data to SPIFFS!

  Display       NodeMCU   Wemos D1 mini
  SDO/MISO      D6        D6        <<<<<< This is not used by this sketch
  LED           3.3V      3.3V
  SCK           D5        D5
  SDI/MOSI      D7        D7
  DC/RS (or AO) D3        D3
  RESET         3.3V      3.3V
  CS            D8        D8
  GND           GND       GND
  VCC           3.3V      3.3V

  nRF24L01+               Wemos D1 mini
  VCC                     3.3V
  GND                     GND
  MISO                    D6
  MOSI                    D7
  SCK                     D5
  CE                      D1
  CSN                     D4
*/

#define MY_NODE_TV
#define SKETCH_NAME       "MySensorsTV"
#define SKETCH_VERSION    "1.0"

#define MY_RF24_CE_PIN    (D1)
#define MY_RF24_CS_PIN    (D4)

#define MY_DEBUG  

#include "DemoSensorConfig.h"

// Call up the SPIFFS FLASH filing system this is part of the ESP Core
#define FS_NO_GLOBALS
#include <FS.h>
#include <JPEGDecoder.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "general_config.h"
#include <ESP8266WiFi.h>

#define CHILD_ID_COMMAND   (0)
#define CHILD_ID_TEXT      (1)

// The TFT control pins are set in the User_Setup.h file <<<<<<<<<<<<<<<<< NOTE!
// that can be found in the "src" folder of the library
// For MySensors TV:
/*
  #define ILI9341_DRIVER
  #define TFT_CS   D8
  #define TFT_DC   D3
  #define TFT_RST  -1
*/

static TFT_eSPI tft = TFT_eSPI();

static String prevCommand;
static String command;

enum modes {
  idle,
  startup,
  c64,
  pong,
};
static modes mode = startup;


void presentation()
{
  Serial.println(F("-- Init MySensors"));
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
  Serial.print(F("NodeID: "));
  Serial.println(getNodeId());

  present(CHILD_ID_COMMAND,  S_INFO, "Command to show");
  present(CHILD_ID_TEXT,     S_INFO, "Text to show");
}

void setup()
{
  delay(10);
  Serial.println(F("-- " SKETCH_NAME " " SKETCH_VERSION));

//  Serial.begin(115200); // Used for messages and the C array generator

  WiFi.mode(WIFI_OFF);
//  WiFi.begin("pietjepuk", "blabla");

  Serial.print(F("-- Init TFT"));
  tft.begin();
  tft.setRotation(3);  // 0 & 2 Portrait. 1 & 3 landscape
  tft.fillScreen(TFT_BLACK);

  Serial.print(F("-- Init SPIFFS"));
  if (!SPIFFS.begin()) {
    Serial.println(F("SPIFFS initialisation failed!"));
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  listFiles(); // Lists the files so you can see what is in the SPIFFS
}

void receive(const MyMessage &message)
{
  if (message.type == V_TEXT)
  {
    if (message.sensor == CHILD_ID_COMMAND)
    {
      String newCommand = String( message.getString() );
      if (prevCommand != newCommand)
      {
        Serial.print(F("New command: "));
        Serial.println(newCommand);
        command = newCommand;
      }
      else
      {
        Serial.println(F("Ignored command; it is already active"));
      }
    }
    else if (message.sensor == CHILD_ID_TEXT)
    {
      const char* text = message.getString();
      Serial.print(F("New text: "));
      Serial.println(text);

      tft.setFreeFont(&FreeSerifBold24pt7b); 
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE);
      tft.setTextDatum(MC_DATUM);
      tft.drawString(text, tft.width() / 2, tft.height() / 2); 
      mode = idle;
    }
  }
}

void loop()
{
  if (command.length() > 0)
  {
    // New command/filename received
    if (command.equalsIgnoreCase(String("c64")))
    {
      mode = c64;
    }
    else if (command.equalsIgnoreCase(String("pong")))
    {
      pong_setup();
      mode = pong;
    }
    else
    {
      // Assume its a JPEG filename in SPIFFS
      String filename = "/";
      filename += command;
      drawJpeg(filename.c_str(), 0, 0);
      mode = idle;
    }
    prevCommand = command;
    command = String();
  }

  switch (mode)
  {
    default:
      break;
    case startup:
      command = "amiga.jpg";
      mode = idle;
      break;
    case c64:
      {
        static unsigned tprev = millis();
        if (millis() - tprev > 150) // // 500 is required; compensate for decoding time
        {
          static bool on = true;
          drawJpeg( on ? "/c64on.jpg" : "/c64off.jpg", 0, 0);
          on = !on;
          tprev = millis();
        }
      }
      break;
    case pong:
      pong_loop();
      break;
  }
}

