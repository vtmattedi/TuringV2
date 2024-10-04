#pragma once

// Make sure to have these definition on top of your code or else Blynk won't be able to connect.
#include <../include/Creds/BlynkCred.h>
#include <../include/Version.h>
// #define PROTO // Toggle comment to switch between upload to Turing or Jupiter

#ifndef PROTO
#define BLYNK_TEMPLATE_ID MYBLYNKTEMPLATE
#define BLYNK_DEVICE_NAME MYBLYNKDEVICE
#define BLYNK_AUTH_TOKEN MYBLYNKAUTH
#endif
#ifdef PROTO
#define BLYNK_TEMPLATE_ID MYBLYNKTEMPLATE_PROTO
#define BLYNK_DEVICE_NAME MYBLYNKDEVICE_PROTO
#define BLYNK_AUTH_TOKEN MYBLYNKAUTH_PROTO
#endif
#define BLYNK_PRINT Serial
#include <Arduino.h>
//TCP
#include <Nightmaretcp.h>
//Blynk
#include <BlynkSimpleEsp32.h>
//HiveMQ
#include <HTTPClient.h>
#include <../lib/pubsubclient-2.8/src/PubSubClient.h>
#include <../include/Creds/HiveMQCred.h>
#include <ArduinoJson.h>

//Legacy stuff for blynk usage
uint16_t Blynk_upload_interval = 60;   // Sets the interval for uploading values to Blynk in seconds
int Blynk_last_upload = 0;             // Keeps the timestamp of the last time it sent data over to Blynk
uint16_t Blynk_telemetry_interval = 5; // Sets the interval for the telemetry data to Blynk in seconds
int Blynk_telemetry_last = 0;          // Keeps the timestamp of the last time it sent telemetry over to Blynk
bool Blynk_telemetry = false;          // Enable/disables Blynk Telemetry


extern void HiveMQ_Callback(char *topic, byte *payload, unsigned int length);

WiFiClientSecure hive_client;
PubSubClient HiveMQ(MQTT_URL, MQTT_PORT, hive_client);

extern String HandleMsg(String, byte);


