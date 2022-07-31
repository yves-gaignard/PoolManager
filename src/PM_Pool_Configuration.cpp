/*
  Yves Gaignard
  
  Pool Configuration management
*/

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// Standard library definitions
#include <Arduino.h>

#include "PM_Pool_Configuration.h"            // Pool Manager configuration management
#include "PM_Error.h"                         // Pool Manager error management

static const char* TAG = "PM_Pool_Configuration";


// Constructor
PM_Pool_Configuration::PM_Pool_Configuration() {
}

// Verify that the abaqus describing the filtration time is valid
PM_Error PM_Pool_Configuration::CheckFiltrationTimeAbaqus() {
  // Rules to check:
  // R1 : the lowest temperature must be -20 and the highest 99
  // R2 : temperature of the temperature range must be consecutives (aka TempMin of a line must be equal to TempMax of previous line)
  // R3 : the filtration time of consecutive lines must increasing 

  int NumberLine = PM_FiltrationTime_Abaqus.size();
  if (NumberLine == 0 ) { return PM_Error(101, ERROR, std::string("The PM_FiltrationTime_Abaqus is empty")); }
  if (PM_FiltrationTime_Abaqus[0].TempMin != -20) { PM_Error Error(102, ERROR, std::string("The minimum temperature of the PM_FiltrationTime_Abaqus is not -20")); return Error;}
  if (PM_FiltrationTime_Abaqus[NumberLine-1].TempMax != 99) { return PM_Error(103, ERROR, std::string("The maximum temperature of the PM_FiltrationTime_Abaqus is not 99")); }
  if (NumberLine >= 1) {
    for (int i = 0; i <= NumberLine; i++ ) {
      if (PM_FiltrationTime_Abaqus[i+1].TempMin != PM_FiltrationTime_Abaqus[i].TempMax) { return PM_Error(104, ERROR, std::string("The max and the min temperatures of two consecutive lines of the PM_FiltrationTime_Abaqus are not the same")); }
      if (PM_FiltrationTime_Abaqus[i+1].Duration < PM_FiltrationTime_Abaqus[i].Duration) { return PM_Error(105, ERROR, std::string("Two consecutive duration of filtration time are not increasing")); }
    }
  }

  return PM_Error(0,INFO, std::string("The filtration time abaqus is validated"));
}


// Verify that the abaqus describing the filtration period is valid
PM_Error PM_Pool_Configuration::CheckFiltrationPeriodAbaqus() {
  // Rules to check:
  // R1 : The array does not need to be sorted
  // R2 : a period cannot recover another one
  // R3 : all periods have to cover the whole 24h of the day (aka from 0h to 24h)
  
  // Sort the table on Start field
  sort(PM_FiltrationPeriod_Abaqus.begin(), PM_FiltrationPeriod_Abaqus.end(), PM_FiltrationPeriod_Start_Cmp);
  
  int NumberLine = PM_FiltrationPeriod_Abaqus.size();

  if (NumberLine == 0 ) { return PM_Error(111, ERROR, std::string("The PM_FiltrationPeriod_Abaqus is empty")); }
  if (PM_FiltrationPeriod_Abaqus[0].Start != 0) { return PM_Error(112, ERROR, std::string("The minimum start of the PM_FiltrationPeriod_Abaqus is not 0")); }
  if (PM_FiltrationPeriod_Abaqus[NumberLine-1].End != 24) { return PM_Error(113, ERROR, std::string("The maximum end of the PM_FiltrationPeriod_Abaqus is not 24")); }
  if (NumberLine >= 1) {
    for (int i = 0; i <= NumberLine; i++ ) {
      if (PM_FiltrationPeriod_Abaqus[i+1].Start != PM_FiltrationPeriod_Abaqus[i].End) { return PM_Error(114, ERROR, std::string("The end and the start periods of two consecutive lines of the PM_FiltrationPeriod_Abaqus are not the same")); }
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


