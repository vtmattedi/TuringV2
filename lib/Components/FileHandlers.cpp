#include "FileHandlers.h"

////
/* 
*  @param 
    @param 
*   @param useSPIFFS True for SPIFFS and false for SD @param   @param newLine 
*/

/// @brief Writes the content to the speficied file.
/// @param FileName FileName The file to be written.
/// @param Message Message Content to be Written.
/// @param useSDCard True for SD and false for LittleFS.
/// @param timestamp Add timestamp at the beginning of the content.
/// @param newLine Write with new line at the end of the content.
void FileWrite(String FileName, String Message, bool useSDCard, bool timestamp, bool newLine)
{
    File logfile;
    String Msg = "";

    if (useSDCard)
    {
       Serial.printf("!SDCARD ALERT! -> '%s':'%s'\n",FileName.c_str(),Message.c_str());
    }
    

    if (timestamp)
    {
        Msg += timestampToDateString(now());
        Msg += " - ";
    }

    Msg += Message;

    if (control_variables.flags.LittleFS_mounted && useSDCard)
    {
        double usage = LittleFS.usedBytes() / LittleFS.totalBytes();
        if (usage > 90)
        {
            Serial.println("SPIFFS is full.");
        }
        logfile = LittleFS.open(FileName, "a", true);
        if (newLine)
            logfile.println(Msg);
        else
            logfile.print(Msg);
        logfile.close();
    }
    else if (control_variables.flags.SD_Mounted && !useSDCard)
    {
        Serial.println("not implemented");
    }
}

void FileWrite_LittleFS(String FileName, String Message)
{
    Serial.printf("Writing '%s' to '%s'\n", Message.c_str(), FileName.c_str());
    File logfile = LittleFS.open(FileName, "a+", true);
    logfile.print(Message);
    logfile.flush();
    logfile.close();
    //  FileWrite(FileName, Message, false, false, false);
}

/**
 * Logs a message to a file '/log.txt' on LittleFS.
 * @param message The message to be logged.
 */
void Log(String message)
{
    message = formatString("[%s] -> %s\n",timestampToDateString(now()).c_str(),message.c_str());
    FileWrite_LittleFS("/log.txt", message);
}

void FileWrite_SD(String FileName, String Message)
{
    FileWrite(FileName, Message, true, false, false);
}

/** Returns the sum of all characteres in the string
 @param string_to_test String to be summed */
uint32_t checksum(String string_to_test)
{
    uint32_t sum = 0;
    for (size_t i = 0; i < string_to_test.length(); i++)
    {
        sum += string_to_test[i];
    }
    // if (mod > 0)
    //     sum  = sum % mod;

    return sum;
}

String listAllFiles(String dir, bool SD_card, byte treesize)
{
    String response = "";
    /// SPIFFS mode
    if (!SD_card)
    {
        // List all available files (if any) in the SPI Flash File System
        if (!control_variables.flags.LittleFS_mounted)
        {
            response += "SPIFFS not Mount";
            return response;
        }
        response += "Used Bytes: ";
        response += LittleFS.usedBytes();
        response += "-----Total Bytes: ";
        response += LittleFS.totalBytes();
        response += "-----Used: ";
        response += map(LittleFS.usedBytes(), 0, LittleFS.totalBytes(), 0, 100);
        response += "%\n";
        response += "Listing files in: ";
        response += dir;
        response += "\n";
        fs::File root = LittleFS.open(dir);
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
