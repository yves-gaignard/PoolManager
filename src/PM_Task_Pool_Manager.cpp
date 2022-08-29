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
bool saveParam(const char*,uint8_t );
bool saveParam(const char*,bool );
bool saveParam(const char*,unsigned long );
bool saveParam(const char*,double );
void SetPhPID(bool);
void SetOrpPID(bool);
void mqttErrorPublish(const char*);
void UpdateTFT(void);
void stack_mon(UBaseType_t&);
void Send_IFTTTNotif(void);

void PM_Task_Pool_Manager(void *pvParameters)
{
  bool DoneForTheDay = false;                     // Reset actions done once per day
  bool d_calc = false;                            // Filtration duration computed

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

  for(;;)
  {  
    // reset watchdog
    esp_task_wdt_reset();

    #ifdef CHRONO
    td = millis();
    #endif    

    // Handle OTA update
    //ArduinoOTA.handle();

    //update pumps
    FiltrationPump.loop();
    PhPump.loop();
    ChlPump.loop();

    //reset time counters at midnight and send sync request to time server
    now = pftime::time(nullptr); // get current time
    tm_now = pftime::localtime(&now);

    LOG_D(TAG, "PhPump.IsRunning() = %d", PhPump.IsRunning());
    if(PhPump.IsRunning() ) {
      pm_measures.PeriodFiltrationUptime = PhPump.GetUpTime();
      saveParam("PFiltrUptime", (unsigned long)pm_measures.PeriodFiltrationUptime);
      LOG_D(TAG, "PeriodFiltrationUptime = %d", pm_measures.PeriodFiltrationUptime);
    }

    
    if (tm_now->tm_hour == 0 && !DoneForTheDay)
    {
      LOG_I(TAG, " !!!!! --- Midnight --- New day parameter computation --- !!!!!");
      
      //First store current Chl and Acid consumptions of the day in Eeprom
      pm_measures.pHMinusTankFill  = PhPump.GetTankFill();
      pm_measures.ChlorineTankFill = ChlPump.GetTankFill();
      saveParam("pHMinusTankFill", pm_measures.pHMinusTankFill);
      saveParam("ChlorinTankFill", pm_measures.ChlorineTankFill);

      //First store current uptime of the period of the filtration pump in Eeprom
      pm_measures.DayFiltrationUptime += pm_measures.PeriodFiltrationUptime;
      saveParam("DayFiltrUptime", (unsigned long)pm_measures.DayFiltrationUptime);
      pm_measures.PeriodFiltrationUptime = 0;
      saveParam("PFiltrUptime", (unsigned long)pm_measures.PeriodFiltrationUptime);

      //Save current uptime and target filtration of the day in the previous day info
      pm_measures.PreviousDayFiltrationUptime= pm_measures.DayFiltrationUptime;
      saveParam("PDayFiltrUptime", (unsigned long)pm_measures.PreviousDayFiltrationUptime);
      pm_measures.PreviousDayFiltrationTarget= pm_measures.DayFiltrationTarget;
      saveParam("PDayFiltrTarget", (unsigned long)pm_measures.PreviousDayFiltrationTarget);
  
      // reset all day's parameters
      FiltrationPump.ResetUpTime();
      PhPump.ResetUpTime();
      PhPump.SetTankFill(pm_measures.pHMinusTankFill);
      ChlPump.ResetUpTime();
      ChlPump.SetTankFill(pm_measures.ChlorineTankFill);

      EmergencyStopFiltPump = false;
      d_calc = false;
      DoneForTheDay = true;
        
      // update the time from NTP server
      PM_Time_Mngt_initialize_time();

      // reset the filtration time and already filtered time for this new day
      pm_measures.DayFiltrationTarget = 0;
      saveParam("DayFiltrTarget", (unsigned long)pm_measures.DayFiltrationTarget);
      pm_measures.DayFiltrationUptime = 0;
      saveParam("DayFiltrUptime", (unsigned long)pm_measures.DayFiltrationUptime);
      
      // Compute the new periods of filtration for this new day
      PM_CalculateNextFiltrationPeriods();   
      if (pm_measures.AutoMode && (now >= pm_measures.PeriodFiltrationStartTime) && (now < pm_measures.PeriodFiltrationEndTime)) {
        if (FiltrationPump.Start() ) {
        LOG_D(TAG, "Start filtration pump");
        LOG_D(TAG, "PhPump.IsRunning() = %d", PhPump.IsRunning());
        } 
        else {
        LOG_E(TAG, "Cannot Start filtration pump !!!!");
        }
      }
      else {
        if (FiltrationPump.Stop() ) {
        LOG_D(TAG, "Stop filtration pump");
        LOG_D(TAG, "PhPump.IsRunning() = %d", PhPump.IsRunning());
        }
        else {
        LOG_E(TAG, "Cannot Stop filtration pump !!!!");
        }
      } 
      d_calc = true;
    }
    else if(tm_now->tm_hour == 1) {
      if (DoneForTheDay) {
        DoneForTheDay = false;
        LOG_I(TAG, " !!!!! --- It is 01am --- End of new day computation --- !!!!!");
      }
    }

    //start filtration pump as scheduled
    if (!EmergencyStopFiltPump && !FiltrationPump.IsRunning() && pm_measures.AutoMode &&
        !PSIError && now >= pm_measures.PeriodFiltrationStartTime && now < pm_measures.PeriodFiltrationEndTime ) {
            if (FiltrationPump.Start() ) {
            LOG_I(TAG, " !!!!! --- Start Filtration pump as scheduled --- !!!!!");
            LOG_D(TAG, "PhPump.IsRunning() = %d", PhPump.IsRunning());
            } 
            else {
            LOG_E(TAG, "Cannot Start filtration pump !!!!");
            }

            if (pm_measures.PeriodFiltrationUptime != 0) {
                pm_measures.DayFiltrationUptime += pm_measures.PeriodFiltrationUptime;
                saveParam("DayFiltrUptime", (unsigned long)pm_measures.DayFiltrationUptime);
                pm_measures.PeriodFiltrationUptime = 0;
                saveParam("PFiltrUptime", (unsigned long)pm_measures.PeriodFiltrationUptime);
                LOG_E(TAG, "PeriodFiltrationUptime != 0 when it should = 0");
            } 
        }
    
    // start PIDs with delay after FiltrationStart in order to let the readings stabilize
    // start inhibited if water temperature below threshold and/or in winter mode
    if (FiltrationPump.IsRunning() && pm_measures.AutoMode && !pm_measures.WinterMode && !pHPID.GetMode() &&
        ((millis() - FiltrationPump.LastStartTime) / 1000 / 60 >= pm_measures.DelayPIDs) &&
        (now >= pm_measures.PeriodFiltrationStartTime) && (now < pm_measures.PeriodFiltrationEndTime) &&
        pm_measures.WaterTemp >= pm_measures.WaterTempLowThreshold)
    {
        //Start PIDs
        SetPhPID(true);
        SetOrpPID(true);
        LOG_I(TAG, " !!!!! --- Start pH and Orp PID as scheduled --- !!!!!");
    }

    //stop filtration pump and PIDs as scheduled unless we are in AntiFreeze mode
    if (pm_measures.AutoMode && FiltrationPump.IsRunning() && !AntiFreezeFiltering && (now >= pm_measures.PeriodFiltrationEndTime || now < pm_measures.PeriodFiltrationStartTime))
    {
        SetPhPID(false);
        SetOrpPID(false);
        if (FiltrationPump.Stop() ) {
            LOG_D(TAG, "Stop filtration pump");
            LOG_D(TAG, "PhPump.IsRunning() = %d", PhPump.IsRunning());
        }
        else {
            LOG_E(TAG, "Cannot Stop filtration pump !!!!");
        }
        pm_measures.DayFiltrationUptime += pm_measures.PeriodFiltrationUptime;
        saveParam("DayFiltrUptime", (unsigned long)pm_measures.DayFiltrationUptime);
        pm_measures.PeriodFiltrationUptime = 0;
        saveParam("PFiltrUptime", (unsigned long)pm_measures.PeriodFiltrationUptime);

        LOG_I(TAG, " !!!!! --- Stop Filtration, pH and Orp PID --- !!!!!");
    }

    //Outside regular filtration hours, start filtration in case of cold Air temperatures (<-2.0deg)
    if (!EmergencyStopFiltPump && pm_measures.AutoMode && !PSIError && !FiltrationPump.IsRunning() && ((now < pm_measures.PeriodFiltrationStartTime) || (now > pm_measures.PeriodFiltrationEndTime)) && (pm_measures.OutAirTemp < -2.0))
    {
        FiltrationPump.Start();
        if (pm_measures.PeriodFiltrationUptime != 0) {
            pm_measures.DayFiltrationUptime += pm_measures.PeriodFiltrationUptime;
            saveParam("DayFiltrUptime", (unsigned long)pm_measures.DayFiltrationUptime);
            pm_measures.PeriodFiltrationUptime = 0;
            saveParam("PFiltrUptime", (unsigned long)pm_measures.PeriodFiltrationUptime);
            LOG_E(TAG, "PeriodFiltrationUptime != 0 when it should = 0");
        } 

        AntiFreezeFiltering = true;
        LOG_I(TAG, " !!!!! --- Start filtration due to antifreeze --- !!!!!");
    }

    //Outside regular filtration hours and if in AntiFreezeFiltering mode but Air temperature rose back above 2.0deg, stop filtration
    if (pm_measures.AutoMode && FiltrationPump.IsRunning() && ((now < pm_measures.PeriodFiltrationStartTime) || (now > pm_measures.PeriodFiltrationEndTime)) && AntiFreezeFiltering && (pm_measures.OutAirTemp > 2.0)) {
        if (FiltrationPump.Stop() ) {
            LOG_D(TAG, "Stop filtration pump");
            LOG_D(TAG, "PhPump.IsRunning() = %d", PhPump.IsRunning());
        }
        else {
            LOG_E(TAG, "Cannot Stop filtration pump !!!!");
        }
        pm_measures.DayFiltrationUptime += pm_measures.PeriodFiltrationUptime;
        saveParam("DayFiltrUptime", (unsigned long)pm_measures.DayFiltrationUptime);
        pm_measures.PeriodFiltrationUptime = 0;
        saveParam("PFiltrUptime", (unsigned long)pm_measures.PeriodFiltrationUptime);

        AntiFreezeFiltering = false;
        LOG_I(TAG, " !!!!! --- Stop filtration due to end of antifreeze --- !!!!!");
    }

    //If filtration pump has been running for over 45secs but pressure is still low, stop the filtration pump, something is wrong, set error flag
    if (FiltrationPump.IsRunning() && ((millis() - FiltrationPump.LastStartTime) > 45000) && (pm_measures.Pressure < pm_measures.PressureMedThreshold)) {
        if (FiltrationPump.Stop() ) {
            LOG_D(TAG, "Stop filtration pump");
            LOG_D(TAG, "PhPump.IsRunning() = %d", PhPump.IsRunning());
        }
        else {
            LOG_E(TAG, "Cannot Stop filtration pump !!!!");
        }
        pm_measures.DayFiltrationUptime += pm_measures.PeriodFiltrationUptime;
        saveParam("DayFiltrUptime", (unsigned long)pm_measures.DayFiltrationUptime);
        pm_measures.PeriodFiltrationUptime = 0;
        saveParam("PFiltrUptime", (unsigned long)pm_measures.PeriodFiltrationUptime);

        PSIError = true;
        LOG_I(TAG, " !!!!! --- Stop filtration as the pressure is still low --- !!!!!");
        //mqttErrorPublish("{\"Pressure Error\":1}");
    }  

    // Over-pressure error
    if (pm_measures.Pressure > pm_measures.PressureHighThreshold) {
        if (FiltrationPump.Stop() ) {
            LOG_D(TAG, "Stop filtration pump");
            LOG_D(TAG, "PhPump.IsRunning() = %d", PhPump.IsRunning());
        }
        else {
            LOG_E(TAG, "Cannot Stop filtration pump !!!!");
        }
        pm_measures.DayFiltrationUptime += pm_measures.PeriodFiltrationUptime;
        saveParam("DayFiltrUptime", (unsigned long)pm_measures.DayFiltrationUptime);
        pm_measures.PeriodFiltrationUptime = 0;
        saveParam("PFiltrUptime", (unsigned long)pm_measures.PeriodFiltrationUptime);

        PSIError = true;
        LOG_I(TAG, " !!!!! --- Stop filtration as the pressure is TOO HIGH --- !!!!!");
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

//Enable/Disable Chl PID
void SetPhPID(bool Enable)
{
  if (Enable)
  {
    //Start PhPID
    PhPump.ClearErrors();
    pm_measures.pHPIDOutput = 0.0;
    pm_measures.pHPIDwindowStartTime = millis();
    pHPID.SetMode(AUTOMATIC);
    pm_measures.pH_RegulationOnOff = 1;
  }
  else
  {
    //Stop PhPID
    pHPID.SetMode(MANUAL);
    pm_measures.pH_RegulationOnOff = 0;
    pm_measures.pHPIDOutput = 0.0;
    PhPump.Stop();
  }
}

//Enable/Disable Orp PID
void SetOrpPID(bool Enable)
{
  if (Enable)
  {
    //Start OrpPID
    ChlPump.ClearErrors();
    pm_measures.OrpPIDOutput = 0.0;
    pm_measures.OrpPIDwindowStartTime = millis();
    OrpPID.SetMode(AUTOMATIC);
    pm_measures.Orp_RegulationOnOff = 1;

  }
  else
  {
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