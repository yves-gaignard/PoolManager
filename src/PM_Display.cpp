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
#include "PM_Constants.h"       // Pool manager constants definitions
#include "PM_Display.h"         // Pool manager Display tools

static const char* TAG = "PM_Display";

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

void PM_Display_init(LiquidCrystal_I2C& lcd){
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(Project.Name.c_str());
  lcd.setCursor(0,1);
  lcd.print("Version:");
  lcd.print(Project.Version.c_str());
  lcd.setCursor(0,3);
  lcd.print("Copyright Y.Gaignard");
}

// Shutdown the display
void PM_Display_noDisplay(LiquidCrystal_I2C& lcd) {
  lcd.noDisplay();
  lcd.noBacklight();
  PM_Display_State = false;
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
void PM_Display_screen_1(LiquidCrystal_I2C& lcd, PM_SwimmingPoolMeasures_str & measures) {
  String DisplayLine;
  if (PM_Display_State == false) {
    lcd.display();
    lcd.backlight();
    PM_Display_State = true;
  }
  lcd.clear();

  lcd.setCursor(0,0);
  DisplayLine = "In:"+measures.InAirTemp_str+" W:"+measures.WaterTemp_str+" Out:"+measures.OutAirTemp_str;
  lcd.print(DisplayLine.c_str());
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
void PM_Display_screen_2(LiquidCrystal_I2C& lcd, PM_SwimmingPoolMeasures_str & measures) {
  String DisplayLine;
  if (PM_Display_State == false) {
    lcd.display();
    lcd.backlight();
    PM_Display_State = true;
  }
  lcd.clear();
  
  lcd.setCursor(0,0);
  DisplayLine = "Screen 2";
  lcd.print(DisplayLine.c_str());
}