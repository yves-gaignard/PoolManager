/*
  Yves Gaignard
  
  Main procedure of the Pool Manager project
*/

#define TAG "PM_Task_Pool_Manager"

// Supervisory task

#include <Arduino.h>              // Arduino framework
#include <esp_task_wdt.h>         // ESP task management library
#include <ESPPerfectTime.h>       // Library for management time

#include "PM_Pool_Manager.h"
#include "PM_Time_Mngt.h"

//static WiFiClient wificlient;

// Functions prototypes
void ProcessCommand(char*);
void StartTime(void);
void readLocalTime(void);
void PM_Write_Filtration_UpTime(boolean ForceWrite);
void PM_FiltrationPumpStart();
void PM_FiltrationPumpStop();
void SetPhPID(bool);
void SetOrpPID(bool);
void mqttErrorPublish(const char*);
void UpdateTFT(void);
void stack_mon(UBaseType_t&);
void Send_IFTTTNotif(void);


unsigned long LastWrittenUpTime = 0;
unsigned long FrequencyWriteUpTime = 60; // in seconds, write frequency on NVS

void PM_Task_Pool_Manager(void *pvParameters)
{
  bool DoneForTheDay = false;                     // Reset actions done once per day
  
  bool AntiFreezeFiltering = false;               // Filtration anti freeze mode
  bool EmergencyStopFiltPump = false;             // flag will be (re)set by double-tapp button
  bool PSIError = false;                          // Water pressure OK

  static UBaseType_t hwm=0;                       // free stack size

  while(!startTasks);
  vTaskDelay(DT3);                                // Scheduling offset 

  esp_task_wdt_add(nullptr);
  TickType_t period = PT3;  
  TickType_t ticktime = xTaskGetTickCount(); 

  #ifdef CHRONO
  unsigned long td;
  int t_act=0,t_min=999,t_max=0;
  float t_mean=0.;
  int n=1;
  #endif

  tm * tm_now;
  time_t now;

  LastWrittenUpTime = pftime::time(nullptr); // get current time

  for(;;)
  {  
    // reset watchdog
    esp_task_wdt_reset();

    // Handle OTA update
    //ArduinoOTA.handle();

    //update pump information
    FiltrationPump.loop();
    PhPump.loop();
    ChlPump.loop();

    PM_Write_Filtration_UpTime(false);

    //reset time counters at midnight and send sync request to time server
    now = pftime::time(nullptr); // get current time
    tm_now = pftime::localtime(&now);

    if (tm_now->tm_hour == 0 && !DoneForTheDay)
    {
      LOG_I(TAG, " !!!!! --- Midnight --- New day parameter computation --- !!!!!");
      
      //First store current Chl and Acid consumptions of the day in Eeprom
      pm_measures.pHMinusTankFill  = PhPump.GetTankFill();
      pm_measures.ChlorineTankFill = ChlPump.GetTankFill();
      PM_NVS_saveParam("pHMinusTankFill", pm_measures.pHMinusTankFill);
      PM_NVS_saveParam("ChlorinTankFill", pm_measures.ChlorineTankFill);

      //First store current uptime of the period of the filtration pump in Eeprom
      PM_Write_Filtration_UpTime(true);
     
      //Save current uptime and target filtration of the day in the previous day info
      pm_measures.PreviousDayFiltrationUptime= pm_measures.DayFiltrationUptime;
      PM_NVS_saveParam("PDayFiltrUptime", (unsigned long)pm_measures.PreviousDayFiltrationUptime);
      pm_measures.PreviousDayFiltrationTarget= pm_measures.DayFiltrationTarget;
      PM_NVS_saveParam("PDayFiltrTarget", (unsigned long)pm_measures.PreviousDayFiltrationTarget);
  
      // reset all day's parameters
      FiltrationPump.ResetUpTime();
      PhPump.ResetUpTime();
      PhPump.SetTankFill(pm_measures.pHMinusTankFill);
      ChlPump.ResetUpTime();
      ChlPump.SetTankFill(pm_measures.ChlorineTankFill);

      EmergencyStopFiltPump = false;
      DoneForTheDay = true;
        
      // update the time from NTP server
      PM_Time_Mngt_initialize_time();

      // reset the filtration time and already filtered time for this new day
      pm_measures.DayFiltrationTarget = 0;
      PM_NVS_saveParam("DayFiltrTarget", (unsigned long)pm_measures.DayFiltrationTarget);
      LOG_I(TAG, "DayFiltrTarget = %d", pm_measures.DayFiltrationTarget);
      pm_measures.DayFiltrationUptime = 0;
      PM_NVS_saveParam("DayFiltrUptime", (unsigned long)pm_measures.DayFiltrationUptime);
      LOG_I(TAG, "DayFiltrUptime = %d", pm_measures.DayFiltrationUptime);
      
      // Compute next period of filtration for this new day
      PM_ComputeNextFiltrationPeriods();   
      if (pm_measures.AutoMode && (now >= pm_measures.PeriodFiltrationStartTime) && (now < pm_measures.PeriodFiltrationEndTime)) {
        PM_FiltrationPumpStart();
      }
      else {
        PM_FiltrationPumpStop();
      } 
    }
    else if(tm_now->tm_hour == 1) {
      if (DoneForTheDay) {
        DoneForTheDay = false;
        LOG_I(TAG, " !!!!! --- It is 01am --- End of new day computation --- !!!!!");
        LOG_I(TAG, "DayFiltrUptime = %d", pm_measures.DayFiltrationUptime);
      }
    }

    //start filtration pump as scheduled
    if (!EmergencyStopFiltPump && !FiltrationPump.IsRunning() && pm_measures.AutoMode &&
        !PSIError && now >= pm_measures.PeriodFiltrationStartTime && now < pm_measures.PeriodFiltrationEndTime ) {
      LOG_I(TAG, " !!!!! --- Start filtration pump as scheduled --- !!!!!");
      PM_FiltrationPumpStart();
    }
    
    // start PIDs with delay after FiltrationStart in order to let the readings stabilize
    // start inhibited if water temperature below threshold and/or in winter mode
    if (FiltrationPump.IsRunning() && pm_measures.AutoMode && !pm_measures.WinterMode && !pHPID.GetMode() &&
        ((millis() - FiltrationPump.LastStartTime) / 1000 / 60 >= pm_measures.DelayPIDs) &&
        (now >= pm_measures.PeriodFiltrationStartTime) && (now < pm_measures.PeriodFiltrationEndTime) &&
        pm_measures.WaterTemp >= pm_measures.WaterTempLowThreshold) {
      LOG_I(TAG, " !!!!! --- Start pH and Orp PID as scheduled --- !!!!!");
      //Start PIDs
      SetPhPID(true);
      SetOrpPID(true);
    }

    //stop filtration pump and PIDs as scheduled unless we are in AntiFreeze mode
    if (pm_measures.AutoMode && FiltrationPump.IsRunning() && !AntiFreezeFiltering 
      && (now >= pm_measures.PeriodFiltrationEndTime || now < pm_measures.PeriodFiltrationStartTime)) {
      LOG_I(TAG, " !!!!! --- Stop Filtration, pH and Orp PID --- !!!!!");
      SetPhPID(false);
      SetOrpPID(false);

      PM_Write_Filtration_UpTime(true);

      // stop the filtration
      PM_FiltrationPumpStop();

      // Compute next period of filtration
      PM_ComputeNextFiltrationPeriods();
    }

    //Outside regular filtration hours, start filtration in case of cold Air temperatures (<-2.0deg)
    if (!EmergencyStopFiltPump && pm_measures.AutoMode && !PSIError && !FiltrationPump.IsRunning() && ((now < pm_measures.PeriodFiltrationStartTime) || (now > pm_measures.PeriodFiltrationEndTime)) && (pm_measures.OutAirTemp < -2.0)) {
      LOG_I(TAG, " !!!!! --- Start antifreeze Filtration pump as outside temperature is < -2° --- !!!!!");
      PM_FiltrationPumpStart();
      AntiFreezeFiltering = true;
    }

    //Outside regular filtration hours and if in AntiFreezeFiltering mode but Air temperature rose back above 2.0deg, stop filtration
    if (pm_measures.AutoMode && FiltrationPump.IsRunning() && ((now < pm_measures.PeriodFiltrationStartTime) || (now > pm_measures.PeriodFiltrationEndTime)) && AntiFreezeFiltering && (pm_measures.OutAirTemp > 2.0)) {
      LOG_I(TAG, " !!!!! --- Stop filtration due to end of antifreeze --- !!!!!");
      PM_Write_Filtration_UpTime(true);

      // stop the filtration
      PM_FiltrationPumpStop();

      AntiFreezeFiltering = false;

      // Compute next period of filtration
      PM_ComputeNextFiltrationPeriods();
    }

    //If filtration pump has been running for over 45secs but pressure is still low, stop the filtration pump, something is wrong, set error flag
    if (FiltrationPump.IsRunning() && ((millis() - FiltrationPump.LastStartTime) > 45000) && (pm_measures.Pressure < pm_measures.PressureMedThreshold)) {
      LOG_I(TAG, " !!!!! --- Stop filtration as the pressure is still low --- !!!!!");
      PM_Write_Filtration_UpTime(true);

      // stop the filtration
      PM_FiltrationPumpStop();

      PSIError = true;
      //mqttErrorPublish("{\"Pressure Error\":1}");
    }  

    // Over-pressure error
    if (pm_measures.Pressure > pm_measures.PressureHighThreshold) {
      LOG_I(TAG, " !!!!! --- Stop filtration as the pressure is TOO HIGH --- !!!!!");
      PM_Write_Filtration_UpTime(true);

      // stop the filtration
      PM_FiltrationPumpStop();

      PSIError = true;
      //mqttErrorPublish("{\"Pressure Error\":1}");
    } else if(pm_measures.Pressure >= pm_measures.PressureMedThreshold) {
        PSIError = false;
    }

    //UPdate Nextion TFT
    //UpdateTFT();

    //Send IFTTT notifications if alarms occured
    //Send_IFTTTNotif();

    #ifdef CHRONO
    t_act = millis() - td;
    if(t_act > t_max) t_max = t_act;
    if(t_act < t_min) t_min = t_act;
    t_mean += (t_act - t_mean)/n;
    ++n;
    Debug.print(DBG_INFO,"[PoolMaster] td: %d t_act: %d t_min: %d t_max: %d t_mean: %4.1f",td,t_act,t_min,t_max,t_mean);
    #endif 

    stack_mon(hwm);
    vTaskDelayUntil(&ticktime,period);
  }
}

void PM_Write_Filtration_UpTime(boolean ForceWrite) {

  LOG_V(TAG, "FiltrationPump.IsRunning() = %d", FiltrationPump.IsRunning());
  if(FiltrationPump.IsRunning() ) {  
    pm_measures.DayFiltrationUptime = (FiltrationPump.UpTime + 500) / 1000 ; // round milliseconds to seconds
  
    // write on flash only every "FrequencyWriteUpTime" seconds or if it has been requested with ForceWrite to save flash memory
    now = pftime::time(nullptr); // get current time
    if ( ForceWrite || now > LastWrittenUpTime + FrequencyWriteUpTime) { 
      LOG_I(TAG, "Write DayFiltrationUptime: %d with FiltrationPump.UpTime = %d , LastWrittenUpTime = %d", pm_measures.DayFiltrationUptime, FiltrationPump.UpTime, LastWrittenUpTime);
      PM_NVS_saveParam("DayFiltrUptime", (unsigned long)pm_measures.DayFiltrationUptime);
      LastWrittenUpTime = now;
    }
    LOG_V(TAG, "DayFiltrationUptime = %d", pm_measures.DayFiltrationUptime);
  }
}

void PM_FiltrationPumpStart() {
  if (FiltrationPump.Start() ) {
    LOG_I(TAG, "Filtration pump started");
    LOG_V(TAG, "FiltrationPump.IsRunning() = %d", FiltrationPump.IsRunning());
  } 
  else {
    LOG_E(TAG, "Cannot Start filtration pump !!!!");
  }
}

void PM_FiltrationPumpStop() {
  if (FiltrationPump.Stop() ) {
    LOG_I(TAG, "Filtration pump stopped");
    LOG_V(TAG, "FiltrationPump.IsRunning() = %d", FiltrationPump.IsRunning());
  }
  else {
    LOG_E(TAG, "Cannot Stop filtration pump !!!!");
  }
}

//Enable/Disable Chl PID
void SetPhPID(bool Enable) {
  if (Enable) {
    //Start PhPID
    PhPump.ClearErrors();
    pm_measures.pHPIDOutput = 0.0;
    pm_measures.pHPIDwindowStartTime = millis();
    pHPID.SetMode(AUTOMATIC);
    pm_measures.pH_RegulationOnOff = 1;
  }
  else {
    //Stop PhPID
    pHPID.SetMode(MANUAL);
    pm_measures.pH_RegulationOnOff = 0;
    pm_measures.pHPIDOutput = 0.0;
    PhPump.Stop();
  }
}

//Enable/Disable Orp PID
void SetOrpPID(bool Enable) {
  if (Enable)  {
    //Start OrpPID
    ChlPump.ClearErrors();
    pm_measures.OrpPIDOutput = 0.0;
    pm_measures.OrpPIDwindowStartTime = millis();
    OrpPID.SetMode(AUTOMATIC);
    pm_measures.Orp_RegulationOnOff = 1;
  }
  else {
    //Stop OrpPID
    OrpPID.SetMode(MANUAL);
    pm_measures.Orp_RegulationOnOff = 0;
    pm_measures.OrpPIDOutput = 0.0;
    ChlPump.Stop();
  }
}

/*
//Send notifications to IFTTT applet in case of alarm
void Send_IFTTTNotif(){
    static const String url1 = IFTTT_key;
    String url2 = "";
    static bool notif_sent[5] = {0,0,0,0,0};

    if(PSIError)
    {
        if(!notif_sent[0])
        {
            if(wificlient.connect("maker.ifttt.com",80))
            {
                url2 = String("?value1=Water%20pressure&value2=");
                if(storage.PSIValue <= storage.PSI_MedThreshold)
                {
                    url2 += String("Low");
                } 
                else if (storage.PSIValue >= storage.PSI_HighThreshold)
                {
                    url2 += String("High");
                }
                url2 += String("%20pressure:%20") + String(storage.PSIValue) + String("bar");
                wificlient.print(String("POST ") + url1 + url2 + String(" HTTP/1.1\r\nHost: maker.ifttt.com\r\nConnection: close\r\n\r\n"));
                notif_sent[0] = true;
            }
        }    
    } else notif_sent[0] = false;

    if(!ChlPump.TankLevel())
    {
        if(!notif_sent[1])
        {
            if(wificlient.connect("maker.ifttt.com",80))
            {
                url2 = String("?value1=Chl%20level&value2=") + String(ChlPump.GetTankFill()) + String("%");
                wificlient.print(String("POST ") + url1 + url2 + String(" HTTP/1.1\r\nHost: maker.ifttt.com\r\nConnection: close\r\n\r\n"));
                notif_sent[1] = true;
            }
        }
    } else notif_sent[1] = false;

    if(!PhPump.TankLevel())
    {
        if(!notif_sent[2])
        {
            if(wificlient.connect("maker.ifttt.com",80))
            {
                url2 = String("?value1=pH+%20level&value2=") + String(PhPump.GetTankFill()) + String("%");
                wificlient.print(String("POST ") + url1 + url2 + String(" HTTP/1.1\r\nHost: maker.ifttt.com\r\nConnection: close\r\n\r\n"));
                notif_sent[2] = true;
            }
        }
    } else notif_sent[2] = false;

    if(ChlPump.UpTimeError)
    {
        if(!notif_sent[3])
        {
            if(wificlient.connect("maker.ifttt.com",80))
            {
                url2 = String("?value1=Chl%20pump%20uptime&value2=") + String(round(ChlPump.UpTime/60000.)) + String("min");
                wificlient.print(String("POST ") + url1 + url2 + String(" HTTP/1.1\r\nHost: maker.ifttt.com\r\nConnection: close\r\n\r\n"));
                notif_sent[3] = true;
            }
        }
    } else notif_sent[3] = false;

    if(PhPump.UpTimeError)
    {
        if(!notif_sent[4])
        {
            if(wificlient.connect("maker.ifttt.com",80))
            {
                url2 = String("?value1=pH+%20pump%20uptime&value2=") + String(round(PhPump.UpTime/60000.)) + String("min");
                wificlient.print(String("POST ") + url1 + url2 + String(" HTTP/1.1\r\nHost: maker.ifttt.com\r\nConnection: close\r\n\r\n"));
                notif_sent[4] = true;
            }
        }
    } else notif_sent[4] = false;     
}
*/