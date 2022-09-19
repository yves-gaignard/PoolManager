/*
  Yves Gaignard
  
  Analog sensor tasks
*/

#define TAG "PM_Tasks_Sensors"

// Supervisory task

#include <Arduino.h>              // Arduino framework
#include <esp_task_wdt.h>         // ESP task management library
#include <RunningMedian.h>
#include <ESPPerfectTime.h>

#include "PM_Pool_Manager.h"
#include "PM_Parameters.h"


// Setup an DFRobot_ADS1115 instance for analog measurements
// ---------------------------------------------------
TwoWire PM_ads_address(PM_ADS1115_Device_Addr);
DFRobot_ADS1115 PM_ads(&PM_ads_address);        // Address 0x48 is the default
static float ph_sensor_value;                   // pH sensor current value
static float orp_sensor_value;                  // ORP sensor current value
static float psi_sensor_value;                  // PSI sensor current value

// Signal filtering library sample buffers
static RunningMedian samples_WTemp = RunningMedian(11);
static RunningMedian samples_ATemp = RunningMedian(11);
static RunningMedian samples_Ph    = RunningMedian(11);
static RunningMedian samples_Orp   = RunningMedian(11);
static RunningMedian samples_PSI   = RunningMedian(11);

// =================================================================================================
//                                GET TEMPERATURE TASK OF POOL MANAGER
// =================================================================================================
void PM_Task_GetTemperature      ( void *pvParameters ) {

  while (!startTasks) ;
  vTaskDelay(DT4);                                // Scheduling offset 

  TickType_t period = PT4;  
  TickType_t ticktime = xTaskGetTickCount();
  static UBaseType_t hwm = 0;

  PM_TemperatureSensors.requestTemperatures();
  vTaskDelayUntil(&ticktime,period);

  //const char *pcTaskName = "Task_GPIO";
  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20];
  time_t now;
  suseconds_t usec;

  std::string deviceName;
  float preciseTemperatureC = 0.0;
  //int   temperatureC =0;

  for( ;; ) {
    now = pftime::time(nullptr); // get current time
    time_tm = pftime::localtime(&now, &usec);  // Change in localtime
	  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    LOG_V(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);

    for (int i = 0 ; i < PM_TemperatureSensors.getDeviceCount(); i++) {
      deviceName = PM_TemperatureSensors.getDeviceNameByIndex(i);

      preciseTemperatureC = PM_TemperatureSensors.getPreciseTempCByName(deviceName);
      //LOG_I(TAG, "Sensor: %19s : % 4.2f°C", deviceName.c_str(),preciseTemperatureC);
      //temperatureC = PM_TemperatureSensors.getTempCByName(deviceName);
      //LOG_D(TAG, "Sensor: %19s : %6d°C", deviceName.c_str(),temperatureC);

      if (deviceName == insideThermometerName) {
        pm_measures.InAirTemp = preciseTemperatureC;
        PM_NVS_saveParam("InAirTemp", pm_measures.InAirTemp);
      }
      else if (deviceName == outsideThermometerName){
        pm_measures.OutAirTemp = preciseTemperatureC;
        PM_NVS_saveParam("OutAirTemp", pm_measures.OutAirTemp);
      } 
      else if (deviceName == waterThermometerName) {
        pm_measures.WaterTemp = preciseTemperatureC;
        PM_NVS_saveParam("WaterTemp", pm_measures.WaterTemp);
      }
    }

    PM_TemperatureSensors.requestTemperatures();
  
    stack_mon(hwm);
    vTaskDelayUntil(&ticktime,period);
  }
}

// =================================================================================================
//                      GET PH, ORP AND PRESSURE SENSOR VALUES TASK OF POOL MANAGER
// =================================================================================================
void AnalogInit()
{
    PM_ads.setAddr_ADS1115(ADS1115_IIC_ADDRESS0);   // 0x48
    PM_ads.setGain(eGAIN_TWOTHIRDS);   // 2/3x gain
    PM_ads.setMode(eMODE_SINGLE);       // single-shot mode
    PM_ads.setRate(eRATE_128);          // 128SPS (default)
    PM_ads.setOSMode(eOSMODE_SINGLE);   // Set to start a single-conversion
    PM_ads.init();
}

void PM_Task_AnalogPoll(void *pvParameters)
{
  while (!startTasks) ;

  TickType_t period = PT1;  
  TickType_t ticktime = xTaskGetTickCount(); 
  static UBaseType_t hwm=0;

  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20];
  time_t now;
  suseconds_t usec;

  //lockI2C();
  //PM_ads.start();
  //unlockI2C();
  //vTaskDelayUntil(&ticktime,period);
  
  for(;;)
  {
    now = pftime::time(nullptr); // get current time
    time_tm = pftime::localtime(&now, &usec);  // Change in localtime
    strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    LOG_V(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);

    lockI2C();

    if (PM_ads.checkADS1115()) {
      ph_sensor_value = PM_ads.readVoltage(PH_SENSOR_ANALOG_Pin); // pH sensor current value
      LOG_D(TAG,"pH: %10.5f mV", ph_sensor_value);
      orp_sensor_value = PM_ads.readVoltage(ORP_SENSOR_ANALOG_Pin); // ORP sensor current value
      //LOG_D(TAG,"pH: %10.5f mV", orp_sensor_value);
      psi_sensor_value = PM_ads.readVoltage(PRESSURE_SENSOR_ANALOG_Pin); // psi sensor current value
      //LOG_D(TAG,"pH: %10.5f mV", psi_sensor_value);

      //pH
      samples_Ph.add(ph_sensor_value);          // compute average of pH from center 5 measurements among 11
      // compute average of ORP from last 5 measurements
      LOG_D(TAG,"samples_Ph: %10.5f", samples_Ph.getAverage(5));
      //pm_measures.pHValue = (samples_Ph.getAverage(5)*0.1875/1000.)*pm_measures.pHCalibCoeffs0 + pm_measures.pHCalibCoeffs1; // Pool Master
      pm_measures.pHValue = (samples_Ph.getAverage(5)/1000)*pm_measures.pHCalibCoeffs0 + pm_measures.pHCalibCoeffs1; // Simple formule from phidget 1130
      //pm_measures.pHValue = (samples_Ph.getAverage(5)*0.1875/1000.)*pm_measures.pHCalibCoeffs0 + pm_measures.pHCalibCoeffs1; // Complex formule from phidget 1130


#ifdef SIMU
      if(!init_simu){
        if(newpHOutput) {
          pHTab[iw] = storage.PhPIDOutput;
          pHCumul = pHTab[0]+pHTab[1]+pHTab[2];
          iw++;
          iw %= 3;
        }
        storage.PhValue = pHLastValue + pHCumul/4500000.*(double)((millis()-pHLastTime)/3600000.);
        pHLastValue = storage.PhValue;
        pHLastTime = millis();
      } else {
        init_simu = false;
        pHLastTime = millis();
        pHLastValue = 7.0;
        storage.PhValue = pHLastValue;
        storage.OrpValue = OrpLastValue;
        OrpLastTime = millis();
        OrpLastValue = 730.0;
        for(uint8_t i=0;i<3;i++) {
          pHTab[i] = 0.;
          ChlTab[i] = 0.;
        }  
      }  
#endif

      //ORP
      samples_Orp.add(orp_sensor_value);
      // compute average of ORP from last 5 measurements
      //LOG_D(TAG,"samples_Orp: %10.5f", samples_Orp.getAverage(5));
      // pm_measures.OrpValue = (samples_Orp.getAverage(5)*0.1875/1000.)*pm_measures.OrpCalibCoeffs0 + pm_measures.OrpCalibCoeffs1;   // Pool Master
      pm_measures.OrpValue = (samples_Orp.getAverage(5)/1000 )*pm_measures.OrpCalibCoeffs0 + pm_measures.OrpCalibCoeffs1; // Simple formule from phidget 1130
      // pm_measures.OrpValue = (samples_Orp.getAverage(5)*0.1875/1000.)*pm_measures.OrpCalibCoeffs0 + pm_measures.OrpCalibCoeffs1; // Complex formule from phidget 1130

#ifdef SIMU
      if(!init_simu){
        if(newChlOutput) {
          ChlTab[jw] = storage.OrpPIDOutput;
          ChlCumul = ChlTab[0]+ChlTab[1]+ChlTab[2];
          jw++;
          jw %= 3;
        }    
        storage.OrpValue = OrpLastValue + ChlCumul/36000.*(double)((millis()-OrpLastTime)/3600000.);
        OrpLastValue = storage.OrpValue;
        OrpLastTime = millis();    
      } 
#endif

      //PSI (water pressure)
      samples_PSI.add(psi_sensor_value);       
      // compute average of PSI from last 5 measurements
      pm_measures.Pressure = (samples_PSI.getAverage(5)/1000.)*pm_measures.PSICalibCoeffs0 + pm_measures.PSICalibCoeffs1;

      LOG_D(TAG,"pH: %5.0f - %4.2f - ORP: %5.0f - %3.0fmV - PSI: %5.0f - %4.2fBar",
        ph_sensor_value,pm_measures.pHValue,orp_sensor_value,pm_measures.OrpValue,psi_sensor_value,pm_measures.Pressure);
    }
    else
    {
      LOG_E(TAG,"ADS115 disconnected");
    }
    unlockI2C();

    #ifdef CHRONO
    t_act = millis() - td;
    if(t_act > t_max) t_max = t_act;
    if(t_act < t_min) t_min = t_act;
    t_mean += (t_act - t_mean)/n;
    ++n;
    Debug.print(DBG_INFO,"[AnalogPoll] td: %d t_act: %d t_min: %d t_max: %d t_mean: %4.1f",td,t_act,t_min,t_max,t_mean);
    #endif 
 
    stack_mon(hwm);
    vTaskDelayUntil(&ticktime,period);
  }  
}