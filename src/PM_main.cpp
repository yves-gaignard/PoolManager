/*
  Yves Gaignard
  
  Main procedure of the Pool Manager project
*/

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// Standard library definitions
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>  // Library for LCD management
#include <WiFi.h>               // Library for WiFi management
#include <WiFiMulti.h>          // Library for WiFi management
#include <ESPPerfectTime.h>     // Library for time maangement
#include <ESPAsyncWebServer.h>  // Library for Web Server Management 

// Project definitions
#include "PM_Structures.h"      // Pool manager structure definitions
#include "PM_Constants.h"       // Pool manager constants definitions
#include "PM_I2CScan.h"         // Pool manager I2C scan tools
#include "PM_Time.h"            // Pool manager time management
#include "PM_Wifi_Functions.h"  // Pool manager wifi management
#include "PM_Web_Server.h"      // Pool manager web server management
#include "PM_OTA_Web_Server.h"  // Pool manager web server management
#include "PM_Display.h"         // Pool manager display device management

static const char* TAG = "PM_main";

// Array of I2C Devices
static byte I2CDevices[128];

// List the type of web server possible
enum WebServerType { 
  OTAWebServer,   // HTTP web server supporting OTA update (Over-The-Air)
  WebServer       // HTTP and HTTPS web servers
};

// Define LCD display
LiquidCrystal_I2C lcd(0x27, 20, 4); 

// Select the type of web server to instantiate
WebServerType ServerType = OTAWebServer;

// To manage the connection on Wifi
boolean IsWifiConnected    = false;

// To manage wifi between multiple networks
WiFiMulti wifiMulti;

// To manage time from NTP server
PM_Time time_now;
time_t  current_time;

// swimming pool measures
PM_SwimmingPoolMeasures     pm_measures     = { 0.0,  0.0,  0.0,   0.0,     0,   450,   750,      0,       0  , 0.0,     0.0,     0,      0,     0.0, false, false, false,   0.0,    0.0  }; 
PM_SwimmingPoolMeasures_str pm_measures_str = { "00", "00", "00", "0.0", "000", "450", "750", "00h00", "00h00","00.0", "00.0", "0000", "0000", "0000", "OFF", "OFF", "OFF", "00.0", "00.0" }; 

// =================================================================================================
//
//                                        SETUP OF POOL MANAGER
//
// =================================================================================================
void setup() {

  esp_log_level_set("*", ESP_LOG_ERROR);        // set all components to ERROR level
  esp_log_level_set("PM_*", ESP_LOG_VERBOSE); 

  Serial.begin(115200);
  ESP_LOGI(TAG, "Starting Project: [%s]  Version: [%s]",Project.Name.c_str(), Project.Version.c_str());
  
  //Init LCD
  PM_Display_init(lcd);

  // Scan all I2C Devices
  int I2CDeviceNumber=0;
  I2CDeviceNumber = PM_I2CScan_Scan(I2CDevices);
  // Print the I2C Devices
  PM_I2CScan_Print(I2CDeviceNumber, I2CDevices);


  // Connect to the strengthest known wifi network
  while ( ! IsWifiConnected){
    IsWifiConnected=PM_Wifi_Functions_DetectAndConnect (wifiMulti);
    delay(100);
  }

  // Initialize time
  time_now.getCurrentNTPTime();
  
  // start Web Server
  switch(ServerType)
  {
    case OTAWebServer: PM_OTA_Web_Server_setup();   break;
    case WebServer:    PM_Web_Server_setup(); break;
  }
  delay(100);
}

// =================================================================================================
//
//                                        LOOP OF POOL MANAGER
//
// =================================================================================================
void loop(void) {
  // start Web Server
  switch(ServerType)
  {
    case OTAWebServer: PM_OTA_Web_Server_loop();   break;
    case WebServer:    PM_Web_Server_loop(); break;
  }
  delay(100);

  //current_time = time_now.getCurrentNTPTime();
  //ESP_LOGI(TAG, "Get Date and Time from NTP server: %s", time_now.convertTimeToString(current_time).c_str());
  //lcd.setCursor(0,3);
  //lcd.print(time_now.convertTimeToString(current_time,"%d/%m/%Y %H:%M:%S").c_str());

  PM_Display_screen_1(lcd, pm_measures_str);
  delay(5000);

  PM_Display_noDisplay(lcd);
  delay(500);


  PM_Display_screen_2(lcd, pm_measures_str);
  delay(5000);

  PM_Display_noDisplay(lcd);
  delay(500);
}

