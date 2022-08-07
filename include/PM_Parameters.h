/*
  Copyright 2022 - Yves Gaignard
  
  Parameters of the Pool Manager project
*/
#ifndef PM_Parameters_h
#define PM_Parameters_h

#include <Arduino.h>
#include <PM_Structures.h>
#include <PM_Secrets.h>

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
#define PM_DisplayButton_Pin 18

// One wire (temperature sensors) is plugged into GPIO 4
#define ONE_WIRE_BUS 4 
#define TEMPERATURE_PRECISION 9

// =========================================================================================================
//                                     Pool Manager Configuration
// =========================================================================================================

// Default date & time format
// ---------------------------
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

// 

#endif