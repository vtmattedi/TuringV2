/*
  NightMareTcp.h - TCP client for microcontrollers using Arduino Framework
  This Library is a TCP server/client built around the WiFi.h lib
  Used mainly neat code used across different projects and also standarize
  and sync changes alog all future codes using nightmare TCP Comunication

  Author: Vitor Mattedi Carvalho
  Date: 29/03/2023
  Version: 1.0
*/

#ifndef NIGHTMARETCP_H
#define NIGHTMARETCP_H

#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif ESP32
#include <WiFi.h>
#endif

#ifndef MAX_CLIENTS
#define MAX_CLIENTS 10
#endif

#include "Arduino.h"
typedef enum
{
  // not fully implemented
  ThreeCharHeaders = 2,
  // String is sent as length:message and when new data arrives, the data is read looking for the size until it reaches a colon then it outputs the msg of that size;
  SizeColon = 1,
  // String is sent and recieved as is.
  AllAvailable = 0
} TransmissionMode;

// Common functions

String PrepareMsg(String msg, TransmissionMode mode, char *Headers = "", char tchar = 0);

String GetIdFromMsg(String);

// base Client for server
class NightMareTCPServerClient
{
private:


public:
  NightMareTCPServerClient();
  // The actual TCP Object
  WiFiClient *client;
  // Last Time the client sent anything
  uint clientsTimeout;
  // Client Status {If a message Z;* is Recieved then it is stored here}
  String clientsStatus;
  // Client ID
  String clientsID;
  // If Clients
  bool clientsRequestUpdates;
  // The Time Stamp for the Status String
  int clientsStatusTime;
  // Set the Nightmare TCP transmission mode. ThreeCharHeaderis not supported here
  TransmissionMode transmissionMode;
  // Sends The msg formatted according to the transmission mode
  void Send(String msg);
  // Resets The internal variables
  void Fast_send();
  // Sends the fast_char to the Server if client exists
  void Reset();

  char _fastChar;
  char _trasmissionChar;
};

// NightMare Server class
class NightMareTCPServer
{
  typedef std::function<String(String, int)> TServerMessageHandler;
  typedef std::function<void()> TFastHandler;

private:
  WiFiServer _wifiServer;
  int _port;
  TFastHandler _fast_callback;
  TServerMessageHandler _message_callback;
  bool _debug;
  bool _EnableNightMareCommands;
  String NightMareCommands(String msg, byte index);
public:
  // the timeout in ms of client inactivity
  uint _timeout;
  // flag that disables timing out clients
  bool _disable_timeout;
  /*NightMare TCP Server
   *@param port the port of the server [100]
   *@param debug prints debug info if enabled [false]
   */
  NightMareTCPServer(int port = 100, bool debug = false);
  // Sets the function to handle server messages. It must return a String Response.
  NightMareTCPServer &setMessageHandler(TServerMessageHandler fn);
  // Sets the handler for the fast callback. it is imediatly triggred when the fast_char is read in the Stream;
  NightMareTCPServer &setFastHandler(TFastHandler fn);
  // Array with the clients of the server
  NightMareTCPServerClient clients[MAX_CLIENTS];
  // Starts the Server.
  void Begin();
  // Sends the msg to all connected clients
  void Broadcast(String msg);
  // looks for data from the clients and trigger the proper callbacks in case of data
  void HandleServer();
  // sets the timeout of the server (-1) will toggle it.
  void set_timeout(int timeout = -1);
  // Sends the message to the client at the client_index
  void Send_to_index(String msg, byte client_index);
  // Sends the message to the client with ip matching client_IP
  void Send_to_IP(String msg, String client_IP);
};

// Nightmare Client Object
class NightMareTCPClient
{
private:
  typedef std::function<void()> TFastHandler;
  typedef std::function<String(String)> TClientMessageHandler;
  TClientMessageHandler _message_callback;
  TFastHandler _fast_callback;
  char _fastChar;
  bool _debug;
  uint _timeout;
  String _ip;
  int _port;
  char _trasmissionChar;
  String _label;

public:
  // Comunication protocol used, check available TransmissionModes;
  TransmissionMode transmissionMode;
  /*NightMare TCP Client
   *@param ip the IP of the server to connect
   *@param port the port of the server [100]
   *@param debug prints debug info if enabled [false]
   */
  NightMareTCPClient(String ip, int port = 100, bool debug = false);
  // the client object with the actual tcp socket
  WiFiClient *client;
  // Sets the new message handler
  NightMareTCPClient &setMessageHandler(TClientMessageHandler fn);
  // Sets the handler for the fast callback. it is imediatly triggred when the fast_char is read in the Stream;
  NightMareTCPClient &setFastHandler(TFastHandler fn);
  // look for data available at the client and also reconnects if not connected
  void HandleClient();
  // Sends the msg to the server if client exists. msg is passed through a prepare message function.
  void Send(String msg);
  // Sets the debug flag
  void setdebug(bool newDebug);
  // Sets the fast_char to be used
  void setFastChar(char newFastchar);
  // Sends the fast_char to the Server if client exists
  void Fast_Send();
  // Sets the label of he object for tracking among many.
  void setName(String newName);
  // Gets the label of the object @returns Arduino String with object's label or "" if label was never set
  String Name();
};

#endif