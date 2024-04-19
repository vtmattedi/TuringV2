#include "ExternalAdc.h"

ExternalAdc ExtAdc;

String get_EA_pin_name(uint8_t pin)
{
    switch (pin)
    {
    case EA_MUX:
        return "  MUX ";
    case EA_VBAT:
        return " VBAT ";
    case EA_MOIST0:
        return "MOIST0";
    case EA_MOIST1:
        return "MOIST1";
    default:
        return "UNKNOWN";
    }
}

void ExternalAdc::setup()
{
    Wire.begin();
    available = EA.begin(0x48);
    if (!available)
    {
        Serial.println("ADS1115 not found");
        return;
    }
    EA.setGain(GAIN_ONE);
    EA.setDataRate(RATE_ADS1115_860SPS);
    Serial.println("ADS1115 Started");
}

String ExternalAdc::i2cScan()
{
    byte error, address;
    int deviceCount = 0;
    String result = "";

    Serial.println("Scanning I2C bus...");

    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            result += "Device found at address 0x";
            if (address < 16)
            {
                result += "0";
            }
            result += String(address, HEX);
            result += "\n";
            deviceCount++;
        }
    }

    if (deviceCount == 0)
    {
        result = "No devices found.";
    }

    return result;
}

String ExternalAdc::adcSweep()
{
    if (!available)
    {
        return "ADS1115 not initialized\n";
    }

    int a = millis();
    String res = "";
    for (size_t i = 0; i < 4; i++)
    {
        int val = EA.readADC_SingleEnded(i);
        if (val < 0)
            val = 0;
        res += "[" + get_EA_pin_name(i) + "]: ";
        res += "Channel " + String(i) + ": " + String(val);
        res += " (" + String(EA.computeVolts(val)) + " V)\n";
        /* code */
    }
    res += "Time: " + String(millis() - a) + "ms\n";
    return res;
}

void ExternalAdc::muxSweep(uint16_t _delay)
{
    if (!available)
    {
        return;
    }
    if (_delay > 2000)
    {
        _delay = 10;
    }

    pinMode(MUX_INPUT, INPUT);
    int a = millis();
    for (size_t i = 0; i < MUX_SIZE; i++)
    {
        setMuxPort(i);
        delay(_delay);
        muxValues[i] = getValue(EA_MUX);
    }
    muxSweepTime = millis() - a;
}

String ExternalAdc::extMuxSweep()
{
    if (!available)
    {
        return "ADS1115 not initialized\n";
    }
    muxSweep();
    String res = "";
    for (size_t i = 0; i < MUX_SIZE; i++)
    {
        res += "[" + getMuxAlias(i) + "]: ";
        res += String(muxValues[i]);
        res += " (" + String(EA.computeVolts(muxValues[i])) + " V)\n";
    }
    res += "Time: " + String(muxSweepTime) + "ms\n";
    return res;
}

String ExternalAdc::sweepAndCompare()
{
    if (!available)
    {
        return "ADS1115 not initialized\n";
    }
    int _timer = millis();
    muxSweep();
    Mux.Sweep();
    _timer = millis() - _timer;
    String res = "";
    for (size_t i = 0; i < MUX_SIZE; i++)
    {
        res += "[" + getMuxAlias(i) + "]: ";
        res += "ADS1115: " + String(muxValues[i]);
        res += " (" + String(EA.computeVolts(muxValues[i])) + " V)  | ";
        res += "ESP-ADC: " + String(Mux[i]);
        res += " (" + String(Mux[i] * 3.3 / 4096.0) + " V)\n";
    }
    res += formatString("ADS took: %d ms, ESP took: %d ms\nTotal Time: %d ms", muxSweepTime, Mux.runtime, _timer);
    return res;
}

int16_t ExternalAdc::getValue(uint8_t pin)
{
    if (!available || pin > 3)
    {
        return -1;
    }
    return EA.readADC_SingleEnded(pin);
}

float ExternalAdc::getVoltage(uint8_t pin)
{
    if (!available || pin > 3)
    {
        return NAN;
    }
    return EA.computeVolts(EA.readADC_SingleEnded(pin));
}

int16_t ExternalAdc::getPinValue(uint8_t pin)
{
    if (!available || pin > MUX_SIZE)
    {
        return -1;
    }
    pinMode(MUX_INPUT, INPUT);
    setMuxPort(pin);
    delay(1);
    muxValues[pin] = getValue(EA_MUX);
    return muxValues[pin];
}

float ExternalAdc::getPinVoltge(uint8_t pin)
{
    int16_t val = getPinValue(pin);
    return EA.computeVolts(val);
}

// Overload [] operator to access elements of anResult array
int16_t &ExternalAdc::operator[](uint8_t index)
{
    if (index >= 0 && index < MUX_SIZE)
        return muxValues[index];
    else
        throw std::out_of_range("Index out of range");
}

void ExternalAdc::otaUpdate(bool start)
{   
    if (start)
    {
        _oldAvailable = available;
        available = false;
    }
    else
    {
        available = _oldAvailable;
    }
}