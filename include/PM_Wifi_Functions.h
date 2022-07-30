/*
  Copyright 2022 - Yves Gaignard
  
  Functions relatives to the Wifi
*/
#ifndef PM_Wifi_Functions_h
#define PM_Wifi_Functions_h

//#include <WiFi.h>
#include <WiFiMulti.h>

#include <PM_Structures.h>
#include <PM_Constants.h>

// ===================================================================================================
// WifiDetectAndConnect : Function to detect the different wifi and to connect to the strengthest
// ===================================================================================================
boolean PM_Wifi_Functions_DetectAndConnect (WiFiMulti& wifiMulti);

#endif