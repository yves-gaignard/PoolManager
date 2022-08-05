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

// Convert array of 8 bytes to std::string
static const std::string PM_OneWireDeviceAddressArrayToString(const byte addr[8]) {
  char buffer[12] = {0};
  std::string _str ="{ ";
  for (int i = 0; i < 8; i++) {
    itoa (addr[i],buffer,HEX);
    _str += itoa(i, buffer, 10);
    _str += " ";
  }
  _str += " }";
  return _str;
}

// Convert array of 8 bytes to std::string
static const std::string PM_OneWireDeviceAddressVectorToString(const std::vector<byte> & AddrVector) {
  char buffer[12] = {0};
  std::string _str ="{ ";
  for (int i = 0; i < AddrVector.size(); i++) {
    itoa (AddrVector[i],buffer,HEX);
    _str += itoa(i, buffer, 10);
    _str += " ";
  }
  _str += " }";
  return _str;
}

#endif