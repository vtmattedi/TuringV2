#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include "WiFiManager.h"
#include "FS.h"
#include "LittleFS.h"

void StartWebServer();

extern WebServer httpServer;
