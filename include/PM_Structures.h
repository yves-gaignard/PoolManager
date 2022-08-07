/*
  Copyright 2022 - Yves Gaignard
  
  Structures definitions of the project
*/
#ifndef PM_Structures_h
#define PM_Structures_h

#include <Arduino.h>

struct PM_ProjectStruct {
  std::string Name;
  std::string Version;
  std::string Author;
};


struct PM_WifiNetwork {
  std::string ssid;
  std::string password;
};
struct PM_WifiConfig {
  IPAddress local_IP;
  IPAddress gateway;
  IPAddress subnet;
  IPAddress primaryDNS;   //optional
  IPAddress secondaryDNS; //optional
};

struct PM_FiltrationDuration {
  int  TempMin;
  int  TempMax;
  int  Duration;
};

struct PM_FiltrationPeriod {
  int Start;     // Time (in hour) of the period start
  int End;       // Time (in hour) of the period end 
  int Priority;  // priority must be greater than 1
};

struct PM_SwimmingPoolMeasures {
  float   InAirTemp;                      // Inside air temperature in °C
  float   WaterTemp;                      // Water temperature in °C of the swimming pool
  float   OutAirTemp;                     // Outside air temperature in °C
  float   pH;                             // pH unit
  int32_t Chlorine;                       // redox measure unit: mV
  int32_t ChlorineMin;                    // Minimum acceptable redox measure unit: mV
  int32_t ChlorineMax;                    // Maximuù acceptbble redox measure unit: mV
  time_t  DayFilterTime;                  // Filter Time since the begin of the day
  time_t  DayFilterMaxTime;               // Maximum Filter Time since the begin of the day
  float   pHMinusVolume;                  // Volume of pH Minus liquid since the last complete fill of the container
  float   ChlorineVolume;                 // Volume of liquid chlorine since the last complete fill of the container
  int32_t ConsumedInstantaneousPower;     // Instantaneous Power in Watt consumed by the filtration pump
  int32_t DayConsumedPower;               // Power in Watt consumed by the filtration pump since the begin of the day
  float   Pressure;                       // Pressure in the filtering device (unit hPa)
  boolean FilterPumpState;                // State of the filtering pump (true, false)
  boolean pHMinusPumpState;               // State of the pH- pump (true, false)
  boolean ChlorinePumpState;              // State of the pH- pump (true, false)
  float   pHMinusMaxVolume;               // Volume max of the pH- container
  float   ChlorineMaxVolume;              // Volume max of the Chlorine container
};

struct PM_SwimmingPoolMeasures_str {
  std::string  InAirTemp_str;                  // Inside air temperature in °C (string format: 28.2 °)
  std::string  WaterTemp_str;                  // Water temperature in °C of the swimming pool (string format 25.2 °)
  std::string  OutAirTemp_str;                 // Outside air temperature in °C (string format 15.4 °)
  std::string  pH_str;                         // pH unit (string format 7.2)
  std::string  Chlorine_str;                   // redox measure unit: mV (string format 3000 mV)
  std::string  ChlorineMin_str;                // Minimum acceptable redox measure unit: mV (string format 3000 mV)
  std::string  ChlorineMax_str;                // Maximum acceptable redox measure unit: mV (string format 3000 mV)
  std::string  DayFilterTime_str;              // Filter Time since the begin of the day (string format 15h26)
  std::string  MaxDayFilterTime_str;           // Maximum Filter Time since the begin of the day (string format 16h)
  std::string  pHMinusVolume_str;              // Volume of pH Minus liquid since the last complete fill of the container (string format 20.4 l)
  std::string  ChlorineVolume_str;             // Volume of liquid chlorine since the last complete fill of the container (string format 12.9 l)
  std::string  ConsumedInstantaneousPower_str; // Instantaneous Power in Watt consumed by the filtration pump (string format 2514 W)
  std::string  DayConsumedPower_str;           // Power in Watt consumed by the filtration pump since the begin of the day (string format 25 kWh)
  std::string  Pressure_str;                   // Pressure in the filtering device (unit hPa) (string format)
  std::string  FilterPumpState_str;            // State of the filtering pump (ON  , OFF)
  std::string  pHMinusPumpState_str;           // State of the pH- pump (ON, OFF)
  std::string  ChlorinePumpState;              // State of the pH- pump (ON, OFF)
  std::string  pHMinusMaxVolume_str;           // Volume max of the pH- container (string format 20.0 l)
  std::string  ChlorineMaxVolume_str;          // Volume max of the Chlorine container (string format 20.0 l)
};


#endif