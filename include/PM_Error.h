/*
  Copyright 2022 - Yves Gaignard
  
  Functions to manage time in Pool Manager project
*/
#ifndef PM_Error_h
#define PM_Error_h

#include <Arduino.h>

enum PM_ERROR_LEVEL { ERROR, WARNING, INFO, DEBUG, VERBOSE };

class PM_Error {
  private:
    int             _error_number;
    PM_ERROR_LEVEL  _error_level;
    std::string     _error_msg;

    void initError (int error_number, PM_ERROR_LEVEL error_level, std::string error_msg);

  public:
    // Constructors
    PM_Error(int error_number, PM_ERROR_LEVEL error_level, std::string error_msg);
    PM_Error(int error_number, std::string error_msg);
    PM_Error(std::string error_msg);
    
    PM_Error       getError();
    int            getErrorNumber();
    std::string    getErrorNumberStr();
    PM_ERROR_LEVEL getErrorLevel();
    std::string    getErrorMsg();
    std::string    getDisplayMsg();
    
};

#endif