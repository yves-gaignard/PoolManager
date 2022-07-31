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
  std::string DisplayLine;
  if (PM_Display_State == false) {
    lcd.noAutoscroll();
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
  std::string DisplayLine;
  if (PM_Display_State == false) {
    lcd.noAutoscroll();
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

// Display an error. The error number is display on (0,0) and the message on (0,1) during DisplayTime seconds
void PM_Display_screen_error_msg(LiquidCrystal_I2C& lcd, std::string& ErrorNumber, std::string& ErrorMsg, int DisplayTime) {
  if (PM_Display_State == false) {
    lcd.noAutoscroll();
    lcd.display();
    lcd.backlight();
    PM_Display_State = true;
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Error:");
  lcd.setCursor(7,0);
  lcd.print(ErrorNumber.c_str());
  lcd.setCursor(0,1);
  PM_Display_scrollMessage(lcd, 1, ErrorMsg, DisplayTime);
}

void PM_Display_scrollMessage(LiquidCrystal_I2C& lcd, int row, std::string message, int displayTime) {
  int time_to_display = displayTime * 1000;
  message+=" ";
  int len = message.length();
  std::string padding="";
  for (int i=0; i < PM_LCD_Cols; i++) {
    message = " " + message;
    padding += " "; 
  } 
  message+=padding;
  std::string sub_message;
  time_t start =millis();
  int j = start;
  while (j<start+time_to_display) {
    for (int position = 0; position < len+PM_LCD_Cols; position++) {
      lcd.setCursor(0, row);
      sub_message=message.substr(position, PM_LCD_Cols);
      //int len_sub = sub_message.length();
      //ESP_LOGV(TAG, "%d - %s",len_sub, sub_message.c_str());
      lcd.print(sub_message.c_str());
      delay(300);
    }
    j=millis();
  }
}
