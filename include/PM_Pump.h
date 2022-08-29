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
    unsigned long GetUpTime();
    void SetTankVolume(double Volume);
    void SetFlowRate(double FlowRate);
    bool Interlock();
    void SetMaxUpTime(unsigned long Max);
    void ResetUpTime();
    void SetTankFill(double);
    double GetTankFill();

    void ClearErrors();
    
    unsigned long UpTime;
    unsigned long MaxUpTime;
    unsigned long CurrMaxUpTime;
    bool UpTimeError;
    unsigned long StartTime;
    unsigned long LastStartTime;
    unsigned long StopTime; 
    double flowrate, tankvolume, tankfill;          
  private:
     
    uint8_t pumppin; 
    uint8_t isrunningsensorpin;
    uint8_t tanklevelpin;
    uint8_t interlockpin;

};
#endif
