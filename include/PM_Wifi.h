/*
  Copyright 2022 - Yves Gaignard
  
  Functions relatives to the Wifi
*/
#ifndef PM_Wifi_h
#define PM_Wifi_h

//#include <WiFi.h>
#include <WiFiMulti.h>

// ===================================================================================================
// WifiDetectAndConnect : Function to detect the different wifi and to connect to the strengthest
// ===================================================================================================
boolean PM_Wifi_DetectAndConnect (WiFiMulti& wifiMulti);

#endif