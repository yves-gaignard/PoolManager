/*
  Copyright 2022 - Yves Gaignard
  
  Functions relatives to the Wifi
*/
#ifndef PM_Display_h
#define PM_Display_h

// Standard library definitions
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>  // Library for LCD management

// Display Initialization procedure
void PM_Display_init(LiquidCrystal_I2C& lcd);

// Activate the display
void PM_Display_Display(LiquidCrystal_I2C& lcd);

// Shutdown the display
void PM_Display_noDisplay(LiquidCrystal_I2C& lcd);

// Get the display state (true = ON, false = OFF)
boolean PM_Display_getState (LiquidCrystal_I2C& lcd);

// Is time to shutdown the display ?
void PM_Display_isTimeToNoDisplay(LiquidCrystal_I2C& lcd);

// Display of screens procedure
void PM_Display_screen_0(LiquidCrystal_I2C& lcd, PM_SwimmingPoolMeasures_str & measures);
void PM_Display_screen_1(LiquidCrystal_I2C& lcd, PM_SwimmingPoolMeasures_str & measures);

#endif