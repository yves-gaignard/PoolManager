/*
  Copyright 2022 - Yves Gaignard
  
  Web Server management
*/
#ifndef PM_OTA_Web_Server_h
#define PM_OTA_Web_Server_h

#include <ESPAsyncWebServer.h>

// Declare handler functions for the various URLs on the server
void PM_OTA_Web_Server_notFound(AsyncWebServerRequest *request);
void PM_OTA_Web_Server_sendWelcome(AsyncWebServerRequest *request);

// Web server setup and start
void PM_OTA_Web_Server_setup(void);

// Web server in loop 
void PM_OTA_Web_Server_loop();

// handler to treat "page not found"
void PM_OTA_Web_Server_notFound(AsyncWebServerRequest *request);

// handler to treat "root URL"
void PM_OTA_Web_Server_root(AsyncWebServerRequest *request);

#endif