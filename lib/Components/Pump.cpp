#include "Pump.h"

PumpController Pump;

uint32_t PumpController::predictNextRun()
{
  uint32_t next_run = last_auto_run + user_preferences.auto_pump_interval;
  if (user_preferences.prefered_pump_hour - hour(next_run) <= 4)
  {
    while (hour(next_run) < user_preferences.prefered_pump_hour)
      next_run += 60;
  }
  else
  {
    next_run += 4 * 3600;
  }
  return next_run;
}

/*Estimates the Water Level based on how much time the pump have been on.
 *@param Pump_runtime The amount of time (in ms) that the pump ran for.
 *@param forceState if > 0 forceState will force the level to 50 or 0 % the first time one of the water level sensors have been triggred
 *@param forceUsedPump if > 0 this will set the amount of run time to it
 */
void PumpController::CalcWaterLevel(int Pump_runtime, int forceState, int forceUsedPump)
{
  // Forces a Water_Level State;
  if (forceState >= 0)
  {
    Water_Level = forceState;
    run_time_from_50 = 0;
    return;
  }
  // Forces pump.UsedPumpTime
  if (forceUsedPump > 0)
  {
    run_time_from_50 = forceUsedPump;
  }
  // Since its leaking only supported for 50 to 0;
  if (Pump_runtime > 0 && WaterState == 1)
  {
    run_time_from_50 += Pump_runtime;
    if (LittleFS.exists("/water.txt"))
      LittleFS.remove("/water.txt");
    FileWrite("/water.txt", String(run_time_from_50), true, false);
  }
  Water_Level = 50 * WaterState - round((double)run_time_from_50 / PUMP_HALF_TO_0_TIME * 50);

  //  Serial.printf("{%d}{%d}{%d} --- %d - %d - %d\n",Pump_runtime,forceState,forceUsedPump,pump.WaterState,Water_Level, pump.UsedPumpTime);
}

// Starts the pump for a specified amount of time
// If the pump is already running or the time is 0 it will return false
// If the water level is 0 and the operation is not forced it will return false
// If the operation is forced it will start the pump regardless of the water level
bool PumpController::Start(int time, bool force)
{
  Serial.printf("[PUMP START] (%d) ms -f = %d\n", time, force);
  if (is_running || time == 0)
  {
    Serial.printf("PUMP ALREADY RUNNING\n");
    return false;
  }
  if (Water_Level == 0 && !force)
  {
    Serial.println("Water Low, Check reservoir or wiring.");
    return false;
  }
  // OLD:// digitalWrite(PUMP_RELAY, !HIGH);
  // SIPO_Write(RELAY_PIN, PUMP_ON);
  digitalWrite(RELAY_PIN, PUMP_ON);
  SIPO_Write(LED_0, HIGH);

  start_time = millis();
  Serial.println("PUMP TURNED ON");
  is_running = true;
  force_start = force;
  if (time > 0)
  {
    end_time = start_time + time;
    String Msg = "PUMP Starting [";
    Msg += time;
    Msg += "ms]\n";
  }
  else
    end_time = 0;
  return false;
}

// Stops the pump and logs the amount of time it was on
// If the pump is not running it will return false
bool PumpController::Stop()
{
  // SIPO_Write(RELAY_PIN, PUMP_OFF);
  digitalWrite(RELAY_PIN, PUMP_OFF);
  auto endtime = millis();
  SIPO_Write(LED_0, LOW);
  String Msg = "";
  if (force_start)
    force_start = false;
  int result = endtime - start_time;
  end_time = 0;
  // SIPO_Write(RELAY_PIN, PUMP_OFF); // Relay is active low
  Serial.printf("[%d] PUMP TURNED Off [%d]\n", caller, result);
  Log(String(result));
  // CalcWaterLevel(result);
  control_variables.pump_str_http = "Pump was on for ";
  control_variables.pump_str_http += result;
  control_variables.pump_str_http += "ms. ";
  control_variables.pump_str_http += " [";
  control_variables.pump_str_http += timestampToDateString(now(), OnlyTime);
  control_variables.pump_str_http += "]";
  is_running = false;
  return false;
};
// Checks wheater or not it is time to water the garden again checks for the prefered hour to do so or
// if it has been 4 hours since the timer has been triggered and the prefered hour have not been reached yet
bool PumpController::auto_pump_time_ajusted(uint32_t current_time)
{
  // Checks for bad prefered time
  if (user_preferences.prefered_pump_hour > 23 && user_preferences.prefered_pump_hour < -1)
    user_preferences.prefered_pump_hour = 19;
  // Waits for the prefered time after the interval has passed
  if (current_time - last_auto_run >= user_preferences.auto_pump_interval && hour() == user_preferences.prefered_pump_hour)
    return true;
  // If we are whithin 4 hours to the interval and at the prefered time we water the garden earlier
  if (current_time - last_auto_run >= user_preferences.auto_pump_interval - 4 * 60 * 60 && hour() == user_preferences.prefered_pump_hour)
    return true;
  // If the prefered time is disabled (-1) waters the garden at the interval
  if (current_time - last_auto_run >= user_preferences.auto_pump_interval && user_preferences.prefered_pump_hour == -1)
    return true;
  // If it has been 4 hours since the interval has passed and the prefered hour have not been reached we water the garden anyways
  if (current_time - last_auto_run >= user_preferences.auto_pump_interval + 4 * 60 * 60)
    return true;
  return false;
}

// Updates the water level and stops the pump if the water level is too low
// If the pump is running and the water level is too low it will stop the pump if operation was not forced
void PumpController::updateWaterLevel(uint8_t newWaterLevel)
{
  if (is_running)
  {
    if (newWaterLevel == 0 && !force_start)
    {
      Log("Water Level too low, Halting Pump.");
      Pump.Stop();
    }
  }

  if (Water_Level != newWaterLevel)
  {
    Log(formatString("Water Level change detected {%d} to {%d}",Water_Level, newWaterLevel));
    SIPO_Write(LED_2, bitRead(newWaterLevel, 0));
    SIPO_Write(LED_1, bitRead(newWaterLevel, 1));    
  }

  Water_Level = newWaterLevel;
}

/**
 * @brief Automatically runs the pump based on user preferences.
 *
 * This function checks the water level before starting the pump. If the water level is low, it prints a warning message and returns without starting the pump. If the water level is sufficient or if the operation is forced, it triggers the pump to start for a duration specified in the user preferences. Additionally, it logs the event and updates the last auto pump timestamp if the SPIFFS file system is mounted and the time is synchronized.
 *
 * @param force A boolean indicating whether to force the pump to start even if the water level is low.
 */
void PumpController::autoRun(bool force)
{
  Serial.printf("Auto Run Called!!!!\n");
  if (Water_Level == 0 && !force)
  {
    //Serial.printf("Low Water Level! check reservoir or wiring.\n");
    return;
  }
  last_auto_run = now();
  Start(user_preferences.auto_pump_duration, force);
  Log(formatString("Automation Pump Triggered [%d]", force));
  if (control_variables.flags.LittleFS_mounted && control_variables.flags.time_synced)
  {
    if (LittleFS.exists("/lastAutoPump.txt"))
    {
      LittleFS.remove("/lastAutoPump.txt");
    }
    FileWrite_LittleFS("/lastAutoPump.txt", String(last_auto_run));
    if (LittleFS.exists("/lastAutoPumpReadable.txt"))
    {
      LittleFS.remove("/lastAutoPumpReadable.txt");
    }
    FileWrite_LittleFS("/lastAutoPumpReadable.txt", timestampToDateString(last_auto_run));
  }
}

/**
 * Performs the automation process for the pump controller.
 *
 * This function checks the water level and the current state of the pump.
 * If the water level is low and the pump is running, it stops the pump.
 * If the pump is running and the specified duration has passed, it stops the pump.
 * If the pump is not running and it is time for an automatic run or a force auto is requested, it triggers the autoRun function.
 *
 * @param forceAuto A boolean indicating whether to force an automatic run or not.
 */
void PumpController::automation(bool forceAuto)
{
  // auto timer = millis() - last_loop;
  if (Water_Level == 0 && is_running && !force_start)
  {
    Stop();
  }

  if (is_running)
  {
    if (millis() > end_time && end_time > 0)
    {
      Stop();
    }
  }
  else
  {
    if (auto_pump_time_ajusted(now()) || forceAuto)
    {
      if (!timeSynced)
      {
        //Serial.printf("Auto pump Triggered but check for time sync first\n");
        return;
      }
     // Serial.printf("Last Auto Run = %d -> %d\n", now(), last_auto_run);
      autoRun(forceAuto);
    }
  }
  // Serial.printf("timer = %d ms\n",timer);
  // last_loop = millis();
}
