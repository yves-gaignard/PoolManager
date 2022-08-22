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


// Setup an ADS1115 instance for analog measurements
// ---------------------------------------------------
// const uint8_t PM_ADS1115_Device_Addr    = 0x48;  // Address 0x48 is the default
#define PM_ADS1115_Device_Addr    0x48   // Address 0x48 is the default

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
const std::string insideThermometerName     = "Inside Thermometer";  // Number: 1
const std::string insideThermometerAddress  = "28CF0F95F0013CFE";    // Number: 1

const std::string outsideThermometerName    = "Outside Thermometer"; // Number: 2
const std::string outsideThermometerAddress = "28675195F0013C34";    // Number: 2

const std::string waterThermometerName      = "Water Thermometer";   // Number: 3
const std::string waterThermometerAddress   = "289F1E95F0013C61";    // Number: 3

// Configuration of the GPIOs
// ---------------------------
// Button to activate the LCD display
#define PM_DisplayButton_Pin  13

// One wire (temperature sensors) is plugged into GPIO 0
#define ONE_WIRE_BUS           0 

// Pump management 
#define FILTRATION_PUMP_Pin   26
#define PH_PUMP_Pin           14
#define CHL_PUMP_Pin          25

// Tank reset button
#define CHL_TANK_RESET_Pin     5
#define PH_TANK_RESET_Pin     17

// Light Buzzer
#define LIGHT_BUZZER_Pin      16

// Winter mode button
#define WINTER_MODE_Pin       33

// Sensors
#define PH_SENSOR_Pin         34
#define ORP_SENSOR_Pin        35
#define PRESSURE_SENSOR_Pin   36

// Loop tasks scheduling parameters
//---------------------------------
// T1 : PM_Task_AnalogPoll
// T2 : PM_Task_ProcessCommand
// T3 : PM_Task_Pool_Manager
// T4 : PM_Task_GetTemperature
// T5 : PM_Task_OrpRegulation
// T6 : PM_Task_pHRegulation
// T7 : PM_Task_LCD
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
const char PM_HourMinFormat[6]    ="%Hh%M";
const char PM_LocalTimeFormat[18] ="%Y-%m-%d %H:%M:%S";
const char PM_UTCTimeFormat[19]   ="%Y-%m-%dT%H:%M:%SZ";

// Default time zone
// ---------------------------
// See Timezone definition here : https://sites.google.com/a/usapiens.com/opnode/time-zones
const char PM_TimeZone[60] = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00"; // Paris, France

// LCD display parameters
// ----------------------
// duration of display inactivity before light off (in seconds)
#define LCD_DISPLAY_TIMEOUT 30

// duration of a screen display before switching to the next time (in seconds)
#define LCD_DISPLAY_SCREEN_DURATION 5

// Precision of 
//-----------------
#define TEMPERATURE_PRECISION 10

#endif