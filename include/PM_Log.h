/*
  Copyright 2022 - Yves Gaignard
  
  Functions to manage log in Pool Manager project
  This class is inspired from Arduino_DebugUtils.
*/
#ifndef PM_Log_h
#define PM_Log_h

#include <Arduino.h>
#include <map>

// Level of LOG possible:
static int const LOG_NONE    = -1;
static int const LOG_ERROR   =  0;
static int const LOG_WARNING =  1;
static int const LOG_INFO    =  2;
static int const LOG_DEBUG   =  3;
static int const LOG_VERBOSE =  4;

// Macro definitions to call easily the PM_Log::print function
// ---------------------------------------------------------------
// 
// Example: 
//           LOG_I(TAG, "Starting Project: [%s]  Version: [%s]",Project.Name.c_str(), Project.Version.c_str());
//
#define LOG_LOG_FORMAT(format)  "[%20s:%-4u] %30s(): " format , pathToFileName(__FILE__), __LINE__, __FUNCTION__
#define LOG_E(tag, format, ...) Log.print(tag, LOG_ERROR  , LOG_LOG_FORMAT(format), ##__VA_ARGS__)
#define LOG_W(tag, format, ...) Log.print(tag, LOG_WARNING, LOG_LOG_FORMAT(format), ##__VA_ARGS__)
#define LOG_I(tag, format, ...) Log.print(tag, LOG_INFO   , LOG_LOG_FORMAT(format), ##__VA_ARGS__)
#define LOG_D(tag, format, ...) Log.print(tag, LOG_DEBUG  , LOG_LOG_FORMAT(format), ##__VA_ARGS__)
#define LOG_V(tag, format, ...) Log.print(tag, LOG_VERBOSE, LOG_LOG_FORMAT(format), ##__VA_ARGS__)

// Class Declaration
// ------------------- 
class PM_Log {
  public:
    PM_Log();

    bool isValidLogLevel(int const log_level);

    void setLogLevel(int const log_level);
    int  getLogLevel() const;

    void setLogOutputStream(Stream * stream);

    void setTag(const char * tag, const int log_level);
    void setTag(const char * tag, const char * log_level);

    void timestampOn();
    void timestampOff();

    void newlineOn();
    void newlineOff();

    void logLabelOn();
    void logLabelOff();

    void formatTimestampOn();
    void formatTimestampOff();

    void setWebSerialOn();
    void setWebSerialOff();

    void print(const char * tag, int const log_level, const char * fmt, ...);
    void print(const char * tag, int const log_level, const __FlashStringHelper * fmt, ...);
private:
    bool      _timestamp_on;
    bool      _newline_on;
    bool      _print_log_label;
    bool      _format_timestamp_on;
    int       _log_level;
    bool      _is_web_serial;
    Stream *  _log_output_stream;
    std::map<std::string, int> _tags;   // unordered map of tags and log_level

    void vPrint(char const * fmt, va_list args);
    void printTimestamp();
    void printLogLabel(int const log_level);
    bool shouldPrint(const char * tag, int const log_level);
};

// Declaration of a Log class usable in any program
// ------------------------------------------------
extern PM_Log Log;

#endif 
