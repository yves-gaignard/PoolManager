/*
  Copyright 2022 - Yves Gaignard
  
  Functions relatives to the Wifi
*/
#ifndef wifi_functions_h
#define wifi_functions_h

#include <WiFi.h>
#include <WiFiMulti.h>

#include <PM_Structures.h>
#include <PM_Constants.h>

// WiFi connect timeout per AP. Increase when connecting takes longer.
const uint32_t wifiConnectTimeoutMs = 10000;

// ===================================================================================================
// WifiDetectAndConnect : Function to detect the different wifi and to connect to the strengthest
// ===================================================================================================
static boolean WifiDetectAndConnect (WiFiMulti& wifiMulti) {

  WiFi.mode(WIFI_STA);
  // Configures static IP address
  if (!WiFi.config(WifiConf.local_IP, WifiConf.gateway, WifiConf.subnet, WifiConf.primaryDNS, WifiConf.secondaryDNS)) {
    Serial.println("Wifi STA Failed to configure");
    return false;
  }

  // Add list of known wifi networks
  wifiMulti.addAP(WifiMM.ssid.c_str(), WifiMM.password.c_str());
  wifiMulti.addAP(WifiLaConche.ssid.c_str(), WifiLaConche.password.c_str());
  
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("#INFO: Wifi scan done");
  if (n == 0) {
      Serial.println("#ERROR: No known networks found");
      return false;
  } 
  else {
    Serial.println("#INFO: "+ String(n)+" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.println("#INFO: "+String(i+1)+": "+WiFi.SSID(i)+" ("+WiFi.RSSI(i)+")"+(WiFi.encryptionType(i) == WIFI_AUTH_OPEN?" ":"*"));
      delay(10);
    }
  }

  // Connect to Wi-Fi using wifiMulti (connects to the SSID with strongest connection)
  Serial.println("");
  Serial.println("#INFO: Connecting Wifi...");
  if(wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("#INFO: WiFi connected to : " + WiFi.SSID());
    Serial.println("#INFO: WiFi strength     : " + String(WiFi.RSSI()));
    Serial.println("#INFO: Local IP address  : " + WiFi.localIP().toString());
  }

  return true;
}

#endif