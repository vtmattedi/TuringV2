#pragma once

#include <Arduino.h>
#include "FileHandlers.h"
#include "HwInfo.h"
#include <../../include/Creds/WifiCred.h>

// Helper for debug
#define wll(varname) varname, WLI.getWlPinByUnit(varname)

/// @brief Configure which pins are used for the water level sensors
/// So you can have the middle, bottom and extra water level sensors 
/// in any order you want.
struct WATER_LEVEL_INFO
{
    uint8_t wl_0_pin = WL_BOTTOM;
    uint8_t wl_1_pin = WL_MIDDLE;
    uint8_t wl_2_pin = WL_EXTRA;
    bool inverse = true;
    int value = -012;
    void configureWl(int wl);
};

#define PREFS_SEPARATOR ";"
#define PREFS_FILENAME "/prefs.cfg"
#define PREFS_CHECKSUM_SEPARATOR "::"

struct Configurations
{
    WATER_LEVEL_INFO WLI;
    // prefered time to water the garden;  -1 = disabled.
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

    byte MQTT_interval = 60;
    String WifiSSID = WIFISSID;
    String WifiPASSWD = WIFIPASSWD;
    /* Make sure to save & load nescessary Configurations
     *
     */

    // Save user Configurations to file [sync order with load]
    void Save(String FileName = PREFS_FILENAME);
    /** Loads user Configurations from file [sync order with save]
     @param FileName the name of the file to be loaded */
    bool load(String FileName = PREFS_FILENAME);
    /** General function to set value of the Configurations
     * @param key the preference variable to be changed @param value the value to be assigned to the preference @return wheater or not key matched any available Configurations
     */
    bool SetValue(String key, int value);
    // Prints Current Values to serial
    String Print();

    // Returns the string representation of the Configurations
    String ToString();
    // Loads the Configurations from a string
    void FromString(const String &data);
};

struct FlagsHolder
{
    bool SD_Mounted = false;
    bool LittleFS_mounted = false;       // Flags to see if mount of SD and  SPIFFS were sucessful
    bool time_synced = false;            // Flags if time have been synced
    bool disable_sensors_update = false; // Flags wheater or not we want to upadate sensors values
};

struct Internals
{
    String pump_str_http = "";
    uint32_t sync_light = 0; // No Idea what does this do
    uint32_t boot_time = 0;
    uint8_t auto_ldr_resume_time = 0; // Time to resume the Auto dimming based on the LDR
    uint last_sensor_update = 0;      // unix timestamp in seconds of last sensor update.
    uint last_sensor_log = 0;         // unix timestamp in seconds of last sensor log.
    uint last_MQTT_update = 0;        // unix timestamp in seconds of last sensor log.
    FlagsHolder flags;
};

extern Internals control_variables;
extern Configurations user_preferences;
extern WATER_LEVEL_INFO WLI;
