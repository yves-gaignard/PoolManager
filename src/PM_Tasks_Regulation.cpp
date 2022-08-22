/*
  Yves Gaignard
  
  pH and Orp regulation tasks
*/

#define TAG "PM_Tasks_Regulation"

// Supervisory task

#include <Arduino.h>              // Arduino framework
#include <esp_task_wdt.h>         // ESP task management library
#include <RunningMedian.h>

#include "PM_Pool_Manager.h"

// =================================================================================================
//                                pH REGULATION TASK OF POOL MANAGER
// =================================================================================================
void PM_Task_pHRegulation(void *pvParameters)
{
  while (!startTasks) ;
  vTaskDelay(DT6);                                // Scheduling offset 

  TickType_t period = PT6;  
  TickType_t ticktime = xTaskGetTickCount();
  static UBaseType_t hwm = 0;

  #ifdef CHRONO
  unsigned long td;
  int t_act=0,t_min=999,t_max=0;
  float t_mean=0.;
  int n=1;
  #endif

  for(;;)
  {
    #ifdef CHRONO
    td = millis();
    #endif 

    //do not compute PID if filtration pump is not running
    // Set also a lower limit at 30s (a lower pump duration does'nt mean anything)

    if (FiltrationPump.IsRunning()  && (pHPID.GetMode() == AUTOMATIC))
    {  
      if(pHPID.Compute()){
        LOG_D(TAG,"Ph  regulation: %10.2f, %13.9f, %13.9f, %17.9f",pm_measures.pHPIDOutput,pm_measures.pHValue,pm_measures.pH_SetPoint,pm_measures.pH_Kp);
        if(pm_measures.pHPIDOutput < (double)30000.) pm_measures.pHPIDOutput = 0.;
        LOG_I(TAG,"Ph  regulation: %10.2f",pm_measures.pHPIDOutput);
    #ifdef SIMU
        newpHOutput = true;
    #endif            
      }
    #ifdef SIMU
      else newpHOutput = false;
    #endif    
      /************************************************
       turn the Acid pump on/off based on pid output
      ************************************************/
	  unsigned long now = millis();
      if (now - pm_measures.pHPIDwindowStartTime > pm_measures.pHPIDWindowSize)
      {
        //time to shift the Relay Window
        pm_measures.pHPIDwindowStartTime += pm_measures.pHPIDWindowSize;
      }
      if ((unsigned long)pm_measures.pHPIDOutput <= now - pm_measures.pHPIDwindowStartTime)
        PhPump.Stop();
      else
        PhPump.Start();   
    }

    #ifdef CHRONO
    t_act = millis() - td;
    if(t_act > t_max) t_max = t_act;
    if(t_act < t_min) t_min = t_act;
    t_mean += (t_act - t_mean)/n;
    ++n;
    Debug.print(DBG_INFO,"[pHRegulation] td: %d t_act: %d t_min: %d t_max: %d t_mean: %4.1f",td,t_act,t_min,t_max,t_mean);
    #endif

    stack_mon(hwm);
    vTaskDelayUntil(&ticktime,period);
  }  
}

// =================================================================================================
//                                Orp REGULATION TASK OF POOL MANAGER
// =================================================================================================
void PM_Task_OrpRegulation(void *pvParameters)
{
  while (!startTasks) ;
  vTaskDelay(DT5);                                // Scheduling offset 

  TickType_t period = PT5;  
  TickType_t ticktime = xTaskGetTickCount();
  static UBaseType_t hwm = 0;

  #ifdef CHRONO
  unsigned long td;
  int t_act=0,t_min=999,t_max=0;
  float t_mean=0.;
  int n=1;
  #endif

  for(;;)
  { 
    #ifdef CHRONO
    td = millis();
    #endif 

    //do not compute PID if filtration pump is not running
    // Set also a lower limit at 30s (a lower pump duration does'nt mean anything)

    if (FiltrationPump.IsRunning() && (OrpPID.GetMode() == AUTOMATIC))
    {
      if(OrpPID.Compute()){
        LOG_D(TAG,"ORP regulation: %10.2f, %13.9f, %12.9f, %17.9f",pm_measures.OrpPIDOutput,pm_measures.OrpValue,pm_measures.Orp_SetPoint,pm_measures.Orp_Kp);
        if(pm_measures.OrpPIDOutput < (double)30000.) pm_measures.OrpPIDOutput = 0.;    
          LOG_I(TAG,"Orp regulation: %10.2f",pm_measures.OrpPIDOutput);
    #ifdef SIMU
          newChlOutput = true;
    #endif      
        }
    #ifdef SIMU
        else newChlOutput = false;
    #endif    
      /************************************************
       turn the Chl pump on/off based on pid output
      ************************************************/
	  unsigned long now = millis();
      if (now - pm_measures.OrpPIDwindowStartTime > pm_measures.OrpPIDWindowSize)
      {
        //time to shift the Relay Window
        pm_measures.OrpPIDwindowStartTime += pm_measures.OrpPIDWindowSize;
      }
      if ((unsigned long)pm_measures.OrpPIDOutput <= now - pm_measures.OrpPIDwindowStartTime)
        ChlPump.Stop();
      else
        ChlPump.Start();
    }

    #ifdef CHRONO
    t_act = millis() - td;
    if(t_act > t_max) t_max = t_act;
    if(t_act < t_min) t_min = t_act;
    t_mean += (t_act - t_mean)/n;
    ++n;
    Debug.print(DBG_INFO,"[OrpRegulation] td: %d t_act: %d t_min: %d t_max: %d t_mean: %4.1f",td,t_act,t_min,t_max,t_mean);
    #endif

    stack_mon(hwm);    
    vTaskDelayUntil(&ticktime,period);
  }
}
