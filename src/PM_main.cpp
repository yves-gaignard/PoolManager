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
#include "PM_Parameters.h"      // Pool manager parameters
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
time_t  PM_Display_Max_Time_Without_Activity=20;  // duration of displaying informat
time_t  PM_Display_Activation_Start=0;            // time of the last LCD activationion without any user interaction
time_t  PM_Display_Screen_Start=0;                // time of the current screen start
time_t  PM_Display_Screen_Duration=5;             // time of display for a screen
int     PM_Display_Current_Screen_Index=1;        // Current displayed screen index 
int     PM_Display_Screen_Number=2;               // Total screen number
boolean PM_Display_Activation_Request=true;       // Request to activate the display

// swimming pool measures
PM_SwimmingPoolMeasures     pm_measures     = { 0.0,  0.0,  0.0,   0.0,     0,   450,   750,      0,       0  , 0.0,     0.0,     0,      0,     0.0, false, false, false,   0.0,    0.0  }; 
PM_SwimmingPoolMeasures_str pm_measures_str = { "00", "00", "00", "0.0", "000", "450", "750", "00h00", "00h00","00.0", "00.0", "0000", "0000", "0000", "OFF", "OFF", "OFF", "00.0", "00.0" }; 


// Tasks declaration
TaskHandle_t Task_Main_Handle      = NULL;
TaskHandle_t Task_LCD_Handle       = NULL;
TaskHandle_t Task_WebServer_Handle = NULL;
TaskHandle_t Task_GPIO_Handle      = NULL;

void PM_Task_Main      ( void *pvParameters );
void PM_Task_LCD       ( void *pvParameters );
void PM_Task_WebServer ( void *pvParameters );
void PM_Task_GPIO      ( void *pvParameters );

// Button declarations
boolean PM_DisplayButton_State = false;    // Current State
int     PM_DisplayButton_LastPressed = 0;  // last time it was pressed in millis
void IRAM_ATTR PM_DisplayButton_ISR();     // Interuption function

// =================================================================================================
//
//                                        SETUP OF POOL MANAGER
//
// =================================================================================================
void setup() {

  // Log level for each part of the project
  esp_log_level_set("*",                 ESP_LOG_ERROR);        // set all components to ERROR level
  esp_log_level_set("PM_Display",        ESP_LOG_INFO); 
  esp_log_level_set("PM_main",           ESP_LOG_INFO); 
  esp_log_level_set("PM_OTA_Web_Server", ESP_LOG_INFO); 
  esp_log_level_set("PM_Time_Mngt",      ESP_LOG_INFO); 
  esp_log_level_set("PM_Time",           ESP_LOG_INFO); 
  esp_log_level_set("PM_Web_Server",     ESP_LOG_INFO); 
  esp_log_level_set("PM_Wifi_Functions", ESP_LOG_INFO); 
  
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

  PM_Display_screen_0(lcd, pm_measures_str);
  PM_Display_Screen_Start=now;

  // Create tasks
  //                          Function           Name          Stack  Param PRIO  Handle                core
  //xTaskCreatePinnedToCore(PM_Task_Main,      "Task_Main",      10000, NULL, 10, &Task_Main_Handle,      0);
  xTaskCreatePinnedToCore(PM_Task_GPIO,      "Task_GPIO",      10000, NULL,  8, &Task_GPIO_Handle,      0);
  xTaskCreatePinnedToCore(PM_Task_LCD,       "Task_LCD",       10000, NULL,  6, &Task_LCD_Handle,       1);
  xTaskCreatePinnedToCore(PM_Task_WebServer, "Task_WebServer", 10000, NULL,  9, &Task_WebServer_Handle, 1);

  // Attribute the GPIOs
  pinMode(PM_DisplayButton_Pin, INPUT_PULLUP);
  attachInterrupt(PM_DisplayButton_Pin, PM_DisplayButton_ISR, FALLING);

  // Infinite loop to never go to main loop. Everything is treated in a task 
  for( ;; ) {}
}
// =================================================================================================
//                                   LOOP OF POOL MANAGER
// =================================================================================================
void loop(void) {
   // Nothing here
}

// =================================================================================================
//                                  MAIN TASK OF POOL MANAGER
// =================================================================================================
void PM_Task_Main      ( void *pvParameters ) {
  //const char *pcTaskName = "Task_Main";
  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20]; 
  for( ;; ) {
    time(&now);
    time_tm = localtime(&now);
	  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    ESP_LOGD(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);

    vTaskDelay( pdMS_TO_TICKS( 50000 ) );
  }
}
// =================================================================================================
//                               LCD MANAGEMENT TASK OF POOL MANAGER
// =================================================================================================
void PM_Task_LCD       ( void *pvParameters ) {
  //const char *pcTaskName = "Task_LCD";
  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20];
  for( ;; ) {
    time(&now);
    time_tm = localtime(&now);
	  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    ESP_LOGD(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);

    // if lcd display button is pressed then set the display ON in case of OFF
    if ( PM_Display_Activation_Request == true) {
      if (PM_Display_getState(lcd) == false ) {  
        // the LCD is OFF. We just set it ON again
        PM_Display_Display(lcd);
        ESP_LOGV(TAG, "%s : Display is shown",timestamp_str);
        // reset the display duration counter
        PM_Display_Activation_Start=now;
      }
      PM_Display_Activation_Request=false;
    }

    // If the display is ON
    if (PM_Display_getState(lcd) == true) {
      // if the timout of the current screen is reached
      if ( (now - PM_Display_Screen_Start) >= PM_Display_Screen_Duration) { 
        // display the next screen
        int screen_index= (PM_Display_Current_Screen_Index+1)%PM_Display_Screen_Number;
        ESP_LOGV(TAG, "%s : Screen index %d",timestamp_str, screen_index);
        switch (screen_index) {
          case 0 : PM_Display_screen_0(lcd, pm_measures_str);
              PM_Display_Screen_Start=now;
              ESP_LOGD(TAG, "%s : Display screen %d",timestamp_str, screen_index);
            break;
          case 1 : PM_Display_screen_1(lcd, pm_measures_str);
              PM_Display_Screen_Start=now;
              ESP_LOGD(TAG, "%s : Display screen %d",timestamp_str, screen_index);
            break;
          default:
            ESP_LOGE(TAG, "%s : Cannot Display screen %d",timestamp_str, screen_index);
        }
        PM_Display_Current_Screen_Index=screen_index;
      } 

      // if no_activity at all during MAX_WITHOUT_ACTIVITES then stop the display
      if (now - PM_Display_Activation_Start >= PM_Display_Max_Time_Without_Activity ) {
        PM_Display_noDisplay(lcd);
        ESP_LOGD(TAG, "%s : Display is stopped",timestamp_str);
      }
    }
    vTaskDelay( pdMS_TO_TICKS( 2000 ) );
  }
}
// =================================================================================================
//                                     WEB SERVER MANAGEMENT TASK OF POOL MANAGER
// =================================================================================================
void PM_Task_WebServer ( void *pvParameters ) {
  //const char *pcTaskName = "Task_WebServer";
  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20];
  for( ;; ) {
    time(&now);
    time_tm = localtime(&now);
	  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    ESP_LOGD(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);

    // run Web Server
    switch(ServerType) {
      case OTAWebServer: PM_OTA_Web_Server_loop();   break;
      case WebServer:    PM_Web_Server_loop(); break;
    }

    vTaskDelay( pdMS_TO_TICKS( 1000 ) );
  }
}
// =================================================================================================
//                                SENSOR MANAGEMENT TASK OF POOL MANAGER
// =================================================================================================
void PM_Task_GPIO      ( void *pvParameters ) {
  //const char *pcTaskName = "Task_GPIO";
  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20];
  for( ;; ) {
    time(&now);
    time_tm = localtime(&now);
	  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    ESP_LOGD(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);


    vTaskDelay( pdMS_TO_TICKS( 25000 ) );
  }

}

// =================================================================================================
//                              DISPLAY BUTTON INTERRUPTION MANAGEMENT
// =================================================================================================
void IRAM_ATTR PM_DisplayButton_ISR() {
  if (millis() - PM_DisplayButton_LastPressed > 100) { // Software debouncing button
    ESP_LOGD(TAG, "Display button pressed");
    PM_Display_Activation_Request = true;
    PM_DisplayButton_State = !PM_DisplayButton_State;
  }
  PM_DisplayButton_LastPressed = millis();
}


