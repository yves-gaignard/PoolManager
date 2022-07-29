/*
  Yves Gaignard
  
  Pool manager time management
*/

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// Standard library definitions
#include <Arduino.h>

#include <PM_Time.h>            // Pool manager time management

static const char* TAG = "PM_Time";

WiFiUDP   PM_Time::_ntpUDP ;
NTPClient PM_Time::_timeNTPClient (PM_Time::_ntpUDP, "fr.pool.ntp.org", 2*3600, 60000);
boolean   PM_Time::_isInitialized = false;

time_t    PM_Time::_initTime;
time_t    PM_Time::_lastGetNTPTime;

// Constructor
PM_Time::PM_Time() {
}

/**
* initialize time from NTP server if possible
*/

boolean PM_Time::initTime() {
  while ( ! _isInitialized ){
    _timeNTPClient.begin();
    delay(2000);
    _timeNTPClient.update();
    String time_now;
    time_t now_epoch = _timeNTPClient.getEpochTime();
    time_now = convertTimeToString(now_epoch);
    _isInitialized = _timeNTPClient.isTimeSet();
    if ( _isInitialized) {
      _initTime=now_epoch;
      ESP_LOGV(TAG, "Initialize Date and Time from NTP server: %s", time_now.c_str());
    }
    else {
      Serial.println("");
      ESP_LOGE(TAG, "cannot Initialize Date and Time from NTP server");
    }
    delay (100);
  }

  return _isInitialized;
}


/**
* get current time from NTP server
*/
time_t PM_Time::getCurrentNTPTime() {
  if ( ! _isInitialized ) {
    initTime();
  }

  time_t now_epoch = _timeNTPClient.getEpochTime();
  _lastGetNTPTime = now_epoch;

  String time_now = convertTimeToString(now_epoch);
  ESP_LOGV(TAG, "Get Date and Time from NTP server: %s", time_now.c_str());

  return _lastGetNTPTime ;
}

/**
* Convert time_t time to string depending on a format
*/
String PM_Time::convertTimeToString(time_t time_in, const char* string_format ) {
  tm * time_tm;
	char timestamp_string[100];
  //char default_time_string_format[50]="%Y-%m-%dT%H:%M:%SZ";
	
	time_tm = gmtime(&time_in);
	
	strftime(timestamp_string, 100, string_format, time_tm);
  return timestamp_string;
}
