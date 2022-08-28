/*
  Yves Gaignard
  
  OTA Web Server management
*/

#define TAG "PM_OTA_Web_Srv"

#define ARDUINOJSON_USE_DOUBLE 1  // Required to force ArduinoJSON to treat float as double

// Standard library definitions
#include <Arduino.h>
#include <AsyncElegantOTA.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>          // JSON library

#include "PM_Pool_Manager.h"
#include "PM_Parameters.h"      // Pool manager parameters
#include "PM_OTA_Web_Srv.h"

AsyncWebServer OTAServer(80);

void PM_OTA_Web_Srv_getMeasures(AsyncWebServerRequest *request);

// Web server setup and start
void PM_OTA_Web_Srv_setup(void) {

  //----------------------------------------------------SPIFFS
  if(!SPIFFS.begin()) {
      LOG_E(TAG, "Cannot open SPIFFS....");
    return;
  }

  LOG_I(TAG, "List of SPIFFS Files:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while(file)
  {
    LOG_I(TAG, "File: %s", file.name());
    file.close();
    file = root.openNextFile();
  }

  OTAServer.on("/", HTTP_GET, PM_OTA_Web_Srv_root);
  OTAServer.on("/getMeasures", HTTP_GET, PM_OTA_Web_Srv_getMeasures);
  OTAServer.onNotFound(PM_OTA_Web_Srv_notFound);
  OTAServer.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/w3.css", "text/css");
  });

  OTAServer.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });
  

  AsyncElegantOTA.begin(&OTAServer);    // Start ElegantOTA
  OTAServer.begin();
  LOG_I(TAG, "HTTP server started");
}

// handler to treat "page not found"
void PM_OTA_Web_Srv_notFound(AsyncWebServerRequest *request) {
  int http_rc = 404;
  char Response_type[]="text/html";
  String Response;
  Response+="<!DOCTYPE html>";
  Response+="<html>";
  Response+="<head><title>Not Found</title></head>";
  Response+="<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>";
  Response+="</html>";
  request->send(http_rc, Response_type, Response);
  LOG_V(TAG, "send %d %s \nResponse: %s", http_rc, Response_type, Response);
}

// handler to treat "root URL"
void PM_OTA_Web_Srv_root(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/index.html", "text/html");
}

// handler to treat "GET Temperature"
void PM_OTA_Web_Srv_getMeasures(AsyncWebServerRequest *request) {
  const int capacity = JSON_OBJECT_SIZE(48);
  StaticJsonDocument<capacity> root;
  LOG_I(TAG, "Start API Get measuressend");
  
  root["Vers"]   = pm_measures.PMVersion;                 //firmware revision
  root["FedDur"] = pm_measures.FilteredDuration;          // Filtration Duration since the begin of the day
  root["DFDur"]  = pm_measures.DayFiltrationDuration;     // Maximum Filtration duration for the whole day
  root["FDu"]    = pm_measures.DayFiltrationDuration;     //Computed filtration duration based on water temperature (hours)
  root["FSta"]   = pm_measures.FiltrationStartTime;       // Next start time of the filtration
  root["FEnd"]   = pm_measures.FiltrationEndTime;         // Next end time of the filtration
  root["pHUTL"]  = pm_measures.pHPumpUpTimeLimit / 60;    // Time in seconds max per day for pH injection
  root["OrpUTL"] = pm_measures.OrpPumpUpTimeLimit / 60;   // Time in seconds max per day for Chlorine injection

  root["Auto"]   = pm_measures.AutoMode;   // Mode Automatic (true) or Manual (false)
  root["Winter"] = pm_measures.WinterMode; // Winter Mode if true  
  root["pHROO"]  = pm_measures.pH_RegulationOnOff;   
  root["OrpROO"] = pm_measures.Orp_RegulationOnOff;   
  root["FPmpS"]  = pm_measures.FilterPumpState;   
  root["pHPmpS"] = pm_measures.pHMinusPumpState;   
  root["OrpPmpS"]= pm_measures.ChlorinePumpState;   

  root["IATemp"] = pm_measures.InAirTemp;   
  root["WaTemp"] = pm_measures.WaterTemp;   
  root["OATemp"] = pm_measures.OutAirTemp;   
  root["WaTLT"]  = pm_measures.WaterTempLowThreshold;   

  root["pH"]     = pm_measures.pHValue;   
  root["Orp"]    = pm_measures.OrpValue;   
  root["PSI"]    = pm_measures.Pressure;   
  root["pHFill"] = pm_measures.pHMinusTankFill;   
  root["ChFill"] = pm_measures.ChlorineTankFill;   

  root["pHWS"]  = pm_measures.pHPIDWindowSize / 1000 / 60;        //pH PID window size (/!\ mins)
  root["ChlWS"] = pm_measures.OrpPIDWindowSize / 1000 / 60;       //Orp PID window size (/!\ mins)
  root["pHSP"]  = pm_measures.pH_SetPoint * 100;                  //pH setpoint (/!\ x100)
  root["OrpSP"] = pm_measures.Orp_SetPoint;                       //Orp setpoint
  //root["WSP"]   = pm_measures.WaterTemp_SetPoint * 100;           //Water temperature setpoint (/!\ x100)
  root["WLT"]   = pm_measures.WaterTempLowThreshold * 100;        //Water temperature low threshold to activate anti-freeze mode (/!\ x100)
  root["PSIHT"] = pm_measures.PressureHighThreshold * 100;            //Water pressure high threshold to trigger error (/!\ x100)
  root["PSIMT"] = pm_measures.PressureMedThreshold * 100;             //Water pressure medium threshold (unused yet) (/!\ x100)

  root["pHC0"]  = pm_measures.pHCalibCoeffs0;            //pH sensor calibration coefficient C0
  root["pHC1"]  = pm_measures.pHCalibCoeffs1;            //pH sensor calibration coefficient C1
  root["OrpC0"] = pm_measures.OrpCalibCoeffs0;           //Orp sensor calibration coefficient C0
  root["OrpC1"] = pm_measures.OrpCalibCoeffs1;           //Orp sensor calibration coefficient C1
  root["PSIC0"] = pm_measures.PSICalibCoeffs0;           //Pressure sensor calibration coefficient C0
  root["PSIC1"] = pm_measures.PSICalibCoeffs1;           //Pressure sensor calibration coefficient C1

  root["pHKp"]  = pm_measures.pH_Kp;    //pH PID coeffcicient Kp
  root["pHKi"]  = pm_measures.pH_Ki;    //pH PID coeffcicient Ki
  root["pHKd"]  = pm_measures.pH_Kd;    //pH PID coeffcicient Kd

  root["OrpKp"] = pm_measures.Orp_Kp;    //Orp PID coeffcicient Kp
  root["OrpKi"] = pm_measures.Orp_Ki;    //Orp PID coeffcicient Ki
  root["OrpKd"] = pm_measures.Orp_Kd;    //Orp PID coeffcicient Kd

  root["Dpid"]   = pm_measures.DelayPIDs;     //Delay from FSta for the water regulation/PIDs to start (mins) 
  //root["PubP"]   = pm_measures.PublishPeriod/1000; // Settings publish period in sec

  root["pHTV"]  = pm_measures.pHMinusTankVolume;           //Acid tank nominal volume (Liters)
  root["ChlTV"] = pm_measures.ChlorineTankVolume;          //Chl tank nominal volume (Liters)
  root["pHFR"]  = pm_measures.pHMinusFlowRate;            //Acid pump flow rate (L/hour)
  root["OrpFR"] = pm_measures.ChlorineFlowRate;           //Chl pump flow rate (L/hour)

  String response;
  serializeJson(root, response);  
  request->send(200, "application/json", response.c_str());
  LOG_I(TAG, "End API Get measuressend");
}

