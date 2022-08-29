/*
  Yves Gaignard
  
  Main procedure of the Pool Manager project
*/

#define TAG "PM_main"
//#define NVS_RESET_DEBUG   // if necessary uncomment this line to reset all data stored in the NVS (Non Volatile Storage)

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
#include <FreeRTOS.h>
#include <esp_task_wdt.h>          // ESP task management library


// Project definitions
#include "PM_Pool_Manager.h"       // Pool manager constant declarations
#include "PM_Log.h"                // Pool manager log management
#include "PM_Structures.h"         // Pool manager structure definitions
#include "PM_Parameters.h"         // Pool manager parameters
#include "PM_Config.h"             // Pool manager configuration parameters
#include "PM_I2CScan.h"            // Pool manager I2C scan tools
#include "PM_Time_Mngt.h"          // Pool manager time management
#include "PM_Temperature.h"        // Pool manager temperature management
#include "PM_Wifi.h"               // Pool manager wifi management
#include "PM_OTA_Web_Srv.h"        // Pool manager web server management
#include "PM_LCD.h"                // Pool manager display device management
#include "PM_Screens.h"            // Pool manager screens
#include "PM_Error.h"              // Pool manager error management
#include "PM_Utils.h"              // Pool manager utilities
#include "PM_Pump.h"               // Pool manager pumps management
#include "PM_Tasks.h"              // Pool manager tasks declarations



// Intantiate the Pool Manager configuration
PM_Config Pool_Configuration;

// To manage time
RTC_DS3231 rtc;  // RTC device handle
boolean    isRTCFound     = true;
boolean    isRTCLostPower = true;

time_t      now;  // Current time (global variable)
suseconds_t usec;
char timestamp_str[20];
tm* time_tm;

// Array of I2C Devices
static byte I2CDevices[128];

// Instantiate LCD display and a screen template
PM_LCD lcd(PM_LCD_Device_Addr, PM_LCD_Cols, PM_LCD_Rows);

// Instantiate screens
PM_Screens screens;

// NVS Non Volatile SRAM (eqv. EEPROM)
Preferences nvs;   

// swimming pool measures
PM_SwimmingPoolMeasures     pm_measures     = {
  PM_VERSION, //  uint8_t PMVersion;                // version of the structure
  true,  // bool    AutoMode;                       // Mode Automatic (true) or Manual (false)
  false, // bool    WinterMode;                     // Winter Mode if true
  false, // bool    pH_RegulationOnOff;             
  false, // bool    Orp_RegulationOnOff;             
  false, // boolean FilterPumpState;                // State of the filtering pump (true, false)
  false, // boolean pHMinusPumpState;               // State of the pH- pump (true, false)
  false, // boolean ChlorinePumpState;              // State of the Chlorine pump (true, false)
  10,    // uint_8  DelayPIDs;                      // Delay of starting PID computation after the start of a filtration pump (in minutes)
  -20.0, // float   InAirTemp;                      // Inside air temperature in °C 
  -20.0, // float   WaterTemp;                      // Water temperature in °C of the swimming pool
  -20.0, // float   OutAirTemp;                     // Outside air temperature in °C
  10.0,  // float   WaterTempLowThreshold;          // Water temperature low threshold to compute PIDs
  7.4,   // double  pHValue;                        // Current pH value
  1800000, //ulong   pHPIDWindowSize;
  0,     // ulong   pHPIDwindowStartTime;           // pH PID window start time   
  2700,  // ulong   pHPumpUpTimeLimit;              // Time in seconds max per day for pH injection
  0.0,   // double  pHPIDOutput;
  7.3,   // double  pH_SetPoint;
  2700000.0, //double  pH_Kp;
  0.0,   // double  pH_Ki;
  0.0,   // double  pH_Kd;
  3.49625783, // double  pHCalibCoeffs0;
  -2.011338191, // double  pHCalibCoeffs1;
  250.0, // double  OrpValue;                       // Current redox measure unit: mV
  1800000, //ulong   OrpPIDWindowSize;
  0,     // ulong   OrpPIDwindowStartTime;          // Orp PID window start time   
  2700,  // ulong   OrpPumpUpTimeLimit;             // Time in seconds max per day for Chlorine injection
  0.0,   // double  OrpPIDOutput;
  7.3,   // double  Orp_SetPoint;
  2700000.0, //double  Orp_Kp;
  0.0,   // double  Orp_Ki;
  0.0,   // double  Orp_Kd;
  -876.430775, // double  OrpCalibCoeffs0;
  2328.8985,   // double  OrpCalibCoeffs1;
  0,     // time_t  DayFiltrationUptime;            // Filtration Duration since the begin of the day
  0,     // time_t  DayFiltrationTarget;            // Maximum Filtration duration for the whole day
  0,     // time_t  PeriodFiltrationUptime;         // Filtration Duration since the begin of the period
  0,     // time_t  PeriodFiltrationStartTime;      // Next period start time of the filtration
  0,     // time_t  PeriodFiltrationEndTime;        // Next period end time of the filtration
  0,     // time_t  PreviousDayFiltrationUptime;    // Filtration Duration of the previous day
  0,     // time_t  PreviousDayFiltrationTarget;    // Target Filtration duration of the previous day
  PM_pH_Pump_Flow_Rate,       // float   pHMinusFlowRate;    // Flow rate of pH Minus liquid injected (liter per hour)
  PM_Chlorine_Pump_Flow_Rate, // float   ChlorineFlowRate;   // Flow rate of Chlorine liquid injected (liter per hour)
  0.0,   // float   pHMinusVolume;                  // Volume of pH Minus liquid since the last complete fill of the container
  0.0,   // float   ChlorineVolume;                 // Volume of liquid chlorine since the last complete fill of the container
  0,     // int32_t ConsumedInstantaneousPower;     // Instantaneous Power in Watt consumed by the filtration pump
  0,     // int32_t DayConsumedPower;               // Power in Watt consumed by the filtration pump since the begin of the day
  1.5,   // float   Pressure;                       // Pressure in the filtering device (unit hPa)
  1.8,   // float   PressureHighThreshold;          // Pressure to consider the filtration pump is started
  0.7,   // float   PressureMedThreshold;           // Pressure to consider the filtration pump is stopped
  1.0,   // double  PSICalibCoeffs0;
  0.0,   // double  PSICalibCoeffs1;
  PM_pH_Tank_Volume,        // float   pHMinusTankVolume;              // Max volume of the pH- tank
  PM_Chlorine_Tank_Volume,  // float   ChlorineTankVolume;             // Max volume of the Chlorine tank
  100.0, // float   pHMinusTankFill;                // % Fill of volume of the pH- tank
  100.0  // float   ChlorineTankFill;               // % Fill of volume of the Chlorine tank
}; 

// Instanciations of Pump and PID objects to make them global. But the constructors are then called 
// before loading of the storage struct. At run time, the attributes take the default
// values of the storage struct as they are compiled, just a few lines above, and not those which will 
// be read from NVS later. This means that the correct objects attributes must be set later in
// the setup function (fortunatelly, init methods exist).

// The three pumps of the system (instanciate the Pump class)
// In this case, all pumps start/Stop are managed by relays. pH and ORP pumps are interlocked with 
// filtration pump
PM_Pump FiltrationPump(FILTRATION_PUMP_Pin, FILTRATION_PUMP_Pin);
PM_Pump PhPump(PH_PUMP_Pin, PH_PUMP_Pin, NO_LEVEL, FILTRATION_PUMP_Pin, pm_measures.pHMinusFlowRate,  pm_measures.pHMinusTankVolume, pm_measures.pHMinusTankFill);
PM_Pump ChlPump(CHL_PUMP_Pin, CHL_PUMP_Pin, NO_LEVEL, FILTRATION_PUMP_Pin, pm_measures.ChlorineFlowRate, pm_measures.ChlorineTankVolume, pm_measures.ChlorineTankFill);

//PIDs instances
//Specify the links and initial tuning parameters
PID pHPID(&pm_measures.pHValue, &pm_measures.pHPIDOutput, &pm_measures.pH_SetPoint, pm_measures.pH_Kp, pm_measures.pH_Ki, pm_measures.pH_Kd, pHPID_DIRECTION);
PID OrpPID(&pm_measures.OrpValue, &pm_measures.OrpPIDOutput, &pm_measures.Orp_SetPoint, pm_measures.Orp_Kp, pm_measures.Orp_Ki, pm_measures.Orp_Kd, OrpPID_DIRECTION);

// To manage the connection on Wifi
boolean IsWifiConnected   = false;

// To manage wifi between multiple networks
WiFiMulti wifiMulti;

// Mutex to share access to I2C bus among two tasks: AnalogPoll and StatusLights
static SemaphoreHandle_t I2CMutex;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature temperatureSensors(&oneWire);

// arrays to hold temperatur sensor device addresses
DeviceAddress insideThermometer, outsideThermometer, waterThermometer;

// Instantiate object to manage all temperature sensors
PM_Temperature PM_TemperatureSensors;

// Mutex to share access to I2C bus among several tasks
static SemaphoreHandle_t mutex;

// Signal to start loop tasks
volatile bool startTasks = false;

// function declarations
void PM_getBoardInfo();
void PM_Time_Init();
void PM_Display_init    ();
void PM_Display_screen_1();
void PM_Display_screen_2();
bool PM_NVS_Init();
bool PM_NVS_Load();
bool PM_NVS_Save();
void PM_Temperature_Init();
void PM_SetpHPID(bool Enable);
void PM_SetOrpPID(bool Enable);
void PM_CalculateNextFiltrationPeriods();
bool saveParam(const char* key, uint8_t val);
bool saveParam(const char* key, bool val);
bool saveParam(const char* key, unsigned long val);
bool saveParam(const char* key, double val);
bool saveParam(const char* key, float val);
int  freeRam();
unsigned stack_hwm();
void stack_mon(UBaseType_t &hwm);
void lockI2C();
void unlockI2C();

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
  Log.setTag("*"                   , LOG_LEVEL);
  Log.setTag("PM_main"             , LOG_VERBOSE);
  Log.setTag("PM_I2CScan"          , LOG_LEVEL);
  Log.setTag("PM_Log"              , LOG_LEVEL);
  Log.setTag("PM_OTA_Web_Srv"      , LOG_LEVEL);
  Log.setTag("PM_Config"           , LOG_LEVEL);
  Log.setTag("PM_Temperature"      , LOG_LEVEL);
  Log.setTag("PM_Time_Mngt"        , LOG_LEVEL);
  Log.setTag("PM_Wifi"             , LOG_LEVEL);
  Log.setTag("PM_Tasks"            , LOG_LEVEL);
  Log.setTag("PM_Task_Pool_Manager", LOG_DEBUG);
  Log.setTag("PM_Tasks_Sensors"    , LOG_DEBUG);
  Log.setTag("PM_Tasks_Regulation" , LOG_DEBUG);
  Log.setTag("PM_Screen"           , LOG_LEVEL);
  Log.setTag("PM_Pump"             , LOG_LEVEL);
    
  // Log.formatTimestampOff(); // time in milliseconds (if necessary)
  LOG_I(TAG, "Starting Project: [%s]  Version: [%s]",Project.Name.c_str(), Project.Version.c_str());

  // Print the information of the board
  PM_getBoardInfo();
  
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
  /*
  DateTime Now(now);
  char LocalTimeFormat[sizeof(PM_LocalTimeFormat)];
  strcpy(LocalTimeFormat, PM_LocalTimeFormat);
  LOG_D(TAG, "Current date and local time is: %s", Now.toString(LocalTimeFormat));
  */
  //suseconds_t usec;
  //char timestamp_str[20];
  now = pftime::time(nullptr); // get current time
  time_tm = pftime::localtime(&now, &usec);  // Change in localtime
  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
  LOG_D(TAG, "Current date and local time is: %s", timestamp_str);
 
#ifdef NVS_RESET_DEBUG
  nvs.begin(Project.Name.c_str(),false);
  if ( ! nvs.clear() ) LOG_E(TAG, "Cannot clear the NVS namespace: %s", Project.Name.c_str());
  nvs.end();
#endif

  // Initialize NVS data 
  if ( ! PM_NVS_Init()) LOG_I(TAG, "Error on NVS initialization phase. See traces");

  //Init LCD
  PM_Display_init();

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
  
  // Attribute the GPIOs
  pinMode(PM_DisplayButton_Pin, INPUT_PULLUP);
  attachInterrupt(PM_DisplayButton_Pin, PM_DisplayButton_ISR, FALLING);

  //Define pins directions
  pinMode(FILTRATION_PUMP_Pin, OUTPUT);
  pinMode(PH_PUMP_Pin, OUTPUT);
  pinMode(CHL_PUMP_Pin, OUTPUT);

  pinMode(LIGHT_BUZZER_Pin, OUTPUT);

  // As the relays on the board are activated by a LOW level, set all levels HIGH at startup
  digitalWrite(FILTRATION_PUMP_Pin,HIGH);
  digitalWrite(PH_PUMP_Pin,HIGH); 
  digitalWrite(CHL_PUMP_Pin,HIGH);
  
  // Warning: pins used here have no pull-ups, provide external ones
  // pinMode(CHL_LEVEL, INPUT);
  // pinMode(PH_LEVEL, INPUT);

  // Initialize watch-dog
  esp_task_wdt_init(WDT_TIMEOUT, true);
  
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
  now = pftime::time(nullptr); // get current time
  time_tm = pftime::localtime(&now, &usec);  // Change in localtime
  strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
  //LOG_D(TAG, "Current date and local time is: %s", timestamp_str);
 
  //LOG_D(TAG, "AutoMode     : %d",pm_measures.AutoMode);
  //LOG_D(TAG, "now          : %d",now);
  //LOG_D(TAG, "Period Uptime: %d",pm_measures.PeriodFiltrationUptime);
  //LOG_D(TAG, "Start Time   : %d",pm_measures.PeriodFiltrationStartTime);
  //LOG_D(TAG, "End   Time   : %d",pm_measures.PeriodFiltrationEndTime);

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


  // Create tasks
  //                          Function           Name          Stack  Param PRIO  Handle                core
  //xTaskCreatePinnedToCore(PM_Task_Main,      "Task_Main",      10000, NULL, 10, nullptr,      0);
  //xTaskCreatePinnedToCore(PM_Task_GPIO,      "Task_GPIO",      10000, NULL,  8, nullptr,      0);

  // Create loop tasks in the scheduler.
  //------------------------------------
  int app_cpu = xPortGetCoreID();

  LOG_I(TAG, "Creating the loop tasks");
  // Create I2C sharing mutex
  I2CMutex = xSemaphoreCreateMutex();

  // Create tasks
  //                          Function                    Name               Stack  Param PRIO  Handle                core
  //xTaskCreatePinnedToCore(PM_Task_AnalogPoll,      "PM_Task_AnalogPoll",      3072, NULL, 1, nullptr,            app_cpu);  // Analog measurement polling task
  //  xTaskCreatePinnedToCore(PM_Task_ProcessCommand,  "PM_Task_ProcessCommand",  3072, NULL, 1, nullptr,            app_cpu); // MQTT commands processing
  xTaskCreatePinnedToCore(PM_Task_Pool_Manager,    "PM_Task_Pool_Manager",    3072, NULL, 1, nullptr,            app_cpu); // Pool Manager: Supervisory task
  xTaskCreatePinnedToCore(PM_Task_GetTemperature,  "PM_Task_GetTemperature",  3072, NULL, 1, nullptr,            app_cpu); // Temperatures measurement
  //xTaskCreatePinnedToCore(PM_Task_OrpRegulation,   "PM_Task_OrpRegulation",   2048, NULL, 1, nullptr,            app_cpu); // ORP regulation loop
  //xTaskCreatePinnedToCore(PM_Task_pHRegulation,    "PM_Task_pHRegulation",    2048, NULL, 1, nullptr,            app_cpu); // pH regulation loop
  xTaskCreatePinnedToCore(PM_Task_LCD,             "Task_LCD",                3072, NULL, 1, nullptr,            app_cpu);
  xTaskCreatePinnedToCore(PM_Task_WebServer,       "Task_WebServer",          3072, NULL, 1, nullptr,            app_cpu);
  //  xTaskCreatePinnedToCore(PM_Task_MeasuresPublish, "PM_Task_MeasuresPublish", 3072, NULL, 1, &pubMeasTaskHandle, app_cpu); // Measures MQTT publish 
  //  xTaskCreatePinnedToCore(PM_Task_SettingsPublish, "PM_Task_SettingsPublish", 3072, NULL, 1, &pubSetTaskHandle,  app_cpu);  // MQTT Settings publish 

  //display remaining RAM/Heap space.
  LOG_I(TAG, "[memCheck] Stack: %d bytes - Heap: %d bytes",stack_hwm(),freeRam());

  // Display the first screen
  PM_Display_screen_1();

  // and the second screen
  PM_Display_screen_2();

  // Start loops tasks
  LOG_I(TAG, "Init done, starting loop tasks");
  startTasks = true;

  delay(1000);          // wait for tasks to start

}
// =================================================================================================
//                                   LOOP OF POOL MANAGER
// =================================================================================================
void loop(void) {
  delay(1000);
  vTaskDelete(nullptr);
}


// =================================================================================================
//                              SCREEN DEFINITIONS
// =================================================================================================

void PM_Display_init    () {
  std::vector<std::string> screen;
  /*  
   |--------------------|
   |         1         2|
   |12345678901234567890|
   |--------------------|
   |     SCREEN 0       |
   |--------------------|
   |Pool Manager        |
   |Version: 1.0.0      |
   |                    | 
   |Yves Gaignard       |
   |--------------------|
*/ 
  screen.clear();
  screen.push_back(Project.Name);
  screen.push_back("Version: " + Project.Version);
  screen.push_back("");
  screen.push_back(Project.Author);

  screens.addScreen("Screen Init");
  screens.setCurrentScreen(0);
  screens.setInactivityTimeOutReset();

  lcd.init();
  lcd.clear();
  lcd.display();
  lcd.backlight();
  lcd.printScreen(screen);
}

void PM_Display_screen_1() {
  std::vector<std::string> screen;
  std::string DisplayLine;
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
  char degreeAsciiChar[2];
  sprintf(degreeAsciiChar, "%c", 176);

  screen.clear();
  std::string InAirTemp  = PM_itoa((int) (pm_measures.InAirTemp + 0.5 - (pm_measures.InAirTemp<0)));
  std::string WaterTemp  = PM_itoa((int) (pm_measures.WaterTemp + 0.5 - (pm_measures.WaterTemp<0)));
  std::string OutAirTemp = PM_itoa((int) (pm_measures.OutAirTemp + 0.5 - (pm_measures.OutAirTemp<0)));
  screen.push_back("In:"+InAirTemp+degreeAsciiChar+" W:"+WaterTemp+degreeAsciiChar+" Out:"+OutAirTemp+degreeAsciiChar);

  std::string pHValue  = PM_dtoa(pm_measures.pHValue, "%3.2f");
  std::string OrpValue = PM_dtoa(pm_measures.OrpValue, "%4.0f");
  screen.push_back("PH:"+pHValue+" Cl:"+OrpValue);

  tm * time_tm;
  char timestamp_str[20];
  time_tm = localtime(&pm_measures.DayFiltrationUptime);
	strftime(timestamp_str, sizeof(timestamp_str), PM_HourMinFormat, time_tm);
  std::string DayFiltrationUptime = timestamp_str;
  time_tm = localtime(&pm_measures.DayFiltrationTarget);
	strftime(timestamp_str, sizeof(timestamp_str), PM_HourMinFormat, time_tm);
  std::string DayFiltrationTarget = timestamp_str;
  screen.push_back("Filtration:"+DayFiltrationUptime+"/"+DayFiltrationTarget);
  
  std::string pHMinusVolume  = PM_ftoa(pm_measures.pHMinusVolume, "%3.2f");
  std::string ChlorineVolume = PM_ftoa(pm_measures.ChlorineVolume, "%3.2f");
  screen.push_back("PH-:"+pHMinusVolume+" Cl:"+ChlorineVolume);

  screens.addScreen("Screen 1");
  screens.setCurrentScreen(1);

  lcd.clear();
  lcd.display();
  lcd.backlight();
  lcd.printScreen(screen);
}

void PM_Display_screen_2() {
  std::vector<std::string> screen;
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
  screen.clear();

  std::string Pressure = PM_itoa((int) (pm_measures.Pressure + 0.5 - (pm_measures.Pressure<0)));
  std::string FilterPumpState = (pm_measures.FilterPumpState == true) ? " ON" : "OFF";
  screen.push_back("Filter:"+FilterPumpState+" P:"+Pressure+"hPa");

  std::string ConsumedInstantaneousPower = PM_itoa(pm_measures.ConsumedInstantaneousPower);
  std::string DayConsumedPower = PM_itoa(pm_measures.DayConsumedPower);
  screen.push_back("Pow:"+ConsumedInstantaneousPower+" Day:"+DayConsumedPower+"kWh");

  screen.push_back("Pump PH-: N/A  Cl: N/A ");

  std::string pHMinusTankFill = PM_itoa((int) (pm_measures.pHMinusTankFill + 0.5 - (pm_measures.pHMinusTankFill<0)));
  std::string ChlorineTankFill = PM_itoa((int) (pm_measures.ChlorineTankFill + 0.5 - (pm_measures.ChlorineTankFill<0)));
  screen.push_back("PH-:"+pHMinusTankFill+"% Cl:"+ChlorineTankFill+"%");

  screens.addScreen("Screen 2");
  screens.setCurrentScreen(2);
  lcd.clear();
  lcd.display();
  lcd.backlight();
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
  isRTCFound     = true;
  isRTCLostPower = true;

  // check if a RTC module is connected
  if (! rtc.begin()) {
    LOG_E(TAG, "Cannot find any RTC device. Time will be initialized through a NTP server");
    isRTCFound = false;
  } else {
    isRTCLostPower=rtc.lostPower();
  }

  // Initialize time from NTP server
   PM_Time_Mngt_initialize_time();
  
  suseconds_t usec;
  char timestamp_str[20];
  tm* time_tm = pftime::localtime(nullptr, &usec);
	strftime(timestamp_str, sizeof(timestamp_str), PM_LocalTimeFormat, time_tm);
  LOG_D(TAG, "Current date and local time is: %s", timestamp_str);
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
  pm_measures.AutoMode                   = nvs.getBool  ("AutoMode"       ,true);
  pm_measures.WinterMode                 = nvs.getBool  ("WinterMode"     ,false);
  pm_measures.InAirTemp                  = nvs.getFloat ("InAirTemp"      ,0.0);
  pm_measures.WaterTemp                  = nvs.getFloat ("WaterTemp"      ,0.0);
  pm_measures.OutAirTemp                 = nvs.getFloat ("OutAirTemp"     ,0.0);
  pm_measures.WaterTempLowThreshold      = nvs.getFloat ("WaterTempLowT"  ,0.0);
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
  pm_measures.pHCalibCoeffs0             = nvs.getDouble("pHCalibCoeffs0" ,0.0);
  pm_measures.pHCalibCoeffs1             = nvs.getDouble("pHCalibCoeffs1" ,0.0);
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
  pm_measures.OrpCalibCoeffs0            = nvs.getDouble("OrpCalibCoeffs0",0.0);
  pm_measures.OrpCalibCoeffs1            = nvs.getDouble("OrpCalibCoeffs1",0.0);
  pm_measures.DelayPIDs                  = nvs.getUChar ("DelayPIDs"      ,0);
  pm_measures.DayFiltrationUptime        = nvs.getULong ("DayFiltrUptime" ,0);
  pm_measures.DayFiltrationTarget        = nvs.getULong ("DayFiltrTarget" ,0);
  pm_measures.PeriodFiltrationUptime     = nvs.getULong ("PFiltrUptime"   ,0);
  pm_measures.PeriodFiltrationStartTime  = nvs.getULong ("PFiltrStartTime",0);
  pm_measures.PeriodFiltrationEndTime    = nvs.getULong ("PFiltrEndTime"  ,0);
  pm_measures.PreviousDayFiltrationUptime= nvs.getULong ("PDayFiltrUptime",0);
  pm_measures.PreviousDayFiltrationTarget= nvs.getULong ("PDayFiltrTarget",0);
  pm_measures.pHMinusFlowRate            = nvs.getFloat ("pHMinusFlowRate",0.0);
  pm_measures.ChlorineFlowRate           = nvs.getFloat ("ChlorinFlowRate",0.0);
  pm_measures.pHMinusVolume              = nvs.getFloat ("pHMinusVolume"  ,0.0);
  pm_measures.ChlorineVolume             = nvs.getFloat ("ChlorineVolume" ,0.0);
  pm_measures.ConsumedInstantaneousPower = nvs.getUInt  ("InstantConsPwr" ,0);
  pm_measures.DayConsumedPower           = nvs.getUInt  ("DayConsumedPwr" ,0);
  pm_measures.Pressure                   = nvs.getFloat ("Pressure"       ,0.0);
  pm_measures.PressureHighThreshold      = nvs.getFloat ("PHighThreshold" ,0.0);
  pm_measures.PressureMedThreshold       = nvs.getFloat ("PMedThreshold"  ,0.0);
  pm_measures.PSICalibCoeffs0            = nvs.getFloat ("PSICalibCoeffs0",0.0);
  pm_measures.PSICalibCoeffs1            = nvs.getFloat ("PSICalibCoeffs1",0.0);
  pm_measures.FilterPumpState            = nvs.getBool  ("FilterPumpOn"   ,false);
  pm_measures.pHMinusPumpState           = nvs.getBool  ("pHMinusPumpOn"  ,false);
  pm_measures.ChlorinePumpState          = nvs.getBool  ("ChlorinePumpOn" ,false);
  pm_measures.pHMinusTankVolume          = nvs.getFloat ("pHMinusTankVol" ,0.0);
  pm_measures.ChlorineTankVolume         = nvs.getFloat ("ChlorineTankVol",0.0);
  pm_measures.pHMinusTankFill            = nvs.getFloat ("pHMinusTankFill",0.0);
  pm_measures.ChlorineTankFill           = nvs.getFloat ("ChlorinTankFill",0.0);

  nvs.end();

  LOG_D(TAG, "%d", pm_measures.PMVersion);
  LOG_D(TAG, "%d, %d, %d, %d, %d", pm_measures.AutoMode, pm_measures.WinterMode, pm_measures.pH_RegulationOnOff, pm_measures.Orp_RegulationOnOff, pm_measures.DelayPIDs);
  LOG_D(TAG, "%2.2f, %2.2f, %2.2f,%2.2f", pm_measures.InAirTemp, pm_measures.WaterTemp,pm_measures.OutAirTemp, pm_measures.WaterTempLowThreshold);
  LOG_D(TAG, "%2.2f, %4.0f", pm_measures.pHValue, pm_measures.OrpValue);
  LOG_D(TAG, "%d, %d", pm_measures.pHPIDWindowSize, pm_measures.OrpPIDWindowSize);
  LOG_D(TAG, "%d, %d", pm_measures.pHPIDwindowStartTime, pm_measures.OrpPIDwindowStartTime);
  LOG_D(TAG, "%d, %d", pm_measures.pHPumpUpTimeLimit, pm_measures.OrpPumpUpTimeLimit);
  LOG_D(TAG, "%4.0f, %4.0f, %8.0f, %3.0f, %3.0f", pm_measures.pHPIDOutput, pm_measures.pH_SetPoint, pm_measures.pH_Kp, pm_measures.pH_Ki, pm_measures.pH_Kd);
  LOG_D(TAG, "%4.0f, %4.0f, %8.0f, %3.0f, %3.0f", pm_measures.OrpPIDOutput, pm_measures.Orp_SetPoint, pm_measures.Orp_Kp, pm_measures.Orp_Ki, pm_measures.Orp_Kd);
  LOG_D(TAG, "%f, %f, %f, %f, %f, %f", pm_measures.pHCalibCoeffs0, pm_measures.pHCalibCoeffs1, pm_measures.OrpCalibCoeffs0, pm_measures.OrpCalibCoeffs1, pm_measures.PSICalibCoeffs0, pm_measures.PSICalibCoeffs1);
  LOG_D(TAG, "%d, %d", pm_measures.DayFiltrationUptime, pm_measures.DayFiltrationTarget);
  LOG_D(TAG, "%d, %d", pm_measures.PreviousDayFiltrationUptime, pm_measures.PreviousDayFiltrationTarget);
  LOG_D(TAG, "%d, %d, %d", pm_measures.PeriodFiltrationUptime, pm_measures.PeriodFiltrationStartTime, pm_measures.PeriodFiltrationEndTime);
  LOG_D(TAG, "%2.2f, %2.2f", pm_measures.pHMinusFlowRate, pm_measures.ChlorineFlowRate);
  LOG_D(TAG, "%2.2f, %2.2f", pm_measures.pHMinusVolume, pm_measures.ChlorineVolume);
  LOG_D(TAG, "%d, %d", pm_measures.ConsumedInstantaneousPower, pm_measures.DayConsumedPower);
  LOG_D(TAG, "%2.2f, %2.2f, %2.2f", pm_measures.Pressure, pm_measures.PressureHighThreshold, pm_measures.PressureMedThreshold);
  LOG_D(TAG, "%d, %d, %d", pm_measures.FilterPumpState, pm_measures.pHMinusPumpState, pm_measures.ChlorinePumpState);
  LOG_D(TAG, "%2.2f, %2.2f", pm_measures.pHMinusTankVolume, pm_measures.ChlorineTankVolume);
  LOG_D(TAG, "%2.2f, %2.2f", pm_measures.pHMinusTankFill, pm_measures.ChlorineTankFill);

  return (pm_measures.PMVersion == PM_VERSION);
}
// =================================================================================================
//                           SAVE MEASURES TO STORAGE
// =================================================================================================
bool PM_NVS_Save() {

  now = pftime::time(nullptr); // get current time
  time_tm = pftime::localtime(&now, &usec);  // Change in localtime
  nvs.begin(Project.Name.c_str(),false);

  // Beware : the key maximum length is only 15 characters

  size_t i = 
       nvs.putUChar ("PMVersion",       pm_measures.PMVersion);
  i += nvs.putBool  ("AutoMode",        pm_measures.AutoMode);
  i += nvs.putBool  ("WinterMode",      pm_measures.WinterMode);
  i += nvs.putFloat ("InAirTemp",       pm_measures.InAirTemp);
  i += nvs.putFloat ("WaterTemp",       pm_measures.WaterTemp);
  i += nvs.putFloat ("OutAirTemp",      pm_measures.OutAirTemp);
  i += nvs.putFloat ("WaterTempLowT",   pm_measures.WaterTempLowThreshold);
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
  i += nvs.putDouble("pHCalibCoeffs0",  pm_measures.pHCalibCoeffs0);
  i += nvs.putDouble("pHCalibCoeffs1",  pm_measures.pHCalibCoeffs1);
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
  i += nvs.putDouble("OrpCalibCoeffs0", pm_measures.OrpCalibCoeffs0);
  i += nvs.putDouble("OrpCalibCoeffs1", pm_measures.OrpCalibCoeffs1);
  i += nvs.putUChar ("DelayPIDs",       pm_measures.DelayPIDs);
  i += nvs.putULong ("DayFiltrUptime",  pm_measures.DayFiltrationUptime);
  i += nvs.putULong ("DayFiltrTarget",  pm_measures.DayFiltrationTarget);
  i += nvs.putULong ("PFiltrUptime",    pm_measures.PeriodFiltrationUptime);
  i += nvs.putULong ("PFiltrStartTime", pm_measures.PeriodFiltrationStartTime);
  i += nvs.putULong ("PFiltrEndTime",   pm_measures.PeriodFiltrationEndTime);
  i += nvs.putULong ("PDayFiltrUptime", pm_measures.PreviousDayFiltrationUptime);
  i += nvs.putULong ("PDayFiltrTarget", pm_measures.PreviousDayFiltrationTarget);
  i += nvs.putFloat ("pHMinusFlowRate", pm_measures.pHMinusFlowRate);
  i += nvs.putFloat ("ChlorinFlowRate", pm_measures.ChlorineFlowRate);
  i += nvs.putFloat ("pHMinusVolume",   pm_measures.pHMinusVolume);
  i += nvs.putFloat ("ChlorineVolume",  pm_measures.ChlorineVolume);
  i += nvs.putUInt  ("InstantConsPwr",  pm_measures.ConsumedInstantaneousPower);
  i += nvs.putUInt  ("DayConsumedPwr",  pm_measures.DayConsumedPower);
  i += nvs.putFloat ("Pressure",        pm_measures.Pressure);
  i += nvs.putFloat ("PHighThreshold",  pm_measures.PressureHighThreshold);
  i += nvs.putFloat ("PMedThreshold",   pm_measures.PressureMedThreshold);
  i += nvs.putFloat ("PSICalibCoeffs0", pm_measures.PSICalibCoeffs0);
  i += nvs.putFloat ("PSICalibCoeffs1", pm_measures.PSICalibCoeffs1);
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
// functions to save any type of parameter (4 overloads with same name but different arguments)
// =================================================================================================
bool saveParam(const char* key, uint8_t val)
{
  nvs.begin(Project.Name.c_str(),false);
  size_t i = nvs.putUChar(key,val);
  return(i == sizeof(val));
}

bool saveParam(const char* key, bool val)
{
  nvs.begin(Project.Name.c_str(),false);
  size_t i = nvs.putBool(key,val);
  return(i == sizeof(val));
}

bool saveParam(const char* key, unsigned long val)
{
  nvs.begin(Project.Name.c_str(),false);
  size_t i = nvs.putULong(key,val);
  return(i == sizeof(val));
}

bool saveParam(const char* key, double val)
{
  nvs.begin(Project.Name.c_str(),false);
  size_t i = nvs.putDouble(key,val);
  return(i == sizeof(val));
}

bool saveParam(const char* key, float val)
{
  nvs.begin(Project.Name.c_str(),false);
  size_t i = nvs.putFloat(key,val);
  return(i == sizeof(val));
}

// =================================================================================================
//                           UTILITY FUNCTIONS
// =================================================================================================
//Compute free RAM
//useful to check if it does not shrink over time
int freeRam () {
  int v = xPortGetFreeHeapSize();
  return v;
}

// Get current free stack 
unsigned stack_hwm(){
  return uxTaskGetStackHighWaterMark(nullptr);
}

// Monitor free stack (display smallest value)
void stack_mon(UBaseType_t &hwm)
{
  UBaseType_t temp = uxTaskGetStackHighWaterMark(nullptr);
  if(!hwm || temp < hwm)
  {
    hwm = temp;
    LOG_D(TAG, "[stack_mon] %s: %d bytes",pcTaskGetTaskName(NULL), hwm);
  }  
}

// Get exclusive access of I2C bus
void lockI2C(){
  xSemaphoreTake(mutex, portMAX_DELAY);
}

// Release I2C bus access
void unlockI2C(){
  xSemaphoreGive(mutex);  
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

  // calculate the filtration duration in seconds depending on the water temperature
  LOG_D(TAG, "Water temperature: %6.2f", pm_measures.WaterTemp);
  pm_measures.DayFiltrationTarget = Pool_Configuration.GetFiltrationDuration(pm_measures.WaterTemp);
  saveParam("DayFiltrTarget", (unsigned long)pm_measures.DayFiltrationTarget);

  tm tm_duration = PM_Time_Mngt_convertSecondsToTm(pm_measures.DayFiltrationTarget);
  LOG_D(TAG, "Filtration duration for this day: %02d:%02d:%02d (%ds)", tm_duration.tm_hour, tm_duration.tm_min, tm_duration.tm_sec, pm_measures.DayFiltrationTarget);
  
  if (pm_measures.DayFiltrationUptime <= pm_measures.DayFiltrationTarget) {
    Pool_Configuration.NextFiltrationPeriod (pm_measures.PeriodFiltrationStartTime, pm_measures.PeriodFiltrationEndTime, pm_measures.DayFiltrationUptime, pm_measures.DayFiltrationTarget);
    saveParam("PFiltrStartTime", (unsigned long)pm_measures.PeriodFiltrationStartTime);
    saveParam("PFiltrEndTime", (unsigned long)pm_measures.PeriodFiltrationEndTime);
  }

  LOG_D(TAG, "Next Filtration period is:");
  tm tm_NextStartTime;
  localtime_r(&pm_measures.PeriodFiltrationStartTime, &tm_NextStartTime);

  tm tm_NextEndTime;
  localtime_r(&pm_measures.PeriodFiltrationEndTime, &tm_NextEndTime);
  
  LOG_D(TAG, "- next start time: %04d/%02d/%02d %02d:%02d:%02d (%u)", tm_NextStartTime.tm_year+1900, tm_NextStartTime.tm_mon+1, tm_NextStartTime.tm_mday, tm_NextStartTime.tm_hour, tm_NextStartTime.tm_min, tm_NextStartTime.tm_sec, pm_measures.PeriodFiltrationStartTime);
  LOG_D(TAG, "- next end time  : %04d/%02d/%02d %02d:%02d:%02d (%u)", tm_NextEndTime.tm_year+1900, tm_NextEndTime.tm_mon+1, tm_NextEndTime.tm_mday, tm_NextEndTime.tm_hour, tm_NextEndTime.tm_min, tm_NextEndTime.tm_sec, pm_measures.PeriodFiltrationEndTime);
}
