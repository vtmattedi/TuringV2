#pragma once

#include <Arduino.h>
#include "HwControl.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

#define SENSOR_ARRAY_SIZE 20

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
    DIGITAL_READ = 3,
};

typedef std::function<String()> TManualUpdate;

struct SensorHWConfig
{
    int8_t _SIPO_ENABLE = 0;
    bool _SIPO_ENABLE_value = 0;
    int8_t _MUX_INDEX = 0;
    int8_t _pin_one = 0;
    int8_t _pin_two = 0;
    int8_t _helper_int8 = 0;
    SensorType _sensorType = SensorType::Unknown_SENSOR;
    TManualUpdate callback = nullptr;
};

class Sensor
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
    String Unit = "";
    int modified_timestamp = 0;
    int timeout_seconds = 300;

    float Value_Float = 0;
    int Value_Int = 0;
    String Value_String = "";
    bool debug = false;
    bool _report = false;

    void init(String iD, String label, SensorDataType type, String unit = "");
    void setStatus(SensorStatus);
    void setDebug(bool);
    void updateValue(int);
    void updateValue(float);
    void updateValue(String);
    String getValue();
    String print();
    void verifyTimestamp();
    void report();
};

void self_update_digital(SensorHWConfig *hw);
void self_update_analog(SensorHWConfig *hw);
void self_update_dht(SensorHWConfig *hw);
void self_update_ds1820b(SensorHWConfig *hw);
void self_update(SensorHWConfig *hw);

#define SENSORS_FILENAME "/sensorsConfigs.cfg"
#define SENSORS_OBJ_SEPARATOR ";"
#define SENSORS_DETAIL_SEPARATOR "::"
#define PREFS_CHECKSUM_SEPARATOR "::"

class SensorList
{
public:
    /// @brief If False will print errors
    bool suppressError = false;
    Sensor *getSensorByIndex(int index);
    Sensor *getSensorById(String SensorID);
    Sensor *getSensorByLabel(String SensorLabel);
    void printAll();
    bool add(String iD, String label, SensorDataType type, String unit = "");
    void reset();
    String printString();
    String serializeValues(bool pretty = false);
    bool loadSensors();
    void saveSensors();
    void timeSynced();
    void timeoutInactives();
    void report();
};

extern SensorList Sensors;
