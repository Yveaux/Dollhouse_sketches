# MySensors Eindhoven Maker Faire 2017 Dollhouse sketches

<img src="https://raw.githubusercontent.com/Yveaux/Dollhouse_sketches/master/images/booth.jpg"> 

<img src="https://raw.githubusercontent.com/Yveaux/Dollhouse_sketches/master/images/dollhouse_mysensored.png"> 

Required libraries
========
| Library       | Location      | Remarks  | SHA * |
| ------------- |---------------|-------|------|
| DemoSensorConfig | ./libraries/DemoSensorConfig | Included in this repository | |
| MySensors | https://github.com/mysensors/MySensors |  2.2.x development | 29fd6d6345497bd551f871350b91260aefab83e2 |
| LedPattern | https://github.com/Yveaux/LedPattern || f3910e47f8f302560af68f6bee2497ad0ebe84d9 |
| WS2812FX_Multi | https://github.com/Yveaux/WS2812FX_Multi || 7565b4e578cd61890e6487ee9eeab64d60eb0022 |
| Arduino_Vcc | https://github.com/Yveaux/Arduino_Vcc || 29261f90869a16456b3015dd3e9580f7df5cf98d |
| Adafruit_INA219 | https://github.com/adafruit/Adafruit_INA219 || f2c2f84ec9e26cf293c33ec62ae577daf124523e |
| Adafruit_NeoPixel | https://github.com/adafruit/Adafruit_NeoPixel || 246dd0f906afcccc9556543ed1333c3ac3e45019 |
| FastLed | https://github.com/FastLED/FastLED || 45f1d34cdf98cf5fce290efd9be4305ea2e6e249 |
| BME280 | https://github.com/finitespace/BME280 || ac82c63bb69b51b34a3608aaac8fafd02e288514 |
| JPEGDecoder | https://github.com/Bodmer/JPEGDecoder || 4164a392bd7ebde7692e08c6e8020b6499c1b921 |
| JQ6500_Serial | https://github.com/sleemanj/JQ6500_Serial || 1dbf0c31fe71a7d32f50e75d4f043a39bc0cb843 |
| MsTimer2 | https://github.com/PaulStoffregen/MsTimer2 || 098cf052949bd60debbcbc9f4ccce90d83517208 |
| TFT_eSPI | https://github.com/Bodmer/TFT_eSPI | ** see below | 5274d35207cdcf50711cb6f031264db5377f021e |

(*) The SHA identifies the exact GIT version of the library used. When running into problems try this exact version of each library first.

(**)
Change user_setup.h to use with Wemos D1 and ILI9341 display:

#define TFT_CS D8

#define TFT_DC D3

#define TFT_RST -1
