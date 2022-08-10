/*
  Yves Gaignard
  
  OTA Web Server management
*/

#define TAG "PM_OTA_Web_Srv"

// Standard library definitions
#include <Arduino.h>
#include <AsyncElegantOTA.h>

#include "PM_Pool_Manager.h"
#include "PM_Parameters.h"      // Pool manager parameters
#include "PM_OTA_Web_Srv.h"

AsyncWebServer OTAServer(80);

// Web server setup and start
void PM_OTA_Web_Srv_setup(void) {

  OTAServer.on("/", HTTP_GET, PM_OTA_Web_Srv_root);
  OTAServer.onNotFound(PM_OTA_Web_Srv_notFound);

  AsyncElegantOTA.begin(&OTAServer);    // Start ElegantOTA
  OTAServer.begin();
  LOG_I(TAG, "HTTP server started");
}

// handler to treat "page not found"
void PM_OTA_Web_Srv_notFound(AsyncWebServerRequest *request) {
  int http_rc = 404;
  char Response_type[]="text/html";
  String Response;
  Response+="<!DOCTYPE html>";
  Response+="<html>";
  Response+="<head><title>Not Found</title></head>";
  Response+="<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>";
  Response+="</html>";
  request->send(http_rc, Response_type, Response);
  LOG_V(TAG, "send %d %s \nResponse: %s", http_rc, Response_type, Response);
}

// handler to treat "root URL"
void PM_OTA_Web_Srv_root(AsyncWebServerRequest *request) {
  int http_rc = 200;
  char Response_type[]="text/html";
  String Response;
  Response+="<!DOCTYPE html>";
  Response+="<html>";
  Response+="<head><title>Hello World!</title></head>";
  Response+="<body>";
  Response+="<h1>Hello World!</h1>";
  Response+="<p>I am Project: " + String(Project.Name.c_str()) + " Version: "+ String(Project.Version.c_str()) + " and I am running for ";
  Response+=((int)(millis()/1000));
  Response+=" seconds.</p>";
  Response+="<p>You are connected via <strong>HTTP</strong>.</p>";
  Response+="</body>";
  Response+="</html>";
  request->send(http_rc, Response_type, Response);
  LOG_V(TAG, "send %d %s \nResponse: %s", http_rc, Response_type, Response);
}
