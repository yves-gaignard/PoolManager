/*
  Copyright 2022 - Yves Gaignard
  
  Non Volatile Storage management
*/
#define TAG "PM_NVS"

#include <Preferences.h>           // Library for preference storage management
#include <ESPPerfectTime.h>

#include "PM_Pool_Manager.h"
#include "PM_NVS.h"
#include "PM_Parameters.h"
#include "PM_Log.h"

// =================================================================================================
//                           MEASURES INITIALIZATION FROM STORAGE
// =================================================================================================
bool PM_NVS_Init() {
  bool rc = false;
  
  //Read ConfigVersion. If does not match expected value, restore default values
  if(nvs.begin(Project.Name.c_str(),true))
  {
    uint8_t vers = nvs.getUChar("PMVersion",0);
    LOG_I(TAG, "Stored version: %d",vers);

    nvs.end();

    if (vers == PM_VERSION)
    {
      LOG_I(TAG, "Same version: %d / %d. Loading settings from NVS",vers,PM_VERSION);
      if(PM_NVS_Load()) {
        rc = true; 
        LOG_I(TAG, "Data from NVS loaded"); //Restore stored values from NVS
      }
    }
    else
    {
      LOG_I(TAG, "New version: %d / %d. Store new default settings",vers,PM_VERSION);
      if(PM_NVS_Save()) {
        rc = true; 
        LOG_I(TAG, "Default settings stored in NVS");  //First time use. Save new default values to NVS
      }
    }

  } else {
    LOG_E(TAG, "NVS error");
    nvs.end();
    LOG_I(TAG, "Version: %d. First saving of settings",PM_VERSION);
    if(PM_NVS_Save()) {
      rc = true; 
      LOG_I(TAG, "Default settings stored in NVS");  //First time use. Save new default values to NVS
    }
  }  

  return rc; 
}
// =================================================================================================
//                           LOAD MEASURES FROM STORAGE
// =================================================================================================
bool PM_NVS_Load() {
  nvs.begin(Project.Name.c_str(),true);

  // Beware : the key maximum length is only 15 characters
  
  pm_measures.PMVersion                  = nvs.getUChar ("PMVersion"      ,0);
  pm_measures.RebootNumber               = nvs.getUChar ("RebootNumber"   ,0);
  pm_measures.LastRebootTimestamp        = nvs.getULong ("LastReboot"     ,0);
  pm_measures.LastDayResetTimestamp      = nvs.getULong ("LastDayReset"   ,0);
  pm_measures.AutoMode                   = nvs.getBool  ("AutoMode"       ,true);
  pm_measures.WinterMode                 = nvs.getBool  ("WinterMode"     ,false);
  pm_measures.InAirTemp                  = nvs.getFloat ("InAirTemp"      ,0.0);
  pm_measures.WaterTemp                  = nvs.getFloat ("WaterTemp"      ,0.0);
  pm_measures.OutAirTemp                 = nvs.getFloat ("OutAirTemp"     ,0.0);
  pm_measures.WaterTempLowThreshold      = nvs.getFloat ("WaterTempLowT"  ,0.0);
  pm_measures.pH_RegulationOnOff         = nvs.getBool  ("pHRegulationOn" ,false);
  pm_measures.pHValue                    = nvs.getDouble("pHValue"        ,0.0);
  pm_measures.pHPIDWindowSize            = nvs.getULong ("pHPIDWindowSize",0);
  pm_measures.pHPIDwindowStartTime       = nvs.getULong ("pHPIDStartTime" ,0);
  pm_measures.pHPumpUpTimeLimit          = nvs.getULong ("pHPumpUpTimeL"  ,0);
  pm_measures.pHPIDOutput                = nvs.getDouble("pHPIDOutput"    ,0.0);
  pm_measures.pH_SetPoint                = nvs.getDouble("pH_SetPoint"    ,0.0);
  pm_measures.pH_Kp                      = nvs.getDouble("pH_Kp"          ,0.0);
  pm_measures.pH_Ki                      = nvs.getDouble("pH_Ki"          ,0.0);
  pm_measures.pH_Kd                      = nvs.getDouble("pH_Kd"          ,0.0);
  pm_measures.pHCalibCoeffs0             = nvs.getDouble("pHCalibCoeffs0" ,0.0);
  pm_measures.pHCalibCoeffs1             = nvs.getDouble("pHCalibCoeffs1" ,0.0);
  pm_measures.Orp_RegulationOnOff        = nvs.getBool  ("OrpRegulationOn",false);
  pm_measures.OrpValue                   = nvs.getDouble("OrpValue"       ,0);
  pm_measures.OrpPIDWindowSize           = nvs.getULong ("OrpPIDWindowSiz",0);
  pm_measures.OrpPIDwindowStartTime      = nvs.getULong ("OrpPIDStartTime",0);
  pm_measures.OrpPumpUpTimeLimit         = nvs.getULong ("OrpPumpUpTimeL" ,0);
  pm_measures.OrpPIDOutput               = nvs.getDouble("OrpPIDOutput"   ,0.0);
  pm_measures.Orp_SetPoint               = nvs.getDouble("Orp_SetPoint"   ,0.0);
  pm_measures.Orp_Kp                     = nvs.getDouble("Orp_Kp"         ,0.0);
  pm_measures.Orp_Ki                     = nvs.getDouble("Orp_Ki"         ,0.0);
  pm_measures.Orp_Kd                     = nvs.getDouble("Orp_Kd"         ,0.0);
  pm_measures.OrpCalibCoeffs0            = nvs.getDouble("OrpCalibCoeffs0",0.0);
  pm_measures.OrpCalibCoeffs1            = nvs.getDouble("OrpCalibCoeffs1",0.0);
  pm_measures.DelayPIDs                  = nvs.getUChar ("DelayPIDs"      ,0);
  pm_measures.DayFiltrationUptime        = nvs.getULong ("DayFiltrUptime" ,0);
  pm_measures.DayFiltrationTarget        = nvs.getULong ("DayFiltrTarget" ,0);
  pm_measures.PeriodFiltrationStartTime  = nvs.getULong ("PFiltrStartTime",0);
  pm_measures.PeriodFiltrationEndTime    = nvs.getULong ("PFiltrEndTime"  ,0);
  pm_measures.PreviousDayFiltrationUptime= nvs.getULong ("PDayFiltrUptime",0);
  pm_measures.PreviousDayFiltrationTarget= nvs.getULong ("PDayFiltrTarget",0);
  pm_measures.pHPumpUptime               = nvs.getULong ("pHPumpUptime"   ,0);
  pm_measures.OrpPumpUptime              = nvs.getULong ("OrpPumpUptime"  ,0);
  pm_measures.pHMinusFlowRate            = nvs.getFloat ("pHMinusFlowRate",0.0);
  pm_measures.ChlorineFlowRate           = nvs.getFloat ("ChlorinFlowRate",0.0);
  pm_measures.pHMinusVolume              = nvs.getFloat ("pHMinusVolume"  ,0.0);
  pm_measures.ChlorineVolume             = nvs.getFloat ("ChlorineVolume" ,0.0);
  pm_measures.ConsumedInstantaneousPower = nvs.getUInt  ("InstantConsPwr" ,0);
  pm_measures.DayConsumedPower           = nvs.getUInt  ("DayConsumedPwr" ,0);
  pm_measures.Pressure                   = nvs.getFloat ("Pressure"       ,0.0);
  pm_measures.PressureHighThreshold      = nvs.getFloat ("PHighThreshold" ,0.0);
  pm_measures.PressureMedThreshold       = nvs.getFloat ("PMedThreshold"  ,0.0);
  pm_measures.PSICalibCoeffs0            = nvs.getFloat ("PSICalibCoeffs0",0.0);
  pm_measures.PSICalibCoeffs1            = nvs.getFloat ("PSICalibCoeffs1",0.0);
  pm_measures.FilterPumpState            = nvs.getBool  ("FilterPumpOn"   ,false);
  pm_measures.pHMinusPumpState           = nvs.getBool  ("pHMinusPumpOn"  ,false);
  pm_measures.ChlorinePumpState          = nvs.getBool  ("ChlorinePumpOn" ,false);
  pm_measures.pHMinusTankVolume          = nvs.getFloat ("pHMinusTankVol" ,0.0);
  pm_measures.ChlorineTankVolume         = nvs.getFloat ("ChlorineTankVol",0.0);
  pm_measures.pHMinusTankFill            = nvs.getFloat ("pHMinusTankFill",0.0);
  pm_measures.ChlorineTankFill           = nvs.getFloat ("ChlorinTankFill",0.0);

  nvs.end();

  LOG_D(TAG, "%d", pm_measures.PMVersion);
  LOG_D(TAG, "%d, %d, %d", pm_measures.RebootNumber, pm_measures.LastRebootTimestamp, pm_measures.LastDayResetTimestamp);
  LOG_D(TAG, "%d, %d, %d, %d, %d", pm_measures.AutoMode, pm_measures.WinterMode, pm_measures.pH_RegulationOnOff, pm_measures.Orp_RegulationOnOff, pm_measures.DelayPIDs);
  LOG_D(TAG, "%2.2f, %2.2f, %2.2f,%2.2f", pm_measures.InAirTemp, pm_measures.WaterTemp,pm_measures.OutAirTemp, pm_measures.WaterTempLowThreshold);
  LOG_D(TAG, "%2.2f, %4.0f", pm_measures.pHValue, pm_measures.OrpValue);
  LOG_D(TAG, "%d, %d", pm_measures.pHPIDWindowSize, pm_measures.OrpPIDWindowSize);
  LOG_D(TAG, "%d, %d", pm_measures.pHPIDwindowStartTime, pm_measures.OrpPIDwindowStartTime);
  LOG_D(TAG, "%d, %d", pm_measures.pHPumpUpTimeLimit, pm_measures.OrpPumpUpTimeLimit);
  LOG_D(TAG, "%4.0f, %4.0f, %8.0f, %3.0f, %3.0f", pm_measures.pHPIDOutput, pm_measures.pH_SetPoint, pm_measures.pH_Kp, pm_measures.pH_Ki, pm_measures.pH_Kd);
  LOG_D(TAG, "%4.0f, %4.0f, %8.0f, %3.0f, %3.0f", pm_measures.OrpPIDOutput, pm_measures.Orp_SetPoint, pm_measures.Orp_Kp, pm_measures.Orp_Ki, pm_measures.Orp_Kd);
  LOG_D(TAG, "%f, %f, %f, %f, %f, %f", pm_measures.pHCalibCoeffs0, pm_measures.pHCalibCoeffs1, pm_measures.OrpCalibCoeffs0, pm_measures.OrpCalibCoeffs1, pm_measures.PSICalibCoeffs0, pm_measures.PSICalibCoeffs1);
  LOG_D(TAG, "%d, %d", pm_measures.DayFiltrationUptime, pm_measures.DayFiltrationTarget);
  LOG_D(TAG, "%d, %d", pm_measures.PreviousDayFiltrationUptime, pm_measures.PreviousDayFiltrationTarget);
  LOG_D(TAG, "%d, %d", pm_measures.PeriodFiltrationStartTime, pm_measures.PeriodFiltrationEndTime);
  LOG_D(TAG, "%d, %d", pm_measures.pHPumpUptime, pm_measures.OrpPumpUptime);
  LOG_D(TAG, "%2.2f, %2.2f", pm_measures.pHMinusFlowRate, pm_measures.ChlorineFlowRate);
  LOG_D(TAG, "%2.2f, %2.2f", pm_measures.pHMinusVolume, pm_measures.ChlorineVolume);
  LOG_D(TAG, "%d, %d", pm_measures.ConsumedInstantaneousPower, pm_measures.DayConsumedPower);
  LOG_D(TAG, "%2.2f, %2.2f, %2.2f", pm_measures.Pressure, pm_measures.PressureHighThreshold, pm_measures.PressureMedThreshold);
  LOG_D(TAG, "%d, %d, %d", pm_measures.FilterPumpState, pm_measures.pHMinusPumpState, pm_measures.ChlorinePumpState);
  LOG_D(TAG, "%2.2f, %2.2f", pm_measures.pHMinusTankVolume, pm_measures.ChlorineTankVolume);
  LOG_D(TAG, "%2.2f, %2.2f", pm_measures.pHMinusTankFill, pm_measures.ChlorineTankFill);

  return (pm_measures.PMVersion == PM_VERSION);
}
// =================================================================================================
//                           SAVE MEASURES TO STORAGE
// =================================================================================================
bool PM_NVS_Save() {

  time_t      now;  // Current time (global variable)
  suseconds_t usec;
  tm*         time_tm;

  now = pftime::time(nullptr); // get current time
  time_tm = pftime::localtime(&now, &usec);  // Change in localtime
  nvs.begin(Project.Name.c_str(),false);

  // Beware : the key maximum length is only 15 characters
  size_t i = 
       nvs.putUChar ("PMVersion",       pm_measures.PMVersion);
  i += nvs.putUChar ("RebootNumber",    pm_measures.RebootNumber);
  i += nvs.putULong ("LastReboot",      pm_measures.LastRebootTimestamp);
  i += nvs.putULong ("LastDayReset",    pm_measures.LastDayResetTimestamp);
  i += nvs.putBool  ("AutoMode",        pm_measures.AutoMode);
  i += nvs.putBool  ("WinterMode",      pm_measures.WinterMode);
  i += nvs.putFloat ("InAirTemp",       pm_measures.InAirTemp);
  i += nvs.putFloat ("WaterTemp",       pm_measures.WaterTemp);
  i += nvs.putFloat ("OutAirTemp",      pm_measures.OutAirTemp);
  i += nvs.putFloat ("WaterTempLowT",   pm_measures.WaterTempLowThreshold);
  i += nvs.putBool  ("pHRegulationOn",  pm_measures.pH_RegulationOnOff);
  i += nvs.putDouble("pHValue",         pm_measures.pHValue);
  i += nvs.putULong ("pHPIDWindowSize", pm_measures.pHPIDWindowSize);
  i += nvs.putULong ("pHPIDStartTime",  pm_measures.pHPIDwindowStartTime);
  i += nvs.putULong ("pHPumpUpTimeL",   pm_measures.pHPumpUpTimeLimit);
  i += nvs.putDouble("pHPIDOutput",     pm_measures.pHPIDOutput);
  i += nvs.putDouble("pH_SetPoint",     pm_measures.pH_SetPoint);
  i += nvs.putDouble("pH_Kp",           pm_measures.pH_Kp);
  i += nvs.putDouble("pH_Ki",           pm_measures.pH_Ki);
  i += nvs.putDouble("pH_Kd",           pm_measures.pH_Kd);
  i += nvs.putDouble("pHCalibCoeffs0",  pm_measures.pHCalibCoeffs0);
  i += nvs.putDouble("pHCalibCoeffs1",  pm_measures.pHCalibCoeffs1);
  i += nvs.putBool  ("OrpRegulationOn", pm_measures.Orp_RegulationOnOff);
  i += nvs.putDouble("OrpValue",        pm_measures.OrpValue);
  i += nvs.putULong ("OrpPIDWindowSiz", pm_measures.OrpPIDWindowSize);
  i += nvs.putULong ("OrpPIDStartTime", pm_measures.OrpPIDwindowStartTime);
  i += nvs.putULong ("OrpPumpUpTimeL",  pm_measures.OrpPumpUpTimeLimit);
  i += nvs.putDouble("OrpPIDOutput",    pm_measures.OrpPIDOutput);
  i += nvs.putDouble("Orp_SetPoint",    pm_measures.Orp_SetPoint);
  i += nvs.putDouble("Orp_Kp",          pm_measures.Orp_Kp);
  i += nvs.putDouble("Orp_Ki",          pm_measures.Orp_Ki);
  i += nvs.putDouble("Orp_Kd",          pm_measures.Orp_Kd);
  i += nvs.putDouble("OrpCalibCoeffs0", pm_measures.OrpCalibCoeffs0);
  i += nvs.putDouble("OrpCalibCoeffs1", pm_measures.OrpCalibCoeffs1);
  i += nvs.putUChar ("DelayPIDs",       pm_measures.DelayPIDs);
  i += nvs.putULong ("DayFiltrUptime",  pm_measures.DayFiltrationUptime);
  i += nvs.putULong ("DayFiltrTarget",  pm_measures.DayFiltrationTarget);
  i += nvs.putULong ("PFiltrStartTime", pm_measures.PeriodFiltrationStartTime);
  i += nvs.putULong ("PFiltrEndTime",   pm_measures.PeriodFiltrationEndTime);
  i += nvs.putULong ("PDayFiltrUptime", pm_measures.PreviousDayFiltrationUptime);
  i += nvs.putULong ("PDayFiltrTarget", pm_measures.PreviousDayFiltrationTarget);
  i += nvs.putULong ("pHPumpUptime"   , pm_measures.pHPumpUptime);
  i += nvs.putULong ("OrpPumpUptime"  , pm_measures.OrpPumpUptime);
  i += nvs.putFloat ("pHMinusFlowRate", pm_measures.pHMinusFlowRate);
  i += nvs.putFloat ("ChlorinFlowRate", pm_measures.ChlorineFlowRate);
  i += nvs.putFloat ("pHMinusVolume",   pm_measures.pHMinusVolume);
  i += nvs.putFloat ("ChlorineVolume",  pm_measures.ChlorineVolume);
  i += nvs.putUInt  ("InstantConsPwr",  pm_measures.ConsumedInstantaneousPower);
  i += nvs.putUInt  ("DayConsumedPwr",  pm_measures.DayConsumedPower);
  i += nvs.putFloat ("Pressure",        pm_measures.Pressure);
  i += nvs.putFloat ("PHighThreshold",  pm_measures.PressureHighThreshold);
  i += nvs.putFloat ("PMedThreshold",   pm_measures.PressureMedThreshold);
  i += nvs.putFloat ("PSICalibCoeffs0", pm_measures.PSICalibCoeffs0);
  i += nvs.putFloat ("PSICalibCoeffs1", pm_measures.PSICalibCoeffs1);
  i += nvs.putBool  ("FilterPumpOn",    pm_measures.FilterPumpState);
  i += nvs.putBool  ("pHMinusPumpOn",   pm_measures.pHMinusPumpState);
  i += nvs.putBool  ("ChlorinePumpOn",  pm_measures.ChlorinePumpState);
  i += nvs.putFloat ("pHMinusTankVol",  pm_measures.pHMinusTankVolume);
  i += nvs.putFloat ("ChlorineTankVol", pm_measures.ChlorineTankVolume);
  i += nvs.putFloat ("pHMinusTankFill", pm_measures.pHMinusTankFill);
  i += nvs.putFloat ("ChlorinTankFill", pm_measures.ChlorineTankFill);

  nvs.end();

  LOG_D(TAG, "Bytes stored in NVS: %d / %d", i, sizeof(pm_measures));

  return (i == sizeof(pm_measures));
}

// =================================================================================================
// functions to save any type of parameter (4 overloads with same name but different arguments)
// =================================================================================================
bool PM_NVS_saveParam(const char* key, uint8_t val)
{
  nvs.begin(Project.Name.c_str(),false);
  size_t i = nvs.putUChar(key,val);
  return(i == sizeof(val));
}

bool PM_NVS_saveParam(const char* key, bool val)
{
  nvs.begin(Project.Name.c_str(),false);
  size_t i = nvs.putBool(key,val);
  return(i == sizeof(val));
}

bool PM_NVS_saveParam(const char* key, unsigned long val)
{
  nvs.begin(Project.Name.c_str(),false);
  size_t i = nvs.putULong(key,val);
  return(i == sizeof(val));
}

bool PM_NVS_saveParam(const char* key, double val)
{
  nvs.begin(Project.Name.c_str(),false);
  size_t i = nvs.putDouble(key,val);
  return(i == sizeof(val));
}

bool PM_NVS_saveParam(const char* key, float val)
{
  nvs.begin(Project.Name.c_str(),false);
  size_t i = nvs.putFloat(key,val);
  return(i == sizeof(val));
}
