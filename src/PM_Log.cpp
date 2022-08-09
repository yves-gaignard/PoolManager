/*
  Yves Gaignard
  
  Functions to manage log in Pool Manager project
  This file is coming from part of Arduino_DebugUtils.
*/

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "PM_Log.h"

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static int const DEFAULT_LOG_LEVEL   = LOG_INFO;
static Stream *  DEFAULT_OUTPUT_STREAM = &Serial;

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

PM_Log::PM_Log() {
  timestampOff();
  newlineOn();
  debugLabelOff();
  formatTimestampOff();
  setDebugLevel(DEFAULT_LOG_LEVEL);
  setDebugOutputStream(DEFAULT_OUTPUT_STREAM);
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void PM_Log::setDebugLevel(int const log_level) {
  _log_level = log_level;
}

int PM_Log::getDebugLevel() const {
  return _log_level;
}

void PM_Log::setDebugOutputStream(Stream * stream) {
  _log_output_stream = stream;
}

void PM_Log::newlineOn() {
    _newline_on = true;
}

void PM_Log::newlineOff() {
    _newline_on = false;
}

void PM_Log::debugLabelOn() {
  _print_log_label = true;
}

void PM_Log::debugLabelOff() {
  _print_log_label = false;
}

void PM_Log::formatTimestampOn() {
  _format_timestamp_on = true;
}

void PM_Log::formatTimestampOff() {
  _format_timestamp_on = false;
}

void PM_Log::timestampOn() {
  _timestamp_on = true;
}

void PM_Log::timestampOff() {
  _timestamp_on = false;
}

void PM_Log::print(int const log_level, const char * fmt, ...)
{
  if (!shouldPrint(log_level))
    return;

  if (_timestamp_on)
    printTimestamp();

  if (_print_log_label)
    printDebugLabel(log_level);

  va_list args;
  va_start(args, fmt);
  vPrint(fmt, args);
  va_end(args);
}

void PM_Log::print(int const log_level, const __FlashStringHelper * fmt, ...)
{
  if (!shouldPrint(log_level))
    return;

  if (_timestamp_on)
    printTimestamp();

  if (_print_log_label)
    printDebugLabel(log_level);

  String fmt_str(fmt);

  va_list args;
  va_start(args, fmt);
  vPrint(fmt_str.c_str(), args);
  va_end(args);
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

void PM_Log::vPrint(char const * fmt, va_list args) {
  // calculate required buffer length
  int msg_buf_size = vsnprintf(nullptr, 0, fmt, args) + 1; // add one for null terminator
#if __STDC_NO_VLA__ == 1
  // in the rare case where VLA is not allowed by compiler, fall back on heap-allocated memory
  char * msg_buf = new char[msg_buf_size];
#else
  char msg_buf[msg_buf_size];
#endif

  vsnprintf(msg_buf, msg_buf_size, fmt, args);

  if (_newline_on) {
    _log_output_stream->println(msg_buf);
  } else {
    _log_output_stream->print(msg_buf);
  }

#if __STDC_NO_VLA__ == 1
  // remember to clean up memory
  delete[] msg_buf;
#endif
}

void PM_Log::printTimestamp()
{
  char timestamp[32];

  if (_format_timestamp_on)
  {
    auto    const  msCount      = millis();

    uint16_t const milliseconds = msCount % 1000;           // ms remaining when converted to seconds
    uint16_t const allSeconds   = msCount / 1000;           // total number of seconds to calculate remaining values

    uint16_t const hours            = allSeconds / 3600;    // convert seconds to hours
    uint16_t const secondsRemaining = allSeconds % 3600;    // seconds left over

    uint16_t const minutes  = secondsRemaining / 60 ;       // convert seconds left over to minutes
    uint16_t const seconds  = secondsRemaining % 60;        // seconds left over

    snprintf(timestamp, sizeof(timestamp),                  // "prints" formatted output to a char array (string)
                "["
                "%02d:"   //HH:
                "%02d:"   //MM:
                "%02d."   //SS.
                "%03d"    //MMM
                "]",
                hours,
                minutes,
                seconds,
                milliseconds
            );
  }
  else
  {
    snprintf(timestamp, sizeof(timestamp), "[ %lu ] ", millis());
  }

  _log_output_stream->print(timestamp);
}

void PM_Log::printDebugLabel(int const log_level)
{
  static char const * LOG_MODE_STRING[5] =
  {
    "[E]",
    "[W]",
    "[I]",
    "[D]",
    "[V]",
  };

  bool is_valid_log_level = (log_level >= LOG_ERROR) && (log_level <= LOG_VERBOSE);
  if (!is_valid_log_level)
    return;

  _log_output_stream->print(LOG_MODE_STRING[log_level]);
}

bool PM_Log::shouldPrint(int const log_level) const
{
  return ((log_level >= LOG_ERROR) && (log_level <= LOG_VERBOSE) && (log_level <= _log_level));
}

/******************************************************************************
   CLASS INSTANTIATION
 ******************************************************************************/

PM_Log Log;

void setDebugMessageLevel(int const log_level) {
  Log.setDebugLevel(log_level);
}

int getDebugMessageLevel() {
  return Log.getDebugLevel();
}
