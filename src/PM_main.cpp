/*
  Yves Gaignard
  
  Main procedure of the Pool Manager project
*/

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// Standard library definitions
#include <Arduino.h>
#include <WiFi.h>               // Library for WiFi management
#include <WiFiMulti.h>          // Library for WiFi management
#include <ESPPerfectTime.h>     // Library for time maangement
#include <ESPAsyncWebServer.h>  // Library for Web Server Management 

// Project definitions
#include <PM_Structures.h>      // Pool manager structure definitions
#include <PM_Constants.h>       // Pool manager constants definitions
#include <PM_Time.h>            // Pool manager time management
#include <PM_Wifi_Functions.h>  // Pool manager wifi management
// #include <PM_Web_Server.h>      // Pool manager web server management
#include <PM_OTA_Web_Server.h>  // Pool manager web server management

static const char* TAG = "PM_main";

// Variables
boolean IsWifiConnected    = false;
boolean IsWebServerStarted = false;

// To manage wifi between multiple networks
WiFiMulti wifiMulti;

// To manage time
PM_Time time_now;
time_t  current_time;

void setup() {

  esp_log_level_set("*", ESP_LOG_ERROR);        // set all components to ERROR level
  esp_log_level_set("PM_*", ESP_LOG_VERBOSE); 

  Serial.begin(115200);
  ESP_LOGI(TAG, "Starting Project: [%s]  Version: [%s]",Project.Name.c_str(), Project.Version.c_str());

  // Connect to the strengthest known wifi network
  while ( ! IsWifiConnected){
    IsWifiConnected=WifiDetectAndConnect (wifiMulti);
    delay(100);
  }

  // Initialize time
  time_now.getCurrentNTPTime();
  
  // start Web Server
  PM_OTA_Web_Server_setup();
  //PM_Web_Server_setup();
  //while ( ! IsWebServerStarted){
  //  IsWebServerStarted=StartWebServer (server);
  //  delay(100);
  //}

}

void loop(void) {
  //PM_Web_Server_loop();
  delay(2000);
  current_time = time_now.getCurrentNTPTime();
  ESP_LOGI(TAG, "Get Date and Time from NTP server: %s", time_now.convertTimeToString(current_time).c_str());
}