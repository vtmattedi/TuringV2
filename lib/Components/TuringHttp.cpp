#include "TuringHttp.h"
#include <D:\Projects\PIO\TuringV2\TuringV2\src\WebPage\Output\webfiles.h>
#include "Update.h"
#include "WiFi.h"

WebServer httpServer;
extern bool handleAPIs();

#define CSS "text/css"
#define HTML "text/html"
#define JS "text/script"
#define PLAIN "text/plain"

bool HandleCommon()
{
    String uri = httpServer.uri();

    if (uri == "/" || uri == "/index" || uri == "/index.html")
    {
        String menu = menu_html;
        menu.replace("{SSID}", WiFi.SSID());
        menu.replace("{IP}", WiFi.localIP().toString());

        httpServer.send(200, "text/html", menu);
    }
    else if (uri == "/sensors" || uri == "/sensors.html")
    {
        httpServer.send(200, "text/html", sensors_html);
    }
    else if (uri == "/menu.js")
    {
        httpServer.send(200, "text/javascript", menu_js);
    }
    else if (uri == "/menu.css")
    {
        httpServer.send(200, "text/css", menu_css);
    }
    else if (uri == "/mux" || uri == "/mux.html")
    {
        httpServer.send(200, "text/html", mux_html);
    }
    else if (uri == "/mux.js")
    {
        httpServer.send(200, "text/javascript", mux_js);
    }
    else if (uri == "/sensors.js")
    {
        httpServer.send(200, "text/javascript", sensors_js);
    }
    else if (uri == "/sensors.css")
    {
        httpServer.send(200, "text/css", sensors_css);
    }
    else if (uri == "/common.css")
    {
        httpServer.send(200, "text/css", common_css);
    }
    else if (uri == "/common.js")
    {
        httpServer.send(200, "text/css", common_js);
    }
       else if (uri == "/terminal" || uri == "/terminal.html")
    {
        httpServer.send(200, HTML, terminal_html);
    }
     else if (uri == "/terminal.js")
    {
        httpServer.send(200, JS, terminal_js);
    }
      else if (uri == "/terminal.css")
    {
        httpServer.send(200, CSS, terminal_css);
    }
    else if (uri == "/update" || uri == "/update.html")
    {
        httpServer.send(200, HTML, update_html);
    }
    else
        return false;
    return true;
}

void HandleMostFiles()
{
    bool done = HandleCommon();

    if (!done)
    {
        done = handleAPIs();
    }

    if (!done)
    {
        String message = "File not found\n\n";
        message += "URI: ";
        message += httpServer.uri();
        message += "\nMETHOD: "; // @token method
        message += (httpServer.method() == HTTP_GET) ? "GET" : "POST";
        message += "\nARGS: "; // @token args
        message += httpServer.args();
        message += "\n";

        for (uint8_t i = 0; i < httpServer.args(); i++)
        {
            message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
        }
        // httpServer.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate")); // @HTTPHEAD send cache
        // httpServer.sendHeader(F("Pragma"), F("no-cache"));
        // httpServer.sendHeader(F("Expires"), F("-1"));
        httpServer.send(404, PLAIN, message);
    }
}

void HandleUpload()
{
    bool error = false;
    // handler for the file upload, get's the sketch bytes, and writes them through the Update object
    HTTPUpload &upload = httpServer.upload();

    // UPLOAD START
    if (upload.status == UPLOAD_FILE_START)
    {
        // if(_debug) Serial.setDebugOutput(true);
        uint32_t maxSketchSpace;

#ifdef ESP8266
        WiFiUDP::stopAll();
        maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
#elif defined(ESP32)
        // Think we do not need to stop WiFIUDP because we haven't started a listener
        // maxSketchSpace = (ESP.getFlashChipSize() - 0x1000) & 0xFFFFF000;
        // #define UPDATE_SIZE_UNKNOWN 0xFFFFFFFF // include update.h
        maxSketchSpace = UPDATE_SIZE_UNKNOWN;
#endif
        Serial.printf("OTA START\n");

        // Update.onProgress(THandlerFunction_Progress fn);
        // Update.onProgress([](unsigned int progress, unsigned int total) {
        //       Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        // });

        if (!Update.begin(maxSketchSpace))
        { // start with max available size
            Serial.printf("OTA ERROR\n");
            error = true;
            Update.end(); // Not sure the best way to abort, I think client will keep sending..
        }
    }
    // UPLOAD WRITE
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        // Serial.print(".");
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
            Serial.printf("OTA WRITE ERROR\n");
            error = true;
        }
    }
    // UPLOAD FILE END
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
        { // true to set the size to the current progress
            Serial.printf("OTA DONE Size: %d \n", upload.totalSize);
        }
        else
        {
            error = true;
        }
    }
    // UPLOAD ABORT
    else if (upload.status == UPLOAD_FILE_ABORTED)
    {
        Update.end();
        Serial.printf("OTA ABORTED \n"); //, upload.totalSize);
        error = true;
    }
    delay(0);
}

void HandlePostUpload()
{
    String msg = "";
    if (Update.hasError())
    {
        msg = "OTA Failed: ";
        msg += Update.errorString();
    }
    else
    {
        msg = "OTA OK";
    }
    httpServer.send(200, PLAIN, msg);
    Serial.printf("%s\n", msg.c_str());
    delay(1000); // send page
    if (!Update.hasError())
    {
        ESP.restart();
    }
}

void HandleFsFile()
{
    if (httpServer.hasArg("filename"))
    {
        httpServer.send(400, PLAIN, "Error: filename missing!");
        return;
    }

    String filename = httpServer.arg("filename");
    File file = LittleFS.open(filename);
    if (file)
    {
        httpServer.streamFile<File>(file, PLAIN, 200);
    }
    else
    {
        httpServer.send(200, PLAIN, "file: " + filename + " was not found");
    }
}

void StartWebServer()
{
    httpServer.onNotFound(HandleMostFiles);
    httpServer.on("/file", HandleFsFile);
    httpServer.on("/upd", HTTP_POST, HandlePostUpload, HandleUpload);
    httpServer.begin();
    Serial.printf("Web Server Started\n");
}