/*
  Yves Gaignard
  
  Pool manager time management
*/

#define TAG "PM_Time_Mngt"

#include <Arduino.h>
#include "PM_Pool_Manager.h"
#include "PM_Time_Mngt.h"
#include "PM_Parameters.h"
#include <string.h>
#include <time.h>

void PM_Time_Mngt_initialize_time(void)
{
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  // Is time set? If not, tm_year will be (1970 - 1900).
  if (timeinfo.tm_year < (2021 - 1900)) {
    LOG_I(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
    PM_Time_Mngt_obtain_time();
    // update 'now' variable with current time
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (2021 - 1900)) {
      LOG_I(TAG, "Time is not set yet. Cannot get time over NTP. Get time from RTC");
      // if there is a RTC module, set the time with RTC time
      if (isRTCFound == true) {
        // set time with the RTC time
        DateTime DT_now (rtc.now());
        char DT_now_str[20]= "YYYY-MM-DD hh:mm:ss";
        DT_now.toString(DT_now_str);
        LOG_I(TAG, "Get the time from RTC: %s", DT_now_str);
        
        // set the time
        now = DT_now.unixtime();
        LOG_I(TAG, "rtc.now = %u", now );
        timeval tv = {now, 0}; 
        timezone tz = {0,0} ;
        int ret = settimeofday(&tv, &tz);
        if ( ret != 0 ) {LOG_E(TAG, "Cannot set time from RTC" ); }; 
      }
    }
  }
  // Set timezone to the zone provided in the parameter file to have the real local time
  char strftime_buf[64];
  setenv("TZ", PM_TimeZone, 1);
  tzset();
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  LOG_I(TAG, "The current local date/time is: %s", strftime_buf);
}

void PM_Time_Mngt_obtain_time(void)
{
  bool getTimeFromNTP = false;
  int retry = 0;
  const int retry_count = 30;
  while (! getTimeFromNTP && ++retry <= retry_count) {
    LOG_I(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    configTime(0, 0,"0.pool.ntp.org","1.pool.ntp.org","2.pool.ntp.org"); // 3 possible NTP servers
    setenv("TZ",PM_TimeZone,1);                       // configure local time with automatic DST  
    tzset();
    delay(200);

    tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        LOG_W(TAG, "Failed to obtain time from NTP servers");
    }
    else {
      getTimeFromNTP = true;
      time(&now);
      char strftime_buf[64];
      localtime_r(&now, &timeinfo);
      strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
      LOG_I(TAG, "The current local date/time is: %s", strftime_buf);
    }
  }

  // If there is no RTC module or if it lost its power, set the time with the NTP time
  if (isRTCLostPower == true || isRTCFound == true) {
    LOG_I(TAG, "Initialize RTC time with NTP time");

    // Get current time. It has been sent by NTP calls.
    time(&now);

    // adjust time of rtc with the time get from NTP server
    DateTime DT_now (now);
    if (DT_now.isValid()) {
      char DT_now_str[20]= "YYYY-MM-DD hh:mm:ss";
      DT_now.toString(DT_now_str);
      LOG_I(TAG, "Adjust the time of RTC with the NTP time: %s", DT_now_str);
      rtc.adjust(DT_now);
    }
    else {
      LOG_E(TAG, "Cannot set time to RTC as DT_now is not valid !!!!" );
    }
  }
}

/**
* Convert time_t time to string depending on a format
*/
std::string PM_Time_Mngt_convertTimeToString(time_t time_in, const char* string_format) {
  tm * time_tm;
	char timestamp_string[100];
  #define MAX_LENGTH_TIME_FORMAT 100
  char time_format[MAX_LENGTH_TIME_FORMAT]="%Y-%m-%dT%H:%M:%SZ";
  
  boolean isGmtRequested = false;
  int len=strlen(string_format);
  if (len > MAX_LENGTH_TIME_FORMAT-1) {
    LOG_E(TAG, "Too long datetime format: %d", len);
    return "";
  }
  if (len > 1) {
    strncpy(time_format,&string_format[len-1],len);
    char one_char[2];
    strncpy(one_char,&string_format[len-1],1);
    one_char[1] = '\0';
    LOG_V(TAG, "last char of format = %s", one_char);
    if  ( strcmp(one_char, "Z") == 0 ) { isGmtRequested = true; }
  }

  if (isGmtRequested) {
    LOG_V(TAG, "GMT time requested");
  	time_tm = gmtime(&time_in);
  }
  else {
    LOG_V(TAG, "LOCAL time requested");
  	time_tm = localtime(&time_in);
  }
	strftime(timestamp_string, 100, string_format, time_tm);
  LOG_V(TAG, "%s", timestamp_string);
  return timestamp_string;
}

/**
* Convert seconds in tm * structure
*/
tm PM_Time_Mngt_convertSecondsToTm(const ulong seconds) {
  tm local_time;
  long l = seconds & LONG_MAX;
  gmtime_r(&l, &local_time);
  return local_time;
}
