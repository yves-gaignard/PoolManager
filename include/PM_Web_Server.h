/*
  Copyright 2022 - Yves Gaignard
  
  Functions relatives to the OTA web server
*/
#ifndef web_server_h
#define web_server_h

//#include <AsyncTCP.h>
//#include <ESPAsyncWebServer.h>
//#include <AsyncElegantOTA.h>

//#include <PM_Structures.h>
//#include <PM_Constants.h>

// timestamp to trigger restart on
extern uint32_t g_restartTime;
// timestamp of last client access
extern uint32_t g_lastAccessTime;

/**
 * Start web server
 */
void webserver_start();

#endif