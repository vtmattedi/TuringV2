
#include "NightMareSensors.h"

Sensor *Sensorlist[SENSOR_ARRAY_SIZE];
SensorList Sensors;

void Sensor::init(String iD, String label, SensorDataType type, String unit)
{
    ID = iD;
    Label = label;
    Type = type;
    Unit = unit;
}
void Sensor::setStatus(SensorStatus newStatus)
{
    Status = newStatus;
}
void Sensor::setDebug(bool newDebug)
{
    debug = newDebug;
    Serial.printf("Debug Flag for [%s] is now %s\n", ID, debug ? "true" : "false");
}
void Sensor::updateValue(int newValue)
{
    if (debug)
        Serial.printf("[%s] - new Int: %d. currentValue: %d.\n", ID, newValue, Value_Int);
    Value_Int = newValue;
    modified_timestamp = now();
}
void Sensor::updateValue(float newValue)
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
void Sensor::updateValue(String newValue)
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
String Sensor::getValue()
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
String Sensor::print()
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
void Sensor::verifyTimestamp()
{
    if (Status != SensorStatus::Offline && now() - modified_timestamp >= timeout_seconds)
    {
        Status = SensorStatus::Offline;
        Serial.printf("[%s] timed out! age: %d secs, last update: %d\n", Label.c_str(), now() - modified_timestamp, modified_timestamp);
    }
}
void Sensor::report()
{
    if (_report)
    {
        String reportValue = getValue();
        Serial.printf("[%s] value: %s age: %d secs,\n", Label.c_str(), reportValue.c_str(), now() - modified_timestamp);
    }
}

void self_update_digital(SensorHWConfig *hw) {}
void self_update_analog(SensorHWConfig *hw) {}
void self_update_dht(SensorHWConfig *hw) {}
void self_update_ds1820b(SensorHWConfig *hw) {}
void self_update(SensorHWConfig *hw)
{
    if (hw->_sensorType == SensorType::Unknown_SENSOR)
    {
        return;
    }

    byte old_sipo_value = SIPO_VALUE;
    if (hw->_SIPO_ENABLE > 0)
        SIPO_Write(hw->_SIPO_ENABLE, hw->_SIPO_ENABLE_value);
    if (hw->_MUX_INDEX > 0)
        setMuxPort(hw->_MUX_INDEX);

    if (hw->_sensorType == SensorType::DS1820b_SENSOR)
    {
        self_update_ds1820b(hw);
    }
    else if (hw->_sensorType == SensorType::DHT_SENSOR)
    {
        self_update_dht(hw);
    }
    else if (hw->_sensorType == SensorType::ANALOG_READ)
    {
        self_update_analog(hw);
    }
    else if (hw->_sensorType == SensorType::DS1820b_SENSOR)
    {
        self_update_digital(hw);
    }
}

Sensor *SensorList::getSensorByIndex(int index)
{
    if (index >= 0 && index < SENSOR_ARRAY_SIZE)
    {
        return Sensorlist[index];
    }

    if (!suppressError)
        Serial.printf("No Sensor with Index:%d array has size: %d\n", index, SENSOR_ARRAY_SIZE);
    return Sensorlist[SENSOR_ARRAY_SIZE - 1];
}
Sensor *SensorList::getSensorById(String SensorID)
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

Sensor *SensorList::getSensorByLabel(String SensorLabel)
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

void SensorList::printAll()
{
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
        if (Sensorlist[i]->ID != "")
            Serial.println(Sensorlist[i]->print());
    }
}
bool SensorList::add(String iD, String label, SensorDataType type, String unit)
{
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
        if (Sensorlist[i]->ID == iD)
        {
            Serial.printf("A sensor with ID %s already exists\n", iD);
            return false;
        }
        if (Sensorlist[i]->ID == "")
        {
            Sensorlist[i]->init(iD, label, type, unit);
            Serial.printf("Sensor %s Created unit %s\n", iD.c_str(), unit.c_str());
            return true;
        }
    }

    Serial.println("SensorList is at Max capacity");
    return false;
}

void SensorList::reset()
{
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {

        if (Sensorlist[i])
            delete Sensorlist[i];
        Sensorlist[i] = new Sensor();
    }
}

String SensorList::printString()
{
    String msg = "";
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
        if (Sensorlist[i]->ID != "")
        {
            msg += Sensorlist[i]->print();
            msg += "\n";
        }
    }
    return msg;
}

/**
 * @brief Serializes the values of the sensors in the SensorList object.
 * 
 * @param pretty A boolean indicating whether to format the JSON output in a pretty way.
 * @return A String containing the serialized values of the sensors.
 */
String SensorList::serializeValues(bool pretty)
{
    String msg = "";

    DynamicJsonDocument doc(4096);
    // String iD, String label, SensorDataType type
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
        Sensor *s = Sensorlist[i];
        if (s->ID != "")
        {
            JsonObject id1 = doc[s->ID].to<JsonObject>();
            id1["label"] = s->Label;
            id1["id"] = s->ID;
            id1["type"] = s->Type;
            id1["unit"] = s->Unit;
        }
    }
    if (pretty)
        serializeJsonPretty(doc, msg);
    else
        serializeJson(doc, msg);

    return msg;
}

bool SensorList::loadSensors()
{
    Serial.printf("\n");
    Serial.print("Loading ");
    Serial.println(SENSORS_FILENAME);

    if (!LittleFS.exists(SENSORS_FILENAME))
    {
        Serial.printf("Cannot find the file.\n");
        return false;
    }
    String json = LittleFS.open(SENSORS_FILENAME, "r").readString();
    DynamicJsonDocument doc(4096);
    DeserializationError de = deserializeJson(doc, json);
    Serial.printf("Parse Result: %s\n", de.c_str());
    if (de != DeserializationError::Ok)
    {
        Serial.printf("Cannot parse the file.\n");
        return false;
    }

    for (JsonPair item : doc.as<JsonObject>())
    {
        String value_id = item.value()["id"];       // "ds18inner", "ds18prob", "moist", "moist2", "ldr", ...
        String value_label = item.value()["label"]; // "DS18 Inner", "DS18 Probe", "Moisture Sensor", ...
        int value_type = item.value()["type"];      // 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1
        String value_unit = item.value()["unit"];
        add(value_id, value_label, static_cast<SensorDataType>(value_type), value_unit);
    }
    Serial.printf("Sensors Loaded!\n");
    return true;
}

void SensorList::saveSensors()
{

    if (LittleFS.exists(SENSORS_FILENAME))
        LittleFS.remove(SENSORS_FILENAME);

    LittleFS.open(SENSORS_FILENAME, "a+", true).println(serializeValues());
    Serial.printf("Sensors Saved!\n");
}

void SensorList::timeoutInactives()
{
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
        if (Sensorlist[i]->ID != "")
        {
            Sensorlist[i]->verifyTimestamp();
        }
    }
}

void SensorList::report()
{
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
        if (Sensorlist[i]->ID != "")
        {
            Sensorlist[i]->report();
        }
    }
}

/// @brief Updates the modified timestamp of all active sensors in the sensor list.
///
/// This function iterates through the sensor list and updates the modified timestamp
/// of all active sensors. An active sensor is defined as a sensor with a non-empty ID
/// and a status other than Offline.
void SensorList::timeSynced()
{
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
        if (Sensorlist[i]->ID != "" && Sensorlist[i]->Status != Offline)
        {
            Sensorlist[i]->modified_timestamp = now();
        }
    }
}
