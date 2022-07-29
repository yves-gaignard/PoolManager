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
#include "PM_Time_Mngt.h"       // Pool manager time management
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

// To manage time
time_t  now;

// Display LCD parameter
time_t  PM_Display_Activation_Start=0;            // time of the last LCD activation
time_t  PM_Display_Max_Time_Without_Activity=300; // duration of displaying information without any user interaction
int     PM_Display_Current_Screen_Index=1;        // Current displayed screen index 
int     PM_Display_Screen_Number=2;               // Total screen number
boolean PM_Display_Activation_Request=true;       // Request to activate the display
boolean PM_Display_Status=true;                   // Status of the current 


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
  esp_log_level_set("PM_*", ESP_LOG_INFO); 

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
  PM_Time_Mngt_initialize_time();
  //time_now.getCurrentNTPTime();
  time(&now);
  ESP_LOGI(TAG, "Current date and local time is: %s", PM_Time_Mngt_convertTimeToString(now, "%d/%m/%Y %H:%M:%S").c_str());
  
  // Start of diaplaying information
  PM_Display_Activation_Start=now;
  
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

  time(&now);
  ESP_LOGI(TAG, "Current date and local time is: %s", PM_Time_Mngt_convertTimeToString(now, "%d/%m/%Y %H:%M:%S").c_str());
  //ESP_LOGI(TAG, "Current date and gmt   time is: %s", PM_Time_Mngt_convertTimeToString(now, "%d/%m/%Y %H:%M:%SZ").c_str());

  if ( (now - PM_Display_Activation_Start) % 10000 < 5000) {
    PM_Display_Activation_Request = true;
  }
  else {
    PM_Display_Activation_Request = false;
  }


  // if lcd activate button is pressed then print next screen
  if ( PM_Display_Activation_Request == true) {
    if (PM_Display_Status == false) {
      // the LCD was shutdowned. We just display it again
      PM_Display_Display(lcd);
    }
    else {
      // display the next screen
      int screen_index= (PM_Display_Current_Screen_Index+1)%PM_Display_Screen_Number;
      switch (screen_index) {
        case 0 : PM_Display_screen_0(lcd, pm_measures_str);
          break;
        case 1 : PM_Display_screen_1(lcd, pm_measures_str);
          break;
        default:
          ESP_LOGE(TAG, "Unknown screen: %d", screen_index);
      }
    }
    // reset the display duration counter
    PM_Display_Activation_Start=now;
  }

  // if no_activity at all during MAX_WITHOUT_ACTIVITES then stop the display
  if (now - PM_Display_Activation_Start >= PM_Display_Max_Time_Without_Activity ) {
    PM_Display_noDisplay(lcd);
    PM_Display_Status = false;
  }

}

