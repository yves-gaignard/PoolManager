/*
  Copyright 2022 - Yves Gaignard
  
  Functions relatives to the Wifi
*/
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// Standard library definitions
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include "PM_Wifi_Functions.h"
#include "PM_Structures.h"
#include "PM_Parameters.h"      // Pool manager parameters

static const char* TAG = "PM_Wifi_Functions";

// WiFi connect timeout per AP. Increase when connecting takes longer.
const uint32_t wifiConnectTimeoutMs = 10000;

// ===================================================================================================
// WifiDetectAndConnect : Function to detect the different wifi and to connect to the strengthest
// ===================================================================================================
boolean PM_Wifi_Functions_DetectAndConnect (WiFiMulti& wifiMulti) {

  String Log_Msg;

  WiFi.mode(WIFI_STA);
  // Configures static IP address
  if (!WiFi.config(WifiConf.local_IP, WifiConf.gateway, WifiConf.subnet, WifiConf.primaryDNS, WifiConf.secondaryDNS)) {
    ESP_LOGE(TAG, "Wifi STA Failed to configure");
    return false;
  }

  // Add list of known wifi networks
  wifiMulti.addAP(WifiMM.ssid.c_str(), WifiMM.password.c_str());
  wifiMulti.addAP(WifiLaConche.ssid.c_str(), WifiLaConche.password.c_str());
  
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  ESP_LOGI(TAG, "Wifi scan done");
  if (n == 0) {
      ESP_LOGE(TAG, "No known networks found");
      return false;
  } 
  else {
    Log_Msg=String(n) +" networks found";
    ESP_LOGI(TAG, "%s", Log_Msg.c_str());
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Log_Msg=String(i+1)+": "+WiFi.SSID(i)+" ("+WiFi.RSSI(i)+")"+(WiFi.encryptionType(i) == WIFI_AUTH_OPEN?" ":"*");
      ESP_LOGI(TAG, "%s", Log_Msg.c_str());
      delay(10);
    }
  }

  // Connect to Wi-Fi using wifiMulti (connects to the SSID with strongest connection)
  ESP_LOGI(TAG, "Connecting Wifi...");
  if(wifiMulti.run() == WL_CONNECTED) {
    Log_Msg="WiFi connected to : " + WiFi.SSID();
    ESP_LOGI(TAG, "%s", Log_Msg.c_str());
    Log_Msg="WiFi strength     : " + String(WiFi.RSSI());
    ESP_LOGI(TAG, "%s", Log_Msg.c_str());
    Log_Msg="Local IP address  : " + WiFi.localIP().toString();
    ESP_LOGI(TAG, "%s", Log_Msg.c_str());
  }

  return true;
}