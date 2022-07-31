/*
  Copyright 2022 - Yves Gaignard
  
  Configuration Parameters of the Pool Manager project
*/
#ifndef PM_Pool_Configuration_h
#define PM_Pool_Configuration_h

#include <Arduino.h>
#include <vector>
#include "PM_Structures.h"
#include "PM_Error.h"


// =========================================================================================================
//                                   Pool parameters that you have to configure
// =========================================================================================================

// ------------------------------------------------------------
// The filtration time  
// ------------------------------------------------------------
// The filtration time is depending on the water temperature. 
// A lot of factors may have to be taken into account
// The result has to be implemented in an abaqus table that you have to adapt in the context of your pool
// Look for on internet, you'll find a lot of abaqus. Choose the one you think it will be the good one

// The filtration time is conformed to the following abaqus ( Water temperature , duration of filtration per day ) can be customized
// The implemented values are based on several information taken on intenet and on my experience of my pool.

// Rules to fill in the abaqus
// R1 : the lowest temperature must be -20 and the highest 99
// R2 : temperature of the temperature range must be consecutives (aka TempMin of a line must be equal to TempMax of previous line)
// R3 : the filtration time of consecutive lines must increasing 
static std::vector<PM_FiltrationTime> PM_FiltrationTime_Abaqus = {
// { TempMin , TempMax, FiltrationTime }  
  {-20, 14,  2} ,       //             <14°C  ==>  2 hours
  { 14, 15,  3} ,       //  >=14°C and <15°C  ==>  3 hours
  { 15, 16,  4} ,       //  >=15°C and <16°C  ==>  4 hours
  { 16, 17,  5} ,       //  >=16°C and <17°C  ==>  5 hours
  { 17, 18,  6} ,       //  >=17°C and <18°C  ==>  6 hours
  { 18, 19,  7} ,       //  >=18°C and <19°C  ==>  7 hours
  { 19, 20,  8} ,       //  >=19°C and <20°C  ==>  8 hours
  { 20, 21,  9} ,       //  >=20°C and <21°C  ==>  9 hours
  { 21, 22, 10} ,       //  >=21°C and <22°C  ==> 10 hours
  { 22, 24, 11} ,       //  >=22°C and <24°C  ==> 11 hours
  { 24, 26, 12} ,       //  >=24°C and <26°C  ==> 12 hours
  { 26, 28, 13} ,       //  >=26°C and <28°C  ==> 13 hours
  { 28, 30, 14} ,       //  >=28°C and <30°C  ==> 14 hours
  { 30, 32, 15} ,       //  >=30°C and <32°C  ==> 15 hours
  { 32, 99, 16}         //  >=32°C            ==> 16 hours
};
/*  Other possibility found on https://www.desjoyaux.fr/faq/combien-de-temps-pour-la-filtration-de-ma-piscine/#:~:text=La%20r%C3%A8gle%20la%20plus%20simple,filtrer%2012%20heures%20par%20jours.
  std::vector<PM_FiltrationTime> PM_FiltrationTime_Abaqus = {
  // { TempMin , TempMax, FiltrationTime }  
  {-20, 10,  2} ,       //             <10°C  ==>  2 hours
  { 10, 12,  4} ,       //  >=10°C and <12°C  ==>  4 hours
  { 12, 16,  6} ,       //  >=12°C and <16°C  ==>  6 hours
  { 16, 24,  8} ,       //  >=16°C and <24°C  ==>  8 hours
  { 24, 27, 12} ,       //  >=24°C and <27°C  ==> 12 hours
  { 27, 30, 20} ,       //  >=27°C and <30°C  ==> 20 hours
  { 30, 99, 24}         //  >=30°C            ==> 24 hours
  };
*/

// ------------------------------------------------------------
// Filtration periods
// ------------------------------------------------------------
// You can spread the filtration duration all along the day based on periods
// The period is a couple of start time and end time.
// The duration of the filtration is spread on these periods depending on the priority. 
// If several periods have the same priority, and if the total duration is less than the sum of the period with this same priority,
// then the duration will be spread on these periods for the same time
// NB: The filtration period is evaluated once a day at midnight just after the day change 
// NB: It is better to filter during the light day

// Rules to check:
// R1 : The array does not need to be sorted
// R2 : a period cannot recover another one
// R3 : all periods have to cover the whole 24h of the day (aka from 0h to 24h)
static std::vector<PM_FiltrationPeriod> PM_FiltrationPeriod_Abaqus = {
// { Start , End, Priority }  
  {  8, 12,  1},  // The main priority 1 is allocated to the morning
  { 14, 20,  1},  // ... and for the afternoon (priority 1 too)
  { 20, 24,  2},  // The evening will be taken if the filtration time is greater than the sum of the morning and the afternoon period
  {  0,  8,  3},  // The night period is considered as less priority than the light day.
  { 12, 14,  4}   // During lunch time, the filtration is on only when necessary 
};
// Example 1: if the duration of the filtration is  6h per day, the filtration will at 8h-11h and 14h-17h as the morning and the afternoon have the same priority
// Example 2: if the duration of the filtration is 12h per day, the filtration will at 8h-12h, 14h-20h and 20h-22h
// Example 3: if the duration of the filtration is 16h per day, the filtration will at 0h-2h, 8h-12h, 14h-20h and 20h-24h
// Example 4: if the duration of the filtration is 23h per day, the filtration will at 0h-2h, 8h-12h, 12h-13h, 14h-20h and 20h-24h

// ------------------------------------------------------------
// pH (potential hydrogen) 
// ------------------------------------------------------------
// the pH of the pool must be between 7.2 and 7.8
const float PM_pH_Min = 7.2;
const float PM_pH_Max = 7.8;

// ------------------------------------------------------------
// ORP ( Oxidation-reduction potential)
// ------------------------------------------------------------
// Ideal ORP is 650mv. 450mV is the minimum and 750mV the maximum
const int PM_ORP_Min = 450;
const int PM_ORP_Max = 750;

// ------------------------------------------------------------
// Pressure in the filtration circuit
// ------------------------------------------------------------
// The pressure of the filtration circuit is taken with a pressure sensor installed on the filtration group
// This pressure is near null when the filtration is OFF, but between 1 and 2 bars when it is ON.
// In case of surpressure, an alert must be raised on the buzzer and on the web site
const float PM_Pressure_Max = 2.0;


// ========================================================================================================
// End of customization
// ========================================================================================================
//Comparator of PM_FiltrationPeriod on Priority of the period
static bool PM_FiltrationPeriod_Priority_Cmp(const PM_FiltrationPeriod& lPeriod, const PM_FiltrationPeriod& rPeriod);

//Comparator of PM_FiltrationPeriod on Start of the period
static bool PM_FiltrationPeriod_Start_Cmp(const PM_FiltrationPeriod& lPeriod, const PM_FiltrationPeriod& rPeriod);

//Comparator of PM_FiltrationPeriod on End of the period
static bool PM_FiltrationPeriod_End_Cmp(const PM_FiltrationPeriod& lPeriod, const PM_FiltrationPeriod& rPeriod);

class PM_Pool_Configuration {
  private:
    // Private functions

    
  public:
    // Constructors
    PM_Pool_Configuration();

    // Verify that the abaqus describing the filtration time is valid
    PM_Error CheckFiltrationTimeAbaqus();

    // Verify that the abaqus describing the filtration period is valid
    PM_Error CheckFiltrationPeriodAbaqus();
  
};

#endif