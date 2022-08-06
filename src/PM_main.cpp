/*
  Yves Gaignard
  
  Main procedure of the Pool Manager project
*/

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// Standard library definitions
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>     // Library for LCD management
#include <WiFi.h>                  // Library for WiFi management
#include <WiFiMulti.h>             // Library for WiFi management
#include <ESPPerfectTime.h>        // Library for time maangement
#include <ESPAsyncWebServer.h>     // Library for Web Server Management
#include <OneWire.h>               // Library for onewire devices
#include <DallasTemperature.h>     // Library for temperature sensors

// Project definitions
#include "PM_Structures.h"         // Pool manager structure definitions
#include "PM_Parameters.h"         // Pool manager parameters
#include "PM_I2CScan.h"            // Pool manager I2C scan tools
#include "PM_Time_Mngt.h"          // Pool manager time management
#include "PM_Temperature.h"        // Pool manager time management
#include "PM_Wifi_Functions.h"     // Pool manager wifi management
#include "PM_OTA_Web_Server.h"     // Pool manager web server management
#include "PM_LCD.h"                // Pool manager display device management
#include "PM_Pool_Configuration.h" // Pool manager configuration parameters
#include "PM_Error.h"              // Pool manager error management
#include "PM_Utils.h"              // Pool manager utilities

static const char* TAG = "PM_main";

// Intantiate the Pool Manager configuration
static PM_Pool_Configuration Pool_Configuration;

// Array of I2C Devices
static byte I2CDevices[128];

// Instantiate LCD display and a screen template
PM_LCD lcd(PM_LCD_Device_Addr, PM_LCD_Cols, PM_LCD_Rows);
std::vector<std::string> screen;

// To manage the connection on Wifi
boolean IsWifiConnected   = false;

// To manage wifi between multiple networks
WiFiMulti wifiMulti;

// To manage time
time_t  now;

// Display LCD parameters
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


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature temperatureSensors(&oneWire);

// arrays to hold temperatur sensor device addresses
DeviceAddress insideThermometer, outsideThermometer, waterThermometer;

// Instantiate object to manage all temperature sensors
PM_Temperature PM_TemperatureSensors;

// Tasks declaration
TaskHandle_t Task_Main_Handle      = NULL;
TaskHandle_t Task_LCD_Handle       = NULL;
TaskHandle_t Task_WebServer_Handle = NULL;
TaskHandle_t Task_GPIO_Handle      = NULL;

void PM_Task_Main      ( void *pvParameters );
void PM_Task_LCD       ( void *pvParameters );
void PM_Task_WebServer ( void *pvParameters );
void PM_Task_GPIO      ( void *pvParameters );

// function declarations
void PM_Display_init    ();
void PM_Display_screen_0(PM_SwimmingPoolMeasures_str & measures);
void PM_Display_screen_1(PM_SwimmingPoolMeasures_str & measures);
void PM_LCD_displayKeyCodes();
void printAddress(DeviceAddress deviceAddress);

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
  esp_log_level_set("PM_Wifi_Functions", ESP_LOG_INFO); 
  
  //Init serial for logs
  Serial.begin(115200);
  ESP_LOGI(TAG, "Starting Project: [%s]  Version: [%s]",Project.Name.c_str(), Project.Version.c_str());
  
  //Init LCD
  PM_Display_init();
  
  // Discover all I2C Devices
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

  // Get current time
  time(&now);
  char timestamp_str[20];
  tm* time_tm = localtime(&now);
	strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
  ESP_LOGI(TAG, "Current date and local time is: %s", timestamp_str);
  
  // Start of diaplaying information
  PM_Display_Activation_Start=now;
  
  // start Web Server
  PM_OTA_Web_Server_setup();
  
  // Verify the configuration of pool manager
  PM_Error Error = Pool_Configuration.CheckFiltrationTimeAbaqus();
  int ErrorNumber = Error.getErrorNumber();
  if (ErrorNumber != 0) {
    ESP_LOGE(TAG, "The Filtration Time Abaqus does not respect the rules. Please check it !");
    std::string Display_ErrorNumber = "Error: "+Error.getErrorNumberStr();
    std::string Display_ErrorMessage = Error.getDisplayMsg();
    ESP_LOGE(TAG, "%s",Display_ErrorMessage.c_str());
    lcd.clear();
    lcd.printLine(0, Display_ErrorNumber);
    lcd.printScrollLine(1, Display_ErrorMessage, 60);
    for ( ;; ) {} //infinite loop as the configuration could not be wrong
  }

  Error = Pool_Configuration.CheckFiltrationPeriodAbaqus();
  if (Error.getErrorNumber() != 0) {
    ESP_LOGE(TAG, "The Filtration Period Abaqus does not respect the rules. Please check it !");
    std::string Display_ErrorNumber = "Error: " + Error.getErrorNumberStr();
    std::string Display_ErrorMessage = Error.getDisplayMsg();
    ESP_LOGE(TAG, "%s",Display_ErrorMessage.c_str());
    lcd.clear();
    lcd.printLine(0, Display_ErrorNumber);
    lcd.printScrollLine(1, Display_ErrorMessage, 60);
    for ( ;; ) {} //infinite loop as the configuration could not be wrong
  }
  

  // Display the first screen
  PM_Display_screen_0(pm_measures_str);
  PM_Display_Screen_Start=now;

  // Attribute the GPIOs
  pinMode(PM_DisplayButton_Pin, INPUT_PULLUP);
  attachInterrupt(PM_DisplayButton_Pin, PM_DisplayButton_ISR, FALLING);
  
  // Declare temperature sensors
  PM_TemperatureSensors.init(temperatureSensors);
  int tempSensorsNumber = PM_TemperatureSensors.getDeviceCount();
  ESP_LOGI(TAG, "%d temperature sensors found",tempSensorsNumber);

  for (int i = 0; i< tempSensorsNumber; i++){
    std::string deviceAddrStr = PM_TemperatureSensors.getDeviceAddress(i);
    ESP_LOGI(TAG, "sensor address [%d] : %s",i , deviceAddrStr.c_str() );
    if ( deviceAddrStr == insideThermometerAddress ) {
      PM_TemperatureSensors.addDevice(insideThermometerName, insideThermometerAddress);
    } else if (deviceAddrStr == outsideThermometerAddress) {
      PM_TemperatureSensors.addDevice(outsideThermometerName, outsideThermometerAddress);
    } else if (deviceAddrStr == waterThermometerAddress) {
      PM_TemperatureSensors.addDevice(waterThermometerName, waterThermometerAddress);
    } else {
      ESP_LOGI(TAG, "Unknown temperature sensor found. Its address is: %s",deviceAddrStr.c_str());
    }
  }
  
  // Create tasks
  //                          Function           Name          Stack  Param PRIO  Handle                core
  //xTaskCreatePinnedToCore(PM_Task_Main,      "Task_Main",      10000, NULL, 10, &Task_Main_Handle,      0);
  xTaskCreatePinnedToCore(PM_Task_GPIO,      "Task_GPIO",      10000, NULL,  8, &Task_GPIO_Handle,      0);
  xTaskCreatePinnedToCore(PM_Task_LCD,       "Task_LCD",       10000, NULL,  6, &Task_LCD_Handle,       1);
  xTaskCreatePinnedToCore(PM_Task_WebServer, "Task_WebServer", 10000, NULL,  9, &Task_WebServer_Handle, 1);

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
      if (lcd.getDisplayState() == false ) {  
        // the LCD is OFF. We just set it ON again
        lcd.display();
        ESP_LOGV(TAG, "%s : Display is shown",timestamp_str);
        // reset the display duration counter
        PM_Display_Activation_Start=now;
      }
      PM_Display_Activation_Request=false;
    }

    // If the display is ON
    if (lcd.getDisplayState() == true) {
      // if the timout of the current screen is reached
      if ( (now - PM_Display_Screen_Start) >= PM_Display_Screen_Duration) { 
        // display the next screen
        int screen_index= (PM_Display_Current_Screen_Index+1)%PM_Display_Screen_Number;
        ESP_LOGV(TAG, "%s : Screen index %d",timestamp_str, screen_index);
        switch (screen_index) {
          case 0 : PM_Display_screen_0(pm_measures_str);
              PM_Display_Screen_Start=now;
              ESP_LOGD(TAG, "%s : Display screen %d",timestamp_str, screen_index);
            break;
          case 1 : PM_Display_screen_1(pm_measures_str);
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
        lcd.noDisplay();
        lcd.noBacklight();
        ESP_LOGD(TAG, "%s : Display is stopped",timestamp_str);
      }
    }
    vTaskDelay( pdMS_TO_TICKS( 5000 ) );
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

    vTaskDelay( pdMS_TO_TICKS( 3000 ) );
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

  std::string deviceName;
  float preciseTemperatureC;
  int   temperatureC;

  for( ;; ) {
    time(&now);
    time_tm = localtime(&now);
	  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    ESP_LOGD(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);


    PM_TemperatureSensors.requestTemperatures();
    for (int i = 0 ; i < PM_TemperatureSensors.getDeviceCount(); i++) {
      deviceName = PM_TemperatureSensors.getDeviceNameByIndex(i);

      preciseTemperatureC = PM_TemperatureSensors.getPreciseTempCByName(deviceName);
      ESP_LOGI(TAG, "%fºC : sensor: %s)",preciseTemperatureC, deviceName.c_str());
      temperatureC = PM_TemperatureSensors.getTempCByName(deviceName);
      ESP_LOGI(TAG, "%d°C : sensor: %s)",temperatureC, deviceName.c_str());

      if (deviceName == insideThermometerName) {
        pm_measures.InAirTemp = preciseTemperatureC;
        pm_measures_str.InAirTemp_str = PM_itoa(temperatureC);
      }
      else if (deviceName == outsideThermometerName){
        pm_measures.OutAirTemp = preciseTemperatureC;
        pm_measures_str.OutAirTemp_str = PM_itoa(temperatureC);
      } 
      else if (deviceName == waterThermometerName) {
        pm_measures.WaterTemp = preciseTemperatureC;
        pm_measures_str.WaterTemp_str = PM_itoa(temperatureC);
      }
    }

    vTaskDelay( pdMS_TO_TICKS( 15000 ) );
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


// =================================================================================================
//                              SCREEN DEFINITIONS
// =================================================================================================
/*  
   |--------------------|
   |         1         2|
   |12345678901234567890|
   |--------------------|
   |     SCREEN INIT    |
   |--------------------|
   |Pool Manager        |
   |Version: 1.0.0      |
   |                    | 
   |Yves Gaignard       |
   |--------------------|
*/ 
void PM_Display_init    () {
  lcd.init();
  lcd.clear();
  lcd.display();
  lcd.backlight();

  screen.clear();
  screen.push_back(Project.Name);
  screen.push_back("Version: " + Project.Version);
  screen.push_back("");
  screen.push_back(Project.Author);

  lcd.printScreen(screen);
}

/*  
   |--------------------|
   |         1         2|
   |12345678901234567890|
   |--------------------|
   |     SCREEN 1       |
   |--------------------|
   |In:28 W:26 Out:15   |
   |ph:7.2 Cl:150 <460  | or |ph:7.2 Cl:150 >600  | 
   |Filter: 12h15 / 16h | 
   |pH-:12.6l Cl:15.2l  |
   |--------------------|
*/  
void PM_Display_screen_0(PM_SwimmingPoolMeasures_str & measures) {
  std::string DisplayLine;
  lcd.clear();
  lcd.display();
  lcd.backlight();
  char degreeAsciiChar[2];
  sprintf(degreeAsciiChar, "%c", 176);
  screen[0] = "In:"+measures.InAirTemp_str+degreeAsciiChar+" W:"+measures.WaterTemp_str+degreeAsciiChar+" Out:"+measures.OutAirTemp_str+degreeAsciiChar;
  DisplayLine = "PH:"+measures.pH_str+" Cl:"+measures.Chlorine_str;
  if (measures.Chlorine_str < measures.ChlorineMin_str) DisplayLine+=" <"+measures.ChlorineMin_str;
  if (measures.Chlorine_str > measures.ChlorineMax_str) DisplayLine+=" >"+measures.ChlorineMax_str;
  screen[1] = DisplayLine;
  screen[2] = "Filter:"+measures.DayFilterTime_str+" / "+measures.MaxDayFilterTime_str;
  screen[3] = "PH-:"+measures.pHMinusVolume_str+" Cl:"+measures.ChlorineVolume_str;

  lcd.printScreen(screen);
}

/*
   |--------------------|
   |         1         2|
   |12345678901234567890|
   |--------------------|
   |     SCREEN 2       |
   |--------------------|
   |Filter:OFF P:1015hPa| or |Filter:ON  P:1015hPa|
   |Pow:1200W Day:4.2kWh|
   |Pump pH-:OFF Cl:OFF | or |Pump pH-:ON  Cl:ON  |
   |Max pH-:20l Cl:20l  |
   |--------------------|
*/
void PM_Display_screen_1(PM_SwimmingPoolMeasures_str & measures) {
  lcd.clear();
  lcd.display();
  lcd.backlight();

  screen[0] = "Filter:"+measures.FilterPumpState_str+" P:"+measures.Pressure_str+"hPa";
  screen[1] = "Pow:"+measures.ConsumedInstantaneousPower_str+" Day:"+measures.Chlorine_str+"kWh";
  screen[2] = "Pump PH-:"+measures.pHMinusPumpState_str+" Cl:"+measures.ChlorinePumpState;
  screen[3] = "Max PH-:"+measures.pHMinusMaxVolume_str+" Cl:"+measures.ChlorineMaxVolume_str;

  lcd.printScreen(screen);
}

// display all keycodes
void PM_LCD_displayKeyCodes(void) {
  uint8_t i = 0;
  LiquidCrystal_I2C* lcd_in = lcd.getLCD();
  int colNumber = lcd.getColumnNumber();
  char ch[10] ;

  while (1) {
    lcd_in->clear();
    lcd_in->display();
    lcd_in->print("Codes 0x"); lcd_in->print(i, HEX);
    lcd_in->print("-0x"); lcd_in->print(i+colNumber-1, HEX);
    lcd_in->setCursor(0, 1);
    std::string line="";
    for (int j=0; j<colNumber; j++) {
      sprintf(ch, "%c", (i+j));
      //sprintf(ch, "%c", 176); // ° 
      line+=ch;
    }
    lcd_in->printf(line.c_str());
    ESP_LOGD(TAG, "Line displayed: %s", line );
    i+=colNumber;
    
    delay(4000);
  }
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}