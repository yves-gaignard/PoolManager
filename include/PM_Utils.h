/*
  Copyright 2022 - Yves Gaignard
  
  Functions relatives to the Wifi
*/
#ifndef PM_Utils_h
#define PM_Utils_h

// Standard library definitions
#include <Arduino.h>
#include "PM_Pool_Manager.h"

// Convert integer to std::string
static const std::string PM_itoa(int i) {
  char buffer[12] = {0};
  std::string _str = itoa(i, buffer, 10);
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

#endif