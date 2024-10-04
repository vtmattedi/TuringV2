#pragma once
#ifndef HWCONTROL_H
#define HWCONTROL_H

#include <Arduino.h>
#include "HwInfo.h"
#include <NightMareNetwork.h>

void setMuxPort(byte port);
void SIPO_Write(byte, bool, bool skipmux = true);
void SIPO_WriteBYTE(byte, bool skipmux = true);
String getMuxAlias(int);
String getSipoAlias(int);
String centerFixed(String, int, int mode = 0);

void printSipo();
void setupPins();

class MuxSweeper
{
public:
    uint32_t last_sweep = 0;
    uint8_t delay_ms = 100;
    uint8_t num_of_reads = 10;
    uint16_t anResult[MUX_SIZE] = {0};
    bool dgResult[MUX_SIZE] = {0};
    int runtime = 0;
    bool debug = false;
    void partialSweep(int size, ...);
    void updatePin(uint8_t pin, bool digital = false);
    void clipPinValue(uint8_t, uint16_t, uint16_t);
    void Sweep();    
    void Print();
    bool &operator()(uint8_t index);
    uint16_t &operator[](uint8_t index);
};

extern MuxSweeper Mux;
/// @brief The Four MUX control pins.
extern const byte MUX_CONTROL[4];
/// @brief The current value of MUX.
extern byte MUX_VALUE;
/// @brief The current value of SIPO. 
extern byte SIPO_VALUE;

#endif