/*
  Copyright 2022 - Yves Gaignard
  
  Functions relatives to the Wifi
*/
#ifndef PM_Utils_h
#define PM_Utils_h

// Standard library definitions
#include <Arduino.h>

// Convert integer to std::string
static const std::string PM_itoa(int i) {
  char buffer[12] = {0};
  std::string _str = itoa(i, buffer, 10);
  return _str;
}

#endif