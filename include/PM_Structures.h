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
  uint8_t PMVersion;                      // version of the structure
  bool    AutoMode;                       // Mode Automatic (true) or Manual (false)
  bool    WinterMode;                     // Winter Mode if true
  bool    pH_RegulationOnOff;             
  bool    Orp_RegulationOnOff;             
  bool    FilterPumpState;                // State of the filtering pump (true, false)
  bool    pHMinusPumpState;               // State of the pH- pump (true, false)
  bool    ChlorinePumpState;              // State of the Chlorine pump (true, false)
  uint8_t DelayPIDs;                      // Delay of starting PID computation after the start of a filtration pump (in minutes)
  float   InAirTemp;                      // Inside air temperature in °C
  float   WaterTemp;                      // Water temperature in °C of the swimming pool
  float   OutAirTemp;                     // Outside air temperature in °C
  float   WaterTempLowThreshold;          // Water temperature low threshold to compute PIDs
  double  pHValue;                        // Current pH value
  ulong   pHPIDWindowSize;
  ulong   pHPIDwindowStartTime;           // pH PID window start time
  ulong   pHPumpUpTimeLimit;              // Time in seconds max per day for pH injection
  double  pHPIDOutput;
  double  pH_SetPoint;
  double  pH_Kp;
  double  pH_Ki;
  double  pH_Kd;
  double  pHCalibCoeffs0;
  double  pHCalibCoeffs1;
  double  OrpValue;                       // Current redox measure unit: mV
  ulong   OrpPIDWindowSize;
  ulong   OrpPIDwindowStartTime;          // Orp PID window start time   
  ulong   OrpPumpUpTimeLimit;             // Time in seconds max per day for Chlorine injection
  double  OrpPIDOutput;
  double  Orp_SetPoint;
  double  Orp_Kp;
  double  Orp_Ki;
  double  Orp_Kd;
  double  OrpCalibCoeffs0;
  double  OrpCalibCoeffs1;
  time_t  DayFiltrationUptime;            // Filtration Duration since the begin of the day
  time_t  DayFiltrationTarget;            // Target Filtration duration for the whole day
  time_t  PeriodFiltrationUptime;         // Filtration Duration since the begin of the period
  time_t  PeriodFiltrationStartTime;      // Next period start time of the filtration
  time_t  PeriodFiltrationEndTime;        // Next period end time of the filtration
  time_t  PreviousDayFiltrationUptime;    // Filtration Duration of the previous day
  time_t  PreviousDayFiltrationTarget;    // Target Filtration duration of the previous day
  float   pHMinusFlowRate;                // Flow rate of pH Minus liquid injected (liter per hour)
  float   ChlorineFlowRate;               // Flow rate of Chlorine liquid injected (liter per hour)
  float   pHMinusVolume;                  // Volume of pH Minus liquid since the last complete fill of the container
  float   ChlorineVolume;                 // Volume of liquid chlorine since the last complete fill of the container
  int32_t ConsumedInstantaneousPower;     // Instantaneous Power in Watt consumed by the filtration pump
  int32_t DayConsumedPower;               // Power in Watt consumed by the filtration pump since the begin of the day
  float   Pressure;                       // Pressure in the filtering device (unit hPa)
  float   PressureHighThreshold;          // Pressure to consider the filtration pump is started
  float   PressureMedThreshold;           // Pressure to consider the filtration pump is stopped
  double  PSICalibCoeffs0;
  double  PSICalibCoeffs1;
  float   pHMinusTankVolume;              // Max volume of the pH- tank
  float   ChlorineTankVolume;             // Max volume of the Chlorine tank
  float   pHMinusTankFill;                // % Fill of volume of the pH- tank
  float   ChlorineTankFill;               // % Fill of volume of the Chlorine tank
};

#endif