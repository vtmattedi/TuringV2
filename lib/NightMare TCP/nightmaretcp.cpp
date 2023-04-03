#include "nightmaretcp.h"
#include <D:\Projects\PIO\Lib_tester\lib\Time-master\TimeLib.h>

#define DefaultFastChar 0xFE
#define DefaultTransmissionChar '!'
#define DONT_COMPILE_SERIAL

// Prepare the msg according to the transmission mode
String PrepareMsg(String msg, TransmissionMode mode, char *headers, char tchar)
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
    r_msg += tchar;
    r_msg += headers;
    r_msg += msg;
    return r_msg;
  }
  return msg;
}

String GetIdFromMsg(String msg)
{
  int Index = 0;
  String rtr = "";
  for (int i = 1; i < msg.length(); i++)
  {
    char c = msg[i];
    if (c == ';')
    {
      Index++;
    }
    else
      rtr += c;
  }
  return rtr;
}

NightMareTCPServerClient::NightMareTCPServerClient()
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
  _fastChar = DefaultFastChar;
  _trasmissionChar = DefaultTransmissionChar;
}

void NightMareTCPServerClient::Send(String msg)
{
  if (client != NULL)
  {
    client->print(PrepareMsg(msg, transmissionMode));
  }
}

void NightMareTCPServerClient::Fast_send()
{
  if (client != NULL)
  {
    client->write(_fastChar);
  }
}

void NightMareTCPServerClient::Reset()
{
  client = NULL;
  clientsTimeout = 0;
  clientsStatus = "";
  clientsID = "";
  clientsRequestUpdates = false;
  clientsStatusTime = 0;
  transmissionMode = TransmissionMode::AllAvailable;
}

NightMareTCPServer::NightMareTCPServer(int port, bool debug) : _message_callback(NULL), _fast_callback(NULL)
{
  _disable_timeout = false;
  _timeout = 15000;
  _EnableNightMareCommands = true;
  _port = port;
  _debug = debug;
}

String NightMareTCPServer::NightMareCommands(String msg, byte index)
{
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
        response += ", (self) ID: ";
        response += clients[i].clientsID;
        response += ", INFO: ";
        response += clients[i].clientsStatus[i];
        response += "\n";
      }
    }
  }
  else if (msg == "REQ_UPDATES")
  {
    clients[index].clientsRequestUpdates = !clients[index].clientsRequestUpdates;
    response = "Your REQ_UPDATES flag is now: ";
    if (clients[index].clientsRequestUpdates)
      response += true;
    else
      response += false;
  }
  else if (msg == "REQ_UPDATES_1")
  {
    clients[index].clientsRequestUpdates = true;
    response += "Your REQ_UPDATES flag is now: true";
  }
  else if (msg == "REQ_UPDATES_0")
  {
    clients[index].clientsRequestUpdates = false;
    response += "Your REQ_UPDATES flag is now: false";
  }
  else if (msg == "TMODE=0" || msg == "7:TMODE=0")
  {
    clients[index].transmissionMode = TransmissionMode::AllAvailable;
    response = "Transmission mode set to AllAvailable";
  }
  else if (msg == "TMODE=1" || msg == "7:TMODE=1")
  {
    clients[index].transmissionMode = TransmissionMode::SizeColon;
    response = "Transmission mode set to SizeColon";
  }
  else
  {
    int indicator_index = msg.indexOf("ID::");
    if (indicator_index >= 0)
    {
      int endindex =  msg.indexOf(";", indicator_index + 4);
      if (endindex > 0)
      clients[index].clientsID = msg.substring(indicator_index + 4, endindex);
      response = "M;ACK;";
    }
    indicator_index = msg.indexOf("Z::");
    if (indicator_index >= 0)
    {
      clients[index].clientsStatus = msg.substring(indicator_index + 3);;
      for (int i = 0; i < MAX_CLIENTS; i++)
      {
        if (clients[i].clientsRequestUpdates)
        {
          clients[i].Send(msg);
        }
      }
      response = "M;ACK;";
    }
  }

  return response;
}

void NightMareTCPServer::Begin()
{
  _wifiServer.begin(_port);
  if (_debug)
  {
    Serial.print("Starting NightMare TCP Server at port:");
    Serial.println(_port);
  }
}

void NightMareTCPServer::Broadcast(String msg)
{
  for (size_t i = 0; i < MAX_CLIENTS; i++)
  {
    clients[i].Send(msg);
  }
}

void NightMareTCPServer::HandleServer()
{
  // polls for new clients
  WiFiClient newClient = _wifiServer.available();
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
        clients[i].Send("REQ_ID");
        i = MAX_CLIENTS;
      }
      else if (i == MAX_CLIENTS)
      {

        Serial.printf("Could not allocate client [");
        Serial.print(newClient.remoteIP().toString());
        Serial.printf(":%d] all (%d) clients spaces are beeing used right now\n", newClient.remotePort(), MAX_CLIENTS);
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

        if (newChar == clients[i]._fastChar)
        {
          if (_fast_callback)
            _fast_callback();
          return;
        }

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
            if (_EnableNightMareCommands)
            {
              String rtr = NightMareCommands(msg, i);
              if (rtr != "")
              {
                clients[i].Send(rtr);
              }
            }

            if (msg != "" && _message_callback)
              clients[i].Send(_message_callback(msg, i));
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

      if (_EnableNightMareCommands)
      {
        String rtr = NightMareCommands(msg, i);
        if (rtr != "")
        {
          clients[i].Send(rtr);
        }
      }

      if (_message_callback)
        clients[i].Send(_message_callback(msg, i));
    }
    // If a client disconnects, clear the memory for a new one
    if (NULL != clients[i].client && !clients[i].client->connected())
    {
      clients[i].client->stop();
      delete clients[i].client;
      clients[i].Reset();
    }
    // check if any client have been idle and time it out
    else if (NULL != clients[i].client && clients[i].client->connected())
    {
      if ((now() - clients[i].clientsTimeout >= _timeout) and !_disable_timeout)
      {
        clients[i].Send("E;0x3F;Timedout;");
        clients[i].client->stop();
        delete clients[i].client;
        clients[i].Reset();
      }
    }
  }
}

void NightMareTCPServer::set_timeout(int timeout)
{
  if (timeout == -1)
    _disable_timeout = !_disable_timeout;
  else if (timeout > 0)
  {
    _timeout = timeout;
  }
}

void NightMareTCPServer::Send_to_IP(String msg, String ip)
{
  for (size_t i = 0; i < MAX_CLIENTS; i++)
  {
    if (clients[i].client != NULL && clients[i].client->remoteIP().toString() == ip)
    {
      clients[i].Send(msg);
      if (_debug)
        Serial.printf("Sending '%s' to '%s' at index [%d]\n", msg.c_str(), ip.c_str(), i);
      return;
    }
    if (_debug)
      Serial.printf("Failed, no client wit IP: '%s' found\n", ip.c_str());
  }
}

void NightMareTCPServer::Send_to_index(String msg, byte index)
{
  if (index >= MAX_CLIENTS)
  {
    if (_debug)
      Serial.printf("Failed. '%d' is outside of clients array which has size: %d\n", index, MAX_CLIENTS);
    return;
  }
  if (clients[index].client == NULL)
    ;
  {
    if (_debug)
      Serial.printf("Failed, client at index '%d' is null\n", index);
    return;
  }
  clients[index].Send(msg);
}

NightMareTCPServer &NightMareTCPServer::setMessageHandler(TServerMessageHandler fn)
{
  _message_callback = fn;
  return *this;
}

NightMareTCPServer &NightMareTCPServer::setFastHandler(TFastHandler fn)
{
  _fast_callback = fn;
  return *this;
}

NightMareTCPClient::NightMareTCPClient(String ip, int port, bool debug) : _message_callback(NULL), _fast_callback(NULL)
{
  _ip = ip;
  _port = port;
  _fastChar = DefaultFastChar;
  _timeout = 0;
  _debug = debug;
  _trasmissionChar = DefaultTransmissionChar;
}

void NightMareTCPClient::HandleClient()
{
  if (client->available() > 0)
  {
    String msg = "";
    int size = 0;
    int index = 0;
    bool sizeFound = false;
    _timeout = now();
    while (client->available() > 0)
    {
      char newChar = client->read();
      if (newChar == _fastChar && _fast_callback)
      {
        _fast_callback();
        return;
      }

      if (newChar != (char)19)
      {

        if (!sizeFound && newChar == ':' && transmissionMode == TransmissionMode::SizeColon)
        {
          size = atoi(msg.c_str());
          sizeFound = true;
          msg = "";
          index = 0;
        }
        else if (sizeFound && index >= size && transmissionMode == TransmissionMode::SizeColon)
        {
          if (msg != "" && _message_callback)
            _message_callback(msg);
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
    if (_message_callback)
      _message_callback(msg);
  }
}

void NightMareTCPClient::Send(String msg)
{
  if (client != NULL)
  {
    client->print(PrepareMsg(msg, transmissionMode));
  }
}

void NightMareTCPClient::setName(String newName)
{
  _label = newName;
}

String NightMareTCPClient::Name()
{
  if (_label)
    return _label;
  return "";
}

void NightMareTCPClient::Fast_Send()
{
  if (client != NULL)
    client->write(_fastChar);
}

void NightMareTCPClient::setdebug(bool newDebug)
{
  _debug = newDebug;
}

void NightMareTCPClient::setFastChar(char newFastChar)
{
  _fastChar = newFastChar;
}
