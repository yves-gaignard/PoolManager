/*
  Copyright 2022 - Yves Gaignard
  
  Declaration of tasks
*/

#include <Arduino.h>
#include <FreeRTOS.h>
#include "PM_LCD.h"

void PM_Task_Pool_Manager   ( void *pvParameters );
void PM_Task_OrpRegulation  ( void *pvParameters );
void PM_Task_pHRegulation   ( void *pvParameters );
void PM_Task_GetTemperature ( void *pvParameters );
void PM_Task_LCD            ( void *pvParameters );
void PM_Task_WebServer      ( void *pvParameters );
