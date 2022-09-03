/*
            Pump - a simple library to handle home-pool filtration and peristaltic pumps
                 (c) Loic74 <loic74650@gmail.com> 2017-2020
Features: 

- keeps track of running time
- keeps track of Tank Levels
- set max running time limit

NB: all timings are in milliseconds
*/

#ifndef PUMP_h
#define PUMP_h
#define PUMP_VERSION "1.0.1"

#include <Arduino.h>

//Constants used in some of the functions below
#define PUMP_ON  0
#define PUMP_OFF 1
#define TANK_FULL  1
#define TANK_EMPTY 0
#define INTERLOCK_OK  0
#define INTERLOCK_NOK 1
#define NO_LEVEL 170           // Pump with tank but without level switch
#define NO_TANK 255            // Pump without tank
#define NO_INTERLOCK 255  

#define DefaultMaxUpTime 30*60*1000 //default value is 30mins  
 
class PM_Pump{
  public:

    PM_Pump(uint8_t PumpPin, uint8_t IsRunningSensorPin, uint8_t TankLevelPin = NO_TANK, 
            uint8_t Interlockpin = NO_INTERLOCK, double FlowRate = 0., double TankVolume = 0., double TankFill = 100.);
    void loop();
    bool Start();
    bool Stop();
    bool IsRunning();
    bool TankLevel();
    double GetTankUsage();
    void SetTankVolume(double Volume);
    void SetFlowRate(double FlowRate);
    bool Interlock();
    void SetMaxUpTime(unsigned long Max);
    void ResetUpTime();
    void SetUpTime(unsigned long SavedUpTime);
    void SetTankFill(double);
    double GetTankFill();

    void ClearErrors();
                         
    unsigned long UpTime;          // (in ms), run time since the last reset of Uptime (usually at midnight) 
    unsigned long MaxUpTime;       // (in ms), maximum authorized uptime (if 0, no max uptime) 
    unsigned long CurrMaxUpTime;   // (in ms), current maximum authorized uptime (if 0, no max uptime). This value is used to compare with uptime. It is initialized with MaxUpTime, but can be extended if needed
    bool UpTimeError;              // if true, means that UpTime reached CurrMaxUpTime
    unsigned long LastLoopTime;    // (in ms), timestamp of the last loop time
    unsigned long LastStartTime;   // (in ms), timestamp of the last pump start
    unsigned long StopTime;        // (in ms), not sure of its usage
    double flowrate;               // (in Liters per hour) flowrate of the pump
    double tankvolume;             // (in Liters) volume of the tank
    double tankfill;               // (in percent) percentage of liquid that tank still contain
  
  private:
    uint8_t pumppin;               // pin of pump command
    uint8_t isrunningsensorpin;    // result of the pin read to know if the pump is running or not
    uint8_t tanklevelpin;          // pin used to determine the tank level
    uint8_t interlockpin;          // pin used to determine if the pump can be run simultaneously with another pump

};
#endif
