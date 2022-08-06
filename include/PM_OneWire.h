/*
  Copyright 2022 - Yves Gaignard
  
  Functions to discover all OneWire devices
*/
#ifndef PM_OneWire_h
#define PM_OneWire_h

#include <OneWire.h>

// Scan all OneWire devices, return an array of device serial numbers 
int PM_OneWire_Scan(OneWire & one_wire, std::vector<byte[8]>& addrArray );

// Scan all OneWire devices and print all device serial numbers to the log  
void PM_OneWire_Scan_And_Print(OneWire & one_wire);

#endif