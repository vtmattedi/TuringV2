#include "Configs.h"

Internals control_variables;
Configurations user_preferences;


static int8_t getWlPinByUnit(uint8_t unit)
{
    switch (unit)
    {
    case 0:
        return WL_BOTTOM;
        break;
    case 1:
        return WL_MIDDLE;
        break;
    case 2:
        return WL_EXTRA;
        break;

    default:
        return -1;
        break;
    }
}

void WATER_LEVEL_INFO::configureWl(int wl)
{
    Serial.printf("[Wl = %d] ", wl);
    if (wl == 0)
    {
        Serial.printf("no changes made\n");
        return;
    }
    int load_value = wl;
    bool inverted = wl < 0;
    wl = _abs(wl);

    int8_t res[] = {getWlPinByUnit(wl / 100),
                    getWlPinByUnit((wl % 100) / 10),
                    getWlPinByUnit(wl % 10)};
    //   Serial.printf("[0] %d -> %d\n", wl / 100, WLI.getWlPinByUnit(wl / 100));
    //   Serial.printf("[1] %d -> %d\n", wll((wl % 100) / 10));
    //   Serial.printf("[2] %d -> %d\n", wll(wl % 10));
    for (size_t i = 0; i < 3; i++)
    {
        if (res[i] < 0) // invalid
        {
            Serial.printf(" -wrong or invalid order. no changes made\n");
            return;
        }
    }
    wl_0_pin = res[0];
    wl_1_pin = res[1];
    wl_2_pin = res[2];
    value = load_value;
    inverse = inverted;
}

// Save user Configurations to file [sync order with load]
void Configurations::Save(String FileName)
{
    String prefs = "";
    prefs += prefered_pump_hour;
    prefs += PREFS_SEPARATOR;
    prefs += auto_pump_interval;
    prefs += PREFS_SEPARATOR;
    prefs += auto_pump_duration;
    prefs += PREFS_SEPARATOR;
    prefs += turn_off_lights_time;
    prefs += PREFS_SEPARATOR;
    prefs += AutoLDR;
    prefs += PREFS_SEPARATOR;
    prefs += sensor_update_interval;
    prefs += PREFS_SEPARATOR;
    prefs += sensor_log_interval;
    prefs += PREFS_SEPARATOR;
    prefs += enable_log;
    prefs += PREFS_SEPARATOR;
    prefs += WLI.value;
    prefs += PREFS_SEPARATOR;
    prefs += WifiSSID;
    prefs += PREFS_SEPARATOR;
    prefs += WifiPASSWD;
    prefs += PREFS_SEPARATOR;

    String msg = "";
    msg += checksum(prefs);
    msg += PREFS_CHECKSUM_SEPARATOR;
    msg += prefs;

    if (LittleFS.exists(FileName))
        LittleFS.remove(FileName);

    FileWrite_LittleFS(FileName, msg);
}

/** Loads user preferences from file [sync order with save]
 @param FileName the name of the file to be loaded */
bool Configurations::load(String FileName)
{
    if (!LittleFS.exists(FileName))
    {
        Serial.printf("Error loading user Configurations: file '%s' not found \n", FileName.c_str());
        return false;
    }
    String load_string = LittleFS.open(FileName).readString();
    if (atoi(load_string.substring(0, load_string.indexOf(PREFS_CHECKSUM_SEPARATOR)).c_str()) != checksum(load_string.substring(load_string.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR))))
    {
        Serial.printf("Error loading user Configurations: checksum did not match, possibly corrputed file.\n");
        Serial.printf("debug: loadstr = '%s', loadstrnum = '%s', checkstr = '%s', checksum = '%d'\n", load_string.c_str(), load_string.substring(0, load_string.indexOf(PREFS_CHECKSUM_SEPARATOR)).c_str(), load_string.substring(load_string.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR)).c_str(), (load_string.substring(load_string.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR))));
        return false;
    }
    load_string = load_string.substring(load_string.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR));
    int separator_index = 0;
    int last_separator_index = 0;
    String current_substring = "";

    for (size_t i = 0; separator_index >= 0; i++)
    {
        current_substring = "";
        separator_index = load_string.indexOf(PREFS_SEPARATOR, last_separator_index + 1);

        if (last_separator_index > 0)
            last_separator_index += strlen(PREFS_SEPARATOR);

        if (separator_index == -1)
            current_substring = load_string.substring(last_separator_index);
        else if (separator_index < load_string.length())
            current_substring = load_string.substring(last_separator_index, separator_index);

        last_separator_index = separator_index;

        if (i == 0)
            prefered_pump_hour = atoi(current_substring.c_str());
        else if (i == 1)
            auto_pump_interval = atoi(current_substring.c_str());
        else if (i == 2)
            auto_pump_duration = atoi(current_substring.c_str());
        else if (i == 3)
            turn_off_lights_time = atoi(current_substring.c_str());
        else if (i == 4)
            AutoLDR = atoi(current_substring.c_str());
        else if (i == 5)
            sensor_update_interval = atoi(current_substring.c_str());
        else if (i == 6)
            sensor_log_interval = atoi(current_substring.c_str());
        else if (i == 7)
            enable_log = atoi(current_substring.c_str());
        else if (i == 8)
            WLI.configureWl(atoi(current_substring.c_str()));
        else if (i == 9 && current_substring != "")
            WifiSSID = current_substring;
        else if (i == 10)
            WifiPASSWD = current_substring;
    }
    Serial.println("User Preferences Loaded");
    return true;
}

/** General function to set value of the Configurations
 * @param key the preference variable to be changed @param value the value to be assigned to the preference @return wheater or not key matched any available Configurations
 */
bool Configurations::SetValue(String key, int value)
{
    key.toLowerCase();
    if (key == "pump_hour")
        prefered_pump_hour = value;
    else if (key == "auto_pump_interval")
        auto_pump_interval = value;
    else if (key == "auto_pump_duration")
        auto_pump_duration = value;
    else if (key == "turn_off_lights")
        turn_off_lights_time = value;
    else if (key == "ldr")
        AutoLDR = value;
    else if (key == "update_interval")
        AutoLDR = value;
    else if (key == "log_interval")
        AutoLDR = value;
    else if (key == "log")
        AutoLDR = value;
    else
        return false;

    Save();
    return true;
}

// Prints Current Values to serial
String Configurations::Print()
{
    String response = "";
    response += "p. hour [pump_hour]: ";
    response += prefered_pump_hour;
    response += " | ap. interval  [auto_pump_interval]:";
    response += auto_pump_interval;
    response += " | ap. duration  [auto_pump_duration]:";
    response += auto_pump_duration;
    response += " | turn off time [turn_off_lights]:";
    response += turn_off_lights_time;
    response += " | Auto LDR [ldr]:";
    response += AutoLDR;
    response += " | Blynk interval [update_interval]:";
    response += sensor_update_interval;
    response += " | Log interval [log_interval]:";
    response += sensor_log_interval;
    response += " | Enable log [log]:";
    response += sensor_log_interval;
    response += "\n";
    return response;
}

String Configurations::ToString()
{
    String result;
    result += "prefered_pump_hour:" + String(prefered_pump_hour) + ";";
    result += "auto_pump_interval:" + String(auto_pump_interval) + ";";
    result += "auto_pump_duration:" + String(auto_pump_duration) + ";";
    result += "turn_off_lights_time:" + String(turn_off_lights_time) + ";";
    result += "AutoLDR:" + String(AutoLDR) + ";";
    result += "sensor_update_interval:" + String(sensor_update_interval) + ";";
    result += "sensor_log_interval:" + String(sensor_log_interval) + ";";
    result += "enable_log:" + String(enable_log) + ";";
    result += "WLI:" + String(WLI.value) + ";";
    result += "WifiSSID:" + WifiSSID + ";";
    result += "WifiPASSWD:" + WifiPASSWD + ";";

    return result;
}

void Configurations::FromString(const String &data)
{
    uint16_t pos = 0;
    while (pos < data.length())
    {
      int colonIndex = data.indexOf(':', pos);
      if (colonIndex == -1)
        break;

      int semicolonIndex = data.indexOf(';', colonIndex);
      if (semicolonIndex == -1)
        semicolonIndex = data.length();

      String key = data.substring(pos, colonIndex);
      String value = data.substring(colonIndex + 1, semicolonIndex);

      if (key.equals("prefered_pump_hour"))
        prefered_pump_hour = value.toInt();
      else if (key.equals("auto_pump_interval"))
        auto_pump_interval = value.toInt();
      else if (key.equals("auto_pump_duration"))
        auto_pump_duration = value.toInt();
      else if (key.equals("turn_off_lights_time"))
        turn_off_lights_time = value.toInt();
      else if (key.equals("AutoLDR"))
        AutoLDR = value.toInt();
      else if (key.equals("sensor_update_interval"))
        sensor_update_interval = value.toInt();
      else if (key.equals("sensor_log_interval"))
        sensor_log_interval = value.toInt();
      else if (key.equals("enable_log"))
        enable_log = value.toInt();
      else if (key.equals("WLI"))
        WLI.configureWl(value.toInt());
      else if (key.equals("WifiSSID"))
        WifiSSID = value;
      else if (key.equals("WifiPASSWD"))
        WifiPASSWD = value;
     

      pos = semicolonIndex + 1;
    }
}