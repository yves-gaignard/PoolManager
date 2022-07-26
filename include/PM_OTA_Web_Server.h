/*
  Copyright 2022 - Yves Gaignard
  
  Web Server management
*/
#ifndef PM_OTA_Web_Server_h
#define PM_OTA_Web_Server_h

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// Standard library definitions

static const char* TAG = "PM_OTA_Web_Server";

/*
  Rui Santos
  Complete project details
   - Arduino IDE: https://RandomNerdTutorials.com/esp32-ota-over-the-air-arduino/
   - VS Code: https://RandomNerdTutorials.com/esp32-ota-over-the-air-vs-code/
  
  This sketch shows a Basic example from the AsyncElegantOTA library: ESP32_Async_Demo
  https://github.com/ayushsharma82/AsyncElegantOTA
*/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

AsyncWebServer OTAServer(80);

// Declare handler functions for the various URLs on the server
void PM_OTA_Web_Server_notFound(AsyncWebServerRequest *request);
void PM_OTA_Web_Server_sendWelcome(AsyncWebServerRequest *request);

// Web server setup and start
void PM_OTA_Web_Server_setup(void) {

  OTAServer.on("/", HTTP_GET, PM_OTA_Web_Server_sendWelcome);
  OTAServer.on("",  HTTP_GET, PM_OTA_Web_Server_notFound);

  AsyncElegantOTA.begin(&OTAServer);    // Start ElegantOTA
  OTAServer.begin();
  Serial.println("HTTP server started");
}

// handler to treat "page not found"
void PM_OTA_Web_Server_notFound(AsyncWebServerRequest *request) {
  /*
  req->discardRequestBody();
  res->setStatusCode(404);
  res->setStatusText("Not Found");
  res->setHeader("Content-Type", "text/html");
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Not Found</title></head>");
  res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
  res->println("</html>");
  */
    request->send(404, "text/plain", "Not found");
}

// handler to treat "root URL"
void PM_OTA_Web_Server_root(AsyncWebServerRequest *request) {
  /*
  res->setHeader("Content-Type", "text/html");

  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Hello World!</title></head>");
  res->println("<body>");
  res->println("<h1>Hello World!</h1>");

  res->print("<p>Your server is running for ");
  res->print((int)(millis()/1000), DEC);
  res->println(" seconds.</p>");

  // You can check if you are connected over a secure connection, eg. if you
  // want to use authentication and redirect the user to a secure connection
  // for that
  if (req->isSecure()) {
    res->println("<p>You are connected via <strong>HTTPS</strong>.</p>");
  } else {
    res->println("<p>You are connected via <strong>HTTP</strong>.</p>");
  }

  res->println("</body>");
  res->println("</html>");
  */
  request->send(200, "text/plain", "Hi! I am ESP32.");
}

#endif