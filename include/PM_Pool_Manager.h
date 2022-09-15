/*
  Copyright 2022 - Yves Gaignard
  
  Functions to manage time in Pool Manager project
*/
#ifndef PM_Pool_Manager_h
#define PM_Pool_Manager_h

#define LOG_LEVEL LOG_INFO      // Possible levels : NONE/ERROR/WARNING/INFO/DEBUG/VERBOSE
//#define LOG_LEVEL LOG_DEBUG      // Possible levels : NONE/ERROR/WARNING/INFO/DEBUG/VERBOSE

#include <Arduino.h>
#include <RTClib.h>
#include <PID_v1.h>             // Library for PID controller (Proportional–Integral–Derivative controller)
#include <ADS1115.h>
#include <Preferences.h>
#include <WiFiMulti.h>             // Library for WiFi management

#include "PM_Log.h"
#include "PM_TFT.h"
#include "PM_Structures.h"
#include "PM_Temperature.h"
#include "PM_Pump.h" 

// Instantiate TFT display and a screen template
extern PM_TFT PM_tft;

// NVS Non Volatile SRAM (eqv. EEPROM)
extern Preferences nvs;   
extern PM_SwimmingPoolMeasures     pm_measures;

// To manage wifi between multiple networks
extern WiFiMulti wifiMulti;

// To manage the connection on Wifi
extern boolean IsWifiConnected;

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

// Instantiate object to manage all other sensors
extern ADS1115Scanner PM_ads;

// Various flags
extern volatile bool startTasks;                       // Signal to start loop tasks
extern bool EmergencyStopFiltPump;                     // Filtering pump stopped manually; needs to be cleared to restart



// extern functions
//extern void PM_Display_screen_1();
//extern void PM_Display_screen_2();

extern void PM_ComputeNextFiltrationPeriods();

extern void PM_Write_UpTime();
extern void PM_FiltrationPumpStart();
extern void PM_FiltrationPumpStop();

extern void lockI2C();
extern void unlockI2C();

extern bool PM_NVS_Init();
// Load measures from NVS
extern bool PM_NVS_Load();
// Save all measures to NVS
extern bool PM_NVS_Save();
// functions to save any type of parameter (4 overloads with same name but different arguments)
extern bool PM_NVS_saveParam(const char* key, uint8_t val);
extern bool PM_NVS_saveParam(const char* key, bool val);
extern bool PM_NVS_saveParam(const char* key, unsigned long val);
extern bool PM_NVS_saveParam(const char* key, double val);
extern bool PM_NVS_saveParam(const char* key, float val);

extern int  freeRam();
extern unsigned stack_hwm();
extern void stack_mon(UBaseType_t &hwm);

#endif