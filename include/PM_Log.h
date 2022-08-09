/*
  Copyright 2022 - Yves Gaignard
  
  Functions to manage log in Pool Manager project
  This file is coming from part of Arduino_DebugUtils.
*/
#ifndef PM_Log_h
#define PM_Log_h

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>
#include <stdarg.h>

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static int const LOG_NONE    = -1;
static int const LOG_ERROR   =  0;
static int const LOG_WARNING =  1;
static int const LOG_INFO    =  2;
static int const LOG_DEBUG   =  3;
static int const LOG_VERBOSE =  4;


#define LOG_LOG_FORMAT(format)  "[%20s:%-4u] %30s(): " format , pathToFileName(__FILE__), __LINE__, __FUNCTION__
#define LOG_E(format, ...) Log.print(LOG_ERROR  , LOG_LOG_FORMAT(format), ##__VA_ARGS__)
#define LOG_W(format, ...) Log.print(LOG_WARNING, LOG_LOG_FORMAT(format), ##__VA_ARGS__)
#define LOG_I(format, ...) Log.print(LOG_INFO   , LOG_LOG_FORMAT(format), ##__VA_ARGS__)
#define LOG_D(format, ...) Log.print(LOG_DEBUG  , LOG_LOG_FORMAT(format), ##__VA_ARGS__)
#define LOG_V(format, ...) Log.print(LOG_VERBOSE, LOG_LOG_FORMAT(format), ##__VA_ARGS__)


void setDebugMessageLevel(int const debug_level);
int  getDebugMessageLevel();

/******************************************************************************
   CLASS DECLARATION
 ******************************************************************************/

class PM_Log {

  public:

    PM_Log();

    void setDebugLevel(int const debug_level);
    int  getDebugLevel() const;

    void setDebugOutputStream(Stream * stream);

    void timestampOn();
    void timestampOff();

    void newlineOn();
    void newlineOff();

    void debugLabelOn();
    void debugLabelOff();

    void formatTimestampOn();
    void formatTimestampOff();

    void print(int const debug_level, const char * fmt, ...);
    void print(int const debug_level, const __FlashStringHelper * fmt, ...);


  private:

    bool      _timestamp_on;
    bool      _newline_on;
    bool      _print_log_label;
    bool      _format_timestamp_on;
    int       _log_level;
    Stream *  _log_output_stream;

    void vPrint(char const * fmt, va_list args);
    void printTimestamp();
    void printDebugLabel(int const log_level);
    bool shouldPrint(int const log_level) const;

};

/******************************************************************************
   EXTERN
 ******************************************************************************/

extern PM_Log Log;

#endif 
