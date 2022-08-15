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

#define PM_VERSION 1
struct PM_SwimmingPoolMeasures {
  time_t  Timestamp;                      // Last modification timestamp
  uint8_t PMVersion;                      // version of the structure
  bool    AutoMode;                       // Mode Automatic (true) or Manual (false)
  bool    WinterMode;                     // Winter Mode if true
  bool    pH_RegulationOnOff;             
  bool    Orp_RegulationOnOff;             
  bool    FilterPumpState;                // State of the filtering pump (true, false)
  bool    pHMinusPumpState;               // State of the pH- pump (true, false)
  bool    ChlorinePumpState;              // State of the pH- pump (true, false)
  float   InAirTemp;                      // Inside air temperature in °C
  float   WaterTemp;                      // Water temperature in °C of the swimming pool
  float   OutAirTemp;                     // Outside air temperature in °C
  double  pHValue;                        // Current pH value
  ulong   pHPIDWindowSize;
  ulong   pHPIDwindowStartTime;           // pH PID window start time
  ulong   pHPumpUpTimeLimit;              // Time in seconds max per day for pH injection
  double  pHPIDOutput;
  double  pH_SetPoint;
  double  pH_Kp;
  double  pH_Ki;
  double  pH_Kd;
  double  OrpValue;                       // Current redox measure unit: mV
  ulong   OrpPIDWindowSize;
  ulong   OrpPIDwindowStartTime;          // Orp PID window start time   
  ulong   OrpPumpUpTimeLimit;             // Time in seconds max per day for Chlorine injection
  double  OrpPIDOutput;
  double  Orp_SetPoint;
  double  Orp_Kp;
  double  Orp_Ki;
  double  Orp_Kd;
  time_t  FilteredDuration;               // Filtration Duration since the begin of the day
  time_t  DayFiltrationDuration;          // Maximum Filtration duration for the whole day
  time_t  FiltrationStartTime;            // Next start time of the filtration
  time_t  FiltrationEndTime;              // Next end time of the filtration
  float   pHMinusFlowRate;                // Flow rate of pH Minus liquid injected (liter per hour)
  float   ChlorineFlowRate;               // Flow rate of Chlorine liquid injected (liter per hour)
  float   pHMinusVolume;                  // Volume of pH Minus liquid since the last complete fill of the container
  float   ChlorineVolume;                 // Volume of liquid chlorine since the last complete fill of the container
  int32_t ConsumedInstantaneousPower;     // Instantaneous Power in Watt consumed by the filtration pump
  int32_t DayConsumedPower;               // Power in Watt consumed by the filtration pump since the begin of the day
  float   Pressure;                       // Pressure in the filtering device (unit hPa)
  float   pHMinusTankVolume;              // Max volume of the pH- tank
  float   ChlorineTankVolume;             // Max volume of the Chlorine tank
  float   pHMinusTankFill;                // % Fill of volume of the pH- tank
  float   ChlorineTankFill;               // % Fill of volume of the Chlorine tank
};

struct PM_SwimmingPoolMeasures_str {
  std::string  Timestamp_str;                  // Last modification timestamp
  std::string  InAirTemp_str;                  // Inside air temperature in °C (string format: 28.2 °)
  std::string  WaterTemp_str;                  // Water temperature in °C of the swimming pool (string format 25.2 °)
  std::string  OutAirTemp_str;                 // Outside air temperature in °C (string format 15.4 °)
  std::string  pH_str;                         // pH unit (string format 7.2)
  std::string  pHPIDwindowStartTime_str;       // pH PID window start time   
  std::string  Chlorine_str;                   // redox measure unit: mV (string format 3000 mV)
  std::string  ChlorinePIDwindowStartTime_str; // Chlorine PID window start time   
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