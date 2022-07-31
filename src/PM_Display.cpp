/*
  Yves Gaignard
  
  Display device management of the Pool Manager project
*/

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// Standard library definitions
#include <Arduino.h>

#include <iomanip>
#include <sstream>

// Project definitions
#include "PM_Structures.h"      // Pool manager structure definitions
#include "PM_Parameters.h"      // Pool manager parameters
#include "PM_Display.h"         // Pool manager Display tools

static const char* TAG = "PM_Display";

// make some custom characters: (generator: https://maxpromer.github.io/LCD-Character-Creator/ )
static byte degree[8] = {
  0b01100,
  0b10010,
  0b10010,
  0b10010,
  0b01100,
  0b00000,
  0b00000,
  0b00000
};


/*
0s
   Welcome on Pool Manager Version 1.0.0
   Copyright Yves Gaignard
2s
   Connect to 192.168.1.99
5s
   |--------------------|
   |    SCREEN INIT     |
   |--------------------|
10s
*/

/* Display Initialization procedure
   |--------------------|
   |         1         2|
   |12345678901234567890|
   |--------------------|
   |    SCREEN INIT     |
   |--------------------|
   |{Project.Name}      |
   |{Project.Version}   |
   |                    | 
   |Copyright Y.Gaignard| 
   |--------------------|
*/
static boolean PM_Display_State=false;
static time_t  PM_Display_StartTime=0;
static time_t  PM_Time_MaxInactitivityTime=5*6;
static time_t  now;
struct tm      timeinfo;
static char    strftime_buf[64];



void PM_Display_init(LiquidCrystal_I2C& lcd){
  // create specific character
  lcd.init();
  lcd.clear();
  lcd.display();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(Project.Name.c_str());
  lcd.setCursor(0,1);
  lcd.print("Version:");
  lcd.print(Project.Version.c_str());
  lcd.setCursor(0,3);
  lcd.print(Project.Author.c_str());
}

// Activate the display
void PM_Display_Display(LiquidCrystal_I2C& lcd) {
  lcd.display();
  lcd.backlight();
  PM_Display_State = true;
  PM_Display_StartTime= millis();
  //PM_Display_StartTime= time(&now);
  localtime_r(&now, &timeinfo);
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGV(TAG, "The current local time is: %s", strftime_buf);
}
// Shutdown the display
void PM_Display_noDisplay(LiquidCrystal_I2C& lcd) {
  lcd.noDisplay();
  lcd.noBacklight();
  PM_Display_State = false;
}

// Get the display status (true = ON, false = OFF)
boolean PM_Display_getState (LiquidCrystal_I2C& lcd) {
  return PM_Display_State;
}

// Is time to shutdown the display ?
void PM_Display_isTimeToNoDisplay(LiquidCrystal_I2C& lcd) {
  now= millis();
  if (now-PM_Display_StartTime > PM_Time_MaxInactitivityTime)
  {
    PM_Display_noDisplay(lcd);
  }
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
void PM_Display_screen_0(LiquidCrystal_I2C& lcd, PM_SwimmingPoolMeasures_str & measures) {
  String DisplayLine;
  if (PM_Display_State == false) {
    lcd.display();
    lcd.backlight();
    PM_Display_State = true;
  }
  lcd.setCursor(0,0);
  lcd.clear();

  DisplayLine = "In:"+measures.InAirTemp_str+"  W:"+measures.WaterTemp_str+"  Out:"+measures.OutAirTemp_str;
  lcd.print(DisplayLine.c_str());
  lcd.createChar(1, degree);
  lcd.setCursor(5,0);  lcd.write(1);
  lcd.setCursor(11,0); lcd.write(1);
  lcd.setCursor(19,0); lcd.write(1);
  lcd.setCursor(0,1);
  DisplayLine = "PH:"+measures.pH_str+" Cl:"+measures.Chlorine_str;
  if (measures.Chlorine_str < measures.ChlorineMin_str) DisplayLine+=" <"+measures.ChlorineMin_str;
  if (measures.Chlorine_str > measures.ChlorineMax_str) DisplayLine+=" >"+measures.ChlorineMax_str;
  lcd.print(DisplayLine.c_str());
  lcd.setCursor(0,2);
  DisplayLine = "Filter:"+measures.DayFilterTime_str+" / "+measures.MaxDayFilterTime_str;
  lcd.print(DisplayLine.c_str());
  lcd.setCursor(0,3);
  DisplayLine = "PH-:"+measures.pHMinusVolume_str+" Cl:"+measures.ChlorineVolume_str;
  lcd.print(DisplayLine.c_str());

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
void PM_Display_screen_1(LiquidCrystal_I2C& lcd, PM_SwimmingPoolMeasures_str & measures) {
  String DisplayLine;
  if (PM_Display_State == false) {
    lcd.display();
    lcd.backlight();
    PM_Display_State = true;
  }
  lcd.setCursor(0,0);
  lcd.clear();

  DisplayLine = "Filter:"+measures.FilterPumpState_str+" P:"+measures.Pressure_str+"hPa";
  lcd.print(DisplayLine.c_str());
  lcd.setCursor(0,1);
  DisplayLine = "Pow:"+measures.ConsumedInstantaneousPower_str+" Day:"+measures.Chlorine_str+"kWh";
  lcd.print(DisplayLine.c_str());
  lcd.setCursor(0,2);
  DisplayLine = "Pump PH-:"+measures.pHMinusPumpState_str+" Cl:"+measures.ChlorinePumpState;
  lcd.print(DisplayLine.c_str());
  lcd.setCursor(0,3);
  DisplayLine = "Max PH-:"+measures.pHMinusMaxVolume_str+" Cl:"+measures.ChlorineMaxVolume_str;
  lcd.print(DisplayLine.c_str());
}

/*

struct PM_SwimmingPoolMeasures_str {
  String  InAirTemp_str;                  // Inside air temperature in °C (string format: 28.2 °)
  String  WaterTemp_str;                  // Water temperature in °C of the swimming pool (string format 25.2 °)
  String  OutAirTemp_str;                 // Outside air temperature in °C (string format 15.4 °)
  String  pH_str;                         // pH unit (string format 7.2)
  String  Chlorine_str;                   // redox measure unit: mV (string format 3000 mV)
  String  ChlorineMin_str;                // Minimum acceptable redox measure unit: mV (string format 3000 mV)
  String  ChlorineMax_str;                // Maximum acceptable redox measure unit: mV (string format 3000 mV)
  String  DayFilterTime_str;              // Filter Time since the begin of the day (string format 15h26)
  String  MaxDayFilterTime_str;           // Maximum Filter Time since the begin of the day (string format 16h)
  String  pHMinusVolume_str;              // Volume of pH Minus liquid since the last complete fill of the container (string format 20.4 l)
  String  ChlorineVolume_str;             // Volume of liquid chlorine since the last complete fill of the container (string format 12.9 l)
  String  ConsumedInstantaneousPower_str; // Instantaneous Power in Watt consumed by the filtration pump (string format 2514 W)
  String  DayConsumedPower_str;           // Power in Watt consumed by the filtration pump since the begin of the day (string format 25 kWh)
  String  Pressure_str;                   // Pressure in the filtering device (unit hPa) (string format)
  String  FilterPumpState_str;            // State of the filtering pump (ON  , OFF)
  String  pHMinusPumpState_str;           // State of the pH- pump (ON, OFF)
  String  ChlorinePumpState;              // State of the pH- pump (ON, OFF)
  String  pHMinusMaxVolume_str;           // Volume max of the pH- container (string format 20.0 l)
  String  ChlorineMaxVolume_str;          // Volume max of the Chlorine container (string format 20.0 l)
};
*/