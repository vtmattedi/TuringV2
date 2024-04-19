#ifndef HWCONTROL_CPP
#define HWCONTROL_CPP

#include "HwControl.h"

MuxSweeper Mux;

/// @brief The Four MUX control pins.
const byte MUX_CONTROL[4] = {MUX_S0, MUX_S1, MUX_S2, MUX_S3};
/// @brief The current value of MUX.
byte MUX_VALUE;
/// @brief The current value of SIPO.
byte SIPO_VALUE;

/// @brief Turns on the MUX and selects a port to be used.
/// @param port the output pin to be changed. [0-15]
void setMuxPort(byte port)
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
    digitalWrite(MUX_ENABLE, MUX_ON);
}

/// @brief Writes a bit to the SIPO Shifter. turns off the MUX.
/// @param pin the output pin to be changed. [0-7].
/// @param value the value to be shifted into that pin.
/// @param skipmux skips the MUX selection after setting the SIPO.
void SIPO_Write(byte pin, bool value, bool skipmux)
{
    // Serial.printf("SIPO_WRITE [%d] [%d]\n", pin, value);
    if (pin > 7)
    {
        Serial.printf("No pin:%d on the SIPO shifter, max is 8.\n", pin);
        return;
    }
    // Disables MUX since SIPO shares DATA and LATCH with MUX Selector pins
    if (!skipmux)
        digitalWrite(MUX_ENABLE, MUX_OFF);
    SIPO_VALUE = (SIPO_VALUE & ~(1 << pin)) | (value << pin);
#ifdef PRINT_SIPO_VAL
    Serial.printf("Writing on pin [%d]: [%d]\n", pin, value);
#endif
    digitalWrite(SIPO_LATCH, LOW);
    shiftOut(SIPO_DATA, SIPO_CLK, MSBFIRST, SIPO_VALUE);
    digitalWrite(SIPO_LATCH, HIGH);
    // restore MUX
    if (!skipmux)
        setMuxPort(MUX_VALUE);

    // SET LED_2 as the visual indicator for relay pin
}

/// @brief Writes a byte to the SIPO Shifter. turns off the MUX.
/// @param newValue The byte to be shifted in.
/// @param skipmux skips the MUX selection after setting the SIPO.
void SIPO_WriteBYTE(byte newValue, bool skipmux)
{
    // Serial.printf("SIPO_WRITEBYTE [%d] \n", newValue);
    if (!skipmux) // Disables MUX since SIPO shares DATA and LATCH with MUX Selector pins
        digitalWrite(MUX_ENABLE, MUX_OFF);
    digitalWrite(SIPO_LATCH, LOW);
    shiftOut(SIPO_DATA, SIPO_CLK, MSBFIRST, newValue);
    digitalWrite(SIPO_LATCH, HIGH);
#ifdef PRINT_SIPO_VAL
    Serial.printf("Writing byte [%02x]\n", newValue);
#endif

    if (!skipmux) // restore MUX
        setMuxPort(MUX_VALUE);
    
    SIPO_VALUE = newValue;
}

/// @brief Gets the alias of a port of the MUX.
/// @param muxPin the port of the mux you want the alias.
/// @return String with the alias of the port.
String getMuxAlias(int muxPin)
{
    switch (muxPin)
    {
    case MOIST_1:
        return "  MOIST_1  ";
    case MOIST_2:
        return "  MOIST_2  ";
    case RAIN_PIN:
        return "    RAIN   ";
    case DHT_PIN:
        return "    DHT    ";
    case DS18b20_PIN:
        return "  DS18b20  ";
    case WL_EXTRA:
        return " WL_EXTRA  ";
    case WL_BOTTOM:
        return " WL_BOTTOM ";
    case WL_MIDDLE:
        return " WL_MIDDLE ";
    case LDR_PIN:
        return "    LDR    ";
    default:
        return "NOT_DEFINED";
    }
}

/// @brief Gets the alias of a pin of the SIPO shifter.
/// @param muxPin the pin of the SIPO shifter you want the alias.
/// @return String with the alias of the pin.
String getSipoAlias(int sipoPin)
{
    switch (sipoPin)
    {
    case LED_0:
        return "LED_PUMP";
    case LED_1:
        return "LED_MED";
    case LED_2:
        return "LED_LOW";
    case MOIST_EN0:
        return "MOIST_EN0";
    case MOIST_EN1:
        return "MOIST_EN1";
    case RELAY_PIN:
        return "NOT_RELAY_PIN";
    default:
        return "NOT_DEFINED";
    }
}

/// @brief Centers a String with a total size.
/// @param str the String input to be centered,
/// @param size The final length of the String.
/// @param mode Alignment, 0 = centered, >0 Left Aligned and <0 Right Aligned.
/// @return The String itslef if length >= size or a String with white spaces around until it reaches the size.
String centerFixed(String str, int size, int mode)
{
    if (size <= 0 || size > 100 /*Arbritary max len*/)
    {
        return str;
    }
    if (str.length() >= size)
        return str;
    bool front = true;
    while (str.length() < size)
    {
        if ((front && mode == 0) || mode < 0)
            str = " " + str;
        else if ((!front && mode == 0) || mode > 0)
            str += " ";
        front = !front;
    }
    return str;
}

/// @brief Sweeps some of the ports of MUX
/// @param size The total num of the ports the be updated.
/// @param args The ports to be updated.
void MuxSweeper::partialSweep(int size, ...)
{
    runtime = millis();
    byte old_mux = MUX_VALUE;
    pinMode(MUX_INPUT, INPUT);
    delay(delay_ms);
    va_list args;
    va_start(args, size);
    for (int i = 0; i < size; i++)
    {
        int c = va_arg(args, int);
        if (debug)
            Serial.printf("%d -> updating pin [%s](%d)\n", i, getMuxAlias(c), c);

        updatePin(c);
    }
    va_end(args);

    setMuxPort(old_mux);
    runtime = millis() - runtime;
}

/// @brief Update the sotred value of a port of the MUX.
/// @param pin The port to be read and updated.
void MuxSweeper::updatePin(uint8_t pin, bool digital)
{
    if (pin >= MUX_SIZE)
        return;

    setMuxPort(pin);

    if (digital)
    {
        pinMode(MUX_INPUT, INPUT);
        dgResult[pin] = digitalRead(MUX_INPUT);
        return;
    }

    // setMuxPort(pin);
    delay(delay_ms);
    int an_read = 0;
    for (size_t j = 0; j < num_of_reads; j++)
    {
        an_read += analogRead(MUX_INPUT);
    }
    an_read /= num_of_reads;
    anResult[pin] = an_read;
    dgResult[pin] = an_read > 930; // 0.75 vdd threshold
}

void MuxSweeper::clipPinValue(uint8_t pin, uint16_t min, uint16_t max)
{
    anResult[pin] = constrain(anResult[pin], min, max);
}

/// @brief Sweeps the mux on all ports, updating the values of all pins
void MuxSweeper::Sweep()
{
    runtime = millis();
    byte old_mux = MUX_VALUE;
    pinMode(MUX_INPUT, INPUT);
    delay(delay_ms * 2);
    for (size_t i = 0; i < MUX_SIZE; i++)
    {
        updatePin(i);
    }
    setMuxPort(old_mux);
    last_sweep = now();
    runtime = millis() - runtime;
}

/// @brief Prints the stored valued of the MUX
void MuxSweeper::Print()
{
    String response = formatString("[%s] Mux Sweep: [%d]ms\n",
                                   timestampToDateString(last_sweep, OnlyTime),
                                   runtime);
    for (size_t i = 0; i < MUX_SIZE; i++)
    {
        response += formatString("[%s]   *port: %2d Analog[%d]: %4d Digital: %d \n",
                                 getMuxAlias(i).c_str(),
                                 i,
                                 num_of_reads,
                                 anResult[i],
                                 dgResult[i]);
    }

    Serial.print(response);
}

// Overload () operator o access elements of dgResult array
bool &MuxSweeper::operator()(uint8_t index)
{
    if (index >= 0 && index < MUX_SIZE)
        return dgResult[index];
    else
        throw std::out_of_range("Index out of range");
}

// Overload [] operator to access elements of anResult array
uint16_t &MuxSweeper::operator[](uint8_t index)
{
    if (index >= 0 && index < MUX_SIZE)
        return anResult[index];
    else
        throw std::out_of_range("Index out of range");
}

/// @brief Prints the Sipo values to the Serial
void printSipo()
{
    String response = formatString("Sipo Info: [0x%02hhX] \n",
                                   SIPO_VALUE);
    for (size_t i = 0; i < SIPO_SIZE; i++)
    {
        response += formatString("[%14s]   *port: %d Value: %s \n",
                                 centerFixed(getSipoAlias(i), 14).c_str(),
                                 i,
                                 bitRead(SIPO_VALUE, i) ? "HIGH" : "LOW");
    }
    Serial.print(response);
}

/// @brief Setups the pins used defined in HwInfo.h
void setupPins()
{
    // Declare PinModes
    pinMode(LDR_PIN, INPUT);
    pinMode(VBAT_PIN, INPUT);
    pinMode(SIPO_LATCH, OUTPUT);
    pinMode(SIPO_CLK, OUTPUT);
    pinMode(SIPO_DATA, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, PUMP_OFF);
    // SIPO_Write(RELAY_PIN, PUMP_OFF, true);
    for (size_t i = 0; i < 4; i++)
    {
        pinMode(MUX_CONTROL[i], OUTPUT);
    }
    pinMode(MUX_INPUT, INPUT_PULLUP);
    pinMode(MUX_ENABLE, OUTPUT);

    setMuxPort(0);
    SIPO_WriteBYTE(SIPO_START_BYTE);
}

#endif