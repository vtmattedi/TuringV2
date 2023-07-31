// Make sure to have these definition on top of your code or else Blynk won't be able to connect.
#include <D:\Projects\PIO\TuringV2\TuringV2\include\Creds\BlynkCred.h>

#define PROTO // Toggle comment to switch between upload to Turing or Jupiter

#ifndef PROTO
#define BLYNK_TEMPLATE_ID MYBLYNKTEMPLATE
#define BLYNK_DEVICE_NAME MYBLYNKDEVICE
#define BLYNK_AUTH_TOKEN MYBLYNKAUTH
#define DEVICE_NAME "Turing"
#endif
#ifdef PROTO
#define BLYNK_TEMPLATE_ID MYBLYNKTEMPLATE_PROTO
#define BLYNK_DEVICE_NAME MYBLYNKDEVICE_PROTO
#define BLYNK_AUTH_TOKEN MYBLYNKAUTH_PROTO
#define DEVICE_NAME "Jupiter"
#endif

#define USE_SPIFFS

#define DEBUG
#define BLYNK_PRINT Serial
bool _disable = false;
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESPmDNS.h>
#include "OneWire.h"
#include "DallasTemperature.h"
#include <D:\Projects\PIO\TuringV2\TuringV2\lib\Time-master\TimeLib.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include "esp_task_wdt.h"
#ifdef USE_SPIFFS
#include <SPIFFS.h>
#include <SPI.h>
#include "FS.h"
#endif
#include <D:\Projects\PIO\TuringV2\TuringV2\include\Creds\WifiCred.h>

#include "DHT.h"
#include <D:\Projects\PIO\TuringV2\TuringV2\lib\NewPing\src\NewPing.h>
#include <D:\Projects\PIO\TuringV2\TuringV2\lib\NightMare TCP\nightmaretcp.h>

#pragma region New Board pins

#define SIPO_CLK 4
#define SIPO_LATCH 17
#define SIPO_DATA 16

#define MUX_S0 17
#define MUX_S1 16
#define MUX_S2 18
#define MUX_S3 19
#define MUX_ENABLE 25
#define MUX_INPUT 33

#define US_TRIG 27
#define US_ECHO 14

#define WS2812_PIN 23
#define WS2811_PIN 26

#define ONBOARD_RGB_PIN 2
#define ONBOARD_RGB_SIZE 3

// SIPO outputs
#define LED_0 0
#define LED_1 1
#define LED_2 2
#define MOIST_EN0 3
#define MOIST_EN1 4
#define RELAY_PIN 5

// MUX Selectors
#define MOIST_1 0
#define MOIST_2 1
#define RAIN_PIN 5
#define DHT_PIN 9
#define DS18b20_PIN 10
#define WL_BOTTOM 13
#define WL_MIDDLE 12
#define LDR_PIN 14

#pragma endregion

#pragma region FastLED

#include <FastLED.h>

#pragma region HIVE MQTT
#include <D:\Projects\PIO\TuringV2\TuringV2\lib\pubsubclient-2.8\src\PubSubClient.h>
#include <D:\Projects\PIO\TuringV2\TuringV2\include\Creds\HiveMQCred.h>
void HiveMQ_Callback(char *topic, byte *payload, unsigned int length);

WiFiClientSecure hive_client;
PubSubClient HiveMQ(hive_client);

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

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

CRGB leds[LED_STRIP_SIZE];
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
        leds[i] = CRGB(baseColor.r * _helperFlag + baseColor2.r * !_helperFlag, baseColor.g * _helperFlag + baseColor2.g * !_helperFlag, baseColor.b * _helperFlag + baseColor2.b * !_helperFlag);
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
      hsv.s = (int)(delta)*255 / xMax;
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
    setMode(led_array, old_animation, old_baseColor, old_baseColor2, _interval);
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

// #define DHT_PIN 17
#define DHT_TYPE DHT11

// #define RAIN_SENSOR 33
// #define MOISTURE_SENSOR 33
// #define LDR_PIN 32
// #define PUMP_RELAY 25
// #define EMPTY_WATER_LEVEL 18
// #define HALF_WATER_LEVEL 19
// #define DS18_BUS 16
// #define DS18_READS 10

#ifdef PROTO
// TEST BUTTON
#define TEST_BUTTON 0
bool digital_debounce = false;
#endif

#define MAX_HISTORY_SIZE 100

OneWire oneWire(MUX_INPUT);       // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature DS18(&oneWire); // Pass our oneWire reference to Dallas Temperature.

DHT dht(MUX_INPUT, DHT_TYPE);

#pragma region TCP server

NightMareTCPServer tcpServer(100);

#pragma endregion

struct Internals
{
  uint sync_light = 0;               // No Idea what does this do
  bool auto_ldr_overriden = false;   // Flag to check if the Auto LDr has been overriden and therefore should be paused and resumed
  uint8_t aut_ldr_resume_time = 0;   // Time to resume the Auto dimming based on the LDR
  bool turn_off_lights_flag = false; // Flag to turn off the lights automaticaly
  // Old Variables
  bool SendWifiResults = false; // Flag to send the results from the wifi scan once its done;
  bool oldWifiStatus = false;   // Flag to not keep wirting to the digital pin ????
  bool is_time_synced = false;  // True if time was synced false if not.
  uint last_sensor_update = 0;  // unix timestamp in seconds of last sensor update.
  uint last_sensor_log = 0;
  uint last_MQTT_update = 0;                             // unix timestamp in seconds of last sensor log.
  bool is_SD_mounted = false, is_SPIFFS_mounted = false; // Flags to see if mount of SD and  SPIFFS were sucessful
  bool time_synced = false;                              // Flags if time have been synced
  bool disable_sensors_update = false;                   // Flags wheater or not we want to upadate sensors values
};

Internals control_variables;

struct Pump_Controller
{
  uint start_time = 0;                 // Millis() of when did the pump started
  bool is_running = false;             // is the pump currently running
  bool programmed_end = false;         // Wheater or not the pump is supposed to stop based on the timer
  uint end_time = 0;                   // Millis() of when the pump is supposed to end
  String last_run_string = "Never On"; // String for the Blynk V10
  uint last_auto_run = 0;              // Time stamp of last time that we auto ran the pump
  bool force_start = false;            // forces the pump to run despites the Water Level sensors
  bool tank_0_percent = false;         // Prevents loops of low water warnings
  bool tank_50_percent = false;        // Prevents loops of half water warnings

#define PUMP_HALF_TO_0_TIME 147707
  byte WaterState = 0;          // The State of the water sensors. overusage of memory this is equal to wl_empty + wl_half
  uint run_time_from_50 = 0;    // Amount of time the pump was on from 50% to 0%
  uint run_time_from_start = 0; // Amount of time the pump was on since manual reset
};
Pump_Controller Pump;

int trigPin = 26;
int echoPin = 27;
int lastState = 0;

NewPing sonar(trigPin, echoPin); // NewPing setup of pins and maximum distance.
byte SIPO_VALUE = 0;
byte MUX_CONTROL[4] = {MUX_S0, MUX_S1, MUX_S2, MUX_S3};
byte MUX_VALUE = 0;
void SIPO_WriteBYTE(byte, bool);
void SIPO_Write(byte, bool, bool);
/**
 * Turns on the mux and selects a port to be used.
 * @param port the output pin to be changed. [0-15]
 */
void setMuxPort(byte port, bool ignoresipo = false)
{
  if (port > 15)
  {
    Serial.printf("No port:%d on mux, max is 15.\n");
    return;
  }
  // Sets the mux selector to the port
  MUX_VALUE = port;
  // if (!ignoresipo)
  //   SIPO_WriteBYTE((SIPO_VALUE / 0xF0) * 0xF0 + port, true);

  for (int i = 0; i < 4; i++)
  {
    bool bitValue = port & (1 << i); // Check the value of each bit in the byte
    digitalWrite(MUX_CONTROL[i], bitValue);
  }

  // Enables the mux
  digitalWrite(MUX_ENABLE, !HIGH);
}
/**
 * Writes a bit to the SIPO Shifter. turns off the MUX.
 * @param pin the output pin to be changed. [0-7]
 * @param value the value to be shifted into that pin.
 * @param skipmux skips the mux selection after setting the SIPO
 */
void SIPO_Write(byte pin, bool value, bool skipmux = false)
{
  if (pin > 7)
  {
    Serial.printf("No pin:%d on the SIPO shifter, max is 8.\n", pin);
    return;
  }
  // Disables MUX since SIPO shares DATA and LATCH with MUX Selector pins
  digitalWrite(MUX_ENABLE, LOW);
  SIPO_VALUE = (SIPO_VALUE & ~(1 << pin)) | (value << pin);
  // Serial.print("0b");
  // Serial.println(SIPO_VALUE, BIN);
  digitalWrite(SIPO_LATCH, LOW);
  shiftOut(SIPO_DATA, SIPO_CLK, MSBFIRST, SIPO_VALUE);
  digitalWrite(SIPO_LATCH, HIGH);
  // restore MUX
  if (!skipmux)
  {
    digitalWrite(MUX_ENABLE, !HIGH);
    setMuxPort(MUX_VALUE);
  }
}
/**
 * Writes a byte to the SIPO Shifter. turns off the MUX.
 * @param newValue The byte to be shifted in.
 * @param skipmux Wheather or not to skip messing with mux.
 */
void SIPO_WriteBYTE(byte newValue, bool skipmux = false)
{
  if (!skipmux)
    // Disables MUX since SIPO shares DATA and LATCH with MUX Selector pins
    digitalWrite(MUX_ENABLE, !LOW);
  digitalWrite(SIPO_LATCH, LOW);
  shiftOut(SIPO_DATA, SIPO_CLK, MSBFIRST, newValue);
  digitalWrite(SIPO_LATCH, HIGH);
  // // restore MUX
  if (!skipmux)
  {
    digitalWrite(MUX_ENABLE, !HIGH);
    setMuxPort(MUX_VALUE);
  }
  SIPO_VALUE = newValue;
}

// Writes the content to the speficied file
// @param FileName The file to be written @param Message Content to be Written @param useSPIFFS True for SPIFFS and false for SD @param timestamp Add timestamp at the beginning of the content @param newLine Write with new line at the end
void FileWrite(String FileName, String Message, bool useSPIFFS = true, bool timestamp = true, bool newLine = true)
{
  File logfile;
  String Msg = "";

  if (timestamp)
  {
    Msg += dayShortStr(dayOfWeek(now()));
    Msg += " ";
    if (day() < 10)
      Msg += "0";
    Msg += day();
    Msg += "/";
    if (month() < 10)
      Msg += "0";
    Msg += month();
    Msg += " ";
    if (hour() < 10)
      Msg += "0";
    Msg += hour();
    Msg += ":";
    if (minute() < 10)
      Msg += "0";
    Msg += minute();
    Msg += " - ";
  }

  Msg += Message;

  if (control_variables.is_SPIFFS_mounted && useSPIFFS)
  {
    double usage = SPIFFS.usedBytes() / SPIFFS.totalBytes();
    if (usage > 90)
    {
      Blynk.logEvent("spiffs_full", String(usage));
      Serial.println("SPIFFS is full.");
    }
    logfile = SPIFFS.open(FileName, "a", true);
    if (newLine)
      logfile.println(Msg);
    else
      logfile.print(Msg);
    logfile.close();
  }
  else if (control_variables.is_SD_mounted && !useSPIFFS)
  {
    Serial.println("not implemented");
  }
}

/** Returns the sum of all characteres in the string
 @param string_to_test String to be summed */
int checksum(String string_to_test)
{
  int sum = 0;
  for (size_t i = 0; i < string_to_test.length(); i++)
  {
    sum += string_to_test[i];
  }
  return sum;
}

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

// not used
uint16_t Rain_level = 0;
int light = 0;
int vbat = 0;
u_int last_time_pump_was_on = 0;

enum SensorStatus
{
  Online = 1,
  Offline = 0,
  Unknown = -1
};

enum SensorDataType
{
  Type_Int = 0,
  Type_Float = 1,
  Type_String = 2
};

enum SensorType
{
  Unknown_SENSOR = -1,
  DS1820b_SENSOR = 0,
  DHT_SENSOR = 1,
  ANALOG_READ = 2,
};

struct Sensor
{
private:
  int8_t _SIPO_ENABLE = 0;
  int8_t _MUX_INDEX = 0;
  int8_t _pin_one = 0;
  int8_t _pin_two = 0;
  int8_t _helper_int8 = 0;
  SensorType _sensorType = SensorType::Unknown_SENSOR;

public:
  SensorStatus Status = Unknown;
  SensorDataType Type = Type_String;
  String ID = "";
  String Label = "";
  int modified_timestamp = 0;
  int timeout_seconds = 5 * 60;

  float Value_Float = 0;
  int Value_Int = 0;
  String Value_String = "";
  bool debug = false;
  bool report = false;

  void Setup(String iD, String label, SensorDataType type)
  {
    ID = iD;
    Label = label;
    Type = type;
  }

  void setStatus(SensorStatus newStatus)
  {
    Status = newStatus;
  }

  void setDebug(bool newDebug)
  {
    debug = newDebug;
    Serial.printf("Debug Flag for [%s] is now %s\n", ID, debug ? "true" : "false");
  }

  void UpdateValue(int newValue)
  {
    if (debug)
      Serial.printf("[%s] - new Int: %d. currentValue: %d.\n", ID, newValue, Value_Int);
    Value_Int = newValue;
    modified_timestamp = now();
  }
  void UpdateValue(float newValue)
  {
    if (isnan(newValue))
    {
      Status = Offline;
    }
    else
    {
      if (debug)
        Serial.printf("[%s] - new Float: %f. currentValue: %f.\n", ID, newValue, Value_Float);
      Status = Online;
      Value_Float = newValue;
      modified_timestamp = now();
    }
  }
  void UpdateValue(String newValue)
  {
    if (debug)
      Serial.printf("[%s] - new String: %s. currentValue: %s.\n", ID, newValue, Value_String);

    if (Type == SensorDataType::Type_String)
      Value_String = newValue;
    else if (Type == SensorDataType::Type_Int)
    {
      int new_int = atoi(newValue.c_str());
      if (new_int != 0 || newValue == "0")
      {
        if (Status != SensorStatus::Online)
          Status = SensorStatus::Online;
      }
      else
      {
        Status = SensorStatus::Offline;
        return;
      }
    }
    else if (Type == SensorDataType::Type_Float)
    {
      int new_int = atof(newValue.c_str());
      if (new_int != 0 || newValue == "0.00")
      {
        if (Status != SensorStatus::Online)
          Status = SensorStatus::Online;
      }
    }
    modified_timestamp = now();
  }
  String getValue()
  {
    String msg = "";
    if (Type == Type_Int)
    {
      msg += Value_Int;
    }
    if (Type == Type_Float)
    {
      msg += Value_Float;
    }
    if (Type == Type_String)
    {
      msg += Value_String;
    }
    return msg;
  }
  String Print()
  {
    String msg = "";
    msg += ID;
    msg += " - [";
    msg += Label;
    msg += "] ";
    if (Status == Online)
      msg += "Online ";
    else if (Status == Offline)
      msg += "Offline ";
    else if (Status == Unknown)
      msg += "Unknown ";
    msg += "| ";
    msg += getValue();

    return msg;
  }
  void Verify_timestamp()
  {
    if (Status == SensorStatus::Online && now() - modified_timestamp >= timeout_seconds)
      Status = SensorStatus::Offline;
  }
  void Report()
  {
    if (report)
    {
      String reportValue = Value_String;
      if (Type == Type_Float)
        reportValue = String(Value_Float);
      if (Type == Type_Int)
        reportValue = String(Value_Int);
      Serial.printf("[%s] value: %s age: %d secs,\n", Label, reportValue, now() - modified_timestamp);
    }
  }
};

#define SENSOR_ARRAY_SIZE 12
Sensor *Sensorlist[SENSOR_ARRAY_SIZE];

struct SensorList
{
  bool suppressError = false;
#define SENSORS_FILENAME "/sensorsConfigs.cfg"
#define SENSORS_OBJ_SEPARATOR ";"
#define SENSORS_DETAIL_SEPARATOR "::"
#define PREFS_CHECKSUM_SEPARATOR "::"

  Sensor *getSensorById(String SensorID)
  {
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID == SensorID)
        return Sensorlist[i];
    }

    if (!suppressError)
      Serial.printf("No Sensor with ID:%s\n", SensorID);
    return Sensorlist[SENSOR_ARRAY_SIZE - 1];
  }

  Sensor *getSensorByLabel(String SensorLabel)
  {
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID == SensorLabel)
        return Sensorlist[i];
    }
    if (!suppressError)
      Serial.printf("No Sensor with Label:%s\n", SensorLabel);
    return Sensorlist[SENSOR_ARRAY_SIZE - 1];
  }

  void PrintAll()
  {
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID != "")
        Serial.println(Sensorlist[i]->Print());
    }
  }

  bool Add(String iD, String label, SensorDataType type)
  {
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID == iD)
      {
        Serial.printf("A sensor with ID %s already exists\n", iD);
        return false;
      }
    }
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID == "")
      {
        Sensorlist[i]->Setup(iD, label, type);
        Serial.printf("Sensor %s Created\n", iD);
        return true;
      }
    }
    Serial.println("SensorList is at Max capacity");
    return false;
  }

  void reset()
  {
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      Sensorlist[i] = new Sensor();
    }
  }

  String PrintString()
  {
    String msg = "";
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID != "")
      {
        msg += Sensorlist[i]->Print();
        msg += "\n";
      }
    }
    return msg;
  }

  String SerializeValues()
  {
    String msg = "";
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID != "")
      {
        msg += Sensorlist[i]->ID;
        msg += ";";
        msg += Sensorlist[i]->getValue();
        msg += ";";
      }
    }
    return msg;
  }

  bool loadSensors()
  {
    Serial.print("Loading ");
    Serial.println(SENSORS_FILENAME);
    if (!SPIFFS.exists(SENSORS_FILENAME))
    {
      Serial.println("'/sensorsConfigs.cfg' not found");
      return false;
    }

    String config = SPIFFS.open(SENSORS_FILENAME).readString();

    if (atoi(config.substring(0, config.indexOf(PREFS_CHECKSUM_SEPARATOR)).c_str()) != checksum(config.substring(config.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR))))
    {
      Serial.printf("Error loading user preferences: checksum did not match, possibly corrputed file.\n");
      Serial.printf("debug: loadstr = '%s', loadstrnum = '%s', checkstr = '%s', checksum = '%d'\n", config.c_str(), config.substring(0, config.indexOf(PREFS_CHECKSUM_SEPARATOR)).c_str(), config.substring(config.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR)).c_str(), checksum(config.substring(config.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR))));
      return false;
    }

    config = config.substring(config.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR));

    int last_sensor_index = 0;
    int sensor_index = 0;
    String sensor_string = "";

    for (size_t i = 0; sensor_index >= 0; i++)
    {
      sensor_string = "";
      sensor_index = config.indexOf(SENSORS_OBJ_SEPARATOR, last_sensor_index + 1);

      if (last_sensor_index > 0)
        last_sensor_index += strlen(SENSORS_OBJ_SEPARATOR);

      if (sensor_index == -1)
        sensor_string = config.substring(last_sensor_index);
      else if (sensor_index < config.length())
        sensor_string = config.substring(last_sensor_index, sensor_index);

      int last_detais_index = 0;
      int details_index = 0;
      String current_detail = "";
      String sensor_ID = "";
      String sensor_label = "";
      SensorDataType sensor_Type = Type_String;

      for (size_t j = 0; details_index >= 0; j++)
      {
        current_detail = "*";
        details_index = sensor_string.indexOf(SENSORS_DETAIL_SEPARATOR, last_detais_index + 1);
        if (last_detais_index > 0)
          last_detais_index += strlen(SENSORS_DETAIL_SEPARATOR);

        if (details_index == -1)
          current_detail = sensor_string.substring(last_detais_index);
        else if (details_index < sensor_string.length())
          current_detail = sensor_string.substring(last_detais_index, details_index);
        // Serial.printf("i: %d j: %d s: %s\n",i,j,current_detail.c_str()); //debug
        last_detais_index = details_index;

        if (j == 0)
        {
          sensor_ID = current_detail;
        }
        else if (j == 1)
        {
          sensor_label = current_detail;
        }
        else if (j == 2)
        {
          if (current_detail == "0")
            sensor_Type = SensorDataType::Type_Int;
          else if (current_detail == "1")
            sensor_Type = SensorDataType::Type_Float;
        }
      }

      if (sensor_string.length() > 0)
      {
        // Serial.printf("%s || %s || %d \n", sensor_ID.c_str(), sensor_label.c_str(), sensor_Type);
        Add(sensor_ID, sensor_label, sensor_Type);
      }
      last_sensor_index = sensor_index;
    }

    return true;
  }

  void saveSensors()
  {
    if (SPIFFS.exists(SENSORS_FILENAME))
      SPIFFS.remove(SENSORS_FILENAME);

    String msg = "";
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID != "")
      {
        msg += Sensorlist[i]->ID;
        msg += SENSORS_DETAIL_SEPARATOR;
        msg += Sensorlist[i]->Label;
        msg += SENSORS_DETAIL_SEPARATOR;
        msg += Sensorlist[i]->Type;
        msg += SENSORS_OBJ_SEPARATOR;
      }
    }
    String file = "";
    file += checksum(msg);
    file += PREFS_CHECKSUM_SEPARATOR;
    file += msg;

    FileWrite(SENSORS_FILENAME, file, true, false, false);
  }

  bool Send_to_MQTT(Sensor *sensor_to_send)
  {
    if (sensor_to_send->ID == "")
      return false;

    if (sensor_to_send->Status == Offline)
      return false;

    String topic = DEVICE_NAME;
    topic += "/";
    topic += sensor_to_send->ID;
    MQTT_Send(topic, sensor_to_send->getValue());
    return true;
  }

  bool Send_All_MQTT()
  {
    bool flag = false;
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Send_to_MQTT(Sensorlist[i]))
        flag = true;
    }
    return flag;
  }

  void Timeout_Inactives()
  {
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID != "")
      {
        Sensorlist[i]->Verify_timestamp();
      }
    }
  }

  void report()
  {
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID != "")
      {
        Sensorlist[i]->Report();
      }
    }
  }
};

SensorList Sensors;

#pragma endregion

String listAllFiles(String dir = "/", bool SD_card = false, byte treesize = 0)
{
  String response = "";
  /// SPIFFS mode
  if (!SD_card)
  {
    // List all available files (if any) in the SPI Flash File System
    if (!control_variables.is_SPIFFS_mounted)
    {
      response += "SPIFFS not Mount";
      return response;
    }
    response += "Used Bytes: ";
    response += SPIFFS.usedBytes();
    response += "-----Total Bytes: ";
    response += SPIFFS.totalBytes();
    response += "-----Used: ";
    response += map(SPIFFS.usedBytes(), 0, SPIFFS.totalBytes(), 0, 100);
    response += "%\n";
    response += "Listing files in: ";
    response += dir;
    response += "\n";
    fs::File root = SPIFFS.open(dir);
    fs::File file = root.openNextFile();
    while (file)
    {
      for (size_t i = 0; i < treesize; i++)
      {
        response += "--";
      }
      response += "FILE: ";
      response += file.name();
      response += " size: ";
      response += (float)file.size() / 1024;
      response += "Kb\n";
      if (file.isDirectory())
        response += listAllFiles(file.name(), false, treesize + 1);
      file = root.openNextFile();
    }
    root.close();
    file.close();
  }

  return response;
}

struct preferences
{
  // prefered time to water the garden; -1 = disabled.
  int8_t prefered_pump_hour = 19;
  // 172800; Time in seconds between each pump activation time 172800 sec = 48 hours
  int auto_pump_interval = 172800;
  // Time in ms for the pump to stay on each time it is automatically triggered
  uint16_t auto_pump_duration = 10000;
  // Time to turnoff lights when using AutoBrightnessLDR; -1 = disabled.
  int8_t turn_off_lights_time = -1;
  //  Control the lights brightness using the LDR
  bool AutoLDR = true;
  // Sets the interval for refreshing the sensor data in seconds.
  uint16_t sensor_update_interval = 1;
  // Sets the interval for logging the sensor data in seconds.
  uint16_t sensor_log_interval = 3600;
  // flag to log the files or not
  bool enable_log = false;

  byte MQTT_interval = 1;

  /* Make sure to save & load nescessary preferences
   *
   */

#define PREFS_SEPARATOR ";"
#define PREFS_FILENAME "/prefs.cfg"
#define PREFS_CHECKSUM_SEPARATOR "::"
  // Save user preferences to file [sync order with load]
  void Save(String FileName = PREFS_FILENAME)
  {
    String prefs = "";
    prefs += prefered_pump_hour;
    prefs += PREFS_SEPARATOR;
    prefs += auto_pump_interval;
    prefs += PREFS_SEPARATOR;
    prefs += auto_pump_duration;
    prefs += PREFS_SEPARATOR;
    prefs += turn_off_lights_time;
    prefs += PREFS_SEPARATOR;
    prefs += AutoLDR;
    prefs += PREFS_SEPARATOR;
    prefs += sensor_update_interval;
    prefs += PREFS_SEPARATOR;
    prefs += sensor_log_interval;
    prefs += PREFS_SEPARATOR;
    prefs += enable_log;
    prefs += PREFS_SEPARATOR;

    String msg = "";
    msg += checksum(prefs);
    msg += PREFS_CHECKSUM_SEPARATOR;
    msg += prefs;

    if (SPIFFS.exists(FileName))
      SPIFFS.remove(FileName);

    FileWrite(FileName, msg, true, false, false);
  }

  /** Loads user preferences from file [sync order with save]
   @param FileName the name of the file to be loaded */
  void load(String FileName = PREFS_FILENAME)
  {
    if (!SPIFFS.exists(FileName))
    {
      Serial.printf("Error loading user preferences: file '%s' not found \n", FileName.c_str());
      return;
    }
    String load_string = SPIFFS.open(FileName).readString();
    if (atoi(load_string.substring(0, load_string.indexOf(PREFS_CHECKSUM_SEPARATOR)).c_str()) != checksum(load_string.substring(load_string.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR))))
    {
      Serial.printf("Error loading user preferences: checksum did not match, possibly corrputed file.\n");
      Serial.printf("debug: loadstr = '%s', loadstrnum = '%s', checkstr = '%s', checksum = '%d'\n", load_string.c_str(), load_string.substring(0, load_string.indexOf(PREFS_CHECKSUM_SEPARATOR)).c_str(), load_string.substring(load_string.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR)).c_str(), (load_string.substring(load_string.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR))));
      return;
    }
    load_string = load_string.substring(load_string.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR));
    int separator_index = 0;
    int last_separator_index = 0;
    String current_substring = "";

    for (size_t i = 0; separator_index >= 0; i++)
    {
      current_substring = "";
      separator_index = load_string.indexOf(PREFS_SEPARATOR, last_separator_index + 1);

      if (last_separator_index > 0)
        last_separator_index += strlen(PREFS_SEPARATOR);

      if (separator_index == -1)
        current_substring = load_string.substring(last_separator_index);
      else if (separator_index < load_string.length())
        current_substring = load_string.substring(last_separator_index, separator_index);

      last_separator_index = separator_index;

      if (i == 0)
        prefered_pump_hour = atoi(current_substring.c_str());
      else if (i == 1)
        auto_pump_interval = atoi(current_substring.c_str());
      else if (i == 2)
        auto_pump_duration = atoi(current_substring.c_str());
      else if (i == 3)
        turn_off_lights_time = atoi(current_substring.c_str());
      else if (i == 4)
        AutoLDR = atoi(current_substring.c_str());
      else if (i == 5)
        sensor_update_interval = atoi(current_substring.c_str());
      else if (i == 6)
        sensor_log_interval = atoi(current_substring.c_str());
      else if (i == 7)
        enable_log = atoi(current_substring.c_str());
    }
    Serial.println("User Preferences Loaded");
  }

  /** General function to set value of the preferences
   * @param key the preference variable to be changed @param value the value to be assigned to the preference @return wheater or not key matched any available preferences
   */
  bool SetValue(String key, int value)
  {
    key.toLowerCase();
    if (key == "pump_hour")
      prefered_pump_hour = value;
    else if (key == "auto_pump_interval")
      auto_pump_interval = value;
    else if (key == "auto_pump_duration")
      auto_pump_duration = value;
    else if (key == "turn_off_lights")
      turn_off_lights_time = value;
    else if (key == "ldr")
      AutoLDR = value;
    else if (key == "update_interval")
      AutoLDR = value;
    else if (key == "log_interval")
      AutoLDR = value;
    else if (key == "log")
      AutoLDR = value;
    else
      return false;

    Save();
    return true;
  }

  // Prints Current Values to serial
  String Print()
  {
    String response = "";
    response += "p. hour [pump_hour]: ";
    response += prefered_pump_hour;
    response += " | ap. interval  [auto_pump_interval]:";
    response += auto_pump_interval;
    response += " | ap. duration  [auto_pump_duration]:";
    response += auto_pump_duration;
    response += " | turn off time [turn_off_lights]:";
    response += turn_off_lights_time;
    response += " | Auto LDR [ldr]:";
    response += AutoLDR;
    response += " | Blynk interval [update_interval]:";
    response += sensor_update_interval;
    response += " | Log interval [log_interval]:";
    response += sensor_log_interval;
    response += " | Enable log [log]:";
    response += sensor_log_interval;
    response += "\n";
    return response;
  }
};

preferences user_preferences;

struct command_result
{
  bool result = false;
  String response = "";
};

command_result handleCommand(String message, char *delimiter = " ", String extra_arg = "")
{
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
  if (command == "Help" || command == "help" || command == "H" || command == "h")
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
    response += "   -DHT11 BUS             @ Port 9\n";
    response += "   -LDR                   @ Port 14\n";
    response += "   -MOISTURE SENSOR 0     @ Port 0\n";
    response += "   -MOISTURE SENSOR 1     @ Port 1\n";
    response += "   -MOISTURE SENSOR 2     @ Port 2\n";
    response += "   -MOISTURE SENSOR 3     @ Port 3\n";
    response += "   -WATER LEVEL SENSOR 0  @ Port 13\n";
    response += "   -WATER LEVEL SENSOR 1  @ Port 12\n";
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
    if (control_variables.is_SPIFFS_mounted)
    {
      if (SPIFFS.exists(args[0]))
      {
        response += SPIFFS.open(args[0]).readString();
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
      response = "SPIFFS not mount";
      resolve = false;
    }
  }
  else if (command == "REQ_FILES")
  {
    if (control_variables.is_SPIFFS_mounted)
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
    response = Sensors.PrintString();
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
    if (SPIFFS.exists("/lastAutoPumpReadable.txt"))
      response += SPIFFS.open("/lastAutoPumpReadable.txt").readString();
    response += " || Next: ";
    response += Pump.last_auto_run + user_preferences.auto_pump_interval;
    response += " || Pump Run Time: ";
    response += Pump.run_time_from_50;
    // response += auto_pump_time_ajusted(now());
  }
  else if (command == "PUMP_TEST")
  {
    response += "last_pump:";
    response += Pump.last_auto_run;
    response += " | now:";
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
      Serial.printf("ADDR: [%d] 0x",i);
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
    Serial.printf("Test Result [0]:%f [1]:%f\n", res,res2);
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
    response = Sensors.SerializeValues();
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

    Sensors.getSensorByLabel(args[0])->report = !Sensors.getSensorByLabel(args[0])->report;
    resolve = true;
    response = "reporting: ";
    response += args[0];
    response += " is now: ";
    response += Sensors.getSensorByLabel(args[0])->report ? "enable" : "disable";
  }
  else if (command == "TOGGLEUPDATES")
  {
    control_variables.disable_sensors_update = !control_variables.disable_sensors_update;
    response += "Sensor updates are now: ";
    response += control_variables.disable_sensors_update ? "disable" : "enable";
    resolve = true;
  }
  else
    resolve = false;

  result.response = response;
  result.result = resolve;

  Serial.printf("message = %s | command = %s | args[0] = %s | args[1] = %s | args[2] = %s |\n", message.c_str(), command.c_str(), args[0].c_str(), args[1].c_str(), args[2].c_str());

  return result;
}

/*Estimates the Water Level bas0ed on how much time the pump have been on.
 *@param Pump_runtime The amount of time (in ms) that the pump ran for.
 *@param forceState if > forceState will force the level to 50 or 0 % the first time one of the water level sensors have been triggred
 *@param forceUsedPump if > 0 this will set the amount of run time to it
 */
void CalcWaterLevel(int Pump_runtime, int forceState = -1, int forceUsedPump = -1)
{
  // Forces a Water_Level State;
  if (forceState >= 0)
  {
    Water_Level = forceState;
    Pump.run_time_from_50 = 0;
    return;
  }
  // Forces pump.UsedPumpTime
  if (forceUsedPump > 0)
  {
    Pump.run_time_from_50 = forceUsedPump;
  }
  // Since its leaking only supported for 50 to 0;
  if (Pump_runtime > 0 && Pump.WaterState == 1)
  {
    Pump.run_time_from_50 += Pump_runtime;
    if (SPIFFS.exists("/water.txt"))
      SPIFFS.remove("/water.txt");
    FileWrite("/water.txt", String(Pump.run_time_from_50), true, false);
  }
  Water_Level = 50 * Pump.WaterState - round((double)Pump.run_time_from_50 / PUMP_HALF_TO_0_TIME * 50);
  Sensors.getSensorById("waterlevel")->UpdateValue(Water_Level);
  Blynk.virtualWrite(V6, Water_Level);
  //  Serial.printf("{%d}{%d}{%d} --- %d - %d - %d\n",Pump_runtime,forceState,forceUsedPump,pump.WaterState,Water_Level, pump.UsedPumpTime);
}

/*Starts the pump
 *@param time The amount of time (in ms) to leave the pump running.
 *t < 0 = runs until StopPump is Called.
 */
void StartPump(int time = -1)
{
  if (Pump.is_running || time == 0)
  {
    Blynk.virtualWrite(V9, "Pump already running\n");
    return;
  }
  if (Water_Level == 0 && !Pump.force_start)
  {
    Blynk.virtualWrite(V9, "Water Low, Check reservoir or wiring.\n");
    return;
  }
  Pump.start_time = millis();
  // OLD:// digitalWrite(PUMP_RELAY, !HIGH);
  SIPO_Write(RELAY_PIN, !HIGH);
  Serial.println("PUMP TURNED ON");
  Pump.is_running = true;
  if (time > 0)
  {
    Pump.end_time = Pump.start_time + time;
    String Msg = "PUMP Starting [";
    Msg += time;
    Msg += "ms]\n";
    Blynk.virtualWrite(V9, Msg);
    Pump.programmed_end = true;
  }
  else
    Pump.programmed_end = false;
  Blynk.virtualWrite(V10, "Pump Running");
}
/*Stops the pump
 *@param halt Stops the pump even if its not the time yet. ignored  if pump was forcifully started.
 */
void StopPump(bool halt = false)
{
  String Msg = "";
  if (halt == true && !Pump.force_start)
  {
    SIPO_Write(RELAY_PIN, !LOW);
    Msg += "[halted]";
  }
  if (millis() < Pump.end_time && Pump.programmed_end && !halt || !Pump.is_running)
    return;

  int result = millis() - Pump.start_time;
  SIPO_Write(RELAY_PIN, !LOW); // Relay is active low
  Serial.printf("PUMP TURNED Off [%d]\n", result);
  Msg += "PUMP was on for ";
  Msg += result;
  Msg += "ms.\n";
  Blynk.virtualWrite(9, Msg);
  Msg = "Last time the Pump was activated: ";
  if (day() < 10)
    Msg += "0";
  Msg += day();
  Msg += "/";
  if (month() < 10)
    Msg += "0";
  Msg += month();
  Msg += " ";
  if (hour() < 10)
    Msg += "0";
  Msg += hour();
  Msg += ":";
  if (minute() < 10)
    Msg += "0";
  Msg += minute();
  FileWrite("/log.txt", String(result));
  CalcWaterLevel(result);
  Blynk.virtualWrite(10, Msg);
  Pump.is_running = false;
}

#pragma region BLYNK

uint16_t Blynk_upload_interval = 60;   // Sets the interval for uploading values to Blynk in seconds
int Blynk_last_upload = 0;             // Keeps the timestamp of the last time it sent data over to Blynk
uint16_t Blynk_telemetry_interval = 5; // Sets the interval for the telemetry data to Blynk in seconds
int Blynk_telemetry_last = 0;          // Keeps the timestamp of the last time it sent telemetry over to Blynk
bool Blynk_telemetry = false;          // Enable/disables Blynk Telemetry

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable

  if (user_preferences.AutoLDR)
  {
    user_preferences.AutoLDR = false;
    control_variables.auto_ldr_overriden = true;
    uint8_t new_hour = hour() + 12;
    if (new_hour > 23)
      new_hour = new_hour - 24;
    control_variables.aut_ldr_resume_time = new_hour;
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
    StartPump();
  else
    StopPump();
}

BLYNK_WRITE(V9)
{
  // Set incoming value from pin V0 to a variable
  String valueV9 = param.asStr();
  Serial.printf("[V9][%d][Blynk] >> Value Recieved = %s \n", now(), valueV9);

  if (control_variables.is_SPIFFS_mounted)
  {
    if (valueV9 == "del logs")
    {
      SPIFFS.remove("/log.txt");
      Blynk.virtualWrite(V9, "logs cleaned");
    }
    else if (valueV9 == "del auto")
    {
      SPIFFS.remove("/lastAutoPump.txt");
      Blynk.virtualWrite(V9, "last auto pump reset");
      Pump.last_auto_run = 0;
    }
    else if (valueV9 == "del sensors")
    {
      SPIFFS.remove("/sensors.txt");
      Blynk.virtualWrite(V9, "sensors log were deleted.");
      control_variables.last_sensor_log = 0;
    }
    else if (valueV9 == "format")
    {
      bool result = SPIFFS.format();
      String msg = "SPIFFS formatted = ";
      msg += result;
      Blynk.virtualWrite(V9, msg);
    }
    else if (valueV9 == "usage")
    {
      double result = SPIFFS.usedBytes() / SPIFFS.totalBytes();
      String msg = "SPIFFS usage = ";
      msg += SPIFFS.usedBytes();
      msg += " of ";
      msg += SPIFFS.totalBytes();
      msg += "Bytes [";
      msg += result;
      msg += "%]";
      Blynk.virtualWrite(V9, msg);
    }
    else if (SPIFFS.exists(valueV9))
      Blynk.virtualWrite(V9, SPIFFS.open(valueV9).readString());
  }
  if (valueV9[0] == '$')
    StartPump(atoi(valueV9.substring(1).c_str()));
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
      LED_Strip.setMode(leds, Solid, newColor);
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
    command_result result = handleCommand(valueV9.substring(1));
    Blynk.virtualWrite(V9, result.response.c_str());
  }
  else if (valueV9 == "scan wifi")
  {
    Serial.println("Async Wifi Scan Requested");
    Blynk.virtualWrite(V9, "Async Wifi Scan Requested");
    WiFi.scanNetworks(true);
    control_variables.SendWifiResults = true;
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
    Blynk.virtualWrite(V9, Sensors.PrintString());
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
    if (!control_variables.is_SPIFFS_mounted)
    {
      msg = "SPIFFS not Mounted!\n";
    }
    else
    {
      msg += "SPIFFS: ";
      msg += (float)SPIFFS.usedBytes() / 1024;
      msg += "/";
      msg += (float)SPIFFS.totalBytes() / 1024;
      msg += " [kb] [";
      msg += (float)SPIFFS.usedBytes() / SPIFFS.totalBytes();
      msg += "%]\n";

      fs::File root = SPIFFS.open("/");
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
  LED_Strip.setMode(leds, Solid, CRGB(valueV20, LED_Strip.baseColor.g, LED_Strip.baseColor.b));
}
BLYNK_WRITE(V21) // GREEN
{
  int valueV21 = param.asInt();
  Serial.printf("[V21][%d][Blynk] >> Value Recieved = %d\n", now(), valueV21);
  LED_Strip.setMode(leds, Solid, CRGB(LED_Strip.baseColor.r, valueV21, LED_Strip.baseColor.b));
}
BLYNK_WRITE(V22) // BLUE
{
  int valueV22 = param.asInt();
  Serial.printf("[V22][%d][Blynk] >> Value Recieved = %d\n", now(), valueV22);
  LED_Strip.setMode(leds, Solid, CRGB(LED_Strip.baseColor.r, LED_Strip.baseColor.g, valueV22));
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
}

void endOTA()
{
  Serial.println("\nEnd");
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
    command_result result = handleCommand(msg.substring(1));
    // response = "[";
    // response += result.result;
    // response += "] ";
    response += result.response;
  }
  else
    response += "M;ACK;";
  return response;
}

// Checks wheater or not it is time to water the garden again checks for the prefered hour to do so or
// if it has been 4 hours since the timer has been triggered and the prefered hour have not been reached yet
bool auto_pump_time_ajusted(uint current_time)
{
  // Checks for bad prefered time
  if (user_preferences.prefered_pump_hour > 23 && user_preferences.prefered_pump_hour < -1)
    user_preferences.prefered_pump_hour = 19;
  // Waits for the prefered time after the interval has passed
  if (current_time - Pump.last_auto_run >= user_preferences.auto_pump_interval && hour() == user_preferences.prefered_pump_hour)
    return true;
  // If we are whithin 4 hours to the interval and at the prefered time we water the garden earlier
  if (current_time - Pump.last_auto_run >= user_preferences.auto_pump_interval - 4 * 60 * 60 && hour() == user_preferences.prefered_pump_hour)
    return true;
  // If the prefered time is disabled (-1) waters the garden at the interval
  if (current_time - Pump.last_auto_run >= user_preferences.auto_pump_interval && user_preferences.prefered_pump_hour == -1)
    return true;
  // If it has been 4 hours since the interval has passed and the prefered hour have not been reached we water the garden anyways
  if (current_time - Pump.last_auto_run >= user_preferences.auto_pump_interval + 4 * 60 * 60)
    return true;
  return false;
}

void NEWupdateValues(bool skipUpload = false)
{
  if (control_variables.disable_sensors_update)
    return;

  int current_time = now();

  if (current_time - control_variables.last_sensor_update >= user_preferences.sensor_update_interval)
  {
    byte RESTORE_MUX = MUX_VALUE;
    byte RESTORE_SIPO = SIPO_VALUE;
    control_variables.last_sensor_update = current_time;
    //analogRead on the mux fucks the onewire thingy
    pinMode(MUX_INPUT, INPUT);
    setMuxPort(DS18b20_PIN);
    DS18.requestTemperatures();
    DS18inner = DS18.getTempCByIndex(0);

    if (DS18inner == -127 || DS18inner == -255 || DS18inner == 85)
      Sensors.getSensorById("ds18inner")->setStatus(Offline);
    else
      Sensors.getSensorById("ds18inner")->UpdateValue(DS18inner);

    DS18Probe = DS18.getTempCByIndex(1);
    if (DS18Probe == -127 || DS18Probe == -255 || DS18Probe == 85)
      Sensors.getSensorById("ds18prob")->setStatus(Offline);
    else
    {
      Sensors.getSensorById("ds18prob")->UpdateValue(DS18Probe);
    }

    SIPO_Write(MOIST_EN0, 1, true);
    setMuxPort(MOIST_1);
    delay(1);
    int totalMoisture = 0;
    for (size_t i = 0; i < MOISTURE_READS; i++)
    {
      totalMoisture += analogRead(MUX_INPUT);
    }
    totalMoisture = totalMoisture / 10;
    Moisture_1 = map(totalMoisture, 0, MAX_MOISTURE_1_READ, 0, 100);
    Sensors.getSensorById("moist")->UpdateValue(Moisture_1);
    setMuxPort(MOIST_2);
    delay(1);
    totalMoisture = 0;
    for (size_t i = 0; i < MOISTURE_READS; i++)
    {
      totalMoisture += analogRead(MUX_INPUT);
    }
    totalMoisture = totalMoisture / 10;
    Moisture_2 = map(totalMoisture, 0, MAX_MOISTURE_1_READ, 0, 100);
    Sensors.getSensorById("moist2")->UpdateValue(Moisture_2);
    SIPO_Write(MOIST_EN0, 0, true);

    setMuxPort(WL_MIDDLE);
    delay(1);
    wl_half = !digitalRead(MUX_INPUT);
    setMuxPort(WL_BOTTOM);
    delay(1);
    wl_empty = !digitalRead(MUX_INPUT);

    if (wl_half + wl_empty != Pump.WaterState)
    {
      Pump.WaterState = wl_half + wl_empty;
      CalcWaterLevel(0, 50 * Pump.WaterState);
    }

    setMuxPort(LDR_PIN);
    delay(1);
    int totalLDR = 0;
    for (size_t i = 0; i < 10; i++)
    {
      totalLDR += analogRead(MUX_INPUT);
    }
    LDR_VALUE = totalLDR / 10;
    Sensors.getSensorById("ldr")->UpdateValue(LDR_VALUE);

    setMuxPort(DHT_PIN);
    DHT11_hum = dht.readHumidity();
    DHT11_temp = dht.readTemperature();
    Sensors.getSensorById("dht_hum")->UpdateValue(DHT11_hum);
    Sensors.getSensorById("dht_temp")->UpdateValue(DHT11_temp);

    SIPO_Write(MOIST_EN1, HIGH, true);
    setMuxPort(RAIN_PIN);
    delay(1);
    int totalRain = 0;
    for (size_t i = 0; i < 250; i++)
    {
      totalRain += analogRead(MUX_INPUT);
    }
    Rain_level = totalRain / 250;
    Sensors.getSensorById("rain")->UpdateValue(Rain_level);
    SIPO_Write(MOIST_EN1, LOW, true);
    SIPO_WriteBYTE(RESTORE_SIPO);
    setMuxPort(RESTORE_MUX);

    if (current_time - control_variables.last_sensor_log >= user_preferences.sensor_log_interval && !skipUpload)
    {
      if (control_variables.is_SPIFFS_mounted)
      {
        String msg = "";
        // Creates the file if it doest exist
        if (!SPIFFS.exists("/sensors.txt"))
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
    if (current_time - control_variables.last_MQTT_update >= user_preferences.MQTT_interval && !skipUpload)
    {
      Sensors.Send_All_MQTT();
    }
  }

  if (auto_pump_time_ajusted(current_time))
  {
    Pump.last_auto_run = current_time;
    StartPump(user_preferences.auto_pump_duration);
    FileWrite("/log.txt", "Automation Pump Triggered");
    if (control_variables.is_SPIFFS_mounted && control_variables.is_time_synced)
    {
      if (SPIFFS.exists("/lastAutoPump.txt"))
      {
        SPIFFS.remove("/lastAutoPump.txt");
      }
      FileWrite("/lastAutoPump.txt", String(current_time), true, false);
      if (SPIFFS.exists("/lastAutoPumpReadable.txt"))
      {
        SPIFFS.remove("/lastAutoPumpReadable.txt");
      }
      FileWrite("/lastAutoPumpReadable.txt", String(" "), true, true, false);
    }
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
      control_variables.is_time_synced = true;
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

void AutoRegulateBrightness()
{
#define LDR_READS 20
  int newBright = 0;
  for (size_t i = 0; i < LDR_READS; i++)
  {
    newBright += analogRead(MUX_INPUT);
  }
  newBright = newBright / LDR_READS;

  if (newBright > 1000)
    newBright = 1000;

  if (user_preferences.turn_off_lights_time >= 0)
  {
    if (hour() == user_preferences.turn_off_lights_time)
      control_variables.turn_off_lights_flag = true;
    if (hour() == 12)
      control_variables.turn_off_lights_flag = false;

    if (control_variables.turn_off_lights_flag)
      newBright = 1000;
  }

  if (now() - control_variables.sync_light > 5)
  {
    if (newBright < 900)
      Blynk.virtualWrite(V0, 0);
    else
      Blynk.virtualWrite(V0, 1);
    control_variables.sync_light = true;
  }
  FastLED.setBrightness(map(newBright, 1000, 0, 0, 255));
  FastLED.show();
}

void LED_TASK(void *pvParameters)
{
  for (;;)
  {
    LED_Strip.run(leds);
    onBoardLEDs.run(onboard_leds);
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void InitializeSensors()
{
  Sensors.reset();
  if (!Sensors.loadSensors())
  {
    Sensors.Add("ds18inner", "DS18 Inner", SensorDataType::Type_Float);
    Sensors.Add("ds18prob", "DS18 Probe", SensorDataType::Type_Float);
    Sensors.Add("moist", "Moisture Sensor", SensorDataType::Type_Int);
    Sensors.Add("ldr", "LDR Sensor", SensorDataType::Type_Int);
    Sensors.Add("dht_hum", "DHT Humidity", SensorDataType::Type_Int);
    Sensors.Add("dht_temp", "DHT Temperature", SensorDataType::Type_Float);
    Sensors.Add("waterlevel", "Water Level", SensorDataType::Type_Int);
    Sensors.Add("rain", "Rain", SensorDataType::Type_Int);
  }
}

// Callback for payload at the mqtt topic
void HiveMQ_Callback(char *topic, byte *payload, unsigned int length)
{

  // GPT:: String incommingMessage((char*) payload, length);
  String incommingMessage = "";
  for (int i = 0; i < length; i++)
    incommingMessage += (char)payload[i];

  Serial.printf("MQTT::[%s]-->[%s]\n", topic, incommingMessage.c_str());
  String in_topic = "";
  in_topic += topic;

  // Only topic Shell will have valid commands and expect a response
  if (in_topic == "Shell")
  {
    command_result result = handleCommand(incommingMessage);
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
    LED_Strip.setMode(leds, LED_Animations::Solid, newColor);
  }
}

void setup()
{
  // Debug console
  Serial.begin(115200);

  FastLED.addLeds<WS2812, WS2812_PIN, GRB>(leds, LED_STRIP_SIZE); // GRB ordering is typical
  FastLED.addLeds<WS2811, WS2811_PIN, BGR>(beacons_leds, BEACONS_SIZE);
  FastLED.addLeds<WS2812, ONBOARD_RGB_PIN, GRB>(onboard_leds, ONBOARD_RGB_SIZE);

#ifdef USE_SPIFFS
  if (SPIFFS.begin(true))
  {
    control_variables.is_SPIFFS_mounted = true;
  }
#endif
  //  Initialize Sensors
  InitializeSensors();
  // Loads user preferences
  user_preferences.load();
  Serial.print(user_preferences.Print());

  // Declare PinModes
  pinMode(LDR_PIN, INPUT);
  // pinMode(VBAT_PIN, INPUT);
  // pinMode(MOISTURE_SENSOR, INPUT);
  // pinMode(EMPTY_WATER_LEVEL, INPUT_PULLUP);
  // pinMode(HALF_WATER_LEVEL, INPUT_PULLUP);
  // pinMode(PUMP_RELAY, OUTPUT);
  pinMode(27, OUTPUT); // AKA Pin 33 Enable.
  pinMode(SIPO_LATCH, OUTPUT);
  pinMode(SIPO_CLK, OUTPUT);
  pinMode(SIPO_DATA, OUTPUT);
  for (size_t i = 0; i < 4; i++)
  {
    pinMode(MUX_CONTROL[i], OUTPUT);
  }
  pinMode(MUX_INPUT, INPUT);
  pinMode(MUX_ENABLE, OUTPUT);

#ifdef PROTO
  pinMode(TEST_BUTTON, INPUT_PULLUP);
#endif

  setMuxPort(0);
  SIPO_WriteBYTE(0);

  WiFi.begin(WIFISSID, WIFIPASSWD);
  bool createAp = false; // flag for creating an AP in case we can't connect to wifi
  bool beauty = true;    // Esthetics for Serial.print();
  WiFi.hostname(DEVICE_NAME);
  // Try to connect to WiFi
  int StartMillis = millis();
  while (WiFi.status() != WL_CONNECTED && !createAp)
  {
    if (millis() % 100 == 0 && beauty)
    {
      Serial.print(".");
      beauty = false;
      leds[0] = CRGB(0xFF * beauty, 0x99 * beauty, 0);
      leds[1] = CRGB(0xFF * !beauty, 0x99 * !beauty, 0);
      FastLED.show();
    }
    else if (millis() % 100 != 0)
    {
      beauty = true;
    }

    if (millis() - StartMillis > 14999)
    {
      Serial.printf("\nNetwork '%s' not found.\n", WIFISSID);
      createAp = true;
    }
  }
  if (createAp)
  {
    // fill_solid(leds, 2, 0xFF0000);
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
    WiFi.enableAP(true);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(WIFI_SOFTAP_SSID, WIFI_SOFTAP_PASSWD);
    // dnsServer.start(53, "*", WiFi.softAPIP());
    Serial.println("");
    Serial.print("Creating WiFi Ap.\n ---SSID:  ");
    Serial.println(WIFI_SOFTAP_SSID);
    Serial.print(" --IP address: ");
    Serial.println(WiFi.softAPIP());
  }
  else
  {
    // fill_solid(leds, 2, 0x00FF00);
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
    Serial.println(WIFISSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal RSSI: ");
    Serial.println(WiFi.RSSI());
    WiFi.setAutoReconnect(true);
  }
  FastLED.setBrightness(64);
  FastLED.clear();
  SetOnboardLEDS(CRGB::Red, true);
  setBeaconColor(0xf0f0);
  // Try to connect to blynk. don't use Blynk.begin(), it will block the code if you don't have an in ternet connection
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
  tcpServer.Begin();

  // Starts the DS18b20 and the DHT sensor.
  setMuxPort(DS18b20_PIN);
  DS18.begin();
  DS18.setResolution(11);
  setMuxPort(0);
  // Serial.println("devs");
  // Serial.println(DS18.getDeviceCount());
  // for (size_t i = 0; i < DS18.getDeviceCount(); i++)
  // {
  //   DeviceAddress aaa;
  //   DS18.getAddress(aaa, i);
  //   printAddress(aaa);
  //   Serial.println();
  // }

  // dht.begin();
  // sensors_hist.Reset();

  control_variables.time_synced = getTime(); // Sync Time Online


  // Loads the last time pump was automatically trigerred
  if (control_variables.is_SPIFFS_mounted)
  {
    if (SPIFFS.exists("/lastAutoPump.txt") && control_variables.is_time_synced)
    {
      String j = SPIFFS.open("/lastAutoPump.txt").readString();
      Pump.last_auto_run = atoi(j.c_str());
    }
  }
  // get initial reading of the values
 NEWupdateValues(true);

  if (Pump.WaterState == 0)
    Pump.tank_0_percent = true;
  if (Pump.WaterState <= 1)
    Pump.tank_50_percent = true;
  hive_client.setCACert(root_ca);
  HiveMQ.setServer(MQTT_URL, MQTT_PORT);
  HiveMQ.setCallback(HiveMQ_Callback);

  if (HiveMQ.connect(DEVICE_NAME, MQTT_USER, MQTT_PASSWD))
    Serial.println("MQTT Connected");
  else
    Serial.printf("Can't Connect to MQTT Error Code : %d\n", HiveMQ.state());
  HiveMQ.subscribe("Turing/Color");

  LED_Strip.setBoundries(0, 10);
  beacons.setBoundries(0, BEACONS_SIZE - 1);
  onBoardLEDs.setBoundries(0, ONBOARD_RGB_SIZE - 1);
  onBoardLEDs.setMode(onboard_leds, LED_Animations::BlinkTwoColor, CRGB::DarkGreen, CRGB::Pink);
  onBoardLEDs.setInterval(500);
  FastLED.setBrightness(64);

  LED_Strip.setInterval(300);

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
  beacons.setMode(beacons_leds, LED_Animations::Solid, 0x00f0f0);
  FileWrite("/log.txt", "Booted");
  if (control_variables.time_synced)
    FileWrite("/log.txt", "Time Synced at boot");
#ifdef PROTO
  Sensors.suppressError = true;
  Serial.printf(">>>>>>>>>>>END OF SETUP %dms<<<<<<<<\n", millis());
#endif
}

void loop()
{
  ArduinoOTA.handle();
  Blynk.run();
  HiveMQ.loop();
  NEWupdateValues();

  tcpServer.HandleServer();

  if (WiFi.scanComplete() > 0 && control_variables.SendWifiResults)
  {
    String returnmsg = "";
    byte numSsid = WiFi.scanComplete();
    for (byte i = 0; i < numSsid; i++)
    {
      returnmsg += WiFi.SSID(i);
      returnmsg += '=';
      returnmsg += WiFi.RSSI(i);
      returnmsg += '\n';
    }
    control_variables.SendWifiResults = false;
    Blynk.virtualWrite(V9, returnmsg);
  }

  EVERY_N_MILLISECONDS(100)
  {
    // if (user_preferences.AutoLDR)
    //   AutoRegulateBrightness();
  }
  EVERY_N_SECONDS(2)
  {
    Sensors.report();
  }

  EVERY_N_SECONDS(300)
  {

    if (!control_variables.time_synced)
    {
      control_variables.time_synced = getTime();
      if (control_variables.time_synced)
        FileWrite("/log.txt", "Time Synced::" + String(millis()));
    }
    if (!WiFi.isConnected())
    {
      WiFi.begin(WIFISSID, WIFIPASSWD);
      int StartMillis = millis();
      bool beauty = false;
      Serial.printf("Trying to reconnect to \'%s\'\n", WIFISSID);
      bool beauty_two = false;
      while (WiFi.status() != WL_CONNECTED && millis() - StartMillis < 10000)
      {
        if (millis() % 500 == 0 && beauty)
        {
          Serial.print(".");
          beauty = false;
          onboard_leds[0] = CRGB(0xFF * beauty_two, 0xFF * !beauty_two, 0);
          beauty_two = !beauty_two;
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
    }
    if (!HiveMQ.connected())
    {
      HiveMQ.disconnect();
      HiveMQ.unsubscribe("Turing/Shell");
      HiveMQ.unsubscribe("Turing/Color");

      if (HiveMQ.connect(DEVICE_NAME, MQTT_USER, MQTT_PASSWD))
        Serial.println("MQTT Connected");
      else
        Serial.printf("Can't Connect to MQTT Error Code : %d\n", HiveMQ.state());
      HiveMQ.subscribe("Turing/Shell");
      HiveMQ.subscribe("Turing/Color");
    }
  }

  if (!wl_empty && !Pump.tank_0_percent)
  {
    Pump.tank_0_percent = true;
    if (Pump.is_running)
      StopPump(true);
    Blynk.logEvent("low_water", "Bottom Sensor is not detecting water");
    FileWrite("/log.txt", "Bottom Sensor Triggered");
  }

  if (wl_empty > 0 && Pump.tank_0_percent)
  {
    Pump.tank_0_percent = false;
    Blynk.logEvent("water_full", "Bottom Sensor detecting water");
    FileWrite("/log.txt", "Bottom Sensor Filled");
  }

  if (!wl_half <= 50 && !Pump.tank_50_percent)
  {
    Pump.tank_50_percent = true;
    FileWrite("/log.txt", "Middle Sensor Triggered");
  }

  if (wl_half > 50 && Pump.tank_50_percent)
  {
    Pump.tank_50_percent = false;
    FileWrite("/log.txt", "Middle Sensor Filled");
  }

  if (Pump.is_running && millis() >= Pump.end_time && Pump.programmed_end)
  {
    StopPump();
  }

  if (control_variables.auto_ldr_overriden && hour() == control_variables.aut_ldr_resume_time)
  {
    control_variables.auto_ldr_overriden = false;
    user_preferences.AutoLDR = true;
  }
#ifdef PROTO
  // TEST AREA
  if (digitalRead(TEST_BUTTON) == LOW && digital_debounce)
  {
    _disable = !_disable;
    FastLED.setBrightness(127 * _disable);
    delay(1);
  }

  if (digitalRead(TEST_BUTTON) == HIGH)
  {
    digital_debounce = true;
  }

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
    command_result res = handleCommand(s);
    Serial.printf("[%d] - ", res.result);
    Serial.println(res.response);
    if (s == "blink")
    {
      LED_Strip.setMode(leds, LED_Animations::FadeInOutSolid, CRGB::Red, 0, 25, 10);
      delay(10000);
      LED_Strip.setMode(leds, LED_Animations::FadeInOutSolid, CRGB::Blue, 0, 25, 10);
      delay(10000);
      LED_Strip.setMode(leds, LED_Animations::FadeInOutSolid, CRGB::Green, 0, 25, 10);
      delay(10000);
      LED_Strip.setMode(leds, LED_Animations::Solid, CRGB::Orange, 0, 2500, 10);
      delay(10000);
    }
    if (s == "sipo")
    {
      Serial.printf("SIPO: %d, MUX: %d\n", SIPO_VALUE, MUX_VALUE);
    }
  }
#endif
}
