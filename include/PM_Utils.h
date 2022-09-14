/*
  Copyright 2022 - Yves Gaignard
  
  Functions relatives to the Wifi
*/
#ifndef PM_Utils_h
#define PM_Utils_h

// Standard library definitions
#include <Arduino.h>
#include <string>
#include <vector>
#include "PM_Pool_Manager.h"

// Convert integer to std::string
static const std::string PM_itoa(int i) {
  char buffer[12] = {0};
  std::string _str = itoa(i, buffer, 10);
  return _str;
}

// Convert float to std::string
static const std::string PM_ftoa(float f, const char * format = "%6.2f") {
  char buffer[20] = {0};
  sprintf(buffer, format, f);
  std::string _str = buffer;
  return _str;
}

// Convert double to std::string
static const std::string PM_dtoa(double f, const char * format = "%6.2f") {
  char buffer[20] = {0};
  sprintf(buffer, format, f);
  std::string _str = buffer;
  return _str;
}

// Convert an int to hex string
template <typename I> 
static std::string n2hexstr(const I w, size_t hex_len = sizeof(I)<<1) {
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len,'0');
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
        rc[i] = digits[(w>>j) & 0x0f];
    return rc;
}

// Convert a hex string to a long
static long hexstr2n( const std::string str) {
    char * p;
    long n = strtoul( str.c_str(), & p, 16 );
    if ( * p != 0 ) { //my bad edit was here
      LOG_D(TAG, "String: %s is not an hexadecimal number", str.c_str());
      return 0;
    }
    else {
        return n;
    }
}

static void ExtractWordsFromString(const std::string Text, std::vector<std::string>& words) {
  std::string pushBackVar;
  int start_sub=0;
  int i;
  for (i=0;i<Text.length();i++)
  {
    if(Text[i]==32)
    {
      if (i > start_sub) {
        pushBackVar = Text.substr(start_sub, i);
        words.push_back(pushBackVar);
      }
      start_sub=i+1;
    }  
  }
  if (start_sub<Text.length()) {
    pushBackVar = Text.substr(start_sub, Text.length());
    words.push_back(pushBackVar);
  }
}

static bool string_iequals(const std::string& a, const std::string& b)
{
    unsigned int sz = a.size();
    if (b.size() != sz)
        return false;
    for (unsigned int i = 0; i < sz; ++i)
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    return true;
}

static bool string_endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

static bool string_startsWith(const std::string& str, const std::string& prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

static bool string_endsWith(const std::string& str, const char* suffix, unsigned suffixLen)
{
    return str.size() >= suffixLen && 0 == str.compare(str.size()-suffixLen, suffixLen, suffix, suffixLen);
}

static bool string_endsWith(const std::string& str, const char* suffix)
{
    return string_endsWith(str, suffix, std::string::traits_type::length(suffix));
}

static bool string_startsWith(const std::string& str, const char* prefix, unsigned prefixLen)
{
    return str.size() >= prefixLen && 0 == str.compare(0, prefixLen, prefix, prefixLen);
}

static bool string_startsWith(const std::string& str, const char* prefix)
{
    return string_startsWith(str, prefix, std::string::traits_type::length(prefix));
}
#endif