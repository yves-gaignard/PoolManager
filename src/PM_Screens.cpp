/*
  Yves Gaignard
  
  Pool manager screens management
*/

#define TAG "PM_Screens"

// Standard library definitions
#include <Arduino.h>

#include "PM_Screens.h"
#include "PM_Log.h"

PM_Screens::PM_Screens(int inactivityMaxTime, int duration) {
  _inactivityMaxTime = inactivityMaxTime;
  _screenDuration = duration;
  time(&_currentScreenStart);
  LOG_D(TAG, "constructor: inactivityMaxTime=%d, duration=%d, time =%d",_inactivityMaxTime, _screenDuration, _currentScreenStart);
}
void PM_Screens::addScreen(std::string screenName) {
  if (std::find(_screenNames.begin(), _screenNames.end(), screenName) == _screenNames.end()) {
    _screenNames.push_back(screenName);
  }
}
void PM_Screens::removeScreen(std::string screenName) {
  _screenNames.erase(std::remove(_screenNames.begin(), _screenNames.end(), screenName), _screenNames.end());
}
void PM_Screens::setInactivityMaxTime(int maxSeconds) {
  _inactivityMaxTime = maxSeconds;
}
void PM_Screens::setScreenSwitchTime(int seconds) {
  _screenDuration = seconds;
}
void PM_Screens::setInactivityTimeOutReset() {
  //LOG_D(TAG, "before _lastScreenActivationStart: time =%d",_currentScreenStart);
  time(&_lastScreenActivationStart);
  //LOG_D(TAG, "_lastScreenActivationStart: time =%d",_currentScreenStart);
}
void PM_Screens::setCurrentScreen(int screenIndex) {
  _currentScreenIndex = screenIndex;
  //LOG_D(TAG, "before setCurrentTime: time =%d",_currentScreenStart);
  time(&_currentScreenStart);
  //LOG_D(TAG, "setCurrentScreen: time =%d",_currentScreenStart);
}
int PM_Screens::getScreenNumber() {
  return _screenNames.size();
}
int  PM_Screens::getCurrentScreenIndex() {
  return _currentScreenIndex;
}
time_t PM_Screens::getDisplayStart(){
  return _lastScreenActivationStart;
}
int PM_Screens::getInactivityMaxTime() {
  return _inactivityMaxTime;
}
int PM_Screens::getLastScreenStartTime() {
  return _currentScreenStart;
}
int PM_Screens::getScreenSwitchTime() {
  return _screenDuration;
}