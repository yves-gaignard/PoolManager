/*
  Copyright 2022 - Yves Gaignard
  
  Functions to manage the differents screens
*/
#ifndef PM_SCREENS_h
#define PM_SCREENS_h

#include <Arduino.h>
#include "PM_Parameters.h"


class PM_Screens {
  public:
    PM_Screens(int inactivityMaxTime = LCD_DISPLAY_TIMEOUT, int duration = LCD_DISPLAY_SCREEN_DURATION);
    void   addScreen(std::string screenName);
    void   removeScreen(std::string screenName);
    void   setInactivityMaxTime(int maxSeconds);
    void   setScreenSwitchTime(int seconds);
    void   setInactivityTimeOutReset();
    void   setCurrentScreen(int screenIndex);

    int    getScreenNumber();
    int    getCurrentScreenIndex();
    int    getScreenIndex(std::vector<std::string> screenName);
    time_t getDisplayStart();
    int    getInactivityMaxTime();
    int    getLastScreenStartTime();
    int    getScreenSwitchTime();

  private:
    std::vector<std::string> _screenNames; // vector of screens names
    int     _inactivityMaxTime;            // time out to shutdown the screen light
    int     _screenDuration;               // duration in seconds of a screen display before switching to the next time
    time_t  _lastScreenActivationStart;    // time of the last display activation without any user interaction
    time_t  _currentScreenStart;           // time of the current screen start
    int     _currentScreenIndex;           // Current displayed screen index 
};

#endif