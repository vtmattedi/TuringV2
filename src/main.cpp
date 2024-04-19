// Make sure to have these definition on top of your code or else Blynk won't be able to connect.
#include <../include/Creds/BlynkCred.h>
#include <../include/Version.h>
// #define PROTO // Toggle comment to switch between upload to Turing or Jupiter
#include <../lib/Components/Components.h>

#ifndef PROTO
#define DEVICE_NAME "Turing"
#endif
#ifdef PROTO
#define DEVICE_NAME "Jupiter"
#endif
#define PUMPTESTER

#define DEBUG

#include <WiFiManager.h>
#include <TuringHttp.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESPmDNS.h>
#include "OneWire.h"
#include "DallasTemperature.h"
#include <../lib/Time-master/TimeLib.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include "esp_task_wdt.h"
#include <../include/Creds/WifiCred.h>
#include "DHT.h"
#include <../lib/NewPing/src/NewPing.h>
#include <Nightmaretcp.h>
#include <NightMareNetwork.h>
#include <ArduinoJson.h>
#include <../lib/Components/ExternalAdc.h>
#pragma region New Board pins
// #include <../src/Components/HwInfo.h> // Moved the defines to a separeted File
#pragma endregion

bool useDirectAdc = false;
#pragma region FastLED
#include <FastLED.h>

void InitializeSensors(bool reset = false);

#pragma region HIVE MQTT

void HiveMQ_Callback(char *topic, byte *payload, unsigned int length);

void MQTT_Send(String topic, String message)
{
  HiveMQ.publish(topic.c_str(), message.c_str(), true);
}

#pragma endregion

// 2 WS2811 lights
// 1 WS2818b strip

// WS2811 Beacons     @26
// WS2812 LED Strip   @23
// 3xWS2812E onboard  @2

#define BEACONS_SIZE 2
#define WS2811_COLOR_ORDER BGR

// best white is TypicalPixelString next is uncorrected
// WS2818b Strip
#define LED_STRIP_SIZE 300
#define LED_STRIP_COLOR_ORDER GRB

#define LED_MAX_AMPS
#define LED_VOLTAGE
#define TOTALSIZE LED_STRIP_SIZE + BEACONS_SIZE + ONBOARD_RGB_SIZE

#define BEACONS_START_INDEX LED_STRIP_SIZE
#define ONBOARD_START_INDEX LED_STRIP_SIZE + BEACONS_SIZE

uint16_t LED_BaseColor_1 = 0x0; // Contains an RGB value for the strip
uint16_t LED_BaseColor_2 = 0x0; // Contains an RGB value for the stripw
int patternIndex = 0;

CRGB strip_leds[LED_STRIP_SIZE];
CRGB beacons_leds[BEACONS_SIZE];
CRGB onboard_leds[ONBOARD_RGB_SIZE];

enum LED_Animations
{
  Solid,
  Blink,
  BlinkTwoColor,
  BlinkFinite,
  IndividualRandom,
  FadeInOutSolid,
  FadeInOutRandom,
  RandomWithFade,
  Codes,
  DebugCodes,
  OnePixelLoop,
  WiFiStrength,
  WiFiStrengthInverse,
  WiFi_Blink
};

struct LED_Controller
{
private:
  unsigned long _lastMillis = 0;
  int _Index1 = 0;
  int _Index2 = 0;
  bool _backwards = false;
  bool _progbar_is_running = false;
  int _start_index = 0;
  int _end_index = TOTALSIZE;
  int _num_of_leds = TOTALSIZE;
  int _interval = 25;
  int _old_interval = 25;
  bool _helperFlag = false;
  int _helperInt = 0;

public:
  byte currenthue = 0;
  LED_Animations current_animation = LED_Animations::IndividualRandom;
  CRGB baseColor = CRGB::GreenYellow;
  CRGB baseColor2 = CRGB::Blue;
  LED_Animations old_animation = LED_Animations::Solid;
  CRGB old_baseColor = CRGB::Red;
  CRGB old_baseColor2 = CRGB::Blue;

  // Set which LEDS are being controlled in the LED array, also inverts [start] and [finish] if [finish] < [start]
  void setBoundries(int start, int finish)
  {
    if (finish < start)
    {
      int middleman = finish;
      finish = start;
      start = middleman;
    }
    if (start < 0)
      start = 0;
    if (finish >= TOTALSIZE)
      finish = TOTALSIZE - 1;

    _start_index = start;
    _end_index = finish;
    _num_of_leds = _end_index - _start_index + 1;
  }

  // Sets the interval for the animations in ms min = 10 ms;
  void setInterval(int newInterval)
  {
    if (newInterval < 10)
    {
      newInterval = 10;
    }
    _interval = newInterval;
  }

  // Sets the new mode.
  void setMode(CRGB led_array[], LED_Animations newAnimation, CRGB color1 = 0x0, CRGB color2 = 0x0, int interval = 25, int _arg0 = 0)
  {
    old_animation = current_animation;
    current_animation = newAnimation;

    old_baseColor = baseColor;
    old_baseColor2 = baseColor2;
    _old_interval = interval;
    baseColor = color1;
    baseColor2 = color2;
    _lastMillis = 0;
    _helperFlag = false;
    _helperInt = 0;

    setInterval(interval);

    if (newAnimation == Solid)
    {
      baseColor = color1;
    }
    else if (newAnimation == IndividualRandom)
    {
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        led_array[i] = CRGB(random8(), random8(), random8());
      }
    }
    else if (newAnimation == Blink)
    {
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        led_array[i] = baseColor;
      }
    }
    else if (newAnimation == BlinkTwoColor)
    {
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        led_array[i] = baseColor;
      }
    }
    else if (newAnimation == BlinkFinite)
    {
      _helperInt = _arg0;
      Serial.println(_arg0);
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        led_array[i] = baseColor;
      }
    }
    else if (newAnimation == FadeInOutSolid)
    {
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        led_array[i] = baseColor;
      }
    }
    else if (current_animation == RandomWithFade)
    {
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        led_array[i] = CHSV(random8(), 255, 255);
      }
      _helperInt = 0;
    }
    else if (current_animation == OnePixelLoop)
    {
      _helperInt = 0;
      _helperFlag = true;
      for (size_t i = _start_index + 1; i <= _end_index; i++)
      {
        led_array[i] = baseColor2;
      }
      led_array[_start_index] = baseColor;
    }
    else if (current_animation == WiFi_Blink)
    {
      _helperFlag = true;
    }
  }

  void setFixed2(CRGB led_array[], int size, CRGB color1 = 0xFF)
  {
    setFixed(led_array, _start_index, _start_index + size, color1);
    Serial.printf("s: %d f: %d  -%dd\n", _start_index, _start_index + size, size);
  }
  void setFixed(CRGB led_array[], int start, int finish, CRGB color1 = 0xFF)
  {
    _progbar_is_running = true;
    Serial.printf("s: %d f: %d\n", start, finish);
    if (finish < start)
    {
      int t = finish;
      finish = start;
      start = t;
    }

    if (start < _start_index)
      start = _start_index;
    if (finish > _end_index)
      finish = _end_index;
    FastLED.clear();
    for (size_t i = start; i <= finish; i++)
    {
      led_array[i] = color1;
    }
    FastLED.show();
  }

  void showStages(CRGB led_array[])
  {
    _progbar_is_running = true;
    byte b = 0;
    FastLED.clear();
    for (size_t i = _start_index; i <= _end_index; i++)
    {
      if (i % 5 == 0)
      {
        led_array[i] = CRGB::White;
        b++;
      }
      else
      {
        led_array[i] = CRGB::Red;
      }
    }
    FastLED.show();
  }

  void run(CRGB led_array[])
  {
    if (millis() < _lastMillis + _interval || _progbar_is_running)
      return;
    _lastMillis = millis();

    if (current_animation == Solid)
    {
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        led_array[i] = baseColor;
      }
    }
    else if (current_animation == IndividualRandom)
    {
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        led_array[i] = CHSV(random8(), 255, 127);
      }
    }
    else if (current_animation == Blink)
    {
      // for (size_t i = _start_index; i <= _end_index; i++)
      // {
      //   leds[i] = CRGB(baseColor.r * _helperFlag, baseColor.g * _helperFlag, baseColor.b * _helperFlag);
      // }
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        CHSV hsv = RGBtoHSV(baseColor);
        hsv.v = hsv.v * _helperFlag;
        led_array[i] = hsv;
      }
      _helperFlag = !_helperFlag;
    }
    else if (current_animation == BlinkTwoColor)
    {
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        strip_leds[i] = CRGB(baseColor.r * _helperFlag + baseColor2.r * !_helperFlag, baseColor.g * _helperFlag + baseColor2.g * !_helperFlag, baseColor.b * _helperFlag + baseColor2.b * !_helperFlag);
      }
      _helperFlag = !_helperFlag;
    }
    else if (current_animation == BlinkFinite)
    {
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        CHSV hsv = RGBtoHSV(baseColor);
        hsv.v = hsv.v * _helperFlag;
        led_array[i] = hsv;
      }
      Serial.println(_helperInt);

      _helperFlag = !_helperFlag;
      _helperInt--;
      if (_helperInt == 0)
        goToLastMode(led_array);
    }
    else if (current_animation == FadeInOutSolid)
    {
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        led_array[i] = baseColor;
        int brigthness = 0;
        if (_backwards)
          brigthness = _helperInt * 10;
        else
          brigthness = (25 - _helperInt) * 10;
        if (brigthness > 255)
          brigthness = 255;
        else if (brigthness < 0)
          brigthness = 0;

        led_array[i].maximizeBrightness(brigthness);
      }
      _helperInt++;
      if (_helperInt == 26)
      {
        _helperInt = 0;
        _backwards = !_backwards;
      }
    }
    else if (current_animation == FadeInOutRandom)
    {
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        led_array[i] = CHSV(random8(), 255, (25 - _helperInt) * 10);
      }
      if (!_backwards)
        _helperInt++;
      else
        _helperInt--;
      if (_helperInt == 26 || _helperInt == 0)
      {
        _helperInt = 0;
        _backwards = !_backwards;
      }
    }
    else if (current_animation == RandomWithFade)
    {
      if (_helperInt >= 100)
      {
        CRGB b = CHSV(random8(), 255, 255);
        for (size_t i = _start_index; i <= _end_index; i++)
        {
          led_array[i] = b;
        }
        _helperInt = 0;
      }
      else
      {
        fade_raw(led_array, _num_of_leds, 5);
        _helperInt++;
      }
    }
    else if (current_animation == DebugCodes)
    {
      CRGB code_colors[3] = {CRGB::Black, CRGB::Black, CRGB::Black};
      if (WiFi.isConnected())
      {

        int rssi = WiFi.RSSI();
        if (rssi < -80)
          code_colors[0] = CRGB::DarkOrange;
        if (rssi < -60)
          code_colors[0] = CRGB::Gold;
        else
          code_colors[0] = CRGB::DarkGreen;
      }
      else
        code_colors[0] = CRGB::DarkRed;
      if (Blynk.connected())
        code_colors[1] = CRGB::DarkGreen;
      else
        code_colors[1] = CRGB::DarkRed;
      if (HiveMQ.connected())
        code_colors[2] = CRGB::DarkGreen;
      else
        code_colors[2] = CRGB::DarkRed;

      for (size_t i = 0; i < 3; i++)
      {
        if (i <= _end_index)
          led_array[i] = code_colors[i].nscale8_video(64);
      }
    }
    else if (current_animation == OnePixelLoop)
    {
      led_array[_start_index + _helperInt] = baseColor2;
      if (_helperFlag)
      {
        _helperInt++;
        if (_start_index + _helperInt > _end_index)
        {
          _helperFlag = !_helperFlag;
          _helperInt--;
        }
      }
      else
      {
        _helperInt--;
        if (_start_index + _helperInt < _start_index)
        {
          _helperFlag = !_helperFlag;
          _helperInt++;
        }
      }
      led_array[_start_index + _helperInt] = baseColor;
    }
    else if (current_animation == WiFiStrength)
    {
      int rssi = WiFi.RSSI();
      int percent_index = map(rssi, -90, -50, _start_index, _end_index);
      for (size_t i = _start_index; i <= _end_index; i++)
      {
        if (i <= percent_index)
        {
          led_array[i] = baseColor;
        }
        else
        {
          led_array[i] = baseColor2;
        }
      }
    }
    else if (current_animation == WiFiStrengthInverse)
    {
      int rssi = WiFi.RSSI();
      int percent_index = map(rssi, -90, -50, _start_index, _end_index);
      for (signed int i = _end_index; i >= _start_index; i--)
      {
        if (i >= _end_index - percent_index)
        {
          led_array[i] = baseColor;
        }
        else
        {
          led_array[i] = baseColor2;
        }
      }
    }
    else if (current_animation == WiFi_Blink)
    {
      if (_helperFlag)
        led_array[_start_index] = baseColor;
      else
        led_array[_start_index] = baseColor2;
      _helperFlag = !_helperFlag;
    }
    FastLED.show();
  }

  CHSV RGBtoHSV(CRGB rgb)
  {
    CHSV hsv = CHSV(0, 0, 0);
    byte xMin = rgb.r;
    if (rgb.g < xMin)
    {
      xMin = rgb.g;
    }
    if (rgb.b < xMin)
    {
      xMin = rgb.b;
    }
    byte xMax = rgb.r;
    if (rgb.g > xMax)
    {
      xMax = rgb.g;
    }
    if (rgb.b > xMax)
    {
      xMax = rgb.b;
    }
    hsv.v = xMax;
    byte delta = xMax - xMin;
    if (xMax != 0)
    {
      hsv.s = (int)(delta) * 255 / xMax;
    }
    else
    {
      hsv.h = 0;
      hsv.s = 0;
      return hsv;
    }
    uint hue = 0;
    if (rgb.r == xMax)
    {
      hue = (rgb.g - rgb.b) * 60 / delta;
    }
    else if (rgb.g == xMax)
    {
      hue = 120 + (rgb.b - rgb.r) * 60 / delta;
    }
    else
    {
      hue = 240 + (rgb.r - rgb.g) * 60 / delta;
    }
    if (hue < 0)
    {
      hue += 360;
    }
    hsv.h = map(hue, 0, 360, 0, 255);
    return hsv;
  }
  void goToLastMode(CRGB led_array[])
  {
    setMode(led_array, old_animation, old_baseColor, old_baseColor2, _old_interval);
  }

  void progBar(CRGB led_array[], byte percentage, byte numLED, CRGB ValueColor = CRGB::DarkGreen, CRGB BackColor = CRGB::DarkRed, int delayMS = 4000)
  {
    _progbar_is_running = true;
    byte oldBrightness = FastLED.getBrightness();
    FastLED.setBrightness(255);
    FastLED.clear();
    if (percentage > 100)
      percentage = 100;
    if (numLED > _end_index)
      numLED = _end_index;
    for (size_t i = _start_index; i < numLED; i++)
    {
      led_array[i] = BackColor;
    }
    FastLED.show();
    int value = round((double)percentage / 100 * numLED);
    for (size_t i = _start_index; i <= value; i++)
    {
      led_array[i] = ValueColor;
      delay(50);
      FastLED.show();
    }
    delay(delayMS);
    FastLED.clear();
    FastLED.setBrightness(oldBrightness);
    _progbar_is_running = false;
  }

  void setCode(CRGB led_array[], String codeString)
  {
    if (current_animation != Codes)
      return;

    CRGB code_colors[3] = {CRGB::Black, CRGB::Black, CRGB::Black};
    if (codeString.length() >= 3)
    {
      for (size_t i = 0; i < 3; i++)
      {
        code_colors[i] = getColorbyChar(codeString[i]);
      }
    }
    else
    {
      for (size_t i = 0; i < codeString.length(); i++)
      {
        code_colors[i] = getColorbyChar(codeString[i]);
      }
    }
    for (size_t i = 0; i < 3; i++)
    {
      led_array[i] = code_colors[i];
    }

    FastLED.show();
  }
  CRGB getColorbyChar(char char_to_parse)
  {
    switch (char_to_parse)
    {
    case 'R':
      return CRGB::Red;
      break;
    case 'G':
      return CRGB::Green;
      break;
    case 'B':
      return CRGB::Blue;
      break;
    case 'Y':
      return CRGB::Yellow;
      break;
    case 'W':
      return CRGB::White;
      break;

    default:
      return CRGB::Black;
      break;
    }
  }
};
LED_Controller LED_Strip, beacons, onBoardLEDs;

// PatternMode oldMode = PatternMode::blink;
void setBeaconColor(CRGB COLOR, bool skipShow = false)
{
  for (size_t i = 0; i < BEACONS_SIZE; i++)
  {
    beacons_leds[i] = COLOR;
  }
  if (!skipShow)
    FastLED.show();
}

void SetOnboardLEDS(CRGB COLOR, bool skipShow = false)
{
  for (size_t i = 0; i < ONBOARD_RGB_SIZE; i++)
  {
    onboard_leds[i] = COLOR;
  }

  if (!skipShow)
    FastLED.show();
}

#pragma endregion

char auth[] = BLYNK_AUTH_TOKEN;
#define TEST_BUTTON 0
#ifdef PROTO
// TEST BUTTON

bool digital_debounce = false;
#endif

#define MAX_HISTORY_SIZE 100
#define DHT_TYPE DHT11
OneWire oneWire(MUX_INPUT);       // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature DS18(&oneWire); // Pass our oneWire reference to Dallas Temperature.
StaticJsonDocument<512> DocJson;
DHT dht(MUX_INPUT, DHT_TYPE);
bool SyncTime(uint32_t time = UINT32_MAX);
#pragma region TCP server

#pragma endregion

// MuxSweeper Mux;
int trigPin = 26;
int echoPin = 27;
int lastState = 0;

NewPing sonar(trigPin, echoPin); // NewPing setup of pins and maximum distance.

#pragma region Sensors

bool wl_half = false;  // water level half.
bool wl_empty = false; // water level empty.
byte Water_Level = 0;  // Water tank % accounted by the 2 sensors.
float DS18inner = 0;   // Value of inner temperature sensor in Celcius.
float DS18Probe = 0;   // Value of DS18 waterproof probe temperature sensor in Celsius.
// uint32_t pump.Last_Auto_Pump = 0; // Time stamp of last time the pump was on;
//  int Auto_pump_interval = 172800;     // 172800; Time in seconds between each pump activation time 172800 s = 48 hours
//  int Auto_pump_timer = 10000;         // Time in ms for the pump to stay on each time it is automatically triggered

#define MAX_MOISTURE_1_READ 3716
#define MOISTURE_READS 10           // number of analog reads for average
byte Moisture_1 = 0;                // Moisture probe 1 % in range - 0 to MAX_MOISTURE_1_READ
byte Moisture_2 = 0;                // Moisture probe 1 % in range - 0 to MAX_MOISTURE_2Moisture_2_READ
float innerTemp = 0, probeTemp = 1; // DS18 temps in Celsius
float DHT11_temp = 0, DHT11_hum = 0;

uint16_t LDR_VALUE = 0;

#pragma endregion

struct command_result
{
  bool result = false;
  String response = "";
};

command_result handleCommand(String message, CommandSource who_are_you)
{
  char *delimiter = " ";
  String extra_arg = "";
  command_result result;
  String response = "";
  bool resolve = true;
  String command = "";
  String args[5] = {"", "", "", "", ""};

  String current_string = "";

  int index = 0;
  int last_index = 0;
  // gets command and args using the delimiters
  for (size_t i = 0; index >= 0; i++)
  {
    current_string = "";
    index = message.indexOf(delimiter, last_index + 1);

    if (last_index > 0)
      last_index += strlen(delimiter);

    if (index == -1)
      current_string = message.substring(last_index);
    else if (index < message.length())
      current_string = message.substring(last_index, index);

    last_index = index;

    if (i == 0)
    {
      current_string.toUpperCase();
      command = current_string;
    }
    else if (i < 6)
      args[i - 1] = current_string;
  }

  // process the command
  command.toUpperCase();
  if (command == "HELP" || command == "H")
  {
    // Do documentation
    response += ("Welcome to NightMare Home Systems ©\nThis is a ESP32 Module and it can answer to the following commands:\n");
    response += ("Quick obs.: the character int [19] or char () is ignored when recieved for facilitating reasons.\n");
    response += ("obs.: [param]* = required parameter, [param] = optional parameter, {param} = optional parameter in any position after required parameters.\n");
    response += ("obs.: the command is normalized to upper case.\n");

    response += "REQ_CLIENTS                                                      > Request the clients connected to the tcp server\n";
    response += "REQ_FILE [file]*                                                 > Request a specific file (read as ASCII)\n";
    response += "REQ_FILES                                                        > Request Available Files \n";
    response += "REQ_LOG                                                          > requests a software reset.\n";
    response += "PREF [Preference] [New Value]                                    > Change the selected user preferece to the new value (always an int_32t).\n";
    response += "GET_PREFS                                                        > Gets the user preferences, the key to be used with PREF and their current values.\n";
    response += "HWINFO                                                           > requests the hardware conections of this board.\n";
    response += "READ_PIN [pin]* [analog?] [#n reads] [SIPO_Enable index]         > Reads Data from one of the pins. (use -A for Analog read)\n";
    response += "READ_MUX [Index]* [#n reads] [SIPO_Enable index] {digital?}      > Reads the value on one of the ports of the MUX. (use -D for digital read)\n";
    response += "WRITE_SIPO [DATA]* [Data format(HEX)(BIN)]                       > Writes a specific data to the SIPO.\n";
    response += "GET_IP                                                           > Gets current IP.\n";
    response += "PUMP_INFO                                                        > Gets the info for when pump was last automaticaly triggered and next scheduled time.\n";
    response += "TRIGGER_US [#n reads]                                            > Reads the value from the Ultrasonic Sensor.\n";
    response = commandInfo;
    args[0].toLowerCase();
    if (args[0] == "json")
    {
      args[1].toLowerCase();
      bool pretty = args[1] == "-p" || args[1] == "p";
      response = generateCommandInfoJson(pretty);
    }
    else
    {
      response = generateCommandInfo();
    }
  }
  else if (command == "HARDWARE_INFO" || command == "HWINFO")
  {
    response += "\nSerial in Parallel out Shift Register: \n";
    response += "  *Clock [";
    response += SIPO_CLK;
    response += "]\n";
    response += "  *Data  [";
    response += SIPO_DATA;
    response += "]\n";
    response += "  *Latch [";
    response += SIPO_LATCH;
    response += "]\n";
    response += "   -LED0             @ 0utput 0\n";
    response += "   -LED1             @ 0utput 1\n";
    response += "   -LED2             @ 0utput 2\n";
    response += "   -PUMP RELAY       @ 0utput 5\n";
    response += "   -MOISTURE EN0     @ 0utput 3\n";
    response += "   -MOISTURE EN1     @ 0utput 4\n";

    response += "16-1 MUX: \n";
    response += "  *Common Input       [";
    response += MUX_INPUT;
    response += "]\n";
    response += "  *Enable             [";
    response += MUX_ENABLE;
    response += "]\n";
    response += "  *Input Selector 0-3 [";
    response += MUX_S0;
    response += "] [";
    response += MUX_S1;
    response += "] [";
    response += MUX_S2;
    response += "] [";
    response += MUX_S3;
    response += "] - bit 0 shared whith SIPO Data and bit 1 shared with SIPO Latch\n";
    response += "   -DS18b20 BUS           @ Port 10\n";
    response += "   -DHT11 BUS             @ Port  9\n";
    response += "   -LDR                   @ Port 14\n";
    response += "   -MOISTURE SENSOR 0     @ Port  0\n";
    response += "   -MOISTURE SENSOR 1     @ Port  1\n";
    response += "   -MOISTURE SENSOR 2     @ Port  2\n";
    response += "   -MOISTURE SENSOR 3     @ Port  3\n";
    response += "   -WATER LEVEL SENSOR 0  @ Port 13\n";
    response += "   -WATER LEVEL SENSOR 1  @ Port 12\n";
    response += "   -RAIN                  @ Port  4\n";
    response += "Onboard RGB: [2]\n";
    response += "WS2818b    : [23]\n";
    response += "WS2811     : [26]\n";
    response += "Ultrasonic HC-SR04: \n";
    response += "  *TRIGGER                           [";
    response += US_TRIG;
    response += "]\n";
    response += "  *ECHO with 5-3.3 voltage divider   [";
    response += US_ECHO;
    response += "]\n";
  }
  else if (command == "REQ_CLIENTS")
  {
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
      if (NULL != tcpServer.clients[i].client)
      {
        response += "Client [";
        response += i;
        response += "], IP: ";
        response += tcpServer.clients[i].client->remoteIP().toString();
        response += ", (self) ID:";
        response += tcpServer.clients[i].clientsID;
        response += ", INFO: ";
        response += tcpServer.clients[i].clientsStatus[i];
        response += "\n";
      }
    }
  }
  else if (command == "REQ_FILE")
  {
    if (control_variables.flags.LittleFS_mounted)
    {
      if (LittleFS.exists(args[0]))
      {
        response += LittleFS.open(args[0]).readString();
      }
      else
      {
        response = "File: '";
        response += args[0];
        response += "' does not exist";
        resolve = false;
      }
    }
    else
    {
      response = "LittleFS not mount";
      resolve = false;
    }
  }
  else if (command == "REQ_FILES")
  {
    if (control_variables.flags.LittleFS_mounted)
    {
      response = listAllFiles("/");
    }
    else
    {
      response = "SPIFFS not mount";
      resolve = false;
    }
  }
  else if (command == "REQ_LOG")
  {
    response = Sensors.printString();
  }
  else if (command == "DEL_FILE")
  {
    if (control_variables.flags.LittleFS_mounted)
    {
      if (LittleFS.exists(args[0]))
      {
        LittleFS.remove(args[0]);
        response = "File: '";
        response += args[0];
        response += "' deleted!";
      }
      else
      {
        response = "File: '";
        response += args[0];
        response += "' does not exist";
        resolve = false;
      }
    }
    else
    {
      response = "LittleFS not mount";
      resolve = false;
    }
  }
  else if (command == "PREF")
  {
    int key = atoi(args[1].c_str());
    if (key == 0 && args[1] == "0" || key != 0)
    {
      if (user_preferences.SetValue(args[0], key))
      {
        response = "Success. ";
        response += args[0];
        response += " is now: ";
        response += key;
      }
      else
      {
        response = "Preference: '";
        response += args[0];
        response += "' was not found";
        resolve = false;
      }
    }
    else
    {
      response = "key: '";
      response += args[1];
      response += "' is in the wrong format";
      resolve = false;
    }
  }
  else if (command == "GET_PREFS")
  {
    response += user_preferences.Print();
    response += "\n";
  }
  else if (command == "GET_IP")
  {
    response += "Current IP: ";
    response += WiFi.localIP().toString();
    response += "\n";
  }
  else if (command == "PUMP_INFO")
  {
    response += "Last Auto Pump  ";
    if (LittleFS.exists("/lastAutoPumpReadable.txt"))
      response += LittleFS.open("/lastAutoPumpReadable.txt").readString();
    response += " || Next: ";
    response += Pump.last_auto_run + user_preferences.auto_pump_interval;
    response += " (";
    response += timestampToDateString(Pump.last_auto_run + user_preferences.auto_pump_interval, DateAndTime);
    response += ") ";
    response += " || Pump Run Time: ";
    response += Pump.run_time_from_50;
    response += " || Estimated next activation: ";
    response += timestampToDateString(Pump.predictNextRun(), DateAndTime);
    if (LittleFS.exists("/lastAutoPump.txt"))
    {
      int t = atoi(LittleFS.open("/lastAutoPump.txt").readString().c_str());
      Serial.printf("%d = %s\n", t, timestampToDateString(t, DateAndTime).c_str());
    }

    // response += auto_pump_time_ajusted(now());
  }
  else if (command == "PUMP_TEST")
  {
    response += "last_pump:";
    response += Pump.last_auto_run;
    response += " (";
    response += timestampToDateString(Pump.last_auto_run, DateAndTime);
    response += ") | now:";
    uint current_time = now();
    response += current_time;
    response += " || p.p.h: ";
    response += user_preferences.prefered_pump_hour;
    response += " || a.p.i: ";
    response += user_preferences.auto_pump_interval;
    String test = "control";
    if (user_preferences.prefered_pump_hour > 23 && user_preferences.prefered_pump_hour < -1)
      user_preferences.prefered_pump_hour = 19;
    // Waits for the prefered time after the interval has passed
    if (current_time - Pump.last_auto_run >= user_preferences.auto_pump_interval && hour() == user_preferences.prefered_pump_hour)
      test += "passOne";
    // If we are whithin 4 hours to the interval and at the prefered time we water the garden earlier
    if (current_time - Pump.last_auto_run >= user_preferences.auto_pump_interval - 4 * 60 * 60 && hour() == user_preferences.prefered_pump_hour)
      test += "passTwo";
    // If the prefered time is disabled (-1) waters the garden at the interval
    if (current_time - Pump.last_auto_run >= user_preferences.auto_pump_interval && user_preferences.prefered_pump_hour == -1)
      test += "passThree";
    // If it has been 4 hours since the interval has passed and the prefered hour have not been reached we water the garden anyways
    if (current_time - Pump.last_auto_run >= user_preferences.auto_pump_interval + 4 * 60 * 60)
      test += "passFour";
    response += "; {";
    response += test;
    response += "}";
  }
  else if (command == "PUMP")
  {
    uint16_t runtime = atoi(args[0].c_str());
    if (runtime == 0 && args[0] != "0")
    {
      /* Conversion Error */
      result.result = false;
      result.response = "failed to parse: ";
      result.response += args[0];
      return result;
    }
    response = "Pump manual trigger for [";
    response += runtime;
    response += "] ms. Pump is on: SIPO output: ";
    response += RELAY_PIN;
    if (args[1] == "-f")
    {
      Pump.force_start = true;
    }
    Pump.Start(runtime, who_are_you);
    if (args[1] == "-f")
    {
      Pump.force_start = false;
    }
  }
  else if (command == "TRIGGER_US")
  {
    pinMode(echoPin, INPUT);
    int reads = atoi(args[0].c_str());
    if (reads < 1 || reads > 20)
      reads = 5;
    int oldMillis = millis();            // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
    int dist = sonar.ping_median(reads); // Send ping, get distance in cm and print result (0 = outside set distance range)
    int timespent = millis() - oldMillis;
    float result = (float)dist / US_ROUNDTRIP_CM;
    response = "dist: ";
    response += result;
    response += "cm, reads: ";
    response += reads;
    response += ", time: ";
    response += timespent;
    response += "ms";
  }
  else if (command == "READ_PIN")
  {
    uint8_t pin = atoi(args[0].c_str());
    uint8_t num_reads = atoi(args[2].c_str());
    uint8_t sipo_enable_pin = atoi(args[3].c_str());
    bool use_sipo = true;
    if (sipo_enable_pin == 0 && args[3] != "0")
      use_sipo = false;

    response = "Reading pin [";
    response += pin;
    response += "]";

    if (args[1] == "ANALOG" || args[1] == "-A")
    {
      response += " [A] ";
      int _analogReads = 0;
      if (num_reads == 0)
        num_reads = 10;
      if (use_sipo)
      {
        response += "[S-EN: ";
        response += sipo_enable_pin;
        response += "]";
        SIPO_Write(sipo_enable_pin, true, true);
      }
      response += "[";
      response += num_reads;
      response += "]: ";

      for (size_t i = 0; i < num_reads; i++)
      {
        int new_read = analogRead(pin);
        response += new_read;
        response += " | ";
        _analogReads += new_read;
      }
      response += "avg : ";
      response += _analogReads / num_reads;

      if (use_sipo)
        SIPO_Write(sipo_enable_pin, true, false);
    }
    else
    {
      response += " [D]: ";
      response += digitalRead(pin);
    }
  }
  else if (command == "READ_MUX")
  {
    uint8_t old_mux_index = MUX_VALUE;
    uint8_t mux_index = atoi(args[0].c_str());
    uint8_t num_reads = atoi(args[1].c_str());
    uint8_t sipo_enable_pin = atoi(args[3].c_str());

    if (mux_index > 15 || (mux_index == 0 && args[0] != "0"))
    {
      result.response = "Invalid mux port [0-15]";
      result.result = false;
      return result;
    }
    bool use_sipo = true;
    if (sipo_enable_pin == 0 && args[3] != "2")
      use_sipo = false;

    if (num_reads == 0)
      num_reads = 3;

    response = "Reading Mux Channel [";
    response += mux_index;
    response += "] ";

    setMuxPort(mux_index);

    if (args[1] == "-D" || args[2] == "-D" || args[3] == "-D" || args[4] == "-D")
    {
      response += "[D]: ";
      response += digitalRead(MUX_INPUT);
    }
    else
    {
      if (use_sipo)
      {
        response += "[S-EN: ";
        response += sipo_enable_pin;
        response += "] ";
        SIPO_Write(sipo_enable_pin, true, true);
      }
      response += "[";
      response += num_reads;
      response += "]: ";

      int _analogReads = 0;
      if (num_reads == 0)

        for (size_t i = 0; i < num_reads; i++)
        {
          int new_read = analogRead(MUX_INPUT);
          response += new_read;
          response += " | ";
          _analogReads += new_read;
        }
      response += "avg : ";
      response += (float)(_analogReads / num_reads);
      if (use_sipo)
        SIPO_Write(sipo_enable_pin, false, true);
    }
    setMuxPort(old_mux_index);
  }
  else if (command == "READ10")
  {
    uint8_t old_mux_index = MUX_VALUE;
    uint8_t mux_index = atoi(args[0].c_str());

    if (mux_index > 15 || (mux_index == 0 && args[0] != "0"))
    {
      result.response = "Invalid mux port [0-15]";
      result.result = false;
      return result;
    }

    response = "Reading Mux Channel [";
    response += mux_index;
    response += "] ";

    setMuxPort(mux_index);

    if (args[1] == "-D" || args[2] == "-D" || args[3] == "-D" || args[4] == "-D")
    {
      response += "[D]: ";
      response += digitalRead(MUX_INPUT);
    }
    else
    {
      int new_read = analogRead(MUX_INPUT);
      response += "avg : ";
      response += new_read;
    }
    //   setMuxPort(old_mux_index);
  }
  else if (command == "READC")
  {

    response = "Reading Mux Channel [";
    response += MUX_VALUE;
    response += "] ";
    delay(1);

    if (args[1] == "-D" || args[2] == "-D" || args[3] == "-D" || args[0] == "-D")
    {
      response += "[D]: ";
      response += digitalRead(MUX_INPUT);
    }
    else
    {
      response += analogRead(MUX_INPUT);
    }

    //   setMuxPort(old_mux_index);
  }
  else if (command == "SET_MUX")
  {
    uint8_t new_byte = strtol(args[0].c_str(), NULL, 10);

    if (new_byte == 0 && args[0] != "0")
    {
      result.response = "Invalid byte [0-255] [BIN|HEX]";
      result.result = false;
      return result;
    }
    else
    {
      if (new_byte > 15)
      {
        result.response = "Invalid byte [0-255] [BIN|HEX]";
        result.result = false;
        return result;
      }
      setMuxPort(new_byte);
      response = "0b";
      for (int i = 0; i < 4; i++)
      {
        response += String(digitalRead(MUX_CONTROL[i]), BIN);
      }
      response += " Written to the mux_selector.";
      resolve = true;
    }
  }
  else if (command == "WRITE_SIPO")
  {
    uint8_t new_byte = 0;
    if (args[1] == "HEX")
      new_byte = strtol(args[0].c_str(), NULL, HEX);
    else if (args[1] == "BIN")
      new_byte = strtol(args[0].c_str(), NULL, BIN);
    else
      new_byte = strtol(args[0].c_str(), NULL, 10);
    if (new_byte == 0 && args[0] != "0")
    {
      result.response = "Invalid byte [0-255] [BIN|HEX]";
      result.result = false;
      return result;
    }
    else
    {

      SIPO_WriteBYTE(new_byte);
      response = "0b";
      response += String(new_byte, BIN);
      response += " Written to the SIPO.";
      resolve = true;
    }
  }
  else if (command == "DS18TEST")
  {
    pinMode(MUX_INPUT, INPUT);
    setMuxPort(DS18b20_PIN);
    Serial.println("Starting test");
    Serial.println(MUX_VALUE);
    delay(1);
    Serial.println("devices: ");
    Serial.println(DS18.getDeviceCount());
    DeviceAddress aaa;
    // oneWire.reset_search();
    // bool result = oneWire.search(aaa);
    // Serial.printf("%d : ",result);
    // for (size_t i = 0; i < 8; i++)
    //   {
    //     if (aaa[i] < 0x10)
    //       Serial.print("0");
    //     Serial.print(aaa[i], HEX);
    //   }
    //   Serial.println();
    for (size_t i = 0; i < DS18.getDeviceCount(); i++)
    {
      DS18.getAddress(aaa, i);
      Serial.printf("ADDR: [%d] 0x", i);
      for (size_t i = 0; i < 8; i++)
      {
        if (aaa[i] < 0x10)
          Serial.print("0");
        Serial.print(aaa[i], HEX);
      }
      Serial.println();
    }
    DS18.requestTemperatures();
    float res = DS18.getTempCByIndex(0);
    float res2 = DS18.getTempCByIndex(1);
    Serial.printf("Test Result [0]:%f [1]:%f\n", res, res2);
  }
  else if (command == "CODE")
  {
    onBoardLEDs.setCode(onboard_leds, args[0]);
    response = "Writing ";
    response += args[0];
    response += "on onboard LEDs";
  }
  else if (command == "SENSORS")
  {
    response = Sensors.serializeValues(true);
    resolve = true;
  }
  else if (command == "SENSORS-RESET")
  {
    InitializeSensors(true);
    response = "Sensors set back to default.";
    resolve = true;
  }
  else if (command == "SIPOPIN")
  {
    uint8_t pin = 0;
    pin = strtol(args[0].c_str(), NULL, 10);
    if (pin == 0 && args[0] != "0")
    {
      result.response = "Invalid pin [0-7]";
      result.result = false;
      return result;
    }
    uint8_t val = 0;
    val = strtol(args[1].c_str(), NULL, 10);
    bool _val = false;
    if (pin == 0 && args[1] != "0")
    {
      result.response = "Invalid value [bool]";
      result.result = false;
      return result;
    }
    if (val > 0)
      _val = true;
    SIPO_Write(pin, _val);
    response = "Writing [";
    response += _val;
    response += "] to pin [";
    response += pin;
    response += "] of SIPO\n";
    resolve = true;
  }
  else if (command == "REPORT")
  {
    args[0].toLowerCase();
    if (args[0] == "all")
    {
      for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
      {
        /* code */
      }

      Sensors.getSensorByLabel(args[0])->_report = !Sensors.getSensorByLabel(args[0])->_report;
    }

    Sensors.getSensorByLabel(args[0])->_report = !Sensors.getSensorByLabel(args[0])->_report;
    resolve = true;
    response = "reporting: ";
    response += args[0];
    response += " is now: ";
    response += Sensors.getSensorByLabel(args[0])->_report ? "enable" : "disable";
  }
  else if (command == "TOGGLEUPDATES")
  {
    control_variables.flags.disable_sensors_update = !control_variables.flags.disable_sensors_update;
    response += "Sensor updates are now: ";
    response += control_variables.flags.disable_sensors_update ? "disable" : "enable";
    resolve = true;
  }
  else if (command == "AGE")
  {
    response = "";
    response += "[";
    response += control_variables.flags.time_synced ? "Synced" : "Not Synced";
    response += "] TimeStamp: ";
    response += control_variables.boot_time;
    response += " || ";
    response += timestampToDateString(control_variables.boot_time, TimeStampFormat::TimeSinceStamp);
    response += " || ";
    response += timestampToDateString(control_variables.boot_time, TimeStampFormat::DateAndTime);
    resolve = true;

    // Serial.printf("[Synced: %d] TimeStamp:  %d|| Hours: %f || Mins: %f\n",control_variables.time_synced,now(), (double)now()/3600, (double)now()/60);
  }
  else if (command == "CONTROL_VARIABLES")
  {
    // response = control_variables.getInternalsAsString();
    response = "not implemented";
    resolve = true;
  }
  else if (command == "MUXSWEEP")
  {
    byte old_mux = MUX_VALUE;
    response += "Mux Sweep:\n";
    for (size_t i = 0; i < 15; i++)
    {
      setMuxPort(i);
      int an_result = 0;
      for (size_t j = 0; j < 5; j++)
      {
        an_result += analogRead(MUX_INPUT);
        delay(10);
      }
      delay(50);
      pinMode(MUX_INPUT, INPUT);
      auto dg_result = digitalRead(MUX_INPUT);
      String an_str = String(an_result / 5);

      response += " [";
      response += getMuxAlias(i);
      response += "]   *port: ";
      if (i < 10)
        response += " ";
      response += i;
      response += " Analog[5]: ";
      for (size_t k = 0; k <= 4; k++)
      {
        if (k > an_str.length())
          response += " ";
      }
      response += an_str;
      response += "  Digital: ";
      response += dg_result;
      response += "\n";
      delay(100);
    }
    // Restores the old value on the mux after the sweep
    setMuxPort(old_mux);
  }
  else if (command == "MUX-SIPOSWEEP")
  {
    byte old_mux = MUX_VALUE;
    response += "Mux-SIPO Sweep:\n";
    SIPO_Write(MOIST_EN0, HIGH);
    response += " - SIPO 3 Enabled:\n";
    for (size_t i = 0; i < 15; i++)
    {
      setMuxPort(i);
      int an_result = 0;
      for (size_t j = 0; j < 5; j++)
      {
        an_result += analogRead(MUX_INPUT);
        delay(10);
      }
      pinMode(MUX_INPUT, INPUT);
      auto dg_result = digitalRead(MUX_INPUT);
      String an_str = String(an_result / 5);
      response += " [";
      response += getMuxAlias(i);
      response += "]   *port: ";
      if (i < 10)
        response += " ";
      response += i;
      response += " Analog[5]: ";
      for (size_t k = 0; k <= 4; k++)
      {
        if (k > an_str.length())
          response += " ";
      }
      response += an_str;
      response += "  Digital: ";
      response += dg_result;
      response += "\n";
    }
    SIPO_Write(MOIST_EN0, LOW);
    SIPO_Write(MOIST_EN1, HIGH);
    response += " - SIPO 4 Enabled:\n";
    for (size_t i = 0; i < 15; i++)
    {
      setMuxPort(i);
      int an_result = 0;
      for (size_t j = 0; j < 5; j++)
      {
        an_result += analogRead(MUX_INPUT);
        delayMicroseconds(100);
      }
      pinMode(MUX_INPUT, INPUT);
      auto dg_result = digitalRead(MUX_INPUT);
      String an_str = String(an_result / 5);

      response += " [";
      response += getMuxAlias(i);
      response += "]   *port: ";
      if (i < 10)
        response += " ";
      response += i;
      response += " Analog[5]: ";
      for (size_t k = 0; k <= 4; k++)
      {
        if (k > an_str.length())
          response += " ";
      }
      response += an_str;
      response += "  Digital: ";
      response += dg_result;
      response += "\n";
    }
    SIPO_Write(MOIST_EN1, LOW);
    // Restores the old value on the mux after the sweep
    setMuxPort(old_mux);
  }
  else if (command == "TESTLED")
  {
    int start = atoi(args[0].c_str());
    int end = atoi(args[1].c_str());
    if (start == 0 && end == 0)
    {
      response = "Inavlid Input: ";
      response += args[0];
      LED_Strip.showStages(strip_leds);
    }
    else
    {
      if (end > 0)
      {
        LED_Strip.setFixed(strip_leds, start, end, CRGB::Blue);
        response = "Setting strip from ";
        response += start;
        response += " to ";
        response += end;
        response += " to Blue";
      }
      else
      {
        LED_Strip.setFixed2(strip_leds, start, CRGB::Blue);
        response = "Setting strip with size ";
        response += start;
        response += " to Blue";
      }
    }
  }
  else if (command == "VERSION")
  {
    response = "VER: ";
    response += VERSION;
    response += " BUILD TIME: ";
    response += BUILD_TIMESTAMP;
  }
  else if (command == "TIME")
  {
    response = formatString("Time: %d | %s", now(), timestampToDateString(now()).c_str());
  }
  else if (command == "AUTOSYNCTIME")
  {
    if (control_variables.flags.time_synced)
    {
      response = "Error: Time is already synced. Use manual 'SYNCTIME' to force sync.";
    }
    else
    {
      bool result = SyncTime(strtoul(args[0].c_str(), NULL, 10));
      response = formatString("[AUTO] %s: Time Sync: %s", result? "success":"failed",  timestampToDateString(now()).c_str());
    }
  }

  else if (command == "SYNCTIME")
  {
    bool result = SyncTime(strtoul(args[0].c_str(), NULL, 10));
    response = formatString("[%d] Now: %d | %s", result, now(), timestampToDateString(now()).c_str());
  }
  else if (command == "LOG")
  {
    if (control_variables.flags.LittleFS_mounted)
    {
      if (LittleFS.exists("/log.txt"))
      {
        response += LittleFS.open("/log.txt").readString();
      }
      else
      {
        response = "File: '/log.txt' does not exist";
        resolve = false;
      }
    }
    else
    {
      response = "LittleFS not mount";
      resolve = false;
    }
  }
  else if (command == "I2CSCAN")
  {
    response = ExtAdc.i2cScan();
  }
  else if (command == "ADC")
  {
    response += ExtAdc.adcSweep();
  }
  else if (command == "EASWEEP")
  {
    // int _delay = atoi(args[0].c_str());
    // if (_delay <= 0 || _delay > 1000)
    // {
    //   _delay = 50;
    // }

    response += ExtAdc.sweepAndCompare();
  }

  else if (command == "DIRECTMOIST")
  {
    useDirectAdc = !useDirectAdc;
    response = "Direct Moisture Reading is now: ";
    response += useDirectAdc ? "enabled" : "disabled";
  }

  else if (command == "SETWIFI")
  {

    user_preferences.WifiSSID = args[0];
    user_preferences.WifiPASSWD = args[1];

    WiFi.disconnect();
    WiFi.begin(user_preferences.WifiSSID, user_preferences.WifiPASSWD);
    response = formatString("WiFi: SSID: '%s' PASSWD: '%s'\n", args[0].c_str(), args[1].c_str());
    if (control_variables.flags.LittleFS_mounted)
    {
      user_preferences.Save();
      response += "Wifi Config Saved!";
    }
    else
    {
      response += "LittleFS not mount cant save Wifi config.";
    }
  }
  else if (command == "CONFIG")
  {
    user_preferences.FromString(args[0]);
    user_preferences.Save();
    response = formatString("Config Saved! -> '%s'", user_preferences.ToString().c_str());
  }
  else if (command == "RESET")
  {
    ESP.restart();
  }
  else
  {
    resolve = false;
    response = "Command '" + command + "' not identified. Try 'help'";
  }
  result.response = response;
  result.result = resolve;
  // Serial.printf("message = %s | command = %s | args[0] = %s | args[1] = %s | args[2] = %s |\n", message.c_str(), command.c_str(), args[0].c_str(), args[1].c_str(), args[2].c_str());

  return result;
}

#pragma region BLYNK

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable

  if (user_preferences.AutoLDR)
  {
    user_preferences.AutoLDR = false;
    uint8_t new_hour = hour() + 12;
    if (new_hour > 23)
      new_hour = new_hour - 24;
    control_variables.auto_ldr_resume_time = new_hour;
  }
  int valueV0 = param.asInt();
  Serial.printf("[V0][%d][Blynk] >> Value Recieved = %d\n", now(), valueV0);
  SIPO_Write(LED_0, valueV0);
  FastLED.setBrightness(valueV0 * 0xFF);
}

BLYNK_WRITE(V8)
{
  // Set incoming value from pin V0 to a variable
  int valueV8 = param.asInt();
  Serial.printf("[V8][%d][Blynk] >> Value Recieved = %d\n", now(), valueV8);
  if (valueV8)
    Pump.Start(-1, 2);
  else
    Pump.Stop();
}

BLYNK_WRITE(V9)
{
  // Set incoming value from pin V0 to a variable
  String valueV9 = param.asStr();
  Serial.printf("[V9][%d][Blynk] >> Value Recieved = %s \n", now(), valueV9);

  if (control_variables.flags.LittleFS_mounted)
  {
    if (valueV9 == "del logs")
    {
      LittleFS.remove("/log.txt");
      Blynk.virtualWrite(V9, "logs cleaned");
    }
    else if (valueV9 == "del auto")
    {
      LittleFS.remove("/lastAutoPump.txt");
      Blynk.virtualWrite(V9, "last auto pump reset");
      Pump.last_auto_run = 0;
    }
    else if (valueV9 == "del sensors")
    {
      LittleFS.remove("/sensors.txt");
      Blynk.virtualWrite(V9, "sensors log were deleted.");
      control_variables.last_sensor_log = 0;
    }
    else if (valueV9 == "format")
    {
      bool result = LittleFS.format();
      String msg = "SPIFFS formatted = ";
      msg += result;
      Blynk.virtualWrite(V9, msg);
    }
    else if (valueV9 == "usage")
    {
      double result = LittleFS.usedBytes() / LittleFS.totalBytes();
      String msg = "SPIFFS usage = ";
      msg += LittleFS.usedBytes();
      msg += " of ";
      msg += LittleFS.totalBytes();
      msg += "Bytes [";
      msg += result;
      msg += "%]";
      Blynk.virtualWrite(V9, msg);
    }
    else if (LittleFS.exists(valueV9))
      Blynk.virtualWrite(V9, LittleFS.open(valueV9).readString());
  }
  if (valueV9[0] == '$')
    Pump.Start(atoi(valueV9.substring(1).c_str()));
  else if (valueV9[0] == '#')
  {
    String msg = "Value Recieved: ";
    if (valueV9[1] == '#')
    {

      msg += " [B] - 0x";
      int newbrightness = strtol(valueV9.substring(2).c_str(), NULL, 16);
      msg += String(newbrightness, HEX);
      if (newbrightness >= 0 && newbrightness < 256)
      {
        FastLED.setBrightness(newbrightness);
      }
      else
      {
        msg += " | Invlaid value";
      }
    }
    else
    {
      msg += " [C] - 0x";
      int newColor = strtol(valueV9.substring(1).c_str(), NULL, 16);
      msg += String(newColor, HEX);
      Serial.print("Color : ");
      Serial.println(newColor, HEX);
      //  Blynk.virtualWrite(V9, newColor);
      // fill_solid(leds, 2, newColor);
      setBeaconColor(newColor);
      LED_Strip.setMode(strip_leds, Solid, newColor);
    }
    Blynk.virtualWrite(V9, msg);
    FastLED.show();
  }
  else if (valueV9[0] == '&')
  {
    int sampleSize = strtol(valueV9.substring(1).c_str(), NULL, 16);
    int ldr = 0;
    for (size_t i = 0; i < sampleSize; i++)
    {
      ldr += analogRead(MUX_INPUT);
    }
    if (sampleSize > 0)
      ldr = ldr / sampleSize;

    Blynk.virtualWrite(V9, String(ldr));
  }
  else if (valueV9[0] == '!')
  {
    command_result result = handleCommand(valueV9.substring(1), _Blynk);
    Blynk.virtualWrite(V9, result.response.c_str());
  }
  else if (valueV9 == "autoldr")
  {
    user_preferences.AutoLDR = !user_preferences.AutoLDR;
    String msg = "Auto Brightness based on LDR is now: ";
    msg += user_preferences.AutoLDR;
    Blynk.virtualWrite(V9, msg);
  }
  else if (valueV9 == "ldr")
  {
    Blynk.virtualWrite(V9, String(analogRead(MUX_INPUT)));
  }
  else if (valueV9 == "sensors")
  {
    Blynk.virtualWrite(V9, Sensors.printString());
  }
  else if (valueV9 == "telemetry")
  {
    Blynk_telemetry = !Blynk_telemetry;
    String msg = "Telemetry is now: ";
    msg += Blynk_telemetry ? "enable.\n" : "disable.\n";
    Blynk.virtualWrite(V9, msg);
  }
  else if (valueV9 == "pump")
  {
    String msg = "Last time pump was Activated: ";
    msg += Pump.last_auto_run;
    Blynk.virtualWrite(V9, msg);
  }
  else if (valueV9 == "moist")
  {
    digitalWrite(27, 1);
    String msg = "Read moisture: ";
    msg += analogRead(MUX_INPUT);
    msg += ".\n";
    Blynk.virtualWrite(V9, msg);
    digitalWrite(27, 0);
  }
  else if (valueV9 == "spiffs")
  {
    String msg = "";
    if (!control_variables.flags.LittleFS_mounted)
    {
      msg = "SPIFFS not Mounted!\n";
    }
    else
    {
      msg += "SPIFFS: ";
      msg += (float)LittleFS.usedBytes() / 1024;
      msg += "/";
      msg += (float)LittleFS.totalBytes() / 1024;
      msg += " [kb] [";
      msg += (float)LittleFS.usedBytes() / LittleFS.totalBytes();
      msg += "%]\n";

      fs::File root = LittleFS.open("/");
      fs::File file = root.openNextFile();
      while (file)
      {
        msg += ("file: ");
        msg += (file.name());
        msg += (" || size: ");
        msg += ((float)file.size() / 1024);
        msg += (" [Kb]");
        if (file.isDirectory())
        {
          msg += " [dir]";
        }
        msg += "\n";
        file = root.openNextFile();
      }
      root.close();
      file.close();
    }

    Blynk.virtualWrite(V9, msg);
  }
}

BLYNK_WRITE(V20) // RED
{
  int valueV20 = param.asInt();
  Serial.printf("[V20][%d][Blynk] >> Value Recieved = %d\n", now(), valueV20);
  LED_Strip.setMode(strip_leds, Solid, CRGB(valueV20, LED_Strip.baseColor.g, LED_Strip.baseColor.b));
}

BLYNK_WRITE(V21) // GREEN
{
  int valueV21 = param.asInt();
  Serial.printf("[V21][%d][Blynk] >> Value Recieved = %d\n", now(), valueV21);
  LED_Strip.setMode(strip_leds, Solid, CRGB(LED_Strip.baseColor.r, valueV21, LED_Strip.baseColor.b));
}

BLYNK_WRITE(V22) // BLUE
{
  int valueV22 = param.asInt();
  Serial.printf("[V22][%d][Blynk] >> Value Recieved = %d\n", now(), valueV22);
  LED_Strip.setMode(strip_leds, Solid, CRGB(LED_Strip.baseColor.r, LED_Strip.baseColor.g, valueV22));
}

BLYNK_CONNECTED()
{
}

#pragma endregion

#pragma region OTA

void startOTA()
{
  String type;
  // is_updating = true;
  //  caso a atualização esteja sendo gravada na memória flash externa, então informa "flash"
  if (ArduinoOTA.getCommand() == 0)
    type = "flash";
  else                   // caso a atualização seja feita pela memória interna (file system), então informa "filesystem"
    type = "filesystem"; // U_SPIFFS
  // exibe mensagem junto ao tipo de gravação
  Serial.println("Start updating " + type);
  ExtAdc.otaUpdate(true);
}

void endOTA()
{
  Serial.println("\nEnd");
  ExtAdc.otaUpdate(false);
}

void progressOTA(unsigned int progress, unsigned int total)
{
  // update_progress = (float)progress / total * 100;
  Serial.printf("Progress: %u%%\r\n", (progress / (total / 100)));
}

void errorOTA(ota_error_t error)
{
  Serial.printf("Error[%u]: ", error);
  if (error == OTA_AUTH_ERROR)
    Serial.println("Auth Failed");
  else if (error == OTA_BEGIN_ERROR)
    Serial.println("Begin Failed");
  else if (error == OTA_CONNECT_ERROR)
    Serial.println("Connect Failed");
  else if (error == OTA_RECEIVE_ERROR)
    Serial.println("Receive Failed");
  else if (error == OTA_END_ERROR)
    Serial.println("End Failed");
  ExtAdc.otaUpdate(false);
}

#pragma endregion

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16)
      Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void triggerPumpProtect()
{
  if (Pump.is_running)
  {
    Pump.Stop();
  }
}

String HandleMsg(String msg, byte index)
{
#ifdef DEBUG
  Serial.print("[");
  Serial.print(tcpServer.clients[index].client->remoteIP());
  Serial.printf("]:%d ", tcpServer.clients[index].client->remotePort());
  Serial.print((char)39);
  Serial.print(msg);
  Serial.println((char)39);
#endif

  String response = "";

  if (msg[0] == '@')
  {
    if (msg[1] == '@')
    {
      int pin = atoi(msg.substring(2).c_str());
      response = "Reading [Analog] from pin [";
      response += pin;
      response += "]. ||";
      if (pin == 33)
        digitalWrite(27, HIGH);
      for (size_t i = 0; i < 10; i++)
      {
        response += " {";
        response += analogRead(pin);
        response += "}";
      }

      if (pin == 33)
        digitalWrite(27, LOW);
    }
    else
    {
      int pin = atoi(msg.substring(1).c_str());
      response = "Reading [DIGITAL] from pin [";
      response += pin;
      response += "]. || {";
      response += digitalRead(pin);
      response += "}";
    }
  }
  else if (msg[0] == '!')
  {
    command_result result = handleCommand(msg.substring(1), _TCP);
    // response = "[";
    // response += result.result;
    // response += "] ";
    response += result.response;
  }
  else
    response += "M;ACK;";
  return response;
}
// #define printbeg Serial.printf("Region
// #define printmid(n) printbeg##n
// #define printend(n)  printmid##n" = %d \n",
// #define printdif(start,end) printend(start)end - start);
#define printdif(start, end) Serial.printf("Region  = %d \n", end - start);
void updateValues(bool skipUpload = false, bool force = false)
{
  // Serial.printf("1\n");
  auto a = millis();
  if (control_variables.flags.disable_sensors_update)
    return;

  int current_time = now();
  if (current_time - control_variables.last_sensor_update >= user_preferences.sensor_update_interval || force)
  {
    // Serial.printf("2\n");
    auto init = millis();
    /*Improvement points: Separate Temperature and A*/
    byte RESTORE_MUX = MUX_VALUE;
    control_variables.last_sensor_update = current_time;
    // @reads = 3, delay between ports = 15 ~212 ms
    Mux.partialSweep(7, MOIST_1, MOIST_2, RAIN_PIN, LDR_PIN, WL_BOTTOM, WL_MIDDLE, WL_EXTRA);
    // Mux.Sweep();
    //  analogRead on the mux fucks the onewire thingy ~358 ms

    // Serial.printf("3\n");
    pinMode(MUX_INPUT, INPUT);
    setMuxPort(DS18b20_PIN);
    delay(Mux.delay_ms);
    DS18.requestTemperatures();
    DS18inner = DS18.getTempCByIndex(0);
    // Serial.printf("4\n");
    if (DS18inner == -127 || DS18inner == -255 || DS18inner == 85)
      Sensors.getSensorById("ds18inner")->setStatus(Offline);
    else
      Sensors.getSensorById("ds18inner")->updateValue(DS18inner);
    DS18Probe = DS18.getTempCByIndex(1); //~15ms
    if (DS18Probe == -127 || DS18Probe == -255 || DS18Probe == 85)
      Sensors.getSensorById("ds18prob")->setStatus(Offline);
    else
      Sensors.getSensorById("ds18prob")->updateValue(DS18Probe);

    // Serial.printf("5\n");
    Sensors.getSensorById("moist")->updateValue((int)map(Mux[MOIST_1], 2700, 1000, 0, 100));
    Sensors.getSensorById("moist2")->updateValue((int)map(Mux[MOIST_2], 2700, 1000, 0, 100));
    auto wl0 = (Mux(user_preferences.WLI.wl_0_pin));
    auto wl1 = (Mux(user_preferences.WLI.wl_1_pin));
    auto wl2 = (Mux(user_preferences.WLI.wl_2_pin));
    // Serial.printf("6\n");
    if (user_preferences.WLI.inverse)
    {
      wl0 = !wl0;
      wl1 = !wl1;
      wl2 = !wl2;
    }
    Sensors.getSensorById("wl0")->updateValue(wl0);
    Sensors.getSensorById("wl1")->updateValue(wl1);
    Sensors.getSensorById("wl2")->updateValue(wl2);
    Sensors.getSensorById("waterlevel")->updateValue(50 * (wl0 + wl1));
    byte wl = wl0 + wl1 * 2 + wl2 * 4;
    // Serial.printf("7\n");
    Pump.updateWaterLevel(wl);
    Sensors.getSensorById("ldr")->updateValue(Mux[LDR_PIN]);
    Sensors.getSensorById("rain")->updateValue(Mux[RAIN_PIN]);
    int vbat = 0;
    for (size_t i = 0; i < 10; i++)
    {
      vbat += analogRead(VBAT_PIN);
    }
    vbat = vbat / 10;
    float res = vbat * (3.30 / 4095.0) * (0.5 / 0.4);
    res = res * 4.9;
    Sensors.getSensorById("vbat")->updateValue(res);
    if (control_variables.flags.LittleFS_mounted)
    {
      float usage = LittleFS.usedBytes();
      usage = usage / LittleFS.totalBytes();
      Sensors.getSensorById("fs")->updateValue(usage * 100);
    }
    else
      Sensors.getSensorById("fs")->setStatus(Offline);

    // Serial.printf("8\n");
    setMuxPort(RESTORE_MUX);
    // Serial.printf("9\n");
    // if (control_variables.flags.LittleFS_mounted)
    // {
    //   float _usedSPIFFS = (float)(LittleFS.usedBytes() / LittleFS.totalBytes());
    //   Sensors.getSensorById("spiffs")->updateValue(_usedSPIFFS);
    //   Sensors.getSensorById("spiffs")->setStatus(Online);
    //   Serial.printf("10a\n");
    // }
    // else
    // {
    //   Sensors.getSensorById("spiffs")->updateValue((float)-1);
    //   Sensors.getSensorById("spiffs")->setStatus(Offline);
    //   Serial.printf("10b\n");
    // }
    // Serial.printf("11\n");
    if (current_time - control_variables.last_sensor_log >= user_preferences.sensor_log_interval && !skipUpload)
    {
      if (control_variables.flags.LittleFS_mounted)
      {
        String msg = "";
        // Creates the file if it doest exist
        if (!LittleFS.exists("/sensors.txt"))
        {
          msg += "Time,TimeTimestamp gmt-03,DS18inner,DS18Probe,WaterLevel,Moisture,DHT_H,DHT_T,LDR";
          FileWrite("/sensors.txt", msg);
        }
        msg = ",";
        msg += current_time;
        msg += ",";
        msg += DS18inner;
        msg += ",";
        msg += DS18Probe;
        msg += ",";
        msg += Water_Level;
        msg += ",";
        msg += Moisture_1;
        msg += ",";
        msg += DHT11_hum;
        msg += ",";
        msg += DHT11_temp;
        msg += ",";
        msg += LDR_VALUE;
        msg += ",";
        FileWrite("/sensors.txt", msg);
      }
      control_variables.last_sensor_log = current_time;
    }
    // Serial.printf("12\n");
    if (current_time - Blynk_telemetry_last >= Blynk_telemetry_interval && !skipUpload && Blynk_telemetry)
    {
      String tmsg = "Z;";
      tmsg += DS18inner;
      tmsg += ";";
      tmsg += DS18Probe;
      tmsg += ";";
      tmsg += wl_half;
      tmsg += ";";
      tmsg += wl_empty;
      tmsg += ";";
      tmsg += Moisture_1;
      tmsg += ";";
      tmsg += WiFi.RSSI();
      tmsg += ";";
      tmsg += DHT11_hum;
      tmsg += ";";
      tmsg += DHT11_temp;
      tmsg += ";";
      tmsg += LDR_VALUE;
      tmsg += ";\n";
      Blynk.virtualWrite(9, tmsg);
      Blynk_telemetry_last = current_time;
    }
    // Serial.printf("13\n");
    if (current_time - Blynk_last_upload >= Blynk_upload_interval && !skipUpload)
    {
      Blynk_last_upload = current_time;
      // Serial.println(analogRead(VBAT_PIN));
      // Serial.println(current_time);
      // Serial.println(current_temp);
      // Serial.println(vbat);
      // Serial.print(Water_Level);
      // Serial.println("%");
      // Serial.println(Rain);
      // Serial.println(sensors_hist.Print(10));
      // sensors_hist.PrintAvgTemp();
      Blynk.virtualWrite(V6, Water_Level);
      Blynk.virtualWrite(V4, DS18inner);
      Blynk.virtualWrite(V5, DS18Probe);
      Blynk.virtualWrite(V7, Moisture_1);
      Blynk.virtualWrite(V11, DHT11_hum);
      Blynk.virtualWrite(V12, DHT11_temp);
      Blynk.virtualWrite(V14, LDR_VALUE);
    }
    // Serial.printf("14\n");
    if (current_time - control_variables.last_MQTT_update >= user_preferences.MQTT_interval && !skipUpload)
    {
      // Sensors.Send_All_MQTT();
      control_variables.last_MQTT_update = current_time;
    }
    auto end = millis() - init;
    // Serial.printf("10\n");
    // Serial.printf("uv = %d ms\n",end);
  }
}

bool getTime()
{
  bool result = false;
  Serial.println("Syncing Time Online");
  HTTPClient http;
  http.begin("http://worldtimeapi.org/api/timezone/America/Bahia.txt"); // HTTP
  int httpCode = http.GET();
  // httpCode will be negative on error
  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    // file found at server
    if (httpCode == HTTP_CODE_OK)
    {
      Serial.printf("[HTTP] OK... code: %d\n", httpCode);
      String payload = http.getString();
      char str[payload.length() + 1];
      strcpy(str, payload.c_str());
      char *pch;
      pch = strtok(str, ":\n");
      int i = 0;
      int raw_offset = 0;
      while (pch != NULL)
      {
        i++;
        if (i == 23)
        {
          raw_offset = atoi(pch);
        }
        if (i == 27)
        {
          setTime(atoi(pch) + raw_offset);
        }
        // printf("%d: %s\n", i, pch);
        pch = strtok(NULL, ":\n");
      }
      String msg = "Time Synced ";
      msg += millis();
      msg += "ms from boot.";
      result = true;
    }
    else
    {
      Serial.printf("[HTTP] Error code: %d\n", httpCode);
    }
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
  return result;
}

void LED_TASK(void *pvParameters)
{
  for (;;)
  {
    LED_Strip.run(strip_leds);
    onBoardLEDs.run(onboard_leds);
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void InitializeSensors(bool reset)
{
  Sensors.reset();
  bool use_default = reset;
  if (!reset)
    use_default = !Sensors.loadSensors();

  if (use_default)
  {
    Sensors.add("ds18inner", "DS18 Inner", SensorDataType::Type_Float, "°C");
    Sensors.add("ds18prob", "DS18 Probe", SensorDataType::Type_Float, "°C");
    Sensors.add("moist", "Moisture Sensor", SensorDataType::Type_Float, "%");
    Sensors.add("moist2", "Moisture Sensor 2", SensorDataType::Type_Float, "%");
    Sensors.add("ldr", "LDR Sensor", SensorDataType::Type_Int, "");
    Sensors.add("dht_hum", "DHT Humidity", SensorDataType::Type_Int, "%");
    Sensors.add("dht_temp", "DHT Temperature", SensorDataType::Type_Float, "°C");
    Sensors.add("waterlevel", "Water Level", SensorDataType::Type_Int, "%");
    Sensors.add("rain", "Rain", SensorDataType::Type_Int);
    Sensors.add("wl0", "Water Level 0", SensorDataType::Type_Int);
    Sensors.add("wl1", "Water Level 1", SensorDataType::Type_Int);
    Sensors.add("wl2", "Water Level 2", SensorDataType::Type_Int);
    Sensors.add("fs", "Storage", SensorDataType::Type_Float, "%");
    Sensors.add("vbat", "Input Voltage", SensorDataType::Type_Float, "V");
    Sensors.saveSensors();
  }
}

// Callback for payload at the mqtt topic
void HiveMQ_Callback(char *topic, byte *payload, unsigned int length)
{

  String incommingMessage = "";
  for (int i = 0; i < length; i++)
    incommingMessage += (char)payload[i];

  // Serial.printf("MQTT::[%s]-->[%s]\n", topic, incommingMessage.c_str());
  //  Serial.printf("MQTT::[%s]-->[%s]\n", topic, incommingMessage.c_str());
  String in_topic = topic;
  // in_topic += topic;


  // Only topic Shell will have valid commands and expect a response
  if (in_topic == "Shell")
  {
    command_result result = handleCommand(incommingMessage, _HiveMQ);
    if (result.result)
    {
      // tag the payload as response;
      result.response = "res::" + result.response;
      // Send the response back into the shell topic
      if (!HiveMQ.publish("Shell", result.response.c_str()))
      {
        Serial.println("Failed to publish response to Shell topic");
      }
    }
  }
  else if (in_topic == "Turing/Color")
  {
    String msg = " [C] - 0x";
    int newColor = strtol(incommingMessage.substring(1).c_str(), NULL, 16);
    msg += String(newColor, HEX);
    Serial.print("Color : ");
    Serial.println(newColor, HEX);
    setBeaconColor(newColor);
    LED_Strip.setMode(strip_leds, LED_Animations::Solid, newColor);
  }
}

void logSensors()
{
  // if (!user_preferences.enable_log);
  // return;

  if (control_variables.flags.LittleFS_mounted)
  {
    String msg = "";
    // Creates the file if it doest exist
    if (!LittleFS.exists("/sensors.txt"))
    {
      msg += "Time,";
      for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
      {
        Sensor *sensor = Sensors.getSensorByIndex(i);
        if (sensor->ID != "" && sensor->Status != Offline)
        {
          msg += sensor->Label;
          msg += ",";
        }
        msg += "\n";
      }
      FileWrite_LittleFS("/sensors.txt", msg);
    }
    msg = "";
    msg += timestampToDateString(now());
    msg += ",";
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      Sensor *sensor = Sensors.getSensorByIndex(i);
      if (sensor->ID != "" && sensor->Status != Offline)
      {
        msg += sensor->getValue();
        msg += ",";
      }
    }
    msg += "\n";
    FileWrite_LittleFS("/sensors.txt", msg);
  }
}

void SendTelemetry()
{
  bool skipUpload = false;
  int current_time = 0;

  if (current_time - control_variables.last_sensor_log >= user_preferences.sensor_log_interval && !skipUpload)
  {
    if (control_variables.flags.LittleFS_mounted)
    {
      String msg = "";
      // Creates the file if it doest exist
      if (!LittleFS.exists("/sensors.txt"))
      {
        msg += "Time,TimeTimestamp gmt-03,DS18inner,DS18Probe,WaterLevel,Moisture,DHT_H,DHT_T,LDR";
        FileWrite("/sensors.txt", msg);
      }
      msg = ",";
      msg += current_time;
      msg += ",";
      msg += DS18inner;
      msg += ",";
      msg += DS18Probe;
      msg += ",";
      msg += Water_Level;
      msg += ",";
      msg += Moisture_1;
      msg += ",";
      msg += DHT11_hum;
      msg += ",";
      msg += DHT11_temp;
      msg += ",";
      msg += LDR_VALUE;
      msg += ",";
      FileWrite("/sensors.txt", msg);
    }
    control_variables.last_sensor_log = current_time;
  }
  // Serial.printf("12\n");
  if (current_time - Blynk_telemetry_last >= Blynk_telemetry_interval && !skipUpload && Blynk_telemetry)
  {
    String tmsg = "Z;";
    tmsg += DS18inner;
    tmsg += ";";
    tmsg += DS18Probe;
    tmsg += ";";
    tmsg += wl_half;
    tmsg += ";";
    tmsg += wl_empty;
    tmsg += ";";
    tmsg += Moisture_1;
    tmsg += ";";
    tmsg += WiFi.RSSI();
    tmsg += ";";
    tmsg += DHT11_hum;
    tmsg += ";";
    tmsg += DHT11_temp;
    tmsg += ";";
    tmsg += LDR_VALUE;
    tmsg += ";\n";
    Blynk.virtualWrite(9, tmsg);
    Blynk_telemetry_last = current_time;
  }
  // Serial.printf("13\n");
  if (current_time - Blynk_last_upload >= Blynk_upload_interval && !skipUpload)
  {
    Blynk_last_upload = current_time;
    // Serial.println(analogRead(VBAT_PIN));
    // Serial.println(current_time);
    // Serial.println(current_temp);
    // Serial.println(vbat);
    // Serial.print(Water_Level);
    // Serial.println("%");
    // Serial.println(Rain);
    // Serial.println(sensors_hist.Print(10));
    // sensors_hist.PrintAvgTemp();
    Blynk.virtualWrite(V6, Water_Level);
    Blynk.virtualWrite(V4, DS18inner);
    Blynk.virtualWrite(V5, DS18Probe);
    Blynk.virtualWrite(V7, Moisture_1);
    Blynk.virtualWrite(V11, DHT11_hum);
    Blynk.virtualWrite(V12, DHT11_temp);
    Blynk.virtualWrite(V14, LDR_VALUE);
  }
}

/// @brief Syncs the time with an online source or sets the time to a specified value.
///
/// This function attempts to sync the system time with an online source if the `time` parameter is set to UINT32_MAX.
/// If syncing online fails or if `time` is not UINT32_MAX, it sets the system time to the value of `time`.
/// After setting the time, it updates various system control variables and checks for the existence of a specific file in the filesystem.
///
/// @param time The UNIX timestamp to set the system time to. If set to UINT32_MAX, the function attempts to sync the time online.
/// @return bool True if the time was successfully synced or set, false otherwise.
bool SyncTime(uint32_t time)
{
  bool result = false;
  // If no argument try to sync time online
  if (time == UINT32_MAX)
  {
    return false;
    result = getTime();
  }
  // if there was an arg setTime to its arg
  if (!result && time != UINT32_MAX)
  {
    setTime(time);
    result = true;
  }

  // If time was successfully synced
  if (result)
  {
    control_variables.boot_time = now() - (millis() - control_variables.boot_time) / 1000;
    Log(formatString("Time Synced. Boot Time was: %s", timestampToDateString(control_variables.boot_time)));
    control_variables.flags.time_synced = true;
    Pump.timeSynced = true; // Changed Logic
    Sensors.timeSynced();
    if (control_variables.flags.LittleFS_mounted)
    {
      // Serial.println(LittleFS.exists("/lastAutoPump.txt"));
      // Serial.println(control_variables.flags.time_synced);
      if (LittleFS.exists("/lastAutoPump.txt"))
      {
        String lastAutoStored = LittleFS.open("/lastAutoPump.txt").readString();
        Serial.print("Last Pump: ");
        Serial.println(lastAutoStored);
        Pump.last_auto_run = atoi(lastAutoStored.c_str());
        Serial.println(Pump.last_auto_run);
      }
    }
  }

  return result;
}

void updateNonCriticalSensors()
{
  // DS18 sensors ->
  pinMode(MUX_INPUT, INPUT);
  setMuxPort(DS18b20_PIN);
  delay(5);
  DS18.requestTemperatures();
  DS18inner = DS18.getTempCByIndex(0);
  if (DS18inner == -127 || DS18inner == -255 || DS18inner == 85)
    Sensors.getSensorById("ds18inner")->setStatus(Offline);
  else
    Sensors.getSensorById("ds18inner")->updateValue(DS18inner);
  DS18Probe = DS18.getTempCByIndex(1); //~15ms
  if (DS18Probe == -127 || DS18Probe == -255 || DS18Probe == 85)
    Sensors.getSensorById("ds18prob")->setStatus(Offline);
  else
    Sensors.getSensorById("ds18prob")->updateValue(DS18Probe);

  // LittleFs->

  if (control_variables.flags.LittleFS_mounted)
  {
    float usage = LittleFS.usedBytes();
    usage = usage / LittleFS.totalBytes();
    Sensors.getSensorById("fs")->updateValue(usage * 100);
  }
  else
    Sensors.getSensorById("fs")->setStatus(Offline);

  // MoistureSensors -> Only have current flowing through the sensor
  SIPO_Write(MOIST_EN0, true);
  // delay(5);
  // Mux.updatePin(MOIST_1);
  // Mux.updatePin(MOIST_2);
  float moist1 = 0;
  float moist2 = 0;
  if (useDirectAdc)
  {
    moist1 = ExtAdc.getVoltage(EA_MOIST0);
    moist2 = ExtAdc.getVoltage(EA_MOIST1);
  }
  else
  {
    moist1 = ExtAdc.getPinVoltge(MOIST_1);
    moist2 = ExtAdc.getPinVoltge(MOIST_2);
  }
  // 2.18 V ~ 2750/4096 -> air
  // 0.80 V ~ 1000/4096 -> full water
  // Serial.printf("%f, %f\n", moist1, moist2);
  constrain(moist1, 0.80, 2.18);
  constrain(moist2, 0.80, 2.18);
  Sensors.getSensorById("moist")->updateValue((float)((2.18 - moist1) * 100 / 1.37));
  Sensors.getSensorById("moist2")->updateValue((float)((2.18 - moist2) * 100 / 1.37));

  // LDR and Rain ->
  Mux.updatePin(LDR_PIN);
  Mux.updatePin(RAIN_PIN);
  Sensors.getSensorById("ldr")->updateValue(Mux[LDR_PIN]);
  Sensors.getSensorById("rain")->updateValue(Mux[RAIN_PIN]);
}

void updateCriticalSensors()
{
  // WaterLevel ->
  Mux.updatePin(user_preferences.WLI.wl_0_pin);
  Mux.updatePin(user_preferences.WLI.wl_1_pin);
  Mux.updatePin(user_preferences.WLI.wl_2_pin);
  auto wl0 = (Mux(user_preferences.WLI.wl_0_pin));
  auto wl1 = (Mux(user_preferences.WLI.wl_1_pin));
  auto wl2 = (Mux(user_preferences.WLI.wl_2_pin));
  if (user_preferences.WLI.inverse)
  {
    wl0 = !wl0;
    wl1 = !wl1;
    wl2 = !wl2;
  }
  Sensors.getSensorById("wl0")->updateValue(wl0);
  Sensors.getSensorById("wl1")->updateValue(wl1);
  Sensors.getSensorById("wl2")->updateValue(wl2);
  Sensors.getSensorById("waterlevel")->updateValue(50 * (wl0 + wl1));
  byte wl = wl0 + wl1 * 2 + wl2 * 4;
  Pump.updateWaterLevel(wl);

  // VBat ->
  // int vbat = 0;
  // for (size_t i = 0; i < 10; i++)
  // {
  //   vbat += analogRead(VBAT_PIN);
  // }
  // vbat = vbat / 10;
  // float res = vbat * (3.30 / 4095.0) * (0.5 / 0.4);
  // res = res * 4.9;

  float vbat = ExtAdc.getVoltage(EA_VBAT) * 4.9;
  Sensors.getSensorById("vbat")->updateValue(vbat);
}

void updateSensors(bool full_update)
{
  byte RESTORE_MUX = MUX_VALUE;

  static int runDivider = 0;
  if (control_variables.flags.disable_sensors_update)
    return;

  // Serial.printf("Update Critical Sensors\n");
  updateCriticalSensors();
  // run critical sensors

  if (runDivider == 10 || full_update)
  {
    // Serial.printf("Update NON-Critical Sensors\n");
    updateNonCriticalSensors();
    runDivider = 0;
    // run non-critical sensors
  }

  runDivider++;
  ///

  setMuxPort(RESTORE_MUX);
}

void updateSensors()
{
  updateSensors(false);
  Sensors.report();
  Sensors.timeoutInactives();
}
bool handleAPIs()
{
  String uri = httpServer.uri();
  // Serial.printf("URI request = %s \n", httpServer.uri());
  if (uri == "/pumpstart")
  {
    Serial.println(uri);
    for (size_t i = 0; i < httpServer.args(); i++)
    {
      Serial.printf("arg[%d] -> name: %s, value: %s\n", i, httpServer.argName(i), httpServer.arg(i));
    }
    bool force = false;
    if (httpServer.hasArg("force"))
      if (httpServer.arg("force") == "true")
        force = true;
    uint16_t time = 5000;
    if (httpServer.hasArg("timer"))
    {
      time = httpServer.arg("timer").toInt();
    }

    // limit user
    if (time > 20000)
    {
      time = 5000;
    }
    if (Pump.is_running)
    {
      httpServer.send(200, "text/plain", "pump already running");
      return true;
    }

    Pump.force_start = force;
    Pump.Start(time, force);
    httpServer.send(200, "text/plain", "ok");
  }
  else if (uri == "/pumpauto")
  {
    if (Pump.is_running)
    {
      httpServer.send(200, "text/plain", "pump already running");
      return true;
    }

    Pump.automation(true);
    httpServer.send(200, "text/plain", "ok");
  }
  else if (uri == "/config")
  {
    bool change = false;
    if (httpServer.hasArg("duration"))
    {
      int dur = httpServer.arg("duration").toInt();
      if (dur != 0)
      {
        user_preferences.auto_pump_duration = dur;
        change = true;
      }
    }
    if (httpServer.hasArg("interval"))
    {
      int interval = httpServer.arg("interval").toInt();
      if (interval != 0)
      {
        user_preferences.auto_pump_interval = interval;
        change = true;
      }
    }
    if (change)
    {
      user_preferences.Save();
      Serial.println("User Pref Saved!");
    }

    httpServer.send(200, "text/plain", user_preferences.Print());
  }
  else if (uri == "/cmd")
  {
    String cmd = "";
    if (httpServer.hasArg("cmd"))
    {
      cmd = httpServer.arg("cmd");
    }
    httpServer.send(200, "text/plain", handleCommand(cmd, _HTTP).response);
  }
  else if (uri == "/pumpdata")
  {
    DynamicJsonDocument doc(512);
    String s = "Last Auto Pump Run: ";
    s += timestampToDateString(Pump.last_auto_run);
    s += ". || [NEXT] : ";
    s += timestampToDateString(Pump.predictNextRun());
    s += ". || [NOW] : ";
    s += timestampToDateString(now());
    doc["pumpinfo"] = s;
    doc["pumpconfirm"] = control_variables.pump_str_http;
    s = "";
    serializeJson(doc, s);
    httpServer.send(200, "text/plain", s);
  }
  else if (uri == "/sipow")
  {
    int8_t pin = -1;
    bool val = false;
    if (httpServer.hasArg("pin"))
    {
      pin = httpServer.arg("pin").toInt();
    }
    if (httpServer.hasArg("val"))
    {
      val = httpServer.arg("val") == "true";
    }
    if (pin >= 0)
    {
      // Serial.printf("URI request = %s \n", httpServer.uri());
      // Serial.printf("Writing pin %d -> %d\n",pin,val);
      SIPO_Write(pin, val);
    }

    DynamicJsonDocument doc(8912);
    doc["muxReadTime"] = Mux.last_sweep;
    for (size_t i = 0; i < MUX_SIZE; i++)
    {
      JsonObject id1 = doc["mux" + String(i)].to<JsonObject>();
      id1["port"] = i;
      id1["analogValue"] = Mux.anResult[i];
      id1["digitalValue"] = Mux.dgResult[i];
      id1["alias"] = getMuxAlias(i);
    }
    for (size_t i = 0; i < SIPO_SIZE; i++)
    {
      JsonObject id1 = doc["sipo" + String(i)].to<JsonObject>();
      id1["pin"] = i;
      id1["value"] = bitRead(SIPO_VALUE, i);
      id1["alias"] = getSipoAlias(i);
    }
    String s;
    serializeJson(doc, s);
    httpServer.send(200, "text/plain", s);
  }
  else if (uri == "/muxdata")
  {

    DynamicJsonDocument doc(8912);
    doc["muxReadTime"] = Mux.last_sweep;
    for (size_t i = 0; i < MUX_SIZE; i++)
    {
      JsonObject id1 = doc["mux" + String(i)].to<JsonObject>();
      id1["port"] = i;
      id1["analogValue"] = Mux.anResult[i];
      id1["digitalValue"] = Mux.dgResult[i];
      id1["alias"] = getMuxAlias(i);
    }
    for (size_t i = 0; i < SIPO_SIZE; i++)
    {
      JsonObject id1 = doc["sipo" + String(i)].to<JsonObject>();
      id1["pin"] = i;
      id1["value"] = bitRead(SIPO_VALUE, i);
      id1["alias"] = getSipoAlias(i);
    }
    String s;
    serializeJson(doc, s);
    httpServer.send(200, "text/plain", s);
  }
  else if (uri == "/changewl")
  {
    int wl = 0;
    String s = "new wl = ";
    if (httpServer.hasArg("wl"))
    {
      wl = httpServer.arg("wl").toInt();
    }

    user_preferences.WLI.configureWl(wl);
    user_preferences.Save();
    s += user_preferences.WLI.value;
    httpServer.send(200, "text/plain", s);
  }
  else if (uri == "/sensordata")
  {
    DynamicJsonDocument doc(8912);
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      doc["pump-status"] = Pump.is_running ? "Running" : "Idle";
      Sensor *s = Sensors.getSensorByIndex(i);
      if (s->ID != "")
      {
        JsonObject id1 = doc[s->ID].to<JsonObject>();
        id1["name"] = s->Label;
        id1["value"] = s->getValue();
        id1["status"] = s->Status;
        id1["unit"] = s->Unit;
      }
    }

    String s;
    serializeJson(doc, s);
    httpServer.send(200, "text/plain", s);
  }
  else if (uri == "/version")
  {
    String s = handleCommand("VERSION", _HTTP).response;
    httpServer.send(200, "text/plain", s);
  }
  else
  {
    return false;
    String message = FPSTR(S_notfound); // @token notfound
    message += FPSTR(S_uri);            // @token uri
    message += httpServer.uri();
    message += FPSTR(S_method); // @token method
    message += (httpServer.method() == HTTP_GET) ? FPSTR(S_GET) : FPSTR(S_POST);
    message += FPSTR(S_args); // @token args
    message += httpServer.args();
    message += F("\n");

    for (uint8_t i = 0; i < httpServer.args(); i++)
    {
      message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
    }
    httpServer.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate")); // @HTTPHEAD send cache
    httpServer.sendHeader(F("Pragma"), F("no-cache"));
    httpServer.sendHeader(F("Expires"), F("-1"));
    httpServer.send(404, FPSTR(HTTP_HEAD_CT2), message);
  }
  return true;
}

// #define STOPATBEGIN
void setup()
{
  control_variables.boot_time = millis();
  // Debug console
  Serial.begin(115200);
#ifdef STOPATBEGIN
  Serial.printf("Type Start to Start: \n");
  bool done = false;
  while (!done)
  {
    if (Serial.available())
    {
      String s = "";
      while (Serial.available() > 0)
      {
        char c = Serial.read();
        if (c != 10 && c != 13)
        {
          s += c;
        }
      }
      Serial.printf("[%s]\n", s.c_str());
      if (s == "start")
      {
        done = !done;
      }
      if (s == "mqtt")
      {
        bool mount = LittleFS.begin();
        bool prefs_loaded = user_preferences.load();
        Serial.printf("fs = % d; result = %d;\n", mount, prefs_loaded);
        WiFi.begin(user_preferences.WifiSSID, user_preferences.WifiPASSWD);

        StartWebServer();

        bool createAp = false; // flag for creating an AP in case we can't connect to wifi
        bool beauty = true;    // Esthetics for Serial.print();
        // WiFi.hostname(DEVICE_NAME);
        //  Try to connect to WiFi
        int StartMillis = millis();
        Serial.println("Connecting to WiFi");
        while (WiFi.status() != WL_CONNECTED && !createAp)
        {
          Serial.print(".");
          delay(100);
        }
        Serial.println("connected");
        Serial.printf("current IP: %s\n", WiFi.localIP().toString().c_str());
        WiFiClient testClient;
        PubSubClient testMqttClient(testClient);
        testMqttClient.setServer("10.10.2.115", 1883);
        testMqttClient.setCallback(HiveMQ_Callback);
        bool res = testMqttClient.connect(DEVICE_NAME, MQTT_USER, MQTT_PASSWD);
        Serial.printf("mqtt res: %d\n", res);
        Serial.printf("Status: %d \n", testMqttClient.state());
        Serial.println(mqttStatusToString(testMqttClient.state()));
        Serial.printf("sub: %d \n", testMqttClient.subscribe("#"));
        while (true)
        {
          testMqttClient.loop();
          EVERY_N_SECONDS(10)
          {
            if (!testMqttClient.connected())
            {
              Serial.printf("current IP: %s\n", WiFi.localIP().toString().c_str());
              bool res = testMqttClient.connect(DEVICE_NAME);
              Serial.printf("mqtt res: %d\n", res);
              Serial.printf("Status: %d \n", testMqttClient.state());
              Serial.println(mqttStatusToString(testMqttClient.state()));
            }
            else
            {
              Serial.println("Writing to mqtt  10s");
              testMqttClient.publish("aa/Debug", "Test");
              Serial.printf("sub: %d \n", testMqttClient.subscribe("#"));
            }
          }
        }
      }
      if (s == "format")
      {
        bool mount = LittleFS.begin(true);
        Serial.printf("LittleFS mount: %d\n", mount);
        if (mount)
        {
          LittleFS.format();
          Serial.println("LittleFS Formated");
          LittleFS.end();
        }
      }
      if (s == "test")
      {
        generateCommandInfoJson(true);

        // Serialize the JSON object for debugging or sending over the network
      }
    }
  }
#endif
  // ArduinoOTA.setHostname(DEVICE_NAME);
  // ArduinoOTA.onStart(startOTA);
  // ArduinoOTA.onEnd(endOTA);
  // ArduinoOTA.onProgress(progressOTA);
  // ArduinoOTA.onError(errorOTA);
  setupPins();
  pinMode(TEST_BUTTON, INPUT_PULLUP);
  FastLED.addLeds<WS2812, WS2812_PIN, GRB>(strip_leds, LED_STRIP_SIZE); // GRB ordering is typical
  // FastLED.addLeds<WS2811, WS2811_PIN, BGR>(beacons_leds, BEACONS_SIZE);
  FastLED.addLeds<WS2812, ONBOARD_RGB_PIN, GRB>(onboard_leds, ONBOARD_RGB_SIZE);

  control_variables.flags.LittleFS_mounted = LittleFS.begin(true);
  Log("Booted");
#ifdef USE_SPIFFS

#endif
  //  Initialize Sensors
  InitializeSensors();
  // Loads user preferences
  bool prefs_loaded = user_preferences.load();
  Serial.print(user_preferences.Print());

  beacons.setBoundries(0, BEACONS_SIZE - 1);
  onBoardLEDs.setBoundries(0, ONBOARD_RGB_SIZE - 1);
  LED_Strip.setBoundries(0, 20 - 1);
  FastLED.setBrightness(0xff);
  ExtAdc.setup();
  // #ifdef PUMPTESTER
  //   return;
  // #endif

  String ssid = user_preferences.WifiSSID;
  Serial.println(user_preferences.WifiSSID.isEmpty());
  for (size_t i = 0; i < ssid.length(); i++)
  {
    Serial.printf("c = %d\n", ssid[i]);
  }
  Serial.printf("%s\n", ssid);
  Serial.println(ssid);

  if (ssid == "" || !prefs_loaded)
  {
    ssid = WIFISSID;
  }

  WiFi.begin(ssid, user_preferences.WifiPASSWD);

  StartWebServer();

  bool createAp = false; // flag for creating an AP in case we can't connect to wifi
  bool beauty = true;    // Esthetics for Serial.print();
  // WiFi.hostname(DEVICE_NAME);
  //  Try to connect to WiFi
  int StartMillis = millis();
  while (WiFi.status() != WL_CONNECTED && !createAp)
  {
    if (millis() % 100 == 0 && beauty)
    {
      Serial.print(".");
      beauty = false;
      onboard_leds[0] = CRGB(0xFF * beauty, 0x99 * beauty, 0);
      onboard_leds[1] = CRGB(0xFF * !beauty, 0x99 * !beauty, 0);
      FastLED.show();
    }
    else if (millis() % 100 != 0)
    {
      beauty = true;
    }

    if (millis() - StartMillis > 14999)
    {
      Serial.printf("\nNetwork '%s' not found.\n", WiFi.SSID().c_str());
      createAp = true;
    }
  }
  if (createAp)
  {
    fill_solid(onboard_leds, 2, 0xFF0000);
    setBeaconColor(CRGB::Red);

    for (size_t i = 0; i < 5; i++)
    {
      FastLED.setBrightness(255);
      FastLED.show();
      delay(200);
      FastLED.setBrightness(0);
      FastLED.show();
      delay(200);
    }
    //   WiFi.enableAP(true);
    //   WiFi.mode(WIFI_AP_STA);
    //   WiFi.softAP(WIFI_SOFTAP_SSID, WIFI_SOFTAP_PASSWD);
    //   // dnsServer.start(53, "*", WiFi.softAPIP());
    //   Serial.println("");
    //   Serial.print("Creating WiFi Ap.\n ---SSID:  ");
    //   Serial.println(WIFI_SOFTAP_SSID);
    //   Serial.print(" --IP address: ");
    //   Serial.println(WiFi.softAPIP());
  }
  else
  {
    auto color = WiFi.status() == WL_CONNECTED ? 0x00FF00 : 0xFF0000;
    fill_solid(onboard_leds, 2, color);
    setBeaconColor(CRGB::Green);

    for (size_t i = 0; i < 5; i++)
    {
      FastLED.setBrightness(255);
      FastLED.show();
      delay(200);
      FastLED.setBrightness(0);
      FastLED.show();
      delay(200);
    }
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal RSSI: ");
    Serial.println(WiFi.RSSI());
    // WiFi.setAutoReconnect(true);

    if (WiFi.status() == WL_CONNECTED)
    {

      ArduinoOTA.begin();

      // Starts the TCP server (used for local debug mostly and NightMare integration)
      // hive_client.setCACert(mqtt_root_ca);
      hive_client.setInsecure();
      HiveMQ.setServer(MQTT_URL, MQTT_PORT);
      HiveMQ.setCallback(HiveMQ_Callback);
      if (HiveMQ.connect(DEVICE_NAME, MQTT_USER, MQTT_PASSWD))
        Serial.println("MQTT Connected");
      else
        Serial.printf("Can't Connect to MQTT Error Code : %d\n", HiveMQ.state());
      HiveMQ.subscribe("Adler/#");
      tcpServer.setMessageHandler(HandleMsg);
      tcpServer.begin();
      Serial.println("TCP Started!");
    }

     SyncTime();
  }
  FastLED.setBrightness(64);
  FastLED.clear();
  // SetOnboardLEDS(CRGB::Red, true);
  setBeaconColor(0xf0f0);
  // Try to connect to blynk. don't use Blynk.begin(), it will block the code if you don't have an in ternet connection
  if (false)
  {

    Blynk.config(auth);
    Blynk.connect();

    // Stars the OTA
    ArduinoOTA.setHostname(DEVICE_NAME);

    ArduinoOTA.onStart(startOTA);
    ArduinoOTA.onEnd(endOTA);
    ArduinoOTA.onProgress(progressOTA);
    ArduinoOTA.onError(errorOTA);
    ArduinoOTA.begin();

    // Starts the TCP server (used for local debug mostly and NightMare integration)
    tcpServer.setMessageHandler(HandleMsg);
    tcpServer.begin();
    /* code */
  }
  // Starts the DS18b20 and the DHT sensor.
  setMuxPort(DS18b20_PIN);
  pinMode(MUX_INPUT, INPUT);
  DS18.begin();
  DS18.setResolution(11);
  DS18.setWaitForConversion(false);
  setMuxPort(0);
  Serial.println("devs");
  Serial.println(DS18.getDeviceCount());
  for (size_t i = 0; i < DS18.getDeviceCount(); i++)
  {
    DeviceAddress aaa;
    DS18.getAddress(aaa, i);
    printAddress(aaa);
    Serial.println();
  }

  // control_variables.time_synced = getTime(); // Sync Time Online
 

  // get initial reading of the values
  updateSensors(true);

  if (Pump.WaterState == 0)
    Pump.tank_0_percent = true;
  if (Pump.WaterState <= 1)
    Pump.tank_50_percent = true;
  if (false)
  {

    if (HiveMQ.connect(DEVICE_NAME, MQTT_USER, MQTT_PASSWD))
      Serial.println("MQTT Connected");
    else
      Serial.printf("Can't Connect to MQTT Error Code : %d\n", HiveMQ.state());
    HiveMQ.subscribe("Turing/Color");
  }
  LED_Strip.setMode(strip_leds, LED_Animations::FadeInOutSolid, CRGB::DeepPink);
  LED_Strip.setInterval(40);
  onBoardLEDs.setMode(onboard_leds, DebugCodes);
  // onBoardLEDs.setMode(onboard_leds, LED_Animations::FadeInOutSolid, CRGB::DarkGreen, CRGB::Pink);
  // onBoardLEDs.setInterval(500);
  FastLED.setBrightness(64);

  xTaskCreatePinnedToCore(
      LED_TASK,    // Function to implement the task /
      "LEDhandle", // Name of the task /
      10000,       //* Stack size in words /
      NULL,        //* Task input parameter /
      32,          //* Priority of the task /
      NULL,        //* Task handle. /
      1);
  // p.progBar(map(WiFi.RSSI(), -90, -40, 0, 100), 60);
  // p.setMode(PatternMode::sinWaves, 0xFF0000, 0x00FF00);
  // default lights
  // beacons.setMode(beacons_leds, LED_Animations::Solid, 0x00f0f0);
  Log("[Finish Setup]");
#ifdef PROTO
  Sensors.suppressError = true;
  Serial.printf(">>>>>>>>>>>END OF SETUP %dms<<<<<<<<\n", millis());
#endif

  Timers.create("Sensors Update", 1, updateSensors);
  Timers.create("Sensors Log", user_preferences.sensor_log_interval, logSensors);
  Mux.delay_ms = 50;
  Mux.num_of_reads = 5;
  Serial.println("End Setup");
}

void loop()
{
  Timers.run();
  // Serial.printf("a\n");
#ifdef PUMPTESTER
  static bool digital_debounce = true;
  if (digitalRead(TEST_BUTTON) == LOW && digital_debounce)
  {
    if (Pump.is_running)
      Pump.Stop();
    else
      Pump.Start(5000);
    while (digitalRead(TEST_BUTTON))
      ;

    delay(10);
    digital_debounce = false;
  }

  if (digitalRead(TEST_BUTTON) == HIGH)
  {
    digital_debounce = true;
  }

#endif
  static uint32_t loop_init = millis();
  loop_init = millis();
  httpServer.handleClient();
  // Serial.printf("b\n");
  auto loop_wmprocess = millis();
  // static bool digital_debounce = true;
  if (digitalRead(TEST_BUTTON) == LOW && digital_debounce)
  {
    // Serial.printf("tt\n");
    Pump.force_start = true;
    Pump.Start(5000, -2);
    delay(10);
    digital_debounce = false;
  }

  if (digitalRead(TEST_BUTTON) == HIGH)
  {
    digital_debounce = true;
  }
  // Serial.printf("c\n");
  // esp_err_t rmt_driver_install(rmt_channel_t channel, size_t rx_buf_size, int intr_alloc_flags);
  ArduinoOTA.handle();
  // Serial.printf("d\n");
  auto loop_OTA = millis();
  // Blynk.run();
  auto loop_Blynk = millis();
  HiveMQ.loop();
  // Serial.printf("e\n");
  auto loop_HIVEMQ = millis();
  //  updateValues();
  // Serial.printf("f\n");
  auto loop_UV = millis();
  tcpServer.handleServer();
  // Serial.printf("g\n");
  auto loop_TCP = millis();
  Pump.automation();
  auto loop_AUTO = millis();

  EVERY_N_SECONDS(60)
  {
    // Serial.printf("l\n");
    if (!control_variables.flags.time_synced)
    {
      SyncTime();
    }
    if (!WiFi.isConnected())
    {
      WiFi.begin(WIFISSID, WIFIPASSWD);
      int StartMillis = millis();
      bool beauty = false;
      Serial.printf("Trying to reconnect to \'%s\'\n", WIFISSID);
      onBoardLEDs.setMode(onboard_leds, WiFi_Blink, CRGB::DarkGreen, CRGB::DarkRed, 500);

      while (WiFi.status() != WL_CONNECTED && millis() - StartMillis < 10000)
      {
        if (millis() % 500 == 0 && beauty)
        {
          Serial.print(".");
          beauty = false;
          FastLED.show();
        }
        else if (millis() % 100 != 0)
        {
          beauty = true;
        }
      }
      if (WiFi.status() != WL_CONNECTED)
      {
        Serial.printf("\nNetwork '%s' not found.\n", WIFISSID);
      }
      onBoardLEDs.goToLastMode(onboard_leds);
    }
    if (!HiveMQ.connected())
    {
      HiveMQ.disconnect();

      if (HiveMQ.connect(DEVICE_NAME, MQTT_USER, MQTT_PASSWD))
      {
        Serial.println("MQTT Connected");
        HiveMQ.subscribe("Adler/#");
      }
      else
        Serial.printf("Can't Connect to MQTT Error Code : %d\n", HiveMQ.state());
    }
  }

  if (!wl_empty && !Pump.tank_0_percent)
  {
    Pump.tank_0_percent = true;
    if (Pump.is_running)
      Pump.Stop();
    Blynk.logEvent("low_water", "Bottom Sensor is not detecting water");
    Log("Bottom Sensor Triggered");
  }

  if (wl_empty > 0 && Pump.tank_0_percent)
  {
    Pump.tank_0_percent = false;
    Blynk.logEvent("water_full", "Bottom Sensor detecting water");
    Log("Bottom Sensor Filled");
  }

  if (!wl_half <= 50 && !Pump.tank_50_percent)
  {
    Pump.tank_50_percent = true;
    Log("Middle Sensor Triggered");
  }

  if (wl_half > 50 && Pump.tank_50_percent)
  {
    Pump.tank_50_percent = false;
    Log("Middle Sensor Filled");
  }

  if (Pump.is_running && millis() >= Pump.end_time)
  {
    Pump.Stop();
  }

#ifdef PROTO
  // TEST AREA
  if (digitalRead(TEST_BUTTON) == LOW && digital_debounce)
  {
    _disable = !_disable;
    FastLED.setBrightness(127 * _disable);
    delay(10);
  }

  if (digitalRead(TEST_BUTTON) == HIGH)
  {
    digital_debounce = true;
  }
#endif
  // Serial.printf("m\n");
  if (Serial.available())
  {
    String s = "";
    while (Serial.available() > 0)
    {
      char c = Serial.read();
      if (c != 10 && c != 13)
      {
        s += c;
      }
    }
    Serial.printf("<<%s\n", s.c_str());
    command_result res = handleCommand(s, _Serial);
    Serial.printf("[%d] - ", res.result);
    Serial.println(res.response);
    if (s == "blink")
    {
      LED_Strip.setMode(strip_leds, LED_Animations::FadeInOutSolid, CRGB::Red, 0, 25, 10);
      delay(10000);
      LED_Strip.setMode(strip_leds, LED_Animations::FadeInOutSolid, CRGB::Blue, 0, 25, 10);
      delay(10000);
      LED_Strip.setMode(strip_leds, LED_Animations::FadeInOutSolid, CRGB::Green, 0, 25, 10);
      delay(10000);
      LED_Strip.setMode(strip_leds, LED_Animations::Solid, CRGB::Orange, 0, 2500, 10);
      delay(10000);
    }
    if (s == "sipo")
    {
      // Serial.printf("SIPO: %d, MUX: %d\n", SIPO_VALUE, MUX_VALUE);
      printSipo();
    }
    if (s == "mux")
    {
      MuxSweeper msw;
      msw.debug = true;
      msw.num_of_reads = 10;
      // msw.delay_ms = 100;
      // msw.partialSweep(7, WL_BOTTOM, WL_MIDDLE, RAIN_PIN, LDR_PIN, WL_EXTRA, MOIST_1, MOIST_2);
      // // msw.Sweep();
      msw.delay_ms = 100;
      // msw.Print();
      msw.Sweep();
      msw.Print();
    }
    if (s == "test")
    {
      Serial.printf("Last Mux Took: %d ms\n", Mux.runtime);
    }
    if (s == "logs")
    {
      command_result res = handleCommand("REQ_FILE /log.txt", _Serial);
      Serial.println(res.response);
    }
    if (s.indexOf("wl") >= 0)
    {
      int c = s.indexOf("wl");
      Pump.updateWaterLevel(s.substring(c + 2).toInt());
    }
  }
  // Serial.printf("n\n");
#ifdef RUNTIME
  auto loop_rest = millis();

  Serial.printf("Loop init [%d]\n", loop_init);
  Serial.printf("Loop wm    [%d] (%d)ms\n", loop_wmprocess, loop_wmprocess - loop_init);
  Serial.printf("Loop OTA   [%d] (%d)ms\n", loop_OTA, loop_OTA - loop_wmprocess);
  Serial.printf("Loop Hive  [%d] (%d)ms\n", loop_HIVEMQ, loop_HIVEMQ - loop_OTA);
  Serial.printf("Loop UV    [%d] (%d)ms\n", loop_UV, loop_UV - loop_HIVEMQ);
  Serial.printf("Loop TCP   [%d] (%d)ms\n", loop_TCP, loop_TCP - loop_UV);
  Serial.printf("Loop Auto  [%d] (%d)ms\n", loop_AUTO, loop_AUTO - loop_TCP);
  Serial.printf("Loop rest  [%d] (%d)ms\n", loop_rest, loop_rest - loop_AUTO);
  Serial.printf("Loop TOTAL (%d)ms\n", loop_rest - loop_init);
#endif
}
