#pragma once

#include <Arduino.h>
#include "HwControl.h"
#include "Config.h"
#include "FileHandlers.h"
#include <NightMareNetwork.h>

/// Used to estimated the amount of water based on the amount of time the pump was on
#define PUMP_HALF_TO_0_TIME 147707

enum CommandSource
{
  _Serial = 10,
  _Blynk = 11,
  _TCP = 12,
  _HiveMQ = 13,
  _HTTP = 14
};

class PumpController
{
public:
  uint32_t start_time = 0;    // Millis() of when did the pump started
  bool is_running = false;    // is the pump currently running
  uint32_t end_time = 0;      // Millis() of when the pump is supposed to end
  uint32_t last_auto_run = 0; // Time stamp of last time that we auto ran the pump
  uint32_t last_loop = 0;
  bool force_start = false;     // forces the pump to run despites the Water Level sensors
  bool tank_0_percent = false;  // Prevents loops of low water warnings
  bool tank_50_percent = false; // Prevents loops of half water warnings
  int caller = -1;
  bool timeSynced = false; //Maybe do not run if time aint synced
  byte WaterState = 0;              // The State of the water sensors. overusage of memory this is equal to wl_empty + wl_half
  uint32_t run_time_from_50 = 0;    // Amount of time the pump was on from 50% to 0%
  uint32_t run_time_from_start = 0; // Amount of time the pump was on since manual reset
  byte Water_Level = 0;
  /// @brief Predicts the next activation of the pump
  /// @return The timestamp of the estimated next activation of the pump
  uint32_t predictNextRun();
  /*@brief Estimates the Water Level based on how much time the pump have been on.
   *@param Pump_runtime The amount of time (in ms) that the pump ran for.
   *@param forceState if > 0 forceState will force the level to 50 or 0 % the first time one of the water level sensors have been triggred
   *@param forceUsedPump if > 0 this will set the amount of run time to it
   */
  void CalcWaterLevel(int Pump_runtime, int forceState = -1, int forceUsedPump = -1);

  bool Start(int time = -1, bool force = false);
  bool Stop();
  // Checks wheater or not it is time to water the garden again checks for the prefered hour to do so or
  // if it has been 4 hours since the timer has been triggered and the prefered hour have not been reached yet
  bool auto_pump_time_ajusted(uint32_t current_time);

  void updateWaterLevel(uint8_t newWaterLevel);

  void autoRun(bool force = false);

  void automation(bool forceAuto = false);
};

extern PumpController Pump;
