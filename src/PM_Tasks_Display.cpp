/*
  Copyright 2022 - Yves Gaignard
  
  Pool Manager tasks for display and web server
*/
#define TAG "PM_Tasks_Display"

#include <Arduino.h>
#include <FreeRTOS.h>

#include "PM_Tasks.h"
#include "PM_Parameters.h"
#include "PM_Log.h"
#include "PM_LCD.h"
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
void PM_Task_LCD       ( void *pvParameters ) {
  
  while (!startTasks) ;
  vTaskDelay(DT7);                                // Scheduling offset 

  TickType_t period = PT7;  
  TickType_t ticktime = xTaskGetTickCount();
  static UBaseType_t hwm = 0;
  
  //UBaseType_t uxPriority;
  //uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20];
  time_t now;

  for( ;; ) {
    time(&now);
    time_tm = localtime(&now);
	strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    //LOG_D(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);

    // if lcd display button is pressed then set the display ON in case of OFF
    if ( PM_Display_Activation_Request == true) {
      if (lcd.getDisplayState() == false ) {  
        // the LCD is OFF. We just set it ON again
        lcd.display();
        //LOG_D(TAG, "%s : Display is shown",timestamp_str);
        // reset the display duration counter
        screens.setInactivityTimeOutReset();
      }
      PM_Display_Activation_Request=false;
    }

    // If the display is ON
    if (lcd.getDisplayState() == true) {
      // if the timout of the current screen is not reached

      if ( (now - screens.getLastScreenStartTime()) > screens.getScreenSwitchTime()) { 
        // display the next screen
        int screen_index= (screens.getCurrentScreenIndex()+1)%screens.getScreenNumber();
        if (screen_index == 0) screen_index++; // don't display anymore the screen 0

        switch (screen_index) {
          case 1 : screens.setCurrentScreen(screen_index); 
              PM_Display_screen_1();
              //LOG_D(TAG, "%s : Display screen %d",timestamp_str, screen_index);
            break;
          case 2 : screens.setCurrentScreen(screen_index);
              PM_Display_screen_2();
              //LOG_D(TAG, "%s : Display screen %d",timestamp_str, screen_index);
            break;
          default:
            LOG_E(TAG, "%s : Cannot Display screen %d",timestamp_str, screen_index);
        }
      } 

      if ((now - screens.getDisplayStart()) >= screens.getInactivityMaxTime()) {
        // if no_activity at all during MAX_WITHOUT_ACTIVITES then stop the display
        lcd.noDisplay();
        lcd.noBacklight();
        //LOG_D(TAG, "%s : Display is stopped",timestamp_str);
      }
    }
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
  //UBaseType_t uxPriority;
  //uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20];
  time_t now;
  for( ;; ) {
    time(&now);
    time_tm = localtime(&now);
    strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    //LOG_D(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);

    stack_mon(hwm);
    vTaskDelayUntil(&ticktime,period);
  }
}

// =================================================================================================
//                              DISPLAY BUTTON INTERRUPTION MANAGEMENT
// =================================================================================================
void IRAM_ATTR PM_DisplayButton_ISR() {
  if (millis() - PM_DisplayButton_LastPressed > 100) { // Software debouncing button
    LOG_I(TAG, "Display button pressed");
    PM_Display_Activation_Request = true;
    PM_DisplayButton_State = !PM_DisplayButton_State;
  }
  PM_DisplayButton_LastPressed = millis();
}