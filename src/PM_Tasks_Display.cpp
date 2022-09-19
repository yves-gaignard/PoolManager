/*
  Copyright 2022 - Yves Gaignard
  
  Pool Manager tasks for display and web server
*/
#define TAG "PM_Tasks_Display"

#include <Arduino.h>
#include <ESPPerfectTime.h>

#include "PM_Tasks.h"
#include "PM_Parameters.h"
#include "PM_Log.h"
#include "PM_TFT.h"
#include "PM_Config.h"
#include "PM_Pool_Manager.h"

// Request to activate the display
boolean PM_Display_Activation_Request=true;

// Button declarations
boolean PM_DisplayButton_State = false;    // Current State
int     PM_DisplayButton_LastPressed = 0;  // last time it was pressed in millis

// =================================================================================================
//                               LCD MANAGEMENT TASK OF POOL MANAGER
// =================================================================================================
void PM_Task_TFT       ( void *pvParameters ) {
  
  while (!startTasks) ;
  vTaskDelay(DT7);                                // Scheduling offset 

  TickType_t period = PT7;  
  TickType_t ticktime = xTaskGetTickCount();
  static UBaseType_t hwm = 0;
  
  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20];
  time_t now;
  suseconds_t usec;

  for( ;; ) {
    now = pftime::time(nullptr); // get current time
    time_tm = pftime::localtime(&now, &usec);  // Change in localtime
	  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    LOG_V(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);

    PM_tft.Loop();

    stack_mon(hwm);
    vTaskDelayUntil(&ticktime,period);
  }
}
// =================================================================================================
//                                     WEB SERVER MANAGEMENT TASK OF POOL MANAGER
// =================================================================================================
void PM_Task_WebServer ( void *pvParameters ) {
  
  while (!startTasks) ;
  vTaskDelay(DT8);                                // Scheduling offset 

  TickType_t period = PT8;  
  TickType_t ticktime = xTaskGetTickCount();
  static UBaseType_t hwm = 0;
  //const char *pcTaskName = "Task_WebServer";
  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20];
  time_t now;
  suseconds_t usec;
  
  for( ;; ) {
    now = pftime::time(nullptr); // get current time
    time_tm = pftime::localtime(&now, &usec);  // Change in localtime
    strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    LOG_V(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);

    stack_mon(hwm);
    vTaskDelayUntil(&ticktime,period);
  }
}
