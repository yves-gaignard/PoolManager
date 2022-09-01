/*
  Yves Gaignard
  
  Functions to manage log in Pool Manager project
  This class is inspired from Arduino_DebugUtils.
*/

#define TAG "PM_Log"

#include <Arduino.h>
#include <ESPPerfectTime.h>
#include <WebSerialLite.h>

#include "PM_Log.h"
#include "PM_Parameters.h"

// Default values
static int const DEFAULT_LOG_LEVEL   = LOG_VERBOSE;
static Stream *  DEFAULT_OUTPUT_STREAM = &Serial;

// Constructor
PM_Log::PM_Log() {
  timestampOn();
  newlineOn();
  logLabelOn();
  formatTimestampOn();
  setLogLevel(DEFAULT_LOG_LEVEL);
  setLogOutputStream(DEFAULT_OUTPUT_STREAM);
  setWebSerialOff();
}

// Public member functions
bool PM_Log::isValidLogLevel(int const log_level) {
  return (log_level >= LOG_ERROR) && (log_level <= LOG_VERBOSE);
}

void PM_Log::setLogLevel(int const log_level) {
  if (isValidLogLevel(log_level)) _log_level = log_level;
}

int PM_Log::getLogLevel() const {
  return _log_level;
}

void PM_Log::setLogOutputStream(Stream * stream) {
  _log_output_stream = stream;
}

void PM_Log::setTag(const char * tag, const int log_level) {
  if (isValidLogLevel(log_level)) {
    // if tag == "*", then set the default log_level
    if (strcmp(tag, "*") == 0 ) {
      setLogLevel(log_level);
      //Serial.println("setTag cas *");
    }
    else {
      if (_tags.find(tag) != _tags.end()) {
        // a same tag already exist, change its value with the new one
        _tags[tag] = log_level;
        //Serial.println("setTag cas replace tag");
      }
      else {
        // add the new tag
        _tags.insert( {tag, log_level});
        //Serial.println("setTag cas insert tag");
      }
    }
  }
}

void PM_Log::newlineOn() {
  _newline_on = true;
}

void PM_Log::newlineOff() {
  _newline_on = false;
}

void PM_Log::logLabelOn() {
  _print_log_label = true;
}

void PM_Log::logLabelOff() {
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

void PM_Log::setWebSerialOn() {
  _is_web_serial = true;
}

void PM_Log::setWebSerialOff() {
  _is_web_serial = false;
}

void PM_Log::print(const char * tag, int const log_level, const char * fmt, ...) {
  if (!shouldPrint(tag, log_level))
    return;

  if (_timestamp_on)
    printTimestamp();

  if (_print_log_label)
    printLogLabel(log_level);

  va_list args;
  va_start(args, fmt);
  vPrint(fmt, args);
  va_end(args);
}

void PM_Log::print(const char * tag, int const log_level, const __FlashStringHelper * fmt, ...) {
  if (!shouldPrint(tag, log_level))
    return;

  if (_timestamp_on)
    printTimestamp();

  if (_print_log_label)
    printLogLabel(log_level);

  String fmt_str(fmt);

  va_list args;
  va_start(args, fmt);
  vPrint(fmt_str.c_str(), args);
  va_end(args);
}


// Private member functions
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
    if (_is_web_serial) WebSerial.println(msg_buf);
  } 
  else {
    _log_output_stream->print(msg_buf);
        if (_is_web_serial) WebSerial.print(msg_buf);

  }

#if __STDC_NO_VLA__ == 1
  // remember to clean up memory
  delete[] msg_buf;
#endif
}

void PM_Log::printTimestamp()
{
  struct tm *tm ;
  suseconds_t usec;
  tm = pftime::localtime(nullptr, &usec);
  long int msec = usec / 1000;

  char timestamp[32];

  if (_format_timestamp_on) {
    // "prints" formatted output to a char array (string)
    snprintf(timestamp, sizeof(timestamp), "[%02d:%02d:%02d.%03ld]", tm->tm_hour, tm->tm_min, tm->tm_sec, msec );
  }
  else {
    // "prints" time in milliseconds from the beginning of the day
    unsigned long time_msec = ((tm->tm_hour * 3600) + (tm->tm_min * 60) + (tm->tm_sec))*1000 + msec;
    snprintf(timestamp, sizeof(timestamp), "[%8lu] ", time_msec);
  }

  _log_output_stream->print(timestamp);
  if (_is_web_serial) WebSerial.print(timestamp);

}

void PM_Log::printLogLabel(int const log_level)
{
  static char const * LOG_MODE_STRING[5] = { "[E]", "[W]", "[I]", "[D]", "[V]" };

  if ( ! isValidLogLevel(log_level))
    return;

  _log_output_stream->print(LOG_MODE_STRING[log_level]);
  if (_is_web_serial) WebSerial.print(LOG_MODE_STRING[log_level]);

}

bool PM_Log::shouldPrint(const char * tag, int const log_level) {
  // int numTag = _tags.size();
  // Serial.print("Number of Tag =");
  // Serial.println(numTag);

  // check if the tag is present in the tag list
  if (_tags.find(tag) != _tags.end()) {
    // yes, it exist, so compare with it
    int tag_log_level = _tags[tag];
    /* Serial.print("case 1: ");
    Serial.print(" log_level=");
    Serial.print(log_level);
    Serial.print(" tag_log_level=");
    Serial.print(tag_log_level);
    Serial.print(" Comparaison=");
    Serial.println((log_level <= tag_log_level)) ;*/
    return (log_level <= tag_log_level);
  } 
  else { 
    // no, so compare with the default log level
    /* Serial.print("case 2: ");
    Serial.print(" log_level=");
    Serial.print(log_level);
    Serial.print(" _log_level=");
    Serial.print(_log_level);
    Serial.print(" Comparaison=");
    Serial.println(( isValidLogLevel(log_level) && (log_level <= _log_level))) ;*/
    return ( isValidLogLevel(log_level) && (log_level <= _log_level));
  }
}

// CLASS INSTANTIATION
// =====================
PM_Log Log;
