/*
  Copyright 2022 - Yves Gaignard
  
  Parameters of the Pool Manager project
*/
#ifndef PM_Parameters_h
#define PM_Parameters_h

#include <Arduino.h>
#include "PM_Structures.h"
#include "PM_Secrets.h"

// Project name, version and author
// ---------------------------------
const PM_ProjectStruct Project {"Pool Manager", "1.0.0", "Yves Gaignard"};

// =========================================================================================================
//                                       Hardware parameters
// =========================================================================================================

// LCD parameters
// ------------------
const uint8_t PM_LCD_Device_Addr = 0x27;
const uint8_t PM_LCD_Cols        = 20;
const uint8_t PM_LCD_Rows        =  4;

// DS3231 parameters
// ------------------
const uint8_t PM_DS3231_Device_Addr     = 0x57;  // The DS3231 device itself
const uint8_t PM_AT24C32_Device_Address = 0x68;  // (4k EEPROM) contained by the DS3231 device.


// Setup an DFRobot_ADS1115 instance for analog measurements
// -----------------------------------------------------------
const uint8_t PM_ADS1115_Device_Addr = 0x48; // address of the DFRobot_ADS1115 device

// list of your Wifi networks with their credentials
// --------------------------------------------------
// You can put credentials in a file: PM_Secrets.h 
// (see an example in file PM_Secrets_examples.h)
const PM_WifiNetwork WifiMM       { WIFI_MM_SSID, WIFI_MM_PASSWORD };
const PM_WifiNetwork WifiLaConche { WIFI_LA_CONCHE_SSID, WIFI_LA_CONCHE_PASSWORD };

// Configuration of the server:
// ----------------------------
// Local IP configuration                   local_IP,            gateway_IP,             subnet,               primaryDNS,        secondaryDNS
const PM_WifiConfig  WifiConf     {IPAddress(192,168,1,99), IPAddress(192,168,1,1), IPAddress(255,255,0,0), IPAddress(1,1,1,1), IPAddress(8,8,8,8)};
// Listening port
const int  PM_WebServerPort = 80; 

// Configuration of the temperature sensors:
// ------------------------------------------
// DS18B20 sensors address
const std::string insideThermometerName     = "Inside Thermometer";
const std::string insideThermometerAddress  = "28569948F6943C83";    // Number: 3 Long

const std::string outsideThermometerName    = "Outside Thermometer";
const std::string outsideThermometerAddress = "283A5548F6AD3CA1";    // Number: 2 Long

const std::string waterThermometerName      = "Water Thermometer";
const std::string waterThermometerAddress   = "28675195F0013C34";    // Number: 2 Small

//const std::string waterThermometerAddress   = "28CF0F95F0013CFE";    // Number: 1 small
//const std::string outsideThermometerAddress = "28675195F0013C34";    // Number: 2 small
//const std::string waterThermometerAddress   = "289F1E95F0013C61";    // Number: 3 small

// Configuration of the GPIOs
// ---------------------------
// Pin number to set or not set the LED backlight of the TFT
#define PM_TFT_Led_Pin        32

// One wire (temperature sensors) is plugged into GPIO 33
#define ONE_WIRE_BUS          33 

// Pump management 
#define FILTRATION_PUMP_Pin   26
#define PH_PUMP_Pin           14
#define CHL_PUMP_Pin          25

// Tank reset button
#define CHL_TANK_LEVEL_Pin     5
#define PH_TANK_LEVEL_Pin     17

// Light Buzzer
#define LIGHT_BUZZER_Pin      16

// Winter mode button
#define WINTER_MODE_Pin       33

// Sensors using ADS1115
#define PH_SENSOR_ANALOG_Pin       0
#define ORP_SENSOR_ANALOG_Pin      1
#define PRESSURE_SENSOR_ANALOG_Pin 2

//Digital input pins connected to Acid and Chl tank level reed switches
//#define CHL_LEVEL             39   // not wired. Use NO_LEVEL option of Pump class
//#define PH_LEVEL              36   //                - " -

// Timeout period of the Task Watchdog Timer (TWDT) in seconds
#define WDT_TIMEOUT           10

// Loop tasks scheduling parameters
//---------------------------------
// T1 : PM_Task_AnalogPoll
// T2 : PM_Task_ProcessCommand
// T3 : PM_Task_Pool_Manager
// T4 : PM_Task_GetTemperature
// T5 : PM_Task_OrpRegulation
// T6 : PM_Task_pHRegulation
// T7 : PM_Task_TFT
// T8 : PM_Task_WebServer
// T9 : PM_Task_MeasuresPublish
// T10: PM_Task_SettingsPublish 

//Periods 
#define PT1 125
#define PT2 500
#define PT3 500
//#define PT4 10000 / (1 << (12 - TEMPERATURE_PRECISION))
#define PT4 10000
#define PT5 1000
#define PT6 1000
#define PT7 500
#define PT8 3000
#define PT9 30000
// Task10 period is initialized with PUBLISHINTERVAL and can be changed dynamically

//Start offsets to spread tasks along time
// Task1 has no delay
#define DT2  190/portTICK_PERIOD_MS
#define DT3  310/portTICK_PERIOD_MS
#define DT4  440/portTICK_PERIOD_MS
#define DT5  560/portTICK_PERIOD_MS
#define DT6  920/portTICK_PERIOD_MS
#define DT7  100/portTICK_PERIOD_MS
#define DT8  850/portTICK_PERIOD_MS
#define DT9  570/portTICK_PERIOD_MS
#define DT10 940/portTICK_PERIOD_MS
// =========================================================================================================
//                                     Software parameters
// =========================================================================================================

// Default date & time format
// ---------------------------
const char PM_HourMinFormat[6]     ="%Hh%M";
const char PM_HourFormat[4]        ="%Hh";
const char PM_LocalTimeFormat[18]  ="%Y-%m-%d %H:%M:%S";
const char PM_DateFormat[9]        ="%Y-%m-%d";
const char PM_UTCTimeFormat[19]    ="%Y-%m-%dT%H:%M:%SZ";

// Default time zone
// ---------------------------
// See Timezone definition here : https://sites.google.com/a/usapiens.com/opnode/time-zones
const char PM_TimeZone[60] = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"; // Paris, France

// TFT display parameters
// ----------------------
// duration of screen inactivity before backlight off (in seconds)
#define TFT_NO_TOUCH_TIMEOUT 60

// frequency of screen refresh (in seconds)
#define TFT_SCREEN_REFRESH 1

// Precision of temperature sensors
//----------------------------------
#define TEMPERATURE_PRECISION 10

// Is the web serial activated
// ----------------------------
#define IS_WEB_SERIAL_ACTIVATED true

#endif