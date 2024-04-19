#pragma once
#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <HwControl.h>
#include <NightMareNetwork.h>

#define EA_MUX 0
#define EA_VBAT 1
#define EA_MOIST0 2
#define EA_MOIST1 3
#define EA_ADDR 0x48
class ExternalAdc
{
private:
    // Make sure it is not acessed when not enabled.
    // I2C lib goes to infinite loop if not connected
    Adafruit_ADS1115 EA;
    bool _oldAvailable = false;

public:
    bool available = false;
    uint16_t muxSweepTime = 0;
    int16_t muxValues[MUX_SIZE] = {0};
    void setup();
    String i2cScan();
    String adcSweep();
    void muxSweep(uint16_t _delay = 0);
    String extMuxSweep();
    String sweepAndCompare(); 
    float getPinVoltge(uint8_t pin);
    int16_t getPinValue(uint8_t pin);
    float getVoltage(uint8_t pin);
    int16_t getValue(uint8_t pin);
    int16_t &operator[](uint8_t index);
    void otaUpdate(bool running);
};

String get_EA_pin_name(uint8_t pin);

extern ExternalAdc ExtAdc;