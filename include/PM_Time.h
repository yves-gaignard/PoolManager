/*
  Copyright 2022 - Yves Gaignard
  
  Functions to manage time in Pool Manager project
*/
#ifndef PM_Time_h
#define PM_Time_h

#include <Arduino.h>
#include <WiFiUDP.h>            // Library for WiFi management
#include <NTPClient.h>          // Library for NTP time management

class PM_Time {
  private:
    static WiFiUDP   _ntpUDP; 
    static NTPClient _timeNTPClient;
    static boolean   _isInitialized;

    static time_t    _initTime;
    static time_t    _lastGetNTPTime;

    /**
     * initialize time from NTP server if possible
     */
     static boolean initTime();

  public:
    // Constructors
    PM_Time();
    
    /**
     * get current time from NTP server
     */
    static time_t getCurrentNTPTime();

    /**
     * Convert a timestamp time_t into String based on a format
     *
     * @param time_in
     * @param string_format
     * @return formatted time
     */
    static String  convertTimeToString(time_t time_in, const char* string_format = "%Y-%m-%dT%H:%M:%SZ");
    
};

#endif