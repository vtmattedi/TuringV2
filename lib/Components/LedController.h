#pragma once

#include <Arduino.h>
#include <FastLED.h>

#define WS2812_PIN 23
#define WS2811_PIN 26
#define ONBOARD_RGB_PIN 2

#define ONBOARD_RGB_SIZE 3

// 3 Ws2012E Lights
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

#define ONBOARD_RGB_COLOR_ORDER GRB
#define ONBOARD_RGB_SIZE 3

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

class Animation
{
public:
    CRGB baseColor;
    void Run(CRGB leds[]);
    void Init(CRGB leds[], CRGB Color);
};

class Solid : Animation
{
    void Init(CRGB leds[], CRGB Color)
    {
        for (size_t i = 0; i < sizeof(leds) / sizeof(leds[0]); i++)
        {
            leds[0] = Color;
        }
        FastLED.show();
    }
    void Run(CRGB leds[])
    {
        return;
    }
};

class Blink : Animation
{
    bool on = true;
    void Init(CRGB leds[], CRGB Color)
    {
        baseColor = Color;
        for (size_t i = 0; i < sizeof(leds) / sizeof(leds[0]); i++)
        {
            leds[0] = Color;
        }
        FastLED.show();
    }
    void Run(CRGB leds[])
    {
        on = !on;
        for (size_t i = 0; i < sizeof(leds) / sizeof(leds[0]); i++)
        {
            leds[0] = baseColor * on;
        }
        return;
    }
};

class Fade : Animation
{
    byte bright = 0x77;
    bool dec = false;
    void Init(CRGB leds[], CRGB Color)
    {
        baseColor = Color;
        for (size_t i = 0; i < sizeof(leds) / sizeof(leds[0]); i++)
        {
            leds[0] = Color;
        }
        FastLED.show();
    }
    void Run(CRGB leds[])
    {
        if (bright == 0xff || bright == 0)
            dec = !dec;
        dec ? bright-- : bright++;

        for (size_t i = 0; i < sizeof(leds) / sizeof(leds[0]); i++)
        {
            leds[i] = baseColor; 
        }
        fadeToBlackBy(leds,sizeof(leds) / sizeof(leds[0]),0xff - bright);
        return;
    }
};

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
