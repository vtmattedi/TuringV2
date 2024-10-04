
#include "commandInfo.h"



const char *commandInfo =
    "| Command             | Description                                                                                                                          |\n"
    "|---------------------|--------------------------------------------------------------------------------------------------------------------------------------|\n"
    "| [Help (H)]          | Displays documentation about available commands and their usage.                                                                     |\n"
    "| [REQ_CLIENTS]       | Requests a list of clients connected to the TCP server.                                                                              |\n"
    "| [REQ_FILE]          | Requests a specific file from the file system (read as ASCII).                                                                       |\n"
    "| [REQ_FILES]         | Requests a list of all available files in the file system.                                                                           |\n"
    "| [REQ_LOG]           | Requests the log data.                                                                                                               |\n"
    "| [PREF]              | Changes a user preference to a new value.                                                                                            |\n"
    "| [GET_PREFS]         | Retrieves and displays user preferences and their current values.                                                                    |\n"
    "| [HWINFO]            | Requests hardware connections information.                                                                                           |\n"
    "| [READ_PIN]          | Reads data from a specified pin.                                                                                                     |\n"
    "| [READ_MUX]          | Reads the value on one of the ports of the MUX.                                                                                      |\n"
    "| [WRITE_SIPO]        | Writes a specific data to the Serial in Parallel out Shift Register (SIPO).                                                          |\n"
    "| [GET_IP]            | Retrieves and displays the current IP address.                                                                                       |\n"
    "| [PUMP_INFO]         | Gets information about the last automatic pump activation and the next scheduled time.                                               |\n"
    "| [TRIGGER_US]        | Reads the value from the Ultrasonic Sensor.                                                                                          |\n"
    "| [DEL_FILE]          | Deletes a specified file from the file system.                                                                                       |\n"
    "| [PUMP]              | Manually triggers the pump for a specified duration.                                                                                 |\n"
    "| [READ10]            | Reads from a specified pin with a 10-bit resolution.                                                                                 |\n"
    "| [READC]             | Reads from a specified pin with a configurable resolution.                                                                           |\n"
    "| [SET_MUX]           | Sets the Multiplexer (MUX) port to a specified value.                                                                                |\n"
    "| [DS18TEST]          | Performs a test on the DS18b20 sensor.                                                                                               |\n"
    "| [CODE]              | Writes a specified code to the onboard LEDs.                                                                                         |\n"
    "| [SENSORS]           | Retrieves and displays sensor values.                                                                                                |\n"
    "| [SENSORS-RESET]     | Resets sensor values to default.                                                                                                     |\n"
    "| [SIPOPIN]           | Writes a specified value to a pin of the Serial in Parallel out (SIPO) shift register.                                               |\n"
    "| [REPORT]            | Toggles reporting of sensor values.                                                                                                  |\n"
    "| [TOGGLEUPDATES]     | Toggles sensor updates.                                                                                                              |\n"
    "| [AGE]               | Displays timestamp and time-related information.                                                                                     |\n"
    "| [CONTROL_VARIABLES] | Not implemented.                                                                                                                     |\n"
    "| [MUXSWEEP]          | Performs a sweep of the Multiplexer (MUX) ports and displays the readings.                                                           |\n"
    "| [MUX-SIPOSWEEP]     | Performs a sweep of the Multiplexer (MUX) ports with SIPO enabled and displays the readings.                                         |\n"
    "| [TESTLED]           | Tests LED functionality by setting LEDs to a specified color.                                                                        |\n"
    "| [VERSION]           | Displays firmware version and build time.                                                                                            |\n"
    "| [SYNCTIME]          | Synchronizes time with a specified value.                                                                                            |\n"
    "| [LOG]               | Retrieves and displays log data.                                                                                                     |\n";

const char *commands[] = {
    "[Help (H)]",
    "[REQ_CLIENTS]",
    "[REQ_FILE]",
    "[REQ_FILES]",
    "[REQ_LOG]",
    "[PREF]",
    "[GET_PREFS]",
    "[HWINFO]",
    "[READ_PIN]",
    "[READ_MUX]",
    "[WRITE_SIPO]",
    "[GET_IP]",
    "[PUMP_INFO]",
    "[TRIGGER_US]",
    "[DEL_FILE]",
    "[PUMP]",
    "[READ10]",
    "[READC]",
    "[SET_MUX]",
    "[DS18TEST]",
    "[CODE]",
    "[SENSORS]",
    "[SENSORS-RESET]",
    "[SIPOPIN]",
    "[REPORT]",
    "[TOGGLEUPDATES]",
    "[AGE]",
    "[CONTROL_VARIABLES]",
    "[MUXSWEEP]",
    "[MUX-SIPOSWEEP]",
    "[TESTLED]",
    "[VERSION]",
    "[SYNCTIME]",
    "[LOG]"
    };

const char *descriptions[] = {
    "Displays documentation about available commands and their usage.",
    "Requests a list of clients connected to the TCP server.",
    "Requests a specific file from the file system (read as ASCII).",
    "Requests a list of all available files in the file system.",
    "Requests the log data.",
    "Changes a user preference to a new value.",
    "Retrieves and displays user preferences and their current values.",
    "Requests hardware connections information.",
    "Reads data from a specified pin.",
    "Reads the value on one of the ports of the MUX.",
    "Writes a specific data to the Serial in Parallel out Shift Register (SIPO).",
    "Retrieves and displays the current IP address.",
    "Gets information about the last automatic pump activation and the next scheduled time.",
    "Reads the value from the Ultrasonic Sensor.",
    "Deletes a specified file from the file system.",
    "Manually triggers the pump for a specified duration.",
    "Reads from a specified pin with a 10-bit resolution.",
    "Reads from a specified pin with a configurable resolution.",
    "Sets the Multiplexer (MUX) port to a specified value.",
    "Performs a test on the DS18b20 sensor.",
    "Writes a specified code to the onboard LEDs.",
    "Retrieves and displays sensor values.",
    "Resets sensor values to default.",
    "Writes a specified value to a pin of the Serial in Parallel out (SIPO) shift register.",
    "Toggles reporting of sensor values.",
    "Toggles sensor updates.",
    "Displays timestamp and time-related information.",
    "Not implemented.",
    "Performs a sweep of the Multiplexer (MUX) ports and displays the readings.",
    "Performs a sweep of the Multiplexer (MUX) ports with SIPO enabled and displays the readings.",
    "Tests LED functionality by setting LEDs to a specified color.",
    "Displays firmware version and build time.",
    "Synchronizes time with a specified value.",
    "Retrieves and displays log data."
    };

// size of the commands and descriptions arrays
const size_t numCommands = sizeof(commands) / sizeof(commands[0]);
const size_t numDescriptions = sizeof(descriptions) / sizeof(descriptions[0]);

extern String centerFixed(String, int, int);
String align(String msg, int size, int mode)
{
    return centerFixed(msg, size, mode);
}

String generateCommandInfoJson(bool pretty)
{

    if (numCommands != numDescriptions)
    {
        Serial.println("Error: Number of commands does not match number of descriptions.");
        return "";
    }

    String json;
    DynamicJsonDocument doc = DynamicJsonDocument(1024);
    for (size_t i = 0; i < numCommands; i++)
    {
        doc[commands[i]] = descriptions[i];
    }
    if (pretty)
    serializeJsonPretty(doc, json);
    else
    serializeJson(doc, json);
    return json;
}

String generateCommandInfo()
{
    if (numCommands != numDescriptions)
    {
        Serial.println("Error: Number of commands does not match number of descriptions.");
        return "";
    }

    int MaxCommand = 0;
    for (size_t i = 0; i < numCommands; i++)
    {
        if (strlen(commands[i]) > MaxCommand)
        {
            MaxCommand = strlen(commands[i]);
        }
    }
    int MaxDescription = 0;
    for (size_t i = 0; i < numCommands; i++)
    {
        if (strlen(descriptions[i]) > MaxDescription)
        {
            MaxDescription = strlen(descriptions[i]);
        }
    }
    // Serial.printf("Size: %d, Max Command: %d, Max Discription: %d\n", numCommands, MaxCommand, MaxDescription);
    String commandInfo = "\n|" + align("Command", MaxCommand) + "| " + align("Description", MaxDescription, 1) + "|\n";
    commandInfo += "|"; //-----------------|--------------------------------------------------------------------------------------------------------------------------------------|\n";
    for (int i = 0; i < MaxCommand; i++)
    {
        commandInfo += "-";
    }
    commandInfo += "|-";
    for (int i = 0; i < MaxDescription; i++)
    {
        commandInfo += "-";
    }
    commandInfo += "|\n";

    for (int i = 0; i < numCommands; i++)
    {
        commandInfo += "|" + align(commands[i], MaxCommand);

        commandInfo += "| " + align(descriptions[i], MaxDescription, 1);

        commandInfo += "|\n";
    }

    return commandInfo;
}