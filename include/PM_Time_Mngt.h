/*
  Copyright 2022 - Yves Gaignard
  
  Functions to manage time in Pool Manager project
*/
#ifndef PM_Time_Mngt_h
#define PM_Time_Mngt_h

#include <Arduino.h>
#include <WiFiUDP.h>            // Library for WiFi management
#include <NTPClient.h>          // Library for NTP time management

// Timezone definition: https://sites.google.com/a/usapiens.com/opnode/time-zones

void PM_Time_Mngt_initialize_time(void);
void PM_Time_Mngt_obtain_time(void);
void PM_Time_Mngt_initialize_sntp(void);
void PM_Time_Mngt_time_sync_notification_cb(struct timeval *tv);
std::string PM_Time_Mngt_convertTimeToString(time_t time_in, const char* string_format );
tm PM_Time_Mngt_convertSecondsToTm(const ulong seconds);

#endif