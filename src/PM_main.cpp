/*
  Yves Gaignard
  
  Main procedure of the Pool Manager project
*/

#define TAG "PM_main"

// Standard library definitions
#include <Arduino.h>
#include <SPI.h>                   // Library for SPI management
#include <LiquidCrystal_I2C.h>     // Library for LCD management
#include <WiFi.h>                  // Library for WiFi management
#include <WiFiMulti.h>             // Library for WiFi management
#include <ESPPerfectTime.h>        // Library for time maangement
#include <ESPAsyncWebServer.h>     // Library for Web Server Management
#include <OneWire.h>               // Library for onewire devices
#include <DallasTemperature.h>     // Library for temperature sensors
#include <RTClib.h>                // Library for time management on RTC DS3231
#include <Preferences.h>           // Library for preference storage management
#include <PID_v1.h>                // Library for PID controller (Proportional–Integral–Derivative controller)
#include <time.h>                  // Library for management time

// Project definitions
#include "PM_Pool_Manager.h"       // Pool manager constant declarations
#include "PM_Log.h"                // Pool manager log management
#include "PM_Structures.h"         // Pool manager structure definitions
#include "PM_Parameters.h"         // Pool manager parameters
#include "PM_I2CScan.h"            // Pool manager I2C scan tools
#include "PM_Time_Mngt.h"          // Pool manager time management
#include "PM_Temperature.h"        // Pool manager temperature management
#include "PM_Wifi.h"               // Pool manager wifi management
#include "PM_OTA_Web_Srv.h"        // Pool manager web server management
#include "PM_LCD.h"                // Pool manager display device management
#include "PM_Config.h"             // Pool manager configuration parameters
#include "PM_Error.h"              // Pool manager error management
#include "PM_Utils.h"              // Pool manager utilities
#include "PM_Pump.h"               // Pool manager pumps management


// Intantiate the Pool Manager configuration
static PM_Config Pool_Configuration;

// To manage time
RTC_DS3231 rtc;  // RTC device handle
time_t     now;  // Current time (global variable)

// Array of I2C Devices
static byte I2CDevices[128];

// Instantiate LCD display and a screen template
PM_LCD lcd(PM_LCD_Device_Addr, PM_LCD_Cols, PM_LCD_Rows);
std::vector<std::string> screen;

// NVS Non Volatile SRAM (eqv. EEPROM)
Preferences nvs;   

// swimming pool measures
PM_SwimmingPoolMeasures     pm_measures     = { 
  now,   // time_t  Timestamp;                      // Last modification timestamp
  PM_VERSION, //  uint8_t PMVersion;                // version of the structure
  true,  // bool    AutoMode;                       // Mode Automatic (true) or Manual (false)
  false, // bool    WinterMode;                     // Winter Mode if true
  false, // bool    pH_RegulationOnOff;             
  false, // bool    Orp_RegulationOnOff;             
  false, // boolean FilterPumpState;                // State of the filtering pump (true, false)
  false, // boolean pHMinusPumpState;               // State of the pH- pump (true, false)
  false, // boolean ChlorinePumpState;              // State of the pH- pump (true, false)
  -20.0, // float   InAirTemp;                      // Inside air temperature in °C 
  -20.0, // float   WaterTemp;                      // Water temperature in °C of the swimming pool
  -20.0, // float   OutAirTemp;                     // Outside air temperature in °C
  7.4,   // double  pHValue;                        // Current pH value
  1800000, //ulong   pHPIDWindowSize;
  0,     // ulong   pHPIDwindowStartTime;           // pH PID window start time   
  2700,  // ulong   pHPumpUpTimeLimit;              // Time in seconds max per day for pH injection
  0.0,   // double  pHPIDOutput;
  7.3,   // double  pH_SetPoint;
  2700000.0, //double  pH_Kp;
  0.0,   // double  pH_Ki;
  0.0,   // double  pH_Kd;
  250.0, // double  OrpValue;                       // Current redox measure unit: mV
  1800000, //ulong   OrpPIDWindowSize;
  0,     // ulong   OrpPIDwindowStartTime;          // Orp PID window start time   
  2700,  // ulong   OrpPumpUpTimeLimit;             // Time in seconds max per day for Chlorine injection
  0.0,   // double  OrpPIDOutput;
  7.3,   // double  Orp_SetPoint;
  2700000.0, //double  Orp_Kp;
  0.0,   // double  Orp_Ki;
  0.0,   // double  Orp_Kd;
  0,     // time_t  FilteredDuration;               // Filtration Duration since the begin of the day
  0,     // time_t  DayFiltrationDuration;          // Maximum Filtration duration for the whole day
  0,     // time_t  FiltrationStartTime;            // Next start time of the filtration
  0,     // time_t  FiltrationEndTime;              // Next end time of the filtration
  PM_pH_Pump_Flow_Rate,       // float   pHMinusFlowRate;    // Flow rate of pH Minus liquid injected (liter per hour)
  PM_Chlorine_Pump_Flow_Rate, // float   ChlorineFlowRate;   // Flow rate of Chlorine liquid injected (liter per hour)
  0.0,   // float   pHMinusVolume;                  // Volume of pH Minus liquid since the last complete fill of the container
  0.0,   // float   ChlorineVolume;                 // Volume of liquid chlorine since the last complete fill of the container
  0,     // int32_t ConsumedInstantaneousPower;     // Instantaneous Power in Watt consumed by the filtration pump
  0,     // int32_t DayConsumedPower;               // Power in Watt consumed by the filtration pump since the begin of the day
  0.0,   // float   Pressure;                       // Pressure in the filtering device (unit hPa)
  PM_pH_Tank_Volume,        // float   pHMinusTankVolume;              // Max volume of the pH- tank
  PM_Chlorine_Tank_Volume,  // float   ChlorineTankVolume;             // Max volume of the Chlorine tank
  100.0, // float   pHMinusTankFill;                // % Fill of volume of the pH- tank
  100.0  // float   ChlorineTankFill;               // % Fill of volume of the Chlorine tank
}; 
PM_SwimmingPoolMeasures_str pm_measures_str = {             "0", "00", "00", "00", "7.4", "0", "0", "000", "00h00", "00h00","00.0", "00.0","00.0", "00.0", "0000", "0000", "0000", "OFF", "OFF", "OFF", "00.0", "00.0" }; 

// Instanciations of Pump and PID objects to make them global. But the constructors are then called 
// before loading of the storage struct. At run time, the attributes take the default
// values of the storage struct as they are compiled, just a few lines above, and not those which will 
// be read from NVS later. This means that the correct objects attributes must be set later in
// the setup function (fortunatelly, init methods exist).

// The four pumps of the system (instanciate the Pump class)
// In this case, all pumps start/Stop are managed by relays. pH, ORP and Robot pumps are interlocked with 
// filtration pump
PM_Pump FiltrationPump(FILTRATION_PUMP, FILTRATION_PUMP);
PM_Pump PhPump(PH_PUMP, PH_PUMP, NO_LEVEL, FILTRATION_PUMP, pm_measures.pHMinusFlowRate,  pm_measures.pHMinusTankVolume, pm_measures.pHMinusTankFill);
PM_Pump ChlPump(CHL_PUMP, CHL_PUMP, NO_LEVEL, FILTRATION_PUMP, pm_measures.ChlorineFlowRate, pm_measures.ChlorineTankVolume, pm_measures.ChlorineTankFill);

//PIDs instances
//Specify the links and initial tuning parameters
PID pHPID(&pm_measures.pHValue, &pm_measures.pHPIDOutput, &pm_measures.pH_SetPoint, pm_measures.pH_Kp, pm_measures.pH_Ki, pm_measures.pH_Kd, pHPID_DIRECTION);
PID OrpPID(&pm_measures.OrpValue, &pm_measures.OrpPIDOutput, &pm_measures.Orp_SetPoint, pm_measures.Orp_Kp, pm_measures.Orp_Ki, pm_measures.Orp_Kd, OrpPID_DIRECTION);

// To manage the connection on Wifi
boolean IsWifiConnected   = false;

// To manage wifi between multiple networks
WiFiMulti wifiMulti;

// Display LCD parameters
time_t  PM_Display_Max_Time_Without_Activity=LCD_DISPLAY_TIMEOUT;  // duration of displaying information
time_t  PM_Display_Screen_Duration=LCD_DISPLAY_SCREEN_DURATION;    // duration of a screen display before switching to the next time
time_t  PM_Display_Activation_Start=0;            // time of the last LCD activationion without any user interaction
time_t  PM_Display_Screen_Start=0;                // time of the current screen start
int     PM_Display_Current_Screen_Index=1;        // Current displayed screen index 
int     PM_Display_Screen_Number=2;               // Total screen number
boolean PM_Display_Activation_Request=true;       // Request to activate the display



// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature temperatureSensors(&oneWire);

// arrays to hold temperatur sensor device addresses
DeviceAddress insideThermometer, outsideThermometer, waterThermometer;

// Instantiate object to manage all temperature sensors
PM_Temperature PM_TemperatureSensors;

// Tasks declaration
TaskHandle_t Task_Main_Handle      = NULL;
TaskHandle_t Task_LCD_Handle       = NULL;
TaskHandle_t Task_WebServer_Handle = NULL;
TaskHandle_t Task_GPIO_Handle      = NULL;

void PM_Task_Main      ( void *pvParameters );
void PM_Task_LCD       ( void *pvParameters );
void PM_Task_WebServer ( void *pvParameters );
void PM_Task_GPIO      ( void *pvParameters );

// function declarations
void PM_getBoardInfo();
void PM_Time_Init();
void PM_Display_init    ();
void PM_Display_screen_0(PM_SwimmingPoolMeasures_str & measures);
void PM_Display_screen_1(PM_SwimmingPoolMeasures_str & measures);
bool PM_NVS_Init();
bool PM_NVS_Load();
bool PM_NVS_Save();
void PM_Temperature_Init();
void PM_SetpHPID(bool Enable);
void PM_SetOrpPID(bool Enable);
void PM_CalculateNextFiltrationPeriods();
void PM_StartFiltrationPump();
void PM_StopFiltrationPump();

// Button declarations
boolean PM_DisplayButton_State = false;    // Current State
int     PM_DisplayButton_LastPressed = 0;  // last time it was pressed in millis
void IRAM_ATTR PM_DisplayButton_ISR();     // Interuption function

// =================================================================================================
//
//                                        SETUP OF POOL MANAGER
//
// =================================================================================================
void setup() {

  //Init serial for logs
  Serial.begin(115200);

  // Set appropriate log level. The defaul LOG_LEVEL is defined in PoolMaster.h
  Log.setTag("*"             , LOG_LEVEL);
  Log.setTag("PM_main"       , LOG_VERBOSE);
  Log.setTag("PM_I2CScan"    , LOG_LEVEL);
  Log.setTag("PM_Log"        , LOG_LEVEL);
  Log.setTag("PM_OTA_Web_Srv", LOG_LEVEL);
  Log.setTag("PM_Config"     , LOG_VERBOSE);
  Log.setTag("PM_Temperature", LOG_LEVEL);
  Log.setTag("PM_Time_Mngt"  , LOG_LEVEL);
  Log.setTag("PM_Wifi"       , LOG_LEVEL);

  // Log.formatTimestampOff(); // time in milliseconds (if necessary)
  LOG_I(TAG, "Starting Project: [%s]  Version: [%s]",Project.Name.c_str(), Project.Version.c_str());

  // Print the information of the board
  PM_getBoardInfo();
  
  //Init LCD
  PM_Display_init();
  
  // Discover all I2C Devices
  int I2CDeviceNumber=0;
  I2CDeviceNumber = PM_I2CScan_Scan(I2CDevices);
  // Print the I2C Devices
  PM_I2CScan_Print(I2CDeviceNumber, I2CDevices);

  // Connect to the strengthest known wifi network
  while ( ! IsWifiConnected){
    IsWifiConnected=PM_Wifi_DetectAndConnect (wifiMulti);
    delay(100);
  }

  // Time initialization
  PM_Time_Init();

  // Get current time
  DateTime Now(now);
  char LocalTimeFormat[sizeof(PM_LocalTimeFormat)];
  strcpy(LocalTimeFormat, PM_LocalTimeFormat);
  LOG_D(TAG, "Current date and local time is: %s", Now.toString(LocalTimeFormat));
  /*
  char timestamp_str[20];
  tm* time_tm = localtime(&now);
	strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
  LOG_D(TAG, "Current date and local time is: %s", timestamp_str);
  */

  // Start of diaplaying information
  PM_Display_Activation_Start=now;
  
    // For DEBUG , remove all keys
  // nvs.begin(Project.Name.c_str(),false);
  // if ( ! nvs.clear() ) LOG_E(TAG, "Cannot clear the NVS namespace: %s", Project.Name.c_str());
  // nvs.end();
 
  // Initialize NVS data 
  if ( ! PM_NVS_Init()) LOG_I(TAG, "Error on NVS initialization phase. See traces");

  // start Web Server
  PM_OTA_Web_Srv_setup();
  
  // Verify the configuration of pool manager
  PM_Error Error = Pool_Configuration.CheckFiltrationTimeAbaqus();
  int ErrorNumber = Error.getErrorNumber();
  if (ErrorNumber != 0) {
    LOG_E(TAG, "The Filtration Time Abaqus does not respect the rules. Please check it !");
    std::string Display_ErrorNumber = "Error: "+Error.getErrorNumberStr();
    std::string Display_ErrorMessage = Error.getDisplayMsg();
    LOG_E(TAG, "%s",Display_ErrorMessage.c_str());
    lcd.clear();
    lcd.printLine(0, Display_ErrorNumber);
    lcd.printScrollLine(1, Display_ErrorMessage, 60);
    for ( ;; ) {} //infinite loop as the configuration could not be wrong
  }

  Error = Pool_Configuration.CheckFiltrationPeriodAbaqus();
  if (Error.getErrorNumber() != 0) {
    LOG_E(TAG, "The Filtration Period Abaqus does not respect the rules. Please check it !");
    std::string Display_ErrorNumber = "Error: " + Error.getErrorNumberStr();
    std::string Display_ErrorMessage = Error.getDisplayMsg();
    LOG_E(TAG, "%s",Display_ErrorMessage.c_str());
    lcd.clear();
    lcd.printLine(0, Display_ErrorNumber);
    lcd.printScrollLine(1, Display_ErrorMessage, 60);
    for ( ;; ) {} //infinite loop as the configuration could not be wrong
  }
  
   // Display the first screen
  PM_Display_screen_0(pm_measures_str);
  PM_Display_Screen_Start=now;

  // Attribute the GPIOs
  pinMode(PM_DisplayButton_Pin, INPUT_PULLUP);
  attachInterrupt(PM_DisplayButton_Pin, PM_DisplayButton_ISR, FALLING);
  
  // Declare temperature sensors
  PM_Temperature_Init();

  // Initialize PIDs
  pm_measures.pHPIDwindowStartTime  = millis();
  pm_measures.OrpPIDwindowStartTime = millis();

  // Limit the PIDs output range in order to limit max. pumps runtime (safety first...)
  pHPID.SetTunings(pm_measures.pH_Kp, pm_measures.pH_Ki, pm_measures.pH_Kd);
  pHPID.SetControllerDirection(pHPID_DIRECTION);
  pHPID.SetSampleTime((int)pm_measures.pHPIDWindowSize);
  pHPID.SetOutputLimits(0, pm_measures.pHPIDWindowSize);    //Whatever happens, don't allow continuous injection of Acid for more than a PID Window

  OrpPID.SetTunings(pm_measures.Orp_Kp, pm_measures.Orp_Ki, pm_measures.Orp_Kd);
  OrpPID.SetControllerDirection(OrpPID_DIRECTION);
  OrpPID.SetSampleTime((int)pm_measures.OrpPIDWindowSize);
  OrpPID.SetOutputLimits(0, pm_measures.OrpPIDWindowSize);  //Whatever happens, don't allow continuous injection of Chl for more than a PID Window

 // PIDs off at start
  PM_SetpHPID (false);
  PM_SetOrpPID(false);

  //Initialize pump instances with stored config data
  FiltrationPump.SetMaxUpTime(0);     //no runtime limit for the filtration pump

  PhPump.SetFlowRate(pm_measures.pHMinusFlowRate);
  PhPump.SetTankVolume(pm_measures.pHMinusTankVolume);
  PhPump.SetTankFill(pm_measures.pHMinusTankFill);
  PhPump.SetMaxUpTime(pm_measures.pHPumpUpTimeLimit * 1000);

  ChlPump.SetFlowRate(pm_measures.ChlorineFlowRate);
  ChlPump.SetTankVolume(pm_measures.ChlorineTankVolume);
  ChlPump.SetTankFill(pm_measures.ChlorineTankFill);
  ChlPump.SetMaxUpTime(pm_measures.OrpPumpUpTimeLimit * 1000);

  // Start filtration pump at power-on if within scheduled time slots -- You can choose not to do this and start pump manually
  PM_CalculateNextFiltrationPeriods();
  if (pm_measures.AutoMode && (now >= pm_measures.FiltrationStartTime) && (now < pm_measures.FiltrationEndTime)) {
    PM_StartFiltrationPump();
    LOG_I(TAG, "Start filtration pump");
  }
  else {
    PM_StopFiltrationPump();
    LOG_I(TAG, "Stop filtration pump");
  } 

  // Create tasks
  //                          Function           Name          Stack  Param PRIO  Handle                core
  //xTaskCreatePinnedToCore(PM_Task_Main,      "Task_Main",      10000, NULL, 10, &Task_Main_Handle,      0);
  xTaskCreatePinnedToCore(PM_Task_GPIO,      "Task_GPIO",      10000, NULL,  8, &Task_GPIO_Handle,      0);
  xTaskCreatePinnedToCore(PM_Task_LCD,       "Task_LCD",       10000, NULL,  6, &Task_LCD_Handle,       1);
  xTaskCreatePinnedToCore(PM_Task_WebServer, "Task_WebServer", 10000, NULL,  9, &Task_WebServer_Handle, 1);

  // Infinite loop to never go to main loop. Everything is treated in a task 
  for( ;; ) {}
}
// =================================================================================================
//                                   LOOP OF POOL MANAGER
// =================================================================================================
void loop(void) {
   // Nothing here
}

// =================================================================================================
//                                  MAIN TASK OF POOL MANAGER
// =================================================================================================
void PM_Task_Main      ( void *pvParameters ) {
  //const char *pcTaskName = "Task_Main";
  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20]; 
  for( ;; ) {
    time(&now);
    time_tm = localtime(&now);
	  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    LOG_D(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);

    vTaskDelay( pdMS_TO_TICKS( 50000 ) );
  }
}
// =================================================================================================
//                               LCD MANAGEMENT TASK OF POOL MANAGER
// =================================================================================================
void PM_Task_LCD       ( void *pvParameters ) {
  //const char *pcTaskName = "Task_LCD";
  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20];
  for( ;; ) {
    time(&now);
    time_tm = localtime(&now);
	  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    LOG_D(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);

    // if lcd display button is pressed then set the display ON in case of OFF
    if ( PM_Display_Activation_Request == true) {
      if (lcd.getDisplayState() == false ) {  
        // the LCD is OFF. We just set it ON again
        lcd.display();
        LOG_V(TAG, "%s : Display is shown",timestamp_str);
        // reset the display duration counter
        PM_Display_Activation_Start=now;
      }
      PM_Display_Activation_Request=false;
    }

    // If the display is ON
    if (lcd.getDisplayState() == true) {
      // if the timout of the current screen is reached
      if ( (now - PM_Display_Screen_Start) >= PM_Display_Screen_Duration) { 
        // display the next screen
        int screen_index= (PM_Display_Current_Screen_Index+1)%PM_Display_Screen_Number;
        LOG_V(TAG, "%s : Screen index %d",timestamp_str, screen_index);
        switch (screen_index) {
          case 0 : PM_Display_screen_0(pm_measures_str);
              PM_Display_Screen_Start=now;
              LOG_D(TAG, "%s : Display screen %d",timestamp_str, screen_index);
            break;
          case 1 : PM_Display_screen_1(pm_measures_str);
              PM_Display_Screen_Start=now;
              LOG_D(TAG, "%s : Display screen %d",timestamp_str, screen_index);
            break;
          default:
            LOG_E(TAG, "%s : Cannot Display screen %d",timestamp_str, screen_index);
        }
        PM_Display_Current_Screen_Index=screen_index;
      } 

      // if no_activity at all during MAX_WITHOUT_ACTIVITES then stop the display
      if (now - PM_Display_Activation_Start >= PM_Display_Max_Time_Without_Activity ) {
        lcd.noDisplay();
        lcd.noBacklight();
        LOG_D(TAG, "%s : Display is stopped",timestamp_str);
      }
    }
    vTaskDelay( pdMS_TO_TICKS( 5000 ) );
  }
}
// =================================================================================================
//                                     WEB SERVER MANAGEMENT TASK OF POOL MANAGER
// =================================================================================================
void PM_Task_WebServer ( void *pvParameters ) {
  //const char *pcTaskName = "Task_WebServer";
  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20];
  for( ;; ) {
    time(&now);
    time_tm = localtime(&now);
	  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    LOG_D(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);

    vTaskDelay( pdMS_TO_TICKS( 3000 ) );
  }
}
// =================================================================================================
//                                SENSOR MANAGEMENT TASK OF POOL MANAGER
// =================================================================================================
void PM_Task_GPIO      ( void *pvParameters ) {
  //const char *pcTaskName = "Task_GPIO";
  UBaseType_t uxPriority;
  uxPriority = uxTaskPriorityGet( NULL );
  tm * time_tm;
  char timestamp_str[20];

  std::string deviceName;
  float preciseTemperatureC;
  int   temperatureC;

  for( ;; ) {
    time(&now);
    time_tm = localtime(&now);
	  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    LOG_D(TAG, "%s : core = %d (priorite %d)",timestamp_str, xPortGetCoreID(), uxPriority);

    char timestamp_str[20];
    tm* time_tm = localtime(&now);
	  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
    LOG_I(TAG, "Current date and local time is: %s", timestamp_str);

    PM_TemperatureSensors.requestTemperatures();
    for (int i = 0 ; i < PM_TemperatureSensors.getDeviceCount(); i++) {
      deviceName = PM_TemperatureSensors.getDeviceNameByIndex(i);

      preciseTemperatureC = PM_TemperatureSensors.getPreciseTempCByName(deviceName);
      LOG_D(TAG, "Sensor: %19s : %f°C", deviceName.c_str(),preciseTemperatureC);
      temperatureC = PM_TemperatureSensors.getTempCByName(deviceName);
      LOG_I(TAG, "Sensor: %19s : %d°C", deviceName.c_str(),temperatureC);

      if (deviceName == insideThermometerName) {
        pm_measures.InAirTemp = preciseTemperatureC;
        pm_measures_str.InAirTemp_str = PM_itoa(temperatureC);
      }
      else if (deviceName == outsideThermometerName){
        pm_measures.OutAirTemp = preciseTemperatureC;
        pm_measures_str.OutAirTemp_str = PM_itoa(temperatureC);
      } 
      else if (deviceName == waterThermometerName) {
        pm_measures.WaterTemp = preciseTemperatureC;
        pm_measures_str.WaterTemp_str = PM_itoa(temperatureC);
      }
    }

    vTaskDelay( pdMS_TO_TICKS( 15000 ) );
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
// =================================================================================================
//                              SCREEN DEFINITIONS
// =================================================================================================
/*  
   |--------------------|
   |         1         2|
   |12345678901234567890|
   |--------------------|
   |     SCREEN INIT    |
   |--------------------|
   |Pool Manager        |
   |Version: 1.0.0      |
   |                    | 
   |Yves Gaignard       |
   |--------------------|
*/ 
void PM_Display_init    () {
  lcd.init();
  lcd.clear();
  lcd.display();
  lcd.backlight();

  screen.clear();
  screen.push_back(Project.Name);
  screen.push_back("Version: " + Project.Version);
  screen.push_back("");
  screen.push_back(Project.Author);

  lcd.printScreen(screen);
}
/*  
   |--------------------|
   |         1         2|
   |12345678901234567890|
   |--------------------|
   |     SCREEN 1       |
   |--------------------|
   |In:28 W:26 Out:15   |
   |ph:7.2 Cl:150 <460  | or |ph:7.2 Cl:150 >600  | 
   |Filter: 12h15 / 16h | 
   |pH-:12.6l Cl:15.2l  |
   |--------------------|
*/  
void PM_Display_screen_0(PM_SwimmingPoolMeasures_str & measures) {
  std::string DisplayLine;
  lcd.clear();
  lcd.display();
  lcd.backlight();
  char degreeAsciiChar[2];
  sprintf(degreeAsciiChar, "%c", 176);
  screen[0] = "In:"+measures.InAirTemp_str+degreeAsciiChar+" W:"+measures.WaterTemp_str+degreeAsciiChar+" Out:"+measures.OutAirTemp_str+degreeAsciiChar;
  DisplayLine = "PH:"+measures.pH_str+" Cl:"+measures.Chlorine_str;
  if (measures.Chlorine_str < measures.ChlorineMin_str) DisplayLine+=" <"+measures.ChlorineMin_str;
  if (measures.Chlorine_str > measures.ChlorineMax_str) DisplayLine+=" >"+measures.ChlorineMax_str;
  screen[1] = DisplayLine;
  screen[2] = "Filter:"+measures.DayFilterTime_str+" / "+measures.MaxDayFilterTime_str;
  screen[3] = "PH-:"+measures.pHMinusVolume_str+" Cl:"+measures.ChlorineVolume_str;

  lcd.printScreen(screen);
}
/*
   |--------------------|
   |         1         2|
   |12345678901234567890|
   |--------------------|
   |     SCREEN 2       |
   |--------------------|
   |Filter:OFF P:1015hPa| or |Filter:ON  P:1015hPa|
   |Pow:1200W Day:4.2kWh|
   |Pump pH-:OFF Cl:OFF | or |Pump pH-:ON  Cl:ON  |
   |Max pH-:20l Cl:20l  |
   |--------------------|
*/
void PM_Display_screen_1(PM_SwimmingPoolMeasures_str & measures) {
  lcd.clear();
  lcd.display();
  lcd.backlight();

  screen[0] = "Filter:"+measures.FilterPumpState_str+" P:"+measures.Pressure_str+"hPa";
  screen[1] = "Pow:"+measures.ConsumedInstantaneousPower_str+" Day:"+measures.Chlorine_str+"kWh";
  screen[2] = "Pump PH-:"+measures.pHMinusPumpState_str+" Cl:"+measures.ChlorinePumpState;
  screen[3] = "Max PH-:"+measures.pHMinusMaxVolume_str+" Cl:"+measures.ChlorineMaxVolume_str;

  lcd.printScreen(screen);
}
// =================================================================================================
//                              BOARD INFO
// =================================================================================================
void PM_getBoardInfo(){
  esp_chip_info_t out_info;
  esp_chip_info(&out_info);
  LOG_I(TAG,"Board info");
  LOG_I(TAG,"CPU frequency       : %dMHz",ESP.getCpuFreqMHz());
  LOG_I(TAG,"CPU Cores           : %d",out_info.cores);
  LOG_I(TAG,"Flash size          : %dMB",ESP.getFlashChipSize()/1000000);
  LOG_I(TAG,"Free RAM            : %d bytes",ESP.getFreeHeap());
  LOG_I(TAG,"Min heap            : %d bytes",esp_get_free_heap_size());
  LOG_I(TAG,"tskIDLE_PRIORITY    : %d",tskIDLE_PRIORITY);
  LOG_I(TAG,"confixMAX_PRIORITIES: %d",configMAX_PRIORITIES);
  LOG_I(TAG,"configTICK_RATE_HZ  : %d",configTICK_RATE_HZ);
}
// =================================================================================================
//                              TIME INITIALIZATION
// =================================================================================================
void PM_Time_Init() {

  // Initialize the time
  // ------------------------
  boolean isRTCFound     = true;
  boolean isRTCLostPower = true;
  DateTime DT_now;
  std::string DT_now_str;

  // check if a RTC module is connected
  if (! rtc.begin()) {
    LOG_E(TAG, "Cannot find any RTC device. Time will be initialized through a NTP server");
    isRTCFound = false;
  } else {
    isRTCLostPower=rtc.lostPower();   
  }

  // Initialize time from NTP server
   PM_Time_Mngt_initialize_time();
  
  // If there is no RTC module or if it lost its power, set the time with the NTP time
  if (isRTCLostPower == true || isRTCFound == true) {
    LOG_I(TAG, "Initialize RTC time with NTP server");

    // Get current time
    time(&now);
    // adjust time of rtc with the time get from NTP server
    DT_now = DateTime(now);
    char DT_now_format[20]= "YYYY-MM-DD hh:mm:ss";
    DT_now_str = DT_now.toString(DT_now_format);
    LOG_I(TAG, "Adjust the time of RTC with the NTP time: %s", DT_now_str.c_str() );
    rtc.adjust(DT_now);
  }
  
  // if there is a RTC module, set the time with RTC time
  if (isRTCFound == true) {
    // set time with the RTC time
    DT_now = rtc.now();
    char DT_now_format[20]= "YYYY-MM-DD hh:mm:ss";
    DT_now_str = DT_now.toString(DT_now_format);
    LOG_I(TAG, "Get the time from RTC: %s", DT_now_str.c_str() );
    // set the time
    now = DT_now.unixtime();
    LOG_I(TAG, "rtc.now = %u", now );
    timeval tv = {now, 0}; 
    timezone tz = {0,0} ;
    int ret = settimeofday(&tv, &tz);
    if ( ret != 0 ) {LOG_E(TAG, "Cannot set time from RTC" ); };
  }
}
// =================================================================================================
//                           MEASURES INITIALIZATION FROM STORAGE
// =================================================================================================
bool PM_NVS_Init() {
  bool rc = false;
  
  //Read ConfigVersion. If does not match expected value, restore default values
  if(nvs.begin(Project.Name.c_str(),true))
  {
    uint8_t vers = nvs.getUChar("PMVersion",0);
    LOG_I(TAG, "Stored version: %d",vers);

    nvs.end();

    if (vers == PM_VERSION)
    {
      LOG_I(TAG, "Same version: %d / %d. Loading settings from NVS",vers,PM_VERSION);
      if(PM_NVS_Load()) {
        rc = true; 
        LOG_I(TAG, "Data from NVS loaded"); //Restore stored values from NVS
      }
    }
    else
    {
      LOG_I(TAG, "New version: %d / %d. Store new default settings",vers,PM_VERSION);
      if(PM_NVS_Save()) {
        rc = true; 
        LOG_I(TAG, "Default settings stored in NVS");  //First time use. Save new default values to NVS
      }
    }

  } else {
    LOG_E(TAG, "NVS error");
    nvs.end();
    LOG_I(TAG, "Version: %d. First saving of settings",PM_VERSION);
    if(PM_NVS_Save()) {
      rc = true; 
      LOG_I(TAG, "Default settings stored in NVS");  //First time use. Save new default values to NVS
    }
  }  

  return rc; 
}
// =================================================================================================
//                           LOAD MEASURES FROM STORAGE
// =================================================================================================
bool PM_NVS_Load() {
  nvs.begin(Project.Name.c_str(),true);

  // Beware : the key maximum length is only 15 characters
  
  pm_measures.PMVersion                  = nvs.getUChar ("PMVersion"      ,0);
  pm_measures.Timestamp                  = nvs.getULong ("Timestamp"      ,0);
  pm_measures.AutoMode                   = nvs.getBool  ("AutoMode"       ,true);
  pm_measures.WinterMode                 = nvs.getBool  ("WinterMode"     ,false);
  pm_measures.InAirTemp                  = nvs.getFloat ("InAirTemp"      ,0.0);
  pm_measures.WaterTemp                  = nvs.getFloat ("WaterTemp"      ,0.0);
  pm_measures.OutAirTemp                 = nvs.getFloat ("OutAirTemp"     ,0.0);
  pm_measures.pH_RegulationOnOff         = nvs.getBool  ("pHRegulationOn" ,false);
  pm_measures.pHValue                    = nvs.getDouble("pHValue"        ,0.0);
  pm_measures.pHPIDWindowSize            = nvs.getULong ("pHPIDWindowSize",0);
  pm_measures.pHPIDwindowStartTime       = nvs.getULong ("pHPIDStartTime" ,0);
  pm_measures.pHPumpUpTimeLimit          = nvs.getULong ("pHPumpUpTimeL"  ,0);
  pm_measures.pHPIDOutput                = nvs.getDouble("pHPIDOutput"    ,0.0);
  pm_measures.pH_SetPoint                = nvs.getDouble("pH_SetPoint"    ,0.0);
  pm_measures.pH_Kp                      = nvs.getDouble("pH_Kp"          ,0.0);
  pm_measures.pH_Ki                      = nvs.getDouble("pH_Ki"          ,0.0);
  pm_measures.pH_Kd                      = nvs.getDouble("pH_Kd"          ,0.0);
  pm_measures.Orp_RegulationOnOff        = nvs.getBool  ("OrpRegulationOn",false);
  pm_measures.OrpValue                   = nvs.getDouble("OrpValue"       ,0);
  pm_measures.OrpPIDWindowSize           = nvs.getULong ("OrpPIDWindowSiz",0);
  pm_measures.OrpPIDwindowStartTime      = nvs.getULong ("OrpPIDStartTime",0);
  pm_measures.OrpPumpUpTimeLimit         = nvs.getULong ("OrpPumpUpTimeL" ,0);
  pm_measures.OrpPIDOutput               = nvs.getDouble("OrpPIDOutput"   ,0.0);
  pm_measures.Orp_SetPoint               = nvs.getDouble("Orp_SetPoint"   ,0.0);
  pm_measures.Orp_Kp                     = nvs.getDouble("Orp_Kp"         ,0.0);
  pm_measures.Orp_Ki                     = nvs.getDouble("Orp_Ki"         ,0.0);
  pm_measures.Orp_Kd                     = nvs.getDouble("Orp_Kd"         ,0.0);
  pm_measures.FilteredDuration           = nvs.getULong ("FiltDuration"   ,0);
  pm_measures.DayFiltrationDuration      = nvs.getULong ("DayFiltDuration",0);
  pm_measures.FiltrationStartTime        = nvs.getULong ("FiltraStartTime",0);
  pm_measures.FiltrationEndTime          = nvs.getULong ("FiltraEndTime"  ,0);
  pm_measures.pHMinusFlowRate            = nvs.getFloat ("pHMinusFlowRate",0.0);
  pm_measures.ChlorineFlowRate           = nvs.getFloat ("ChlorinFlowRate",0.0);
  pm_measures.pHMinusVolume              = nvs.getFloat ("pHMinusVolume"  ,0.0);
  pm_measures.ChlorineVolume             = nvs.getFloat ("ChlorineVolume" ,0.0);
  pm_measures.ConsumedInstantaneousPower = nvs.getUInt  ("InstantConsPwr" ,0);
  pm_measures.DayConsumedPower           = nvs.getUInt  ("DayConsumedPwr" ,0);
  pm_measures.Pressure                   = nvs.getFloat ("Pressure"       ,0.0);
  pm_measures.FilterPumpState            = nvs.getBool  ("FilterPumpOn"   ,false);
  pm_measures.pHMinusPumpState           = nvs.getBool  ("pHMinusPumpOn"  ,false);
  pm_measures.ChlorinePumpState          = nvs.getBool  ("ChlorinePumpOn" ,false);
  pm_measures.pHMinusTankVolume          = nvs.getFloat ("pHMinusTankVol" ,0.0);
  pm_measures.ChlorineTankVolume         = nvs.getFloat ("ChlorineTankVol",0.0);
  pm_measures.pHMinusTankFill            = nvs.getFloat ("pHMinusTankFill",0.0);
  pm_measures.ChlorineTankFill           = nvs.getFloat ("ChlorinTankFill",0.0);

  nvs.end();

  LOG_D(TAG, "%d", pm_measures.PMVersion);
  LOG_D(TAG, "%d", pm_measures.Timestamp);
  LOG_D(TAG, "%d, %d, %d, %d", pm_measures.AutoMode, pm_measures.WinterMode, pm_measures.pH_RegulationOnOff, pm_measures.Orp_RegulationOnOff);
  LOG_D(TAG, "%2.2f, %2.2f, %2.2f", pm_measures.InAirTemp, pm_measures.WaterTemp,pm_measures.OutAirTemp);
  LOG_D(TAG, "%2.2f, %4.0f", pm_measures.pHValue, pm_measures.OrpValue);
  LOG_D(TAG, "%d, %d", pm_measures.pHPIDWindowSize, pm_measures.OrpPIDWindowSize);
  LOG_D(TAG, "%d, %d", pm_measures.pHPIDwindowStartTime, pm_measures.OrpPIDwindowStartTime);
  LOG_D(TAG, "%d, %d", pm_measures.pHPumpUpTimeLimit, pm_measures.OrpPumpUpTimeLimit);
  LOG_D(TAG, "%4.0f, %4.0f, %8.0f, %3.0f, %3.0f", pm_measures.pHPIDOutput, pm_measures.pH_SetPoint, pm_measures.pH_Kp, pm_measures.pH_Ki, pm_measures.pH_Kd);
  LOG_D(TAG, "%4.0f, %4.0f, %8.0f, %3.0f, %3.0f", pm_measures.OrpPIDOutput, pm_measures.Orp_SetPoint, pm_measures.Orp_Kp, pm_measures.Orp_Ki, pm_measures.Orp_Kd);
  LOG_D(TAG, "%d, %d", pm_measures.FilteredDuration, pm_measures.DayFiltrationDuration);
  LOG_D(TAG, "%d, %d", pm_measures.FiltrationStartTime, pm_measures.FiltrationEndTime);
  LOG_D(TAG, "%2.2f, %2.2f", pm_measures.pHMinusFlowRate, pm_measures.ChlorineFlowRate);
  LOG_D(TAG, "%2.2f, %2.2f", pm_measures.pHMinusVolume, pm_measures.ChlorineVolume);
  LOG_D(TAG, "%d, %d", pm_measures.ConsumedInstantaneousPower, pm_measures.DayConsumedPower);
  LOG_D(TAG, "%2.2f", pm_measures.Pressure);
  LOG_D(TAG, "%d, %d, %d", pm_measures.FilterPumpState, pm_measures.pHMinusPumpState, pm_measures.ChlorinePumpState);
  LOG_D(TAG, "%2.2f, %2.2f", pm_measures.pHMinusTankVolume, pm_measures.ChlorineTankVolume);
  LOG_D(TAG, "%2.2f, %2.2f", pm_measures.pHMinusTankFill, pm_measures.ChlorineTankFill);

  return (pm_measures.PMVersion == PM_VERSION);
}
// =================================================================================================
//                           SAVE MEASURES TO STORAGE
// =================================================================================================
bool PM_NVS_Save() {

  localtime(&now);
  nvs.begin(Project.Name.c_str(),false);

  // Beware : the key maximum length is only 15 characters

  size_t i = 
       nvs.putUChar ("PMVersion",       pm_measures.PMVersion);
  i += nvs.putULong ("Timestamp",       now);
  i += nvs.putBool  ("AutoMode",        pm_measures.AutoMode);
  i += nvs.putBool  ("WinterMode",      pm_measures.WinterMode);
  i += nvs.putFloat ("InAirTemp",       pm_measures.InAirTemp);
  i += nvs.putFloat ("WaterTemp",       pm_measures.WaterTemp);
  i += nvs.putFloat ("OutAirTemp",      pm_measures.OutAirTemp);
  i += nvs.putBool  ("pHRegulationOn",  pm_measures.pH_RegulationOnOff);
  i += nvs.putDouble("pHValue",         pm_measures.pHValue);
  i += nvs.putULong ("pHPIDWindowSize", pm_measures.pHPIDWindowSize);
  i += nvs.putULong ("pHPIDStartTime",  pm_measures.pHPIDwindowStartTime);
  i += nvs.putULong ("pHPumpUpTimeL",   pm_measures.pHPumpUpTimeLimit);
  i += nvs.putDouble("pHPIDOutput",     pm_measures.pHPIDOutput);
  i += nvs.putDouble("pH_SetPoint",     pm_measures.pH_SetPoint);
  i += nvs.putDouble("pH_Kp",           pm_measures.pH_Kp);
  i += nvs.putDouble("pH_Ki",           pm_measures.pH_Ki);
  i += nvs.putDouble("pH_Kd",           pm_measures.pH_Kd);
  i += nvs.putBool  ("OrpRegulationOn", pm_measures.Orp_RegulationOnOff);
  i += nvs.putDouble("OrpValue",        pm_measures.OrpValue);
  i += nvs.putULong ("OrpPIDWindowSiz", pm_measures.OrpPIDWindowSize);
  i += nvs.putULong ("OrpPIDStartTime", pm_measures.OrpPIDwindowStartTime);
  i += nvs.putULong ("OrpPumpUpTimeL",  pm_measures.OrpPumpUpTimeLimit);
  i += nvs.putDouble("OrpPIDOutput",    pm_measures.OrpPIDOutput);
  i += nvs.putDouble("Orp_SetPoint",    pm_measures.Orp_SetPoint);
  i += nvs.putDouble("Orp_Kp",          pm_measures.Orp_Kp);
  i += nvs.putDouble("Orp_Ki",          pm_measures.Orp_Ki);
  i += nvs.putDouble("Orp_Kd",          pm_measures.Orp_Kd);
  i += nvs.putULong ("FiltDuration",    pm_measures.FilteredDuration);
  i += nvs.putULong ("DayFiltDuration", pm_measures.DayFiltrationDuration);
  i += nvs.putULong ("FiltraStartTime", pm_measures.FiltrationStartTime);
  i += nvs.putULong ("FiltraEndTime",   pm_measures.FiltrationEndTime);
  i += nvs.putFloat ("pHMinusFlowRate", pm_measures.pHMinusFlowRate);
  i += nvs.putFloat ("ChlorinFlowRate", pm_measures.ChlorineFlowRate);
  i += nvs.putFloat ("pHMinusVolume",   pm_measures.pHMinusVolume);
  i += nvs.putFloat ("ChlorineVolume",  pm_measures.ChlorineVolume);
  i += nvs.putUInt  ("InstantConsPwr",  pm_measures.ConsumedInstantaneousPower);
  i += nvs.putUInt  ("DayConsumedPwr",  pm_measures.DayConsumedPower);
  i += nvs.putFloat ("Pressure",        pm_measures.Pressure);
  i += nvs.putBool  ("FilterPumpOn",    pm_measures.FilterPumpState);
  i += nvs.putBool  ("pHMinusPumpOn",   pm_measures.pHMinusPumpState);
  i += nvs.putBool  ("ChlorinePumpOn",  pm_measures.ChlorinePumpState);
  i += nvs.putFloat ("pHMinusTankVol",  pm_measures.pHMinusTankVolume);
  i += nvs.putFloat ("ChlorineTankVol", pm_measures.ChlorineTankVolume);
  i += nvs.putFloat ("pHMinusTankFill", pm_measures.pHMinusTankFill);
  i += nvs.putFloat ("ChlorinTankFill", pm_measures.ChlorineTankFill);

  nvs.end();

  LOG_D(TAG, "Bytes stored in NVS: %d / %d", i, sizeof(pm_measures));

  return (i == sizeof(pm_measures));
}
// =================================================================================================
//                           INITIALIZE TEMPERATURE SENSORS
// =================================================================================================
void PM_Temperature_Init() {
  
  // Declare temperature sensors
  PM_TemperatureSensors.init(temperatureSensors);
  int tempSensorsNumber = PM_TemperatureSensors.getDeviceCount();
  LOG_I(TAG, "%d temperature sensors found",tempSensorsNumber);

  for (int i = 0; i< tempSensorsNumber; i++){
    std::string deviceAddrStr = PM_TemperatureSensors.getDeviceAddress(i);
    LOG_I(TAG, "sensor address [%d] : %s",i , deviceAddrStr.c_str() );
    if ( deviceAddrStr == insideThermometerAddress ) {
      PM_TemperatureSensors.addDevice(insideThermometerName, insideThermometerAddress);
    } else if (deviceAddrStr == outsideThermometerAddress) {
      PM_TemperatureSensors.addDevice(outsideThermometerName, outsideThermometerAddress);
    } else if (deviceAddrStr == waterThermometerAddress) {
      PM_TemperatureSensors.addDevice(waterThermometerName, waterThermometerAddress);
    } else {
      LOG_I(TAG, "Unknown temperature sensor found. Its address is: %s",deviceAddrStr.c_str());
    }
  }
}

// =================================================================================================
//                           INITIALIZE pH PID
// =================================================================================================
//Enable/Disable Chl PID
void PM_SetpHPID(bool Enable)
{
  if (Enable)
  {
    //Start pHPID
    PhPump.ClearErrors();
    pm_measures.pHPIDOutput = 0.0;
    pm_measures.pHPIDwindowStartTime = millis();
    pHPID.SetMode(AUTOMATIC);
    pm_measures.pH_RegulationOnOff = 1;
  }
  else
  {
    //Stop pHPID
    pHPID.SetMode(MANUAL);
    pm_measures.pH_RegulationOnOff = 0;
    pm_measures.pHPIDOutput = 0.0;
    PhPump.Stop();
  }
}
// =================================================================================================
//                           INITIALIZE Orp PID
// =================================================================================================
//Enable/Disable Orp PID
void PM_SetOrpPID(bool Enable)
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
// =================================================================================================
//                         CALCULATE FILTRATION PERIODS OF THE DAY
// =================================================================================================
void PM_CalculateNextFiltrationPeriods() {

  if (pm_measures.DayFiltrationDuration == 0 ) {
    // calculate the filtration duration in seconds depending on the water temperature
    pm_measures.DayFiltrationDuration = Pool_Configuration.GetFiltrationDuration(pm_measures.WaterTemp);
  }
  LOG_D(TAG, "Water temperature: %6.2f", pm_measures.WaterTemp);
  tm tm_duration = PM_Time_Mngt_convertSecondsToTm(pm_measures.DayFiltrationDuration);
  LOG_D(TAG, "Filtration duration for this day: %02d:%02d:%02d (%ds)", tm_duration.tm_hour, tm_duration.tm_min, tm_duration.tm_sec, pm_measures.DayFiltrationDuration);
  
  if (pm_measures.FilteredDuration <= pm_measures.DayFiltrationDuration) {
    Pool_Configuration.NextFiltrationPeriod (pm_measures.FiltrationStartTime, pm_measures.FiltrationEndTime, pm_measures.FilteredDuration, pm_measures.DayFiltrationDuration);
  }

  LOG_D(TAG, "Next Filtration period is:");
  tm tm_NextStartTime;
  localtime_r(&pm_measures.FiltrationStartTime, &tm_NextStartTime);

  tm tm_NextEndTime;
  localtime_r(&pm_measures.FiltrationEndTime, &tm_NextEndTime);
  
  LOG_D(TAG, "- next start time: %04d/%02d/%02d %02d:%02d:%02d (%u)", tm_NextStartTime.tm_year+1900, tm_NextStartTime.tm_mon+1, tm_NextStartTime.tm_mday, tm_NextStartTime.tm_hour, tm_NextStartTime.tm_min, tm_NextStartTime.tm_sec, pm_measures.FiltrationStartTime);
  LOG_D(TAG, "- next end time  : %04d/%02d/%02d %02d:%02d:%02d (%u)", tm_NextEndTime.tm_year+1900, tm_NextEndTime.tm_mon+1, tm_NextEndTime.tm_mday, tm_NextEndTime.tm_hour, tm_NextEndTime.tm_min, tm_NextEndTime.tm_sec, pm_measures.FiltrationEndTime);
}

// =================================================================================================
//                         START AND STOP FILTRATION PUMP
// =================================================================================================
void PM_StartFiltrationPump() {
    FiltrationPump.Start();
}
void PM_StopFiltrationPump() {
    FiltrationPump.Stop();

}
