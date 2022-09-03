/*
  Yves Gaignard
  
  Pool Configuration management
*/

#define TAG "PM_Config"

// Standard library definitions
#include <Arduino.h>
#include <ESPPerfectTime.h>
#include <Date.h>

#include "PM_Pool_Manager.h"
#include "PM_Config.h"                        // Pool Manager configuration management
#include "PM_Error.h"                         // Pool Manager error management
#include "PM_Utils.h"                         // Pool Manager utility tools
#include "PM_Time_Mngt.h"                     // Pool Manager time management

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

// Get filtration duration in seconds depending on the temperature
ulong PM_Config::GetFiltrationDuration (float waterTemperature) {
  int NumberLine = PM_FiltrationDuration_Abaqus.size();
  ulong DurationInSeconds = 2 * 3600; // in any case, the duration must be greater than 0
  tm tm_duration;   
  
  for (int i = 0; i <= NumberLine-1; i++ ) {
    if ((float)PM_FiltrationDuration_Abaqus[i].TempMin <= waterTemperature && waterTemperature < (float)PM_FiltrationDuration_Abaqus[i].TempMax) {
      DurationInSeconds = PM_FiltrationDuration_Abaqus[i].Duration * 3600;
      LOG_D(TAG, "Found a corresponding filtration duration in table for water temperature: %6.2f", waterTemperature);
      LOG_D(TAG, "- Min temperature: %6.2f", (float)PM_FiltrationDuration_Abaqus[i].TempMin);
      LOG_D(TAG, "- Max temperature: %6.2f", (float)PM_FiltrationDuration_Abaqus[i].TempMax);
      tm_duration = PM_Time_Mngt_convertSecondsToTm(DurationInSeconds);
      LOG_D(TAG, "- Duration       : %02d:%02d:%02d (%us)", tm_duration.tm_hour, tm_duration.tm_min, tm_duration.tm_sec, DurationInSeconds);
    }
  }
  tm_duration = PM_Time_Mngt_convertSecondsToTm(DurationInSeconds);
  LOG_I(TAG, "Get filtration duration for water temperature: %6.2f : %02d:%02d:%02d (%us)", waterTemperature, tm_duration.tm_hour, tm_duration.tm_min, tm_duration.tm_sec, DurationInSeconds);

  return DurationInSeconds;
}

// Get next period of filtration
void PM_Config::GetNextFiltrationPeriod (time_t &NextStartTime, time_t &NextEndTime, const ulong FiltrationDoneInSeconds, const ulong FiltrationDurationInSeconds) {
  int NumberLine = PM_FiltrationPeriod_Abaqus.size();
  time_t AvailableDuration = 0;
  time_t diffTime = 0;
  tm     tm_NextStartTime;
  tm     tm_NextEndTime;
  tm     tm_duration;
  tm     tm_diffTime;
  time_t now;
  tm *   tm_now;

  now = pftime::time(nullptr); // get current time
  tm_now = pftime::localtime(&now);
  LOG_D(TAG, "Current time: %04d/%02d/%02d %02d:%02d:%02d:", tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday,tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

  LOG_D(TAG, "Calculate the next filtration period for:");
  diffTime = (time_t)FiltrationDurationInSeconds;
  tm_duration = PM_Time_Mngt_convertSecondsToTm(FiltrationDurationInSeconds);
  LOG_D(TAG, "- total day duration   : %02d:%02d:%02d  in seconds: %u", tm_duration.tm_hour, tm_duration.tm_min, tm_duration.tm_sec, FiltrationDurationInSeconds);
  tm_duration = PM_Time_Mngt_convertSecondsToTm(FiltrationDoneInSeconds);
  LOG_D(TAG, "- duration already done: %02d:%02d:%02d  in seconds: %u", tm_duration.tm_hour, tm_duration.tm_min, tm_duration.tm_sec, FiltrationDoneInSeconds);

  // Calculate the time duration until the end of the day
  sort(PM_FiltrationPeriod_Abaqus.begin(), PM_FiltrationPeriod_Abaqus.end(), PM_FiltrationPeriod_Priority_Cmp);
  
  for (int i = 0; i <= NumberLine-1; i++ ) {
    LOG_D(TAG, "In the period, Start Time is: %02d , End Time is: %02d", PM_FiltrationPeriod_Abaqus[i].Start, PM_FiltrationPeriod_Abaqus[i].End);
    
    if (PM_FiltrationPeriod_Abaqus[i].End <= tm_now->tm_hour) {
      // too late for this period
      LOG_D(TAG, "Too late for this period");
    
    } else if (PM_FiltrationPeriod_Abaqus[i].Start <= tm_now->tm_hour && tm_now->tm_hour < PM_FiltrationPeriod_Abaqus[i].End) {
      // we are just in this period, count only the time from now to the end of the period
      LOG_D(TAG, "we are just in this period, count only the time from now to the end of the period");
      if ( PM_FiltrationPeriod_Abaqus[i].End >= 24 ) {  // need to calcule the next day date
        Date Today;              // == today
        Date Tomorrow = Today+1; // == Tomorrow
        tm_NextEndTime.tm_year = Tomorrow.getYear() -1900;
        tm_NextEndTime.tm_mon  = Tomorrow.getMonth() -1;
        tm_NextEndTime.tm_mday = Tomorrow.getDay();
        tm_NextEndTime.tm_hour = 0;
        tm_NextEndTime.tm_min  = 0;
        tm_NextEndTime.tm_sec  = 0;
        NextEndTime = mktime(&tm_NextEndTime);
      } 
      else {
        localtime_r(&now, &tm_NextEndTime);   
        tm_NextEndTime.tm_hour = PM_FiltrationPeriod_Abaqus[i].End;
        tm_NextEndTime.tm_min  = 0;
        tm_NextEndTime.tm_sec  = 0;
        NextEndTime = mktime(&tm_NextEndTime);
      }
      LOG_D(TAG, "In the period, NextEndTime is: %02d:%02d:%02d", tm_NextEndTime.tm_hour, tm_NextEndTime.tm_min, tm_NextEndTime.tm_sec);
      diffTime=NextEndTime - now;
      tm_duration = PM_Time_Mngt_convertSecondsToTm(diffTime);
      LOG_D(TAG, "Available duration in the period: %02d:%02d:%02d in s: %d", tm_duration.tm_hour, tm_duration.tm_min, tm_duration.tm_sec, diffTime);

      AvailableDuration += diffTime;
      tm_duration = PM_Time_Mngt_convertSecondsToTm(AvailableDuration);
      LOG_D(TAG, "Total available duration : %02d:%02d:%02d in s: %d", tm_duration.tm_hour, tm_duration.tm_min, tm_duration.tm_sec, AvailableDuration);
    } 
    else {
      // this period is on the next hours
      LOG_D(TAG, "This period is on the next hours");
      diffTime = (PM_FiltrationPeriod_Abaqus[i].End - PM_FiltrationPeriod_Abaqus[i].Start)*3600;
      tm_duration = PM_Time_Mngt_convertSecondsToTm(diffTime);
      LOG_D(TAG, "In a next period: Available duration : %02d:%02d:%02d in s: %d", tm_duration.tm_hour, tm_duration.tm_min, tm_duration.tm_sec, diffTime);
      AvailableDuration += diffTime;
      tm_duration = PM_Time_Mngt_convertSecondsToTm(AvailableDuration);
      LOG_D(TAG, "Total available duration : %02d:%02d:%02d in s: %d", tm_duration.tm_hour, tm_duration.tm_min, tm_duration.tm_sec, AvailableDuration);
    }
  }
  tm_duration = PM_Time_Mngt_convertSecondsToTm(AvailableDuration);
  LOG_D(TAG, "Still available filtration time by the end of the day: %02d:%02d:%02d in s: %d", tm_duration.tm_hour, tm_duration.tm_min, tm_duration.tm_sec, AvailableDuration);

  localtime_r(&now,&tm_NextStartTime);  // for eradicate compiler warnings
  localtime_r(&now,&tm_NextEndTime);    // for eradicate compiler warnings

  if (AvailableDuration >= (FiltrationDurationInSeconds-FiltrationDoneInSeconds)) {
    LOG_D(TAG, "There is enough time by the end of the day");
    tm_duration = PM_Time_Mngt_convertSecondsToTm(AvailableDuration);
    diffTime=(FiltrationDurationInSeconds-FiltrationDoneInSeconds);
    tm_diffTime = PM_Time_Mngt_convertSecondsToTm(diffTime);
    LOG_D(TAG, "Need %02d:%02d:%02d (%ds) and stay only : %02d:%02d:%02d (%ds)", tm_duration.tm_hour, tm_duration.tm_min, tm_duration.tm_sec, AvailableDuration, tm_diffTime.tm_hour, tm_diffTime.tm_min, tm_diffTime.tm_sec, diffTime);
    // It keeps enough time by the end of the day
    for (int i = 0; i <= NumberLine-1; i++ ) {
      if (PM_FiltrationPeriod_Abaqus[i].End <= tm_now->tm_hour) {
        // too late for this period
        LOG_D(TAG, "too late for this period");
      } else if (PM_FiltrationPeriod_Abaqus[i].Start <= tm_now->tm_hour && tm_now->tm_hour < PM_FiltrationPeriod_Abaqus[i].End) {
        // we are just in this period, count only the time from now to the end of the period
        LOG_D(TAG, "we are just in this period, count only the time from now to the end of the period");
        localtime_r(&now, &tm_NextStartTime);
        NextStartTime = now;

        if ( PM_FiltrationPeriod_Abaqus[i].End >= 24 ) {  // need to calcule the new date
          Date Today;              // == today
          Date Tomorrow = Today+1; // == Tomorrow
          tm_NextEndTime.tm_year = Tomorrow.getYear() -1900;
          tm_NextEndTime.tm_mon  = Tomorrow.getMonth() -1;
          tm_NextEndTime.tm_mday = Tomorrow.getDay();
          tm_NextEndTime.tm_hour = 0;
          tm_NextEndTime.tm_min  = 0;
          tm_NextEndTime.tm_sec  = 0;
          NextEndTime = mktime(&tm_NextEndTime);
        } 
        else {
          localtime_r(&now, &tm_NextEndTime);
          tm_NextEndTime.tm_hour = PM_FiltrationPeriod_Abaqus[i].End;
          tm_NextEndTime.tm_min  = 0;
          tm_NextEndTime.tm_sec  = 0;
          NextEndTime = mktime(&tm_NextEndTime);
        }
        if ( (NextEndTime - NextStartTime) > (FiltrationDurationInSeconds - FiltrationDoneInSeconds) ) {
          NextEndTime = NextStartTime + (FiltrationDurationInSeconds - FiltrationDoneInSeconds);
          localtime_r(&NextEndTime, &tm_NextEndTime);
        } 
        break;
      } 
      else {
        // this period is on the following hours
        LOG_D(TAG, "this period is on the following hours");
        localtime_r(&now, &tm_NextStartTime);
        tm_NextStartTime.tm_hour = PM_FiltrationPeriod_Abaqus[i].Start;
        tm_NextStartTime.tm_min  = 0;
        tm_NextStartTime.tm_sec  = 0;
        NextStartTime = mktime(&tm_NextStartTime);
        
        localtime_r(&now, &tm_NextEndTime);
        tm_NextEndTime.tm_hour = PM_FiltrationPeriod_Abaqus[i].End;
        tm_NextEndTime.tm_min  = 0;
        tm_NextEndTime.tm_sec  = 0;
        NextEndTime = mktime(&tm_NextEndTime);
        if ( (NextEndTime - NextStartTime) > (FiltrationDurationInSeconds - FiltrationDoneInSeconds) ) {
          NextEndTime = NextStartTime + (FiltrationDurationInSeconds - FiltrationDoneInSeconds);
          localtime_r(&NextEndTime, &tm_NextEndTime);
        }
        break;
      }
    }
  } 
  else {
    // Not enough time by the end of the day, we keep all of this time nevertheless
    LOG_D(TAG, "Not enough time by the end of the day, we keep all of this time nevertheless");
    localtime_r(&now, &tm_NextStartTime);
    NextStartTime = now;

    Date Today;              // == today
    Date Tomorrow = Today+1; // == Tomorrow
    tm_NextEndTime.tm_year = Tomorrow.getYear() -1900;
    tm_NextEndTime.tm_mon  = Tomorrow.getMonth() -1;
    tm_NextEndTime.tm_mday = Tomorrow.getDay();
    tm_NextEndTime.tm_hour = 0;
    tm_NextEndTime.tm_min  = 0;
    tm_NextEndTime.tm_sec  = 0;
    NextEndTime = mktime(&tm_NextEndTime);
  }
  LOG_D(TAG, "- next start time: %04d/%02d/%02d %02d:%02d:%02d", tm_NextStartTime.tm_year+1900, tm_NextStartTime.tm_mon+1, tm_NextStartTime.tm_mday, tm_NextStartTime.tm_hour, tm_NextStartTime.tm_min, tm_NextStartTime.tm_sec);
  LOG_D(TAG, "- next end time  : %04d/%02d/%02d %02d:%02d:%02d", tm_NextEndTime.tm_year+1900, tm_NextEndTime.tm_mon+1, tm_NextEndTime.tm_mday, tm_NextEndTime.tm_hour, tm_NextEndTime.tm_min, tm_NextEndTime.tm_sec);
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


