/*
  Yves Gaignard
  
  Pool Configuration management
*/

#define TAG "PM_Pool_Config"

// Standard library definitions
#include <Arduino.h>

#include "PM_Pool_Manager.h"
#include "PM_Config.h"                        // Pool Manager configuration management
#include "PM_Error.h"                         // Pool Manager error management
#include "PM_Utils.h"                         // Pool Manager utility tools

// Constructor
PM_Config::PM_Config() {
}

// Verify that the abaqus describing the filtration time is valid
PM_Error PM_Config::CheckFiltrationTimeAbaqus() {
  // Rules to check:
  // R1 : the lowest temperature must be -20 and the highest 99
  // R2 : temperature of the temperature range must be consecutives (aka TempMin of a line must be equal to TempMax of previous line)
  // R3 : the filtration time of consecutive lines must increasing 

  int NumberLine = PM_FiltrationDuration_Abaqus.size();
  LOG_I(TAG, "Number of line: %d", NumberLine);
  if (NumberLine == 0 ) { 
    int ErrorNumber=101;
    std::string ErrorMsg = "The PM_FiltrationDuration_Abaqus is empty";
    LOG_E(TAG, "Error: %d - %s", ErrorNumber, ErrorMsg.c_str());
    return PM_Error(ErrorNumber, ERROR, ErrorMsg.c_str()); 
  }
  if (PM_FiltrationDuration_Abaqus[0].TempMin != -20) { 
    int ErrorNumber=102;
    std::string ErrorMsg = "The minimum temperature of the PM_FiltrationDuration_Abaqus is not -20";
    LOG_E(TAG, "Error: %d - %s", ErrorNumber, ErrorMsg.c_str());
    return PM_Error(ErrorNumber, ERROR, ErrorMsg.c_str()); 
  }
  if (PM_FiltrationDuration_Abaqus[NumberLine-1].TempMax != 99) {
    int ErrorNumber=103;
    std::string ErrorMsg = "The maximum temperature of the PM_FiltrationDuration_Abaqus is not 99";
    LOG_E(TAG, "Error: %d - %s", ErrorNumber, ErrorMsg.c_str());
    return PM_Error(ErrorNumber, ERROR, ErrorMsg.c_str()); 
  }
  if (NumberLine >= 1) {
    for (int i = 0; i <= NumberLine-2; i++ ) {
      if (PM_FiltrationDuration_Abaqus[i+1].TempMin != PM_FiltrationDuration_Abaqus[i].TempMax) {
        std::string ErrorMsg = "The max [" + PM_itoa(i+1) +"] and the min [" + PM_itoa(i) + "] temperatures of the PM_FiltrationDuration_Abaqus are not the same";
        LOG_E(TAG, "Error: %d - %s", 104, ErrorMsg.c_str());
        return PM_Error(104, ERROR, ErrorMsg.c_str()); 
      }
      if (PM_FiltrationDuration_Abaqus[i+1].Duration < PM_FiltrationDuration_Abaqus[i].Duration) { 
        std::string ErrorMsg = "The duration [" + PM_itoa(i+1) +"] is not greater or eaqul to the duration [" + PM_itoa(i) + "]";
        LOG_E(TAG, "Error: %d - %s", 105, ErrorMsg.c_str());
        return PM_Error(105, ERROR, ErrorMsg.c_str()); 
      }
    }
  }

  return PM_Error(0,INFO, std::string("The filtration duration abaqus is validated"));
}


// Verify that the abaqus describing the filtration period is valid
PM_Error PM_Config::CheckFiltrationPeriodAbaqus() {
  // Rules to check:
  // R1 : The array does not need to be sorted
  // R2 : a period cannot recover another one
  // R3 : all periods have to cover the whole 24h of the day (aka from 0h to 24h)
  
  // Sort the table on Start field
  sort(PM_FiltrationPeriod_Abaqus.begin(), PM_FiltrationPeriod_Abaqus.end(), PM_FiltrationPeriod_Start_Cmp);
  
  int NumberLine = PM_FiltrationPeriod_Abaqus.size();
  LOG_I(TAG, "Number of line: %d", NumberLine);

  if (NumberLine == 0 ) { return PM_Error(111, ERROR, std::string("The PM_FiltrationPeriod_Abaqus is empty")); }
  if (PM_FiltrationPeriod_Abaqus[0].Start != 0) { return PM_Error(112, ERROR, std::string("The minimum start of the PM_FiltrationPeriod_Abaqus is not 0")); }
  if (PM_FiltrationPeriod_Abaqus[NumberLine-1].End != 24) { return PM_Error(113, ERROR, std::string("The maximum end of the PM_FiltrationPeriod_Abaqus is not 24")); }
  if (NumberLine >= 1) {
    for (int i = 0; i <= NumberLine-2; i++ ) {
      if (PM_FiltrationPeriod_Abaqus[i+1].Start != PM_FiltrationPeriod_Abaqus[i].End) { 
        std::string ErrorMsg = "The end [" + PM_itoa(i+1) +"] and the start [" + PM_itoa(i) + "] periods of the PM_FiltrationPeriod_Abaqus are not the same";
        LOG_E(TAG, "Error: %d - %s", 114, ErrorMsg.c_str());
        return PM_Error(114, ERROR, ErrorMsg.c_str()); 
      }
    }
  }
  return PM_Error(0,INFO, std::string("The filtration period abaqus is validated"));
}

//Comparator of PM_FiltrationPeriod on Priority of the period
static bool PM_FiltrationPeriod_Priority_Cmp(const PM_FiltrationPeriod& lPeriod, const PM_FiltrationPeriod& rPeriod) {
   return lPeriod.Priority < rPeriod.Priority;
}

//Comparator of PM_FiltrationPeriod on Start of the period
static bool PM_FiltrationPeriod_Start_Cmp(const PM_FiltrationPeriod& lPeriod, const PM_FiltrationPeriod& rPeriod){
   return lPeriod.Start < rPeriod.Start;
}

//Comparator of PM_FiltrationPeriod on End of the period
static bool PM_FiltrationPeriod_End_Cmp(const PM_FiltrationPeriod& lPeriod, const PM_FiltrationPeriod& rPeriod){
   return lPeriod.End < rPeriod.End;
}


