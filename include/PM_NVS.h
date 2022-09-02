/*
  Copyright 2022 - Yves Gaignard
  
  Non Volatile Storage management
*/
#ifndef PM_NVS_h
#define PM_NVS_h

#include <Arduino.h>

// Measures initialization from NVS
bool PM_NVS_Init();
// Load measures from NVS
bool PM_NVS_Load();
// Save all measures to NVS
bool PM_NVS_Save();
// functions to save any type of parameter (4 overloads with same name but different arguments)
bool PM_NVS_saveParam(const char* key, uint8_t val);
bool PM_NVS_saveParam(const char* key, bool val);
bool PM_NVS_saveParam(const char* key, unsigned long val);
bool PM_NVS_saveParam(const char* key, double val);
bool PM_NVS_saveParam(const char* key, float val);

#endif