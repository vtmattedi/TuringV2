#pragma once

#include <LittleFS.h>
#include <Arduino.h>
#include "FS.h"
#include <NightMareNetwork.h>
#include "Configs.h"

void FileWrite(String FileName, String Message, bool useSDCard = false, bool timestamp = true, bool newLine = true);
void FileWrite_LittleFS(String FileName, String Message);
void FileWrite_SD(String FileName, String Message);
/** Returns the sum of all characteres in the string
 @param string_to_test String to be summed */
uint32_t checksum(String string_to_test);

void Log(String message);

String listAllFiles(String dir = "/", bool SD_card = false, byte treesize = 0);
