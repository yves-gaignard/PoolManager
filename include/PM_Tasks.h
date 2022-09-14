/*
  Copyright 2022 - Yves Gaignard
  
  Declaration of tasks
*/

#include <Arduino.h>

void PM_Task_Pool_Manager   ( void *pvParameters );
void PM_Task_OrpRegulation  ( void *pvParameters );
void PM_Task_pHRegulation   ( void *pvParameters );
void PM_Task_GetTemperature ( void *pvParameters );
void PM_Task_AnalogPoll     ( void *pvParameters );
void PM_Task_TFT            ( void *pvParameters );
void PM_Task_WebServer      ( void *pvParameters );
