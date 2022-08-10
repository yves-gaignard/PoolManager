/*
  Yves Gaignard
  
  Pool manager time management
*/

// Standard library definitions
#include <Arduino.h>

#include "PM_Error.h"            // Pool manager Error management

// Constructors
PM_Error::PM_Error(int error_number, PM_ERROR_LEVEL error_level, std::string error_msg) {
  initError (error_number, error_level, error_msg);
}
PM_Error::PM_Error(int error_number, std::string error_msg) {
  initError (error_number, VERBOSE, error_msg); 
}
PM_Error::PM_Error(std::string error_msg) {
  initError (0, VERBOSE, error_msg);   
}
void PM_Error::initError (int error_number, PM_ERROR_LEVEL error_level, std::string error_msg) {
  _error_number=error_number;
  _error_level=error_level;
  _error_msg=error_msg;
}

PM_Error       PM_Error::getError() {
  return *this;
}

int            PM_Error::getErrorNumber() {
  return _error_number;
}
std::string    PM_Error::getErrorNumberStr() {
  char buffer[100] = {0};
  std::string ErrorNumber_str = itoa(_error_number, buffer, 10);
  return ErrorNumber_str;
}
PM_ERROR_LEVEL PM_Error::getErrorLevel() {
  return _error_level; 
}
std::string    PM_Error::getErrorMsg() {
  return _error_msg;
}
std::string    PM_Error::getDisplayMsg() {
  char buffer[100] = {0};
  std::string ErrorNumber_str = itoa(_error_number, buffer, 10);
  std::string DisplayError_Msg = "Error: " + ErrorNumber_str + " - "+_error_msg;
  return DisplayError_Msg;
}
    