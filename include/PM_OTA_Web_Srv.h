/*
  Copyright 2022 - Yves Gaignard
  
  Web Server management
*/
#ifndef PM_OTA_Web_Srv_h
#define PM_OTA_Web_Srv_h

#include <ESPAsyncWebServer.h>

// Declare handler functions for the various URLs on the server
void PM_OTA_Web_Srv_notFound(AsyncWebServerRequest *request);
void PM_OTA_Web_Srv_sendWelcome(AsyncWebServerRequest *request);

// Web server setup and start
void PM_OTA_Web_Srv_setup(boolean isWebSerial);

// Web server in loop 
void PM_OTA_Web_Srv_loop();

// handler to treat "page not found"
void PM_OTA_Web_Srv_notFound(AsyncWebServerRequest *request);

// handler to treat "root URL"
void PM_OTA_Web_Srv_root(AsyncWebServerRequest *request);

#endif