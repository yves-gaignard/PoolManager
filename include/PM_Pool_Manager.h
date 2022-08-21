/*
  Copyright 2022 - Yves Gaignard
  
  Functions to manage time in Pool Manager project
*/
#ifndef PM_Pool_Manager_h
#define PM_Pool_Manager_h

#define LOG_LEVEL LOG_INFO      // Possible levels : NONE/ERROR/WARNING/INFO/DEBUG/VERBOSE

#include <Arduino.h>
#include <RTClib.h>
#include <PID_v1.h>             // Library for PID controller (Proportional–Integral–Derivative controller)
#include "PM_Log.h"
#include "PM_LCD.h"
#include "PM_Structures.h"
#include "PM_Screens.h"
#include "PM_Temperature.h"
#include "PM_Pump.h" 

// Instantiate LCD display and a screen template
extern PM_LCD lcd;
extern PM_Screens screens;

extern PM_SwimmingPoolMeasures     pm_measures;
extern PM_SwimmingPoolMeasures_str pm_measures_str;

//PIDs instances
extern PID pHPID;
extern PID OrpPID;

//Pumps instances
extern PM_Pump FiltrationPump;
extern PM_Pump PhPump;
extern PM_Pump ChlPump;

// To manage time
extern RTC_DS3231 rtc;  // RTC device handle
extern boolean    isRTCFound;
extern boolean    isRTCLostPower;
extern time_t     now;  // Current time (global variable)

// Instantiate object to manage all temperature sensors
extern PM_Temperature PM_TemperatureSensors;

// Various flags
extern volatile bool startTasks;                       // Signal to start loop tasks
extern bool EmergencyStopFiltPump;                     // Filtering pump stopped manually; needs to be cleared to restart



// extern functions
extern void PM_Display_screen_1();
extern void PM_Display_screen_2();

extern void PM_CalculateNextFiltrationPeriods();

extern bool saveParam(const char* key, uint8_t val);
extern bool saveParam(const char* key, bool val);
extern bool saveParam(const char* key, unsigned long val);
extern bool saveParam(const char* key, double val);
extern bool saveParam(const char* key, float val);

extern int  freeRam();
extern unsigned stack_hwm();
extern void stack_mon(UBaseType_t &hwm);

#endif