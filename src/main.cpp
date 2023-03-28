// Make sure to have these definition on top of your code or else Blynk won't be able to connect.
#include <D:\Projects\PIO\Turing\include\BlynkCred.h>

#define PROTO // Toggle comment to switch between upload to Turing or Jupiter

#ifndef PROTO
#define BLYNK_TEMPLATE_ID MYBLYNKTEMPLATE
#define BLYNK_DEVICE_NAME MYBLYNKDEVICE
#define BLYNK_AUTH_TOKEN MYBLYNKAUTH
#define DEVICE_NAME "Turing"
#endif
#ifdef PROTO
#define BLYNK_TEMPLATE_ID MYBLYNKTEMPLATE_PROTO
#define BLYNK_DEVICE_NAME MYBLYNKDEVICE_PROTO
#define BLYNK_AUTH_TOKEN MYBLYNKAUTH_PROTO
#define DEVICE_NAME "Jupiter"
#endif

#define USE_SPIFFS

// Comment this out to disable prints and save space
#define DEBUG
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESPmDNS.h>
#include "OneWire.h"
#include "DallasTemperature.h"
#include <D:\Projects\PIO\Turing\lib\Time-master\Time.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include "esp_task_wdt.h"
#ifdef USE_SPIFFS
#include <SPIFFS.h>
#include <SPI.h>
#include "FS.h"
#endif
#include <D:\Projects\PIO\Turing\include\WifiCred.h>

#include "DHT.h"

#pragma region FastLED

#include <FastLED.h>
#include <D:\Projects\PIO\Turing\lib\NewPing\src\NewPing.h>

// 2 WS2811 lights
// 1 WS2818b strip

// WS2811
#define WS2811_SIZE 2
#define WS2811_PIN 26
#define WS2811_COLOR_ORDER BGR

// best white is TypicalPixelString next is uncorrected

// WS2818b Strip
#define LED_STRIP_SIZE 300
#define LED_STRIP_PIN 23
#define LED_STRIP_COLOR_ORDER GRB

#define LED_MAX_AMPS
#define LED_VOLTAGE
#define TOTALSIZE LED_STRIP_SIZE + WS2811_SIZE
uint16_t LED_BaseColor_1 = 0x0; // Contains an RGB value for the strip
uint16_t LED_BaseColor_2 = 0x0; // Contains an RGB value for the stripw
int patternIndex = 0;

CRGB leds[TOTALSIZE];

enum PatternMode
{
  blink,
  blinktwo,
  rainbow,
  movingrainbow,
  scaleup,
  scaledown,
  solidColor,
  pump,
  randomMode,
  sinWaves,
  fillColors,
  randomFill
};

struct Pattern
{

  unsigned long lastMillis = 0;
  int interval = 25;
  int Index1 = 0;
  int Index2 = 0;
  int maxIndex = 20;
  byte currenthue = 0;
  CRGB baseColor = CRGB::Red;
  CRGB baseColor2 = CRGB::Blue;
  CRGB baseColor3 = CRGB::Green;
  PatternMode mode = PatternMode::scaleup;
  PatternMode oldMode = PatternMode::scaleup;
  bool backwards = false;
  bool progbar_is_running = false;

  void setMode(PatternMode newMode, CRGB color1 = 0x0, CRGB color2 = 0x0)
  {
    oldMode = mode;
    mode = newMode;
    lastMillis = 0;

    if (mode == PatternMode::pump)
    {
      Index1 = Index1 / 2;
      interval = 25;
      backwards = false;
    }
    else if (mode == PatternMode::scaleup)
    {
      Index1 = 0;
      interval = 25;
      backwards = false;
    }
    else if (mode == PatternMode::randomMode)
    {
      Index1 = 0;
      interval = 25;
      backwards = false;
    }
    else if (mode == PatternMode::sinWaves)
    {
      interval = 15;
      Index1 = 0;
    }
    else if (mode == PatternMode::fillColors)
    {
      interval = 25;
      Index1 = 0;
    }
    else if (mode == PatternMode::solidColor)
    {
      baseColor = color1;
      interval = 500;
    }
    else if (mode == PatternMode::randomFill)
    {
      interval = 15;
      Index1 = 0;
      backwards = false;
    }
  }

  void run()
  {
    if (millis() < lastMillis + interval || progbar_is_running)
      return;
    lastMillis = millis();
    // interval = map(Index1,maxIndex,0,15,75);

    if (mode == PatternMode::scaleup)
    {
      int index = Index1;
      if (backwards)
        index = maxIndex - 1 - Index1;
      int pos = 0;
      pos = map(beat8(120, 0), 0, 255, 0, maxIndex - 1); //,59,0,0);
                                                         // if (backwards)
                                                         ///  pos = map(beat8(40,0),255,0,0,maxIndex-1);
      leds[index] = CHSV(map(Index1, 0, maxIndex, currenthue, currenthue + maxIndex), 255, 127);
      Index1++;
      fade_raw(leds, maxIndex, 16);

      if (Index1 >= maxIndex)
      {
        Index1 = 0;
        backwards = !backwards;
        currenthue += 19;
      }
    }
    else if (mode == PatternMode::pump)
    {
      int index = 30 + Index1;
      int index2 = 29 - Index1;
      if (backwards)
      {
        index = maxIndex - 1 - Index1;
        index2 = 0 + Index1;
      }
      byte b = map(Index1, 0, maxIndex / 2, 128, 170);
      byte c = map(Index1, 0, maxIndex / 2, 170, 128);
      leds[index] = CHSV(b, 255, 127);
      leds[index2] = CHSV(b, 255, 127);
      Index1++;
      fade_raw(leds, maxIndex, 16);
      if (Index1 >= maxIndex / 2)
      {
        Index1 = 0;
        backwards = !backwards;
      }
    }
    else if (mode == PatternMode::randomMode)
    {
      int index = random8(0, maxIndex - 1);
      if (backwards)
        index = maxIndex - 1 - Index1;
      int pos = 0;
      pos = map(beat8(120, 0), 0, 255, 0, maxIndex - 1); //,59,0,0);
                                                         // if (backwards)
                                                         ///  pos = map(beat8(40,0),255,0,0,maxIndex-1);
      leds[index] = CHSV(map(Index1, 0, maxIndex, currenthue, currenthue + maxIndex), 255, 127);
      Index1++;
      fade_raw(leds, maxIndex, 16);

      if (Index1 >= maxIndex)
      {
        Index1 = 0;
        // backwards = !backwards;
        currenthue = random8();
      }
    }
    else if (mode == PatternMode::blink)
    {
      CRGB b[LED_STRIP_SIZE];
      for (size_t i = 0; i < maxIndex; i++)
      {
        if (i >= maxIndex - 1)
          b[0] = leds[i];
        else
          b[i + 1] = leds[i];
      }
      for (size_t i = 0; i < LED_STRIP_SIZE; i++)
      {
        leds[i] = b[i];
      }
    }
    else if (mode == PatternMode::sinWaves)
    {
      int pos = beatsin8(15, 0, maxIndex - 1);
      int pos2 = beatsin8(15, 0, maxIndex - 1, 0, 127);
      int pos3 = beatsin8(10, 0, maxIndex - 1) + beatsin8(20, 0, maxIndex - 1, 0, 75);
      leds[pos2] = CHSV(currenthue, 255, 127);
      leds[pos] = CHSV(currenthue + 85, 255, 127);
      leds[pos3 / 2] = CHSV(currenthue + 169, 255, 127);
      blur1d(leds, maxIndex, 16);
      // blur1d(leds, maxIndex, 16);
      fade_raw(leds, maxIndex, 16);
      if (pos >= maxIndex - 1)
      {
        currenthue = random8();
      }
    }
    else if (mode == PatternMode::fillColors)
    {
      int index = Index1;
      if (backwards)
        index = maxIndex - 1 - Index1;
      leds[index] = CHSV(currenthue, 255, 127);
      // blur1d(leds, maxIndex, 16);
      //  fade_raw(,maxIndex,16);
      FastLED.setBrightness(beatsin8(10, 50, 170, 0, 127));
      Index1++;
      if (Index1 >= maxIndex)
      {
        Index1 = 0;
        backwards = !backwards;
        currenthue = random8();
      }
    }
    else if (mode == PatternMode::solidColor)
    {
      for (size_t i = 0; i < maxIndex; i++)
      {
        leds[i] = baseColor;
      }
    }
    else if (mode == PatternMode::randomFill)
    {
      leds[Index1] = CHSV(currenthue, 255, 127);
      if (backwards)
        Index1--;
      else
        Index1++;
      if ((Index1 >= maxIndex - 1 && !backwards) || (Index1 <= 0 && backwards))
      {
        backwards = !backwards;
        uint8_t newhue = currenthue + 65;
        // while(currenthue - newhue < 10)
        // {
        //   newhue = random8();
        // }
        currenthue = newhue;
      }
    }
    FastLED.show();
  }

  void progBar(byte percentage, byte numLED, CRGB ValueColor = CRGB::DarkGreen, CRGB BackColor = CRGB::DarkRed, int delayMS = 4000)
  {
    progbar_is_running = true;
    byte oldBrightness = FastLED.getBrightness();
    FastLED.setBrightness(255);
    FastLED.clear();
    if (percentage > 100)
      percentage = 100;
    if (numLED > maxIndex)
      numLED = maxIndex;
    for (size_t i = 0; i < numLED; i++)
    {
      leds[i] = BackColor;
    }
    FastLED.show();
    int value = round((double)percentage / 100 * numLED);
    for (size_t i = 0; i <= value; i++)
    {
      leds[i] = ValueColor;
      delay(50);
      FastLED.show();
    }
    delay(delayMS);
    FastLED.clear();
    FastLED.setBrightness(oldBrightness);
    progbar_is_running = false;
  }
};

Pattern p;
// PatternMode oldMode = PatternMode::blink;
void setBeaconColor(CRGB COLOR, bool skipShow = false)
{
  leds[TOTALSIZE - 1] = COLOR;
  leds[TOTALSIZE - 2] = COLOR;
  if (!skipShow)
    FastLED.show();
}

#pragma endregion

// bool autoBrightLDR = true;
char auth[] = BLYNK_AUTH_TOKEN;

#define DHT_PIN 17
#define DHT_TYPE DHT11

// #define RAIN_SENSOR 33
#define MOISTURE_SENSOR 33
#define LDR_PIN 32
#define PUMP_RELAY 25
#define EMPTY_WATER_LEVEL 18
#define HALF_WATER_LEVEL 19
#define DS18_BUS 16
#define DS18_READS 10
#define ONBOARD_LED_PIN 5
#define VBAT_PIN 35

#ifdef PROTO
// TEST BUTTON
#define TEST_BUTTON 0
bool digital_debounce = false;
#endif

#define MAX_HISTORY_SIZE 100

OneWire oneWire(DS18_BUS);        // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature DS18(&oneWire); // Pass our oneWire reference to Dallas Temperature.

DHT dht(DHT_PIN, DHT_TYPE);

#pragma region TCP server

#define MAX_CLIENTS 10 // Maximum active simultaneous clients.
// If this is changed, be aware of the memory limitations
// Also, note that for clients connected after this limit
// There is already a system in place to liberate availability
// (timeout)

WiFiServer wifiServer(100); // Creates the tcp server object at port 100

// You may change the default Timeout for inactive clients connected and if its disable by default
// It is not recomended to disable it because it can only listen to MAX_CLIENTS clients at a time.
int SET_TIMEOUT = 300;
bool DISABLE_TIMEOUT = false;

enum TransmissionMode
{
  ThreeCharHeaders = 2,
  SizeColon = 1,
  AllAvailable = 0
};

String PrepareMsg(String msg, TransmissionMode mode, String Headers = "");

struct NightMareTCPClient
{
  // The actual TCP Object
  WiFiClient *client = NULL;
  // Last Time the client sent anything
  int clientsTimeout = 0;
  // Client Status {If a message Z;* is Recieved then it is stored here}
  String clientsStatus = "";
  // Client ID
  String clientsID = "";
  // If Clients
  bool clientsRequestUpdates = false;
  // The Time Stamp for the Status String
  int clientsStatusTime = 0;
  // Set the Nightmare TCP transmission mode. ThreeCharHeaderis not supported here
  TransmissionMode transmissionMode = TransmissionMode::AllAvailable;

  // Sends The msg formatted according to the transmission mode
  void send(String msg)
  {

    if (client != NULL)
    {
      String m = PrepareMsg(msg, transmissionMode);
      Serial.println(m);
      client->print(m);
    }
  }

  // Resets The internal variables
  void reset()
  {
    client = NULL;
    clientsTimeout = 0;
    clientsStatus = "";
    clientsID = "";
    clientsRequestUpdates = false;
    clientsStatusTime = 0;
    transmissionMode = TransmissionMode::AllAvailable;
  }
};

NightMareTCPClient clients[MAX_CLIENTS];

// Set the Nightmare TCP transmission mode. ThreeCharHeaderis not supported here
// Transmission Char for ThreeHeader protocols
char TransmissionChar = '!';

#pragma endregion

#pragma region Sensors

bool is_time_synced = false;         // True if time was synced false if not.
uint16_t sensor_update_interval = 1; // Sets the interval for refreshing the sensor data in seconds.
uint16_t sensor_log_interval = 3600; // Sets the interval for refreshing the sensor data in seconds.
int last_sensor_update = 0;          // unix timestamp in seconds of last sensor update.
int last_sensor_log = 0;             // unix timestamp in seconds of last sensor log.
bool wl_half = false;                // water level half.
bool wl_empty = false;               // water level empty.
byte Water_Level = 0;                // Water tank % accounted by the 2 sensors.
float DS18inner = 0;                 // Value of inner temperature sensor in Celcius.
float DS18Probe = 0;                 // Value of DS18 waterproof probe temperature sensor in Celsius.
uint32_t Last_Auto_Pump = 0;         // Time stamp of last time the pump was on;
// int Auto_pump_interval = 172800;     // 172800; Time in seconds between each pump activation time 172800 s = 48 hours
// int Auto_pump_timer = 10000;         // Time in ms for the pump to stay on each time it is automatically triggered

#define MAX_MOISTURE_1_READ 3716
#define MOISTURE_READS 10           // number of analog reads for average
byte Moisture_1 = 0;                // Moisture probe 1 % in range - 0 to MAX_MOISTURE_1_READ
float innerTemp = 0, probeTemp = 1; // DS18 temps in Celsius
float DHT11_temp = 0, DHT11_hum = 0;

uint16_t LDR_VALUE = 0;

// not used
uint16_t Rain_level = 0;
int light = 0;
int vbat = 0;
u_int last_time_pump_was_on = 0;

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

struct Sensor
{
  SensorStatus Status = Unknown;
  SensorDataType Type = Type_String;
  String ID = "";
  String Label = "";
  int modified_timestamp = 0;

  float Value_Float = 0;
  int Value_Int = 0;
  String Value_String = "";
  bool debug = false;

  void Setup(String iD, String label, SensorDataType type)
  {
    ID = iD;
    Label = label;
    Type = type;
  }

  void setStatus(SensorStatus newStatus)
  {
    Status = newStatus;
  }

  void setDebug(bool newDebug)
  {
    debug = newDebug;
    Serial.printf("Debug Flag for [%s] is now %s\n", ID, debug ? "true" : "false");
  }

  void UpdateValue(int newValue)
  {
    if (debug)
      Serial.printf("[%s] - new Int: %d. currentValue: %d.\n", ID, newValue, Value_Int);
    Value_Int = newValue;
    modified_timestamp = now();
  }
  void UpdateValue(float newValue)
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
  void UpdateValue(String newValue)
  {
    if (debug)
      Serial.printf("[%s] - new String: %s. currentValue: %s.\n", ID, newValue, Value_String);
    Value_String = newValue;
    modified_timestamp = now();
  }
  String getValue()
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
  String Print()
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
};

#define SENSOR_ARRAY_SIZE 12
Sensor *Sensorlist[SENSOR_ARRAY_SIZE];

struct SensorList
{

  Sensor *getSensorById(String SensorID)
  {
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID == SensorID)
        return Sensorlist[i];
    }

    Serial.printf("No Sensor with ID:%s\n", SensorID);
    return Sensorlist[SENSOR_ARRAY_SIZE - 1];
  }

  Sensor *getSensorByLabel(String SensorLabel)
  {
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID == SensorLabel)
        return Sensorlist[i];
    }
    Serial.printf("No Sensor with Label:%s\n", SensorLabel);
    return Sensorlist[SENSOR_ARRAY_SIZE - 1];
  }

  void PrintAll()
  {
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID != "")
        Serial.println(Sensorlist[i]->Print());
    }
  }

  bool Add(String iD, String label, SensorDataType type)
  {
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID == iD)
      {
        Serial.printf("A sensor with ID %s already exists\n", iD);
        return false;
      }
    }
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID == "")
      {
        Sensorlist[i]->Setup(iD, label, type);
        Serial.printf("Sensor %s Created\n", iD);
        return true;
      }
    }
    Serial.println("SensorList is at Max capacity");
    return false;
  }

  void reset()
  {
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      Sensorlist[i] = new Sensor();
    }
  }

  String PrintString()
  {
    String msg = "";
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID != "")
      {
        msg += Sensorlist[i]->Print();
        msg += "\n";
      }
    }
    return msg;
  }

  String SerializeValues()
  {
    String msg = "";
    for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
    {
      if (Sensorlist[i]->ID != "")
      {
        msg += Sensorlist[i]->ID;
        msg += ";";
        msg += Sensorlist[i]->getValue();
        msg += ";";
      }
    }
    return msg;
  }
};

SensorList Sensors;

#pragma endregion

uint pump_start_timestamp = 0;                         // Millis() of when did the pump started
bool pump_is_running = false;                          // is the pump currently running
bool pump_prog_end = false;                            // Wheater or not the pump is supposed to stop based on the timer
uint pump_end = 0;                                     // Millis() of when the pump is supposed to end
String pump_last_run = "Never On";                     // String for the Blynk V10
bool pump_force = false;                               // forces the pump to run despites the Water Level sensors
bool low_water_triggered = false;                      // Prevents loops of low water warnings
bool is_SD_mounted = false, is_SPIFFS_mounted = false; // Flags to see if mount of SD and  SPIFFS were sucessful
bool fifty_water_marker = false;                       // Prevents loops of half water warnings

#define PUMP_HALF_TO_0_TIME 147707
byte WaterState = 0;   // The State of the water sensors. overusage of memory this is equal to wl_empty + wl_half
uint UsedPumpTime = 0; // Amountof time the pump was on from 50% to 0%

bool SendWifiResults = false; // Flag to send the results from the wifi scan once its done;
bool oldWifiStatus = false;   // Flag to not keep wirting to the digital pin ????

struct Internals
{
  uint sync_light = 0;
  bool auto_ldr_overriden = false;
  uint8_t aut_ldr_resume_time = 0;
  bool turn_off_lights_flag = false;
};

Internals control_variables;
int trigPin = 26;
int echoPin = 27;
int lastState = 0;
NewPing sonar(trigPin, echoPin); // NewPing setup of pins and maximum distance.

struct SonarHandler
{ 
  long sonar_hist[10];
};


// Writes the content to the speficied file
// @param FileName The file to be written @param Message Content to be Written @param useSPIFFS True for SPIFFS and false for SD @param timestamp Add timestamp at the beginning of the content @param newLine Write with new line at the end
void FileWrite(String FileName, String Message, bool useSPIFFS = true, bool timestamp = true, bool newLine = true)
{
  File logfile;
  String Msg = "";

  if (timestamp)
  {
    Msg += dayShortStr(dayOfWeek(now()));
    Msg += ", ";
    if (day() < 10)
      Msg += "0";
    Msg += day();
    Msg += "/";
    if (month() < 10)
      Msg += "0";
    Msg += month();
    Msg += " ";
    if (hour() < 10)
      Msg += "0";
    Msg += hour();
    Msg += ":";
    if (minute() < 10)
      Msg += "0";
    Msg += minute();
  }

  Msg += Message;

  if (is_SPIFFS_mounted && useSPIFFS)
  {
    double usage = SPIFFS.usedBytes() / SPIFFS.totalBytes();
    if (usage > 90)
    {
      Blynk.logEvent("spiffs_full", String(usage));
      Serial.println("SPIFFS is full.");
    }
    logfile = SPIFFS.open(FileName, "a", true);
    if (newLine)
      logfile.println(Msg);
    else
      logfile.print(Msg);
    logfile.close();
  }
  else if (is_SD_mounted && !useSPIFFS)
  {
    Serial.println("not implemented");
  }
}

/** Returns the sum of all characteres in the string
 @param string_to_test String to be summed */
int checksum(String string_to_test)
{
  int sum = 0;
  for (size_t i = 0; i < string_to_test.length(); i++)
  {
    sum += string_to_test[i];
  }
  return sum;
}

String listAllFiles(String dir = "/", bool SD_card = false, byte treesize = 0)
{
  String response = "";
  /// SPIFFS mode
  if (!SD_card)
  {
    // List all available files (if any) in the SPI Flash File System
    if (!is_SPIFFS_mounted)
    {
      response += "SPIFFS not Mount";
      return response;
    }
    response += "Used Bytes: ";
    response += SPIFFS.usedBytes();
    response += "-----Total Bytes: ";
    response += SPIFFS.totalBytes();
    response += "-----Used: ";
    response += map(SPIFFS.usedBytes(), 0, SPIFFS.totalBytes(), 0, 100);
    response += "%\n";
    response += "Listing files in: ";
    response += dir;
    response += "\n";
    fs::File root = SPIFFS.open(dir);
    fs::File file = root.openNextFile();
    while (file)
    {
      for (size_t i = 0; i < treesize; i++)
      {
        response += "--";
      }
      response += "FILE: ";
      response += file.name();
      response += " size: ";
      response += (float)file.size() / 1024;
      response += "Kb\n";
      if (file.isDirectory())
        response += listAllFiles(file.name(), false, treesize + 1);
      file = root.openNextFile();
    }
    root.close();
    file.close();
  }

  return response;
}

struct preferences
{
  // prefered time to water the garden; -1 = disabled.
  int8_t prefered_pump_hour = 19;
  // 172800; Time in seconds between each pump activation time 172800 sec = 48 hours
  int auto_pump_interval = 172800;
  // Time in ms for the pump to stay on each time it is automatically triggered
  uint16_t auto_pump_duration = 10000;
  // Time to turnoff lights when using AutoBrightnessLDR; -1 = disabled.
  int8_t turn_off_lights_time = -1;
  //  Control the lights brightness using the LDR
  bool AutoLDR = true;

  /* Make sure to save & load nescessary preferences
   *
   */

#define PREFS_SEPARATOR ";"
#define PREFS_FILENAME "/prefs.cfg"
#define PREFS_CHECKSUM_SEPARATOR "::"
  // Save user preferences to file [sync order with load]
  void Save(String FileName = PREFS_FILENAME)
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

    String msg = "";
    msg += checksum(prefs);
    msg += PREFS_CHECKSUM_SEPARATOR;
    msg += prefs;

    if (SPIFFS.exists(FileName))
      SPIFFS.remove(FileName);

    FileWrite(FileName, msg, true, false, false);
  }

  /** Loads user preferences from file [sync order with save]
   @param FileName the name of the file to be loaded */
  void load(String FileName = PREFS_FILENAME)
  {
    if (!SPIFFS.exists(FileName))
    {
      Serial.printf("Error loading user preferences: file '%s' not found \n", FileName.c_str());
      return;
    }
    String load_string = SPIFFS.open(FileName).readString();
    if (atoi(load_string.substring(0, load_string.indexOf(PREFS_CHECKSUM_SEPARATOR)).c_str()) != checksum(load_string.substring(load_string.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR))))
    {
      Serial.printf("Error loading user preferences: checksum did not match, possibly corrputed file.\n");
      Serial.printf("debug: loadstr = '%s', loadstrnum = '%s', checkstr = '%s', checksum = '%d'\n", load_string.c_str(), load_string.substring(0, load_string.indexOf(PREFS_CHECKSUM_SEPARATOR)).c_str(), load_string.substring(load_string.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR)).c_str(), (load_string.substring(load_string.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR))));
      return;
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
    }
    Serial.println("User Preferences Loaded");
  }

  /** General function to set value of the preferences
   * @param key the preference variable to be changed @param value the value to be assigned to the preference @return wheater or not key matched any available preferences
   */
  bool SetValue(String key, int value)
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
    else
      return false;

    Save();
    return true;
  }

  // Prints Current Values to serial
  String Print()
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
    response += "\n";
    return response;
  }
};

preferences user_preferences;

struct command_result
{
  bool result = false;
  String response = "";
};

command_result handleCommand(String message, char *delimiter = " ", String extra_arg = "")
{
  command_result result;
  String response = "";
  bool resolve = true;
  String command = "";
  String args[5] = {"", "", "", "", ""};

  String current_string = "";

  int index = 0;
  int last_index = 0;

  // gets command and args using the delimiters
  for (size_t i = 0; index >= 0; i++)
  {
    current_string = "";
    index = message.indexOf(delimiter, last_index + 1);

    if (last_index > 0)
      last_index += strlen(delimiter);

    if (index == -1)
      current_string = message.substring(last_index);
    else if (index < message.length())
      current_string = message.substring(last_index, index);

    last_index = index;

    if (i == 0)
    {
      current_string.toUpperCase();
      command = current_string;
    }
    else if (i < 6)
      args[i - 1] = current_string;
  }

  // process the command
  command.toUpperCase();
  if (command == "Help" || command == "help" || command == "H" || command == "h")
  {
    // Do documentation
    response += ("Welcome to NightMare Home Systems ©\nThis is a ESP32 Module and it can answer to the following commands:\n");
    response += ("Quick obs.: the character int [19] or char () is ignored when recieved for facilitating reasons.");

    response += "REQ_CLIENTS     > Gets the current state of available variables\n";
    response += "REQ_FILE [file] > Toggles the LIGH_RELAY state\n";
    response += "REQ_FILES       > Sets the TIMEOUT value for \n";
    response += "REQ_LOG         > requests a software reset.\n";
    response += "PREF            > requests a software reset.\n";
    response += "GET_PREFS       > requests a software reset.\n";
  }
  else if (command == "REQ_CLIENTS")
  {
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
      if (NULL != clients[i].client)
      {
        response += "Client [";
        response += i;
        response += "], IP: ";
        response += clients[i].client->remoteIP().toString();
        response += ", (self) ID:";
        response += clients[i].clientsID;
        response += ", INFO: ";
        response += clients[i].clientsStatus[i];
        response += "\n";
      }
    }
  }
  else if (command == "REQ_FILE")
  {
    if (is_SPIFFS_mounted)
    {
      if (SPIFFS.exists(args[0]))
      {
        response += SPIFFS.open(args[0]).readString();
      }
      else
      {
        response = "File: '";
        response += args[0];
        response += "' does not exist";
        resolve = false;
      }
    }
    else
    {
      response = "SPIFFS not mount";
      resolve = false;
    }
  }
  else if (command == "REQ_FILES")
  {
    if (is_SPIFFS_mounted)
    {
      response = listAllFiles("/");
    }
    else
    {
      response = "SPIFFS not mount";
      resolve = false;
    }
  }
  else if (command == "REQ_LOG")
  {
    response = Sensors.SerializeValues();
  }
  else if (command == "PREF")
  {
    int key = atoi(args[1].c_str());
    if (key == 0 && args[1] == "0" || key != 0)
    {
      if (user_preferences.SetValue(args[0], key))
      {
        response = "Success. ";
        response += args[0];
        response += " is now: ";
        response += key;
      }
      else
      {
        response = "Preference: '";
        response += args[0];
        response += "' was not found";
        resolve = false;
      }
    }
    else
    {
      response = "key: '";
      response += args[1];
      response += "' is in the wrong format";
      resolve = false;
    }
  }
  else if (command == "GET_PREFS")
  {
    response += user_preferences.Print();
    response += "\n";
  }
  else if (command == "GET_IP")
  {
    response += "Current IP: ";
    response += WiFi.localIP().toString();
    response += "\n";
  }
  else if (command == "PUMP_INFO")
  {
    response += "Last Auto Pump  ";
    if (SPIFFS.exists("/lastAutoPumpReadable.txt"))
      response += SPIFFS.open("/lastAutoPumpReadable.txt").readString();
    response += " || Next: ";
    response += Last_Auto_Pump + user_preferences.auto_pump_interval;
    response += " || Pump Run Time: ";
    response += UsedPumpTime;
  }
  else if (command == "TRIGGER_US")
  {
    pinMode(echoPin, INPUT);
    int reads = atoi(args[0].c_str());
    if (reads < 1 || reads > 20 )
    reads = 5; 
    int oldMillis = millis();        // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
    int dist = sonar.ping_median(reads); // Send ping, get distance in cm and print result (0 = outside set distance range)
    int timespent = millis() - oldMillis;
    float result = (float)dist / US_ROUNDTRIP_CM;
    response = "dist: ";
    response += result;
    response += "cm, reads: ";
    response += reads;
    response += ", time: ";
    response += timespent;
    response += "ms";

  }
  else
    resolve = false;

  result.response = response;
  result.result = resolve;

  Serial.printf("message = %s | command = %s | args[0] = %s | args[1] = %s | args[2] = %s |\n", message.c_str(), command.c_str(), args[0].c_str(), args[1].c_str(), args[1].c_str());

  return result;
}

// Calculates the water level
void CalcWaterLevel(int Pump_runtime, int forceState = -1, int forceUsedPump = -1)
{
  // Forces a Water_Level State;
  if (forceState >= 0)
  {
    Water_Level = forceState;
    UsedPumpTime = 0;
  }
  // Forces UsedPumpTime
  if (forceUsedPump > 0)
  {
    UsedPumpTime = forceUsedPump;
  }
  // Since its leaking only supported for 50 to 0;
  if (Pump_runtime > 0 && WaterState == 1)
  {
    UsedPumpTime += Pump_runtime;
    if (SPIFFS.exists("/water.txt"))
      SPIFFS.remove("/water.txt");
    FileWrite("/water.txt", String(UsedPumpTime), true, false);
  }
  Water_Level = 50 * WaterState - round((double)UsedPumpTime / PUMP_HALF_TO_0_TIME * 50);
  Sensors.getSensorById("waterlevel")->UpdateValue(Water_Level);
  Blynk.virtualWrite(V6, Water_Level);
  //  Serial.printf("{%d}{%d}{%d} --- %d - %d - %d\n",Pump_runtime,forceState,forceUsedPump,WaterState,Water_Level, UsedPumpTime);
}

// Starts the pump, if time < 0 without time to stop, time = 0: error, time > 0: the time in ms for the pump to run
void StartPump(int time = -1)
{
  if (pump_is_running || time == 0)
  {
    Blynk.virtualWrite(V9, "Pump already running\n");
    return;
  }
  if (Water_Level == 0 && !pump_force)
  {
    Blynk.virtualWrite(V9, "Water Low, Check reservoir or wiring.\n");
    return;
  }
  p.setMode(PatternMode::pump);
  pump_start_timestamp = millis();
  digitalWrite(PUMP_RELAY, !HIGH);
  Serial.println("PUMP TURNED ON");
  pump_is_running = true;
  if (time > 0)
  {
    pump_end = pump_start_timestamp + time;
    String Msg = "PUMP Starting [";
    Msg += time;
    Msg += "ms]\n";
    Blynk.virtualWrite(V9, Msg);
    pump_prog_end = true;
  }
  else
    pump_prog_end = false;
  Blynk.virtualWrite(V10, "Pump Running");
}

void StopPump(bool halt = false)
{
  String Msg = "";
  if (halt == true && !pump_force)
  {
    digitalWrite(PUMP_RELAY, !LOW);
    Msg += "[halted]";
  }
  if (millis() < pump_end && pump_prog_end && !halt || !pump_is_running)
    return;

  int result = millis() - pump_start_timestamp;
  digitalWrite(PUMP_RELAY, !LOW); // Relay is active low
  Serial.printf("PUMP TURNED Off [%d]\n", result);
  Msg += "PUMP was on for ";
  Msg += result;
  Msg += "ms.\n";
  Blynk.virtualWrite(9, Msg);
  Msg = "Last time the Pump was activated: ";
  if (day() < 10)
    Msg += "0";
  Msg += day();
  Msg += "/";
  if (month() < 10)
    Msg += "0";
  Msg += month();
  Msg += " ";
  if (hour() < 10)
    Msg += "0";
  Msg += hour();
  Msg += ":";
  if (minute() < 10)
    Msg += "0";
  Msg += minute();
  FileWrite("/log.txt", String(result));
  CalcWaterLevel(result);
  Blynk.virtualWrite(10, Msg);
  pump_is_running = false;
  p.setMode(p.oldMode);
}

#pragma region BLYNK

uint16_t Blynk_upload_interval = 60;   // Sets the interval for uploading values to Blynk in seconds
int Blynk_last_upload = 0;             // Keeps the timestamp of the last time it sent data over to Blynk
uint16_t Blynk_telemetry_interval = 5; // Sets the interval for the telemetry data to Blynk in seconds
int Blynk_telemetry_last = 0;          // Keeps the timestamp of the last time it sent telemetry over to Blynk
bool Blynk_telemetry = false;          // Enable/disables Blynk Telemetry

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable

  if (user_preferences.AutoLDR)
  {
    user_preferences.AutoLDR = false;
    control_variables.auto_ldr_overriden = true;
    uint8_t new_hour = hour() + 12;
    if (new_hour > 23)
      new_hour = new_hour - 24;
    control_variables.aut_ldr_resume_time = new_hour;
  }
  int valueV0 = param.asInt();
  Serial.printf("[V0][%d][Blynk] >> Value Recieved = %d\n", now(), valueV0);
  digitalWrite(ONBOARD_LED_PIN, valueV0);
  FastLED.setBrightness(valueV0 * 0xFF);
}

BLYNK_WRITE(V8)
{
  // Set incoming value from pin V0 to a variable
  int valueV8 = param.asInt();
  Serial.printf("[V8][%d][Blynk] >> Value Recieved = %d\n", now(), valueV8);
  if (valueV8)
    StartPump();
  else
    StopPump();
}

BLYNK_WRITE(V9)
{
  // Set incoming value from pin V0 to a variable
  String valueV9 = param.asStr();
  Serial.printf("[V9][%d][Blynk] >> Value Recieved = %s \n", now(), valueV9);

  if (is_SPIFFS_mounted)
  {
    if (valueV9 == "del logs")
    {
      SPIFFS.remove("/log.txt");
      Blynk.virtualWrite(V9, "logs cleaned");
    }
    else if (valueV9 == "del auto")
    {
      SPIFFS.remove("/lastAutoPump.txt");
      Blynk.virtualWrite(V9, "last auto pump reset");
      Last_Auto_Pump = 0;
    }
    else if (valueV9 == "del sensors")
    {
      SPIFFS.remove("/sensors.txt");
      Blynk.virtualWrite(V9, "sensors log were deleted.");
      last_sensor_log = 0;
    }
    else if (valueV9 == "format")
    {
      bool result = SPIFFS.format();
      String msg = "SPIFFS formatted = ";
      msg += result;
      Blynk.virtualWrite(V9, msg);
    }
    else if (valueV9 == "usage")
    {
      double result = SPIFFS.usedBytes() / SPIFFS.totalBytes();
      String msg = "SPIFFS usage = ";
      msg += SPIFFS.usedBytes();
      msg += " of ";
      msg += SPIFFS.totalBytes();
      msg += "Bytes [";
      msg += result;
      msg += "%]";
      Blynk.virtualWrite(V9, msg);
    }
    else if (SPIFFS.exists(valueV9))
      Blynk.virtualWrite(V9, SPIFFS.open(valueV9).readString());
  }
  if (valueV9[0] == '$')
    StartPump(atoi(valueV9.substring(1).c_str()));
  else if (valueV9[0] == '#')
  {
    String msg = "Value Recieved: ";
    if (valueV9[1] == '#')
    {

      msg += " [B] - 0x";
      int newbrightness = strtol(valueV9.substring(2).c_str(), NULL, 16);
      msg += String(newbrightness, HEX);
      if (newbrightness >= 0 && newbrightness < 256)
      {
        FastLED.setBrightness(newbrightness);
      }
      else
      {
        msg += " | Invlaid value";
      }
    }
    else
    {
      msg += " [C] - 0x";
      int newColor = strtol(valueV9.substring(1).c_str(), NULL, 16);
      msg += String(newColor, HEX);
      Serial.print("Color : ");
      Serial.println(newColor, HEX);
      //  Blynk.virtualWrite(V9, newColor);
      // fill_solid(leds, 2, newColor);
      setBeaconColor(newColor);
      p.setMode(solidColor, newColor);
    }
    Blynk.virtualWrite(V9, msg);
    FastLED.show();
  }
  else if (valueV9[0] == '&')
  {
    int sampleSize = strtol(valueV9.substring(1).c_str(), NULL, 16);
    int ldr = 0;
    for (size_t i = 0; i < sampleSize; i++)
    {
      ldr += analogRead(LDR_PIN);
    }
    if (sampleSize > 0)
      ldr = ldr / sampleSize;

    Blynk.virtualWrite(V9, String(ldr));
  }
  else if (valueV9[0] == '!')
  {
    command_result result = handleCommand(valueV9.substring(1));
    Blynk.virtualWrite(V9, result.response.c_str());
  }
  else if (valueV9 == "scan wifi")
  {
    Serial.println("Async Wifi Scan Requested");
    Blynk.virtualWrite(V9, "Async Wifi Scan Requested");
    WiFi.scanNetworks(true);
    SendWifiResults = true;
  }
  else if (valueV9 == "autoldr")
  {
    user_preferences.AutoLDR = !user_preferences.AutoLDR;
    String msg = "Auto Brightness based on LDR is now: ";
    msg += user_preferences.AutoLDR;
    Blynk.virtualWrite(V9, msg);
  }
  else if (valueV9 == "ldr")
  {
    Blynk.virtualWrite(V9, String(analogRead(LDR_PIN)));
  }
  else if (valueV9 == "sensors")
  {
    Blynk.virtualWrite(V9, Sensors.PrintString());
  }
  else if (valueV9 == "telemetry")
  {
    Blynk_telemetry = !Blynk_telemetry;
    String msg = "Telemetry is now: ";
    msg += Blynk_telemetry ? "enable.\n" : "disable.\n";
    Blynk.virtualWrite(V9, msg);
  }
  else if (valueV9 == "pump")
  {
    String msg = "Last time pump was Activated: ";
    msg += Last_Auto_Pump;
    Blynk.virtualWrite(V9, msg);
  }
  else if (valueV9 == "moist")
  {
    digitalWrite(27, 1);
    String msg = "Read moisture: ";
    msg += analogRead(MOISTURE_SENSOR);
    msg += ".\n";
    Blynk.virtualWrite(V9, msg);
    digitalWrite(27, 0);
  }
  else if (valueV9 == "spiffs")
  {
    String msg = "";
    if (!is_SPIFFS_mounted)
    {
      msg = "SPIFFS not Mounted!\n";
    }
    else
    {
      msg += "SPIFFS: ";
      msg += (float)SPIFFS.usedBytes() / 1024;
      msg += "/";
      msg += (float)SPIFFS.totalBytes() / 1024;
      msg += " [kb] [";
      msg += (float)SPIFFS.usedBytes() / SPIFFS.totalBytes();
      msg += "%]\n";

      fs::File root = SPIFFS.open("/");
      fs::File file = root.openNextFile();
      while (file)
      {
        msg += ("file: ");
        msg += (file.name());
        msg += (" || size: ");
        msg += ((float)file.size() / 1024);
        msg += (" [Kb]");
        if (file.isDirectory())
        {
          msg += " [dir]";
        }
        msg += "\n";
        file = root.openNextFile();
      }
      root.close();
      file.close();
    }

    Blynk.virtualWrite(V9, msg);
  }
}

BLYNK_WRITE(V20) // RED
{
  int valueV20 = param.asInt();
  Serial.printf("[V20][%d][Blynk] >> Value Recieved = %d\n", now(), valueV20);
  p.setMode(PatternMode::solidColor, CRGB(valueV20, p.baseColor.g, p.baseColor.b));
}
BLYNK_WRITE(V21) // GREEN
{
  int valueV21 = param.asInt();
  Serial.printf("[V21][%d][Blynk] >> Value Recieved = %d\n", now(), valueV21);
  p.setMode(PatternMode::solidColor, CRGB(p.baseColor.r, valueV21, p.baseColor.b));
}
BLYNK_WRITE(V22) // BLUE
{
  int valueV22 = param.asInt();
  Serial.printf("[V22][%d][Blynk] >> Value Recieved = %d\n", now(), valueV22);
  p.setMode(PatternMode::solidColor, CRGB(p.baseColor.r, p.baseColor.g, valueV22));
}
BLYNK_CONNECTED()
{
}

#pragma endregion

#pragma region OTA

void startOTA()
{
  String type;
  // is_updating = true;
  //  caso a atualização esteja sendo gravada na memória flash externa, então informa "flash"
  if (ArduinoOTA.getCommand() == 0)
    type = "flash";
  else                   // caso a atualização seja feita pela memória interna (file system), então informa "filesystem"
    type = "filesystem"; // U_SPIFFS
  // exibe mensagem junto ao tipo de gravação
  Serial.println("Start updating " + type);
}
// exibe mensagem
void endOTA()
{
  Serial.println("\nEnd");
}
// exibe progresso em porcentagem
void progressOTA(unsigned int progress, unsigned int total)
{
  // update_progress = (float)progress / total * 100;
  Serial.printf("Progress: %u%%\r\n", (progress / (total / 100)));
}

void errorOTA(ota_error_t error)
{
  Serial.printf("Error[%u]: ", error);
  if (error == OTA_AUTH_ERROR)
    Serial.println("Auth Failed");
  else if (error == OTA_BEGIN_ERROR)
    Serial.println("Begin Failed");
  else if (error == OTA_CONNECT_ERROR)
    Serial.println("Connect Failed");
  else if (error == OTA_RECEIVE_ERROR)
    Serial.println("Receive Failed");
  else if (error == OTA_END_ERROR)
    Serial.println("End Failed");
}

#pragma endregion

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16)
      Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

#pragma region TCP

String HandleMsg(String msg, NightMareTCPClient &_client)
{
#ifdef DEBUG
  Serial.print("[");
  Serial.print(_client.client->remoteIP());
  Serial.printf("]:%d ", _client.client->remotePort());
  Serial.print((char)39);
  Serial.print(msg);
  Serial.println((char)39);
#endif

  String response = "";

  if (msg == "Help" || msg == "help" || msg == "H" || msg == "h")
  {
    response += ("Welcome to NightMare Home Systems ©\nThis is a ESP32 Module and it can answer to the following commands:\n");
    response += ("Quick obs.: the character int [19] or char () is ignored when recieved for facilitating reasons.");
    response += ("'A' > Gets the current state of available variables\n'L' > Toggles the LIGH_RELAY state\n");
    response += ("'T;******;' > Sets the TIMEOUT value for the tcp server.[Replace '******' with a long.\n");
    response += ("'SOFTWARE_RESET' requests a software reset.");
  }
  else if (msg == "REQ_CLIENTS")
  {
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
      if (NULL != clients[i].client)
      {
        response += "Client [";
        response += i;
        response += "], IP: ";
        response += clients[i].client->remoteIP().toString();
        response += ", (self) ID:";
        response += clients[i].clientsID;
        response += ", INFO: ";
        response += clients[i].clientsStatus[i];
        response += "\n";
      }
    }
  }
  else if (msg == "REQ_UPDATES")
  {
    _client.clientsRequestUpdates = !_client.clientsRequestUpdates;
    response = "Your REQ_UPDATES flag is now: ";
    if (_client.clientsRequestUpdates)
      response += true;
    else
      response += false;
  }
  else if (msg == "REQ_UPDATES_1")
  {
    response += "Your REQ_UPDATES flag is now: true";
  }
  else if (msg == "REQ_UPDATES_0")
  {
    _client.clientsRequestUpdates = false;
    response += "Your REQ_UPDATES flag is now: false";
  }
  else if (msg == "TMODE=0" || msg == "7:TMODE=0")
  {
    _client.transmissionMode = TransmissionMode::AllAvailable;
    response = "Transmission mode set to AllAvailable";
  }
  else if (msg == "TMODE=1" || msg == "7:TMODE=1")
  {
    _client.transmissionMode = TransmissionMode::SizeColon;
    response = "Transmission mode set to SizeColon";
  }
  else if (msg == "REQ_SENSORS_FILE")
  {
    if (is_SPIFFS_mounted)
    {
      if (SPIFFS.exists("/sensors.txt"))
      {
        if (_client.client != NULL)
        {
          String msg = "$";
          msg += SPIFFS.open("/sensors.txt").size();
          msg += "$";
          _client.client->print(msg);
          _client.client->print(SPIFFS.open("/sensors.txt").readString().c_str());
        }
      }
    }
  }
  else if (msg == "REQ_LOG_FILE")
  {
    if (is_SPIFFS_mounted)
    {
      if (SPIFFS.exists("/log.txt"))
      {
        if (_client.client != NULL)
        {
          String msg = "$";
          msg += SPIFFS.open("/log.txt").size();
          msg += "$";
          _client.client->print(msg);
          _client.client->print(SPIFFS.open("/log.txt").readString().c_str());
        }
      }
    }
  }
  else if (msg == "REQ_LOG")
  {
    response = Sensors.SerializeValues();
  }
  else if (msg[0] == 'Z') // Client Message (Available to broadcast)
  {
    _client.clientsStatus = msg;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
      if (clients[i].clientsRequestUpdates)
      {
        if (NULL != clients[i].client)
          clients[i].send(msg);
      }
    }
    response += "M;ACK;";
  }
  else if (msg[0] == 'A') // Current Variables
  {
    response += ("\nCurrent SET_TIMEOUT value is: ");
    response += (SET_TIMEOUT);
    response += (" and it is: ");
    if (DISABLE_TIMEOUT)
      response += ("Disabled");
    else
      response += ("Enabled");
    response += ("\n");
  }
  else if (msg[0] == 'I') // Self-Indentification
  {
    for (int i = 1; i < msg.length(); i++)
    {
      char c = msg[i];
      if (c == ';')
      {
        i = msg.length();
      }
      else
        _client.clientsID += c;
    }
    response += "M;ACK;";
  }
  else if (msg[0] == '$')
  {
    response += "Sensors Label: [";
    response += Sensors.getSensorByLabel(msg.substring(1))->Label;
    response += "] value:";
    response += Sensors.getSensorByLabel(msg.substring(1))->getValue();
  }
  else if (msg[0] == '@')
  {
    if (msg[1] == '@')
    {
      int pin = atoi(msg.substring(2).c_str());
      response = "Reading [Analog] from pin [";
      response += pin;
      response += "]. ||";
      if (pin == 33)
        digitalWrite(27, HIGH);
      for (size_t i = 0; i < 10; i++)
      {
        response += " {";
        response += analogRead(pin);
        response += "}";
      }

      if (pin == 33)
        digitalWrite(27, LOW);
    }
    else
    {
      int pin = atoi(msg.substring(1).c_str());
      response = "Reading [DIGITAL] from pin [";
      response += pin;
      response += "]. || {";
      response += digitalRead(pin);
      response += "}";
    }
  }
  else if (msg[0] == '!')
  {
    command_result result = handleCommand(msg.substring(1));
    // response = "[";
    // response += result.result;
    // response += "] ";
    response += result.response;
  }
  else
    response += "M;ACK;";
  return response;
}

String PrepareMsg(String msg, TransmissionMode mode, String headers)
{
  if (mode == TransmissionMode::AllAvailable)
    return msg; // SendMsg
  else if (mode == TransmissionMode::SizeColon)
  {
    String r_msg = "";
    r_msg += msg.length();
    r_msg += ":";
    r_msg += msg;
    return r_msg;
  }
  else if (mode == TransmissionMode::ThreeCharHeaders)
  {
    String r_msg = "";
    r_msg += TransmissionChar;
    r_msg += headers;
    r_msg += msg;
    return r_msg;
  }
  return msg;
}

void handleTCP()
{
  // polls for new clients
  WiFiClient newClient = wifiServer.available();
  if (newClient)
  {
    Serial.println("new client connected");
    // Find the first unused space
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
      if (NULL == clients[i].client)
      {
        clients[i].client = new WiFiClient(newClient);
        Serial.printf("client [");
        Serial.print(newClient.remoteIP().toString());
        Serial.printf(":%d] allocated at (%d)\n", newClient.remotePort(), i);
        clients[i].clientsTimeout = now();
        clients[i].send("REQ_ID");
        break;
      }
      else if (i == MAX_CLIENTS)
      {
#ifdef DEBUG
        Serial.printf("Could not allocate client [");
        Serial.print(newClient.remoteIP().toString());
        Serial.printf(":%d] all (%d) clients spaces are beeing used right now\n", newClient.remotePort(), MAX_CLIENTS);
#endif
      }
    }
  }

  // Check whether each client has some data
  for (int i = 0; i < MAX_CLIENTS; ++i)
  {
    // If the client is in use, and has some data...
    if (NULL != clients[i].client && clients[i].client->available())
    {

      String msg = "";
      clients[i].clientsTimeout = now();
      int size = 0;
      int index = 0;
      bool sizeFound = false;
      while (NULL != clients[i].client && clients[i].client->available())
      {
        char newChar = clients[i].client->read();

#ifdef FAST_LIGHT_TOGGLE
        if (newChar == CONTROL_CHAR)
          Light_Toggle();
#endif

        if (newChar != (char)19)
        {
          if (sizeFound == 0 && newChar == ':' && clients[i].transmissionMode == TransmissionMode::SizeColon)
          {
            size = atoi(msg.c_str());
            sizeFound = true;
            msg = "";
            index = 0;
          }
          else if (sizeFound == 1 && index >= size && clients[i].transmissionMode == TransmissionMode::SizeColon)
          {
            if (msg != "")
              clients[i].send(HandleMsg(msg, clients[i]));
            sizeFound = false;
            msg = "";
            msg += newChar;
          }
          else
          {
            index++;
            msg += newChar;
          }
        }
      }
      clients[i].send(HandleMsg(msg, clients[i]));
    }
    // If a client disconnects, clear the memory for a new one
    if (NULL != clients[i].client && !clients[i].client->connected())
    {
      clients[i].client->stop();
      delete clients[i].client;
      clients[i].reset();
    }
    // check if any client have been idle and time it out
    else if (NULL != clients[i].client && clients[i].client->connected())
    {
      if ((now() - clients[i].clientsTimeout >= SET_TIMEOUT) and !DISABLE_TIMEOUT)
      {
        clients[i].send("E;0x3F;Timedout;");
#ifdef DEBUG
        Serial.printf("Client[%d] timeouted\n", i);
#endif
        clients[i].client->stop();
        delete clients[i].client;
        clients[i].reset();
      }
    }
  }
}

void BroadcastMessage(String msg)
{
  for (int i = 0; i < MAX_CLIENTS; i++)
  {
    if (clients[i].clientsRequestUpdates)
    {
      if (NULL != clients[i].client)
        clients[i].send(msg);
    }
  }
}

#pragma endregion

#define SENSORS_FILENAME "/sensorsConfigs.cfg"
#define SENSORS_OBJ_SEPARATOR ";"
#define SENSORS_DETAIL_SEPARATOR "::"

bool loadSensors()
{
  Serial.print("Loading ");
  Serial.println(SENSORS_FILENAME);
  if (!SPIFFS.exists(SENSORS_FILENAME))
  {
    Serial.println("'/sensorsConfigs.cfg' not found");
    return false;
  }

  String config = SPIFFS.open(SENSORS_FILENAME).readString();

  if (atoi(config.substring(0, config.indexOf(PREFS_CHECKSUM_SEPARATOR)).c_str()) != checksum(config.substring(config.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR))))
  {
    Serial.printf("Error loading user preferences: checksum did not match, possibly corrputed file.\n");
    Serial.printf("debug: loadstr = '%s', loadstrnum = '%s', checkstr = '%s', checksum = '%d'\n", config.c_str(), config.substring(0, config.indexOf(PREFS_CHECKSUM_SEPARATOR)).c_str(), config.substring(config.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR)).c_str(), checksum(config.substring(config.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR))));
    return false;
  }

  config = config.substring(config.indexOf(PREFS_CHECKSUM_SEPARATOR) + strlen(PREFS_CHECKSUM_SEPARATOR));

  int last_sensor_index = 0;
  int sensor_index = 0;
  String sensor_string = "";

  for (size_t i = 0; sensor_index >= 0; i++)
  {
    sensor_string = "";
    sensor_index = config.indexOf(SENSORS_OBJ_SEPARATOR, last_sensor_index + 1);

    if (last_sensor_index > 0)
      last_sensor_index += strlen(SENSORS_OBJ_SEPARATOR);

    if (sensor_index == -1)
      sensor_string = config.substring(last_sensor_index);
    else if (sensor_index < config.length())
      sensor_string = config.substring(last_sensor_index, sensor_index);

    int last_detais_index = 0;
    int details_index = 0;
    String current_detail = "";
    String sensor_ID = "";
    String sensor_label = "";
    SensorDataType sensor_Type = Type_String;

    for (size_t j = 0; details_index >= 0; j++)
    {
      current_detail = "*";
      details_index = sensor_string.indexOf(SENSORS_DETAIL_SEPARATOR, last_detais_index + 1);
      if (last_detais_index > 0)
        last_detais_index += strlen(SENSORS_DETAIL_SEPARATOR);

      if (details_index == -1)
        current_detail = sensor_string.substring(last_detais_index);
      else if (details_index < sensor_string.length())
        current_detail = sensor_string.substring(last_detais_index, details_index);
      // Serial.printf("i: %d j: %d s: %s\n",i,j,current_detail.c_str()); //debug
      last_detais_index = details_index;

      if (j == 0)
      {
        sensor_ID = current_detail;
      }
      else if (j == 1)
      {
        sensor_label = current_detail;
      }
      else if (j == 2)
      {
        if (current_detail == "0")
          sensor_Type = SensorDataType::Type_Int;
        else if (current_detail == "1")
          sensor_Type = SensorDataType::Type_Float;
      }
    }

    if (sensor_string.length() > 0)
    {
      // Serial.printf("%s || %s || %d \n", sensor_ID.c_str(), sensor_label.c_str(), sensor_Type);
      Sensors.Add(sensor_ID, sensor_label, sensor_Type);
    }
    last_sensor_index = sensor_index;
  }

  return true;
}

void saveSensors()
{
  if (SPIFFS.exists(SENSORS_FILENAME))
    SPIFFS.remove(SENSORS_FILENAME);

  String msg = "";
  for (size_t i = 0; i < SENSOR_ARRAY_SIZE; i++)
  {
    if (Sensorlist[i]->ID != "")
    {
      msg += Sensorlist[i]->ID;
      msg += SENSORS_DETAIL_SEPARATOR;
      msg += Sensorlist[i]->Label;
      msg += SENSORS_DETAIL_SEPARATOR;
      msg += Sensorlist[i]->Type;
      msg += SENSORS_OBJ_SEPARATOR;
    }
  }
  String file = "";
  file += checksum(msg);
  file += PREFS_CHECKSUM_SEPARATOR;
  file += msg;

  FileWrite(SENSORS_FILENAME, file, true, false, false);
}

// Checks wheater or not it is time to water the garden again checks for the prefered hour to do so or
// if it has been 4 hours since the timer has been triggered and the prefered hour have not been reached yet
bool auto_pump_time_ajusted(int current_time)
{
  // Checks for bad prefered time
  if (user_preferences.prefered_pump_hour > 23 && user_preferences.prefered_pump_hour < -1)
    user_preferences.prefered_pump_hour = 19;
  // Waits for the prefered time after the interval has passed
  if (current_time - Last_Auto_Pump >= user_preferences.auto_pump_interval && hour() == user_preferences.prefered_pump_hour)
    return true;
  // If we are whithin 4 hours to the interval and at the prefered time we water the garden earlier
  if (current_time - Last_Auto_Pump >= user_preferences.auto_pump_interval - 4 * 60 * 60 && hour() == user_preferences.prefered_pump_hour)
    return true;
  // If the prefered time is disabled (-1) waters the garden at the interval
  if (current_time - Last_Auto_Pump >= user_preferences.auto_pump_interval && user_preferences.prefered_pump_hour == -1)
    return true;
  // If it has been 4 hours since the interval has passed and the prefered hour have not been reached we water the garden anyways
  if (current_time - Last_Auto_Pump >= user_preferences.auto_pump_interval + 4 * 60 * 60)
    return true;
  return false;
}

void updateValues(bool skipUpload = false)
{
  int current_time = now();

  if (current_time - last_sensor_update >= sensor_update_interval)
  {

    DS18.requestTemperatures();
    last_sensor_update = current_time;

    DS18inner = DS18.getTempCByIndex(0);

    if (DS18inner == -127 || DS18inner == -255 || DS18inner == 85)
      Sensors.getSensorById("ds18inner")->setStatus(Offline);
    else
      Sensors.getSensorById("ds18inner")->UpdateValue(DS18inner);

    DS18Probe = DS18.getTempCByIndex(1);
    if (DS18Probe == -127 || DS18Probe == -255 || DS18Probe == 85)
      Sensors.getSensorById("ds18prob")->setStatus(Offline);
    else
    {
      Sensors.getSensorById("ds18prob")->UpdateValue(DS18Probe);
    }

    wl_half = !digitalRead(HALF_WATER_LEVEL);
    wl_empty = !digitalRead(EMPTY_WATER_LEVEL);

    if (wl_half + wl_empty != WaterState)
    {
      WaterState = wl_half + wl_empty;
      CalcWaterLevel(0, 50 * WaterState);
    }

    digitalWrite(27, 1);
    int totalMoisture = 0;

    for (size_t i = 0; i < MOISTURE_READS; i++)
    {
      totalMoisture += analogRead(MOISTURE_SENSOR);
    }

    totalMoisture = totalMoisture / 10;
    Moisture_1 = map(totalMoisture, 0, MAX_MOISTURE_1_READ, 0, 100);
    Sensors.getSensorById("moist")->UpdateValue(Moisture_1);

    int totalLDR = 0;
    for (size_t i = 0; i < 10; i++)
    {
      totalLDR += analogRead(LDR_PIN);
    }
    LDR_VALUE = totalLDR / 10;
    Sensors.getSensorById("ldr")->UpdateValue(LDR_VALUE);

    digitalWrite(27, 0);

    DHT11_hum = dht.readHumidity();
    DHT11_temp = dht.readTemperature();
    Sensors.getSensorById("dht_hum")->UpdateValue(DHT11_hum);
    Sensors.getSensorById("dht_temp")->UpdateValue(DHT11_temp);

    //   Serial.printf("%f  -   %f \n",DHT11_hum,DHT11_temp);
    if (current_time - last_sensor_log >= sensor_log_interval && !skipUpload)
    {
      if (is_SPIFFS_mounted)
      {
        String msg = "";
        // Creates the file if it doest exist
        if (!SPIFFS.exists("/sensors.txt"))
        {
          msg += "Time,TimeTimestamp gmt-03,DS18inner,DS18Probe,WaterLevel,Moisture,DHT_H,DHT_T,LDR";
          FileWrite("/sensors.txt", msg);
        }
        msg = ",";
        msg += current_time;
        msg += ",";
        msg += DS18inner;
        msg += ",";
        msg += DS18Probe;
        msg += ",";
        msg += Water_Level;
        msg += ",";
        msg += Moisture_1;
        msg += ",";
        msg += DHT11_hum;
        msg += ",";
        msg += DHT11_temp;
        msg += ",";
        msg += LDR_VALUE;
        msg += ",";
        FileWrite("/sensors.txt", msg);
      }
      last_sensor_log = current_time;
    }
    if (current_time - Blynk_telemetry_last >= Blynk_telemetry_interval && !skipUpload && Blynk_telemetry)
    {
      String tmsg = "Z;";
      tmsg += DS18inner;
      tmsg += ";";
      tmsg += DS18Probe;
      tmsg += ";";
      tmsg += wl_half;
      tmsg += ";";
      tmsg += wl_empty;
      tmsg += ";";
      tmsg += Moisture_1;
      tmsg += ";";
      tmsg += WiFi.RSSI();
      tmsg += ";";
      tmsg += DHT11_hum;
      tmsg += ";";
      tmsg += DHT11_temp;
      tmsg += ";";
      tmsg += LDR_VALUE;
      tmsg += ";\n";
      Blynk.virtualWrite(9, tmsg);
      Blynk_telemetry_last = current_time;
    }
    if (current_time - Blynk_last_upload >= Blynk_upload_interval && !skipUpload)
    {
      Blynk_last_upload = current_time;
      // Serial.println(analogRead(VBAT_PIN));
      // Serial.println(current_time);
      // Serial.println(current_temp);
      // Serial.println(vbat);
      // Serial.print(Water_Level);
      // Serial.println("%");
      // Serial.println(Rain);
      // Serial.println(sensors_hist.Print(10));
      // sensors_hist.PrintAvgTemp();
      Blynk.virtualWrite(V6, Water_Level);
      Blynk.virtualWrite(V4, DS18inner);
      Blynk.virtualWrite(V5, DS18Probe);
      Blynk.virtualWrite(V7, Moisture_1);
      Blynk.virtualWrite(V11, DHT11_hum);
      Blynk.virtualWrite(V12, DHT11_temp);
      Blynk.virtualWrite(V14, LDR_VALUE);
    }
  }

  if (auto_pump_time_ajusted(current_time))
  {
    Last_Auto_Pump = current_time;
    StartPump(user_preferences.auto_pump_duration);
    FileWrite("/log.txt", "Automation Pump Triggered");
    if (is_SPIFFS_mounted && is_time_synced)
    {
      if (SPIFFS.exists("/lastAutoPump.txt"))
      {
        SPIFFS.remove("/lastAutoPump.txt");
      }
      FileWrite("/lastAutoPump.txt", String(current_time), true, false);
      if (SPIFFS.exists("/lastAutoPumpReadable.txt"))
      {
        SPIFFS.remove("/lastAutoPumpReadable.txt");
      }
      FileWrite("/lastAutoPumpReadable.txt", String(" "), true, true, false);
    }
  }
}

void getTime()
{
  Serial.println("Syncing Time Online");
  HTTPClient http;
  http.begin("http://worldtimeapi.org/api/timezone/America/Bahia.txt"); // HTTP
  int httpCode = http.GET();
  // httpCode will be negative on error
  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    // file found at server
    if (httpCode == HTTP_CODE_OK)
    {
      Serial.printf("[HTTP] OK... code: %d\n", httpCode);
      String payload = http.getString();
      char str[payload.length() + 1];
      strcpy(str, payload.c_str());
      char *pch;
      pch = strtok(str, ":\n");
      int i = 0;
      int raw_offset = 0;
      while (pch != NULL)
      {
        i++;
        if (i == 23)
        {
          raw_offset = atoi(pch);
        }
        if (i == 27)
        {
          setTime(atoi(pch) + raw_offset);
        }
        // printf("%d: %s\n", i, pch);
        pch = strtok(NULL, ":\n");
      }
      String msg = "Time Synced ";
      is_time_synced = true;
      msg += millis();
      msg += "ms from boot.";
    }
    else
    {
      Serial.printf("[HTTP] Error code: %d\n", httpCode);
    }
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

void AutoBrightness()
{
  /* int16_t newBright = analogRead(LDR_PIN);
   if (newBright > 1000)
     newBright = 1000;

   FastLED.setBrightness(map(newBright, 1000, 0, 0, 255));*/
  byte newBrighness = 0;
  byte h = hour();
  byte m = minute();
  if (h >= 19 || h < 5)
    newBrighness = 200;
  else if (h >= 18 || h < 6)
    newBrighness = 127;
  else if ((h >= 17 && m >= 3) || (h <= 6 && m <= 30))
    newBrighness = 127;
  FastLED.setBrightness(newBrighness);
  FastLED.show();
}

void AutoBrightnessLDR()
{
#define LDR_READS 20
  int newBright = 0;
  for (size_t i = 0; i < LDR_READS; i++)
  {
    newBright += analogRead(LDR_PIN);
  }
  newBright = newBright / LDR_READS;

  if (newBright > 1000)
    newBright = 1000;
  FastLED.setBrightness(map(newBright, 1000, 0, 0, 255));
  FastLED.show();
}

void BetterAutoBrightnessLDR()
{
#define LDR_READS 20
  int newBright = 0;
  for (size_t i = 0; i < LDR_READS; i++)
  {
    newBright += analogRead(LDR_PIN);
  }
  newBright = newBright / LDR_READS;

  if (newBright > 1000)
    newBright = 1000;

  if (user_preferences.turn_off_lights_time >= 0)
  {
    if (hour() == user_preferences.turn_off_lights_time)
      control_variables.turn_off_lights_flag = true;
    if (hour() == 12)
      control_variables.turn_off_lights_flag = false;

    if (control_variables.turn_off_lights_flag)
      newBright = 1000;
  }

  if (now() - control_variables.sync_light > 5)
  {
    if (newBright < 900)
      Blynk.virtualWrite(V0, 0);
    else
      Blynk.virtualWrite(V0, 1);
    control_variables.sync_light = true;
  }
  FastLED.setBrightness(map(newBright, 1000, 0, 0, 255));
  FastLED.show();
}

long getUS(bool use_newping = true)
{
  long duration = 0;
    if (use_newping)
  {
     duration = sonar.ping_median(10); // Send ping, get distance in cm and print result (0 = outside set distance range)
  }
  else
  {
    digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  }
  return duration;
}


void taskp(void *pvParameters)
{
  int count = 0;
  for (;;)
  {
    p.run();
    
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void InitializeSensors()
{
  Sensors.reset();
  if (!loadSensors())
  {
    Sensors.Add("ds18inner", "DS18 Inner", Type_Float);
    Sensors.Add("ds18prob", "DS18 Probe", Type_Float);
    Sensors.Add("moist", "Moisture Sensor", Type_Int);
    Sensors.Add("ldr", "LDR Sensor", Type_Int);
    Sensors.Add("dht_hum", "DHT Humidity", Type_Int);
    Sensors.Add("dht_temp", "DHT Temperature", Type_Float);
    Sensors.Add("waterlevel", "Water Level", Type_Int);
  }
}

void setup()
{

  pinMode(ONBOARD_LED_PIN, OUTPUT);
  digitalWrite(PUMP_RELAY, HIGH); // Pump Relay is active low
  // Debug console
  Serial.begin(115200);

  FastLED.addLeds<WS2812, LED_STRIP_PIN, GRB>(leds, LED_STRIP_SIZE); // GRB ordering is typical
  FastLED.addLeds<WS2811, WS2811_PIN, BGR>(leds, LED_STRIP_SIZE, WS2811_SIZE);

#ifdef USE_SPIFFS
  if (SPIFFS.begin(true))
  {
    is_SPIFFS_mounted = true;
  }
#endif
  //  Initialize Sensors
  InitializeSensors();
  // Loads user preferences
  user_preferences.load();
  Serial.print(user_preferences.Print());

  WiFi.begin(WIFISSID, WIFIPASSWD);
  bool createAp = false; // flag for creating an AP in case we can't connect to wifi
  bool beauty = true;    // Esthetics for Serial.print();
  WiFi.hostname(DEVICE_NAME);
  // Try to connect to WiFi
  int StartMillis = millis();
  while (WiFi.status() != WL_CONNECTED && !createAp)
  {
    if (millis() % 100 == 0 && beauty)
    {
      Serial.print(".");
      beauty = false;
      leds[0] = CRGB(0xFF * beauty, 0x99 * beauty, 0);
      leds[1] = CRGB(0xFF * !beauty, 0x99 * !beauty, 0);
      FastLED.show();
    }
    else if (millis() % 100 != 0)
    {
      beauty = true;
    }

    if (millis() - StartMillis > 14999)
    {
      Serial.printf("\nNetwork '%s' not found.\n", WIFISSID);
      createAp = true;
    }
  }
  if (createAp)
  {
    // fill_solid(leds, 2, 0xFF0000);
    setBeaconColor(CRGB::Red);

    for (size_t i = 0; i < 5; i++)
    {
      FastLED.setBrightness(255);
      FastLED.show();
      delay(200);
      FastLED.setBrightness(0);
      FastLED.show();
      delay(200);
    }
    WiFi.enableAP(true);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(WIFI_SOFTAP_SSID, WIFI_SOFTAP_PASSWD);
    // dnsServer.start(53, "*", WiFi.softAPIP());
    Serial.println("");
    Serial.print("Creating WiFi Ap.\n ---SSID:  ");
    Serial.println(WIFI_SOFTAP_SSID);
    Serial.print(" --IP address: ");
    Serial.println(WiFi.softAPIP());
  }
  else
  {
    // fill_solid(leds, 2, 0x00FF00);
    setBeaconColor(CRGB::Green);

    for (size_t i = 0; i < 5; i++)
    {
      FastLED.setBrightness(255);
      FastLED.show();
      delay(200);
      FastLED.setBrightness(0);
      FastLED.show();
      delay(200);
    }
    Serial.print("Connected to ");
    Serial.println(WIFISSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    WiFi.setAutoReconnect(true);
  }
  FastLED.setBrightness(255);
  FastLED.clear();
  FastLED.show();
  // Try to connect to blynk. don't use Blynk.begin(), it will block the code if you don't have an in ternet connection
  Blynk.config(auth);
  Blynk.connect();

  // Stars the OTA
  ArduinoOTA.setHostname(DEVICE_NAME);

  ArduinoOTA.onStart(startOTA);
  ArduinoOTA.onEnd(endOTA);
  ArduinoOTA.onProgress(progressOTA);
  ArduinoOTA.onError(errorOTA);
  ArduinoOTA.begin();

  // Starts the TCP server (used for local debug mostly and NightMare integration)
  wifiServer.begin();

  // Starts the DS18b20 and the DHT sensor.
  DS18.begin();
  DS18.setResolution(10);
  // Serial.println("devs");
  // Serial.println(DS18.getDeviceCount());
  // for (size_t i = 0; i < DS18.getDeviceCount(); i++)
  // {
  //   DeviceAddress aaa;
  //   DS18.getAddress(aaa, i);
  //   printAddress(aaa);
  //   Serial.println();
  // }

  dht.begin();
  // sensors_hist.Reset();

  getTime(); // Sync Time Online

  // Declare PinModes
  pinMode(LDR_PIN, INPUT);
  pinMode(VBAT_PIN, INPUT);
  pinMode(MOISTURE_SENSOR, INPUT);
  pinMode(EMPTY_WATER_LEVEL, INPUT_PULLUP);
  pinMode(HALF_WATER_LEVEL, INPUT_PULLUP);
  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(27, OUTPUT); // AKA Pin 33 Enable.
#ifdef PROTO
  pinMode(TEST_BUTTON, INPUT_PULLUP);
#endif

  WiFi.setAutoReconnect(true);

  // Loads the last time pump was automatically trigerred
  if (is_SPIFFS_mounted)
  {
    if (SPIFFS.exists("/lastAutoPump.txt") && is_time_synced)
    {
      String j = SPIFFS.open("/lastAutoPump.txt").readString();
      Last_Auto_Pump = atoi(j.c_str());
    }
  }
  // get initial reading of the values
  updateValues(true);

  if (WaterState == 0)
    low_water_triggered = true;
  if (WaterState <= 1)
    fifty_water_marker = true;

  xTaskCreatePinnedToCore(
      taskp,       // Function to implement the task /
      "LEDhandle", // Name of the task /
      10000,       //* Stack size in words /
      NULL,        //* Task input parameter /
      32,          //* Priority of the task /
      NULL,        //* Task handle. /
      1);
  // p.progBar(map(WiFi.RSSI(), -90, -40, 0, 100), 60);
  // p.setMode(PatternMode::sinWaves, 0xFF0000, 0x00FF00);

#ifdef PROTO
  Serial.println("SPIFFS FILES");
  Serial.print(listAllFiles("/"));
  // Serial.println(SPIFFS.open(SENSORS_FILENAME).readString());
  command_result result = handleCommand("REQ_FILES");
  Serial.printf("%s  || %d\n", result.response.c_str(), result.result);
#endif
}

void loop()
{
  ArduinoOTA.handle();
  Blynk.run();
  updateValues();
  handleTCP();

  if (WiFi.scanComplete() > 0 && SendWifiResults)
  {
    String returnmsg = "";
    byte numSsid = WiFi.scanComplete();
    for (byte i = 0; i < numSsid; i++)
    {
      returnmsg += WiFi.SSID(i);
      returnmsg += '=';
      returnmsg += WiFi.RSSI(i);
      returnmsg += '\n';
    }
    SendWifiResults = false;
    Blynk.virtualWrite(V9, returnmsg);
  }

  EVERY_N_MILLISECONDS(100)
  {
    if (user_preferences.AutoLDR)
      BetterAutoBrightnessLDR();
  }

  if (!wl_empty && !low_water_triggered)
  {
    low_water_triggered = true;
    if (pump_is_running)
      StopPump(true);
    Blynk.logEvent("low_water", "Bottom Sensor is not detecting water");
    FileWrite("/log.txt", "Bottom Sensor Triggered");
  }

  if (wl_empty > 0 && low_water_triggered)
  {
    low_water_triggered = false;
    Blynk.logEvent("water_full", "Bottom Sensor detecting water");
    FileWrite("/log.txt", "Bottom Sensor Filled");
  }

  if (!wl_half <= 50 && !fifty_water_marker)
  {
    fifty_water_marker = true;
    FileWrite("/log.txt", "Middle Sensor Triggered");
  }

  if (wl_half > 50 && fifty_water_marker)
  {
    fifty_water_marker = false;
    FileWrite("/log.txt", "Middle Sensor Filled");
  }

  if (pump_is_running && millis() >= pump_end && pump_prog_end)
  {
    StopPump();
  }

  if (control_variables.auto_ldr_overriden && hour() == control_variables.aut_ldr_resume_time)
  {
    control_variables.auto_ldr_overriden = false;
    user_preferences.AutoLDR = true;
  }
#ifdef PROTO
  // TEST AREA
  if (digitalRead(TEST_BUTTON) == LOW && digital_debounce)
  {
    pinMode(27, INPUT); 
    pinMode(26, INPUT); 
    int oldMillis = millis();        // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
    int dist = sonar.ping_median(7); // Send ping, get distance in cm and print result (0 = outside set distance range)
    int timespent = millis() - oldMillis;
    float result = (float)dist / US_ROUNDTRIP_CM;
    Serial.printf("dist: %.1f cm, time: %d ms\n", result, timespent);
    digital_debounce = false;
    pinMode(27, OUTPUT); 
    pinMode(26, OUTPUT); 
    delay(1);
  }

  if (digitalRead(TEST_BUTTON) == HIGH)
  {
    digital_debounce = true;
  }

#endif
}
